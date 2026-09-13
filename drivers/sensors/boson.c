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
 * Boson driver.
 */
#include "board_config.h"
#if (OMV_BOSON_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "py/mphal.h"
#include "framebuffer.h"
#include "omv_csi.h"

#include "Client_API.h"
#include "UART_Connector.h"
#include "serialPortAdapter.h"

#define FLIR_BOSON_BOOT_TRY_COUNT (10)
#define FLIR_BOSON_BOOT_TIME_MS (1000)

static int boson_framesize = 0;

static int reset(omv_csi_t *csi) {
    FSLP_set_csi(csi);
    csi->color_palette = NULL;

    int i = 0;
    FLR_BOSON_PARTNUMBER_T part;

    // Older FLIR Boson (< IDD 4.x) cameras take forever to boot up.
    for (; i < FLIR_BOSON_BOOT_TRY_COUNT; i++) {
        if (i > 1) {
            // Print something to prevent the user from thinking the camera is stuck.
            mp_printf(MP_PYTHON_PRINTER,
                      "CSI: FLIR Boson not ready, retrying (%d/%d)...\n",
                      i, FLIR_BOSON_BOOT_TRY_COUNT - 1);
        }

        // Give the camera time to boot.
        mp_hal_delay_ms(FLIR_BOSON_BOOT_TIME_MS);

        // Turn the com port on.
        Initialize();

        if (bosonGetCameraPN(&part) == FLR_OK) {
            break;
        }
    }

    if (i == FLIR_BOSON_BOOT_TRY_COUNT) {
        return -1;
    }

    if (!strncmp((char *) (part.value + 2), "640", 3)) {
        boson_framesize = OMV_CSI_FRAMESIZE_VGA;
    } else if (!strncmp((char *) (part.value + 2), "320", 3)) {
        boson_framesize = OMV_CSI_FRAMESIZE_QVGA;
    } else {
        return -1;
    }

    // Always restore factory defaults to ensure the camera is in a known state.
    FLR_RESULT ret = bosonRestoreFactoryDefaultsFromFlash();

    // FLIR BOSON may glitch after restoring factory defaults.
    if (ret != FLR_OK && ret != FLR_COMM_ERROR_READING_COMM) {
        return -1;
    }

    if (dvoSetOutputFormat(FLR_DVO_DEFAULT_FORMAT) != FLR_OK) {
        return -1;
    }

    if (dvoSetType(FLR_DVO_TYPE_MONO8) != FLR_OK) {
        return -1;
    }

    if (dvoApplyCustomSettings() != FLR_OK) {
        return -1;
    }

    if (telemetrySetState(FLR_DISABLE) != FLR_OK) {
        return -1;
    }

    return 0;
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    return (pixformat == PIXFORMAT_GRAYSCALE) ? 0 : -1;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    return (framesize == boson_framesize) ? 0 : -1;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    FSLP_set_csi(csi);

    if (gaoSetTestRampState(enable ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
        return -1;
    }

    if (testRampSetType(0, FLR_TESTRAMP_VERT_SHADE) != FLR_OK) {
        return -1;
    }

    return 0;
}

static int post_process(omv_csi_t *csi, image_t *image, uint32_t flags) {
    int num_pixels = csi->resolution[boson_framesize][0] * csi->resolution[boson_framesize][1];

    if (csi->color_palette && (framebuffer_get_buffer_size(csi->fb) >= (num_pixels * sizeof(uint16_t)))) {
        for (int32_t i = num_pixels - 1; i >= 0; i--) {
            ((uint16_t *) image->data)[i] = csi->color_palette[image->data[i]];
        }

        image->pixfmt = PIXFORMAT_RGB565;
        csi->fb->pixfmt = PIXFORMAT_RGB565;
    }

    return 0;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;

    FSLP_set_csi(csi);

    switch (request) {
        case OMV_CSI_IOCTL_BOSON_GET_SOFTWARE_REV: {
            uint32_t *major = va_arg(ap, uint32_t *);
            uint32_t *minor = va_arg(ap, uint32_t *);
            uint32_t *patch = va_arg(ap, uint32_t *);
            ret = (bosonGetSoftwareRev(major, minor, patch) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FPA_TEMP: {
            int *temp = va_arg(ap, int *);
            int16_t fpa_temp = 0;
            ret = (bosonlookupFPATempDegCx10(&fpa_temp) == FLR_OK) ? 0 : -1;
            *temp = fpa_temp;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_GAIN_MODE: {
            int mode = va_arg(ap, int);
            ret = (bosonSetGainMode((FLR_BOSON_GAINMODE_E) mode) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_GAIN_MODE: {
            int *mode = va_arg(ap, int *);
            FLR_BOSON_GAINMODE_E gain_mode = FLR_BOSON_HIGH_GAIN;
            ret = (bosonGetGainMode(&gain_mode) == FLR_OK) ? 0 : -1;
            *mode = gain_mode;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_RUN_FFC: {
            ret = (bosonRunFFC() == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_STATUS: {
            int *status = va_arg(ap, int *);
            FLR_BOSON_FFCSTATUS_E ffc_status = FLR_BOSON_NO_FFC_PERFORMED;
            ret = (bosonGetFfcStatus(&ffc_status) == FLR_OK) ? 0 : -1;
            *status = ffc_status;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_MODE: {
            int mode = va_arg(ap, int);
            ret = (bosonSetFFCMode((FLR_BOSON_FFCMODE_E) mode) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_MODE: {
            int *mode = va_arg(ap, int *);
            FLR_BOSON_FFCMODE_E ffc_mode = FLR_BOSON_MANUAL_FFC;
            ret = (bosonGetFFCMode(&ffc_mode) == FLR_OK) ? 0 : -1;
            *mode = ffc_mode;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_TEMP_THRESHOLD: {
            int threshold = va_arg(ap, int);
            ret = (bosonSetFFCTempThreshold(threshold) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_TEMP_THRESHOLD: {
            int *threshold = va_arg(ap, int *);
            uint16_t temp_threshold = 0;
            ret = (bosonGetFFCTempThreshold(&temp_threshold) == FLR_OK) ? 0 : -1;
            *threshold = temp_threshold;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_FRAME_THRESHOLD: {
            int threshold = va_arg(ap, int);
            ret = (bosonSetFFCFrameThreshold(threshold) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_FRAME_THRESHOLD: {
            int *threshold = va_arg(ap, int *);
            uint32_t frame_threshold = 0;
            ret = (bosonGetFFCFrameThreshold(&frame_threshold) == FLR_OK) ? 0 : -1;
            *threshold = frame_threshold;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_NUM_FRAMES: {
            int frames = va_arg(ap, int);
            ret = (gaoSetNumFFCFrames(frames) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_NUM_FRAMES: {
            int *frames = va_arg(ap, int *);
            uint16_t num_frames = 0;
            ret = (gaoGetNumFFCFrames(&num_frames) == FLR_OK) ? 0 : -1;
            *frames = num_frames;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_RADIOMETRY_CAPABLE: {
            int *capable = va_arg(ap, int *);
            FLR_ENABLE_E enable = FLR_DISABLE;
            ret = (radiometryGetRadiometryCapable(&enable) == FLR_OK) ? 0 : -1;
            *capable = (enable == FLR_ENABLE);
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_TLINEAR_ENABLE: {
            int enable = va_arg(ap, int);
            ret = (TLinearSetControl(enable ? FLR_ENABLE : FLR_DISABLE) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_TLINEAR_ENABLE: {
            int *enabled = va_arg(ap, int *);
            FLR_ENABLE_E enable = FLR_DISABLE;
            ret = (TLinearGetControl(&enable) == FLR_OK) ? 0 : -1;
            *enabled = (enable == FLR_ENABLE);
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_TEMP_STABLE_ENABLE: {
            int enable = va_arg(ap, int);
            ret = (radiometrySetTempStableEnable(enable ? FLR_ENABLE : FLR_DISABLE) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_TEMP_STABLE_ENABLE: {
            int *enabled = va_arg(ap, int *);
            FLR_ENABLE_E enable = FLR_DISABLE;
            ret = (radiometryGetTempStableEnable(&enable) == FLR_OK) ? 0 : -1;
            *enabled = (enable == FLR_ENABLE);
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_EMISSIVITY: {
            float emissivity = (float) va_arg(ap, double);
            ret = (radiometrySetEmissivityTarget(emissivity) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_EMISSIVITY: {
            float *emissivity = va_arg(ap, float *);
            ret = (radiometryGetEmissivityTarget(emissivity) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_TEMP_BACKGROUND: {
            float temp = (float) va_arg(ap, double);
            ret = (radiometrySetTempBackground(temp) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_TEMP_BACKGROUND: {
            float *temp = va_arg(ap, float *);
            ret = (radiometryGetTempBackground(temp) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPOT_METER_ENABLE: {
            int enable = va_arg(ap, int);
            ret = (spotMeterSetEnable(enable ? FLR_ENABLE : FLR_DISABLE) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_ENABLE: {
            int *enabled = va_arg(ap, int *);
            FLR_ENABLE_E enable = FLR_DISABLE;
            ret = (spotMeterGetEnable(&enable) == FLR_OK) ? 0 : -1;
            *enabled = (enable == FLR_ENABLE);
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPOT_METER_ROI: {
            int x = va_arg(ap, int);
            int y = va_arg(ap, int);
            int w = va_arg(ap, int);
            int h = va_arg(ap, int);
            FLR_ROI_T roi;
            roi.rowStart = y;
            roi.rowStop = y + h - 1;
            roi.colStart = x;
            roi.colStop = x + w - 1;
            ret = (spotMeterSetRoi(roi) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_ROI: {
            int *x = va_arg(ap, int *);
            int *y = va_arg(ap, int *);
            int *w = va_arg(ap, int *);
            int *h = va_arg(ap, int *);
            FLR_ROI_T roi = {0};
            ret = (spotMeterGetRoi(&roi) == FLR_OK) ? 0 : -1;
            *x = roi.colStart;
            *y = roi.rowStart;
            *w = roi.colStop - roi.colStart + 1;
            *h = roi.rowStop - roi.rowStart + 1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_ROI_MAX: {
            int *w = va_arg(ap, int *);
            int *h = va_arg(ap, int *);
            uint16_t width = 0;
            uint16_t height = 0;
            ret = (spotMeterGetRoiMaxSize(&width, &height) == FLR_OK) ? 0 : -1;
            *w = width;
            *h = height;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPOT_METER_MODE: {
            int mode = va_arg(ap, int);
            ret = (spotMeterSetStatsMode((FLR_SPOTMETER_STATS_TEMP_MODE_E) mode) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_MODE: {
            int *mode = va_arg(ap, int *);
            FLR_SPOTMETER_STATS_TEMP_MODE_E stats_mode = FLR_SPOTMETER_CELCIUS;
            ret = (spotMeterGetStatsMode(&stats_mode) == FLR_OK) ? 0 : -1;
            *mode = stats_mode;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_STATS: {
            int *mean = va_arg(ap, int *);
            int *deviation = va_arg(ap, int *);
            int *min_value = va_arg(ap, int *);
            int *min_x = va_arg(ap, int *);
            int *min_y = va_arg(ap, int *);
            int *max_value = va_arg(ap, int *);
            int *max_x = va_arg(ap, int *);
            int *max_y = va_arg(ap, int *);
            uint16_t mean16 = 0;
            uint16_t deviation16 = 0;
            FLR_SPOTMETER_SPOT_PARAM_T min = {0};
            FLR_SPOTMETER_SPOT_PARAM_T max = {0};
            ret = (spotMeterGetSpotStats(&mean16, &deviation16, &min, &max) == FLR_OK) ? 0 : -1;
            *mean = mean16;
            *deviation = deviation16;
            *min_value = min.value;
            *min_x = min.column;
            *min_y = min.row;
            *max_value = max.value;
            *max_x = max.column;
            *max_y = max.row;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_TEMP_STATS: {
            float *mean = va_arg(ap, float *);
            float *deviation = va_arg(ap, float *);
            float *min_value = va_arg(ap, float *);
            int *min_x = va_arg(ap, int *);
            int *min_y = va_arg(ap, int *);
            float *max_value = va_arg(ap, float *);
            int *max_x = va_arg(ap, int *);
            int *max_y = va_arg(ap, int *);
            FLR_SPOTMETER_STAT_PARAM_TEMP_T min = {0};
            FLR_SPOTMETER_STAT_PARAM_TEMP_T max = {0};
            ret = (spotMeterGetTempStats(mean, deviation, &min, &max) == FLR_OK) ? 0 : -1;
            *min_value = min.value;
            *min_x = min.column;
            *min_y = min.row;
            *max_value = max.value;
            *max_x = max.column;
            *max_y = max.row;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_TEMP_FROM_COUNTS: {
            int rbfo_type = va_arg(ap, int);
            int counts = va_arg(ap, int);
            float *temp = va_arg(ap, float *);
            ret = (radiometryGetTempFromCounts((FLR_RADIOMETRY_RBFO_TYPE_E) rbfo_type,
                                               counts, temp) == FLR_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_BOSON_GET_RBFO: {
            int rbfo_type = va_arg(ap, int);
            int low_gain = va_arg(ap, int);
            float *r = va_arg(ap, float *);
            float *b = va_arg(ap, float *);
            float *f = va_arg(ap, float *);
            float *o = va_arg(ap, float *);
            FLR_RESULT result;
            FLR_RADIOMETRY_RBFO_PARAMS_T params = {0};
            if (rbfo_type == FLR_RADIOMETRY_FACTORY_RBFO) {
                result = low_gain ? radiometryGetRBFOLowGainFactory(&params)
                                  : radiometryGetRBFOHighGainFactory(&params);
            } else {
                result = low_gain ? radiometryGetRBFOLowGainDefault(&params)
                                  : radiometryGetRBFOHighGainDefault(&params);
            }
            ret = (result == FLR_OK) ? 0 : -1;
            *r = params.RBFO_R;
            *b = params.RBFO_B;
            *f = params.RBFO_F;
            *o = params.RBFO_O;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int boson_init(omv_csi_t *csi) {
    // Initialize csi structure
    csi->reset = reset;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_colorbar = set_colorbar;
    csi->post_process = post_process;
    csi->ioctl = ioctl;

    // Set csi flags
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->mono_bpp = sizeof(uint8_t);

    // Override standard resolutions
    csi->resolution[OMV_CSI_FRAMESIZE_VGA][0] = 640;
    csi->resolution[OMV_CSI_FRAMESIZE_VGA][1] = 512;

    csi->resolution[OMV_CSI_FRAMESIZE_QVGA][0] = 320;
    csi->resolution[OMV_CSI_FRAMESIZE_QVGA][1] = 256;

    if (reset(csi) != 0) {
        return OMV_CSI_ERROR_CSI_INIT_FAILED;
    }

    csi->chip_id = (boson_framesize == OMV_CSI_FRAMESIZE_VGA) ? BOSON_640_ID : BOSON_320_ID;

    return 0;
}
#endif // (OMV_BOSON_ENABLE == 1)
