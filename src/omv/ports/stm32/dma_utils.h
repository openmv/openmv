/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * STM32 DMA helper functions.
 */
#ifndef __DMA_UTILS_H__
#define __DMA_UTILS_H__

#ifdef MDMA
#define MDMA_CHAN_TO_INSTANCE(x) \
    (MDMA_Channel_TypeDef *) (MDMA_Channel0_BASE + ((MDMA_Channel1_BASE - MDMA_Channel0_BASE) * x))
#endif

uint8_t dma_utils_channel_to_irqn(DMA_Stream_TypeDef *dma_channel);
uint8_t dma_utils_channel_to_id(DMA_Stream_TypeDef *dma_channel);
int dma_utils_set_irq_descr(DMA_Stream_TypeDef *dma_channel, DMA_HandleTypeDef *dma_descr);
#endif // __DMA_UTILS_H__
