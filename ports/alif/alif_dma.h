/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alif DMA driver.
 */
#ifndef __ALIF_DMA_H__
#define __ALIF_DMA_H__
#include <stdint.h>
#include <stdbool.h>
#include "dma_ctrl.h"
#include "dma_op.h"
#include "dma_mapping.h"

typedef enum {
    DMA_BSWAP_NONE,
    DMA_BSWAP_16BIT,
    DMA_BSWAP_32BIT,
    DMA_BSWAP_64BIT,
    DMA_BSWAP_128BIT,
} dma_swap_t;

typedef enum {
    DMA_BURST_SIZE_1,
    DMA_BURST_SIZE_2,
    DMA_BURST_SIZE_4,
    DMA_BURST_SIZE_8,
    DMA_BURST_SIZE_16,
} dma_burst_size_t;

typedef enum {
    DMA_EVENT_ABORTED   = (1 << 0),
    DMA_EVENT_COMPLETE  = (1 << 1),
} dma_event_t;

typedef enum {
    DMA_FLAGS_SINGLE_CH = DMA_CHANNEL_FLAG_I2S_MONO_MODE,
} dma_flags_t;

typedef struct {
    DMA_Type *inst;
    int8_t index;
} dma_channel_t;

typedef struct {
    DMA_Type *inst;
    uint32_t request;
    uint32_t priority;
    uint8_t direction;
    uint8_t burst_size;
    uint8_t burst_blen;
    uint8_t byte_swap;
    uint32_t flags;
} dma_config_t;

typedef void (*dma_callback_t) (dma_event_t event);

int dma_deinit_all();
int dma_alloc(dma_channel_t *channel, dma_config_t *config);
int dma_start(dma_channel_t *channel, void *src, void *dst0, void *dst1, uint32_t size, dma_callback_t callback);
int dma_abort(dma_channel_t *channel, bool dealloc);
void *dma_target_address(dma_channel_t *channel);
#endif //__ALIF_DMA_H__
