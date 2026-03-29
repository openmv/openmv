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
#ifndef __UMALLOC_H__
#define __UMALLOC_H__

#if !defined(LINKER_SCRIPT)
#include <stdint.h>
#include <stdlib.h>
#endif

// Memory attributes (bits 0-7)
#define UMA_FAST            (1 << 0)
#define UMA_ITCM            ((1 << 1) | UMA_FAST)
#define UMA_DTCM            ((1 << 2) | UMA_FAST)
#define UMA_MEM_ATTR_MASK   (0xFF)

// Allocation flags (bits 8+)
#define UMA_MAYBE           (1 << 8)
#define UMA_STRICT          (1 << 9)
#define UMA_CACHE           (1 << 10)
#define UMA_PERSIST         (1 << 11)

#if !defined(LINKER_SCRIPT)
#ifndef UMA_MAX_POOLS
#define UMA_MAX_POOLS       4
#endif

typedef struct {
    void *tlsf;
    size_t size;
    size_t free;
    size_t peak;
    uintptr_t base;
    uintptr_t end;
    uint32_t flags;
} uma_pool_t;

typedef struct {
    size_t free_count;
    size_t free_bytes;
    size_t used_count;
    size_t used_bytes;
    size_t persist_count;
    size_t persist_bytes;
} uma_stats_t;

void uma_init0(void);
void uma_init(void);
void uma_fail(void);
void uma_add_pool(void *mem, size_t size, uint32_t flags);
void *uma_malloc(size_t size, uint32_t flags);
void *uma_malign(size_t size, size_t align, uint32_t flags);
void *uma_calloc(size_t size, uint32_t flags);
void *uma_realloc(void *ptr, size_t size, uint32_t flags);
void  uma_free(void *ptr);
void  uma_collect(void);
size_t uma_avail(uint32_t flags);
int uma_pool_count(void);
uma_pool_t *uma_pool_get(int index);
void uma_get_stats(uma_stats_t *stats);

#endif // !LINKER_SCRIPT
#endif /* __UMALLOC_H__ */
