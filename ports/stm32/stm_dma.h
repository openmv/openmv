/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023-2024 OpenMV, LLC.
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
 * STM32 DMA helper functions.
 */
#ifndef __STM_DMA_H__
#define __STM_DMA_H__
#include <stdint.h>
#include <stdbool.h>
#include STM32_HAL_H

extern const DMA_InitTypeDef stm_dma_csi_init;
extern const DMA_InitTypeDef stm_dma_spi_init;
extern const DMA_InitTypeDef stm_dma_sai_init;
extern const DMA_InitTypeDef stm_dma_dfsdm_init;
extern const DMA_InitTypeDef stm_dma_mdf_init;

uint8_t stm_dma_channel_to_irqn(void *dma_channel);
uint8_t stm_dma_channel_to_id(void *dma_channel);
int stm_dma_set_irq_descr(void *dma_channel, DMA_HandleTypeDef *dma_descr);
uint8_t stm_dma_mpu_region_size(uint32_t size);
int stm_dma_init(DMA_HandleTypeDef *dma_descr, void *dma_channel, uint32_t request,
                 uint32_t direction, uint32_t ssize, uint32_t dsize, uint32_t ports,
                 const DMA_InitTypeDef *init, bool circular);

#if defined(STM32N6)
int stm_dma_ll_init(DMA_HandleTypeDef *dma_descr, DMA_QListTypeDef *dma_queue,
        DMA_NodeTypeDef *dma_nodes, size_t nodes_count, uint32_t ports);
#endif

#ifdef OMV_MDMA_CHANNEL_DCMI_0
#include "omv_csi.h"

#define MDMA_BUFFER_SIZE    (64)
#define MDMA_CHAN_TO_INSTANCE(x) \
    (MDMA_Channel_TypeDef *) (MDMA_Channel0_BASE + ((MDMA_Channel1_BASE - MDMA_Channel0_BASE) * x))

void stm_mdma_init(omv_csi_t *csi, uint32_t bytes_per_pixel, uint32_t x_crop);
void stm_mdma_init_channel(omv_csi_t *csi, MDMA_InitTypeDef *init, uint32_t bytes_per_pixel, uint32_t x_crop);
void stm_mdma_start(omv_csi_t *csi, uint32_t src, uint32_t dst, uint32_t line_width, uint32_t line_count);
#endif  // OMV_MDMA_CHANNEL_DCMI_0

#endif // __STM_DMA_H__
