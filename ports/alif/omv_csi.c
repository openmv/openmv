/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alif CSI driver.
 */
#if MICROPY_PY_CSI
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "py/mphal.h"
#include "py/runtime.h"

#include "alif_hal.h"
#include "cpi.h"
#include "sys_ctrl_cpi.h"
#include "system_utils.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_gpu.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "unaligned_memcpy.h"

// Bits missing from cpi.h
#define CAM_CFG_INTERFACE_Pos       (0U)
#define CAM_CFG_CSI_HALT_EN_Pos     (1U)
#define CAM_CFG_RW_ROUNDUP_Pos      (8U)
#define CAM_CFG_PXCLK_POL_Pos       (12U)
#define CAM_CFG_ENDIANNESS_Pos      (20U)

#define CPI_VSYNC_MODE_DISABLE      (0)
#define CPI_VSYNC_MODE_ENABLE       (1)

#define CPI_VSYNC_WAIT_DISABLE      (0)
#define CPI_VSYNC_WAIT_ENABLE       (1)

#define CPI_IRQ_FLAGS   (CAM_INTR_STOP | CAM_INTR_VSYNC | CAM_INTR_HSYNC |    \
                         CAM_INTR_INFIFO_OVERRUN | CAM_INTR_OUTFIFO_OVERRUN | \
                         CAM_INTR_BRESP_ERR)

#define CPI_ERROR_FLAGS (CAM_INTR_INFIFO_OVERRUN |  \
                         CAM_INTR_OUTFIFO_OVERRUN | \
                         CAM_INTR_BRESP_ERR)

static uint32_t omv_csi_get_fb_offset(omv_csi_t *csi);

void CAM_IRQHandler(void) {
    uint32_t mask = 0;
    omv_csi_t *csi = omv_csi_get(-1);
    CPI_Type *cpi = csi->base;

    uint32_t status = cpi_get_interrupt_status(cpi);

    if (status & CAM_INTR_VSYNC) {
        mask |= CAM_INTR_VSYNC;
    }

    if (status & CAM_INTR_HSYNC) {
        mask |= CAM_INTR_HSYNC;
    }

    if (status & CAM_INTR_INFIFO_OVERRUN) {
        mask |= CAM_INTR_INFIFO_OVERRUN;
        omv_csi_abort(csi, true, true);
    }

    if (status & CAM_INTR_OUTFIFO_OVERRUN) {
        mask |= CAM_INTR_OUTFIFO_OVERRUN;
        omv_csi_abort(csi, true, true);
    }

    if (status & CAM_INTR_BRESP_ERR) {
        mask |= CAM_INTR_BRESP_ERR;
        omv_csi_abort(csi, true, true);
    }

    if (status & CAM_INTR_STOP) {
        mask |= CAM_INTR_STOP;
        cpi->CAM_CTRL = 0;

        if (!(status & CPI_ERROR_FLAGS)) {
            // Release the buffer from free queue -> used queue.
            framebuffer_release(csi->fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
        }

        // Acquire a buffer from the free queue.
        vbuffer_t *buffer = framebuffer_acquire(csi->fb, FB_FLAG_FREE | FB_FLAG_PEEK);

        if (buffer != NULL) {
            cpi->CAM_CTRL = 0;
            cpi->CAM_CTRL = CAM_CTRL_SW_RESET;
            cpi->CAM_FRAME_ADDR = LocalToGlobal(buffer->data + omv_csi_get_fb_offset(csi));
            cpi_irq_handler_clear_intr_status(cpi, mask);
            cpi->CAM_CTRL = (CAM_CTRL_SNAPSHOT | CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
        }

        if (csi->frame_cb.fun && !(status & CPI_ERROR_FLAGS)) {
            csi->frame_cb.fun(csi->frame_cb.arg);
        }
    }

    // Clear interrupts.
    cpi_irq_handler_clear_intr_status(cpi, mask);
}

static bool alif_csi_is_active(omv_csi_t *csi) {
    CPI_Type *cpi = csi->base;

    return (cpi->CAM_CTRL & CAM_CTRL_BUSY);
}

int alif_csi_isp_reset(omv_csi_t *csi) {
    #ifdef IMLIB_ENABLE_GAMMA_LUT
    imlib_update_gamma_table(-0.2f, 1.0f, 2.2f);
    #endif
    return 0;
}

int alif_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        CPI_Type *cpi = csi->base;

        // Configure the FIFO.
        cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_RD_WMARK_Msk;
        cpi->CAM_FIFO_CTRL = 0x08;

        cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_WR_WMARK_Msk;
        cpi->CAM_FIFO_CTRL |= (0x18 << CAM_FIFO_CTRL_WR_WMARK_Pos);

        cpi->CAM_CFG = 0;
        // Configure the capture interface (CPI, LPCPI or CSI).
        cpi->CAM_CFG |= (CPI_INTERFACE_PARALLEL << CAM_CFG_INTERFACE_Pos);
        cpi->CAM_CFG |= (CPI_VSYNC_MODE_DISABLE << CAM_CFG_VSYNC_MODE_Pos);
        cpi->CAM_CFG |= (CPI_VSYNC_WAIT_DISABLE << CAM_CFG_VSYNC_WAIT_Pos);

        // Set VSYNC, HSYNC and PIXCLK polarities.
        cpi->CAM_CFG |= (csi->vsync_pol << CAM_CFG_VSYNC_POL_Pos);
        cpi->CAM_CFG |= (csi->hsync_pol << CAM_CFG_HSYNC_POL_Pos);
        cpi->CAM_CFG |= (!csi->pixck_pol << CAM_CFG_PXCLK_POL_Pos);

        // Configure the data bus width, mode, endianness.
        cpi->CAM_CFG |= (CPI_ROW_ROUNDUP_DISABLE << CAM_CFG_RW_ROUNDUP_Pos);
        cpi->CAM_CFG |= (CPI_DATA_MODE_BIT_8 << CAM_CFG_DATA_MODE_Pos);
        cpi->CAM_CFG |= (CPI_CODE10ON8_CODING_DISABLE << CAM_CFG_CODE10ON8_Pos);
        cpi->CAM_CFG |= (CPI_DATA_ENDIANNESS_LSB_FIRST << CAM_CFG_ENDIANNESS_Pos);

        // Configure the data mask (for 16-bits mode only).
        cpi->CAM_CFG &= ~CAM_CFG_DATA_MASK_Msk;
        cpi->CAM_CFG |= (0 << CAM_CFG_DATA_MASK_Pos);

        // Configure IPI color mode (for CSI mode only).
        cpi->CAM_CSI_CMCFG = CPI_COLOR_MODE_CONFIG_IPI48_RGB565;
    }
    return 0;
}

int alif_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    CPI_Type *cpi = csi->base;

    // Stop CPI
    cpi->CAM_CTRL = 0;

    // Disable IRQs.
    NVIC_DisableIRQ(CAM_IRQ_IRQn);
    cpi_disable_interrupt(cpi, CPI_IRQ_FLAGS);
    cpi_irq_handler_clear_intr_status(cpi, CPI_IRQ_FLAGS);
    return 0;
}

static uint32_t alif_clk_get_frequency(omv_clk_t *clk) {
    uint32_t div = (CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL & CAMERA_PIXCLK_CTRL_DIVISOR_Msk) >>
                   CAMERA_PIXCLK_CTRL_DIVISOR_Pos;
    if (CLKCTL_PER_MST->CAMERA_PIXCLK_CTRL & CAMERA_PIXCLK_CTRL_CLK_SEL) {
        return 480000000 / div;
    } else {
        return 400000000 / div;
    }
}

static int alif_clk_set_frequency(omv_clk_t *clk, uint32_t frequency) {
    // Configure CPI clock source (400MHz or 480MHz) and divider.
    if (frequency >= 24000000) {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_480MZ, 20);
    } else if (frequency >= 12000000) {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_480MZ, 40);
    } else if (frequency >= 6000000) {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_480MZ, 80);
    } else {
        set_cpi_pixel_clk(CPI_PIX_CLKSEL_400MZ, 100);
    }
    return 0;
}

static uint32_t omv_csi_get_fb_offset(omv_csi_t *csi) {
    uint32_t offset = 0;
    uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
    uint32_t line_size_bytes = csi->resolution[csi->framesize][0] * bytes_per_pixel;

    // Offset the pixels buffer for debayering.
    if (csi->raw_output && csi->pixformat == PIXFORMAT_RGB565) {
        offset += line_size_bytes * csi->resolution[csi->framesize][1];
    }

    return offset;
}

// This is the default snapshot function, which can be replaced in omv_csi_init functions.
int alif_csi_snapshot(omv_csi_t *csi, image_t *dst_image, uint32_t flags) {
    CPI_Type *cpi = csi->base;
    framebuffer_t *fb = csi->fb;
    vbuffer_t *buffer = NULL;

    // Configure and re/start the capture if it's not alrady active
    // and there are no pending buffers (from non-blocking capture).
    if (!alif_csi_is_active(csi) && !framebuffer_readable(fb)) {
        uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
        uint32_t line_size_bytes = csi->resolution[csi->framesize][0] * bytes_per_pixel;

        // Acquire a buffer from the free queue.
        buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);

        // Check if buffer is not ready or is not 64-bit aligned.
        if ((!buffer) || (LocalToGlobal(buffer->data) & 0x7)) {
            return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
        }

        // Ensure that the transfer size is compatible with DMA restrictions.
        if ((!line_size_bytes) ||
            (line_size_bytes % sizeof(uint64_t)) ||
            csi->transpose ||
            (csi->pixformat == PIXFORMAT_JPEG)) {
            return OMV_CSI_ERROR_INVALID_FRAMESIZE;
        }

        if (!csi->raw_output &&
            ((csi->pixformat == PIXFORMAT_RGB565 && csi->rgb_swap) ||
             (csi->pixformat == PIXFORMAT_YUV422 && csi->yuv_swap))) {
            cpi->CAM_CFG |= (CPI_DATA_ENDIANNESS_MSB_FIRST << CAM_CFG_ENDIANNESS_Pos);
        } else {
            cpi->CAM_CFG &= ~(CPI_DATA_ENDIANNESS_MSB_FIRST << CAM_CFG_ENDIANNESS_Pos);
        }

        // Find maximum burst size that perfectly fits the line size.
        cpi->CAM_FIFO_CTRL &= ~CAM_FIFO_CTRL_RD_WMARK_Msk;

        for (uint32_t i = 16; i >= 4; i--) {
            if (!(line_size_bytes % (i * 8))) {
                cpi->CAM_FIFO_CTRL |= (i << CAM_FIFO_CTRL_RD_WMARK_Pos);
                break;
            }
        }

        cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_DATA_Msk;
        cpi->CAM_VIDEO_FCFG = line_size_bytes;
        cpi->CAM_VIDEO_FCFG &= ~CAM_VIDEO_FCFG_ROW_Msk;
        cpi->CAM_VIDEO_FCFG |= ((csi->resolution[csi->framesize][1] - 1) << CAM_VIDEO_FCFG_ROW_Pos);
        cpi->CAM_FRAME_ADDR = LocalToGlobal(buffer->data + omv_csi_get_fb_offset(csi));

        // Configure and enable CSI interrupts.
        cpi_irq_handler_clear_intr_status(cpi, CPI_IRQ_FLAGS);
        cpi_enable_interrupt(cpi, CPI_IRQ_FLAGS);
        NVIC_ClearPendingIRQ(CAM_IRQ_IRQn);
        NVIC_SetPriority(CAM_IRQ_IRQn, IRQ_PRI_CSI);
        NVIC_EnableIRQ(CAM_IRQ_IRQn);

        // Reset CSI and start the capture.
        cpi->CAM_CTRL = 0;
        cpi->CAM_CTRL = CAM_CTRL_SW_RESET;
        cpi->CAM_CTRL = (CAM_CTRL_SNAPSHOT | CAM_CTRL_START | CAM_CTRL_FIFO_CLK_SEL);
    }

    // Let the camera know we want to trigger it now.
    #if defined(OMV_CSI_FSYNC_PIN)
    if (csi->frame_sync) {
        omv_gpio_write(OMV_CSI_FSYNC_PIN, 1);
    }
    #endif

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
    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;

    // Set the framebuffer pixel format.
    switch (csi->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            fb->pixfmt = PIXFORMAT_GRAYSCALE;
            break;
        case PIXFORMAT_RGB565:
            fb->pixfmt = PIXFORMAT_RGB565;
            break;
        case PIXFORMAT_YUV422: {
            fb->pixfmt = PIXFORMAT_YUV;
            fb->subfmt_id = csi->yuv_format;
            fb->pixfmt = imlib_yuv_shift(fb->pixfmt, fb->x);
            break;
            case PIXFORMAT_BAYER:
                fb->pixfmt = PIXFORMAT_BAYER;
                fb->subfmt_id = csi->cfa_format;
                fb->pixfmt = imlib_bayer_shift(fb->pixfmt, fb->x, fb->y, csi->transpose);
                break;
        }
        default:
            break;
    }

    // Initialize a frame using the frame buffer.
    framebuffer_to_image(fb, dst_image);

    // Set the frame's pixel format to bayer for raw sensors.
    if (csi->raw_output && csi->pixformat != PIXFORMAT_BAYER) {
        dst_image->pixfmt = PIXFORMAT_BAYER;
        dst_image->subfmt_id = csi->cfa_format;
        dst_image->pixfmt = imlib_bayer_shift(dst_image->pixfmt, fb->x, fb->y, csi->transpose);
    }

    // Crop first to reduce the frame size before debayering.
    if (omv_csi_get_cropped(csi)) {
        image_t src_cimage = *dst_image;
        image_t dst_cimage = *dst_image;

        src_cimage.w = csi->resolution[csi->framesize][0];
        src_cimage.h = csi->resolution[csi->framesize][1];

        // Offset the pixels buffer for the debayer code.
        if (csi->pixformat == PIXFORMAT_RGB565) {
            src_cimage.pixels += omv_csi_get_fb_offset(csi);
            dst_cimage.pixels += omv_csi_get_fb_offset(csi);
        }

        rectangle_t srect = { fb->x, fb->y, fb->u, fb->v };
        rectangle_t drect = { 0, 0, fb->u, fb->v };
        if (omv_gpu_draw_image(&src_cimage, &srect, &dst_cimage, &drect, 255, NULL, NULL, 0, NULL) != 0) {
            return OMV_CSI_ERROR_IO_ERROR;
        }
    }

    // Debayer the frame to match the target pixel format.
    if (csi->raw_output && csi->pixformat != PIXFORMAT_BAYER) {
        image_t src_image = *dst_image;

        // Offset the pixels buffer for the debayer code.
        if (csi->pixformat == PIXFORMAT_RGB565) {
            src_image.pixels += omv_csi_get_fb_offset(csi);
        }

        // Set the target pixel format before debayer.
        dst_image->pixfmt = fb->pixfmt;

        uint32_t r, g, b;
        if (csi->stats_enabled) {
            uint32_t gb, gr;
            int ret = omv_csi_ioctl(csi, OMV_CSI_IOCTL_GET_RGB_STATS, &r, &gb, &gr, &b);
            if (ret != 0) {
                return ret;
            }

            g = (gb + gr) / 2;
            omv_csi_stats_update(csi, &r, &g, &b, mp_hal_ticks_ms());
        }

        omv_csi_get_stats(csi, &r, &g, &b);

        // Debayer frame.
        imlib_debayer_image_awb(dst_image, &src_image, false, r, g, b);
    }
    return 0;
}

int omv_csi_ops_init(omv_csi_t *csi) {
    // Set CPI base (LP/CPI).
    csi->base = OMV_CSI_BASE;

    // Set CSI ops.
    csi->abort = alif_csi_abort;
    csi->config = alif_csi_config;
    csi->snapshot = alif_csi_snapshot;
    csi->isp_reset = alif_csi_isp_reset;

    // Set CSI clock ops.
    csi->clk->freq = OMV_CSI_CLK_FREQUENCY;
    csi->clk->set_freq = alif_clk_set_frequency;
    csi->clk->get_freq = alif_clk_get_frequency;
    return 0;
}
#endif // MICROPY_PY_CSI
