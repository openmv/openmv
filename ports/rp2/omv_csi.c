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

extern void __fatal_error(const char *msg);

static void dma_irq_handler() {
    omv_csi_t *csi = omv_csi_get(-1);
    framebuffer_t *fb = csi->fb;

    if (dma_irqn_get_channel_status(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL)) {
        // Clear the interrupt request.
        dma_irqn_acknowledge_channel(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL);

        // Release the current buffer and acquire a new one.
        framebuffer_release(fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
        vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);

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

static bool rp2_csi_is_active(omv_csi_t *csi) {
    return dma_channel_is_busy(OMV_CSI_DMA_CHANNEL);
}

static int rp2_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        // PIXCLK
        gpio_init(OMV_CSI_PXCLK_PIN);
        gpio_set_dir(OMV_CSI_PXCLK_PIN, GPIO_IN);

        // HSYNC
        gpio_init(OMV_CSI_HSYNC_PIN);
        gpio_set_dir(OMV_CSI_HSYNC_PIN, GPIO_IN);

        // VSYNC
        gpio_init(OMV_CSI_VSYNC_PIN);
        gpio_set_dir(OMV_CSI_VSYNC_PIN, GPIO_IN);

        // Install new DMA IRQ handler.
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
    }
    return 0;
}

static int rp2_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
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

static uint32_t rp2_clk_get_frequency(omv_clk_t *clk) {
    return OMV_CSI_CLK_FREQUENCY;
}

static int rp2_clk_set_frequency(omv_clk_t *clk, uint32_t frequency) {
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

int omv_csi_set_windowing(omv_csi_t *csi, int x, int y, int w, int h) {
    return OMV_CSI_ERROR_CTL_UNSUPPORTED;
}

static int rp2_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    vbuffer_t *buffer = NULL;
    framebuffer_t *fb = csi->fb;

    // If there's no ready buffer in the fifo, and the DMA is Not currently
    // transferring a new buffer, reconfigure and restart the DMA transfer.
    if (!rp2_csi_is_active(csi) && !framebuffer_readable(fb)) {
        // Acquire a buffer from the free queue.
        if (!(buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK))) {
            return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
        }

        // Configure the DMA on the first frame.
        dma_channel_abort(OMV_CSI_DMA_CHANNEL);
        dma_irqn_set_channel_enabled(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL, false);

        dma_channel_config c = dma_channel_get_default_config(OMV_CSI_DMA_CHANNEL);
        channel_config_set_read_increment(&c, false);
        channel_config_set_write_increment(&c, true);
        channel_config_set_dreq(&c, pio_get_dreq(OMV_CSI_PIO, OMV_CSI_SM, false));
        channel_config_set_bswap(&c, csi->rgb_swap && (fb->bpp == 2));

        dma_channel_configure(OMV_CSI_DMA_CHANNEL, &c,
                (uint32_t *) buffer->data, // Destinatinon pointer.
                &OMV_CSI_PIO->rxf[OMV_CSI_SM], // Source pointer.
                (fb->u * fb->v * fb->bpp) >> 2, // Number of transfers in words.
                true      // Start immediately, will block on SM.
                );

        // Re-enable DMA IRQs.
        dma_irqn_set_channel_enabled(OMV_CSI_DMA, OMV_CSI_DMA_CHANNEL, true);

        // Re-enable the state machine.
        pio_sm_clear_fifos(OMV_CSI_PIO, OMV_CSI_SM);
        pio_sm_set_enabled(OMV_CSI_PIO, OMV_CSI_SM, true);

        // Unblock the state machine
        pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (fb->v - 1));
        pio_sm_put_blocking(OMV_CSI_PIO, OMV_CSI_SM, (fb->u * fb->bpp) - 1);
    }

    // One shot DMA transfers must be invalidated.
    framebuffer_flags_t fb_flags = FB_FLAG_USED | FB_FLAG_PEEK | FB_FLAG_INVALIDATE;

    // Wait for a frame to be ready.
    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        if ((buffer = framebuffer_acquire(fb, fb_flags))) {
            break;
        }

        if (flags & OMV_CSI_FLAG_NON_BLOCK) {
            return OMV_CSI_ERROR_WOULD_BLOCK;
        }

        if ((mp_hal_ticks_ms() - start) > OMV_CSI_TIMEOUT_MS) {
            omv_csi_abort(csi, true, false);
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
    }

    // Set the framebuffer width/height.
    fb->w = fb->u;
    fb->h = fb->v;
    // Set the framebuffer pixel format.
    fb->pixfmt = csi->pixformat;

    // Set the user image.
    framebuffer_init_image(fb, image);
    return 0;
}

int omv_csi_ops_init(omv_csi_t *csi) {
    // Set CSI ops.
    csi->abort = rp2_csi_abort;
    csi->config = rp2_csi_config;
    csi->snapshot = rp2_csi_snapshot;

    // Set CSI clock ops.
    csi->clk->freq = OMV_CSI_CLK_FREQUENCY;
    csi->clk->set_freq = rp2_clk_set_frequency;
    csi->clk->get_freq = rp2_clk_get_frequency;
    return 0;
}
#endif // MICROPY_PY_CSI
