/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor driver for rp2 port.
 */
#if MICROPY_PY_SENSOR
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "omv_i2c.h"
#include "sensor.h"
#include "framebuffer.h"

#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "dcmi.pio.h"

// Sensor struct.
sensor_t sensor = {};

static void dma_irq_handler();
extern void __fatal_error(const char *msg);

static void sensor_dma_config(int w, int h, int bpp, uint32_t *capture_buf, bool rev_bytes) {
    dma_channel_abort(OMV_CSI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL, false);

    dma_channel_config c = dma_channel_get_default_config(OMV_CSI_DMA_CHANNEL);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(OMV_CSI_PIO, OMV_CSI_SM, false));
    channel_config_set_bswap(&c, rev_bytes);

    dma_channel_configure(OMV_CSI_DMA_CHANNEL, &c,
                          capture_buf, // Destinatinon pointer.
                          &OMV_CSI_PIO->rxf[OMV_CSI_SM], // Source pointer.
                          (w * h * bpp) >> 2, // Number of transfers in words.
                          true      // Start immediately, will block on SM.
                          );

    // Re-enable DMA IRQs.
    dma_irqn_set_channel_enabled(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL, true);
}

int sensor_init() {
    int init_ret = 0;

    // PIXCLK
    gpio_init(OMV_CSI_PXCLK_PIN);
    gpio_set_dir(OMV_CSI_PXCLK_PIN, GPIO_IN);

    // HSYNC
    gpio_init(OMV_CSI_HSYNC_PIN);
    gpio_set_dir(OMV_CSI_HSYNC_PIN, GPIO_IN);

    // VSYNC
    gpio_init(OMV_CSI_VSYNC_PIN);
    gpio_set_dir(OMV_CSI_VSYNC_PIN, GPIO_IN);

    #if defined(OMV_CSI_POWER_PIN)
    gpio_init(OMV_CSI_POWER_PIN);
    gpio_set_dir(OMV_CSI_POWER_PIN, GPIO_OUT);
    gpio_pull_down(OMV_CSI_POWER_PIN);
    gpio_put(OMV_CSI_POWER_PIN, 1);
    #endif

    #if defined(OMV_CSI_RESET_PIN)
    gpio_init(OMV_CSI_RESET_PIN);
    gpio_set_dir(OMV_CSI_RESET_PIN, GPIO_OUT);
    gpio_pull_up(OMV_CSI_RESET_PIN);
    gpio_put(OMV_CSI_RESET_PIN, 1);
    #endif

    // Reset the sensor state
    memset(&sensor, 0, sizeof(sensor_t));

    // Set default snapshot function.
    // Some sensors need to call snapshot from init.
    sensor.snapshot = sensor_snapshot;

    // Configure the sensor external clock (XCLK).
    if (sensor_set_xclk_frequency(OMV_CSI_XCLK_FREQUENCY) != 0) {
        // Failed to initialize the sensor clock.
        return SENSOR_ERROR_TIM_INIT_FAILED;
    }

    // Detect and initialize the image sensor.
    if ((init_ret = sensor_probe_init(OMV_CSI_I2C_ID, OMV_CSI_I2C_SPEED)) != 0) {
        // Sensor probe/init failed.
        return init_ret;
    }

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    // Set new DMA IRQ handler.
    // Disable IRQs.
    irq_set_enabled(OMV_CSI_DMA_IRQ, false);

    // Clear DMA interrupts.
    dma_irqn_acknowledge_channel(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL);

    // Remove current handler if any
    irq_handler_t irq_handler = irq_get_exclusive_handler(OMV_CSI_DMA_IRQ);
    if (irq_handler != NULL) {
        irq_remove_handler(OMV_CSI_DMA_IRQ, irq_handler);
    }

    // Set new exclusive IRQ handler.
    irq_set_exclusive_handler(OMV_CSI_DMA_IRQ, dma_irq_handler);
    // Or set shared IRQ handler, but this needs to be called once.
    // irq_add_shared_handler(OMV_CSI_DMA_IRQ, dma_irq_handler, PICO_DEFAULT_IRQ_PRIORITY);

    irq_set_enabled(OMV_CSI_DMA_IRQ, true);

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Disable Frame callback.
    sensor_set_frame_callback(NULL);

    /* All good! */
    sensor.detected = true;

    return 0;
}

int sensor_abort(bool fifo_flush, bool in_irq) {
    // Disable DMA channel
    dma_channel_abort(OMV_CSI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL, false);

    // Disable state machine.
    pio_sm_set_enabled(OMV_CSI_PIO, OMV_CSI_SM, false);
    pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);

    // Clear bpp flag.
    MAIN_FB()->pixfmt = PIXFORMAT_INVALID;

    return 0;
}

int sensor_set_xclk_frequency(uint32_t frequency) {
    uint32_t p = 4;

    // Allocate pin to the PWM
    gpio_set_function(OMV_CSI_MXCLK_PIN, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the GPIO
    uint slice_num = pwm_gpio_to_slice_num(OMV_CSI_MXCLK_PIN);

    // Set period to p cycles
    pwm_set_wrap(slice_num, p - 1);

    // Set channel A 50% duty cycle.
    pwm_set_chan_level(slice_num, PWM_CHAN_A, p / 2);

    // Set sysclk divider
    // f = 125000000 / (p * (1 + (p/16)))
    pwm_set_clkdiv_int_frac(slice_num, 1, p);

    // Set the PWM running
    pwm_set_enabled(slice_num, true);

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h) {
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

static void dma_irq_handler() {
    if (dma_irqn_get_channel_status(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL)) {
        // Clear the interrupt request.
        dma_irqn_acknowledge_channel(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL);

        framebuffer_get_tail(FB_NO_FLAGS);
        vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer != NULL) {
            // Set next buffer and retrigger the DMA channel.
            dma_channel_set_write_addr(OMV_CSI_DMA_CHANNEL, buffer->data, true);

            // Unblock the state machine
            pio_sm_restart(OMV_CSI_PIO, OMV_CSI_SM);
            pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);
            pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (MAIN_FB()->v - 1));
            pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (MAIN_FB()->u * MAIN_FB()->bpp) - 1);
        }
    }
}

// This is the default snapshot function, which can be replaced in sensor_init functions.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags) {
    // Compress the framebuffer for the IDE preview.
    framebuffer_update_jpeg_buffer();

    if (sensor_check_framebuffer_size() != 0) {
        return SENSOR_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    // Free the current FB head.
    framebuffer_free_current_buffer();

    // Set framebuffer pixel format.
    if (sensor->pixformat == PIXFORMAT_INVALID) {
        return SENSOR_ERROR_INVALID_PIXFORMAT;
    }
    MAIN_FB()->pixfmt = sensor->pixformat;

    vbuffer_t *buffer = framebuffer_get_head(FB_NO_FLAGS);

    // If there's no ready buffer in the fifo, and the DMA is Not currently
    // transferring a new buffer, reconfigure and restart the DMA transfer.
    if (buffer == NULL && !dma_channel_is_busy(OMV_CSI_DMA_CHANNEL)) {
        framebuffer_setup_buffers();

        buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer == NULL) {
            return SENSOR_ERROR_FRAMEBUFFER_ERROR;
        }

        // Configure the DMA on the first frame, for later frames only the write is changed.
        sensor_dma_config(MAIN_FB()->u, MAIN_FB()->v, MAIN_FB()->bpp,
                          (void *) buffer->data, (sensor->hw_flags.rgb_swap && MAIN_FB()->bpp == 2));


        // Re-enable the state machine.
        pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);
        pio_sm_set_enabled(OMV_CSI_PIO, OMV_CSI_SM, true);

        // Unblock the state machine
        pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (MAIN_FB()->v - 1));
        pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (MAIN_FB()->u * MAIN_FB()->bpp) - 1);
    }

    // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        buffer = framebuffer_get_head(FB_NO_FLAGS);
        if ((mp_hal_ticks_ms() - ticks) > 3000) {
            sensor_abort(true, false);
            return SENSOR_ERROR_CAPTURE_TIMEOUT;
        }
    }

    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

    // Set the user image.
    framebuffer_init_image(image);
    return 0;
}
#endif
