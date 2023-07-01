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
#include <stdbool.h>
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "common.h"
#include "dma_utils.h"

// Defined in micropython/ports/stm32/dma.c
// or in uvc/src/main.c
extern DMA_HandleTypeDef *dma_handle[16];

uint8_t dma_utils_channel_to_irqn(DMA_Stream_TypeDef *dma_channel) {
    if (0) {
    #if defined(DMA1_Stream0)
    } else if (dma_channel == DMA1_Stream0) {
        return DMA1_Stream0_IRQn;
    #endif
    #if defined(DMA1_Stream1)
    } else if (dma_channel == DMA1_Stream1) {
        return DMA1_Stream1_IRQn;
    #endif
    #if defined(DMA1_Stream2)
    } else if (dma_channel == DMA1_Stream2) {
        return DMA1_Stream2_IRQn;
    #endif
    #if defined(DMA1_Stream3)
    } else if (dma_channel == DMA1_Stream3) {
        return DMA1_Stream3_IRQn;
    #endif
    #if defined(DMA1_Stream4)
    } else if (dma_channel == DMA1_Stream4) {
        return DMA1_Stream4_IRQn;
    #endif
    #if defined(DMA1_Stream5)
    } else if (dma_channel == DMA1_Stream5) {
        return DMA1_Stream5_IRQn;
    #endif
    #if defined(DMA1_Stream6)
    } else if (dma_channel == DMA1_Stream6) {
        return DMA1_Stream6_IRQn;
    #endif
    #if defined(DMA1_Stream7)
    } else if (dma_channel == DMA1_Stream7) {
        return DMA1_Stream7_IRQn;
    #endif
    #if defined(DMA2_Stream0)
    } else if (dma_channel == DMA2_Stream0) {
        return DMA2_Stream0_IRQn;
    #endif
    #if defined(DMA2_Stream1)
    } else if (dma_channel == DMA2_Stream1) {
        return DMA2_Stream1_IRQn;
    #endif
    #if defined(DMA2_Stream2)
    } else if (dma_channel == DMA2_Stream2) {
        return DMA2_Stream2_IRQn;
    #endif
    #if defined(DMA2_Stream3)
    } else if (dma_channel == DMA2_Stream3) {
        return DMA2_Stream3_IRQn;
    #endif
    #if defined(DMA2_Stream4)
    } else if (dma_channel == DMA2_Stream4) {
        return DMA2_Stream4_IRQn;
    #endif
    #if defined(DMA2_Stream5)
    } else if (dma_channel == DMA2_Stream5) {
        return DMA2_Stream5_IRQn;
    #endif
    #if defined(DMA2_Stream6)
    } else if (dma_channel == DMA2_Stream6) {
        return DMA2_Stream6_IRQn;
    #endif
    #if defined(DMA2_Stream7)
    } else if (dma_channel == DMA2_Stream7) {
        return DMA2_Stream7_IRQn;
    #endif
    }
    return 0;
}

// This returns a DMA ID that can be used to index into the dma_handle
// array defined in micropython. Setting a DMA handle in that array allows
// DMA IRQ handlers (which are all defined in micropython) to use it.
uint8_t dma_utils_channel_to_id(DMA_Stream_TypeDef *dma_channel) {
    uint8_t dma_id = -1;

    if (0) {
    #if defined(DMA1_Stream0)
    } else if (dma_channel == DMA1_Stream0) {
        dma_id = 0;
    #endif
    #if defined(DMA1_Stream1)
    } else if (dma_channel == DMA1_Stream1) {
        dma_id = 1;
    #endif
    #if defined(DMA1_Stream2)
    } else if (dma_channel == DMA1_Stream2) {
        dma_id = 2;
    #endif
    #if defined(DMA1_Stream3)
    } else if (dma_channel == DMA1_Stream3) {
        dma_id = 3;
    #endif
    #if defined(DMA1_Stream4)
    } else if (dma_channel == DMA1_Stream4) {
        dma_id = 4;
    #endif
    #if defined(DMA1_Stream5)
    } else if (dma_channel == DMA1_Stream5) {
        dma_id = 5;
    #endif
    #if defined(DMA1_Stream6)
    } else if (dma_channel == DMA1_Stream6) {
        dma_id = 6;
    #endif
    #if defined(DMA1_Stream7)
    } else if (dma_channel == DMA1_Stream7) {
        dma_id = 7;
    #endif
    #if defined(DMA2_Stream0)
    } else if (dma_channel == DMA2_Stream0) {
        dma_id = 8;
    #endif
    #if defined(DMA2_Stream1)
    } else if (dma_channel == DMA2_Stream1) {
        dma_id = 9;
    #endif
    #if defined(DMA2_Stream2)
    } else if (dma_channel == DMA2_Stream2) {
        dma_id = 10;
    #endif
    #if defined(DMA2_Stream3)
    } else if (dma_channel == DMA2_Stream3) {
        dma_id = 11;
    #endif
    #if defined(DMA2_Stream4)
    } else if (dma_channel == DMA2_Stream4) {
        dma_id = 12;
    #endif
    #if defined(DMA2_Stream5)
    } else if (dma_channel == DMA2_Stream5) {
        dma_id = 13;
    #endif
    #if defined(DMA2_Stream6)
    } else if (dma_channel == DMA2_Stream6) {
        dma_id = 14;
    #endif
    #if defined(DMA2_Stream7)
    } else if (dma_channel == DMA2_Stream7) {
        dma_id = 15;
    #endif
    }

    return dma_id;
}

int dma_utils_set_irq_descr(DMA_Stream_TypeDef *dma_channel, DMA_HandleTypeDef *dma_descr) {
    uint8_t dma_id = dma_utils_channel_to_id(dma_channel);
    if (dma_id != -1) {
        dma_handle[dma_id] = dma_descr;
        return 0;
    }
    return -1;
}
