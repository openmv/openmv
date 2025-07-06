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
 * STM32 DCMIPP ISP driver.
 */

#include <string.h>
#include "imlib.h"
#include "stm_isp.h"
#include "omv_boardconfig.h"

#ifdef DCMIPP
float stm_isp_update_awb(DCMIPP_HandleTypeDef *dcmipp, uint32_t pipe, uint32_t n_pixels) {
    uint32_t avg[3];
    uint32_t shift[3];
    uint32_t multi[3];

    for (int i = 0; i < 3; i++) {
        // DCMIPP_STATEXT_MODULE1
        HAL_DCMIPP_PIPE_GetISPAccumulatedStatisticsCounter(dcmipp, pipe, i + 1, &avg[i]);
    }

    // Averages are collected from bayer components (4R 2G 4B).
    avg[0] = OMV_MAX((avg[0] * 256 * 4) / n_pixels, 1);
    avg[1] = OMV_MAX((avg[1] * 256 * 2) / n_pixels, 1);
    avg[2] = OMV_MAX((avg[2] * 256 * 4) / n_pixels, 1);

    // Compute global luminance
    float luminance = avg[0] * 0.299 + avg[1] * 0.587 + avg[2] * 0.114;

    // Calculate average and exposure factors for each channel (R, G, B)
    for (int i = 0; i < 3; i++) {
        shift[i] = 0;
        multi[i] = roundf((luminance * 128.0f / avg[i]));
        while (multi[i] >= 255.0f && shift[i] < 7) {
            multi[i] /= 2;
            shift[i]++;
        }
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

    HAL_DCMIPP_PIPE_SetISPExposureConfig(dcmipp, pipe, &expcfg);

    return luminance;
}

int stm_isp_config_pipeline(DCMIPP_HandleTypeDef *dcmipp, uint32_t pipe,
                            pixformat_t pixformat, bool raw_output) {
    // Configure the pixel processing pipeline.
    DCMIPP_PipeConfTypeDef pcfg = {
        .FrameRate = DCMIPP_FRAME_RATE_ALL
    };

    if (pixformat == PIXFORMAT_RGB565) {
        pcfg.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
    } else if (pixformat == PIXFORMAT_GRAYSCALE || pixformat == PIXFORMAT_BAYER) {
        pcfg.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_MONO_Y8_G8_1;
    } else if (pixformat == PIXFORMAT_YUV422) {
        pcfg.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_YUV422_1;
    } else {
        return -1;
    }

    if (HAL_DCMIPP_PIPE_SetConfig(dcmipp, pipe, &pcfg) != HAL_OK) {
        return -1;
    }

    // Early exit if no debayer is needed.
    if (!raw_output || pixformat == PIXFORMAT_BAYER) {
        return 0;
    }

    // Configure ISP debayer.
    DCMIPP_RawBayer2RGBConfTypeDef rawcfg = {
        .RawBayerType = DCMIPP_RAWBAYER_BGGR,
        .VLineStrength = DCMIPP_RAWBAYER_ALGO_NONE,
        .HLineStrength = DCMIPP_RAWBAYER_ALGO_NONE,
        .PeakStrength = DCMIPP_RAWBAYER_ALGO_NONE,
        .EdgeStrength = DCMIPP_RAWBAYER_ALGO_NONE,
    };

    if (HAL_DCMIPP_PIPE_SetISPRawBayer2RGBConfig(dcmipp, pipe, &rawcfg) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableISPRawBayer2RGB(dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    DCMIPP_ExposureConfTypeDef expcfg = {
        .ShiftRed = 0,
        .MultiplierRed = 128,
        .ShiftGreen = 0,
        .MultiplierGreen = 128,
        .ShiftBlue = 0,
        .MultiplierBlue = 128,
    };

    if (HAL_DCMIPP_PIPE_SetISPExposureConfig(dcmipp, pipe, &expcfg) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableISPExposure(dcmipp, pipe) != HAL_OK) {
        return -1;
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
        if (HAL_DCMIPP_PIPE_SetISPStatisticExtractionConfig(dcmipp,
                    pipe, i,
                    &statcfg[i - DCMIPP_STATEXT_MODULE1]) != HAL_OK) {
            return -1;
        }

        if (HAL_DCMIPP_PIPE_EnableISPStatisticExtraction(dcmipp, pipe, i) != HAL_OK) {
            return -1;
        }
    }
    
    return 0;
}
#endif  // DCMIPP
