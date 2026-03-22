/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Unified memory allocator.
 */
#ifndef __OMV_ALLOC_H__
#define __OMV_ALLOC_H__
#include <stdint.h>
#include <stdlib.h>

#define OMV_ALLOC_FAST      (1 << 0)
#define OMV_ALLOC_SLOW      (1 << 1)
#define OMV_ALLOC_CACHED    (1 << 2)
#define OMV_ALLOC_DMA       (1 << 3)

void omv_alloc_init(void);
void omv_alloc_add_pool(void *mem, size_t size, uint32_t flags);
void *omv_alloc_malloc(size_t size, uint32_t flags);
void *omv_alloc_memalign(size_t align, size_t size, uint32_t flags);
void *omv_alloc_calloc(size_t num, size_t size, uint32_t flags);
void *omv_alloc_realloc(void *ptr, size_t size);
void  omv_alloc_free(void *ptr);
NORETURN void omv_alloc_fail(void);
void omv_alloc_print_stats(void);

#endif /* __OMV_ALLOC_H__ */
