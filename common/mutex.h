/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2025 OpenMV, LLC.
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
 * Mutex implementation.
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

// Conflicts with picosdk
#define mutex_t omv_mutex_t

typedef enum {
    MUTEX_TID_IDE = 1,
    MUTEX_TID_OMV = 2
} mutex_tid_t;

typedef volatile struct {
    atomic_size_t tid;
    atomic_flag lock;
} mutex_t;

void mutex_init0(mutex_t *mutex);
void mutex_lock(mutex_t *mutex, size_t tid);
bool mutex_try_lock(mutex_t *mutex, size_t tid);
// Prevents a thread from locking twice in a row.
bool mutex_try_lock_fair(mutex_t *mutex, size_t tid);
void mutex_unlock(mutex_t *mutex, size_t tid);
#endif // __MUTEX_H__
