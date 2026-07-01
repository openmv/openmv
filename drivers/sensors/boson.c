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
    FSLP_set_csi(csi);

    switch (request) {
        case OMV_CSI_IOCTL_BOSON_GET_FPA_TEMP: {
            int16_t temp;
            if (bosonlookupFPATempDegCx10(&temp) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)temp;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_RUN_FFC: {
            if (bosonRunFFC() != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_STATUS: {
            FLR_BOSON_FFCSTATUS_E status;
            if (bosonGetFfcStatus(&status) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int) status;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_MODE: {
            int mode = va_arg(ap, int);
            if (bosonSetFFCMode((FLR_BOSON_FFCMODE_E) mode) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_GAIN_MODE: {
            int mode = va_arg(ap, int);
            if (bosonSetGainMode((FLR_BOSON_GAINMODE_E) mode) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_GAIN_MODE: {
            FLR_BOSON_GAINMODE_E mode;
            if (bosonGetGainMode(&mode) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)mode;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_AGC_PARAM: {
            int param_id = va_arg(ap, int);
            // va_arg promotes float to double
            float value = (float)va_arg(ap, double);
            FLR_RESULT ret;
            switch (param_id) {
                case 0: ret = agcSetLinearPercent(value); break;
                case 1: ret = agcSetd2br(value); break;
                case 2: ret = agcSetSigmaR(value); break;
                case 3: ret = agcSetMaxGain(value); break;
                case 4: ret = agcSetdf(value); break;
                case 5: ret = agcSetGamma(value); break;
                default: return -1;
            }
            return (ret == FLR_OK) ? 0 : -1;
        }
        case OMV_CSI_IOCTL_BOSON_GET_IMAGE_STATS: {
            uint16_t mean, peak, base;
            if (imageStatsGetImageStats(&mean, &peak, &base) != FLR_OK) {
                return -1;
            }
            uint16_t *out_mean = va_arg(ap, uint16_t *);
            uint16_t *out_peak = va_arg(ap, uint16_t *);
            uint16_t *out_base = va_arg(ap, uint16_t *);
            *out_mean = mean;
            *out_peak = peak;
            *out_base = base;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_STATS_ROI: {
            FLR_ROI_T roi;
            roi.rowStart = (uint16_t)va_arg(ap, int);
            roi.rowStop  = (uint16_t)va_arg(ap, int);
            roi.colStart = (uint16_t)va_arg(ap, int);
            roi.colStop  = (uint16_t)va_arg(ap, int);
            if (imageStatsSetROI(roi) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_ISOTHERM_ENABLE: {
            int en = va_arg(ap, int);
            if (isothermSetEnable(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_ISOTHERM_TEMPS: {
            int table = va_arg(ap, int);
            int32_t t1 = va_arg(ap, int32_t);
            int32_t t2 = va_arg(ap, int32_t);
            int32_t t3 = va_arg(ap, int32_t);
            int32_t t4 = va_arg(ap, int32_t);
            int32_t t5 = va_arg(ap, int32_t);
            if (isothermSetTemps((FLR_ISOTHERM_GAIN_E)table, t1, t2, t3, t4, t5) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_ISOTHERM_UNIT: {
            int unit = va_arg(ap, int);
            if (isothermSetUnit((FLR_ISOTHERM_UNIT_E)unit) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_GAIN_SWITCH: {
            FLR_BOSON_GAIN_SWITCH_PARAMS_T params;
            params.pHighToLowPercent = (uint32_t)va_arg(ap, int);
            params.cHighToLowPercent = (uint32_t)va_arg(ap, int);
            params.pLowToHighPercent = (uint32_t)va_arg(ap, int);
            params.hysteresisPercent = (uint32_t)va_arg(ap, int);
            if (bosonSetGainSwitchParams(params) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SERIAL: {
            uint32_t sn;
            if (bosonGetCameraSN(&sn) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = sn;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_PART_NUMBER: {
            // TODO: return the part number from top of file, use sizeof FLR_BOSON_PARTNUMBER_T
            // instead of hardcoding 20

            FLR_BOSON_PARTNUMBER_T pn;
            if (bosonGetCameraPN(&pn) != FLR_OK) return -1;
            char *out = va_arg(ap, char *);
            memcpy(out, pn.value, sizeof(FLR_BOSON_PARTNUMBER_T));
            return 0;
        }
        // Image Stats & Spot Meter IOCTLs
        case OMV_CSI_IOCTL_BOSON_GET_STATS_ROI: {
            FLR_ROI_T roi;
            if (imageStatsGetROI(&roi) != FLR_OK) {
                return -1;
            }
            uint16_t *out_rs = va_arg(ap, uint16_t *);
            uint16_t *out_re = va_arg(ap, uint16_t *);
            uint16_t *out_cs = va_arg(ap, uint16_t *);
            uint16_t *out_ce = va_arg(ap, uint16_t *);
            *out_rs = roi.rowStart;
            *out_re = roi.rowStop;
            *out_cs = roi.colStart;
            *out_ce = roi.colStop;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPOT_METER_ENABLE: {
            int en = va_arg(ap, int);
            if (spotMeterSetEnable(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_ENABLE: {
            FLR_ENABLE_E en;
            if (spotMeterGetEnable(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPOT_METER_ROI: {
            FLR_ROI_T roi;
            roi.rowStart = (uint16_t)va_arg(ap, int);
            roi.rowStop  = (uint16_t)va_arg(ap, int);
            roi.colStart = (uint16_t)va_arg(ap, int);
            roi.colStop  = (uint16_t)va_arg(ap, int);
            if (spotMeterSetRoi(roi) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_ROI: {
            FLR_ROI_T roi;
            if (spotMeterGetRoi(&roi) != FLR_OK) {
                return -1;
            }
            uint16_t *out_rs = va_arg(ap, uint16_t *);
            uint16_t *out_re = va_arg(ap, uint16_t *);
            uint16_t *out_cs = va_arg(ap, uint16_t *);
            uint16_t *out_ce = va_arg(ap, uint16_t *);
            *out_rs = roi.rowStart;
            *out_re = roi.rowStop;
            *out_cs = roi.colStart;
            *out_ce = roi.colStop;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_STATS: {
            uint16_t mean, deviation;
            FLR_SPOTMETER_SPOT_PARAM_T min_param, max_param;
            if (spotMeterGetSpotStats(&mean, &deviation, &min_param, &max_param) != FLR_OK) {
                return -1;
            }
            uint16_t *out_mean = va_arg(ap, uint16_t *);
            uint16_t *out_dev  = va_arg(ap, uint16_t *);
            uint16_t *out_min_val = va_arg(ap, uint16_t *);
            uint16_t *out_min_row = va_arg(ap, uint16_t *);
            uint16_t *out_min_col = va_arg(ap, uint16_t *);
            uint16_t *out_max_val = va_arg(ap, uint16_t *);
            uint16_t *out_max_row = va_arg(ap, uint16_t *);
            uint16_t *out_max_col = va_arg(ap, uint16_t *);
            *out_mean = mean;
            *out_dev  = deviation;
            *out_min_val = min_param.value;
            *out_min_row = min_param.row;
            *out_min_col = min_param.column;
            *out_max_val = max_param.value;
            *out_max_row = max_param.row;
            *out_max_col = max_param.column;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_TEMP_STATS: {
            float mean, deviation;
            FLR_SPOTMETER_STAT_PARAM_TEMP_T min_param, max_param;
            if (spotMeterGetTempStats(&mean, &deviation, &min_param, &max_param) != FLR_OK) {
                return -1;
            }
            float *out_mean = va_arg(ap, float *);
            float *out_dev  = va_arg(ap, float *);
            float *out_min_val = va_arg(ap, float *);
            uint16_t *out_min_row = va_arg(ap, uint16_t *);
            uint16_t *out_min_col = va_arg(ap, uint16_t *);
            float *out_max_val = va_arg(ap, float *);
            uint16_t *out_max_row = va_arg(ap, uint16_t *);
            uint16_t *out_max_col = va_arg(ap, uint16_t *);
            *out_mean = mean;
            *out_dev  = deviation;
            *out_min_val = min_param.value;
            *out_min_row = min_param.row;
            *out_min_col = min_param.column;
            *out_max_val = max_param.value;
            *out_max_row = max_param.row;
            *out_max_col = max_param.column;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_ROI_MAX: {
            uint16_t width, height;
            if (spotMeterGetRoiMaxSize(&width, &height) != FLR_OK) {
                return -1;
            }
            uint16_t *out_w = va_arg(ap, uint16_t *);
            uint16_t *out_h = va_arg(ap, uint16_t *);
            *out_w = width;
            *out_h = height;
            return 0;
        }
        // Isotherm Getters
        case OMV_CSI_IOCTL_BOSON_GET_ISOTHERM_ENABLE: {
            FLR_ENABLE_E en;
            if (isothermGetEnable(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_ISOTHERM_UNIT: {
            FLR_ISOTHERM_UNIT_E unit;
            if (isothermGetUnit(&unit) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)unit;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_ISOTHERM_TEMPS: {
            int table = va_arg(ap, int);
            int32_t t1, t2, t3, t4, t5;
            if (isothermGetTemps((FLR_ISOTHERM_GAIN_E)table, &t1, &t2, &t3, &t4, &t5) != FLR_OK) {
                return -1;
            }
            int32_t *out_t1 = va_arg(ap, int32_t *);
            int32_t *out_t2 = va_arg(ap, int32_t *);
            int32_t *out_t3 = va_arg(ap, int32_t *);
            int32_t *out_t4 = va_arg(ap, int32_t *);
            int32_t *out_t5 = va_arg(ap, int32_t *);
            *out_t1 = t1;
            *out_t2 = t2;
            *out_t3 = t3;
            *out_t4 = t4;
            *out_t5 = t5;
            return 0;
        }
        // Radiometry, Sensor Info & Overtemp
        case OMV_CSI_IOCTL_BOSON_GET_RADIOMETRY_CAPABLE: {
            FLR_ENABLE_E capable;
            if (radiometryGetRadiometryCapable(&capable) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)capable;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_RADIOMETRY_ENABLE: {
            int en = va_arg(ap, int);
            if (radiometrySetTempStableEnable(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_RADIOMETRY_ENABLE: {
            FLR_ENABLE_E en;
            if (radiometryGetTempStableEnable(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_TLINEAR_ENABLE: {
            int en = va_arg(ap, int);
            if (TLinearSetControl(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_TLINEAR_ENABLE: {
            FLR_ENABLE_E en;
            if (TLinearGetControl(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPOT_METER_STATS_MODE: {
            int mode = va_arg(ap, int);
            if (spotMeterSetStatsMode((FLR_SPOTMETER_STATS_TEMP_MODE_E)mode) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPOT_METER_STATS_MODE: {
            FLR_SPOTMETER_STATS_TEMP_MODE_E mode;
            if (spotMeterGetStatsMode(&mode) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)mode;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_EMISSIVITY: {
            // va_arg promotes float to double
            float val = (float)va_arg(ap, double);
            if (radiometrySetEmissivityTarget(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_EMISSIVITY: {
            float val;
            if (radiometryGetEmissivityTarget(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SOFTWARE_REV: {
            uint32_t major, minor, patch;
            if (bosonGetSoftwareRev(&major, &minor, &patch) != FLR_OK) {
                return -1;
            }
            uint32_t *out_major = va_arg(ap, uint32_t *);
            uint32_t *out_minor = va_arg(ap, uint32_t *);
            uint32_t *out_patch = va_arg(ap, uint32_t *);
            *out_major = major;
            *out_minor = minor;
            *out_patch = patch;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SENSOR_SN: {
            uint32_t sn;
            if (bosonGetSensorSN(&sn) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = sn;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_OVERTEMP_THRESHOLD: {
            float temp;
            if (bosonGetOverTempThreshold(&temp) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = temp;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_OVERTEMP_EVENT_COUNT: {
            uint32_t count;
            if (bosonGetOverTempEventCounter(&count) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = count;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_LOW_POWER_MODE: {
            uint16_t mode;
            if (bosonGetLowPowerMode(&mode) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)mode;
            return 0;
        }
        // FFC control IOCTLs
        case OMV_CSI_IOCTL_BOSON_GET_FFC_MODE: {
            FLR_BOSON_FFCMODE_E mode;
            if (bosonGetFFCMode(&mode) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)mode;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_TEMP_THRESHOLD: {
            int val = va_arg(ap, int);
            if (bosonSetFFCTempThreshold((uint16_t)val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_TEMP_THRESHOLD: {
            uint16_t val;
            if (bosonGetFFCTempThreshold(&val) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_FRAME_THRESHOLD: {
            uint32_t val = (uint32_t)va_arg(ap, int);
            if (bosonSetFFCFrameThreshold(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_FRAME_THRESHOLD: {
            uint32_t val;
            if (bosonGetFFCFrameThreshold(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_TEMP_THRESHOLD_LG: {
            int val = va_arg(ap, int);
            if (bosonSetFFCTempThresholdLowGain((uint16_t)val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_TEMP_THRESHOLD_LG: {
            uint16_t val;
            if (bosonGetFFCTempThresholdLowGain(&val) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_FRAME_THRESHOLD_LG: {
            uint32_t val = (uint32_t)va_arg(ap, int);
            if (bosonSetFFCFrameThresholdLowGain(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_FRAME_THRESHOLD_LG: {
            uint32_t val;
            if (bosonGetFFCFrameThresholdLowGain(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_WARN_TIME: {
            int val = va_arg(ap, int);
            if (bosonSetFFCWarnTimeInSecx10((uint16_t)val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_WARN_TIME: {
            uint16_t val;
            if (bosonGetFFCWarnTimeInSecx10(&val) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_DESIRED: {
            uint32_t val;
            if (bosonGetFfcDesired(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_LAST_FFC_FRAME_COUNT: {
            uint32_t val;
            if (bosonGetLastFFCFrameCount(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_LAST_FFC_TEMP: {
            uint16_t val;
            if (bosonGetLastFFCTempDegKx10(&val) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_STARTUP_FFC_PERIOD: {
            uint32_t val = (uint32_t)va_arg(ap, int);
            if (bosonSetTimeForQuickFFCsInSecs(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_STARTUP_FFC_PERIOD: {
            uint32_t val;
            if (bosonGetTimeForQuickFFCsInSecs(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        // FFC correction pipeline IOCTLs
        case OMV_CSI_IOCTL_BOSON_SET_SCNR_ENABLE: {
            int en = va_arg(ap, int);
            if (scnrSetEnableState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SCNR_ENABLE: {
            FLR_ENABLE_E en;
            if (scnrGetEnableState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_TF_ENABLE: {
            int en = va_arg(ap, int);
            if (tfSetEnableState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_TF_ENABLE: {
            FLR_ENABLE_E en;
            if (tfGetEnableState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SPNR_ENABLE: {
            int en = va_arg(ap, int);
            if (spnrSetEnableState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SPNR_ENABLE: {
            FLR_ENABLE_E en;
            if (spnrGetEnableState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_BPR_ENABLE: {
            int en = va_arg(ap, int);
            if (bprSetState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_BPR_ENABLE: {
            FLR_ENABLE_E en;
            if (bprGetState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_LFSR_ENABLE: {
            int en = va_arg(ap, int);
            if (lfsrSetApplyOffsetEnableState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_LFSR_ENABLE: {
            FLR_ENABLE_E en;
            if (lfsrGetApplyOffsetEnableState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SRNR_ENABLE: {
            int en = va_arg(ap, int);
            if (srnrSetEnableState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SRNR_ENABLE: {
            FLR_ENABLE_E en;
            if (srnrGetEnableState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_GAO_ENABLE: {
            int en = va_arg(ap, int);
            if (gaoSetAppliedClipEnable(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_GAO_ENABLE: {
            FLR_ENABLE_E en;
            if (gaoGetAppliedClipEnable(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_FFC_PIPE_STATE: {
            int en = va_arg(ap, int);
            if (gaoSetFfcState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_FFC_PIPE_STATE: {
            FLR_ENABLE_E en;
            if (gaoGetFfcState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_SFFC_STATE: {
            int en = va_arg(ap, int);
            if (gaoSetSffcState(en ? FLR_ENABLE : FLR_DISABLE) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_SFFC_STATE: {
            FLR_ENABLE_E en;
            if (gaoGetSffcState(&en) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)en;
            return 0;
        }
        // AGC control IOCTLs
        case OMV_CSI_IOCTL_BOSON_SET_AGC_MODE: {
            int mode = va_arg(ap, int);
            if (agcSetMode((FLR_AGC_MODE_E)mode) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_MODE: {
            FLR_AGC_MODE_E mode;
            if (agcGetMode(&mode) != FLR_OK) {
                return -1;
            }
            int *out = va_arg(ap, int *);
            *out = (int)mode;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_AGC_PERCENT_PER_BIN: {
            float val = (float)va_arg(ap, double);
            if (agcSetPercentPerBin(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_PERCENT_PER_BIN: {
            float val;
            if (agcGetPercentPerBin(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_AGC_OUTLIER_CUT: {
            float val = (float)va_arg(ap, double);
            if (agcSetOutlierCut(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_OUTLIER_CUT: {
            float val;
            if (agcGetOutlierCut(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_AGC_DETAIL_HEADROOM: {
            float val = (float)va_arg(ap, double);
            if (agcSetDetailHeadroom(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_DETAIL_HEADROOM: {
            float val;
            if (agcGetDetailHeadroom(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_LINEAR_PERCENT: {
            float val;
            if (agcGetLinearPercent(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_D2BR: {
            float val;
            if (agcGetd2br(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_SIGMA_R: {
            float val;
            if (agcGetSigmaR(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_MAX_GAIN: {
            float val;
            if (agcGetMaxGain(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_DF: {
            float val;
            if (agcGetdf(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_AGC_GAMMA: {
            float val;
            if (agcGetGamma(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        // Dynamic range control IOCTLs
        case OMV_CSI_IOCTL_BOSON_GET_GAIN_SWITCH_PARAMS: {
            FLR_BOSON_GAIN_SWITCH_PARAMS_T params;
            if (bosonGetGainSwitchParams(&params) != FLR_OK) {
                return -1;
            }
            uint32_t *out_ht = va_arg(ap, uint32_t *);
            uint32_t *out_hp = va_arg(ap, uint32_t *);
            uint32_t *out_lp = va_arg(ap, uint32_t *);
            uint32_t *out_hy = va_arg(ap, uint32_t *);
            *out_ht = params.pHighToLowPercent;
            *out_hp = params.cHighToLowPercent;
            *out_lp = params.pLowToHighPercent;
            *out_hy = params.hysteresisPercent;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_GAIN_SWITCH_HYST_TIME: {
            float val = (float)va_arg(ap, double);
            if (bosonSetGainSwitchHysteresisTime(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_GAIN_SWITCH_HYST_TIME: {
            float val;
            if (bosonGetGainSwitchHysteresisTime(&val) != FLR_OK) {
                return -1;
            }
            float *out = va_arg(ap, float *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_SET_GAIN_SWITCH_FRAME_THRESH: {
            uint32_t val = (uint32_t)va_arg(ap, int);
            if (bosonSetGainSwitchFrameThreshold(val) != FLR_OK) {
                return -1;
            }
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_GAIN_SWITCH_FRAME_THRESH: {
            uint32_t val;
            if (bosonGetGainSwitchFrameThreshold(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        case OMV_CSI_IOCTL_BOSON_GET_GAIN_SWITCH_DESIRED: {
            uint32_t val;
            if (bosonGetGainSwitchDesired(&val) != FLR_OK) {
                return -1;
            }
            uint32_t *out = va_arg(ap, uint32_t *);
            *out = val;
            return 0;
        }
        default:
            return -1;
    }
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
