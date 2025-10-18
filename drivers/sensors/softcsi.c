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

static uint32_t step = 0;

static int reset(omv_csi_t *csi) {
    step = 0;
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

    // This driver can't use handle NULL images.
    if (!image) {
        return 0;
    }

    // Acquire a new free buffer.
    vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);

    if (!buffer) {
        return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
    }

    // Set the framebuffer pixel format.
    fb->pixfmt = csi->pixformat;

    // Set the framebuffer width/height.
    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;

    // The new buffer hasn't been released yet, so the data pointer
    // has to be set manually after calling framebuffer_to_image.
    framebuffer_to_image(fb, image);
    image->pixels = buffer->data;

    uint32_t offset = (step++ / 4);

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

    // Move the buffer from free queue -> used queue.
    framebuffer_release(fb, FB_FLAG_FREE);
    framebuffer_to_image(fb, image);
    return 0;
}

int softcsi_init(omv_csi_t *csi) {
    csi->reset = reset;
    csi->abort = NULL;
    csi->config = NULL;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->snapshot = snapshot;

    csi->auxiliary = 1;
    csi->vsync_pol = 1;
    csi->hsync_pol = 0;
    csi->pixck_pol = 0;
    csi->frame_sync = 0;
    csi->mono_bpp = 1;

    return 0;
}
#endif // (OMV_SOFTCSI_ENABLE == 1)
