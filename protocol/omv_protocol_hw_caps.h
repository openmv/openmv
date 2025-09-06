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
 * OpenMV Protocol Hardware Capabilities Definitions
 * This header provides macros for boards to easily define their hardware
 * capabilities without manual bit manipulation.
 */
#ifndef __OMV_PROTOCOL_HW_CAPS_H__
#define __OMV_PROTOCOL_HW_CAPS_H__

#include <stdint.h>
#include "cmsis_gcc.h"

#ifndef __PMU_NUM_EVENTCNT
#define __PMU_NUM_EVENTCNT 0
#endif

/***************************************************************************
* Hardware Capability Bit Definitions
***************************************************************************/

#define OMV_PROTOCOL_HW_CAPS_HAS_GPU         (1U << 0)   // Graphics Processing Unit
#define OMV_PROTOCOL_HW_CAPS_HAS_NPU         (1U << 1)   // Neural Processing Unit
#define OMV_PROTOCOL_HW_CAPS_HAS_ISP         (1U << 2)   // Image Signal Processor
#define OMV_PROTOCOL_HW_CAPS_HAS_VENC        (1U << 3)   // Video encoder present
#define OMV_PROTOCOL_HW_CAPS_HAS_JPEG        (1U << 4)   // JPEG encoder present
#define OMV_PROTOCOL_HW_CAPS_HAS_DRAM        (1U << 5)   // DRAM present
#define OMV_PROTOCOL_HW_CAPS_HAS_CRC         (1U << 6)   // Hardware-accelerated CRC
#define OMV_PROTOCOL_HW_CAPS_HAS_PMU         (1U << 7)   // Performance Monitoring Unit
#define OMV_PROTOCOL_HW_CAPS_PMU_EVENTCNT    ((__PMU_NUM_EVENTCNT & 0xFF) << 8)  // PMU number of event counters
#define OMV_PROTOCOL_HW_CAPS_HAS_WIFI        (1U << 16)  // WiFi module present
#define OMV_PROTOCOL_HW_CAPS_HAS_BT          (1U << 17)  // Bluetooth available
#define OMV_PROTOCOL_HW_CAPS_HAS_SD          (1U << 18)  // SD card slot available
#define OMV_PROTOCOL_HW_CAPS_HAS_ETH         (1U << 19)  // Ethernet interface
#define OMV_PROTOCOL_HW_CAPS_HAS_USB_HS      (1U << 20)  // USB High-Speed capable
#define OMV_PROTOCOL_HW_CAPS_HAS_MULTICORE   (1U << 21)  // Multi-core processor


// Prefix paste
#define OMV_PROTOCOL_HW_CAPS_(x) OMV_PROTOCOL_HW_CAPS_##x

// Expander helpers
#define EXPAND(x) x

// Recursive apply with OR
#define FE_1(f, x) f(x)
#define FE_2(f, x, ...) f(x) | FE_1(f, __VA_ARGS__)
#define FE_3(f, x, ...) f(x) | FE_2(f, __VA_ARGS__)
#define FE_4(f, x, ...) f(x) | FE_3(f, __VA_ARGS__)
#define FE_5(f, x, ...) f(x) | FE_4(f, __VA_ARGS__)
#define FE_6(f, x, ...) f(x) | FE_5(f, __VA_ARGS__)
#define FE_7(f, x, ...) f(x) | FE_6(f, __VA_ARGS__)
#define FE_8(f, x, ...) f(x) | FE_7(f, __VA_ARGS__)
#define FE_9(f, x, ...) f(x) | FE_8(f, __VA_ARGS__)
#define FE_10(f, x, ...) f(x) | FE_9(f, __VA_ARGS__)
#define FE_11(f, x, ...) f(x) | FE_10(f, __VA_ARGS__)
#define FE_12(f, x, ...) f(x) | FE_11(f, __VA_ARGS__)
#define FE_13(f, x, ...) f(x) | FE_12(f, __VA_ARGS__)
#define FE_14(f, x, ...) f(x) | FE_13(f, __VA_ARGS__)
#define FE_15(f, x, ...) f(x) | FE_14(f, __VA_ARGS__)
#define FE_16(f, x, ...) f(x) | FE_15(f, __VA_ARGS__)
#define FE_17(f, x, ...) f(x) | FE_16(f, __VA_ARGS__)
#define FE_18(f, x, ...) f(x) | FE_17(f, __VA_ARGS__)
#define FE_19(f, x, ...) f(x) | FE_18(f, __VA_ARGS__)
#define FE_20(f, x, ...) f(x) | FE_19(f, __VA_ARGS__)
#define FE_21(f, x, ...) f(x) | FE_20(f, __VA_ARGS__)
#define FE_22(f, x, ...) f(x) | FE_21(f, __VA_ARGS__)
#define FE_23(f, x, ...) f(x) | FE_22(f, __VA_ARGS__)
#define FE_24(f, x, ...) f(x) | FE_23(f, __VA_ARGS__)
#define FE_25(f, x, ...) f(x) | FE_24(f, __VA_ARGS__)
#define FE_26(f, x, ...) f(x) | FE_25(f, __VA_ARGS__)
#define FE_27(f, x, ...) f(x) | FE_26(f, __VA_ARGS__)
#define FE_28(f, x, ...) f(x) | FE_27(f, __VA_ARGS__)
#define FE_29(f, x, ...) f(x) | FE_28(f, __VA_ARGS__)
#define FE_30(f, x, ...) f(x) | FE_29(f, __VA_ARGS__)
#define FE_31(f, x, ...) f(x) | FE_30(f, __VA_ARGS__)
#define FE_32(f, x, ...) f(x) | FE_31(f, __VA_ARGS__)

// Dispatcher (up to 32 args)
#define GET_FE_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,          \
                     _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                     _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
                     _31, _32, NAME, ...) NAME

#define FOR_EACH(f, ...)                                                                          \
    EXPAND(GET_FE_MACRO(__VA_ARGS__,                                                              \
                        FE_32, FE_31, FE_30, FE_29, FE_28, FE_27, FE_26, FE_25, FE_24, FE_23,     \
                        FE_22, FE_21, FE_20, FE_19, FE_18, FE_17, FE_16, FE_15, FE_14, FE_13,     \
                        FE_12, FE_11, FE_10, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1 \
                        ) (f, __VA_ARGS__))

// Main macro
#define OMV_PROTOCOL_HW_CAPS_MAKE(...) \
    FOR_EACH(OMV_PROTOCOL_HW_CAPS_, __VA_ARGS__) | OMV_PROTOCOL_HW_CAPS_PMU_EVENTCNT
#endif // __OMV_PROTOCOL_HW_CAPS_H__
