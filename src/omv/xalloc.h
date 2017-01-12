/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Memory allocation functions.
 *
 */
#ifndef __XALLOC_H__
#define __XALLOC_H__
#include <stdint.h>
void *xalloc(uint32_t size);
void *xalloc_try_alloc(uint32_t size);
void *xalloc0(uint32_t size);
void xfree(void *mem);
void *xrealloc(void *mem, uint32_t size);
#endif // __XALLOC_H__
