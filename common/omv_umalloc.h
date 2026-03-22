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
#ifndef __OMV_UMALLOC_H__
#define __OMV_UMALLOC_H__
#include <stdint.h>
#include <stdlib.h>

#define UMA_FAST      (1 << 0)
#define UMA_SLOW      (1 << 1)
#define UMA_DMA       (1 << 2)

void uma_init(void);
void uma_fail(void);
void uma_add_pool(void *mem, size_t size, uint32_t flags);
void *uma_malloc(size_t size, uint32_t flags);
void *uma_malign(size_t size, size_t align, uint32_t flags);
void *uma_calloc(size_t num, size_t size, uint32_t flags);
void *uma_realloc(void *ptr, size_t size);
void  uma_free(void *ptr);
size_t uma_avail(uint32_t flags);
void uma_stats(void);

#endif /* __OMV_UMALLOC_H__ */
