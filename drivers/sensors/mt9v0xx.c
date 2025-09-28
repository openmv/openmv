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
 * MT9V0XX driver.
 */
#include "omv_boardconfig.h"
#if (OMV_MT9V0XX_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "omv_i2c.h"
#include "omv_csi.h"
#include "mt9v0xx.h"
#include "py/mphal.h"

#define ACTIVE_SENSOR_WIDTH     (752)
#define ACTIVE_SENSOR_HEIGHT    (480)

#define MONO_CFA_ID             (0)
#define RCCC_CFA_ID             (5)
#define BAYER_CFA_ID            (6)

static int16_t readout_x = 0;
static int16_t readout_y = 0;

static enum {
    MONO_CFA, RCCC_CFA, BAYER_CFA
} cfa_type = MONO_CFA;

static bool is_mt9v0x2(omv_csi_t *csi) {
    return (csi->chip_id == MT9V0X2_ID) || (csi->chip_id == MT9V0X2_C_ID);
}

static bool is_mt9v0x4(omv_csi_t *csi) {
    return (csi->chip_id == MT9V0X4_ID) || (csi->chip_id == MT9V0X4_C_ID);
}

static int reset(omv_csi_t *csi) {
    int ret = 0;
    readout_x = 0;
    readout_y = 0;

    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_RESET, MT9V0XX_RESET_SOFT_RESET);

    if (is_mt9v0x4(csi)) {
        uint16_t chip_control;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL,
                              (chip_control & (~MT9V0X4_CHIP_CONTROL_RESERVED)));
    }

    uint16_t read_mode;
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_READ_MODE, &read_mode);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_READ_MODE,
                          read_mode | MT9V0XX_READ_MODE_ROW_FLIP | MT9V0XX_READ_MODE_COL_FLIP);

    if (is_mt9v0x4(csi)) {
        // We have to copy the differences from context A into context B registers so that we can
        // ping-pong between them seamlessly...

        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0X4_READ_MODE_B, &read_mode);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_READ_MODE_B,
                              read_mode | MT9V0XX_READ_MODE_ROW_FLIP | MT9V0XX_READ_MODE_COL_FLIP);

        uint16_t shutter_width1;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_SHUTTER_WIDTH1, &shutter_width1);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_SHUTTER_WIDTH1_B, shutter_width1);

        uint16_t shutter_width2;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_SHUTTER_WIDTH2, &shutter_width2);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_SHUTTER_WIDTH2_B, shutter_width2);

        uint16_t shutter_control;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_SHUTTER_WIDTH_CONTROL, &shutter_control);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_SHUTTER_WIDTH_CONTROL_B, shutter_control);

        uint16_t voltage_level_1;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_V1_CONTROL, &voltage_level_1);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_V1_CONTROL_B, voltage_level_1);

        uint16_t voltage_level_2;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_V2_CONTROL, &voltage_level_2);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_V2_CONTROL_B, voltage_level_2);

        uint16_t voltage_level_3;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_V3_CONTROL, &voltage_level_3);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_V3_CONTROL_B, voltage_level_3);

        uint16_t voltage_level_4;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_V4_CONTROL, &voltage_level_4);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_V4_CONTROL_B, voltage_level_4);

        uint16_t analog_gain;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_ANALOG_GAIN, &analog_gain);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_ANALOG_GAIN_B, analog_gain);

        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_PIXEL_OPERATION_MODE,
                              0);

        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_ADC_COMPANDING_MODE,
                              MT9V0XX_ADC_COMPANDING_MODE_LINEAR | MT9V0X4_ADC_COMPANDING_MODE_LINEAR_B);

        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_ROW_NOISE_CORR_CONTROL,
                              MT9V0X4_ROW_NOISE_CORR_ENABLE | MT9V0X4_ROW_NOISE_CORR_ENABLE_B);

        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE,
                              MT9V0XX_AEC_ENABLE | MT9V0X4_AEC_ENABLE_B | MT9V0XX_AGC_ENABLE | MT9V0X4_AGC_ENABLE_B);
    }

    if (is_mt9v0x2(csi)) {
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X2_PIXEL_CLOCK, MT9V0XX_PIXEL_CLOCK_INV_PXL_CLK);
    }

    return ret;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint16_t reg_data;
    if (omv_i2c_readw(csi->i2c, csi->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return omv_i2c_writew(csi->i2c, csi->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    switch (cfa_type) {
        case BAYER_CFA: {
            if (pixformat == PIXFORMAT_RGB565 ||
                pixformat == PIXFORMAT_BAYER ||
                pixformat == PIXFORMAT_GRAYSCALE) {
                return 0;
            }
            return -1;
        }
        default: {
            if (pixformat != PIXFORMAT_GRAYSCALE) {
                return -1;
            }
            return 0;
        }
    }
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    uint16_t chip_control, read_mode;
    int ret = 0;
    uint16_t w = csi->resolution[framesize][0];
    uint16_t h = csi->resolution[framesize][1];

    if ((w > ACTIVE_SENSOR_WIDTH) || (h > ACTIVE_SENSOR_HEIGHT)) {
        return -1;
    }

    if ((cfa_type == BAYER_CFA) && (w % 16)) {
        // Must be a multiple of 16 in bayer mode.
        return -1;
    }

    if (omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control) != 0) {
        return -1;
    }

    // EDIT: WORKS BETTER TO STAY IN CONTEXT A
    //
    // Determine which context to switch to...
    int context = 1; // chip_control & MT9V0X4_CHIP_CONTROL_CONTEXT;
    int read_mode_addr = context ? MT9V0XX_READ_MODE : MT9V0X4_READ_MODE_B;
    int col_start_addr = context ? MT9V0XX_COL_START : MT9V0X4_COL_START_B;
    int row_start_addr = context ? MT9V0XX_ROW_START : MT9V0X4_ROW_START_B;
    int window_height_addr = context ? MT9V0XX_WINDOW_HEIGHT : MT9V0X4_WINDOW_HEIGHT_B;
    int window_width_addr = context ? MT9V0XX_WINDOW_WIDTH : MT9V0X4_WINDOW_WIDTH_B;
    int horizontal_blanking_addr = context ? MT9V0XX_HORIZONTAL_BLANKING : MT9V0X4_HORIZONTAL_BLANKING_B;

    if (omv_i2c_readw(csi->i2c, csi->slv_addr, read_mode_addr, &read_mode) != 0) {
        return -1;
    }

    int read_mode_mul = 1;
    read_mode &= 0xFFF0;

    if (cfa_type != BAYER_CFA) {
        if ((w <= (ACTIVE_SENSOR_WIDTH / 4)) && (h <= (ACTIVE_SENSOR_HEIGHT / 4))) {
            read_mode_mul = 4;
            read_mode |= MT9V0XX_READ_MODE_COL_BIN_4 | MT9V0XX_READ_MODE_ROW_BIN_4;
        } else if ((w <= (ACTIVE_SENSOR_WIDTH / 2)) && (h <= (ACTIVE_SENSOR_HEIGHT / 2))) {
            read_mode_mul = 2;
            read_mode |= MT9V0XX_READ_MODE_COL_BIN_2 | MT9V0XX_READ_MODE_ROW_BIN_2;
        }
    }

    int readout_x_max = (ACTIVE_SENSOR_WIDTH - (w * read_mode_mul)) / 2;
    int readout_y_max = (ACTIVE_SENSOR_HEIGHT - (h * read_mode_mul)) / 2;
    readout_x = IM_CLAMP(readout_x, -readout_x_max, readout_x_max);
    readout_y = IM_CLAMP(readout_y, -readout_y_max, readout_y_max);

    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, col_start_addr,
                          readout_x_max - readout_x + MT9V0XX_COL_START_MIN); // sensor is mirrored by default
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, row_start_addr,
                          readout_y_max - readout_y + MT9V0XX_ROW_START_MIN); // sensor is mirrored by default
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, window_width_addr, w * read_mode_mul);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, window_height_addr, h * read_mode_mul);

    // Notes: 1. The MT9V0XX uses column parallel analog-digital converters, thus short row timing is not possible.
    // The minimum total row time is 690 columns (horizontal width + horizontal blanking). The minimum
    // horizontal blanking is 61. When the window width is set below 627, horizontal blanking
    // must be increased.
    //
    // The STM32H7 needs more than 94+(752-640) clocks between rows otherwise it can't keep up with the pixel rate.
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, horizontal_blanking_addr,
                          MT9V0XX_HORIZONTAL_BLANKING_DEF + (ACTIVE_SENSOR_WIDTH - IM_MIN(w * read_mode_mul, 640)));

    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, read_mode_addr, read_mode);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_PIXEL_COUNT, (w * h) / 8);

    if (is_mt9v0x4(csi)) {
        // We need more setup time for the pixel_clk at the full data rate...
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_PIXEL_CLOCK,
                              (read_mode_mul == 1) ? MT9V0XX_PIXEL_CLOCK_INV_PXL_CLK : 0);
    }

    // EDIT: WORKS BETTER TO STAY IN CONTEXT A
    //
    // Flip the context.
    // ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL,
    //         chip_control ^ MT9V0X4_CHIP_CONTROL_CONTEXT);

    return ret;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    int mask = (is_mt9v0x4(csi))
        ? (MT9V0X4_ROW_NOISE_CORR_ENABLE | MT9V0X4_ROW_NOISE_CORR_ENABLE_B)
        : MT9V0X2_ROW_NOISE_CORR_ENABLE;
    uint16_t reg;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_TEST_PATTERN, &reg);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_TEST_PATTERN,
                          (reg & (~(MT9V0XX_TEST_PATTERN_ENABLE | MT9V0XX_TEST_PATTERN_GRAY_MASK)))
                          | ((enable != 0) ? (MT9V0XX_TEST_PATTERN_ENABLE | MT9V0XX_TEST_PATTERN_GRAY_VERTICAL) : 0));
    ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_ROW_NOISE_CORR_CONTROL, &reg);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_ROW_NOISE_CORR_CONTROL,
                          (reg & (~mask)) | ((enable == 0) ? mask : 0));
    if (!csi->disable_delays) {
        ret |= csi->snapshot(csi, NULL, 0); // Force shadow mode register to update...
    }
    return ret;
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    int agc_mask = (is_mt9v0x4(csi))
        ? (MT9V0XX_AGC_ENABLE | MT9V0X4_AGC_ENABLE_B)
        : MT9V0XX_AGC_ENABLE;
    uint16_t reg;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE, &reg);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE,
                          (reg & (~agc_mask)) | ((enable != 0) ? agc_mask : 0));
    if (!csi->disable_delays) {
        ret |= csi->snapshot(csi, NULL, 0); // Force shadow mode register to update...
    }

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        int gain = IM_CLAMP(fast_roundf(expf((gain_db / 20.0f) * M_LN10) * 16.0f), 16, 64);

        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_ANALOG_GAIN, &reg);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_ANALOG_GAIN, (reg & 0xFF80) | gain);

        if (is_mt9v0x4(csi)) {
            ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0X4_ANALOG_GAIN_B, &reg);
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_ANALOG_GAIN_B, (reg & 0xFF80) | gain);
        }
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        int gain_ceiling = IM_CLAMP(fast_roundf(expf((gain_db_ceiling / 20.0f) * M_LN10) * 16.0f), 16, 64);
        int max_gain = (is_mt9v0x4(csi)) ? MT9V0X4_MAX_GAIN : MT9V0X2_MAX_GAIN;

        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, max_gain, &reg);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, max_gain, (reg & 0xFF80) | gain_ceiling);
    }

    return ret;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    uint16_t chip_control, reg, gain;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control);
    int context = chip_control & MT9V0X4_CHIP_CONTROL_CONTEXT;
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE, &reg);

    if (reg & (context ? MT9V0X4_AGC_ENABLE_B : MT9V0XX_AGC_ENABLE)) {
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_AGC_GAIN_OUTPUT, &gain);
    } else {
        int analog_gain = context ? MT9V0X4_ANALOG_GAIN_B : MT9V0XX_ANALOG_GAIN;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, analog_gain, &gain);
    }

    *gain_db = 20.0f * log10f((gain & 0x7F) / 16.0f);
    return ret;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    int aec_mask = (is_mt9v0x4(csi))
        ? (MT9V0XX_AEC_ENABLE | MT9V0X4_AEC_ENABLE_B)
        : MT9V0XX_AEC_ENABLE;
    uint16_t chip_control, reg, read_mode_reg, row_time_0, row_time_1;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control);
    int context = chip_control & MT9V0X4_CHIP_CONTROL_CONTEXT;
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE, &reg);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE,
                          (reg & (~aec_mask)) | ((enable != 0) ? aec_mask : 0));
    if (!csi->disable_delays) {
        ret |= csi->snapshot(csi, NULL, 0); // Force shadow mode register to update...
    }
    int read_mode = context ? MT9V0X4_READ_MODE_B : MT9V0XX_READ_MODE;
    int window_width = context ? MT9V0X4_WINDOW_WIDTH_B : MT9V0XX_WINDOW_WIDTH;
    int horizontal_blanking = context ? MT9V0X4_HORIZONTAL_BLANKING_B : MT9V0XX_HORIZONTAL_BLANKING;
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, read_mode, &read_mode_reg);
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, window_width, &row_time_0);
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, horizontal_blanking, &row_time_1);

    int clock = omv_csi_get_clk_frequency(csi, false);

    int exposure = IM_MIN(exposure_us, MICROSECOND_CLKS / 2) * (clock / MICROSECOND_CLKS);
    int row_time = row_time_0 + row_time_1;
    int coarse_time = exposure / row_time;
    int fine_time = exposure % row_time;

    // Fine shutter time is global.
    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_TOTAL_SHUTTER_WIDTH, coarse_time);

        if (is_mt9v0x4(csi)) {
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL, fine_time);
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_TOTAL_SHUTTER_WIDTH_B, coarse_time);
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL_B, fine_time);
        }
    } else if ((enable != 0) && (exposure_us >= 0)) {
        int max_expose = (is_mt9v0x4(csi)) ? MT9V0X4_MAX_EXPOSE : MT9V0X2_MAX_EXPOSE;
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, max_expose, coarse_time);

        if (is_mt9v0x4(csi)) {
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL, fine_time);
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL_B, fine_time);
        }
    }

    return ret;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    uint16_t chip_control, reg, read_mode_reg, row_time_0, row_time_1, int_pixels = 0, int_rows = 0;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control);
    int context = chip_control & MT9V0X4_CHIP_CONTROL_CONTEXT;
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_AEC_AGC_ENABLE, &reg);

    int read_mode = context ? MT9V0X4_READ_MODE_B : MT9V0XX_READ_MODE;
    int window_width = context ? MT9V0X4_WINDOW_WIDTH_B : MT9V0XX_WINDOW_WIDTH;
    int horizontal_blanking = context ? MT9V0X4_HORIZONTAL_BLANKING_B : MT9V0XX_HORIZONTAL_BLANKING;
    int fine_shutter_width_total = context ? MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL_B : MT9V0X4_FINE_SHUTTER_WIDTH_TOTAL;
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, read_mode, &read_mode_reg);
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, window_width, &row_time_0);
    ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, horizontal_blanking, &row_time_1);
    if (is_mt9v0x4(csi)) {
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, fine_shutter_width_total, &int_pixels);
    }

    int clock = omv_csi_get_clk_frequency(csi, false);

    if (reg & (context ? MT9V0X4_AEC_ENABLE_B : MT9V0XX_AEC_ENABLE)) {
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_AEC_EXPOSURE_OUTPUT, &int_rows);
    } else {
        int total_shutter_width = context ? MT9V0X4_TOTAL_SHUTTER_WIDTH_B : MT9V0XX_TOTAL_SHUTTER_WIDTH;
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, total_shutter_width, &int_rows);
    }

    *exposure_us = ((int_rows * (row_time_0 + row_time_1)) + int_pixels) / (clock / MICROSECOND_CLKS);
    return ret;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    uint16_t read_mode;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_READ_MODE, &read_mode);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_READ_MODE, // inverted behavior
                          (read_mode & (~MT9V0XX_READ_MODE_COL_FLIP)) | ((enable == 0) ? MT9V0XX_READ_MODE_COL_FLIP : 0));

    if (is_mt9v0x4(csi)) {
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0X4_READ_MODE_B, &read_mode);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_READ_MODE_B, // inverted behavior
                              (read_mode & (~MT9V0XX_READ_MODE_COL_FLIP)) | ((enable == 0) ? MT9V0XX_READ_MODE_COL_FLIP : 0));
    }

    if (!csi->disable_delays) {
        ret |= csi->snapshot(csi, NULL, 0); // Force shadow mode register to update...
    }
    return ret;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    uint16_t read_mode;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_READ_MODE, &read_mode);
    ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_READ_MODE, // inverted behavior
                          (read_mode & (~MT9V0XX_READ_MODE_ROW_FLIP)) | ((enable == 0) ? MT9V0XX_READ_MODE_ROW_FLIP : 0));

    if (is_mt9v0x4(csi)) {
        ret |= omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0X4_READ_MODE_B, &read_mode);
        ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0X4_READ_MODE_B, // inverted behavior
                              (read_mode & (~MT9V0XX_READ_MODE_ROW_FLIP)) | ((enable == 0) ? MT9V0XX_READ_MODE_ROW_FLIP : 0));
    }

    if (!csi->disable_delays) {
        ret |= csi->snapshot(csi, NULL, 0); // Force shadow mode register to update...
    }
    return ret;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;
    uint16_t chip_control;

    // The MT9V0XX does not have a hardware scaler so the readout w/h must be equal to the
    // framesize w/h.
    int tmp_readout_w = csi->resolution[csi->framesize][0];
    int tmp_readout_h = csi->resolution[csi->framesize][1];
    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        tmp_readout_w = ACTIVE_SENSOR_WIDTH;
        tmp_readout_h = ACTIVE_SENSOR_HEIGHT;
    }

    if (cfa_type != BAYER_CFA) {
        if ((tmp_readout_w <= (ACTIVE_SENSOR_WIDTH / 4)) && (tmp_readout_h <= (ACTIVE_SENSOR_HEIGHT / 4))) {
            tmp_readout_w *= 4;
            tmp_readout_h *= 4;
        } else if ((tmp_readout_w <= (ACTIVE_SENSOR_WIDTH / 2)) && (tmp_readout_h <= (ACTIVE_SENSOR_HEIGHT / 2))) {
            tmp_readout_w *= 2;
            tmp_readout_h *= 2;
        }
    }

    switch (request) {
        case OMV_CSI_IOCTL_SET_READOUT_WINDOW: {
            int tmp_readout_x = va_arg(ap, int);
            int tmp_readout_y = va_arg(ap, int);
            int readout_x_max = (ACTIVE_SENSOR_WIDTH - tmp_readout_w) / 2;
            int readout_y_max = (ACTIVE_SENSOR_HEIGHT - tmp_readout_h) / 2;
            tmp_readout_x = IM_CLAMP(tmp_readout_x, -readout_x_max, readout_x_max);
            tmp_readout_y = IM_CLAMP(tmp_readout_y, -readout_y_max, readout_y_max);
            bool changed = (tmp_readout_x != readout_x) ||
                           (tmp_readout_y != readout_y);
            readout_x = tmp_readout_x;
            readout_y = tmp_readout_y;
            if (changed && (csi->framesize != OMV_CSI_FRAMESIZE_INVALID)) {
                ret |= set_framesize(csi, csi->framesize);
            }
            break;
        }
        case OMV_CSI_IOCTL_GET_READOUT_WINDOW: {
            *va_arg(ap, int *) = readout_x;
            *va_arg(ap, int *) = readout_y;
            *va_arg(ap, int *) = tmp_readout_w;
            *va_arg(ap, int *) = tmp_readout_h;
            break;
        }
        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE: {
            int enable = va_arg(ap, int);
            ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control);
            ret |= omv_i2c_writew(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL,
                                  (chip_control & (~MT9V0XX_CHIP_CONTROL_MODE_MASK))
                                  | ((enable != 0) ? MT9V0XX_CHIP_CONTROL_SNAP_MODE : MT9V0XX_CHIP_CONTROL_MASTER_MODE));
            if (!csi->disable_delays) {
                ret |= csi->snapshot(csi, NULL, 0); // Force shadow mode register to update...
            }
            break;
        }
        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE: {
            int *enable = va_arg(ap, int *);
            ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CHIP_CONTROL, &chip_control);
            if (ret >= 0) {
                *enable = ((chip_control & MT9V0XX_CHIP_CONTROL_MODE_MASK) == MT9V0XX_CHIP_CONTROL_SNAP_MODE);
            }
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int mt9v0xx_init(omv_csi_t *csi) {
    // Initialize csi structure.
    csi->reset = reset;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_colorbar = set_colorbar;
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->ioctl = ioctl;

    // Set csi flags
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 0;
    csi->frame_sync = 1;
    csi->mono_bpp = 1;
    csi->cfa_format = SUBFORMAT_ID_BGGR;

    // Force old versions to the newest.
    if (csi->chip_id == MT9V0X2_ID_V_1 ||
        csi->chip_id == MT9V0X2_ID_V_2) {
        csi->chip_id = MT9V0X2_ID;
    }

    uint16_t cfa_type_reg;
    int ret = omv_i2c_readw(csi->i2c, csi->slv_addr, MT9V0XX_CFA_ID_REG, &cfa_type_reg);
    switch ((cfa_type_reg >> 9) & 0x7) {
        case BAYER_CFA_ID: {
            cfa_type = BAYER_CFA;
            switch (csi->chip_id) {
                case MT9V0X2_ID: {
                    csi->chip_id = MT9V0X2_C_ID;
                    break;
                }
                case MT9V0X4_ID: {
                    csi->chip_id = MT9V0X4_C_ID;
                    break;
                }
                default: {
                    break;
                }
            }
            csi->raw_output = 1;
            break;
        }
        default: {
            cfa_type = MONO_CFA;
            break;
        }
    }
    return ret;
}

#endif // (OMV_MT9V0XX_ENABLE == 1)
