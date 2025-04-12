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
#include <stdbool.h>
#include STM32_HAL_H
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_common.h"
#include "dma_utils.h"

#if defined(GPDMA1)
DMA_HandleTypeDef *dma_handle[32];
#else
// Defined in micropython/ports/stm32/dma.c or in uvc/src/main.c
extern DMA_HandleTypeDef *dma_handle[16];
#endif

uint8_t dma_utils_channel_to_irqn(void *dma_channel) {
    if (0) {
    #if defined(DMA1_Stream0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFFF00) == DMA1_BASE) {
        return ((DMA_Stream_TypeDef *) dma_channel - DMA1_Stream0) + DMA1_Stream0_IRQn;
    #endif
    #if defined(DMA2_Stream0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFFF00) == DMA2_BASE) {
        return ((DMA_Stream_TypeDef *) dma_channel - DMA2_Stream0) + DMA2_Stream0_IRQn;
    #endif
    #if defined(GPDMA1_Channel0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFF000) == GPDMA1_BASE) {
        return ((DMA_Channel_TypeDef *) dma_channel - GPDMA1_Channel0) + GPDMA1_Channel0_IRQn;
    #endif
    #if defined(HPDMA1_Channel0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFF000) == HPDMA1_BASE) {
        return ((DMA_Channel_TypeDef *) dma_channel - HPDMA1_Channel0) + HPDMA1_Channel0_IRQn;
    #endif
    }
    return 0;
}

// This returns a DMA ID that can be used to index into the dma_handle
// array defined in micropython. Setting a DMA handle in that array allows
// DMA IRQ handlers (which are all defined in micropython) to use it.
uint8_t dma_utils_channel_to_id(void *dma_channel) {
    if (0) {
    #if defined(DMA1_Stream0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFFF00) == DMA1_BASE) {
        return ((DMA_Stream_TypeDef *) dma_channel - DMA1_Stream0);
    #endif
    #if defined(DMA2_Stream0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFFF00) == DMA2_BASE) {
        return ((DMA_Stream_TypeDef *) dma_channel - DMA2_Stream0) + 8;
    #endif
    #if defined(GPDMA1_Channel0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFF000) == GPDMA1_BASE) {
        return ((DMA_Channel_TypeDef *) dma_channel - GPDMA1_Channel0);
    #endif
    #if defined(HPDMA1_Channel0)
    } else if ((((uint32_t) dma_channel) & 0xFFFFF000) == HPDMA1_BASE) {
        return ((DMA_Channel_TypeDef *) dma_channel - HPDMA1_Channel0) + 16;
    #endif
    }
    return -1;
}

int dma_utils_set_irq_descr(void *dma_channel, DMA_HandleTypeDef *dma_descr) {
    uint8_t dma_id = dma_utils_channel_to_id(dma_channel);
    if (dma_id != -1) {
        dma_handle[dma_id] = dma_descr;
        return 0;
    }
    return -1;
}

uint8_t dma_utils_mpu_region_size(uint32_t size) {
    #if (__ARM_ARCH <= 7)
    switch (size) {
        case 0x00000020U: {
            return MPU_REGION_SIZE_32B;
        }
        case 0x00000040U: {
            return MPU_REGION_SIZE_64B;
        }
        case 0x00000080U: {
            return MPU_REGION_SIZE_128B;
        }
        case 0x00000100U: {
            return MPU_REGION_SIZE_256B;
        }
        case 0x00000200U: {
            return MPU_REGION_SIZE_512B;
        }
        case 0x00000400U: {
            return MPU_REGION_SIZE_1KB;
        }
        case 0x00000800U: {
            return MPU_REGION_SIZE_2KB;
        }
        case 0x00001000U: {
            return MPU_REGION_SIZE_4KB;
        }
        case 0x00002000U: {
            return MPU_REGION_SIZE_8KB;
        }
        case 0x00004000U: {
            return MPU_REGION_SIZE_16KB;
        }
        case 0x00008000U: {
            return MPU_REGION_SIZE_32KB;
        }
        case 0x00010000U: {
            return MPU_REGION_SIZE_64KB;
        }
        case 0x00020000U: {
            return MPU_REGION_SIZE_128KB;
        }
        case 0x00040000U: {
            return MPU_REGION_SIZE_256KB;
        }
        case 0x00080000U: {
            return MPU_REGION_SIZE_512KB;
        }
        case 0x00100000U: {
            return MPU_REGION_SIZE_1MB;
        }
        case 0x00200000U: {
            return MPU_REGION_SIZE_2MB;
        }
        case 0x00400000U: {
            return MPU_REGION_SIZE_4MB;
        }
        case 0x00800000U: {
            return MPU_REGION_SIZE_8MB;
        }
        case 0x01000000U: {
            return MPU_REGION_SIZE_16MB;
        }
        case 0x02000000U: {
            return MPU_REGION_SIZE_32MB;
        }
        case 0x04000000U: {
            return MPU_REGION_SIZE_64MB;
        }
        case 0x08000000U: {
            return MPU_REGION_SIZE_128MB;
        }
        case 0x10000000U: {
            return MPU_REGION_SIZE_256MB;
        }
        case 0x20000000U: {
            return MPU_REGION_SIZE_512MB;
        }
        case 0x40000000U: {
            return MPU_REGION_SIZE_1GB;
        }
        case 0x80000000U: {
            return MPU_REGION_SIZE_2GB;
        }
        default: {
            return MPU_REGION_SIZE_4GB;
        }
    }
    #endif
    return -1;
}

#if defined(GPDMA1)
static inline void dma_utils_irq_handler(size_t irqn) {
    if (dma_handle[irqn] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[irqn]);
    }
}

void GPDMA1_Channel0_IRQHandler(void) {
    dma_utils_irq_handler(0);
}

void GPDMA1_Channel1_IRQHandler(void) {
    dma_utils_irq_handler(1);
}

void GPDMA1_Channel2_IRQHandler(void) {
    dma_utils_irq_handler(2);
}

void GPDMA1_Channel3_IRQHandler(void) {
    dma_utils_irq_handler(3);
}

void GPDMA1_Channel4_IRQHandler(void) {
    dma_utils_irq_handler(4);
}

void GPDMA1_Channel5_IRQHandler(void) {
    dma_utils_irq_handler(5);
}

void GPDMA1_Channel6_IRQHandler(void) {
    dma_utils_irq_handler(6);
}

void GPDMA1_Channel7_IRQHandler(void) {
    dma_utils_irq_handler(7);
}

void GPDMA1_Channel8_IRQHandler(void) {
    dma_utils_irq_handler(8);
}

void GPDMA1_Channel9_IRQHandler(void) {
    dma_utils_irq_handler(9);
}

void GPDMA1_Channel10_IRQHandler(void) {
    dma_utils_irq_handler(10);
}

void GPDMA1_Channel11_IRQHandler(void) {
    dma_utils_irq_handler(11);
}

void GPDMA1_Channel12_IRQHandler(void) {
    dma_utils_irq_handler(12);
}

void GPDMA1_Channel13_IRQHandler(void) {
    dma_utils_irq_handler(13);
}

void GPDMA1_Channel14_IRQHandler(void) {
    dma_utils_irq_handler(14);
}

void GPDMA1_Channel15_IRQHandler(void) {
    dma_utils_irq_handler(15);
}

void HPDMA1_Channel0_IRQHandler(void) {
    dma_utils_irq_handler(16);
}

void HPDMA1_Channel1_IRQHandler(void) {
    dma_utils_irq_handler(17);
}

void HPDMA1_Channel2_IRQHandler(void) {
    dma_utils_irq_handler(18);
}

void HPDMA1_Channel3_IRQHandler(void) {
    dma_utils_irq_handler(19);
}

void HPDMA1_Channel4_IRQHandler(void) {
    dma_utils_irq_handler(20);
}

void HPDMA1_Channel5_IRQHandler(void) {
    dma_utils_irq_handler(21);
}

void HPDMA1_Channel6_IRQHandler(void) {
    dma_utils_irq_handler(22);
}

void HPDMA1_Channel7_IRQHandler(void) {
    dma_utils_irq_handler(23);
}

void HPDMA1_Channel8_IRQHandler(void) {
    dma_utils_irq_handler(24);
}

void HPDMA1_Channel9_IRQHandler(void) {
    dma_utils_irq_handler(25);
}

void HPDMA1_Channel10_IRQHandler(void) {
    dma_utils_irq_handler(26);
}

void HPDMA1_Channel11_IRQHandler(void) {
    dma_utils_irq_handler(27);
}

void HPDMA1_Channel12_IRQHandler(void) {
    dma_utils_irq_handler(28);
}

void HPDMA1_Channel13_IRQHandler(void) {
    dma_utils_irq_handler(29);
}

void HPDMA1_Channel14_IRQHandler(void) {
    dma_utils_irq_handler(30);
}

void HPDMA1_Channel15_IRQHandler(void) {
    dma_utils_irq_handler(31);
}
#endif
