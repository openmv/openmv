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

#if defined(DMA2)
#define USE_DMA             (1)
#define DMA_MAX_TRANSFER    (0xFFFFU * 4U)
#endif

#if defined(OMV_MDMA_CHANNEL_DCMI_0)
#define USE_MDMA            (1)
#endif

#if !defined(DCMIPP)
#define USE_DCMI            (1)
#define DCMI_IS_ACTIVE()    (DCMI->CR & DCMI_CR_ENABLE)
#else
#define USE_DCMIPP          (1)
// NOTE using PIPE1.
#define DCMI_IS_ACTIVE()    (DCMIPP->P1FCTCR & DCMIPP_P1FCTCR_CPTREQ)
#define DCMIPP_PIPE         (DCMIPP_PIPE1)
#endif

#ifndef OMV_CSI_DMA_XFER_PORTS
#define OMV_CSI_DMA_XFER_PORTS (0)
#endif

#define LINE_WIDTH_ALIGNMENT (16)

extern uint8_t _line_buf;
extern uint32_t hal_get_exti_gpio(uint32_t line);

#if USE_DCMI
void DCMI_IRQHandler(void) {
    omv_csi_t *csi = omv_csi_get(-1);
    HAL_DCMI_IRQHandler(&csi->dcmi);
}
#endif

#if USE_DCMIPP
void CSI_IRQHandler(void) {
    omv_csi_t *csi = omv_csi_get(-1);
    HAL_DCMIPP_CSI_IRQHandler(&csi->dcmi);
}

void DCMIPP_IRQHandler(void) {
    omv_csi_t *csi = omv_csi_get(-1);
    HAL_DCMIPP_IRQHandler(&csi->dcmi);
}
#endif

#if USE_MDMA
void omv_csi_mdma_irq_handler(void) {
    omv_csi_t *csi = omv_csi_get(-1);

    if (MDMA->GISR0 & (1 << OMV_MDMA_CHANNEL_DCMI_0)) {
        HAL_MDMA_IRQHandler(&csi->mdma0);
    }
    if (MDMA->GISR0 & (1 << OMV_MDMA_CHANNEL_DCMI_1)) {
        HAL_MDMA_IRQHandler(&csi->mdma1);
    }
}
#endif

static int stm_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT) {
        #if USE_DMA

        // Configure and initialize DMA.
        if (stm_dma_init(&csi->dma, OMV_CSI_DMA_CHANNEL, OMV_CSI_DMA_REQUEST,
                         DMA_PERIPH_TO_MEMORY, 4, 4, OMV_CSI_DMA_XFER_PORTS,
                         &stm_dma_csi_init, true)) {
            return OMV_CSI_ERROR_DMA_INIT_FAILED;
        }

        // Set DMA IRQ handle
        stm_dma_set_irq_descr(OMV_CSI_DMA_CHANNEL, &csi->dma);
    
        // Configure the DMA IRQ Channel
        csi->dma_irqn = stm_dma_channel_to_irqn(OMV_CSI_DMA_CHANNEL);
        NVIC_SetPriority(csi->dma_irqn, IRQ_PRI_DMA21);

        #if USE_MDMA
        csi->mdma0.Instance = MDMA_CHAN_TO_INSTANCE(OMV_MDMA_CHANNEL_DCMI_0);
        csi->mdma1.Instance = MDMA_CHAN_TO_INSTANCE(OMV_MDMA_CHANNEL_DCMI_1);
        #endif

        #endif  // USE_DMA

        // Configure DCMI/PP.
        #if USE_DCMIPP
        // Initialize the DCMIPP
        csi->dcmi.Instance = DCMIPP;
        if (HAL_DCMIPP_Init(&csi->dcmi) != HAL_OK) {
            return -1;
        }

        // Configure and enable DCMI IRQ Channel
        NVIC_SetPriority(DCMIPP_IRQn, IRQ_PRI_DCMI);
        HAL_NVIC_EnableIRQ(DCMIPP_IRQn);

        // Configure and enable CSI IRQ Channel
        NVIC_SetPriority(CSI_IRQn, IRQ_PRI_DCMI);
        HAL_NVIC_EnableIRQ(CSI_IRQn);
        #else
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
            return -1;
        }

        // Configure and enable DCMI IRQ Channel
        NVIC_SetPriority(DCMI_IRQn, IRQ_PRI_DCMI);
        HAL_NVIC_EnableIRQ(DCMI_IRQn);
        #endif
    } else if (config == OMV_CSI_CONFIG_DEINIT) {
        #if USE_DCMI
        HAL_NVIC_DisableIRQ(DCMI_IRQn);
        HAL_DCMI_DeInit(&csi->dcmi);
        #else
        HAL_NVIC_DisableIRQ(DCMIPP_IRQn);
        HAL_DCMIPP_DeInit(&csi->dcmi);
        #endif
    } else if (config == OMV_CSI_CONFIG_PIXFORMAT) {
        #if USE_DCMI
        DCMI->CR &= ~(DCMI_CR_JPEG_Msk << DCMI_CR_JPEG_Pos);
        DCMI->CR |= (csi->pixformat == PIXFORMAT_JPEG) ? DCMI_JPEG_ENABLE : DCMI_JPEG_DISABLE;
        #else

        // Reset DCMI and pipes states to allow reconfiguring them. Note
        // that abort() doesn't reset the state unless the pipe is active.
        csi->dcmi.State = HAL_DCMIPP_STATE_INIT;
        for (size_t i=0; i<DCMIPP_NUM_OF_PIPES; i++) {
            csi->dcmi.PipeState[i] = HAL_DCMIPP_PIPE_STATE_RESET;
        }

        // Select and configure the DCMIPP source.
        if (csi->mipi_if) {
            DCMIPP_CSI_ConfTypeDef scfg = {
                .NumberOfLanes = DCMIPP_CSI_TWO_DATA_LANES,
                .DataLaneMapping = DCMIPP_CSI_PHYSICAL_DATA_LANES,
                .PHYBitrate = (csi->mipi_brate == 850) ? DCMIPP_CSI_PHY_BT_850 : DCMIPP_CSI_PHY_BT_1200,
            };
            if (HAL_DCMIPP_CSI_SetConfig(&csi->dcmi, &scfg) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
            // Configure CSI virtual channel and pipe.
            DCMIPP_CSI_PIPE_ConfTypeDef pcfg = {
                .DataTypeMode = DCMIPP_DTMODE_DTIDA,
                .DataTypeIDA = DCMIPP_DT_RAW10,
                .DataTypeIDB = DCMIPP_DT_RAW10,
            };

            if (HAL_DCMIPP_CSI_SetVCConfig(&csi->dcmi, DCMIPP_VIRTUAL_CHANNEL0,
                                           DCMIPP_CSI_DT_BPP10) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
            if (HAL_DCMIPP_CSI_PIPE_SetConfig(&csi->dcmi, DCMIPP_PIPE, &pcfg) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
        } else {
            DCMIPP_ParallelConfTypeDef scfg = {
                .SynchroMode = DCMIPP_SYNCHRO_HARDWARE,
                .ExtendedDataMode = DCMIPP_INTERFACE_8BITS,
                .VSPolarity = csi->vsync_pol ? DCMIPP_VSPOLARITY_HIGH : DCMIPP_VSPOLARITY_LOW,
                .HSPolarity = csi->hsync_pol ? DCMIPP_HSPOLARITY_HIGH : DCMIPP_HSPOLARITY_LOW,
                .PCKPolarity = csi->pixck_pol ? DCMIPP_PCKPOLARITY_RISING : DCMIPP_PCKPOLARITY_FALLING,
            };
            if (csi->raw_output) {
                scfg.Format = DCMIPP_FORMAT_RAW8;
            } else if (csi->pixformat == PIXFORMAT_RGB565) {
                scfg.Format = DCMIPP_FORMAT_RGB565;
                scfg.SwapCycles = (csi->rgb_swap == 1) ? DCMIPP_SWAPCYCLES_DISABLE : DCMIPP_SWAPCYCLES_ENABLE;
            } else if (csi->pixformat == PIXFORMAT_GRAYSCALE) {
                scfg.Format = (csi->mono_bpp == 1) ? DCMIPP_FORMAT_MONOCHROME_8B : DCMIPP_FORMAT_YUV422;
            } else if (csi->pixformat == PIXFORMAT_YUV422) {
                scfg.Format = DCMIPP_FORMAT_YUV422;
                scfg.SwapCycles = (csi->yuv_swap == 1) ? DCMIPP_SWAPCYCLES_ENABLE : DCMIPP_SWAPCYCLES_DISABLE;
            } else if (csi->pixformat == PIXFORMAT_BAYER) {
                scfg.Format = DCMIPP_FORMAT_RAW8;
            } else {
                return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED;
            }
            if (HAL_DCMIPP_PARALLEL_SetConfig(&csi->dcmi, &scfg) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }
        }

        // Configure the pixel processing pipeline.
        DCMIPP_PipeConfTypeDef pcfg = { .FrameRate = DCMIPP_FRAME_RATE_ALL };
        if (csi->pixformat == PIXFORMAT_RGB565) {
            pcfg.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
        } else if (csi->pixformat == PIXFORMAT_GRAYSCALE || csi->pixformat == PIXFORMAT_BAYER) {
            pcfg.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_MONO_Y8_G8_1;
        } else if (csi->pixformat == PIXFORMAT_YUV422) {
            pcfg.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_YUV422_1;
        } else {
            return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED;
        }
        if (HAL_DCMIPP_PIPE_SetConfig(&csi->dcmi, DCMIPP_PIPE, &pcfg) != HAL_OK) {
            return OMV_CSI_ERROR_CSI_INIT_FAILED;
        }

        // Configure debayer.
        if (csi->raw_output && csi->pixformat != PIXFORMAT_BAYER) {
            DCMIPP_RawBayer2RGBConfTypeDef rawcfg = {
                .RawBayerType = DCMIPP_RAWBAYER_BGGR,
                .VLineStrength = DCMIPP_RAWBAYER_ALGO_NONE,
                .HLineStrength = DCMIPP_RAWBAYER_ALGO_NONE,
                .PeakStrength = DCMIPP_RAWBAYER_ALGO_NONE,
                .EdgeStrength = DCMIPP_RAWBAYER_ALGO_NONE,
            };

            if (HAL_DCMIPP_PIPE_SetISPRawBayer2RGBConfig(&csi->dcmi, DCMIPP_PIPE, &rawcfg) != HAL_OK ||
                HAL_DCMIPP_PIPE_EnableISPRawBayer2RGB(&csi->dcmi, DCMIPP_PIPE) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            DCMIPP_ExposureConfTypeDef expcfg = {
                .ShiftRed = 0,
                .MultiplierRed = 128,
                .ShiftGreen = 0,
                .MultiplierGreen = 128,
                .ShiftBlue = 0,
                .MultiplierBlue = 128,
            };

            if (HAL_DCMIPP_PIPE_SetISPExposureConfig(&csi->dcmi, DCMIPP_PIPE, &expcfg) != HAL_OK ||
                HAL_DCMIPP_PIPE_EnableISPExposure(&csi->dcmi, DCMIPP_PIPE) != HAL_OK) {
                return OMV_CSI_ERROR_CSI_INIT_FAILED;
            }

            const uint32_t statsrc[] = {
                DCMIPP_STAT_EXT_SOURCE_PRE_BLKLVL_R,
                DCMIPP_STAT_EXT_SOURCE_PRE_BLKLVL_G,
                DCMIPP_STAT_EXT_SOURCE_PRE_BLKLVL_B
            };
            DCMIPP_StatisticExtractionConfTypeDef statcfg[3];

            for (size_t i = 0; i < 3; i++) {
                statcfg[i].Source = statsrc[i];
                statcfg[i].Mode = DCMIPP_STAT_EXT_MODE_AVERAGE;
                statcfg[i].Bins = DCMIPP_STAT_EXT_AVER_MODE_ALL_PIXELS; //NOEXT16;
            }

            for (size_t i = DCMIPP_STATEXT_MODULE1; i <= DCMIPP_STATEXT_MODULE3; i++) {
                if (HAL_DCMIPP_PIPE_SetISPStatisticExtractionConfig(&csi->dcmi,
                                                                    DCMIPP_PIPE, i,
                                                                    &statcfg[i - DCMIPP_STATEXT_MODULE1]) != HAL_OK) {
                    return OMV_CSI_ERROR_CSI_INIT_FAILED;
                }

                if (HAL_DCMIPP_PIPE_EnableISPStatisticExtraction(&csi->dcmi, DCMIPP_PIPE, i) != HAL_OK) {
                    return OMV_CSI_ERROR_CSI_INIT_FAILED;
                }
            }
        }
        #endif
    }
    return 0;
}

// Stop the DCMI from generating more DMA requests, and disable the DMA.
static int stm_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    if (!DCMI_IS_ACTIVE()) {
        return 0;
    }

    #if USE_DCMI
    DCMI->CR &= ~DCMI_CR_ENABLE;
    #endif

    #if USE_DMA
    if (in_irq) {
        HAL_DMA_Abort_IT(&csi->dma);
    } else {
        HAL_DMA_Abort(&csi->dma);
    }
    HAL_NVIC_DisableIRQ(csi->dma_irqn);
    #endif

    #if USE_MDMA
    if (!in_irq) {
        HAL_MDMA_Abort(&csi->mdma0);
        HAL_MDMA_Abort(&csi->mdma1);
    }
    HAL_MDMA_DeInit(&csi->mdma0);
    HAL_MDMA_DeInit(&csi->mdma1);
    #endif

    #if USE_DCMI
    __HAL_DCMI_DISABLE_IT(&csi->dcmi, DCMI_IT_FRAME);
    __HAL_DCMI_CLEAR_FLAG(&csi->dcmi, DCMI_FLAG_FRAMERI);
    #else
    if (!csi->mipi_if) {
        HAL_DCMIPP_PIPE_Stop(&csi->dcmi, DCMIPP_PIPE);
    } else {
        HAL_DCMIPP_CSI_PIPE_Stop(&csi->dcmi, DCMIPP_PIPE, DCMIPP_VIRTUAL_CHANNEL0);
    }
    for (size_t i=0; i<DCMIPP_NUM_OF_PIPES; i++) {
        csi->dcmi.PipeState[i] = HAL_DCMIPP_PIPE_STATE_RESET;
    }
    #endif

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
    return (OMV_CSI_TIM_PCLK_FREQ() * 2) / (clk->tim.Init.Period + 1);
}

static int stm_clk_set_frequency(omv_clk_t *clk, uint32_t frequency) {
    #if (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_TIM)
    if (frequency == 0) {
        if (clk->tim.Init.Period) {
            HAL_TIM_PWM_Stop(&clk->tim, OMV_CSI_TIM_CHANNEL);
            HAL_TIM_PWM_DeInit(&clk->tim);
            memset(&clk->tim, 0, sizeof(clk->tim));
        }
        return 0;
    }

    clk->tim.Instance = OMV_CSI_TIM;

    // TCLK (PCLK * 2)
    int tclk = OMV_CSI_TIM_PCLK_FREQ() * 2;

    // Find highest possible frequency under requested.
    int period = fast_ceilf(tclk / ((float) frequency)) - 1;
    int pulse = (period + 1) / 2;

    if (clk->tim.Init.Period && (clk->tim.Init.Period != period)) {
        // __HAL_TIM_SET_AUTORELOAD sets clk->tim.Init.Period...
        __HAL_TIM_SET_AUTORELOAD(&clk->tim, period);
        __HAL_TIM_SET_COMPARE(&clk->tim, OMV_CSI_TIM_CHANNEL, pulse);
        return 0;
    }

    /* Timer base configuration */
    clk->tim.Init.Period = period;
    clk->tim.Init.Prescaler = 0;
    clk->tim.Init.CounterMode = TIM_COUNTERMODE_UP;
    clk->tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    clk->tim.Init.RepetitionCounter = 0;
    clk->tim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    /* Timer channel configuration */
    TIM_OC_InitTypeDef TIMOCHandle;
    TIMOCHandle.Pulse = pulse;
    TIMOCHandle.OCMode = TIM_OCMODE_PWM1;
    TIMOCHandle.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIMOCHandle.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    TIMOCHandle.OCFastMode = TIM_OCFAST_DISABLE;
    TIMOCHandle.OCIdleState = TIM_OCIDLESTATE_RESET;
    TIMOCHandle.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    if ((HAL_TIM_PWM_Init(&clk->tim) != HAL_OK)
        || (HAL_TIM_PWM_ConfigChannel(&clk->tim, &TIMOCHandle, OMV_CSI_TIM_CHANNEL) != HAL_OK)
        || (HAL_TIM_PWM_Start(&clk->tim, OMV_CSI_TIM_CHANNEL) != HAL_OK)) {
        return -1;
    }
    #elif (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_MCO)
    // Pass through the MCO1 clock with source input set to HSE (12MHz).
    // Note MCO1 is multiplexed on OPENMV2/TIM1 only.
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
    #elif (OMV_CSI_CLK_SOURCE == OMV_CSI_CLK_SOURCE_OSC)
    // An external oscillator is used for the csi clock.
    // Configure and enable external oscillator if needed.
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

#if USE_DCMI
// If the image is cropped by more than 1 word in width, align the line start to a word
// address to improve copy performance. Do not crop by more than 1 word as this will
// result in less time between DMA transfers complete interrupts on 16-byte boundaries.
static uint32_t get_dcmi_hw_crop(omv_csi_t *csi, uint32_t bytes_per_pixel) {
    framebuffer_t *fb = csi->fb;
    uint32_t byte_x_offset = (fb->x * bytes_per_pixel) % sizeof(uint32_t);
    uint32_t width_remainder = (resolution[csi->framesize][0] - (fb->x + fb->u)) * bytes_per_pixel;

    if (byte_x_offset && (width_remainder >= (sizeof(uint32_t) - byte_x_offset))) {
        return byte_x_offset;
    }

    return 0;
}
#endif

#if USE_DCMI
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
#else
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *dcmipp, uint32_t pipe) {
#endif
    omv_csi_t *csi = omv_csi_get(-1);
    framebuffer_t *fb = csi->fb;

    #if USE_MDMA
    // Clear out any stale flags.
    DMA2->LIFCR = DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5;
    // Re-enable the DMA IRQ to catch the next start line.
    HAL_NVIC_EnableIRQ(csi->dma_irqn);
    #endif

    // Reset DCMI_DMAConvCpltUser frame drop state.
    csi->first_line = false;
    if (csi->drop_frame) {
        csi->drop_frame = false;
        // Reset the buffer's state if the frame was dropped.
        vbuffer_t *buffer = framebuffer_get_tail(fb, FB_PEEK);
        if (buffer) {
            buffer->reset_state = true;
        }
        return;
    }

    framebuffer_get_tail(fb, FB_NO_FLAGS);

    if (csi->frame_cb.fun) {
        csi->frame_cb.fun(csi->frame_cb.arg);
    }

    #if USE_DCMIPP
    // Get the destination buffer address.
    vbuffer_t *buffer = framebuffer_get_tail(fb, FB_PEEK);
    if (buffer == NULL) {
        omv_csi_abort(csi, false, false);
    } else {
        HAL_DCMIPP_PIPE_SetMemoryAddress(dcmipp, pipe, DCMIPP_MEMORY_ADDRESS_0, (uint32_t) buffer->data);
    }
    #endif
}

#if USE_DCMI
// This function is called after each line transfer is complete, with a pointer to the
// buffer that was used. At this point, the DMA transfers the next line to the next buffer.
// Using line buffers allows performing post-processing before writing the frame to the
// framebuffer, and help hide external RAM latency.
void DCMI_DMAConvCpltUser(uint32_t addr) {
    omv_csi_t *csi = omv_csi_get(-1);
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

    vbuffer_t *buffer = framebuffer_get_tail(fb, FB_PEEK);
    if (buffer == NULL) {
        omv_csi_abort(csi, false, true);
        return;
    }

    if (csi->pixformat == PIXFORMAT_JPEG) {
        if (csi->jpg_format == 3) {
            // JPEG MODE 3: Variable line width per frame, with the last line potentially shorter and
            // no padding. `offset` is incremented once every max transfer, and the DMA counter holds
            // the total size.
            buffer->offset += 1;
        } else if (csi->jpg_format == 4) {
            // JPEG MODE 4: Fixed width and height per frame. Each line starts with two bytes indicating
            // valid data length, followed by image data and optional padding (0xFF). `offset` holds the
            // total size.
            uint16_t size = __REV16(*((uint16_t *) addr));
            if (buffer->offset + size > framebuffer_get_buffer_size(fb)) {
                buffer->jpeg_buffer_overflow = true;
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
#endif

#if USE_DCMIPP
void omv_csi_update_awb(omv_csi_t *csi, uint32_t n_pixels) {
    uint32_t avg[3];
    uint32_t shift[3];
    uint32_t multi[3];

    for (int i = 0; i < 3; i++) {
        // DCMIPP_STATEXT_MODULE1
        HAL_DCMIPP_PIPE_GetISPAccumulatedStatisticsCounter(&csi->dcmi, DCMIPP_PIPE, i + 1, &avg[i]);
    }

    // Averages are collected from bayer components (4R 2G 4B).
    avg[0] = OMV_MAX((avg[0] * 256 * 4) / n_pixels, 1);
    avg[1] = OMV_MAX((avg[1] * 256 * 2) / n_pixels, 1);
    avg[2] = OMV_MAX((avg[2] * 256 * 4) / n_pixels, 1);

    // Compute global luminance
    float luminance = avg[0] * 0.299 + avg[1] * 0.587 + avg[2] * 0.114;
    //printf("Luminance: %f AVG_R: %lu, AVG_G: %lu, AVG_B: %lu\n", (double) luminance, avg[0], avg[1], avg[2]);

    if (csi->ioctl) {
        omv_csi_ioctl(csi, OMV_CSI_IOCTL_UPDATE_AGC_AEC, fast_floorf(luminance));
    }

    // Calculate average and exposure factors for each channel (R, G, B)
    for (int i = 0; i < 3; i++) {
        shift[i] = 0;
        multi[i] = roundf((luminance * 128.0f / avg[i]));
        while (multi[i] >= 255.0f && shift[i] < 7) {
            multi[i] /= 2;
            shift[i]++;
        }
        //printf("Channel %d: Expf: %lu Shift: %lu, Multi: %lu\n", i, expf, shift[i], multi[i]);
    }

    // Configure RGB exposure settings.
    DCMIPP_ExposureConfTypeDef expcfg = {
        .ShiftRed = shift[0],
        .MultiplierRed = multi[0],
        .ShiftGreen = shift[1],
        .MultiplierGreen = multi[1],
        .ShiftBlue = shift[2],
        .MultiplierBlue = multi[2],
    };
    HAL_DCMIPP_PIPE_SetISPExposureConfig(&csi->dcmi, DCMIPP_PIPE, &expcfg);
}
#endif

static int stm_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    uint32_t length = 0;
    framebuffer_t *fb = csi->fb;

    if (csi->pixformat == PIXFORMAT_INVALID) {
        return OMV_CSI_ERROR_INVALID_PIXFORMAT;
    }

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    // Compress the framebuffer for the IDE preview, if not the first frame, the
    // framebuffer is enabled, and the image sensor doesn't support JPEG encoding.
    if (flags & OMV_CSI_CAPTURE_FLAGS_UPDATE) {
        framebuffer_update_jpeg_buffer(fb);
    }

    // Ensure that the raw frame fits into the FB. It will be switched from RGB565 to BAYER
    // first to save space before being cropped until it fits.
    omv_csi_auto_crop_framebuffer(csi);

    // Restore frame buffer width and height if they were changed before. BPP is restored later.
    // Note that JPEG compression is done first on the framebuffer with the user settings.
    uint32_t w = fb->u;
    uint32_t h = fb->v;

    // TODO
    // If DCMI_DMAConvCpltUser() happens before framebuffer_free_current_buffer(); below then the
    // transfer is stopped and it will be re-enabled again right afterwards in the single vbuffer
    // case.
    framebuffer_free_current_buffer(fb);

    // Configure and start the capture.
    if (!DCMI_IS_ACTIVE()) {
        framebuffer_setup_buffers(fb);

        // Get the destination buffer address.
        vbuffer_t *buffer = framebuffer_get_tail(fb, FB_PEEK);
        if (buffer == NULL) {
            return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
        }

        #if USE_DCMI
        // Setup the size and address of the transfer
        uint32_t bytes_per_pixel = omv_csi_get_src_bpp(csi);
        uint32_t x_crop = get_dcmi_hw_crop(csi, bytes_per_pixel);
        uint32_t line_width_bytes = resolution[csi->framesize][0] * bytes_per_pixel;

        // Shrink the captured pixel count by one word to allow cropping to fix alignment.
        if (x_crop) {
            line_width_bytes -= sizeof(uint32_t);
        }

        length = line_width_bytes * h;

        // Error out if the transfer size is not compatible with DMA transfer restrictions.
        if ((!line_width_bytes) ||
            (line_width_bytes % sizeof(uint32_t)) ||
            (line_width_bytes > (OMV_LINE_BUF_SIZE / 2)) ||
            (!length) || (length % LINE_WIDTH_ALIGNMENT)) {
            return OMV_CSI_ERROR_INVALID_FRAMESIZE;
        }

        HAL_DCMI_DisableCrop(&csi->dcmi);
        if (csi->pixformat != PIXFORMAT_JPEG) {
            // Vertically crop the image. Horizontal cropping is done in software.
            HAL_DCMI_ConfigCrop(&csi->dcmi, x_crop, fb->y, line_width_bytes - 1, h - 1);
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
        ((DMA_Stream_TypeDef *) csi->dma.Instance)->CR &= ~(DMA_SxCR_CIRC | DMA_SxCR_CT | DMA_SxCR_DBM);
        HAL_NVIC_EnableIRQ(csi->dma_irqn);

        // HAL_DCMI_Start_DMA and HAL_DCMI_Start_DMA_MB both perform circular transfers,
        // differing only in size, with an interrupt after every half of the transfer.
        if ((csi->pixformat == PIXFORMAT_JPEG) && (csi->jpg_format == 3)) {
            // Start a one-shot transfer to the framebuffer, used only for JPEG mode 3.
            uint32_t size = framebuffer_get_buffer_size(fb);
            length = IM_MIN(size, (DMA_MAX_TRANSFER * 2U));
            HAL_DCMI_Start_DMA(&csi->dcmi, DCMI_MODE_SNAPSHOT,
                               (uint32_t) buffer->data, length / sizeof(uint32_t));
            // HAL_DCMI_Start_DMA splits bigger transfers.
            if (length > DMA_MAX_TRANSFER) {
                length /= 2;
            }
        #if USE_MDMA
        } else if ((csi->pixformat != PIXFORMAT_JPEG) && (!csi->transpose)) {
            // Special transfer mode that uses DMA in circular mode and MDMA
            // to move the lines to the final destination.
            ((DMA_Stream_TypeDef *) csi->dma.Instance)->CR |= DMA_SxCR_CIRC;
            HAL_DCMI_Start_DMA(&csi->dcmi, DCMI_MODE_CONTINUOUS,
                               (uint32_t) &_line_buf, line_width_bytes / sizeof(uint32_t));
        #endif  // USE_MDMA
        } else {
            // Start a multibuffer (line by line) transfer.
            HAL_DCMI_Start_DMA_MB(&csi->dcmi, DCMI_MODE_CONTINUOUS,
                                  (uint32_t) &_line_buf, length / sizeof(uint32_t), h);
        }
        #else
        uint32_t bytes_per_pixel = omv_csi_get_dst_bpp(csi);
        uint32_t line_width_bytes = fb->u * bytes_per_pixel;

        if (!line_width_bytes ||
            line_width_bytes % LINE_WIDTH_ALIGNMENT) {
            return OMV_CSI_ERROR_INVALID_FRAMESIZE;
        }

        // Configure crop
        DCMIPP_CropConfTypeDef ccfg = {
            .HStart = fb->x,
            .VStart = fb->y,
            .HSize = fb->u,
            .VSize = fb->v,
        };
        if (HAL_DCMIPP_PIPE_SetCropConfig(&csi->dcmi, DCMIPP_PIPE, &ccfg) != HAL_OK ||
            HAL_DCMIPP_PIPE_EnableCrop(&csi->dcmi, DCMIPP_PIPE) != HAL_OK) {
            return OMV_CSI_ERROR_CSI_INIT_FAILED;
        }

        // Set output pitch
        if (HAL_DCMIPP_PIPE_SetPitch(&csi->dcmi, DCMIPP_PIPE, line_width_bytes) != HAL_OK) {
            return OMV_CSI_ERROR_CSI_INIT_FAILED;
        }

        // Start the DCMIPP
        if (!csi->mipi_if) {
            if (HAL_DCMIPP_PIPE_Start(&csi->dcmi, DCMIPP_PIPE, (uint32_t) buffer->data,
                                      DCMIPP_MODE_CONTINUOUS) != HAL_OK) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }
        } else {
            if (HAL_DCMIPP_CSI_PIPE_Start(&csi->dcmi, DCMIPP_PIPE, DCMIPP_VIRTUAL_CHANNEL0,
                                          (uint32_t) buffer->data, DCMIPP_MODE_CONTINUOUS) != HAL_OK) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }
        }
        #endif  // USE_DCMI
    }

    // Trigger the camera if FSYNC is enabled.
    #if defined(OMV_CSI_FSYNC_PIN)
    if (csi->frame_sync) {
        omv_gpio_write(OMV_CSI_FSYNC_PIN, 1);
    }
    #endif

    #if USE_DCMI
    // In JPEG mode, enable the end of frame interrupt.
    if (DCMI->CR & DCMI_JPEG_ENABLE) {
        __HAL_DCMI_ENABLE_IT(&csi->dcmi, DCMI_IT_FRAME);
    }
    #endif

    framebuffer_flags_t fb_flags = FB_NO_FLAGS;

    #if USE_MDMA
    // csi->mdma0.State will be HAL_MDMA_STATE_RESET if the MDMA is not initialized.
    if (csi->mdma0.State != HAL_MDMA_STATE_RESET) {
        fb_flags = FB_INVALIDATE;
    }
    #endif

    // Wait for a frame to be ready.
    vbuffer_t *buffer = NULL;
    for (uint32_t tick_start = HAL_GetTick(); !(buffer = framebuffer_get_head(fb, fb_flags)); ) {
        __WFI();
        if ((HAL_GetTick() - tick_start) > OMV_CSI_TIMEOUT_MS) {
            omv_csi_abort(csi, true, false);
            #if defined(OMV_CSI_FSYNC_PIN)
            if (csi->frame_sync) {
                omv_gpio_write(OMV_CSI_FSYNC_PIN, 0);
            }
            #endif
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
    }

    #if USE_DMA
    // In JPEG 3 mode, the transfer must be aborted as it waits for data indefinitely.
    if ((csi->pixformat == PIXFORMAT_JPEG) && (csi->jpg_format == 3)) {
        omv_csi_abort(csi, true, false);
    }
    #endif

    // We're done receiving data.
    #if defined(OMV_CSI_FSYNC_PIN)
    if (csi->frame_sync) {
        omv_gpio_write(OMV_CSI_FSYNC_PIN, 0);
    }
    #endif

    // The JPEG in the frame buffer is actually invalid.
    if (buffer->jpeg_buffer_overflow) {
        return OMV_CSI_ERROR_JPEG_OVERFLOW;
    }

    // Prepare the frame buffer w/h/bpp values given the image type.
    if (!csi->transpose) {
        fb->w = w;
        fb->h = h;
    } else {
        fb->w = h;
        fb->h = w;
    }

    // Fix the BPP.
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
                size = buffer->offset * length;
                // The DMA counter holds the number of bytes per transfer.
                #if USE_DMA
                if (__HAL_DMA_GET_COUNTER(&csi->dma)) {
                    // Add in the uncompleted transfer length.
                    size += ((length / sizeof(uint32_t)) - __HAL_DMA_GET_COUNTER(&csi->dma)) * sizeof(uint32_t);
                }
                #endif
            }
            // Clean trailing data after 0xFFD9 at the end of the jpeg byte stream.
            fb->pixfmt = PIXFORMAT_JPEG;
            fb->size = jpeg_clean_trailing_bytes(size, buffer->data);
            break;
        }
        default:
            break;
    }

    // Set the user image.
    framebuffer_init_image(fb, image);
    #if USE_DCMIPP
    if (csi->raw_output) {
        omv_csi_update_awb(csi, w * h);
    }
    #endif
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
