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

extern framebuffer_t *fb;

typedef struct jpegbuffer {
    int w,h;
    int size;
    int enabled;
    int quality;
    mutex_t lock;
    uint8_t pixels[];
} jpegbuffer_t;

extern jpegbuffer_t *jpeg_fb;

// Use these macros to get a pointer to main or JPEG framebuffer.
#define MAIN_FB()       (fb)
#define JPEG_FB()       (jpeg_fb)
// Returns MAIN FB size
#define MAIN_FB_SIZE()  (MAIN_FB()->w*MAIN_FB()->h*MAIN_FB()->bpp)

// Use this macro to get a pointer to the free SRAM area located after the framebuffer.
#define FB_PIXELS() (MAIN_FB()->pixels+MAIN_FB_SIZE())

// Transfers the frame buffer to the jpeg frame buffer if not locked.
void copy_fb_to_jpeg_fb();

#endif /* __FRAMEBUFFER_H__ */
