#include <stm32f4xx.h>
#include <core_cm4.h>
#include <core_cmInstr.h>
#include "mutex.h"

void mutex_init(mutex_t *mutex)
{
    mutex_unlock(mutex);
}

void mutex_lock(mutex_t *mutex)
{
    volatile int locked = 0;
    /* wait for mutex to be unlocked */
    do {
        /* attemp ex read */
        while (__LDREXW(mutex) != 0);

        /* attempt to lock mutex */
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

    /* attempt to lock mutex */
    locked = __STREXW(1, mutex);
    __DMB();

    return (locked == 0);
}

void mutex_unlock(mutex_t *mutex)
{
    __DMB();
    *mutex = 0;
}
