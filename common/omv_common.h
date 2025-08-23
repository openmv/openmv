/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * Common macros.
 */
#ifndef __OMV_COMMON_H__
#define __OMV_COMMON_H__

#define OMV_ATTR_ALIGNED(x, a)      x __attribute__((aligned(a)))
#define OMV_ATTR_SECTION(x, s)      x __attribute__((section(s)))
#define OMV_ATTR_ALWAYS_INLINE      inline __attribute__((always_inline))
#define OMV_ATTR_OPTIMIZE(o)        __attribute__((optimize(o)))
#define OMV_ATTR_SEC_ALIGN(x, s, a) x __attribute__((section(s), aligned(a)))
#define OMV_DEBUG_BREAKPOINT()      __asm__ volatile ("BKPT")

#ifndef __DCACHE_PRESENT
#define OMV_CACHE_LINE_SIZE         (32)
#else
#define OMV_CACHE_LINE_SIZE         (__SCB_DCACHE_LINE_SIZE)
#endif

#ifndef OMV_DMA_ALIGNMENT
#define OMV_DMA_ALIGNMENT           (OMV_CACHE_LINE_SIZE)
#endif

#define OMV_ALIGN_TO(x, alignment) \
    ((((uintptr_t)(x)) + (alignment) - 1) & ~((uintptr_t)((alignment) - 1)))

#define OMV_ALIGN_DOWN(x, alignment) \
    ((uintptr_t)(x) & ~((uintptr_t)(alignment) - 1))

#define check_timeout_ms(start_ms, timeout) \
    ((mp_hal_ticks_ms() - start_ms) > timeout)

#define check_timeout_us(start_us, timeout) \
    ((mp_hal_ticks_us() - start_us) > timeout)

#ifdef OMV_DEBUG_PRINTF
#define debug_printf(fmt, ...) \
    do { printf("%s(): " fmt, __func__, ##__VA_ARGS__);} while (0)
#else
#define debug_printf(...)
#endif

#define OMV_MAX(a, b)             ({ \
        __typeof__ (a) _a = (a);     \
        __typeof__ (b) _b = (b);     \
        _a > _b ? _a : _b;           \
    })

#define OMV_MIN(a, b)             ({ \
        __typeof__ (a) _a = (a);     \
        __typeof__ (b) _b = (b);     \
        _a < _b ? _a : _b;           \
    })

#define OMV_ARRAY_SIZE(a)         (sizeof(a) / sizeof(a[0]))

#if OMV_PROFILE_ENABLE
#include <stdio.h>
#include "py/mphal.h"
#define OMV_CONCATENATE_DETAIL(x, y) x##y
#define OMV_CONCATENATE(x, y)   OMV_CONCATENATE_DETAIL(x, y)
#define OMV_PROFILE_START(F)    mp_uint_t OMV_CONCATENATE(_ticks_start_, F) = mp_hal_ticks_us()
#define OMV_PROFILE_PRINT(F)    printf("%s:%s %u us\n", __FUNCTION__, #F, mp_hal_ticks_us() - OMV_CONCATENATE(_ticks_start_, F))
#else
#define OMV_PROFILE_START(F)
#define OMV_PROFILE_PRINT(F)
#endif

// Returns a pointer to the containing structure
// ptr: Pointer to the member within the structure
// type: Type of the containing structure
// member: Name of the member within the structure
#define OMV_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#endif //__OMV_COMMON_H__
