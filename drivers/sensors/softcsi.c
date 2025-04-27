/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * Virtual image sensor.
 */
#include "omv_boardconfig.h"
#if (OMV_SOFTCSI_ENABLE == 1)

#include <stdio.h>
#include "omv_csi.h"
#include "vospi.h"
#include "py/mphal.h"
#include "omv_common.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "framebuffer.h"

static int reset(omv_csi_t *csi) {
    return 0;
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    switch (pixformat) {
        case PIXFORMAT_RGB565:
        case PIXFORMAT_GRAYSCALE:
            return 0;
        default:
            return -1;
    }
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    return 0;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    return 0;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    framebuffer_t *fb = csi->fb;

    if (!image) {
        return 0;
    }

    framebuffer_update_jpeg_buffer(fb);

    if (fb->n_buffers != 1) {
        framebuffer_set_buffers(fb, 1);
    }

    if (csi->pixformat == PIXFORMAT_INVALID) {
        return OMV_CSI_ERROR_INVALID_PIXFORMAT;
    }

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    if (omv_csi_check_framebuffer_size(csi) == -1) {
        return OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    framebuffer_free_current_buffer(fb);
    vbuffer_t *buffer = framebuffer_get_tail(fb, FB_NO_FLAGS);

    if (!buffer) {
        return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
    }

    if (!csi->transpose) {
        fb->w = fb->u;
        fb->h = fb->v;
    } else {
        fb->w = fb->v;
        fb->h = fb->u;
    }

    fb->pixfmt = csi->pixformat;
    framebuffer_init_image(fb, image);

    static uint32_t step = 0;
    uint32_t offset = (step / 4);

    for (size_t y = 0; y < image->h; y++) {
        for (size_t x = 0; x < image->w; x++) {
            size_t tx = x;
            size_t ty = y;
    
            if (csi->hmirror) {
                tx = image->w - 1 - tx;
            }
    
            if (csi->vflip) {
                ty = image->h - 1 - ty;
            }
    
            size_t pattern_x = csi->transpose ? y : x;
            size_t pattern_y = csi->transpose ? x : y;
    
            switch (csi->pixformat) {
                case PIXFORMAT_GRAYSCALE: {
                    uint8_t value = ((((pattern_x + offset) / 16) + (pattern_y / 16)) % 2) ? 0xFF : 0x00;
                    IMAGE_PUT_GRAYSCALE_PIXEL(image, tx, ty, value);
                    break;
                }
                case PIXFORMAT_RGB565: {
                    uint8_t r = (((pattern_x + offset) % image->w) * 31) / (image->w - 1);
                    uint8_t g = (y * 63) / (image->h - 1);
                    uint8_t b = ((((pattern_x + offset) % image->w) ^ y) * 31) / (image->w + image->h - 2);
                    uint16_t pixel = COLOR_R5_G6_B5_TO_RGB565(r, g, b);
                    IMAGE_PUT_RGB565_PIXEL(image, tx, ty, pixel);
                    break;
                }
                default:
                    break;
            }
        }
    }
    
    step++;
    return 0;
}

int softcsi_init(omv_csi_t *csi) {
    csi->reset = reset;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->snapshot = snapshot;

    csi->vsync_pol = 1;
    csi->hsync_pol = 0;
    csi->pixck_pol = 0;
    csi->frame_sync = 0;
    csi->mono_bpp = 1;

    return 0;
}
#endif // (OMV_SOFTCSI_ENABLE == 1)
