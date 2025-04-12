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
 * This is a standard implementation of mutexs on ARM processors following the ARM guide.
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHEJCHB.html
 *
 * Note: The Cortex-M0/M0+ does Not have the Load/Store exclusive instructions, on these
 * CPUs the locking function is implemented with atomic access using disable/enable IRQs.
 */
#include "mutex.h"
#include "cmsis_compiler.h"
#include "py/mphal.h"

void mutex_init0(omv_mutex_t *mutex) {
    __DMB();
    mutex->tid = 0;
    mutex->lock = 0;
    mutex->last_tid = 0;
}

static void _mutex_lock(omv_mutex_t *mutex, uint32_t tid, bool blocking) {
    #if (__ARM_ARCH < 7)
    do {
        __disable_irq();
        if (mutex->lock == 0) {
            mutex->lock = 1;
            mutex->tid = tid;
        }
        __enable_irq();
    } while (mutex->tid != tid && blocking);
    #else
    do {
        // Attempt to lock the mutex
        if (__LDREXW(&mutex->lock) == 0) {
            if (__STREXW(1, &mutex->lock) == 0) {
                // Set TID if mutex is locked
                mutex->tid = tid;
            }
        }
    } while (mutex->tid != tid && blocking);
    #endif
    __DMB();
}

void mutex_lock(omv_mutex_t *mutex, uint32_t tid) {
    _mutex_lock(mutex, tid, true);
}

int mutex_try_lock(omv_mutex_t *mutex, uint32_t tid) {
    // If the mutex is already locked by the current thread then
    // release it and return without locking, otherwise try to lock it.
    if (mutex->tid == tid) {
        mutex_unlock(mutex, tid);
    } else {
        _mutex_lock(mutex, tid, false);
    }

    return (mutex->tid == tid);
}

int mutex_try_lock_alternate(omv_mutex_t *mutex, uint32_t tid) {
    if (mutex->last_tid != tid) {
        if (mutex_try_lock(mutex, tid)) {
            mutex->last_tid = tid;
            return 1;
        }
    }

    return 0;
}

int mutex_lock_timeout(omv_mutex_t *mutex, uint32_t tid, uint32_t timeout) {
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while ((mp_hal_ticks_ms() - tick_start) >= timeout) {
        if (mutex_try_lock(mutex, tid)) {
            return 1;
        }
        __WFI();
    }
    return 0;
}

void mutex_unlock(omv_mutex_t *mutex, uint32_t tid) {
    if (mutex->tid == tid) {
        __DMB();
        mutex->tid = 0;
        mutex->lock = 0;
    }
}
