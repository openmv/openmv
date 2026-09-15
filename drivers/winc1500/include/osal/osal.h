/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
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
 * Minimal OSAL shim for the WINC1500 host driver.
 */
#ifndef __WINC_OSAL_H__
#define __WINC_OSAL_H__
#include <stdint.h>
#include <stddef.h>
#include "umalloc.h"

typedef enum {
    OSAL_RESULT_FALSE = 0,
    OSAL_RESULT_TRUE = 1,
} OSAL_RESULT;

typedef enum {
    OSAL_SEM_TYPE_BINARY = 0,
    OSAL_SEM_TYPE_COUNTING,
} OSAL_SEM_TYPE;

typedef uintptr_t OSAL_MUTEX_HANDLE_TYPE;
typedef uintptr_t OSAL_SEM_HANDLE_TYPE;

#define OSAL_WAIT_FOREVER       (0xFFFFFFFFU)

// The driver is only ever entered from the MicroPython context: calls from the
// WINC interrupt are deferred to PendSV, which cannot preempt a driver call.
// There is therefore nothing to serialize, and blocking here would deadlock, so
// the locks always succeed immediately.
static inline OSAL_RESULT OSAL_MUTEX_Create(OSAL_MUTEX_HANDLE_TYPE *mutex) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_MUTEX_Delete(OSAL_MUTEX_HANDLE_TYPE *mutex) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_MUTEX_Lock(OSAL_MUTEX_HANDLE_TYPE *mutex, uint32_t waitMS) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_MUTEX_Unlock(OSAL_MUTEX_HANDLE_TYPE *mutex) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_SEM_Create(OSAL_SEM_HANDLE_TYPE *semID, OSAL_SEM_TYPE type,
                                          uint8_t maxCount, uint8_t initialCount) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_SEM_Delete(OSAL_SEM_HANDLE_TYPE *semID) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_SEM_Pend(OSAL_SEM_HANDLE_TYPE *semID, uint32_t waitMS) {
    return OSAL_RESULT_TRUE;
}

static inline OSAL_RESULT OSAL_SEM_Post(OSAL_SEM_HANDLE_TYPE *semID) {
    return OSAL_RESULT_TRUE;
}

#define OSAL_Malloc(size)   uma_malloc((size), 0)
#define OSAL_Free(ptr)      uma_free(ptr)

#endif // __WINC_OSAL_H__
