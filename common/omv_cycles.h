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
 * DWT cycle counter utilities.
 */
#ifndef __OMV_CYCLES_H__
#define __OMV_CYCLES_H__
#include <stdint.h>
#if OMV_OS_UNIX
#include "py/mphal.h"
#else
#include CMSIS_MCU_H
#endif

#if OMV_OS_UNIX
// The host counter uses microseconds rather than CPU cycles.
#define OMV_CYCLES_PER_US       (1UL)
#define OMV_CYCLES_PER_MS       (1000UL)
#else
#ifndef OMV_CPU_FREQ_HZ
#define OMV_CPU_FREQ_HZ         SystemCoreClock
#endif

#define OMV_CYCLES_PER_US       (OMV_CPU_FREQ_HZ / 1000000UL)
#define OMV_CYCLES_PER_MS       (OMV_CPU_FREQ_HZ / 1000UL)
#endif

void omv_cycles_init(void);

static inline uint32_t omv_cycles_now(void) {
    #if OMV_OS_UNIX
    return (uint32_t) mp_hal_ticks_us();
    #else
    return DWT->CYCCNT;
    #endif
}

static inline uint32_t omv_cycles_to_us(uint32_t cycles) {
    return cycles / OMV_CYCLES_PER_US;
}

static inline uint32_t omv_cycles_to_ms(uint32_t cycles) {
    return cycles / OMV_CYCLES_PER_MS;
}
#endif // __OMV_CYCLES_H__
