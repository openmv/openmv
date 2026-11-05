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
#if defined(OMV_CSI_STATS_IMAGE_ENABLE)
// The gains and CCM last written to the ISP, which the AWB stats undo in the next frame.
static void stm_isp_reset_xform(omv_csi_t *csi) {
    memset(&csi->stats_xform, 0, sizeof(csi->stats_xform));
    for (int i = 0; i < 3; i++) {
        csi->stats_xform.gains[i] = 1.0f;
    }
    csi->stats_xform.gamma = 1.0f;
}

// The stats undo the gains the ISP applies, which are rounded to the multiplier and shift.
static void stm_isp_set_xform_gains(omv_csi_t *csi, const uint32_t multi[3], const uint32_t shift[3]) {
    for (int i = 0; i < 3; i++) {
        csi->stats_xform.gains[i] = (multi[i] << shift[i]) / 128.0f;
    }
}

// A NULL CCM means the color conversion block is disabled.
static void stm_isp_set_xform_ccm(omv_csi_t *csi, const omv_csi_ccm_t *ccm) {
    csi->stats_xform.ccm_valid = ccm != NULL;
    if (ccm != NULL) {
        memcpy(csi->stats_xform.ccm, ccm->coeffs, sizeof(csi->stats_xform.ccm));
        memcpy(csi->stats_xform.offsets, ccm->offsets, sizeof(csi->stats_xform.offsets));
    }
}

// The ISP's averages initialize the stats, then RGB565 frames are measured from their pixels.
static bool stm_isp_image_stats(omv_csi_t *csi) {
    return csi->stats.initialized && csi->pixformat == PIXFORMAT_RGB565;
}

// Measures the AWB from a captured frame, before the ISP settings change for the next one.
void stm_isp_update_stats(omv_csi_t *csi, const image_t *image) {
    if (!csi->stats_enabled || !stm_isp_image_stats(csi)) {
        return;
    }

    // The gamma table is only loaded for MIPI sensors (see stm_csi_isp_reset).
    csi->stats_xform.gamma = csi->mipi_if ? STM_ISP_GAMMA : 1.0f;
    omv_csi_stats_update_image(csi, image, mp_hal_ticks_ms());
}
#endif // defined(OMV_CSI_STATS_IMAGE_ENABLE)

#ifdef OMV_CSI_HW_CCM_ENABLE
// Matrix3x3_Coeff: 11-bit signed, 2 int + 8 dec -> 1.0 = 256.
// Added3x1_Coeff:  10-bit signed, 9 int + 0 dec -> 1.0 = 1.
#define DCMIPP_CCM_ONE          (256)
#define DCMIPP_CCM_MIN          (-1024)
#define DCMIPP_CCM_MAX          (1023)
#define DCMIPP_CCM_OFFSET_MIN   (-512)
#define DCMIPP_CCM_OFFSET_MAX   (511)

// Quantizes the blended CCM into the ISP's color conversion block.
static void stm_isp_update_ccm(omv_csi_t *csi, uint32_t pipe, const float avg[3], float luminance) {
    omv_csi_ccm_t ccm;
    int16_t coeffs[3][3];
    int16_t offsets[3];

    if (!omv_csi_ccm_compute(csi->ccm, avg, luminance, &ccm)) {
        return;
    }

    for (int r = 0; r < 3; r++) {
        float sum = 0.0f;
        int qsum = 0;

        for (int c = 0; c < 3; c++) {
            int q = fast_roundf(ccm.coeffs[r][c] * DCMIPP_CCM_ONE);
            coeffs[r][c] = IM_CLAMP(q, DCMIPP_CCM_MIN, DCMIPP_CCM_MAX);
            qsum += coeffs[r][c];
            sum += ccm.coeffs[r][c];
        }

        // Push the rounding error into the diagonal so that neutrals stay neutral.
        int diag = coeffs[r][r] + fast_roundf(sum * DCMIPP_CCM_ONE) - qsum;
        coeffs[r][r] = IM_CLAMP(diag, DCMIPP_CCM_MIN, DCMIPP_CCM_MAX);

        int offset = fast_roundf(ccm.offsets[r]);
        offsets[r] = IM_CLAMP(offset, DCMIPP_CCM_OFFSET_MIN, DCMIPP_CCM_OFFSET_MAX);
    }

    DCMIPP_ColorConversionConfTypeDef cfg = {
        .ClampOutputSamples = DISABLE,
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

    #if defined(OMV_CSI_STATS_IMAGE_ENABLE)
    // The stats undo the CCM the ISP applies, which is rounded to its fixed point format.
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            ccm.coeffs[r][c] = coeffs[r][c] / (float) DCMIPP_CCM_ONE;
        }
        ccm.offsets[r] = offsets[r];
    }

    stm_isp_set_xform_ccm(csi, &ccm);
    #endif
}
#endif // OMV_CSI_HW_CCM_ENABLE

float stm_isp_update_awb(omv_csi_t *csi, uint32_t pipe) {
    uint32_t avg[3];
    float gain[3];
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

    #if defined(OMV_CSI_STATS_IMAGE_ENABLE)
    bool image_stats = stm_isp_image_stats(csi);
    #else
    bool image_stats = false;
    #endif

    if (csi->stats_enabled && !image_stats) {
        omv_csi_stats_update(csi, &avg[0], &avg[1], &avg[2], mp_hal_ticks_ms());
    }

    // There are no colors to balance, the exposure gains stay at unity.
    if (csi->cfa_mono) {
        return luminance;
    }

    omv_csi_stats_get_gains(csi, &gain[0], &gain[1], &gain[2]);

    #if defined(OMV_CSI_STATS_IMAGE_ENABLE)
    // The CCM block only stays enabled while stm_isp_update_ccm() updates it.
    stm_isp_set_xform_ccm(csi, NULL);
    #endif

    // Calculate the exposure factors for each channel (R, G, B)
    for (int i = 0; i < 3; i++) {
        shift[i] = 0;
        multi[i] = roundf(gain[i] * 128.0f);
        while (multi[i] >= 255.0f && shift[i] < 7) {
            multi[i] /= 2;
            shift[i]++;
        }
    }

    #if defined(OMV_CSI_STATS_IMAGE_ENABLE)
    stm_isp_set_xform_gains(csi, multi, shift);
    #endif

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
    // Grayscale is packed from the green channel, which the CCM would make noisier.
    if (csi->pixformat == PIXFORMAT_RGB565 || csi->pixformat == PIXFORMAT_YUV422) {
        float illuminant[3];
        omv_csi_stats_get(csi, &illuminant[0], &illuminant[1], &illuminant[2]);
        stm_isp_update_ccm(csi, pipe, illuminant, luminance);
    }
    #endif // OMV_CSI_HW_CCM_ENABLE

    return luminance;
}

#ifdef OMV_CSI_HW_BLC_ENABLE
// The black level follows the sensor gain, which only needs to be read as often as the AWB settles.
#define STM_ISP_BLC_PERIOD_MS   (250)

// How far past a rounding boundary the black level must move before it changes.
#define STM_ISP_BLC_HYSTERESIS  (0.75f)

// The sensor clamps its output at zero, which rectifies the noise around black into a floor that grows with
// gain. It's subtracted before the AWB gains, which would otherwise tint it.
void stm_isp_update_black_level(omv_csi_t *csi, uint32_t pipe) {
    if (csi->blc_slope <= 0.0f || csi->pixformat == PIXFORMAT_BAYER) {
        return;
    }

    // The ISP init disables the block, so it's updated right away after that.
    bool enabled = HAL_DCMIPP_PIPE_IsEnabledISPBlackLevelCalibration(&csi->dcmipp, pipe);
    uint32_t ms = mp_hal_ticks_ms();

    if (enabled && ms - csi->blc_ms < STM_ISP_BLC_PERIOD_MS) {
        return;
    }

    float gain_db;
    csi->blc_ms = ms;
    if (omv_csi_get_gain_db(csi, &gain_db) != 0) {
        return;
    }

    DCMIPP_BlackLevelConfTypeDef cfg;
    HAL_DCMIPP_PIPE_GetISPBlackLevelCalibrationConfig(&csi->dcmipp, pipe, &cfg);
    float level = csi->blc_slope * powf(10.0f, gain_db / 20.0f);

    // Hysteresis keeps a gain that sits on a rounding boundary from toggling the level.
    if (enabled && fabsf(level - cfg.GreenCompBlackLevel) < STM_ISP_BLC_HYSTERESIS) {
        return;
    }

    int rounded = IM_CLAMP(fast_roundf(level), 0, 255);
    cfg.RedCompBlackLevel = rounded;
    cfg.GreenCompBlackLevel = rounded;
    cfg.BlueCompBlackLevel = rounded;

    HAL_DCMIPP_PIPE_SetISPBlackLevelCalibrationConfig(&csi->dcmipp, pipe, &cfg);
    HAL_DCMIPP_PIPE_EnableISPBlackLevelCalibration(&csi->dcmipp, pipe);
}
#endif // OMV_CSI_HW_BLC_ENABLE

int stm_isp_init(omv_csi_t *csi, uint32_t pipe, pixformat_t pixformat, bool raw_output) {
    #if defined(OMV_CSI_STATS_IMAGE_ENABLE)
    // The exposure gains are reset to unity and the CCM is disabled below.
    stm_isp_reset_xform(csi);
    #endif

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

    // Enabled by the black level update when the sensor provides a black level slope.
    HAL_DCMIPP_PIPE_DisableISPBlackLevelCalibration(&csi->dcmipp, pipe);

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

    if (csi->cfa_mono) {
        // There's nothing to debayer, the pipe replicates the raw pixels.
        HAL_DCMIPP_PIPE_DisableISPRawBayer2RGB(&csi->dcmipp, pipe);
    } else if (HAL_DCMIPP_PIPE_SetISPRawBayer2RGBConfig(&csi->dcmipp, pipe, &rawcfg) != HAL_OK ||
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
