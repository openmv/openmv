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
 * Minimal CMSIS device header for MPS3 AN547 (Corstone SSE-300).
 */
#ifndef __CMSIS_MCU_MPS3_AN547_H__
#define __CMSIS_MCU_MPS3_AN547_H__

typedef enum IRQn {
    NonMaskableInt_IRQn   = -14,
    HardFault_IRQn        = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn         = -11,
    UsageFault_IRQn       = -10,
    SecureFault_IRQn      =  -9,
    SVCall_IRQn           =  -5,
    DebugMonitor_IRQn     =  -4,
    PendSV_IRQn           =  -2,
    SysTick_IRQn          =  -1,
} IRQn_Type;

#define __CM55_REV              0x0001U
#define __NVIC_PRIO_BITS        3U
#define __Vendor_SysTickConfig  0U
#define __DSP_PRESENT           1U
#define __FPU_PRESENT           1U
#define __MVE_PRESENT           1U
#define __MVE_FP                1U
#define __VTOR_PRESENT          1U
#define __ICACHE_PRESENT        1U
#define __DCACHE_PRESENT        1U
#define __PMU_PRESENT           1U
#define __PMU_NUM_EVENTCNT      8U
#define __SAUREGION_PRESENT     8U

#undef __FPU_USED
#include "core_cm55.h"

#endif // __CMSIS_MCU_MPS3_AN547_H__
