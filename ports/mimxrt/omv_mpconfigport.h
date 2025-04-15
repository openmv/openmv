/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023 OpenMV, LLC.
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
 * MicroPython port config.
 */
#define MICROPY_NLR_RAISE_HOOK                 \
    do {                                       \
        extern void fb_alloc_free_till_mark(); \
        fb_alloc_free_till_mark();             \
    } while (0);

#define MICROPY_ENABLE_VM_ABORT             (1)
#define MICROPY_PY_NETWORK_PPP_LWIP         (0)
#define MICROPY_PY_MACHINE_UART_IRQ         (0)
#define MICROPY_OPT_COMPUTED_GOTO           (1)
#define MICROPY_GC_SPLIT_HEAP               (1)
#define CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE "lib/cyw43-driver/firmware/w4343WA1_7_45_98_102_combined.h"
#define MICROPY_BANNER_NAME_AND_VERSION "OpenMV " OPENMV_GIT_TAG "; MicroPython " MICROPY_GIT_TAG

#include <mpconfigport.h>
