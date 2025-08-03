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
#include <stdint.h>
#include <string.h>
#include STM32_HAL_H
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_common.h"
#include "stm_dma.h"

#if defined(GPDMA1)
static DMA_HandleTypeDef *dma_handle[32];
#else
// Defined in micropython/ports/stm32/dma.c or in uvc/src/main.c
extern DMA_HandleTypeDef *dma_handle[16];
#endif

const DMA_InitTypeDef stm_dma_csi_init = {
    #if defined(STM32N6)
    .BlkHWRequest = DMA_BREQ_SINGLE_BURST,
    .Priority = DMA_HIGH_PRIORITY,
    .SrcBurstLength = 1,
    // The maximum allowed AXI burst length 16 for HPDMA.
    // TODO: Should set dynamically for GPDMA or other buses.
    .DestBurstLength = 16,
    .TransferEventMode = DMA_TCEM_BLOCK_TRANSFER,
    #elif defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
    .PeriphInc = DMA_PINC_DISABLE,
    .MemInc = DMA_MINC_ENABLE,
    .Priority = DMA_PRIORITY_HIGH,
    .FIFOMode = DMA_FIFOMODE_ENABLE,
    .FIFOThreshold = DMA_FIFO_THRESHOLD_FULL,
    .MemBurst = DMA_MBURST_INC4,
    .PeriphBurst = DMA_PBURST_SINGLE,
    #else
    #error Unsupported MCU
    #endif
};

const DMA_InitTypeDef stm_dma_spi_init = {
    #if defined(STM32N6)
    .BlkHWRequest = DMA_BREQ_SINGLE_BURST,
    .Priority = DMA_HIGH_PRIORITY,
    .SrcBurstLength = 1,
    .DestBurstLength = 1,
    .TransferEventMode = DMA_TCEM_BLOCK_TRANSFER,
    #elif defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
    .PeriphInc = DMA_PINC_DISABLE,
    .MemInc = DMA_MINC_ENABLE,
    .Priority = DMA_PRIORITY_HIGH,
    // If the FIFO is disabled (DMA direct mode), the source and
    // destination transfer widths are equal and both are defined
    // by PSIZE (MSIZE is ignored).
    .FIFOMode = DMA_FIFOMODE_DISABLE,
    .FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL,
    // Note MBURST and PBURST are ignored in direct mode.
    .MemBurst = DMA_MBURST_SINGLE,
    .PeriphBurst = DMA_PBURST_SINGLE,
    #else
    #error Unsupported MCU
    #endif
};

#if defined(OMV_SAI)
const DMA_InitTypeDef stm_dma_sai_init = {
    .PeriphInc = DMA_PINC_DISABLE,
    .MemInc = DMA_MINC_ENABLE,
    .Priority = DMA_PRIORITY_HIGH,
    .FIFOMode = DMA_FIFOMODE_ENABLE,
    .FIFOThreshold = DMA_FIFO_THRESHOLD_FULL,
    .MemBurst = DMA_MBURST_SINGLE,
    .PeriphBurst = DMA_PBURST_SINGLE,
};
#endif

#if defined(OMV_DFSDM)
const DMA_InitTypeDef stm_dma_dfsdm_init = {
    .PeriphInc = DMA_PINC_DISABLE,
    .MemInc = DMA_MINC_ENABLE,
    .Priority = DMA_PRIORITY_HIGH,
    .FIFOMode = DMA_FIFOMODE_DISABLE,   // Note: wasn't set
    .FIFOThreshold = DMA_FIFO_THRESHOLD_FULL,
    .MemBurst = DMA_MBURST_SINGLE,
    .PeriphBurst = DMA_PBURST_SINGLE,
};
#endif

#if defined(OMV_MDF)
const DMA_InitTypeDef stm_dma_mdf_init = {
    .BlkHWRequest = DMA_BREQ_SINGLE_BURST,
    .Priority = DMA_HIGH_PRIORITY,
    .SrcBurstLength = 1,
    .DestBurstLength = 1,
    .TransferEventMode = DMA_TCEM_BLOCK_TRANSFER,
};
#endif

uint8_t stm_dma_channel_to_irqn(void *dma_channel) {
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
uint8_t stm_dma_channel_to_id(void *dma_channel) {
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

int stm_dma_set_irq_descr(void *dma_channel, DMA_HandleTypeDef *dma_descr) {
    uint8_t dma_id = stm_dma_channel_to_id(dma_channel);
    if (dma_id != -1) {
        dma_handle[dma_id] = dma_descr;
        return 0;
    }
    return -1;
}

uint8_t stm_dma_mpu_region_size(uint32_t size) {
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

#if defined(HPDMA1_Channel0)
static bool stm_dma_is_hp_channel(void *dma_channel) {
    return ((((uint32_t) dma_channel) & 0xFFFFF000) == HPDMA1_BASE);
}
#endif

static uint32_t stm_dma_width(uint32_t size, bool source) {
    #if defined(STM32N6)
    switch(size) {
        case 1: return (source) ? DMA_SRC_DATAWIDTH_BYTE : DMA_DEST_DATAWIDTH_BYTE;
        case 2: return (source) ? DMA_SRC_DATAWIDTH_HALFWORD : DMA_DEST_DATAWIDTH_HALFWORD;
        case 4: return (source) ? DMA_SRC_DATAWIDTH_WORD : DMA_DEST_DATAWIDTH_WORD;
        case 8: return (source) ? DMA_SRC_DATAWIDTH_DOUBLEWORD : DMA_DEST_DATAWIDTH_DOUBLEWORD;
        default: return -1;
    }
    #else
    switch(size) {
        case 1: return (source) ? DMA_PDATAALIGN_BYTE : DMA_MDATAALIGN_BYTE;
        case 2: return (source) ? DMA_PDATAALIGN_HALFWORD : DMA_MDATAALIGN_HALFWORD;
        case 4: return (source) ? DMA_PDATAALIGN_WORD : DMA_MDATAALIGN_WORD;
        default: return -1;
    }
    #endif
}

#if defined(STM32N6)
static int stm_dma_sec_config(DMA_HandleTypeDef *dma_descr) {
    // Configure default security attributes.
    uint32_t chan_flags = DMA_CHANNEL_PRIV | DMA_CHANNEL_SEC |
                          DMA_CHANNEL_SRC_SEC | DMA_CHANNEL_DEST_SEC;

    if (HAL_DMA_ConfigChannelAttributes(dma_descr, chan_flags) != HAL_OK) {
        return -1;
    }

    // Enable isolation for HPDMA channels.
    if (stm_dma_is_hp_channel(dma_descr->Instance)) {
        DMA_IsolationConfigTypeDef isocfg = {
            .CidFiltering =  DMA_ISOLATION_ON,
            .StaticCid = DMA_CHANNEL_STATIC_CID_1,
        };

        if (HAL_DMA_SetIsolationAttributes(dma_descr, &isocfg) != HAL_OK) {
            return -1;
        }
    }

    return 0;
}
#endif

int stm_dma_init(DMA_HandleTypeDef *dma_descr, void *dma_channel, uint32_t request,
                 uint32_t direction, uint32_t ssize, uint32_t dsize, uint32_t ports,
                 const DMA_InitTypeDef *init, bool circular) {
    bool dma_init_done = true;
    DMA_InitTypeDef *dma_init = &dma_descr->Init;

    // Set channel
    dma_descr->Instance = dma_channel;

    // Copy static init.
    memcpy(dma_init, init, sizeof(DMA_InitTypeDef));

    // Set request
    #if defined(STM32H7) || defined(STM32N6)
    dma_init->Request = request;
    #else
    dma_init->Channel = request;
    #endif
    
    // Set direction
    dma_init->Direction = direction;

    // Set src/dest increment
    #if defined(STM32N6)
    if (direction == DMA_PERIPH_TO_MEMORY) {
        dma_init->SrcInc = DMA_SINC_FIXED;
        dma_init->DestInc = DMA_DINC_INCREMENTED;
    } else {
        dma_init->SrcInc = DMA_SINC_INCREMENTED;
        dma_init->DestInc = DMA_DINC_FIXED;
    }
    #endif

    // Configure src/dest size/alignment
    #if defined(STM32N6)
    dma_init->SrcDataWidth = stm_dma_width(ssize, true);
    dma_init->DestDataWidth = stm_dma_width(dsize, false);
    #else
    dma_init->PeriphDataAlignment = stm_dma_width(ssize, true);
    dma_init->MemDataAlignment = stm_dma_width(dsize, false);
    #endif

    // Set mode.
    #if defined(STM32N6)
    dma_init->Mode = DMA_NORMAL;
    #else
    dma_init->Mode = circular ? DMA_CIRCULAR : DMA_NORMAL;
    #endif

    // Set allocated ports.
    #if defined(STM32N6)
    dma_init->TransferAllocatedPort = ports;
    #endif    

    // F4, F7, H7 or N6 in non-circular mode.
    #if defined(STM32N6)
    dma_init_done = !circular;
    #endif

    if (dma_init_done) {
        HAL_DMA_DeInit(dma_descr);
        if (HAL_DMA_Init(dma_descr) != HAL_OK) {
            return -1;
        }

        #if defined(STM32N6)
        if (stm_dma_sec_config(dma_descr)) {
            return -1;
        }
        #endif
    }
    return 0;
}

#if defined(STM32N6)
int stm_dma_ll_init(DMA_HandleTypeDef *dma_descr, DMA_QListTypeDef *dma_queue,
        DMA_NodeTypeDef *dma_nodes, size_t nodes_count, uint32_t ports) {
    bool is_hp = stm_dma_is_hp_channel(dma_descr->Instance);

    DMA_NodeConfTypeDef node_conf = {
        .SrcSecure = DMA_CHANNEL_SRC_SEC,
        .DestSecure = DMA_CHANNEL_DEST_SEC,
        .DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE,
        .DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED,
        .TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED,
        .NodeType = is_hp ? DMA_HPDMA_LINEAR_NODE : DMA_GPDMA_LINEAR_NODE,
    };

    // Copy Node DMA init.
    memcpy(&node_conf.Init, &dma_descr->Init, sizeof(DMA_InitTypeDef));

    // Clear DMA queue and node(s).
    memset(dma_queue, 0, sizeof(DMA_QListTypeDef));
    memset(dma_nodes, 0, sizeof(DMA_NodeTypeDef) * nodes_count);

    DMA_NodeTypeDef *prev_node = NULL;
    for (size_t i=0; i<nodes_count; i++) {
        if (HAL_DMAEx_List_BuildNode(&node_conf, &dma_nodes[i]) != HAL_OK ||
            HAL_DMAEx_List_InsertNode(dma_queue, prev_node, &dma_nodes[i]) != HAL_OK) {
            return -1;
        }
        prev_node = &dma_nodes[i];
    }

    if (HAL_DMAEx_List_SetCircularMode(dma_queue) != HAL_OK) {
        return -1;
    }

    dma_descr->InitLinkedList.Priority = DMA_HIGH_PRIORITY;
    dma_descr->InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    dma_descr->InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    dma_descr->InitLinkedList.LinkAllocatedPort = ports;
    dma_descr->InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;

    if (HAL_DMAEx_List_Init(dma_descr) != HAL_OK ||
        HAL_DMAEx_List_LinkQ(dma_descr, dma_queue) != HAL_OK) {
        return -1;
    }

    if (stm_dma_sec_config(dma_descr)) {
        return -1;
    }

    return 0;
}
#endif

#ifdef OMV_MDMA_CHANNEL_DCMI_0
void stm_mdma_init(omv_csi_t *csi, uint32_t bytes_per_pixel, uint32_t x_crop) {
    framebuffer_t *fb = csi->fb;

    stm_mdma_init_channel(csi, &csi->mdma0.Init, bytes_per_pixel, x_crop);
    memcpy(&csi->mdma1.Init, &csi->mdma0.Init, sizeof(MDMA_InitTypeDef));
    HAL_MDMA_Init(&csi->mdma0);

    // If we are not transposing the image we can fully offload image capture from the CPU.
    if (!csi->transpose) {
        // MDMA will trigger on each TC from DMA and transfer one line to the frame buffer.
        csi->mdma1.Init.Request = MDMA_REQUEST_DMA2_Stream1_TC;
        csi->mdma1.Init.TransferTriggerMode = MDMA_BLOCK_TRANSFER;
        // We setup MDMA to repeatedly reset itself to transfer the same line buffer.
        csi->mdma1.Init.SourceBlockAddressOffset = -(fb->u * bytes_per_pixel);
    }

    HAL_MDMA_Init(&csi->mdma1);
    if (!csi->transpose) {
        HAL_MDMA_ConfigPostRequestMask(&csi->mdma1, (uint32_t) &DMA2->LIFCR, DMA_FLAG_TCIF1_5);
    }
}

// Configures an MDMA channel to completely offload the CPU in copying one line of pixels.
void stm_mdma_init_channel(omv_csi_t *csi, MDMA_InitTypeDef *init, uint32_t bytes_per_pixel, uint32_t x_crop) {
    framebuffer_t *fb = csi->fb;

    init->Request = MDMA_REQUEST_SW;
    init->TransferTriggerMode = MDMA_REPEAT_BLOCK_TRANSFER;
    init->Priority = MDMA_PRIORITY_VERY_HIGH;
    init->DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    init->BufferTransferLength = MDMA_BUFFER_SIZE;
    // The source address is 1KB aligned. So, a burst size of 16 beats
    // (AHB Max) should not break. Destination lines may not be aligned
    // however so the burst size must be computed.
    init->SourceBurst = MDMA_SOURCE_BURST_16BEATS;
    init->SourceBlockAddressOffset = 0;
    init->DestBlockAddressOffset = 0;

    if ((csi->pixformat == PIXFORMAT_RGB565 && csi->rgb_swap) ||
        (csi->pixformat == PIXFORMAT_YUV422 && csi->yuv_swap)) {
        init->Endianness = MDMA_LITTLE_BYTE_ENDIANNESS_EXCHANGE;
    } else {
        init->Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    }

    uint32_t line_offset_bytes = (fb->x * bytes_per_pixel) - x_crop;
    uint32_t line_width_bytes = fb->u * bytes_per_pixel;

    if (csi->transpose) {
        line_width_bytes = bytes_per_pixel;
        init->DestBlockAddressOffset = (fb->v - 1) * bytes_per_pixel;
    }

    // YUV422 Source -> Y Destination
    if ((csi->pixformat == PIXFORMAT_GRAYSCALE) && (csi->mono_bpp == 2)) {
        line_width_bytes /= 2;
        if (csi->transpose) {
            init->DestBlockAddressOffset /= 2;
        }
    }

    // The destination will be 32-byte aligned, so the line width is broken
    // into the largest power of 2.  The source may have an offset, further
    // limiting this to a sub power of 2.
    for (int i = 3; i >= 0; i--) {
        if (!(line_width_bytes % (1 << i))) {
            for (int j = IM_MIN(i, 2); j >= 0; j--) {
                if (!(line_offset_bytes % (1 << j))) {
                    init->SourceInc = MDMA_CTCR_SINC_1 | (j << MDMA_CTCR_SINCOS_Pos);
                    init->SourceDataSize = j << MDMA_CTCR_SSIZE_Pos;
                    break;
                }
            }

            init->DestinationInc = MDMA_CTCR_DINC_1 | (i << MDMA_CTCR_DINCOS_Pos);
            init->DestDataSize = i << MDMA_CTCR_DSIZE_Pos;

            // Find the burst size we can break the destination transfer up into.
            uint32_t count = MDMA_BUFFER_SIZE >> i;

            for (int i = 7; i >= 0; i--) {
                if (!(count % (1 << i))) {
                    init->DestBurst = i << MDMA_CTCR_DBURST_Pos;
                    break;
                }
            }

            break;
        }
    }

    // YUV422 Source -> Y Destination
    if ((csi->pixformat == PIXFORMAT_GRAYSCALE) && (csi->mono_bpp == 2)) {
        init->SourceInc = MDMA_SRC_INC_HALFWORD;
        init->SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    }
}

void stm_mdma_start(omv_csi_t *csi, uint32_t src, uint32_t dst, uint32_t line_width, uint32_t line_count) {
    // mdma0 will copy this line of the image to the final destination.
    __HAL_UNLOCK(&csi->mdma0);
    csi->mdma0.State = HAL_MDMA_STATE_READY;
    HAL_MDMA_Start(&csi->mdma0, src, dst, line_width, 1);

    // mdma1 will copy all remaining lines of the image to the final destination.
    __HAL_UNLOCK(&csi->mdma1);
    csi->mdma1.State = HAL_MDMA_STATE_READY;
    HAL_MDMA_Start(&csi->mdma1, src, dst + line_width, line_width, line_count - 1);
}

int omv_csi_dma_memcpy(omv_csi_t *csi, void *dma, void *dst, void *src, int bpp, bool transposed) {
    framebuffer_t *fb = csi->fb;
    MDMA_HandleTypeDef *handle = dma;

    // Drop the frame if MDMA is not keeping up as the image will be corrupted.
    if (handle->Instance->CCR & MDMA_CCR_EN) {
        csi->drop_frame = true;
        return 0;
    }

    // If MDMA is still running, HAL_MDMA_Start() will start a new transfer.
    __HAL_UNLOCK(handle);
    handle->State = HAL_MDMA_STATE_READY;
    HAL_MDMA_Start(handle,
                   (uint32_t) src,
                   (uint32_t) dst,
                   transposed ? bpp : (fb->u * bpp),
                   transposed ? fb->u : 1);
    return 0;
}
#endif  // OMV_MDMA_CHANNEL_DCMI_0

#if defined(GPDMA1)
static inline void stm_dma_irq_handler(size_t irqn) {
    if (dma_handle[irqn] != NULL) {
        HAL_DMA_IRQHandler(dma_handle[irqn]);
    }
}

void GPDMA1_Channel0_IRQHandler(void) {
    stm_dma_irq_handler(0);
}

void GPDMA1_Channel1_IRQHandler(void) {
    stm_dma_irq_handler(1);
}

void GPDMA1_Channel2_IRQHandler(void) {
    stm_dma_irq_handler(2);
}

void GPDMA1_Channel3_IRQHandler(void) {
    stm_dma_irq_handler(3);
}

void GPDMA1_Channel4_IRQHandler(void) {
    stm_dma_irq_handler(4);
}

void GPDMA1_Channel5_IRQHandler(void) {
    stm_dma_irq_handler(5);
}

void GPDMA1_Channel6_IRQHandler(void) {
    stm_dma_irq_handler(6);
}

void GPDMA1_Channel7_IRQHandler(void) {
    stm_dma_irq_handler(7);
}

void GPDMA1_Channel8_IRQHandler(void) {
    stm_dma_irq_handler(8);
}

void GPDMA1_Channel9_IRQHandler(void) {
    stm_dma_irq_handler(9);
}

void GPDMA1_Channel10_IRQHandler(void) {
    stm_dma_irq_handler(10);
}

void GPDMA1_Channel11_IRQHandler(void) {
    stm_dma_irq_handler(11);
}

void GPDMA1_Channel12_IRQHandler(void) {
    stm_dma_irq_handler(12);
}

void GPDMA1_Channel13_IRQHandler(void) {
    stm_dma_irq_handler(13);
}

void GPDMA1_Channel14_IRQHandler(void) {
    stm_dma_irq_handler(14);
}

void GPDMA1_Channel15_IRQHandler(void) {
    stm_dma_irq_handler(15);
}
#endif // GPDMA1

#if defined(HPDMA1)
void HPDMA1_Channel0_IRQHandler(void) {
    stm_dma_irq_handler(16);
}

void HPDMA1_Channel1_IRQHandler(void) {
    stm_dma_irq_handler(17);
}

void HPDMA1_Channel2_IRQHandler(void) {
    stm_dma_irq_handler(18);
}

void HPDMA1_Channel3_IRQHandler(void) {
    stm_dma_irq_handler(19);
}

void HPDMA1_Channel4_IRQHandler(void) {
    stm_dma_irq_handler(20);
}

void HPDMA1_Channel5_IRQHandler(void) {
    stm_dma_irq_handler(21);
}

void HPDMA1_Channel6_IRQHandler(void) {
    stm_dma_irq_handler(22);
}

void HPDMA1_Channel7_IRQHandler(void) {
    stm_dma_irq_handler(23);
}

void HPDMA1_Channel8_IRQHandler(void) {
    stm_dma_irq_handler(24);
}

void HPDMA1_Channel9_IRQHandler(void) {
    stm_dma_irq_handler(25);
}

void HPDMA1_Channel10_IRQHandler(void) {
    stm_dma_irq_handler(26);
}

void HPDMA1_Channel11_IRQHandler(void) {
    stm_dma_irq_handler(27);
}

void HPDMA1_Channel12_IRQHandler(void) {
    stm_dma_irq_handler(28);
}

void HPDMA1_Channel13_IRQHandler(void) {
    stm_dma_irq_handler(29);
}

void HPDMA1_Channel14_IRQHandler(void) {
    stm_dma_irq_handler(30);
}

void HPDMA1_Channel15_IRQHandler(void) {
    stm_dma_irq_handler(31);
}
#endif
