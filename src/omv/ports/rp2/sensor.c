/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer for nRF port.
 */
#if MICROPY_PY_SENSOR
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "cambus.h"
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

static void sensor_dma_config(int w, int h, int bpp, uint32_t *capture_buf, bool rev_bytes)
{
    dma_channel_abort(DCMI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(DCMI_DMA, DCMI_DMA_CHANNEL, false);

    dma_channel_config c = dma_channel_get_default_config(DCMI_DMA_CHANNEL);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(DCMI_PIO, DCMI_SM, false));
    channel_config_set_bswap(&c, rev_bytes);

    dma_channel_configure(DCMI_DMA_CHANNEL, &c,
        capture_buf,                // Destinatinon pointer.
        &DCMI_PIO->rxf[DCMI_SM],    // Source pointer.
        (w*h*bpp)>>2,               // Number of transfers in words.
        true                        // Start immediately, will block on SM.
    );

    // Re-enable DMA IRQs.
    dma_irqn_set_channel_enabled(DCMI_DMA, DCMI_DMA_CHANNEL, true);
}

int sensor_init()
{
    int init_ret = 0;

    // PIXCLK
    gpio_init(DCMI_PXCLK_PIN);
    gpio_set_dir(DCMI_PXCLK_PIN, GPIO_IN);

    // HSYNC
    gpio_init(DCMI_HSYNC_PIN);
    gpio_set_dir(DCMI_HSYNC_PIN, GPIO_IN);

    // VSYNC
    gpio_init(DCMI_VSYNC_PIN);
    gpio_set_dir(DCMI_VSYNC_PIN, GPIO_IN);

    #if defined(DCMI_PWDN_PIN)
    gpio_init(DCMI_PWDN_PIN);
    gpio_set_dir(DCMI_PWDN_PIN, GPIO_OUT);
    gpio_pull_down(DCMI_PWDN_PIN);
    DCMI_PWDN_HIGH();
    #endif

    #if defined(DCMI_RESET_PIN)
    gpio_init(DCMI_RESET_PIN);
    gpio_set_dir(DCMI_RESET_PIN, GPIO_OUT);
    gpio_pull_up(DCMI_RESET_PIN);
    DCMI_RESET_HIGH();
    #endif

    // Reset the sesnor state
    memset(&sensor, 0, sizeof(sensor_t));

    // Set default snapshot function.
    // Some sensors need to call snapshot from init.
    sensor.snapshot = sensor_snapshot;

    // Configure the sensor external clock (XCLK).
    if (sensor_set_xclk_frequency(OMV_XCLK_FREQUENCY) != 0) {
        // Failed to initialize the sensor clock.
        return SENSOR_ERROR_TIM_INIT_FAILED;
    }

    // Detect and initialize the image sensor.
    if ((init_ret = sensor_probe_init()) != 0) {
        // Sensor probe/init failed.
        return init_ret;
    }

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    // Set new DMA IRQ handler.
    // Disable IRQs.
    irq_set_enabled(DCMI_DMA_IRQ, false);

    // Clear DMA interrupts.
    dma_irqn_acknowledge_channel(DCMI_DMA, DCMI_DMA_CHANNEL);

    // Remove current handler if any
    irq_handler_t irq_handler = irq_get_exclusive_handler(DCMI_DMA_IRQ);
    if (irq_handler != NULL) {
        irq_remove_handler(DCMI_DMA_IRQ, irq_handler);
    }

    // Set new exclusive IRQ handler.
    irq_set_exclusive_handler(DCMI_DMA_IRQ, dma_irq_handler);
    // Or set shared IRQ handler, but this needs to be called once.
    // irq_add_shared_handler(DCMI_DMA_IRQ, dma_irq_handler, PICO_DEFAULT_IRQ_PRIORITY);

    irq_set_enabled(DCMI_DMA_IRQ, true);

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Disable Frame callback.
    sensor_set_frame_callback(NULL);

    /* All good! */
    sensor.detected = true;

    return 0;
}

int sensor_abort()
{
    // Disable DMA channel
    dma_channel_abort(DCMI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(DCMI_DMA, DCMI_DMA_CHANNEL, false);

    // Disable state machine.
    pio_sm_set_enabled(DCMI_PIO, DCMI_SM, false);
    pio_sm_clear_fifos(DCMI_PIO, DCMI_SM);

    // Clear bpp flag.
    MAIN_FB()->bpp = -1;

    return 0;
}

int sensor_set_xclk_frequency(uint32_t frequency)
{
    uint32_t p = 4;

    // Allocate pin to the PWM
    gpio_set_function(DCMI_XCLK_PIN, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to the GPIO
    uint slice_num = pwm_gpio_to_slice_num(DCMI_XCLK_PIN);

    // Set period to p cycles
    pwm_set_wrap(slice_num, p-1);

    // Set channel A 50% duty cycle.
    pwm_set_chan_level(slice_num, PWM_CHAN_A, p/2);

    // Set sysclk divider
    // f = 125000000 / (p * (1 + (p/16)))
    pwm_set_clkdiv_int_frac(slice_num, 1, p);

    // Set the PWM running
    pwm_set_enabled(slice_num, true);

    return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
    return SENSOR_ERROR_CTL_UNSUPPORTED;
}

static void dma_irq_handler()
{
    if (dma_irqn_get_channel_status(DCMI_DMA, DCMI_DMA_CHANNEL)) {
        // Clear the interrupt request.
        dma_irqn_acknowledge_channel(DCMI_DMA, DCMI_DMA_CHANNEL);

        framebuffer_get_tail(FB_NO_FLAGS);
        vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer != NULL) {
            // Set next buffer and retrigger the DMA channel.
            dma_channel_set_write_addr(DCMI_DMA_CHANNEL, buffer->data, true);

            // Unblock the state machine
            pio_sm_restart(DCMI_PIO, DCMI_SM);
            pio_sm_clear_fifos(DCMI_PIO, DCMI_SM);
            pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->v - 1));
            pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->u * MAIN_FB()->bpp) - 1);
        }
    }
}

// This is the default snapshot function, which can be replaced in sensor_init functions.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    // Compress the framebuffer for the IDE preview.
    framebuffer_update_jpeg_buffer();

    if (sensor_check_framebuffer_size() != 0) {
        return SENSOR_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    // Free the current FB head.
    framebuffer_free_current_buffer();

    switch (sensor->pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            MAIN_FB()->bpp = 1;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_RGB565:
            MAIN_FB()->bpp = 2;
            break;
        default:
            return SENSOR_ERROR_INVALID_PIXFORMAT;
    }

    vbuffer_t *buffer = framebuffer_get_head(FB_NO_FLAGS);

    // If there's no ready buffer in the fifo, and the DMA is Not currently
    // transferring a new buffer, reconfigure and restart the DMA transfer.
    if (buffer == NULL && !dma_channel_is_busy(DCMI_DMA_CHANNEL)) {
        buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer == NULL) {
            return SENSOR_ERROR_FRAMEBUFFER_ERROR;
        }

        // Configure the DMA on the first frame, for later frames only the write is changed.
        sensor_dma_config(MAIN_FB()->u, MAIN_FB()->v, MAIN_FB()->bpp, (void *) buffer->data,
                (SENSOR_HW_FLAGS_GET(sensor, SENSOR_HW_FLAGS_RGB565_REV) && MAIN_FB()->bpp == 2));


        // Re-enable the state machine.
        pio_sm_clear_fifos(DCMI_PIO, DCMI_SM);
        pio_sm_set_enabled(DCMI_PIO, DCMI_SM, true);

        // Unblock the state machine
        pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->v - 1));
        pio_sm_put_blocking(DCMI_PIO, DCMI_SM, (MAIN_FB()->u * MAIN_FB()->bpp) - 1);
    }

    // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        buffer = framebuffer_get_head(FB_NO_FLAGS);
        if ((mp_hal_ticks_ms() - ticks) > 3000) {
            sensor_abort();
            return SENSOR_ERROR_CAPTURE_TIMEOUT;
        }
    }

    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

    // Set the user image.
    if (image != NULL) {
        image->w      = MAIN_FB()->w;
        image->h      = MAIN_FB()->h;
        image->bpp    = MAIN_FB()->bpp;
        image->pixels = buffer->data;
    }

    return 0;
}
#endif
