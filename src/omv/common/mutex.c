/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mutex implementation.
 */
#include "mutex.h"
#include "cmsis_gcc.h"
#include "py/mphal.h"

// This is a standard implementation of mutexs on ARM processors following the ARM guide.
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHEJCHB.html

void mutex_init(mutex_t *mutex)
{
    __DMB();
    mutex->tid = 0;
    mutex->lock = 0;
}

void mutex_lock(mutex_t *mutex, uint32_t tid)
{
    volatile int locked = 0;
    // Wait for mutex to be unlocked
    do {
        // Attempt exclusive read
        while (__LDREXW(&mutex->lock) != 0);

        // Attempt to lock mutex
        locked = __STREXW(1, &mutex->lock);

        // Set TID if mutex is locked
        if (locked == 0) {
            mutex->tid = tid;
        }
    } while (locked != 0);

    __DMB();
}

int mutex_try_lock(mutex_t *mutex, uint32_t tid)
{
    volatile int locked = 1;

    // If mutex is already locked by the current thread
    // then release the the mutex, else attempt to lock it.
    if (mutex->tid == tid) {
        mutex_unlock(mutex, tid);
    } else if (__LDREXW(&mutex->lock) == 0) {
        // Attempt to lock the mutex
        locked = __STREXW(1, &mutex->lock);
        __DMB();

        // Set TID if mutex is locked
        if (locked == 0) {
            mutex->tid = tid;
        }
    }

    return (locked == 0);
}

int mutex_lock_timeout(mutex_t *mutex, uint32_t tid, uint32_t timeout)
{
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while ((mp_hal_ticks_ms() - tick_start) >= timeout) {
        if (mutex_try_lock(mutex, tid)) {
            return 1;
        }
        __WFI();
    }
    return 0;
}

void mutex_unlock(mutex_t *mutex, uint32_t tid)
{
    if (mutex->tid == tid) {
        __DMB();
        mutex->tid = 0;
        mutex->lock = 0;
    }
}
