/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Framebuffer stuff.
 *
 */
#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
#include <stdint.h>
#include "mutex.h"

typedef struct framebuffer {
    int w,h;
    int bpp;
    uint8_t pixels[];
} framebuffer_t;

extern framebuffer_t *fb_framebuffer;

typedef struct jpegbuffer {
    int w,h;
    int size;
    int enabled;
    int quality;
    mutex_t lock;
    uint8_t pixels[];
} jpegbuffer_t;

extern jpegbuffer_t *jpeg_fb_framebuffer;

// Use these macros to get a pointer to main or JPEG framebuffer.
#define MAIN_FB()           (fb_framebuffer)
#define JPEG_FB()           (jpeg_fb_framebuffer)

// Use this macro to get a pointer to the free SRAM area located after the framebuffer.
#define MAIN_FB_PIXELS()    (MAIN_FB()->pixels + fb_buffer_size())

// Use this macro to get a pointer to the free SRAM area located after the framebuffer.
#define JPEG_FB_PIXELS()    (JPEG_FB()->pixels + JPEG_FB()->size)

// Returns the main frame buffer size, factoring in pixel formats.
uint32_t fb_buffer_size();

// Transfers the frame buffer to the jpeg frame buffer if not locked.
void fb_update_jpeg_buffer();
#endif /* __FRAMEBUFFER_H__ */
