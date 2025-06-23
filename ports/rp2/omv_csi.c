/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2021-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Sensor driver for rp2 port.
 */
#if MICROPY_PY_CSI
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "omv_i2c.h"
#include "omv_csi.h"

#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "dcmi.pio.h"

static void dma_irq_handler();
extern void __fatal_error(const char *msg);

static void omv_csi_dma_config(int w, int h, int bpp, uint32_t *capture_buf, bool rev_bytes) {
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

int omv_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    // Disable DMA channel
    dma_channel_abort(OMV_CSI_DMA_CHANNEL);
    dma_irqn_set_channel_enabled(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL, false);

    // Disable state machine.
    pio_sm_set_enabled(OMV_CSI_PIO, OMV_CSI_SM, false);
    pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);

    if (csi->fb) {
        // Clear bpp flag.
        csi->fb->pixfmt = PIXFORMAT_INVALID;
    }

    return 0;
}

int omv_csi_set_clk_frequency(uint32_t frequency) {
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

int omv_csi_set_windowing(int x, int y, int w, int h) {
    return OMV_CSI_ERROR_CTL_UNSUPPORTED;
}

static void dma_irq_handler() {
    framebuffer_t *fb = csi.fb;

    if (dma_irqn_get_channel_status(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL)) {
        // Clear the interrupt request.
        dma_irqn_acknowledge_channel(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL);

        framebuffer_get_tail(fb, FB_NO_FLAGS);
        vbuffer_t *buffer = framebuffer_get_tail(fb, FB_PEEK);
        if (buffer != NULL) {
            // Set next buffer and retrigger the DMA channel.
            dma_channel_set_write_addr(OMV_CSI_DMA_CHANNEL, buffer->data, true);

            // Unblock the state machine
            pio_sm_restart(OMV_CSI_PIO, OMV_CSI_SM);
            pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);
            pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (fb->v - 1));
            pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (fb->u * fb->bpp) - 1);
        }
    }
}

int omv_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    framebuffer_t *fb = csi->fb;

    // Compress the framebuffer for the IDE preview.
    framebuffer_update_jpeg_buffer(fb);

    if (omv_csi_check_framebuffer_size() != 0) {
        return OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    // Free the current FB head.
    framebuffer_free_current_buffer(fb);

    // Set framebuffer pixel format.
    if (csi->pixformat == PIXFORMAT_INVALID) {
        return OMV_CSI_ERROR_INVALID_PIXFORMAT;
    }
    fb->pixfmt = csi->pixformat;

    vbuffer_t *buffer = framebuffer_get_head(fb, FB_NO_FLAGS);

    // If there's no ready buffer in the fifo, and the DMA is Not currently
    // transferring a new buffer, reconfigure and restart the DMA transfer.
    if (buffer == NULL && !dma_channel_is_busy(OMV_CSI_DMA_CHANNEL)) {
        framebuffer_setup_buffers(fb);

        buffer = framebuffer_get_tail(fb, FB_PEEK);
        if (buffer == NULL) {
            return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
        }

        // Configure the DMA on the first frame, for later frames only the write is changed.
        omv_csi_dma_config(fb->u, fb->v, fb->bpp,
                           (void *) buffer->data,
                           (csi->rgb_swap && fb->bpp == 2));

        // Re-enable the state machine.
        pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);
        pio_sm_set_enabled(OMV_CSI_PIO, OMV_CSI_SM, true);

        // Unblock the state machine
        pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (fb->v - 1));
        pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (fb->u * fb->bpp) - 1);
    }

    // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        buffer = framebuffer_get_head(fb, FB_NO_FLAGS);
        if ((mp_hal_ticks_ms() - ticks) > 3000) {
            omv_csi_abort(csi, true, false);
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
    }

    fb->w = fb->u;
    fb->h = fb->v;

    // Set the user image.
    framebuffer_init_image(fb, image);
    return 0;
}
#endif

int omv_csi_init() {
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

    // Reset the csi state
    memset(&csi, 0, sizeof(omv_csi_t));

    // Set default framebuffer
    csi.fb = framebuffer_get(0);

    // Set I2C bus
    csi.i2c = &csi_i2c;

    // Set default snapshot function.
    csi.snapshot = omv_csi_snapshot;

    // Configure the csi external clock (XCLK).
    if (omv_csi_set_clk_frequency(OMV_CSI_CLK_FREQUENCY) != 0) {
        // Failed to initialize the csi clock.
        return OMV_CSI_ERROR_TIM_INIT_FAILED;
    }

    // Detect and initialize the image sensor.
    if ((init_ret = omv_csi_probe_init(OMV_CSI_I2C_ID, OMV_CSI_I2C_SPEED)) != 0) {
        // Sensor probe/init failed.
        return init_ret;
    }

    // Set default color palette.
    csi.color_palette = rainbow_table;

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
    omv_csi_set_vsync_callback(NULL);

    // Disable Frame callback.
    omv_csi_set_frame_callback(NULL);

    /* All good! */
    csi.detected = true;

    return 0;
}
