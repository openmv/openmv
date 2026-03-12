/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * Ethos-U HAL for QEMU/FVP port.
 */
#ifdef ETHOS_U
#include <stdio.h>
#include CMSIS_MCU_H

#include "py/mphal.h"
#include "py/runtime.h"

#include "ethosu_driver.h"

// SSE-300 Ethos-U NPU base address and IRQ.
#define ETHOSU_BASE_ADDRESS     ((void *) 0x48102000)
#define ETHOSU_IRQ              56
#define ETHOSU_IRQ_PRI          5

static struct ethosu_driver ethosu_driver;

struct ethosu_semaphore_t {
    uint8_t count;
};

void *ethosu_semaphore_create(void) {
    static struct ethosu_semaphore_t sem;
    sem.count = 0;
    return &sem;
}

int ethosu_semaphore_take(void *sem, uint64_t timeout) {
    struct ethosu_semaphore_t *s = sem;
    while (s->count == 0) {
        __WFE();
    }
    s->count--;
    return 0;
}

int ethosu_semaphore_give(void *sem) {
    struct ethosu_semaphore_t *s = sem;
    s->count++;
    __SEV();
    return 0;
}

void ethosu_semaphore_destroy(void *sem) {
}

void ethosu_flush_dcache(uint32_t *p, size_t bytes) {
    if (!p) {
        SCB_CleanDCache();
    } else {
        SCB_CleanDCache_by_Addr(p, bytes);
    }
}

void ethosu_invalidate_dcache(uint32_t *p, size_t bytes) {
    if (!p) {
        SCB_CleanInvalidateDCache();
    } else {
        SCB_InvalidateDCache_by_Addr(p, bytes);
    }
}

uint64_t ethosu_address_remap(uint64_t address, int index) {
    return address;
}

void ethosu_inference_begin(struct ethosu_driver *drv, void *user_arg) {
    mp_handle_pending_internal(MP_HANDLE_PENDING_CALLBACKS_ONLY);
}

void ethosu_inference_end(struct ethosu_driver *drv, void *user_arg) {
    mp_handle_pending_internal(MP_HANDLE_PENDING_CALLBACKS_ONLY);
}

static void ETHOSU_IRQ_Handler(void) {
    ethosu_irq_handler(&ethosu_driver);
}

int qemu_npu_init(void) {
    if (ethosu_init(&ethosu_driver,
                    ETHOSU_BASE_ADDRESS,
                    NULL,
                    0,
                    1,
                    1)) {
        return -1;
    }

    NVIC_SetVector((IRQn_Type) ETHOSU_IRQ, (uint32_t) ETHOSU_IRQ_Handler);
    NVIC_SetPriority((IRQn_Type) ETHOSU_IRQ, ETHOSU_IRQ_PRI);
    NVIC_ClearPendingIRQ((IRQn_Type) ETHOSU_IRQ);
    NVIC_EnableIRQ((IRQn_Type) ETHOSU_IRQ);
    return 0;
}
#endif // ETHOS_U
