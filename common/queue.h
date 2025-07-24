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
 * Single-producer, single-consumer, lock-free bounded Queue.
 */
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stddef.h>
#include <stdbool.h>

#if __STDC_VERSION__ < 201112L
typedef size_t queue_index_t;
#warning "Atomics not supported"
#else
#include <stdatomic.h>
#define HAVE_STDATOMIC_H
typedef atomic_size_t queue_index_t;
#endif

typedef struct {
    size_t capacity;
    queue_index_t head;
    queue_index_t tail; // TODO pad with cache line if needed.
    void *items[];
} queue_t;

// One extra slot is used to distinguish full from empty.
#define queue_calc_size(capacity) \
    (sizeof(queue_t) + ((capacity) + 1) * sizeof(void *))

void queue_init(queue_t **q, size_t capacity, void *buffer);
void queue_flush(queue_t *q);
queue_t *queue_alloc(size_t capacity);
void queue_destroy(queue_t *q);
bool queue_is_empty(const queue_t *q);
bool queue_push(queue_t *q, void *item);
void *queue_pop(queue_t *q, bool peek);
size_t queue_size(const queue_t *q);
void *queue_swap(queue_t *q0, queue_t *q1);
#endif // __QUEUE_H__
