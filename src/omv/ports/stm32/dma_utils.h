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
#ifndef __DMA_UTILS_H__
#define __DMA_UTILS_H__

#ifdef MDMA
#define MDMA_CHAN_TO_INSTANCE(x) \
    (MDMA_Channel_TypeDef *) (MDMA_Channel0_BASE + ((MDMA_Channel1_BASE - MDMA_Channel0_BASE) * x))
#endif

uint8_t dma_utils_channel_to_irqn(void *dma_channel);
uint8_t dma_utils_channel_to_id(void *dma_channel);
int dma_utils_set_irq_descr(void *dma_channel, DMA_HandleTypeDef *dma_descr);
uint8_t dma_utils_mpu_region_size(uint32_t size);
#endif // __DMA_UTILS_H__
