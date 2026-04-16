/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * STM32 VC8000 EWL (Encoder Wrapper Layer) implementation.
 */
#include <stdatomic.h>
#include <string.h>
#include STM32_HAL_H

#include "board_config.h"

#if (OMV_VC8000_ENABLE == 1)
#include "py/mphal.h"
#include "irq.h"

#include "imlib.h"
#include "ewl.h"
#include "reg_offset_v7.h"

#ifndef EWL_TIMEOUT
#define EWL_TIMEOUT 100UL
#endif

// Required by ewl.h; used by the input line buffer test path (unused here).
u32 (*pollInputLineBufTestFunc) (void) = NULL;

// Minimal EWL instance.
typedef struct {
    u32 clientType;
} omv_ewl_inst_t;

static omv_ewl_inst_t ewl_inst;
static atomic_int venc_irq_fired;

// ---------------------------------------------------------------------------
// Hardware register access
// ---------------------------------------------------------------------------

u32 EWLReadAsicID(void) {
    return LL_VENC_ReadRegister(0UL);
}

EWLHwConfig_t EWLReadAsicConfig(void) {
    EWLHwConfig_t cfg = {};
    u32 cfgval;

    // First capability word is in register 63.
    cfgval = LL_VENC_ReadRegister(63UL);
    cfg.maxEncodedWidth = cfgval & ((1U << 12U) - 1U);
    cfg.h264Enabled = (cfgval >> 27U) & 1U;
    cfg.vp8Enabled = (cfgval >> 26U) & 1U;
    cfg.jpegEnabled = (cfgval >> 25U) & 1U;
    cfg.vsEnabled = (cfgval >> 24U) & 1U;
    cfg.rgbEnabled = (cfgval >> 28U) & 1U;
    cfg.searchAreaSmall = (cfgval >> 29U) & 1U;
    cfg.scalingEnabled = (cfgval >> 30U) & 1U;
    cfg.busType = (cfgval >> 20U) & 15U;
    cfg.synthesisLanguage = (cfgval >> 16U) & 15U;
    cfg.busWidth = (cfgval >> 12U) & 15U;

    // Second capability word is in register 296.
    cfgval = LL_VENC_ReadRegister(296UL);
    cfg.addr64Support = (cfgval >> 31U) & 1U;
    cfg.dnfSupport = (cfgval >> 30U) & 1U;
    cfg.rfcSupport = (cfgval >> 28U) & 3U;
    cfg.enhanceSupport = (cfgval >> 27U) & 1U;
    cfg.instantSupport = (cfgval >> 26U) & 1U;
    cfg.svctSupport = (cfgval >> 25U) & 1U;
    cfg.inAxiIdSupport = (cfgval >> 24U) & 1U;
    cfg.inLoopbackSupport = (cfgval >> 23U) & 1U;
    cfg.irqEnhanceSupport = (cfgval >> 22U) & 1U;

    return cfg;
}

// EWL register offset is in bytes; LL layer takes a register index (word units).
void EWLWriteReg(const void *inst, u32 offset, u32 val) {
    (void) inst;
    LL_VENC_WriteRegister((offset >> 2), val);
}

u32 EWLReadReg(const void *inst, u32 offset) {
    (void) inst;
    return LL_VENC_ReadRegister((offset >> 2));
}

void EWLEnableHW(const void *inst, u32 offset, u32 val) {
    EWLWriteReg(inst, offset, val);
}

void EWLDisableHW(const void *inst, u32 offset, u32 val) {
    EWLWriteReg(inst, offset, val);
}

void EWLWriteRegAll(const void *inst, const u32 *table, u32 size) {
    for (u32 i = 0; i < size; i++) {
        EWLWriteReg(inst, i * 4, table[i]);
    }
}

void EWLReadRegAll(const void *inst, u32 *table, u32 size) {
    for (u32 i = 0; i < size; i++) {
        table[i] = EWLReadReg(inst, i * 4);
    }
}

void EWLDCacheRangeFlush(const void *inst, EWLLinearMem_t *info) {
    (void) inst;
    SCB_CleanDCache_by_Addr((uint32_t *) info->virtualAddress, (int32_t) info->size);
}

void EWLDCacheRangeRefresh(const void *inst, EWLLinearMem_t *info) {
    (void) inst;
    SCB_InvalidateDCache_by_Addr((uint32_t *) info->virtualAddress, (int32_t) info->size);
}

// ---------------------------------------------------------------------------
// EWL init / release
// ---------------------------------------------------------------------------

const void *EWLInit(EWLInitParam_t *param) {
    if (param == NULL) {
        return NULL;
    }
    if (VENC_REG(1U) & ~ASIC_STATUS_FRAME_READY) {
        __HAL_RCC_VENC_FORCE_RESET();
        __HAL_RCC_VENC_RELEASE_RESET();
    }
    ewl_inst.clientType = param->clientType;
    atomic_store_explicit(&venc_irq_fired, 0, memory_order_relaxed);
    return &ewl_inst;
}

i32 EWLRelease(const void *inst) {
    (void) inst;
    return EWL_OK;
}

// ---------------------------------------------------------------------------
// Hardware reservation (single instance; reservation is always granted)
// ---------------------------------------------------------------------------

i32 EWLReserveHw(const void *inst) {
    (void) inst;
    return EWL_OK;
}

void EWLReleaseHw(const void *inst) {
    (void) inst;
}

// ---------------------------------------------------------------------------
// DMA-coherent linear memory allocation
//
// This memory must be cache aligned as it's given to the VENC hardware DMA.
// ---------------------------------------------------------------------------

i32 EWLMallocLinear(const void *inst, u32 size, EWLLinearMem_t *info) {
    (void) inst;
    void *ptr = uma_malloc(size, UMA_CACHE | UMA_MAYBE | UMA_FAST);
    if (ptr == NULL) {
        return EWL_ERROR;
    }
    info->virtualAddress = (u32 *) ptr;
    info->busAddress = (ptr_t) ptr;
    info->size = OMV_ALIGN_TO(size, OMV_CACHE_LINE_SIZE);
    return EWL_OK;
}

void EWLFreeLinear(const void *inst, EWLLinearMem_t *info) {
    (void) inst;
    uma_free(info->virtualAddress);
    info->virtualAddress = NULL;
    info->busAddress = 0;
    info->size = 0;
}

i32 EWLMallocRefFrm(const void *inst, u32 size, EWLLinearMem_t *info) {
    return EWLMallocLinear(inst, size, info);
}

void EWLFreeRefFrm(const void *inst, EWLLinearMem_t *info) {
    EWLFreeLinear(inst, info);
}

// ---------------------------------------------------------------------------
// SW-only memory utilities (thin wrappers around libc)
// ---------------------------------------------------------------------------

void *EWLmalloc(u32 n) {
    return uma_malloc(n, UMA_MAYBE | UMA_FAST);
}

void *EWLcalloc(u32 n, u32 s) {
    return uma_calloc(n * s, UMA_MAYBE | UMA_FAST);
}

void EWLfree(void *p) {
    uma_free(p);
}

void *EWLmemcpy(void *d, const void *s, u32 n) {
    return memcpy(d, s, (size_t) n);
}

void *EWLmemset(void *d, i32 c, u32 n) {
    return memset(d, c, (size_t) n);
}

int EWLmemcmp(const void *s1, const void *s2, u32 n) {
    return memcmp(s1, s2, (size_t) n);
}

// ---------------------------------------------------------------------------
// On-chip SRAM base for input MB line buffer
// VENCRAM is dedicated to the encoder and is not mapped to the CPU address
// space, so this returns EWL_ERROR (HW line buffer loopback not used).
// ---------------------------------------------------------------------------

i32 EWLGetInputLineBufferBase(const void *inst, EWLLinearMem_t *info) {
    (void) inst;
    (void) info;
    return EWL_ERROR;
}

// ---------------------------------------------------------------------------
// Hardware synchronization
// ---------------------------------------------------------------------------

i32 EWLWaitHwRdy(const void *inst, u32 *slicesReady) {
    if (inst == NULL) {
        return EWL_HW_WAIT_ERROR;
    }

    // atomic_exchange atomically reads and clears the flag in one operation,
    // so there is no window between the test and the clear where a new ISR
    // could set it and have the store silently clobber it.
    for (mp_uint_t tickstart = mp_hal_ticks_ms();
         !atomic_exchange_explicit(&venc_irq_fired, 0, memory_order_acq_rel); ) {

        mp_uint_t elapsed = mp_hal_ticks_ms() - tickstart;
        if (elapsed > EWL_TIMEOUT) {
            return EWL_HW_WAIT_TIMEOUT;
        }

        imlib_poll_events_noexc();
    }

    if (slicesReady != NULL) {
        *slicesReady = (VENC_REG(21UL) >> 16) & 0xFFUL;
    }

    return EWL_OK;
}

void VENC_IRQHandler(void) {
    u32 hw_handshake_status = READ_BIT(VENC_REG(BASE_HEncInstantInput >> 2U), (1U << 29U));
    u32 irq_status = VENC_REG(1U);

    if (!hw_handshake_status && (irq_status & ASIC_STATUS_FUSE)) {
        VENC_REG(1U) = ASIC_STATUS_FUSE | ASIC_IRQ_LINE;
        // Read back the IRQ status to update its value.
        irq_status = VENC_REG(1U);
    }

    // See if there are other flags than the FUSE status raised
    if (irq_status != 0U) {
        // Status flag is raised, clear the ones that the IRQ needs to clear and signal to EWLWaitHwReady.
        VENC_REG(1U) = ASIC_STATUS_SLICE_READY | ASIC_IRQ_LINE;
        atomic_store_explicit(&venc_irq_fired, 1, memory_order_release);
    }
}
#endif // OMV_VC8000_ENABLE
