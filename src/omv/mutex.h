/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mutex.
 *
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__
#include <stdint.h>
typedef volatile uint32_t mutex_t;
void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
int mutex_try_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);
#endif /* __MUTEX_H__ */
