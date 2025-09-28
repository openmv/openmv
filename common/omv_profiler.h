/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * OpenMV code profiler.
 */
#ifndef __OMV_PROFILER_H__
#define __OMV_PROFILER_H__

#if OMV_PROFILER_ENABLE
#include <stdio.h>
#include "py/mphal.h"
#if __PMU_PRESENT
#include "pmu_armv8.h"
#endif
#ifndef __cplusplus
#include "common/mutex.h"
#endif
#include "common/omv_common.h"

#ifndef __PMU_NUM_EVENTCNT
#define __PMU_NUM_EVENTCNT 0
#endif

// Must be a power of 2
#ifndef OMV_PROFILER_HASH_SIZE
#define OMV_PROFILER_HASH_SIZE  256
#endif

#ifndef OMV_PROFILER_STACK_DEPTH
#define OMV_PROFILER_STACK_DEPTH 32
#endif

typedef enum {
    OMV_PROFILER_INCLUSIVE = 0,
    OMV_PROFILER_EXCLUSIVE = 1,
} omv_profiler_mode_t;

// Profile record structure
typedef struct __attribute__((packed)) _prof_data {
    void *func_addr;        // Function address
    void *call_addr;        // Caller address
    uint32_t call_count;    // Number of times called
    uint32_t min_ticks;     // Minimum execution time
    uint32_t max_ticks;     // Maximum execution time
    uint64_t total_ticks;   // Total time in ticks
    uint64_t total_cycles;  // Total CPU cycles
    #if __PMU_PRESENT
    uint64_t total_events[__PMU_NUM_EVENTCNT];
    #endif
    struct _prof_data *next; // Next in hash collision chain
} omv_profiler_data_t;

OMV_ATTR_NO_INSTRUMENT void omv_profiler_init(void);
OMV_ATTR_NO_INSTRUMENT void omv_profiler_reset(void);
OMV_ATTR_NO_INSTRUMENT size_t omv_profiler_get_size(void);
OMV_ATTR_NO_INSTRUMENT const void *omv_profiler_get_data(void);
OMV_ATTR_NO_INSTRUMENT void omv_profiler_set_mode(uint32_t mode);
OMV_ATTR_NO_INSTRUMENT void omv_profiler_set_event(uint32_t num, uint32_t type);
#ifndef __cplusplus
OMV_ATTR_NO_INSTRUMENT mutex_t *omv_profiler_lock(void);
#endif

// Manual instrumentation macros
#define OMV_PROFILER_ENTER(func)                        \
    do {                                                \
        void *func_addr = (void *) (func);              \
        void *call_addr = __builtin_return_address(0);  \
        __cyg_profile_func_enter(func_addr, call_addr); \
    } while (0)

#define OMV_PROFILER_EXIT(func)                        \
    do {                                               \
        void *func_addr = (void *) (func);             \
        void *call_addr = __builtin_return_address(0); \
        __cyg_profile_func_exit(func_addr, call_addr); \
    } while (0)

#else
// Disabled - empty macros
#define OMV_PROFILER_ENTER(func) do {} while (0)
#define OMV_PROFILER_EXIT(func) do {} while (0)
#endif // OMV_PROFILER_ENABLE
#endif // __OMV_PROFILER_H__
