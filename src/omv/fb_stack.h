/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Interface for using extra frame buffer RAM as a stack.
 *
 */
#ifndef __FB_STACK_H__
#define __FB_STACK_H__
#include <stdint.h>
void fb_init();
void *fb_alloc(uint32_t size);
void *fb_alloc0(uint32_t size);
void fb_free();
#endif /* __FF_WRAPPER_H__ */
