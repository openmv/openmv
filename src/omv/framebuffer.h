/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Framebuffer functions.
 */
#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
#include <stdint.h>
#include "imlib.h"
#include "mutex.h"

typedef struct framebuffer {
    int32_t x,y;
    int32_t w,h;
    int32_t u,v;
    int32_t bpp;
    int32_t streaming_enabled;
    // NOTE: This buffer must be aligned on a 16 byte boundary
    uint8_t pixels[];
} framebuffer_t;

extern framebuffer_t *fb_framebuffer;

typedef struct jpegbuffer {
    int32_t w,h;
    int32_t size;
    int32_t enabled;
    int32_t quality;
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

// Force fb streaming to the IDE off.
void fb_set_streaming_enabled(bool enable);
bool fb_get_streaming_enabled();

// Encode jpeg data for transmission over a text channel.
int fb_encode_for_ide_new_size(image_t *img);
void fb_encode_for_ide(uint8_t *ptr, image_t *img);

// Returns the main frame buffer size, factoring in pixel formats.
uint32_t fb_buffer_size();

// Transfers the frame buffer to the jpeg frame buffer if not locked.
void fb_update_jpeg_buffer();
#endif /* __FRAMEBUFFER_H__ */
