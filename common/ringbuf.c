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
#include <string.h>
#include "ringbuf.h"

void ring_buf_init(ring_buf_t *buf) {
    memset(buf, 0, sizeof(*buf));
}

int ring_buf_empty(ring_buf_t *buf) {
    return (buf->head == buf->tail);
}

void ring_buf_put(ring_buf_t *buf, uint8_t c) {
    if ((buf->tail + 1) % BUFFER_SIZE == buf->head) {
        /*buffer is full*/
        return;
    }

    buf->data[buf->tail] = c;
    buf->tail = (buf->tail + 1) % BUFFER_SIZE;
}

uint8_t ring_buf_get(ring_buf_t *buf) {
    uint8_t c;
    if (buf->head == buf->tail) {
        /*buffer is empty*/
        return 0;
    }

    c = buf->data[buf->head];
    buf->head = (buf->head + 1) % BUFFER_SIZE;
    return c;
}
