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
 * STM32 CSI driver.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "py/mphal.h"
#include "irq.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "stm_dma.h"
#include "stm_isp.h"
#include "stm_pwm.h"

#if defined(DCMIPP)
#define USE_DCMIPP          (1)
// NOTE using PIPE1.
#define DCMIPP_PIPE         (DCMIPP_PIPE1)
#endif

#if defined(PSSI)
#define DCMI_IRQn           DCMI_PSSI_IRQn
#define DCMI_IRQHandler     DCMI_PSSI_IRQHandler
#define DMA_PRIORITY_HIGH   DMA_HIGH_PRIORITY
#endif

#if defined(OMV_MDMA_CHANNEL_DCMI_0)
#define USE_MDMA            (1)
#endif

#ifndef OMV_CSI_DMA_XFER_PORTS
#define OMV_CSI_DMA_XFER_PORTS  (0)
#endif

#ifndef OMV_CSI_DMA_MAX_SIZE
#define OMV_CSI_DMA_MAX_SIZE    (0xFFFFU * 4U)
#endif

#ifndef OMV_CSI_LINE_ALIGNMENT
#define OMV_CSI_LINE_ALIGNMENT  (16)
#endif

typedef enum {
    CSI_HANDLE_DCMI = 0,
    CSI_HANDLE_DCMIPP = 1,
} csi_handle_t;

extern uint8_t _line_buf;
// Stores the CSI handle associated with DCMI/DCMIPP.
static omv_csi_t *stm_csi_all[2] = { 0 };

#if defined(STM32N6)
static DMA_QListTypeDef dma_queue;
// Nodes can't be places in CSI state because the need to be uncacheable.
static DMA_NodeTypeDef OMV_ATTR_SECTION(dma_nodes[2], ".dma_buffer");
#endif

void DCMI_IRQHandler(void) {
    omv_csi_t *csi = stm_csi_all[CSI_HANDLE_DCMI];
    HAL_DCMI_IRQHandler(&csi->dcmi);
}

#if USE_DCMIPP
void CSI_IRQHandler(void) {
    omv_csi_t *csi = stm_csi_all[CSI_HANDLE_DCMIPP];
    HAL_DCMIPP_CSI_IRQHandler(&csi->dcmipp);
}

void DCMIPP_IRQHandler(void) {
    omv_csi_t *csi = stm_csi_all[CSI_HANDLE_DCMIPP];
    HAL_DCMIPP_IRQHandler(&csi->dcmipp);
}
#endif

#if USE_MDMA
void omv_csi_mdma_irq_handler(void) {
    omv_csi_t *csi = stm_csi_all[CSI_HANDLE_DCMI];

    if (MDMA->GISR0 & (1 << OMV_MDMA_CHANNEL_DCMI_0)) {
        HAL_MDMA_IRQHandler(&csi->mdma0);
    }
    if (MDMA->GISR0 & (1 << OMV_MDMA_CHANNEL_DCMI_1)) {
        HAL_MDMA_IRQHandler(&csi->mdma1);
    }
}
#endif

static bool stm_csi_is_active(omv_csi_t *csi) {
    #if USE_DCMIPP
    if (csi->mipi_if) {
        return (DCMIPP->P1FCTCR & DCMIPP_P1FCTCR_CPTREQ);
    }
    #endif
    return (DCMI->CR & DCMI_CR_ENABLE);
}

static int stm_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        if (!csi->mipi_if) {
            // Configure and initialize DMA.
            if (stm_dma_init(&csi->dma, OMV_CSI_DMA_CHANNEL, OMV_CSI_DMA_REQUEST,
                        DMA_PERIPH_TO_MEMORY, 4, 4, OMV_CSI_DMA_XFER_PORTS,
                        &stm_dma_csi_init, true)) {
                return OMV_CSI_ERROR_DMA_INIT_FAILED;
            }

            #if defined(STM32N6)
            // Initialize DMA in circular mode.
            if (stm_dma_ll_init(&csi->dma, &dma_queue, dma_nodes,
                        OMV_ARRAY_SIZE(dma_nodes), OMV_CSI_DMA_LIST_PORTS)) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
            #endif

            // Set DMA IRQ handle
            stm_dma_set_irq_descr(OMV_CSI_DMA_CHANNEL, &csi->dma);

            // Configure the DMA IRQ Channel
            csi->dma_irqn = stm_dma_channel_to_irqn(OMV_CSI_DMA_CHANNEL);
            NVIC_SetPriority(csi->dma_irqn, IRQ_PRI_DMA21);

            #if USE_MDMA
            csi->mdma0.Instance = MDMA_CHAN_TO_INSTANCE(OMV_MDMA_CHANNEL_DCMI_0);
            csi->mdma1.Instance = MDMA_CHAN_TO_INSTANCE(OMV_MDMA_CHANNEL_DCMI_1);
            #endif

            csi->dcmi.Instance = DCMI;
            csi->dcmi.Init.VSPolarity = csi->vsync_pol ? DCMI_VSPOLARITY_HIGH : DCMI_VSPOLARITY_LOW;
            csi->dcmi.Init.HSPolarity = csi->hsync_pol ? DCMI_HSPOLARITY_HIGH : DCMI_HSPOLARITY_LOW;
            csi->dcmi.Init.PCKPolarity = csi->pixck_pol ? DCMI_PCKPOLARITY_RISING : DCMI_PCKPOLARITY_FALLING;
            csi->dcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
            csi->dcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
            csi->dcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
            csi->dcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;

            // Link the DMA handle to the DCMI handle
            __HAL_LINKDMA(&csi->dcmi, DMA_Handle, csi->dma);

            // Initialize the DCMI
            HAL_DCMI_DeInit(&csi->dcmi);
            if (HAL_DCMI_Init(&csi->dcmi) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            // Store CSI handle used for DCMI
            stm_csi_all[CSI_HANDLE_DCMI] = csi;

            // Configure and enable DCMI IRQ Channel
            NVIC_SetPriority(DCMI_IRQn, IRQ_PRI_DCMI);
            HAL_NVIC_EnableIRQ(DCMI_IRQn);
        } else {
            #if USE_DCMIPP
            // Initialize the DCMIPP
            csi->dcmipp.Instance = DCMIPP;

            HAL_DCMIPP_DeInit(&csi->dcmipp);
            if (HAL_DCMIPP_Init(&csi->dcmipp) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            // Select and configure the DCMIPP source.
            DCMIPP_CSI_ConfTypeDef scfg = {
                .NumberOfLanes = DCMIPP_CSI_TWO_DATA_LANES,
                .DataLaneMapping = DCMIPP_CSI_PHYSICAL_DATA_LANES,
                .PHYBitrate = (csi->mipi_brate == 850) ? DCMIPP_CSI_PHY_BT_850 : DCMIPP_CSI_PHY_BT_1200,
            };

            if (HAL_DCMIPP_CSI_SetConfig(&csi->dcmipp, &scfg) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            // Configure CSI virtual channel and pipe.
            DCMIPP_CSI_PIPE_ConfTypeDef csi_pcfg = {
                .DataTypeMode = DCMIPP_DTMODE_DTIDA,
                .DataTypeIDA = DCMIPP_DT_RAW10,
                .DataTypeIDB = DCMIPP_DT_RAW10,
            };

            if (HAL_DCMIPP_CSI_SetVCConfig(&csi->dcmipp, DCMIPP_VIRTUAL_CHANNEL0,
                        DCMIPP_CSI_DT_BPP10) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            if (HAL_DCMIPP_CSI_PIPE_SetConfig(&csi->dcmipp, DCMIPP_PIPE, &csi_pcfg) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            // Store CSI handle used for DCMIPP
            stm_csi_all[CSI_HANDLE_DCMIPP] = csi;

            // Configure and enable DCMI IRQ Channel
            NVIC_SetPriority(DCMIPP_IRQn, IRQ_PRI_DCMI);
            HAL_NVIC_EnableIRQ(DCMIPP_IRQn);

            // Configure and enable CSI IRQ Channel
            NVIC_SetPriority(CSI_IRQn, IRQ_PRI_DCMI);
            HAL_NVIC_EnableIRQ(CSI_IRQn);
            #endif
        }
    } else if (config == OMV_CSI_CONFIG_DEINIT) {
        if (!csi->mipi_if) {
            HAL_NVIC_DisableIRQ(DCMI_IRQn);
            HAL_DCMI_DeInit(&csi->dcmi);
        } else {
            #if USE_DCMIPP
            HAL_NVIC_DisableIRQ(DCMIPP_IRQn);
            HAL_DCMIPP_DeInit(&csi->dcmipp);
            #endif
        }
    } else if (config == OMV_CSI_CONFIG_PIXFORMAT) {
        if (!csi->mipi_if) {
            DCMI->CR &= ~(DCMI_CR_JPEG_Msk << DCMI_CR_JPEG_Pos);
            DCMI->CR |= (csi->pixformat == PIXFORMAT_JPEG) ? DCMI_JPEG_ENABLE : DCMI_JPEG_DISABLE;
        } else {
            #if USE_DCMIPP
            csi->dcmipp.State = HAL_DCMIPP_STATE_READY;
            // Reset pipes states to allow reconfiguring them.
            for (size_t i=0; i<DCMIPP_NUM_OF_PIPES; i++) {
                csi->dcmipp.PipeState[i] = HAL_DCMIPP_PIPE_STATE_RESET;
            }
            // Configure the pixel processing pipeline.
            if (stm_isp_config_pipeline(&csi->dcmipp, DCMIPP_PIPE, csi->pixformat, csi->raw_output)) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
            #endif
        }
    }

    return 0;
}

// Stop the DCMI from generating more DMA requests, and disable the DMA.
static int stm_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    csi->dma_size = 0;

    if (!stm_csi_is_active(csi)) {
        return 0;
    }

    if (!csi->mipi_if) {
        DCMI->CR &= ~DCMI_CR_ENABLE;
        while (DCMI->CR & DCMI_CR_ENABLE);

        #if defined(STM32N6)
        HAL_DMA_Abort(&csi->dma);
        #else
        if (in_irq) {
            HAL_DMA_Abort_IT(&csi->dma);
        } else {
            HAL_DMA_Abort(&csi->dma);
        }
        #endif
        HAL_NVIC_DisableIRQ(csi->dma_irqn);

        #if USE_MDMA
        if (!in_irq) {
            HAL_MDMA_Abort(&csi->mdma0);
            HAL_MDMA_Abort(&csi->mdma1);
        }
        HAL_MDMA_DeInit(&csi->mdma0);
        HAL_MDMA_DeInit(&csi->mdma1);
        #endif

        __HAL_DCMI_DISABLE_IT(&csi->dcmi, DCMI_IT_FRAME);
        __HAL_DCMI_CLEAR_FLAG(&csi->dcmi, DCMI_FLAG_FRAMERI);
    } else {
        #if USE_DCMIPP
        HAL_DCMIPP_CSI_PIPE_Stop(&csi->dcmipp, DCMIPP_PIPE, DCMIPP_VIRTUAL_CHANNEL0);
        #endif // USE_DCMIPP
    }
    return 0;
}

static int stm_csi_shutdown(omv_csi_t *csi, int enable) {
    int ret = 0;
    if (enable) {
        ret = omv_csi_config(csi, OMV_CSI_CONFIG_DEINIT);
    } else {
        ret = omv_csi_config(csi, OMV_CSI_CONFIG_INIT);
    }
    return ret;
}

static uint32_t stm_clk_get_frequency(omv_clk_t *clk) {
    if (!clk->tim.Instance) {
        return 0;
    }
    return stm_pwm_get_frequency(&clk->tim, OMV_CSI_TIM_CHANNEL);
}

static int stm_clk_set_frequency(omv_clk_t *clk, uint32_t frequency) {
    #if (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_MCO)
    // Pass through the MCO1 clock with source input set to HSE (12MHz).
    // Note MCO1 is multiplexed on OPENMV2/TIM1 only.
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
    #elif (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_OSC)
    // An external oscillator is used for the csi clock.
    // Configure and enable external oscillator if needed.
    #elif (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_TIM)
    if (stm_pwm_start(&clk->tim, OMV_CSI_TIM, OMV_CSI_TIM_CHANNEL, frequency)) {
        return OMV_CSI_ERROR_TIM_INIT_FAILED;
    }
    #else
    #error "OMV_CSI_CLK_SOURCE is not set!"
    #endif // (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_TIM)
    return 0;
}

int omv_csi_set_vsync_callback(omv_csi_t *csi, omv_csi_cb_t cb) {
    if (cb.fun == NULL) {
        #if (DCMI_VSYNC_EXTI_SHARED == 0)
        // Disable VSYNC EXTI IRQ
        omv_gpio_irq_enable(OMV_CSI_VSYNC_PIN, false);
        #endif
    } else {
        // Enable VSYNC EXTI IRQ
        omv_gpio_irq_register(OMV_CSI_VSYNC_PIN, cb.fun, cb.arg);
        omv_gpio_irq_enable(OMV_CSI_VSYNC_PIN, true);
    }
    return 0;
}

// If the image is cropped by more than 1 word in width, align the line start to a word
// address to improve copy performance. Do not crop by more than 1 word as this will
// result in less time between DMA transfers complete interrupts on 16-byte boundaries.
static uint32_t get_dcmi_hw_crop(omv_csi_t *csi, uint32_t bytes_per_pixel) {
    framebuffer_t *fb = csi->fb;
    uint32_t byte_x_offset = (fb->x * bytes_per_pixel) % 4;
    uint32_t width_remainder = (csi->resolution[csi->framesize][0] - (fb->x + fb->u)) * bytes_per_pixel;

    if (byte_x_offset && (width_remainder >= (4 - byte_x_offset))) {
        return byte_x_offset;
    }

    return 0;
}

static void stm_csi_frame_event(omv_csi_t *csi, uint32_t pipe) {
    framebuffer_t *fb = csi->fb;

    #if USE_MDMA
    // Clear out any stale flags.
    DMA2->LIFCR = DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5;
    // Re-enable the DMA IRQ to catch the next start line.
    HAL_NVIC_EnableIRQ(csi->dma_irqn);
    #endif

    csi->first_line = false;
    if (csi->drop_frame) {
        csi->drop_frame = false;
        // Reset the buffer's state if the frame was dropped.
        vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);
        framebuffer_reset(buffer);
        return;
    }

    // Release the buffer from free queue -> used queue.
    framebuffer_release(fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);

    if (csi->frame_cb.fun) {
        csi->frame_cb.fun(csi->frame_cb.arg);
    }

    #if defined(STM32N6)
    // Acquire a buffer from the free queue.
    vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);

    if (buffer == NULL) {
        omv_csi_abort(csi, false, false);
    } else if (csi->mipi_if) {
        HAL_DCMIPP_PIPE_SetMemoryAddress(&csi->dcmipp, pipe,
                                         DCMIPP_MEMORY_ADDRESS_0, (uint32_t) buffer->data);
    } else if (csi->one_shot) {
        HAL_DCMI_Stop(&csi->dcmi);
        HAL_DCMI_Start_DMA(&csi->dcmi, DCMI_MODE_SNAPSHOT, (uint32_t) buffer->data, csi->dma_size);
    }
    #endif  // STM32N6
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
    stm_csi_frame_event(OMV_CONTAINER_OF(hdcmi, omv_csi_t, dcmi), 0);
}

#if USE_DCMIPP
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *hdcmi, uint32_t pipe) {
    stm_csi_frame_event(OMV_CONTAINER_OF(hdcmi, omv_csi_t, dcmipp), pipe);
}
#endif

#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
// This function is called after each transfer is complete,
// with a pointer to the buffer that was used.
void DCMI_DMAConvCpltUser(DCMI_HandleTypeDef *hdcmi, uint32_t addr) {
    omv_csi_t *csi = OMV_CONTAINER_OF(hdcmi, omv_csi_t, dcmi);
    framebuffer_t *fb = csi->fb;

    // Throttle frames to match the current frame rate.
    omv_csi_throttle_framerate(csi);

    if (csi->drop_frame) {
        #if USE_MDMA
        if (!csi->transpose) {
            HAL_NVIC_DisableIRQ(csi->dma_irqn);
        }
        #endif
        return;
    }

    // Acquire a buffer from the free queue.
    vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);
    if (buffer == NULL) {
        omv_csi_abort(csi, false, true);
        return;
    }

    if (csi->pixformat == PIXFORMAT_JPEG) {
        if (csi->jpg_format == 3) {
            // JPEG MODE 3: Variable line width per frame, with the last line
            // potentially shorter and no padding. `offset` is incremented once
            // every max transfer, and the DMA counter holds the total size.
            buffer->offset += 1;
        } else if (csi->jpg_format == 4) {
            // JPEG MODE 4: Fixed width and height per frame. Each line starts
            // with two bytes indicating valid data length, followed by image
            // data and optional padding (0xFF). `offset` holds the total size.
            uint16_t size = __REV16(*((uint16_t *) addr));
            if (buffer->offset + size > framebuffer_get_buffer_size(fb)) {
                buffer->flags |= VB_FLAG_OVERFLOW;
                return;
            }
            unaligned_memcpy(buffer->data + buffer->offset, ((uint16_t *) addr) + 1, size);
            buffer->offset += size;
        }
        return;
    }

    #if USE_MDMA
    // DCMI_DMAConvCpltUser is called with the other MAR register.
    // So, we have to fix the address in full MDMA offload mode.
    if (!csi->transpose) {
        addr = (uint32_t) &_line_buf;
    }
    #endif

    uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
    uint8_t *src = ((uint8_t *) addr) + (fb->x * bytes_per_pixel) - get_dcmi_hw_crop(csi, bytes_per_pixel);
    uint8_t *dst = buffer->data;

    if (csi->pixformat == PIXFORMAT_GRAYSCALE) {
        bytes_per_pixel = sizeof(uint8_t);
    }

    #if USE_MDMA
    // For non-JPEG, non-transposed modes, offload the capture to MDMA.
    // Note that MDMA is started here, not in FRAME/VSYNC callbacks, to
    // maximize the time before the frame has to be dropped.
    if (!csi->transpose) {
        stm_mdma_start(csi, (uint32_t) src, (uint32_t) dst, fb->u * bytes_per_pixel, fb->v);
        HAL_NVIC_DisableIRQ(csi->dma_irqn);
        return;
    }
    #endif

    if (!csi->transpose) {
        dst += fb->u * bytes_per_pixel * buffer->offset++;
    } else {
        dst += bytes_per_pixel * buffer->offset++;
    }

    #if USE_MDMA
    // Two MDMA channels are used to maximize the time available to finish the transfer.
    omv_csi_copy_line(csi, (buffer->offset % 2) ? &csi->mdma1 : &csi->mdma0, src, dst);
    #else
    omv_csi_copy_line(csi, NULL, src, dst);
    #endif
}
#endif  // #if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)

static int stm_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    vbuffer_t *buffer = NULL;
    framebuffer_t *fb = csi->fb;

    // Configure and re/start the capture if it's not alrady active
    // and there are no pending buffers (from non-blocking capture).
    if (!stm_csi_is_active(csi) && !framebuffer_readable(fb)) {
        // Acquire a buffer from the free queue.
        if (!(buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK))) {
            return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
        }

        if (csi->mipi_if) {
            #if USE_DCMIPP
            uint32_t bytes_per_pixel = omv_csi_get_dst_bpp(csi);
            uint32_t line_width = fb->u * bytes_per_pixel;

            if (!line_width ||
                line_width % OMV_CSI_LINE_ALIGNMENT) {
                return OMV_CSI_ERROR_INVALID_FRAMESIZE;
            }

            // Configure crop
            DCMIPP_CropConfTypeDef ccfg = {
                .HStart = fb->x,
                .VStart = fb->y,
                .HSize = fb->u,
                .VSize = fb->v,
            };
            if (HAL_DCMIPP_PIPE_SetCropConfig(&csi->dcmipp, DCMIPP_PIPE, &ccfg) != HAL_OK ||
                HAL_DCMIPP_PIPE_EnableCrop(&csi->dcmipp, DCMIPP_PIPE) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            // Set output pitch
            if (HAL_DCMIPP_PIPE_SetPitch(&csi->dcmipp, DCMIPP_PIPE, line_width) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            // Start the DCMIPP
            if (HAL_DCMIPP_CSI_PIPE_Start(&csi->dcmipp, DCMIPP_PIPE, DCMIPP_VIRTUAL_CHANNEL0,
                                          (uint32_t) buffer->data, DCMIPP_MODE_CONTINUOUS) != HAL_OK) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }
            #endif  // USE_DCMIPP
        } else {
            // Setup the size and address of the transfer
            uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
            uint32_t x_crop = get_dcmi_hw_crop(csi, bytes_per_pixel);
            uint32_t line_width = csi->resolution[csi->framesize][0] * bytes_per_pixel;

            // Shrink the captured pixel count by one word to allow cropping to fix alignment.
            if (x_crop) {
                line_width -= 4;
            }

            csi->dma_size = line_width * fb->v / 4;

            // Error out if the transfer size is not compatible with DMA transfer restrictions.
            if ((!line_width) || (line_width % 4) ||
                #if defined(OMV_LINE_BUF_SIZE)
                (line_width > (OMV_LINE_BUF_SIZE / 2)) ||
                #endif
                (!csi->dma_size) || (csi->dma_size % OMV_CSI_LINE_ALIGNMENT)) {
                return OMV_CSI_ERROR_INVALID_FRAMESIZE;
            }

            HAL_DCMI_DisableCrop(&csi->dcmi);
            if (csi->pixformat != PIXFORMAT_JPEG) {
                // Vertically crop the image. Horizontal cropping is done in software.
                HAL_DCMI_ConfigCrop(&csi->dcmi, x_crop, fb->y, line_width - 1, fb->v - 1);
                HAL_DCMI_EnableCrop(&csi->dcmi);
            }

            #if USE_MDMA
            // Configure MDMA for non-JPEG modes. MDMA will be used to either
            // completely offload the transfer, in case of non-transposed mode
            // or copy transposed lines.
            if (csi->pixformat != PIXFORMAT_JPEG) {
                stm_mdma_init(csi, bytes_per_pixel, x_crop);
            }
            #endif

            // Reset the DMA state and re-enable it.
            #if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
            OMV_CSI_DMA_CHANNEL->CR &= ~(DMA_SxCR_CIRC | DMA_SxCR_CT | DMA_SxCR_DBM);
            #endif
            HAL_NVIC_EnableIRQ(csi->dma_irqn);

            // HAL_DCMI_Start_DMA and HAL_DCMI_Start_DMA_MB both perform circular transfers,
            // differing only in size, with an interrupt after every half of the transfer.
            if ((csi->pixformat == PIXFORMAT_JPEG) && (csi->jpg_format == 3)) {
                // Start a one-shot transfer to the framebuffer, used only for JPEG mode 3.
                uint32_t size = framebuffer_get_buffer_size(fb);
                csi->dma_size = IM_MIN(size, (OMV_CSI_DMA_MAX_SIZE * 2U)) / 4;
                csi->one_shot = true;
                HAL_DCMI_Start_DMA(&csi->dcmi, DCMI_MODE_SNAPSHOT, (uint32_t) buffer->data, csi->dma_size);
            #if USE_MDMA
            } else if ((csi->pixformat != PIXFORMAT_JPEG) && (!csi->transpose)) {
                // Special transfer mode that uses DMA in circular mode and MDMA
                // to move the lines to the final destination.
                ((DMA_Stream_TypeDef *) csi->dma.Instance)->CR |= DMA_SxCR_CIRC;
                HAL_DCMI_Start_DMA(&csi->dcmi, DCMI_MODE_CONTINUOUS, (uint32_t) &_line_buf, line_width / 4);
            #endif  // USE_MDMA
            } else {
                #if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
                // Start a multibuffer (line by line) transfer.
                HAL_DCMI_Start_DMA_MB(&csi->dcmi, DCMI_MODE_CONTINUOUS, (uint32_t) &_line_buf, csi->dma_size, fb->v);
                #else
                csi->one_shot = true;
                HAL_DCMI_Start_DMA(&csi->dcmi, DCMI_MODE_SNAPSHOT, (uint32_t) buffer->data, csi->dma_size);
                #endif
            }
        }
    }

    // In JPEG mode, enable the end of frame interrupt.
    if (!csi->mipi_if && csi->pixformat == PIXFORMAT_JPEG) {
        __HAL_DCMI_ENABLE_IT(&csi->dcmi, DCMI_IT_FRAME);
    }

    // One shot DMA transfers must be invalidated.
    framebuffer_flags_t fb_flags = FB_FLAG_USED | FB_FLAG_PEEK |
                                   ((csi->one_shot) ? FB_FLAG_INVALIDATE : 0);

    // Wait for a frame to be ready.
    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        if ((buffer = framebuffer_acquire(fb, fb_flags))) {
            break;
        }

        if (flags & OMV_CSI_FLAG_NON_BLOCK) {
            return OMV_CSI_ERROR_WOULD_BLOCK;
        }

        if ((mp_hal_ticks_ms() - start) > OMV_CSI_TIMEOUT_MS) {
            omv_csi_abort(csi, true, false);
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
    }

    // In JPEG 3 mode, the transfer must be aborted as it waits for data indefinitely.
    if (!csi->mipi_if && (csi->pixformat == PIXFORMAT_JPEG) && (csi->jpg_format == 3)) {
        omv_csi_abort(csi, true, false);
    }

    // The JPEG in the framebuffer is actually invalid.
    if (buffer->flags & VB_FLAG_OVERFLOW) {
        return OMV_CSI_ERROR_JPEG_OVERFLOW;
    }

    // Set the framebuffer width/height.
    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;

    // Set the framebuffer pixel format.
    switch (csi->pixformat) {
        case PIXFORMAT_GRAYSCALE:
            fb->pixfmt = PIXFORMAT_GRAYSCALE;
            break;
        case PIXFORMAT_RGB565:
            fb->pixfmt = PIXFORMAT_RGB565;
            break;
        case PIXFORMAT_BAYER:
            fb->pixfmt = PIXFORMAT_BAYER;
            fb->subfmt_id = csi->cfa_format;
            fb->pixfmt = imlib_bayer_shift(fb->pixfmt, fb->x, fb->y, csi->transpose);
            break;
        case PIXFORMAT_YUV422: {
            fb->pixfmt = PIXFORMAT_YUV;
            fb->subfmt_id = csi->yuv_format;
            fb->pixfmt = imlib_yuv_shift(fb->pixfmt, fb->x);
            break;
        }
        case PIXFORMAT_JPEG: {
            int32_t size = 0;
            if (csi->jpg_format == 4) {
                // Offset is the total frame size.
                size = buffer->offset;
            } else {
                // Offset is the number of length-size transfers performed.
                size = buffer->offset * csi->dma_size / 2;
                // The DMA counter holds the number of bytes per transfer.
                if (!csi->mipi_if && __HAL_DMA_GET_COUNTER(&csi->dma)) {
                    // Add in the uncompleted transfer length.
                    size += ((csi->dma_size / 2) - __HAL_DMA_GET_COUNTER(&csi->dma)) * 4;
                }
            }
            // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
            fb->pixfmt = PIXFORMAT_JPEG;
            fb->size = jpeg_clean_trailing_bytes(size, buffer->data);
            break;
        }
        default:
            break;
    }

    #if USE_DCMIPP
    if (csi->raw_output) {
        float luminance = stm_isp_update_awb(&csi->dcmipp, DCMIPP_PIPE, fb->u * fb->v);
        if (csi->ioctl) {
            omv_csi_ioctl(csi, OMV_CSI_IOCTL_UPDATE_AGC_AEC, fast_floorf(luminance));
        }

    }
    #endif

    // Set the user image.
    framebuffer_init_image(fb, image);
    return 0;
}

int omv_csi_ops_init(omv_csi_t *csi) {
    // Set CSI ops.
    csi->abort = stm_csi_abort;
    csi->config = stm_csi_config;
    csi->shutdown = stm_csi_shutdown;
    csi->snapshot = stm_csi_snapshot;

    // Set CSI clock ops.
    csi->clk->freq = OMV_CSI_CLK_FREQUENCY;
    csi->clk->set_freq = stm_clk_set_frequency;
    csi->clk->get_freq = stm_clk_get_frequency;
    return 0;
}
