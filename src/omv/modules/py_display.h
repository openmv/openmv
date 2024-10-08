/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * Display Python module.
 */
#ifndef __PY_DISPLAY_H__
#define __PY_DISPLAY_H__

#include "omv_gpio.h"
#include "omv_spi.h"
#include "py_image.h"

#define FRAMEBUFFER_COUNT    3

typedef enum {
    DISPLAY_RESOLUTION_QVGA,
    DISPLAY_RESOLUTION_TQVGA,
    DISPLAY_RESOLUTION_FHVGA,
    DISPLAY_RESOLUTION_FHVGA2,
    DISPLAY_RESOLUTION_VGA,
    DISPLAY_RESOLUTION_THVGA,
    DISPLAY_RESOLUTION_FWVGA,
    DISPLAY_RESOLUTION_FWVGA2,
    DISPLAY_RESOLUTION_TFWVGA,
    DISPLAY_RESOLUTION_TFWVGA2,
    DISPLAY_RESOLUTION_SVGA,
    DISPLAY_RESOLUTION_WSVGA,
    DISPLAY_RESOLUTION_XGA,
    DISPLAY_RESOLUTION_SXGA,
    DISPLAY_RESOLUTION_SXGA2,
    DISPLAY_RESOLUTION_UXGA,
    DISPLAY_RESOLUTION_HD,
    DISPLAY_RESOLUTION_FHD,
    DISPLAY_RESOLUTION_MAX
} display_resolution_t;

typedef struct _py_display_obj_t {
    mp_obj_base_t base;
    uint32_t vcid;
    uint32_t width;
    uint32_t height;
    uint32_t framesize;
    uint32_t refresh;
    uint32_t intensity;
    bool bgr;
    bool byte_swap;
    bool display_on;
    bool portrait;
    mp_obj_t controller;
    mp_obj_t bl_controller;
    #if defined(OMV_SPI_DISPLAY_CONTROLLER)
    omv_spi_t spi_bus;
    bool spi_tx_running;
    uint32_t spi_baudrate;
    #endif
    bool triple_buffer;
    uint32_t framebuffer_tail;
    volatile uint32_t framebuffer_head;
    uint16_t *framebuffers[FRAMEBUFFER_COUNT];
} py_display_obj_t;

// Display protocol
typedef struct _py_display_p_t {
    void (*deinit) (py_display_obj_t *self);
    void (*clear) (py_display_obj_t *self, bool display_off);
    void (*write) (py_display_obj_t *self, image_t *src_img, int dst_x_start, int dst_y_start,
                   float x_scale, float y_scale, rectangle_t *roi, int rgb_channel, int alpha,
                   const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint);
    void (*set_backlight) (py_display_obj_t *self, uint32_t intensity);
    int (*bus_write) (py_display_obj_t *self, uint8_t cmd, uint8_t *args, size_t n_args, bool dcs);
    int (*bus_read) (py_display_obj_t *self, uint8_t cmd, uint8_t *args, size_t n_args, uint8_t *buf, size_t len, bool dcs);
} py_display_p_t;

extern const mp_obj_type_t py_spi_display_type;
extern const mp_obj_type_t py_rgb_display_type;
extern const mp_obj_type_t py_dsi_display_type;
extern const mp_obj_type_t py_display_data_type;
extern const mp_obj_dict_t py_display_locals_dict;
#endif // __PY_DISPLAY_H__
