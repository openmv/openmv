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
#include "py/mphal.h"
#include "stm_isp.h"
#include "board_config.h"

#ifdef DCMIPP
#ifdef OMV_CSI_HW_CCM_ENABLE
// Matrix3x3_Coeff: 11-bit signed, 2 int + 8 dec -> 1.0 = 256.
// Added3x1_Coeff:  10-bit signed, 9 int + 0 dec -> 1.0 = 1.
#define DCMIPP_CCM_ONE          (256)
#define DCMIPP_CCM_MIN          (-1024)
#define DCMIPP_CCM_MAX          (1023)
#define DCMIPP_CCM_OFFSET_MIN   (-512)
#define DCMIPP_CCM_OFFSET_MAX   (511)

// Log chromaticity (ln(R/G), ln(B/G)) of an illuminant.
static void stm_isp_ccm_chroma(const float avg[3], float chroma[2]) {
    float g = OMV_MAX(avg[1], 1.0f);

    chroma[0] = logf(OMV_MAX(avg[0], 1.0f) / g);
    chroma[1] = logf(OMV_MAX(avg[2], 1.0f) / g);
}

// Projects the AWB illuminant onto the polyline of calibration illuminants in log
// chromaticity space and blends the CCMs at the ends of the closest segment (~linear in mired).
static void stm_isp_update_ccm(omv_csi_t *csi, uint32_t pipe, const uint32_t avg[3]) {
    if (csi->ccm == NULL || csi->ccm_count == 0) {
        return;
    }

    float cur[2];
    float cur_avg[3] = { avg[0], avg[1], avg[2] };
    stm_isp_ccm_chroma(cur_avg, cur);

    size_t idx = 0;
    float t = 0.0f;
    float best = INFINITY;

    for (size_t i = 0; i + 1 < csi->ccm_count; i++) {
        float p0[2];
        float p1[2];
        stm_isp_ccm_chroma(csi->ccm[i].avg, p0);
        stm_isp_ccm_chroma(csi->ccm[i + 1].avg, p1);

        float dx = p1[0] - p0[0];
        float dy = p1[1] - p0[1];
        float len = OMV_MAX(dx * dx + dy * dy, 1e-6f);
        float u = IM_CLAMP(((cur[0] - p0[0]) * dx + (cur[1] - p0[1]) * dy) / len, 0.0f, 1.0f);

        float ex = p0[0] + u * dx - cur[0];
        float ey = p0[1] + u * dy - cur[1];
        float dist = ex * ex + ey * ey;

        if (dist < best) {
            best = dist;
            idx = i;
            t = u;
        }
    }

    const omv_csi_ccm_t *a = &csi->ccm[idx];
    const omv_csi_ccm_t *b = &csi->ccm[OMV_MIN(idx + 1, csi->ccm_count - 1)];
    int16_t coeffs[3][3];
    int16_t offsets[3];

    for (int r = 0; r < 3; r++) {
        float sum = 0.0f;
        int qsum = 0;

        for (int c = 0; c < 3; c++) {
            float v = a->coeffs[r][c] + t * (b->coeffs[r][c] - a->coeffs[r][c]);
            int q = fast_roundf(v * DCMIPP_CCM_ONE);
            coeffs[r][c] = IM_CLAMP(q, DCMIPP_CCM_MIN, DCMIPP_CCM_MAX);
            qsum += coeffs[r][c];
            sum += v;
        }

        // Push the rounding error into the diagonal so that neutrals stay neutral.
        int diag = coeffs[r][r] + fast_roundf(sum * DCMIPP_CCM_ONE) - qsum;
        coeffs[r][r] = IM_CLAMP(diag, DCMIPP_CCM_MIN, DCMIPP_CCM_MAX);

        int offset = fast_roundf(a->offsets[r] + t * (b->offsets[r] - a->offsets[r]));
        offsets[r] = IM_CLAMP(offset, DCMIPP_CCM_OFFSET_MIN, DCMIPP_CCM_OFFSET_MAX);
    }

    DCMIPP_ColorConversionConfTypeDef cfg = {
        .ClampOutputSamples = ENABLE,
        .OutputSamplesType = DCMIPP_CLAMP_RGB,
        .RR = coeffs[0][0],
        .RG = coeffs[0][1],
        .RB = coeffs[0][2],
        .RA = offsets[0],
        .GR = coeffs[1][0],
        .GG = coeffs[1][1],
        .GB = coeffs[1][2],
        .GA = offsets[1],
        .BR = coeffs[2][0],
        .BG = coeffs[2][1],
        .BB = coeffs[2][2],
        .BA = offsets[2],
    };

    HAL_DCMIPP_PIPE_SetISPColorConversionConfig(&csi->dcmipp, pipe, &cfg);
    HAL_DCMIPP_PIPE_EnableISPColorConversion(&csi->dcmipp, pipe);
}
#endif // OMV_CSI_HW_CCM_ENABLE

float stm_isp_update_awb(omv_csi_t *csi, uint32_t pipe) {
    uint32_t avg[3];
    uint32_t shift[3];
    uint32_t multi[3];
    float luminance;

    uint32_t src_w = csi->src_w ? csi->src_w : csi->resolution[csi->framesize][0];
    uint32_t src_h = csi->src_h ? csi->src_h : csi->resolution[csi->framesize][1];
    uint32_t n_pixels = src_w * src_h;

    for (int i = 0; i < 3; i++) {
        // DCMIPP_STATEXT_MODULE1
        HAL_DCMIPP_PIPE_GetISPAccumulatedStatisticsCounter(&csi->dcmipp, pipe, i + 1, &avg[i]);
    }

    // Averages are collected from bayer components (4R 2G 4B).
    avg[0] = OMV_MAX((avg[0] * 256 * 4) / n_pixels, 1);
    avg[1] = OMV_MAX((avg[1] * 256 * 2) / n_pixels, 1);
    avg[2] = OMV_MAX((avg[2] * 256 * 4) / n_pixels, 1);

    // Compute raw (un-smoothed) luminance for AEC before the EMA filter.
    luminance = avg[0] * 0.299f + avg[1] * 0.587f + avg[2] * 0.114f;

    if (csi->stats_enabled) {
        omv_csi_stats_update(csi, &avg[0], &avg[1], &avg[2], mp_hal_ticks_ms());
    }

    omv_csi_get_stats(csi, &avg[0], &avg[1], &avg[2]);

    // Compute global luminance (EMA-smoothed, used for AWB)
    float awb_luminance = avg[0] * 0.299f + avg[1] * 0.587f + avg[2] * 0.114f;

    // Calculate average and exposure factors for each channel (R, G, B)
    for (int i = 0; i < 3; i++) {
        shift[i] = 0;
        multi[i] = roundf((awb_luminance * 128.0f / avg[i]));
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

    HAL_DCMIPP_PIPE_SetISPExposureConfig(&csi->dcmipp, pipe, &expcfg);

    #ifdef OMV_CSI_HW_CCM_ENABLE
    stm_isp_update_ccm(csi, pipe, avg);
    #endif // OMV_CSI_HW_CCM_ENABLE

    return luminance;
}

int stm_isp_init(omv_csi_t *csi, uint32_t pipe, pixformat_t pixformat, bool raw_output) {
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

    if (HAL_DCMIPP_PIPE_SetConfig(&csi->dcmipp, pipe, &pcfg) != HAL_OK) {
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
        if (HAL_DCMIPP_PIPE_SetISPStatisticExtractionConfig(&csi->dcmipp, pipe, i,
                                                            &statcfg[i - DCMIPP_STATEXT_MODULE1]) != HAL_OK) {
            return -1;
        }

        if (HAL_DCMIPP_PIPE_EnableISPStatisticExtraction(&csi->dcmipp, pipe, i) != HAL_OK) {
            return -1;
        }
    }

    if (HAL_DCMIPP_PIPE_SetISPBadPixelRemovalConfig(&csi->dcmipp, pipe,
                                                    DCMIPP_BAD_PXL_REM_SRENGTH_4) != HAL_OK) {
        return -1;
    }

    if (HAL_DCMIPP_PIPE_EnableISPBadPixelRemoval(&csi->dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    // Enabled by the AWB update when the sensor provides color correction matrices.
    HAL_DCMIPP_PIPE_DisableISPColorConversion(&csi->dcmipp, pipe);

    // Early exit if no debayer is needed.
    if (!raw_output || pixformat == PIXFORMAT_BAYER) {
        HAL_DCMIPP_PIPE_DisableISPRawBayer2RGB(&csi->dcmipp, pipe);
        HAL_DCMIPP_PIPE_DisableISPExposure(&csi->dcmipp, pipe);
        HAL_DCMIPP_PIPE_DisableISPCtrlContrast(&csi->dcmipp, pipe);
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

    if (HAL_DCMIPP_PIPE_SetISPRawBayer2RGBConfig(&csi->dcmipp, pipe, &rawcfg) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableISPRawBayer2RGB(&csi->dcmipp, pipe) != HAL_OK) {
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

    if (HAL_DCMIPP_PIPE_SetISPExposureConfig(&csi->dcmipp, pipe, &expcfg) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableISPExposure(&csi->dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    if (HAL_DCMIPP_PIPE_EnableISPCtrlContrast(&csi->dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    return 0;
}

int stm_isp_update_gamma_table(omv_csi_t *csi, uint32_t pipe,
                               float brightness, float contrast, float gamma) {
    uint8_t g_tab[9]; // sizeof(DCMIPP_ContrastConfTypeDef)

    for (int i = 0; i < 9; i++) {
        // x goes from 0.0001 to 1.0 (to avoid division by zero)
        float x = IM_MAX(i / 8.0f, 0.0001f);
        float y = powf(x, 1.0f / gamma) * contrast + brightness;
        int gain = fast_roundf(y / x * 16.0f);
        g_tab[i] = IM_CLAMP(gain, 0, 63);
    }

    if (HAL_DCMIPP_PIPE_SetISPCtrlContrastConfig(&csi->dcmipp, pipe,
                                                 (DCMIPP_ContrastConfTypeDef *) &g_tab) != HAL_OK) {
        return -1;
    }

    return 0;
}

static int stm_isp_axis_scaler(uint32_t src, int dec, uint32_t dst,
                               uint32_t *ratio_out, uint32_t *div_out) {
    // 16-bit fields, used as unsigned 3 integer 13 fixed-point (3.13).
    // Valid values are 8192 (= 1x) to 65535 (= 7.999x).
    uint32_t ratio = 8192 * (src >> dec) / dst;

    if (ratio > 65535) {
        return -1;
    }

    *ratio_out = ratio;
    // 10-bit fields, used as unsigned 10 decimal.
    // Valid values are 128 (= inv8x) to 1023 (= inv1x).
    *div_out = (1024 * 8192 - 1) / ratio;
    return 0;
}

int stm_isp_set_scaler(omv_csi_t *csi, uint32_t pipe) {
    framebuffer_t *fb = csi->fb;
    uint32_t dst_w = csi->resolution[csi->framesize][0];
    uint32_t dst_h = csi->resolution[csi->framesize][1];
    uint32_t src_w = csi->src_w ? csi->src_w : dst_w;
    uint32_t src_h = csi->src_h ? csi->src_h : dst_h;

    if (csi->pixformat == PIXFORMAT_BAYER && (src_w != dst_w || src_h != dst_h)) {
        return -1;
    }

    // Compute center trimmed aspect ratio (AR) correct crop.
    uint32_t ar_x;
    uint32_t ar_y;
    uint32_t ar_w;
    uint32_t ar_h;

    if (src_w * dst_h > dst_w * src_h) {
        // src is wider than dst: crop horizontally, keep full height.
        ar_h = src_h;
        ar_w = src_h * dst_w / dst_h;
        ar_x = (src_w - ar_w) / 2;
        ar_y = 0;
    } else {
        // src is taller than dst: crop vertically, keep full width.
        ar_w = src_w;
        ar_h = src_w * dst_h / dst_w;
        ar_x = 0;
        ar_y = (src_h - ar_h) / 2;
    }

    // Combine AR crop with user window (coordinates in output/target space).
    ar_x += fb->x * ar_w / dst_w;
    ar_y += fb->y * ar_h / dst_h;
    ar_w = fb->u * ar_w / dst_w;
    ar_h = fb->v * ar_h / dst_h;

    // Determine decimation and downsize ratios.
    int h_dec = -1;
    int v_dec = -1;
    uint32_t h_ratio;
    uint32_t h_div;
    uint32_t v_ratio;
    uint32_t v_div;

    for (int i = 0; i < 4; i++) {
        if (!stm_isp_axis_scaler(ar_w, i, fb->u, &h_ratio, &h_div)) {
            h_dec = i;
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (!stm_isp_axis_scaler(ar_h, i, fb->v, &v_ratio, &v_div)) {
            v_dec = i;
            break;
        }
    }

    // Target image is too small.
    if (h_dec < 0 || v_dec < 0) {
        return -1;
    }

    DCMIPP_CropConfTypeDef ccfg = {
        .HStart = ar_x,
        .VStart = ar_y,
        .HSize = ar_w,
        .VSize = ar_h,
    };

    if (HAL_DCMIPP_PIPE_SetCropConfig(&csi->dcmipp, pipe, &ccfg) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableCrop(&csi->dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    DCMIPP_DecimationConfTypeDef dec = {
        .HRatio = (h_dec << DCMIPP_P1DECR_HDEC_Pos),
        .VRatio = (v_dec << DCMIPP_P1DECR_VDEC_Pos),
    };

    if (HAL_DCMIPP_PIPE_SetDecimationConfig(&csi->dcmipp, pipe, &dec) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableDecimation(&csi->dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    DCMIPP_DownsizeTypeDef ds = {
        .HSize = fb->u,
        .VSize = fb->v,
        .HRatio = h_ratio,
        .VRatio = v_ratio,
        .HDivFactor = h_div,
        .VDivFactor = v_div,
    };

    if (HAL_DCMIPP_PIPE_SetDownsizeConfig(&csi->dcmipp, pipe, &ds) != HAL_OK ||
        HAL_DCMIPP_PIPE_EnableDownsize(&csi->dcmipp, pipe) != HAL_OK) {
        return -1;
    }

    return 0;
}
#endif  // DCMIPP
