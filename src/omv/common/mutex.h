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
