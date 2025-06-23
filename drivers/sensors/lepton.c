/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * Lepton driver.
 */
#include "omv_boardconfig.h"
#if (OMV_LEPTON_ENABLE == 1)

#include <stdio.h>
#include "omv_csi.h"
#include "vospi.h"
#include "py/mphal.h"
#include "omv_common.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "framebuffer.h"

#include "LEPTON_SDK.h"
#include "LEPTON_AGC.h"
#include "LEPTON_SYS.h"
#include "LEPTON_VID.h"
#include "LEPTON_OEM.h"
#include "LEPTON_RAD.h"
#include "LEPTON_I2C_Reg.h"

#define LEPTON_BOOT_TIMEOUT        (1000)
#define LEPTON_SNAPSHOT_RETRY      (3)
#define LEPTON_SNAPSHOT_TIMEOUT    (10000)

// Min/Max temperatures in Celsius.
#define LEPTON_MIN_TEMP_NORM       (-10.0f)
#define LEPTON_MIN_TEMP_HIGH       (-10.0f)
#define LEPTON_MIN_TEMP_DEFAULT    (-10.0f)

#define LEPTON_MAX_TEMP_NORM       (140.0f)
#define LEPTON_MAX_TEMP_HIGH       (600.0f)
#define LEPTON_MAX_TEMP_DEFAULT    (40.0f)

typedef struct lepton_state {
    int h_res;
    int v_res;
    bool vflip;
    bool hmirror;
    float min_temp;
    float max_temp;
    bool radiometry;
    bool high_temp_mode;
    bool measurement_mode;
    LEP_CAMERA_PORT_DESC_T port;
} lepton_state_t;

static lepton_state_t lepton;

static int lepton_reset(omv_csi_t *csi, bool measurement_mode, bool high_temp_mode);

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint16_t reg_data;
    if (omv_i2c_readw2(csi->i2c, csi->slv_addr, reg_addr, &reg_data)) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return omv_i2c_writew2(csi->i2c, csi->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    return ((pixformat != PIXFORMAT_GRAYSCALE) && (pixformat != PIXFORMAT_RGB565)) ? -1 : 0;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    return 0;
}

static int set_contrast(omv_csi_t *csi, int level) {
    return 0;
}

static int set_brightness(omv_csi_t *csi, int level) {
    return 0;
}

static int set_saturation(omv_csi_t *csi, int level) {
    return 0;
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    return 0;
}

static int set_quality(omv_csi_t *csi, int quality) {
    return 0;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    return 0;
}

static int set_special_effect(omv_csi_t *csi, omv_csi_sde_t sde) {
    return 0;
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    return 0;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    return 0;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    return 0;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    return 0;
}

static int set_auto_whitebal(omv_csi_t *csi, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    return 0;
}

static int get_rgb_gain_db(omv_csi_t *csi, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    lepton.hmirror = enable;
    return 0;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    lepton.vflip = enable;
    return 0;
}

static int set_lens_correction(omv_csi_t *csi, int enable, int radi, int coef) {
    return 0;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;

    if ((!lepton.h_res) || (!lepton.v_res)) {
        return -1;
    }

    switch (request) {
        case OMV_CSI_IOCTL_LEPTON_GET_WIDTH: {
            int *width = va_arg(ap, int *);
            *width = lepton.h_res;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_HEIGHT: {
            int *height = va_arg(ap, int *);
            *height = lepton.v_res;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_RADIOMETRY: {
            int *type = va_arg(ap, int *);
            *type = lepton.radiometry;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_REFRESH: {
            int *refresh = va_arg(ap, int *);
            *refresh = (lepton.h_res == 80) ? 27 : 9;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_RESOLUTION: {
            int *resolution = va_arg(ap, int *);
            *resolution =  lepton.radiometry ? 16 : 14;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_RUN_COMMAND: {
            int command = va_arg(ap, int);
            ret = (LEP_RunCommand(&lepton.port, command) == LEP_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_SET_ATTRIBUTE: {
            int command = va_arg(ap, int);
            uint16_t *data = va_arg(ap, uint16_t *);
            size_t data_len = va_arg(ap, size_t);
            ret = (LEP_SetAttribute(&lepton.port, command, (LEP_ATTRIBUTE_T_PTR) data, data_len) == LEP_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_ATTRIBUTE: {
            int command = va_arg(ap, int);
            uint16_t *data = va_arg(ap, uint16_t *);
            size_t data_len = va_arg(ap, size_t);
            ret = (LEP_GetAttribute(&lepton.port, command, (LEP_ATTRIBUTE_T_PTR) data, data_len) == LEP_OK) ? 0 : -1;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_FPA_TEMP: {
            int *temp = va_arg(ap, int *);
            LEP_SYS_FPA_TEMPERATURE_KELVIN_T tfpa;
            ret = (LEP_GetSysFpaTemperatureKelvin(&lepton.port, &tfpa) == LEP_OK) ? 0 : -1;
            *temp = tfpa;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_AUX_TEMP: {
            int *temp = va_arg(ap, int *);
            LEP_SYS_AUX_TEMPERATURE_KELVIN_T taux;
            ret = (LEP_GetSysAuxTemperatureKelvin(&lepton.port, &taux) == LEP_OK) ? 0 : -1;
            *temp = taux;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_SET_MODE: {
            int measurement_mode_in = va_arg(ap, int);
            int high_temp_mode_in = va_arg(ap, int);
            if (lepton.measurement_mode != measurement_mode_in) {
                lepton.measurement_mode = measurement_mode_in;
                lepton.high_temp_mode = high_temp_mode_in;
                ret = lepton_reset(csi, lepton.measurement_mode, lepton.high_temp_mode);
            }
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_MODE: {
            int *measurement_mode_out = va_arg(ap, int *);
            int *high_temp_mode_out = va_arg(ap, int *);
            *measurement_mode_out = lepton.measurement_mode;
            *high_temp_mode_out = lepton.high_temp_mode;
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_SET_RANGE: {
            float *arg_min_temp = va_arg(ap, float *);
            float *arg_max_temp = va_arg(ap, float *);
            float min_temp_range = (lepton.high_temp_mode) ? LEPTON_MIN_TEMP_HIGH : LEPTON_MIN_TEMP_NORM;
            float max_temp_range = (lepton.high_temp_mode) ? LEPTON_MAX_TEMP_HIGH : LEPTON_MAX_TEMP_NORM;
            // Don't use clamp here, the order of comparison is important.
            lepton.min_temp = IM_MAX(IM_MIN(*arg_min_temp, *arg_max_temp), min_temp_range);
            lepton.max_temp = IM_MIN(IM_MAX(*arg_max_temp, *arg_min_temp), max_temp_range);
            break;
        }
        case OMV_CSI_IOCTL_LEPTON_GET_RANGE: {
            float *ptr_min_temp = va_arg(ap, float *);
            float *ptr_max_temp = va_arg(ap, float *);
            *ptr_min_temp = lepton.min_temp;
            *ptr_max_temp = lepton.max_temp;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

static int lepton_reset(omv_csi_t *csi, bool measurement_mode, bool high_temp_mode) {
    LEP_RAD_ENABLE_E rad;
    LEP_AGC_ROI_T roi;
    memset(&lepton.port, 0, sizeof(LEP_CAMERA_PORT_DESC_T));

    if (!csi->auxiliary) {
        omv_gpio_write(OMV_CSI_POWER_PIN, 0);
        mp_hal_delay_ms(10);

        omv_gpio_write(OMV_CSI_POWER_PIN, 1);
        mp_hal_delay_ms(10);

        omv_gpio_write(OMV_CSI_RESET_PIN, 0);
        mp_hal_delay_ms(10);

        omv_gpio_write(OMV_CSI_RESET_PIN, 1);
        mp_hal_delay_ms(1000);
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(1)) {
        if (LEP_OpenPort(csi->i2c, LEP_CCI_TWI, 0, &lepton.port) == LEP_OK) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= LEPTON_BOOT_TIMEOUT) {
            return -1;
        }
    }

    if (csi->auxiliary) {
        LEP_RunOemReboot(&lepton.port);
        mp_hal_delay_ms(1000);
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(1)) {
        LEP_SDK_BOOT_STATUS_E status;
        if (LEP_GetCameraBootStatus(&lepton.port, &status) != LEP_OK) {
            return -1;
        }

        if (status == LEP_BOOT_STATUS_BOOTED) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= LEPTON_BOOT_TIMEOUT) {
            return -1;
        }
    }

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(1)) {
        LEP_UINT16 status;
        if (LEP_DirectReadRegister(&lepton.port, LEP_I2C_STATUS_REG, &status) != LEP_OK) {
            return -1;
        }

        if (!(status & LEP_I2C_STATUS_BUSY_BIT_MASK)) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= LEPTON_BOOT_TIMEOUT) {
            return -1;
        }
    }

    if (LEP_GetRadEnableState(&lepton.port, &rad) != LEP_OK
        || LEP_GetAgcROI(&lepton.port, &roi) != LEP_OK) {
        return -1;
    }

    // Use the low gain mode to enable high temperature readings (~450C) on Lepton 3.5
    LEP_SYS_GAIN_MODE_E gain_mode = lepton.high_temp_mode ? LEP_SYS_GAIN_MODE_LOW : LEP_SYS_GAIN_MODE_HIGH;
    if (LEP_SetSysGainMode(&lepton.port, gain_mode) != LEP_OK) {
        return -1;
    }

    if (!lepton.measurement_mode) {
        if (LEP_SetRadEnableState(&lepton.port, LEP_RAD_DISABLE) != LEP_OK
            || LEP_SetAgcEnableState(&lepton.port, LEP_AGC_ENABLE) != LEP_OK
            || LEP_SetAgcCalcEnableState(&lepton.port, LEP_AGC_ENABLE) != LEP_OK) {
            return -1;
        }
    }

    lepton.h_res = roi.endCol + 1;
    lepton.v_res = roi.endRow + 1;
    lepton.radiometry = (rad == LEP_RAD_ENABLE);
    return 0;
}

static int sleep(omv_csi_t *csi, int enable) {
    return 0;
}

static int match(omv_csi_t *csi, size_t id) {
    return (id == LEPTON_ID) || ((id >> 8) == LEPTON_ID);
}

static int reset(omv_csi_t *csi) {
    vospi_deinit();

    memset(&lepton, 0, sizeof(lepton_state_t));
    lepton.min_temp = LEPTON_MIN_TEMP_DEFAULT;
    lepton.max_temp = LEPTON_MAX_TEMP_DEFAULT;

    // Extra delay after power-on
    mp_hal_delay_ms(1000);

    if (lepton_reset(csi, false, false) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    if (csi->fb && vospi_init(lepton.v_res, csi->fb) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    framebuffer_t *fb = csi->fb;

    if (flags & OMV_CSI_CAPTURE_FLAGS_UPDATE) {
        framebuffer_update_jpeg_buffer(fb);
    }

    if (csi->pixformat == PIXFORMAT_INVALID) {
        return OMV_CSI_ERROR_INVALID_PIXFORMAT;
    }

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    if (!lepton.h_res || !lepton.v_res) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    if (resolution[csi->framesize][0] < lepton.h_res || resolution[csi->framesize][1] < lepton.v_res) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    if (omv_csi_check_framebuffer_size(csi) == -1) {
        return OMV_CSI_ERROR_FRAMEBUFFER_OVERFLOW;
    }

    for (int i = 0; i < LEPTON_SNAPSHOT_RETRY; i++) {
        if (vospi_snapshot(LEPTON_SNAPSHOT_TIMEOUT) == 0) {
            break;
        }
        if (i + 1 == LEPTON_SNAPSHOT_RETRY) {
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
        // The FLIR lepton might have crashed so reset it (it does this).
        if (lepton_reset(csi, lepton.measurement_mode, lepton.high_temp_mode) != 0) {
            return OMV_CSI_ERROR_CTL_FAILED;
        }
    }

    if (!csi->transpose) {
        fb->w = fb->u;
        fb->h = fb->v;
    } else {
        fb->w = fb->v;
        fb->h = fb->u;
    }

    fb->pixfmt = csi->pixformat;

    image_t fb_image; 
    framebuffer_init_image(fb, &fb_image);

    LEP_SYS_FPA_TEMPERATURE_KELVIN_T kelvin;
    if (lepton.measurement_mode && (!lepton.radiometry)) {
        if (LEP_GetSysFpaTemperatureKelvin(&lepton.port, &kelvin) != LEP_OK) {
            return OMV_CSI_ERROR_IO_ERROR;
        }
    }
    
    fb_alloc_mark();
    image_t temp = {
        .w = (!csi->transpose) ? lepton.h_res : lepton.v_res,
        .h = (!csi->transpose) ? lepton.v_res : lepton.h_res,
        .pixfmt = PIXFORMAT_GRAYSCALE,
        .data = fb_alloc(lepton.h_res * lepton.v_res, FB_ALLOC_CACHE_ALIGN),
    };

    // When not in measurment mode set the min and max temperatures such that 0-255 values from the
    // sensor, which are grayscale pixels 0-255, are not clipped when interpreted as Kelvin values.
    float min = -273.15f; // in Celsius -> 0.0f in Kelvin
    float max = -270.6f; // in Celsius -> 2.55f in Kelvin

    // When in measurment mode the lepton provides 14-bit or 16-bit values that must be clamped
    // between the min and max temperatures in celsius and scaled to 0-255 values.
    if (lepton.measurement_mode) {
        min = lepton.min_temp;
        max = lepton.max_temp;
    }

    imlib_fill_image_from_lepton(&temp, lepton.h_res, lepton.v_res, (uint16_t *) fb_image.data, min, max,
                                 false, (!lepton.measurement_mode) || lepton.radiometry, kelvin,
                                 lepton.hmirror, lepton.vflip, csi->transpose);

    imlib_draw_image(&fb_image, &temp, 0, 0, 1.0f, 1.0f, NULL, -1, 255,
                     (csi->pixformat == PIXFORMAT_RGB565) ? csi->color_palette : NULL, NULL,
                     IMAGE_HINT_BILINEAR | IMAGE_HINT_CENTER | IMAGE_HINT_SCALE_ASPECT_EXPAND,
                     NULL, NULL, NULL);

    fb_alloc_free_till_mark();
    framebuffer_init_image(fb, image);
    return 0;
}

int lepton_init(omv_csi_t *csi) {
    csi->reset = reset;
    csi->sleep = sleep;
    csi->config = NULL;
    csi->abort = NULL;
    csi->match = match;
    csi->snapshot = snapshot;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_contrast = set_contrast;
    csi->set_brightness = set_brightness;
    csi->set_saturation = set_saturation;
    csi->set_gainceiling = set_gainceiling;
    csi->set_quality = set_quality;
    csi->set_colorbar = set_colorbar;
    csi->set_special_effect = set_special_effect;
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_auto_whitebal = set_auto_whitebal;
    csi->get_rgb_gain_db = get_rgb_gain_db;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->set_lens_correction = set_lens_correction;
    csi->ioctl = ioctl;

    csi->auxiliary = 1;
    csi->vsync_pol = 1;
    csi->hsync_pol = 0;
    csi->pixck_pol = 0;
    csi->frame_sync = 0;
    csi->mono_bpp = 1;

    // Extra delay after power-on
    mp_hal_delay_ms(1000);

    if (reset(csi) != 0) {
        return -1;
    }

    LEP_OEM_PART_NUMBER_T part;
    if (LEP_GetOemFlirPartNumber(&lepton.port, &part) != LEP_OK) {
        return OMV_CSI_ERROR_CSI_INIT_FAILED;
    }

    // 500 == Lepton
    // xxxx == Version
    // 01/00 == Shutter/NoShutter
    if (!strncmp(part.value, "500-0771", 8)) {
        csi->chip_id = LEPTON_3_5;
    } else if (!strncmp(part.value, "500-0726", 8)) {
        csi->chip_id = LEPTON_3_0;
    } else if (!strncmp(part.value, "500-0763", 8)) {
        csi->chip_id = LEPTON_2_5;
    } else if (!strncmp(part.value, "500-0659", 8)) {
        csi->chip_id = LEPTON_2_0;
    } else if (!strncmp(part.value, "500-0690", 8)) {
        csi->chip_id = LEPTON_1_6;
    } else if (!strncmp(part.value, "500-0643", 8)) {
        csi->chip_id = LEPTON_1_5;
    }
    return 0;
}
#endif // (OMV_LEPTON_ENABLE == 1)
