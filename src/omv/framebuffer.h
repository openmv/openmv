/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Global framebuffer pointer.
 *
 */
#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__
#include "mutex.h"
extern char _fb_base;
static struct framebuffer {
    int w,h;
    int bpp;
    int ready;
    mutex_t lock;
    int lock_tried;
    uint8_t pixels[];
// Note all instances of fb point to the same memory address.
}*fb = (struct framebuffer *) &_fb_base;
#endif /* __FRAMEBUFFER_H__ */
