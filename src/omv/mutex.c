/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mutex.
 *
 */
#include <stm32f4xx.h>
#include "mutex.h"

// This is a standard implementation of mutexs on ARM processors following the ARM guide.
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHEJCHB.html

void mutex_init(mutex_t *mutex)
{
    mutex_unlock(mutex);
}

void mutex_lock(mutex_t *mutex)
{
    volatile int locked = 0;
    // Wait for mutex to be unlocked
    do {
        // Attempt exclusive read
        while (__LDREXW(mutex) != 0);

        // Attempt to lock mutex
        locked = __STREXW(1, mutex);
    } while (locked != 0);
    __DMB();
}

int mutex_try_lock(mutex_t *mutex)
{
    volatile int locked = 1;

    if (__LDREXW(mutex) != 0) {
        return 0;
    }

    // Attempt to lock mutex
    locked = __STREXW(1, mutex);
    __DMB();

    return (locked == 0);
}

void mutex_unlock(mutex_t *mutex)
{
    __DMB();
    *mutex = 0;
}
