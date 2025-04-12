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
 * Simple Ring Buffer implementation.
 */
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
#include <stdint.h>
#define BUFFER_SIZE    (1024)

typedef struct ring_buffer {
    volatile uint32_t head;
    volatile uint32_t tail;
    uint8_t data[BUFFER_SIZE];
} ring_buf_t;

void ring_buf_init(ring_buf_t *buf);
int ring_buf_empty(ring_buf_t *buf);
void ring_buf_put(ring_buf_t *buf, uint8_t c);
uint8_t ring_buf_get(ring_buf_t *buf);
#endif /* __RING_BUFFER_H__ */
