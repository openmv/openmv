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

typedef enum {
    FB_NOFLAGS =  (0<<0),
    FB_DIRTY   =  (1<<0),
    FB_USED    =  (1<<1),
} framebuffer_flags_t;

#define FB_SET_FLAG(x, f)  ((x)->flags |= (f))
#define FB_GET_FLAG(x, f)  ((x)->flags & (f))
#define FB_CLR_FLAG(x, f)  ((x)->flags &= (~f))

typedef struct framebuffer {
    int32_t x,y;
    int32_t w,h;
    int32_t u,v;
    int32_t bpp;
    uint32_t flags;
    uint8_t *pixels;
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

int32_t framebuffer_get_x();
int32_t framebuffer_get_y();
int32_t framebuffer_get_u();
int32_t framebuffer_get_v();

int32_t framebuffer_get_width();
int32_t framebuffer_get_height();
int32_t framebuffer_get_depth();

// Sets the frame buffer w, h and bpp.
void framebuffer_set(int32_t w, int32_t h, int32_t bpp);

// Returns a pointer to the start of the current frame buffer.
uint8_t *framebuffer_get_buffer();

// Returns a pointer to the end of the last frame buffer.
uint8_t *framebuffer_get_buffer_end();

// Returns the max frame size that could fit in the frame buffer.
uint32_t framebuffer_get_buffer_size();

// Returns the currently used frame buffer.
framebuffer_t *framebuffer_get_framebuffer();

// Initializes an image_t struct from the current frame buffer.
void framebuffer_initialize_image(image_t *img);

// Initializes an image_t struct from the target frame buffer (fb).
void framebuffer_initialize_image_from_fb(image_t *img, framebuffer_t *fb);

// Copy the last used frame buffer to the JPEG buffer (compressing it first if needed), if
// the JPEG buffer is large enough, otherwise encode and stream the frame buffer to the IDE.
void framebuffer_update_jpeg_buffer();

// Swaps the target frame buffer in double buffering mode.
void framebuffer_swap_buffers(uint32_t offset);

// Use these macros to get a pointer to main or JPEG framebuffer.
#define MAIN_FB()           (framebuffer)
#define JPEG_FB()           (jpeg_framebuffer)

#endif /* __FRAMEBUFFER_H__ */
