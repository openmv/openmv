/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
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
#include "common.h"

typedef struct framebuffer {
    int32_t x,y;
    int32_t w,h;
    int32_t u,v;
    int32_t bpp;
    int32_t streaming_enabled;
    // NOTE: This buffer must be aligned on a 32 byte boundary
    OMV_ATTR_ALIGNED(uint8_t pixels[], 32);
} framebuffer_t;

extern framebuffer_t *framebuffer;

typedef struct jpegbuffer {
    int32_t w,h;
    int32_t size;
    int32_t enabled;
    int32_t quality;
    mutex_t lock;
    // NOTE: This buffer must be aligned on a 32 byte boundary
    OMV_ATTR_ALIGNED(uint8_t pixels[], 32);
} jpegbuffer_t;

extern jpegbuffer_t *jpeg_framebuffer;

// Force fb streaming to the IDE off.
void fb_set_streaming_enabled(bool enable);
bool fb_get_streaming_enabled();

// Encode jpeg data for transmission over a text channel.
int fb_encode_for_ide_new_size(image_t *img);
void fb_encode_for_ide(uint8_t *ptr, image_t *img);

void framebuffer_init0();

// Initializes an image_t struct with the frame buffer.
void framebuffer_initialize_image(image_t *img);

// Transfers the frame buffer to the jpeg frame buffer if not locked.
void fb_update_jpeg_buffer();

int32_t framebuffer_get_x();
int32_t framebuffer_get_y();
int32_t framebuffer_get_u();
int32_t framebuffer_get_v();

int32_t framebuffer_get_width();
int32_t framebuffer_get_height();
int32_t framebuffer_get_depth();

// Return the size of the current frame (w * h * bpp) if the framebuffer is initialized,
// otherwise return 0 if the framebuffer is unintialized or invalid (e.g. first frame).
uint32_t framebuffer_get_frame_size();

// Return the max frame size that fits the framebuffer
// (i.e OMV_RAW_BUF_SIZE - sizeof(framebuffer_t))
uint32_t framebuffer_get_buffer_size();

// Return the current buffer address.
uint8_t *framebuffer_get_buffer();

// Set the framebuffer w, h and bpp.
void framebuffer_set(int32_t w, int32_t h, int32_t bpp);

// Use these macros to get a pointer to main or JPEG framebuffer.
#define MAIN_FB()           (framebuffer)
#define JPEG_FB()           (jpeg_framebuffer)

#endif /* __FRAMEBUFFER_H__ */
