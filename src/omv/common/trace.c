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
 * Trace buffer.
 */
#include <stdint.h>
#include "cmsis_compiler.h"
#include "trace.h"

#define TRACEBUF_SIZE    (256)
typedef struct _tracebuf_t {
    uint8_t idx;
    uint8_t buf[TRACEBUF_SIZE];
} tracebuf_t;

static tracebuf_t tracebuf;

void trace_init() {
    tracebuf.idx = 0;
    for (int i = 0; i < TRACEBUF_SIZE; i++) {
        tracebuf.buf[i] = 0;
    }
}

void trace_insert(uint32_t x) {
    __disable_irq();
    if (tracebuf.idx < TRACEBUF_SIZE) {
        tracebuf.buf[tracebuf.idx++] = x;
    }
    __enable_irq();
}
