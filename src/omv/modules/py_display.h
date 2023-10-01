/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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
    mp_obj_t controller;
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
    #ifdef OMV_DSI_DISPLAY_CONTROLLER
    // To be implemented by MIPI DSI controllers.
    int (*dsi_write) (py_display_obj_t *self, uint8_t cmd, uint8_t *args, size_t n_args, bool dcs);
    int (*dsi_read) (py_display_obj_t *self, uint8_t cmd, uint8_t *args, size_t n_args, uint8_t *buf, size_t len, bool dcs);
    #endif
} py_display_p_t;

extern const mp_obj_type_t py_spi_display_type;
extern const mp_obj_type_t py_rgb_display_type;
extern const mp_obj_type_t py_dsi_display_type;
extern const mp_obj_type_t py_display_data_type;
extern const mp_obj_dict_t py_display_locals_dict;
#endif // __PY_DISPLAY_H__
