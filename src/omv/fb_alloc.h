/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Interface for using extra frame buffer RAM as a stack.
 *
 */
#ifndef __FB_ALLOC_H__
#define __FB_ALLOC_H__
#include <stdint.h>
void fb_alloc_fail();
void fb_alloc_init0();
uint32_t fb_avail();
void fb_alloc_mark();
void fb_alloc_free_till_mark();
void *fb_alloc(uint32_t size);
void *fb_alloc0(uint32_t size);
void *fb_alloc_all(uint32_t *size); // returns pointer and sets size
void *fb_alloc0_all(uint32_t *size); // returns pointer and sets size
void fb_free();
void fb_free_all();
#endif /* __FF_ALLOC_H__ */
