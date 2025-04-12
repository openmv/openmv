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
 * Mutex implementation.
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__
#include <stdint.h>
#define MUTEX_TID_IDE    (1 << 0)
#define MUTEX_TID_OMV    (1 << 1)

typedef volatile struct {
    uint32_t tid;
    uint32_t lock;
    uint32_t last_tid;
} omv_mutex_t;

void mutex_init0(omv_mutex_t *mutex);
void mutex_lock(omv_mutex_t *mutex, uint32_t tid);
int mutex_try_lock(omv_mutex_t *mutex, uint32_t tid);
int mutex_try_lock_alternate(omv_mutex_t *mutex, uint32_t tid);
int mutex_lock_timeout(omv_mutex_t *mutex, uint32_t tid, uint32_t timeout);
void mutex_unlock(omv_mutex_t *mutex, uint32_t tid);
#endif /* __MUTEX_H__ */
