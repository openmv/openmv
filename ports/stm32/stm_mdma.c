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
 * STM32 MDMA utils.
 */
#include <stdint.h>
#include STM32_HAL_H
#include "omv_csi.h"
#include "stm_mdma.h"

#ifdef OMV_MDMA_CHANNEL_DCMI_0
void omv_csi_mdma_irq_handler(void) {
    omv_csi_t *csi = omv_csi_get(-1);

    if (MDMA->GISR0 & (1 << OMV_MDMA_CHANNEL_DCMI_0)) {
        HAL_MDMA_IRQHandler(&csi->mdma0);
    }
    if (MDMA->GISR0 & (1 << OMV_MDMA_CHANNEL_DCMI_1)) {
        HAL_MDMA_IRQHandler(&csi->mdma1);
    }
}

static void stm_mdma_init_channel(omv_csi_t *csi,
        MDMA_InitTypeDef *init, uint32_t bytes_per_pixel, uint32_t x_crop);

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

// Configures an MDMA channel to completely offload the CPU in copying one line of pixels.
static void stm_mdma_init_channel(omv_csi_t *csi,
        MDMA_InitTypeDef *init, uint32_t bytes_per_pixel, uint32_t x_crop) {
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
#endif  // OMV_MDMA_CHANNEL_DCMI_0
