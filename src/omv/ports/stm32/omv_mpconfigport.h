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
 * MicroPython port config.
 */
#include <mpconfigport.h>

#define MICROPY_NLR_RAISE_HOOK                 \
    do {                                       \
        extern void fb_alloc_free_till_mark(); \
        fb_alloc_free_till_mark();             \
    } while (0);

#define MICROPY_ENABLE_VM_ABORT             (1)
#define MICROPY_OPT_COMPUTED_GOTO           (1)
#define MICROPY_GC_SPLIT_HEAP               (1)
#define MICROPY_PY_VFS                      (1)
#define MICROPY_PY_SOCKET_EXTENDED_STATE    (1)
#define MICROPY_BANNER_NAME_AND_VERSION "OpenMV " OPENMV_GIT_TAG "; MicroPython " MICROPY_GIT_TAG
#define MICROPY_BOARD_FATAL_ERROR           __fatal_error
#define MICROPY_HW_DMA_ENABLE_AUTO_TURN_OFF (0)

#define MICROPY_HW_ETH_DMA_ATTRIBUTE __attribute__((aligned(16384), section(".dma_buffer")));

void __fatal_error(const char *);
