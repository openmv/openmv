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
#include "mutex.h"
#include "sensor.h"

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

// Use these macros to get the start/end addresses of the main or JPEG framebuffer.
#define MAIN_FB_START()     (MAIN_FB()->pixels)
#define JPEG_FB_START()     (JPEG_FB()->pixels)
#define MAIN_FB_END()       (MAIN_FB()->pixels + fb_framebuffer_total_size())
#define JPEG_FB_END()       (JPEG_FB()->pixels + JPEG_FB()->size)

// Force fb streaming to the IDE off.
void fb_set_streaming_enabled(bool enable);
bool fb_get_streaming_enabled();

// Encode jpeg data for transmission over a text channel.
int fb_encode_for_ide_new_size(image_t *img);
void fb_encode_for_ide(uint8_t *ptr, image_t *img);

// Initializes an image_t struct with the frame buffer.
void fb_initialize_image(image_t *img);

// Transfers the frame buffer to the jpeg frame buffer if not locked.
void fb_update_jpeg_buffer();

#endif /* __FRAMEBUFFER_H__ */
