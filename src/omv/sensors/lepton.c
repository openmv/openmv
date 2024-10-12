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

extern uint16_t _vospi_buf[];
static lepton_state_t lepton;

static int lepton_reset(omv_csi_t *csi, bool measurement_mode, bool high_temp_mode);

static int sleep(omv_csi_t *csi, int enable) {
    if (enable) {
        omv_gpio_write(OMV_CSI_POWER_PIN, 0);
        mp_hal_delay_ms(100);
    } else {
        omv_gpio_write(OMV_CSI_POWER_PIN, 1);
        mp_hal_delay_ms(100);
    }

    return 0;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint16_t reg_data;
    if (omv_i2c_readw2(&csi->i2c_bus, csi->slv_addr, reg_addr, &reg_data)) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return omv_i2c_writew2(&csi->i2c_bus, csi->slv_addr, reg_addr, reg_data);
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
            *resolution = 14;
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
    omv_gpio_write(OMV_CSI_POWER_PIN, 0);
    mp_hal_delay_ms(10);

    omv_gpio_write(OMV_CSI_POWER_PIN, 1);
    mp_hal_delay_ms(10);

    omv_gpio_write(OMV_CSI_RESET_PIN, 0);
    mp_hal_delay_ms(10);

    omv_gpio_write(OMV_CSI_RESET_PIN, 1);
    mp_hal_delay_ms(1000);

    LEP_RAD_ENABLE_E rad;
    LEP_AGC_ROI_T roi;
    memset(&lepton.port, 0, sizeof(LEP_CAMERA_PORT_DESC_T));

    for (mp_uint_t start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(1)) {
        if (LEP_OpenPort(&csi->i2c_bus, LEP_CCI_TWI, 0, &lepton.port) == LEP_OK) {
            break;
        }
        if ((mp_hal_ticks_ms() - start) >= LEPTON_BOOT_TIMEOUT) {
            return -1;
        }
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

static int reset(omv_csi_t *csi) {
    static bool vospi_initialized = false;

    memset(&lepton, 0, sizeof(lepton_state_t));
    lepton.min_temp = LEPTON_MIN_TEMP_DEFAULT;
    lepton.max_temp = LEPTON_MAX_TEMP_DEFAULT;

    if (lepton_reset(csi, false, false) != 0) {
        return -1;
    }

    if (vospi_initialized == false) {
        if (vospi_init(lepton.v_res, _vospi_buf) != 0) {
            return -1;
        }
        vospi_initialized = true;
    }

    return 0;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    framebuffer_update_jpeg_buffer();

    if (MAIN_FB()->n_buffers != 1) {
        framebuffer_set_buffers(1);
    }

    if (omv_csi_check_framebuffer_size(csi) == -1) {
        return -1;
    }

    if ((!lepton.h_res) || (!lepton.v_res) || (!csi->framesize) || (!csi->pixformat)) {
        return -1;
    }

    framebuffer_free_current_buffer();
    vbuffer_t *buffer = framebuffer_get_tail(FB_NO_FLAGS);

    if (!buffer) {
        return -1;
    }

    for (int i = 0; i < LEPTON_SNAPSHOT_RETRY; i++) {
        if (vospi_snapshot(LEPTON_SNAPSHOT_TIMEOUT) == 0) {
            break;
        }
        if (i + 1 == LEPTON_SNAPSHOT_RETRY) {
            return -1;
        }
        // The FLIR lepton might have crashed so reset it (it does this).
        if (lepton_reset(csi, lepton.measurement_mode, lepton.high_temp_mode) != 0) {
            return -1;
        }
    }

    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;
    MAIN_FB()->pixfmt = csi->pixformat;

    framebuffer_init_image(image);

    float x_scale = resolution[csi->framesize][0] / ((float) lepton.h_res);
    float y_scale = resolution[csi->framesize][1] / ((float) lepton.v_res);
    // MAX == KeepAspectRationByExpanding - MIN == KeepAspectRatio
    float scale = IM_MAX(x_scale, y_scale), scale_inv = 1.0f / scale;
    int x_offset = (resolution[csi->framesize][0] - (lepton.h_res * scale)) / 2;
    int y_offset = (resolution[csi->framesize][1] - (lepton.v_res * scale)) / 2;
    // The code below upscales the source image to the requested frame size
    // and then crops it to the window set by the user.

    LEP_SYS_FPA_TEMPERATURE_KELVIN_T kelvin;
    if (lepton.measurement_mode && (!lepton.radiometry)) {
        if (LEP_GetSysFpaTemperatureKelvin(&lepton.port, &kelvin) != LEP_OK) {
            return -1;
        }
    }

    for (int y = y_offset, yy = fast_ceilf(lepton.v_res * scale) + y_offset; y < yy; y++) {
        if ((MAIN_FB()->y <= y) && (y < (MAIN_FB()->y + MAIN_FB()->v))) {
            // user window cropping

            uint16_t *row_ptr = _vospi_buf + (fast_floorf(y * scale_inv) * lepton.h_res);

            for (int x = x_offset, xx = fast_ceilf(lepton.h_res * scale) + x_offset; x < xx; x++) {
                if ((MAIN_FB()->x <= x) && (x < (MAIN_FB()->x + MAIN_FB()->u))) {
                    // user window cropping

                    // Value is the 14/16-bit value from the FLIR IR camera.
                    // However, with AGC enabled only the bottom 8-bits are non-zero.
                    int value = row_ptr[fast_floorf(x * scale_inv)];

                    if (lepton.measurement_mode) {
                        // Need to convert 14/16-bits to 8-bits ourselves...
                        if (!lepton.radiometry) {
                            value = (value - 8192) + kelvin;
                        }
                        float celsius = (value * 0.01f) - 273.15f;
                        celsius = IM_CLAMP(celsius, lepton.min_temp, lepton.max_temp);
                        value = __USAT(IM_DIV(((celsius - lepton.min_temp) * 255),
                                              (lepton.max_temp - lepton.min_temp)), 8);
                    }

                    int t_x = x - MAIN_FB()->x;
                    int t_y = y - MAIN_FB()->y;

                    if (lepton.hmirror) {
                        t_x = MAIN_FB()->u - t_x - 1;
                    }
                    if (lepton.vflip) {
                        t_y = MAIN_FB()->v - t_y - 1;
                    }

                    switch (csi->pixformat) {
                        case PIXFORMAT_GRAYSCALE: {
                            IMAGE_PUT_GRAYSCALE_PIXEL(image, t_x, t_y, value & 0xFF);
                            break;
                        }
                        case PIXFORMAT_RGB565: {
                            IMAGE_PUT_RGB565_PIXEL(image, t_x, t_y, csi->color_palette[value & 0xFF]);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int lepton_init(omv_csi_t *csi) {
    csi->reset = reset;
    csi->sleep = sleep;
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

    csi->vsync_pol = 1;
    csi->hsync_pol = 0;
    csi->pixck_pol = 0;
    csi->frame_sync = 0;
    csi->mono_bpp = 1;

    if (reset(csi) != 0) {
        return -1;
    }

    LEP_OEM_PART_NUMBER_T part;
    if (LEP_GetOemFlirPartNumber(&lepton.port, &part) != LEP_OK) {
        return -1;
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
