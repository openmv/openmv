/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2024 OpenMV, LLC.
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
 * STM32 Nema HAL layer.
 */
#include <stdlib.h>
#include STM32_HAL_H

#include "omv_boardconfig.h"

#if (OMV_GPU_NEMA == 1)
#include "py/mphal.h"
#include "irq.h"

#include "omv_common.h"
#include "nema_core.h"
#include "nema_sys_defs.h"

#ifndef OMV_GPU_NEMA_RING_SIZE
#define OMV_GPU_NEMA_RING_SIZE 1024
#endif

volatile static int last_cl_id;
static GPU2D_HandleTypeDef gpu2d = { 0 };
static nema_ringbuffer_t ring_buf = {{0}};
#if OMV_GPU_NEMA_MM_STATIC
uint8_t OMV_ATTR_SECTION(OMV_ATTR_ALIGNED(RING_BUFFER[OMV_GPU_NEMA_RING_SIZE], 32), ".dma_buffer");
#endif

int32_t nema_sys_init(void) {
    // Initialize GPU2D
    gpu2d.Instance = GPU2D;
    HAL_GPU2D_Init(&gpu2d);

    // Configure and enable texture cache
    HAL_ICACHE_DeInit();
    HAL_ICACHE_Disable();
    HAL_ICACHE_ConfigAssociativityMode(ICACHE_4WAYS);
    HAL_ICACHE_Enable();

    // Allocate ring buffer.
    #if OMV_GPU_NEMA_MM_STATIC
    ring_buf.bo.fd = 0;
    ring_buf.bo.size = sizeof(RING_BUFFER);
    ring_buf.bo.base_virt = RING_BUFFER;
    ring_buf.bo.base_phys = (uint32_t) RING_BUFFER;
    #else
    ring_buf.bo = nema_buffer_create(OMV_GPU_NEMA_RING_SIZE);
    #endif

    // Initialize ring buffer.
    if (nema_rb_init(&ring_buf, 1) != 0) {
        return -1;
    }

    NVIC_SetPriority(GPU2D_IRQn, IRQ_PRI_GPU);
    NVIC_EnableIRQ(GPU2D_IRQn);

    NVIC_SetPriority(GPU2D_ER_IRQn, IRQ_PRI_GPU);
    NVIC_EnableIRQ(GPU2D_ER_IRQn);
    return 0;
}

int nema_wait_irq_cl(int cl_id) {
    while (last_cl_id < cl_id) {
        __WFI();
    }
    return 0;
}

int nema_wait_irq_brk(int brk_id) {
    while (nema_reg_read(GPU2D_BREAKPOINT) == 0U) {
        __WFI();
    }
    return 0;
}

uint32_t nema_reg_read(uint32_t reg) {
    return HAL_GPU2D_ReadRegister(&gpu2d, reg);
}

void nema_reg_write(uint32_t reg, uint32_t value) {
    HAL_GPU2D_WriteRegister(&gpu2d, reg, value);
}

nema_buffer_t nema_buffer_create(int size) {
    nema_buffer_t bo = {
        .base_virt = nema_host_malloc(size),
        .base_phys = (uint32_t) bo.base_virt,
        .size = size,
    };
    return bo;
}

nema_buffer_t nema_buffer_create_pool(int pool, int size) {
    return nema_buffer_create(size);
}

void *nema_buffer_map(nema_buffer_t *bo) {
    return bo->base_virt;
}

void nema_buffer_unmap(nema_buffer_t *bo) {
    UNUSED(bo);
}

void nema_buffer_destroy(nema_buffer_t *bo) {
    nema_host_free(bo->base_virt);
}

uintptr_t nema_buffer_phys(nema_buffer_t *bo) {
    return bo->base_phys;
}

void nema_buffer_flush(nema_buffer_t *bo) {
    #if !OMV_GPU_NEMA_MM_STATIC
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *) bo->base_virt, bo->size);
    #endif
}

void nema_host_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

void *nema_host_malloc(unsigned size) {
    return malloc(size);
}

int nema_mutex_lock(int mutex_id) {
    return 0;
}

int nema_mutex_unlock(int mutex_id) {
    return 0;
}

void GPU2D_IRQHandler(void) {
    HAL_GPU2D_IRQHandler(&gpu2d);
}

void GPU2D_ER_IRQHandler(void) {
    HAL_GPU2D_ER_IRQHandler(&gpu2d);
}

void HAL_GPU2D_CommandListCpltCallback(GPU2D_HandleTypeDef *gpu2d, uint32_t CmdListID) {
    last_cl_id = CmdListID;
}
#endif // OMV_GPU_NEMA
