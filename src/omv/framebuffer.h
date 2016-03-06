/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Framebuffer pointer.
 *
 */
#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
extern char _fb_base;
static struct framebuffer {
    int w,h;
    int bpp;
    int ready;
    int request;
    uint8_t pixels[];
// Note all instances of fb point to the same memory address.
}*fb = (struct framebuffer *) &_fb_base;

// The JPEG offset allows JPEG compression of the framebuffer without overwriting the pixels.
// The offset size may need to be adjusted depending on the quality, otherwise JPEG data may
// overwrite image pixels before they are compressed (see framebuffer.h)
#define FB_JPEG_OFFS_SIZE  (1*1024)

// Use this macro to get a pointer to the free SRAM area located after the framebuffer.
// If JPEG is enabled, this macro returns pixels + the JPEG image size (usually stored in bpp).
#define FB_PIXELS() ((fb->bpp > 2)? (fb->pixels+fb->bpp) : (fb->pixels+fb->w*fb->h*fb->bpp+FB_JPEG_OFFS_SIZE))
#endif /* __FRAMEBUFFER_H__ */
