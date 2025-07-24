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
 *
 * ARM processors have weak memory ordering and may reorder memory operations
 * for performance. The compiler can also reorder operations. This queue uses
 * acquire/release semantics to ensure proper synchronization between producer
 * and consumer threads.
 */
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void queue_init(queue_t **q, size_t capacity, void *buffer) {
    if (!q || !buffer) {
        return;
    }
    
    *q = (queue_t *) buffer;
    (*q)->capacity = capacity;
    queue_flush(*q);
}

queue_t *queue_alloc(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }
    
    void *buffer = malloc(queue_calc_size(capacity));
    if (!buffer) {
        return NULL;
    }
    
    queue_t *q;
    queue_init(&q, capacity, buffer);
    return q;
}

void queue_destroy(queue_t *q) {
    free(q);
}

void queue_flush(queue_t *q) {
    if (!q) {
        return;
    }

    #ifndef HAVE_STDATOMIC_H
    q->head = 0;
    q->tail = 0;
    #else
    atomic_store_explicit(&q->head, 0, memory_order_relaxed);
    atomic_store_explicit(&q->tail, 0, memory_order_relaxed);
    #endif
}

bool queue_push(queue_t *q, void *item) {
    if (!q || !item) {
        return false;
    }
    
    #ifndef HAVE_STDATOMIC_H
    size_t new_tail = (q->tail + 1) % (q->capacity + 1);

    if (new_tail == q->head) {
        return false; // Queue full
    }
    q->items[q->tail] = item;
    q->tail = new_tail;
    #else
    size_t old_tail = atomic_load_explicit(&q->tail, memory_order_relaxed);
    size_t new_tail = (old_tail + 1) % (q->capacity + 1);

    // Ensure that the previous head is consumed.
    if (new_tail == atomic_load_explicit(&q->head, memory_order_acquire)) {
        return false; // Queue full
    }

    q->items[old_tail] = item;
    // Release ensures the write completes before advancing tail
    atomic_store_explicit(&q->tail, new_tail, memory_order_release);
    #endif
    
    return true;
}

void *queue_pop(queue_t *q, bool peek) {
    if (queue_is_empty(q)) {
        return NULL;
    }

    #ifndef HAVE_STDATOMIC_H
    void *item = q->items[q->head];
    if (!peek) {
        q->head = (q->head + 1) % (q->capacity + 1);
    }
    #else
    size_t old_head = atomic_load_explicit(&q->head, memory_order_relaxed);
    size_t new_head = (old_head + 1) % (q->capacity + 1);
    void *item = q->items[old_head];

    if (!peek) {
        // Release ensures the read completes before advancing head
        atomic_store_explicit(&q->head, new_head, memory_order_release);
    }
    #endif
    
    return item;
}

bool queue_is_empty(const queue_t *q) {
    if (!q) {
        return true;
    }
    
    #ifndef HAVE_STDATOMIC_H
    size_t head = q->head;
    size_t tail = q->tail;
    #else
    // Ensure the current thread sees all prior updates to head and tail
    size_t head = atomic_load_explicit(&q->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&q->tail, memory_order_acquire);
    #endif
    return head == tail;
}

size_t queue_size(const queue_t *q) {
    if (!q) {
        return 0;
    }
    
    #ifndef HAVE_STDATOMIC_H
    size_t head = q->head;
    size_t tail = q->tail;
    #else
    // Ensure the current thread sees all prior updates to head and tail
    size_t head = atomic_load_explicit(&q->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&q->tail, memory_order_acquire);
    #endif
    
    // Calculate size considering the circular buffer
    if (tail >= head) {
        return tail - head;
    } else {
        return (q->capacity + 1) - head + tail;
    }
}

// Swap the last item of q0 (tail - 1) with the first item of q1 (head).
// Currently, this is only used for a special case in the framebuffer,
// and it's only safe under very specific conditions.
void *queue_swap(queue_t *q0, queue_t *q1) {
    #ifndef HAVE_STDATOMIC_H
    size_t tail = (q0->tail == 0) ? q0->capacity : q0->tail - 1;
    size_t head = q1->head;
    #else
    size_t tail = atomic_load_explicit(&q0->tail, memory_order_acquire);
    size_t head = atomic_load_explicit(&q1->head, memory_order_acquire);
    tail = (tail == 0) ? q0->capacity : tail - 1;
    #endif

    void *item0 = q0->items[tail];
    void *item1 = q1->items[head];

    q0->items[tail] = item1;
    q1->items[head] = item0;

    return item0;
}
