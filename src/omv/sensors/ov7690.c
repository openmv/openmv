/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2020 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2020 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV7690 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_ENABLE_OV7690 == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "omv_i2c.h"
#include "sensor.h"
#include "ov7690.h"
#include "ov7690_regs.h"
#include "py/mphal.h"

static const uint8_t default_regs[][2] = {

// From App Note.

    {0x0c, 0xd6},
    {0x48, 0x42},
    {0x27, 0x80},
    {0x64, 0x10},
    {0x68, 0xb4},
    {0x69, 0x12},
    {0x2f, 0x60},
    {0x41, 0x43},
    {0x44, 0x24},
    {0x4b, 0x0e},
    {0x4c, 0x7b},
    {0x4d, 0x0a},
    {0x29, 0x50},
    {0x1b, 0x19},
    {0x39, 0x80},
    {0x80, 0x7f},
    {0x81, 0xff},
    {0x91, 0x20},
    {0x21, 0x44},
    {0x11, 0x01},
    {0x12, 0x04}, // {0x12, 0x00},
    {0x82, 0x07}, // {0x82, 0x03},
    {0xd0, 0x48},
    {0x2b, 0x38},
    {0x15, 0x14},
    {0x16, 0x03},
    {0x17, 0x69},
    {0x18, 0xa4},
    {0x19, 0x0b}, // {0x19, 0x0c},
    {0x1a, 0xf6},
    {0x3e, 0x30},
    {0xc8, 0x02},
    {0xc9, 0x80},
    {0xca, 0x01},
    {0xcb, 0xe0},
    {0xcc, 0x02},
    {0xcd, 0x80},
    {0xce, 0x01},
    {0xcf, 0xe0},
    {0x80, 0x7f},
    {0x85, 0x10},
    {0x86, 0x00},
    {0x87, 0x00},
    {0x88, 0x00},
    {0x89, 0x35},
    {0x8a, 0x30},
    {0x8b, 0x33},
    {0xbb, 0xbe},
    {0xbc, 0xc0},
    {0xbd, 0x02},
    {0xbe, 0x16},
    {0xbf, 0xc2},
    {0xc0, 0xd9},
    {0xc1, 0x1e},
    {0xb4, 0x36},
    {0xb5, 0x06},
    {0xb7, 0x00},
    {0xb6, 0x04},
    {0xb8, 0x06},
    {0xb9, 0x02},
    {0xba, 0x00},
    {0x24, 0x78},
    {0x25, 0x68},
    {0x26, 0xb4},
    {0x81, 0xff},
    {0x5a, 0x30},
    {0x5b, 0xa5},
    {0x5c, 0x30},
    {0x5d, 0x20},
    {0xa3, 0x05},
    {0xa4, 0x10},
    {0xa5, 0x25},
    {0xa6, 0x46},
    {0xa7, 0x57},
    {0xa8, 0x64},
    {0xa9, 0x70},
    {0xaa, 0x7c},
    {0xab, 0x87},
    {0xac, 0x90},
    {0xad, 0x9f},
    {0xae, 0xac},
    {0xaf, 0xc1},
    {0xb0, 0xd5},
    {0xb1, 0xe7},
    {0xb2, 0x21},
    {0x8c, 0x5c},
    {0x8d, 0x11},
    {0x8e, 0x12},
    {0x8f, 0x19},
    {0x90, 0x50},
    {0x91, 0x21},
    {0x92, 0x9c},
    {0x93, 0x9b},
    {0x94, 0x0c},
    {0x95, 0x0d},
    {0x96, 0xff},
    {0x97, 0x00},
    {0x98, 0x3f},
    {0x99, 0x30},
    {0x9a, 0x4d},
    {0x9b, 0x3d},
    {0x9c, 0xf0},
    {0x9d, 0xf0},
    {0x9e, 0xf0},
    {0x9f, 0xff},
    {0xa0, 0x5f},
    {0xa1, 0x61},
    {0xa2, 0x0c},
    {0x14, 0x21},
    {0x13, 0xf7},

// OpenMV Custom.

    // Frame Rate Adjustment for 24 Mhz input clock - 30 fps, PCLK = 24Mhz
    {0x11, 0x00},
    {0x29, 0x50},
    {0x2a, 0x30},
    {0x2b, 0x08},
    {0x2c, 0x00},
    {0x15, 0x00},
    {0x2d, 0x00},
    {0x2e, 0x00},

    // Night Mode with Auto Frame Rate - For 24Mhz/26Mhz Clock Input - 30fps ~ 3.75 night mode for 60Hz light environment
    {0x11, 0x00},
    {0x15, 0xcc},

    // Banding Filter Settings for 24MHz Input Clock - 30fps for 50/60Hz light frequency
    {0x13, 0xef}, // banding filter enable
    {0x50, 0x99}, // 50Hz banding filter
    {0x51, 0x7f}, // 60Hz banding filter
    {0x21, 0x34}, // 3 step for 50hz, 4 step for 60hz
    {0x14, 0xb2}, // Auto detect banding filter

    // Simple White Balance
    {0x13, 0xef}, // AWB on
    {0x8e, 0x92}, // enable simple AWB

// End.

    {0x00, 0x00},
};

#define NUM_CONTRAST_LEVELS    (9)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS][1] = {
    {0xd0}, /* -4 */
    {0x80}, /* -3 */
    {0x48}, /* -2 */
    {0x20}, /* -1 */
    {0x00}, /*  0 */
    {0x00}, /* +1 */
    {0x00}, /* +2 */
    {0x00}, /* +3 */
    {0x00}, /* +4 */
};

#define NUM_SATURATION_LEVELS    (9)

static int reset(sensor_t *sensor) {
    // Reset all registers
    int ret = omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG12, 0x80);

    // Delay 2 ms
    mp_hal_delay_ms(2);

    // Write default regsiters
    for (int i = 0; default_regs[i][0]; i++) {
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Delay 300 ms
    if (!sensor->disable_delays) {
        mp_hal_delay_ms(300);
    }

    return ret;
}

static int sleep(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG0E, &reg);

    if (enable) {
        reg |= 0x8;
    } else {
        reg &= ~0x8;
    }

    // Write back register
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG0E, reg) | ret;
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr) {
    uint8_t reg_data;
    if (omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data) {
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat) {
    if ((pixformat == PIXFORMAT_BAYER) && sensor->framesize && (sensor->framesize != FRAMESIZE_VGA)) {
        // bayer for vga only
        return -1;
    }

    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG12, &reg);

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            reg = (reg & 0xFC) | 0x2;
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG3E, 0x30);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG82, 0x07);
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            reg = (reg & 0xFC) | 0x0;
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG3E, 0x30);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG82, 0x07);
            break;
        case PIXFORMAT_BAYER:
            reg = (reg & 0xFC) | 0x1;
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG3E, 0x20);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG82, 0x00);
            break;
        default:
            return -1;
    }

    // Write back register
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG12, reg) | ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize) {
    uint8_t reg;
    int ret = 0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];
    bool vflip;

    if (((w > 640) || (h > 480))
        || (sensor->pixformat && (sensor->pixformat == PIXFORMAT_BAYER) && (framesize != FRAMESIZE_VGA))) {
        // bayer for vga only
        return -1;
    }

    // Sample VFLIP
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG0C, &reg);
    vflip = !(reg & 0x80);

    if ((w <= 320) && (h <= 240)) {
        // Set QVGA Resolution
        uint8_t reg;
        int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG12, &reg);
        reg = (reg & 0xBF) | 0x40;
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG12, reg);

        // Set QVGA Window Size
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG19, 0x03 - vflip);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGC8, 0x02);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGC9, 0x80);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCA, 0x00);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCB, 0xF0);
    } else {
        // Set VGA Resolution
        uint8_t reg;
        int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG12, &reg);
        reg = (reg & 0xBF) | 0x00;
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG12, reg);

        // Set VGA Window Size
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG19, 0x0b - vflip);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGC8, 0x02);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGC9, 0x80);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCA, 0x01);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCB, 0xE0);
    }

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCC, w >> 8);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCD, w);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCE, h >> 8);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGCF, h);

    return ret;
}

static int set_contrast(sensor_t *sensor, int level) {
    uint8_t reg;
    int ret = 0;
    int new_level = NUM_CONTRAST_LEVELS / 2;
    if (new_level < 0 || new_level >= NUM_CONTRAST_LEVELS) {
        return -1;
    }

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, 0xd5, 0x20);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, 0xd4, 0x10 + (4 * new_level));
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, 0xd3, contrast_regs[new_level][0]);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REGD2, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGD2, (reg & 0xFB) | ((level != 0) << 2));
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, 0xdc, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, 0xdc, (level < 0) ? (reg | 0x04) : (reg & 0xFB));
    return ret;
}

static int set_brightness(sensor_t *sensor, int level) {
    return 0;
}

static int set_saturation(sensor_t *sensor, int level) {
    uint8_t reg;
    int ret = 0;
    int new_level = NUM_SATURATION_LEVELS / 2;
    if (new_level < 0 || new_level >= NUM_SATURATION_LEVELS) {
        return -1;
    }

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGD8, 0x10 * new_level);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGD9, 0x10 * new_level);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REGD2, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REGD2, (reg & 0xFD) | ((level != 0) << 1));
    return ret;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG14, &reg);

    // Set gain ceiling
    reg = (reg & 0x8F) | (gainceiling << 4);
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG14, reg) | ret;
}

static int set_quality(sensor_t *sensor, int qs) {
    return 0;
}

static int set_colorbar(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG82, &reg);

    // Enable colorbars
    reg = (reg & 0xF7) | ((enable != 0) << 3);
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG82, reg) | ret;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG13, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG13, (reg & 0xFB) | ((enable != 0) << 2));

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        float gain = IM_MAX(IM_MIN(fast_expf((gain_db / 20.0) * fast_log(10.0)), 128.0), 1.0);

        int gain_temp = fast_roundf(fast_log2(IM_MAX(gain / 2.0, 1.0)));
        int gain_hi = 0x3F >> (6 - gain_temp);
        int gain_lo = IM_MIN(fast_roundf(((gain / (1 << gain_temp)) - 1.0) * 16.0), 15);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, GAIN, (gain_hi << 4) | (gain_lo << 0));
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG15, &reg);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG15, (reg & 0xFC) | (gain_hi >> 4));
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        float gain_ceiling = IM_MAX(IM_MIN(fast_expf((gain_db_ceiling / 20.0) * fast_log(10.0)), 128.0), 2.0);

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG14, &reg);
        ret |=
            omv_i2c_writeb(&sensor->i2c_bus,
                           sensor->slv_addr,
                           REG14,
                           (reg & 0x8F) | ((fast_ceilf(fast_log2(gain_ceiling)) - 1) << 4));
    }

    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db) {
    uint8_t gain, reg15;
    int ret = 0;

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, GAIN, &gain);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG15, &reg15);

    int hi_gain = 1 <<
                  (((reg15 >>
                     1) & 1) +
                   ((reg15 >> 0) & 1) + ((gain >> 7) & 1) + ((gain >> 6) & 1) + ((gain >> 5) & 1) + ((gain >> 4) & 1));
    float lo_gain = 1.0 + (((gain >> 0) & 0xF) / 16.0);
    *gain_db = 20.0 * (fast_log(hi_gain * lo_gain) / fast_log(10.0));

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG13, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG13, (reg & 0xFE) | ((enable != 0) << 0));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG12, &reg);
        int t_line = (reg & 0x40) ? (320 + 456) : (640 + 136);

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG3E, &reg);
        int t_pclk = (reg & 0x10) ? 2 : 1;

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, PLL, &reg);
        int pll_div = reg >> 6, pll_mult = 1;

        switch ((reg >> 4) & 0x3) {
            case 0: pll_div = 1; break;
            case 1: pll_mult = 4; break;
            case 2: pll_mult = 6; break;
            case 3: pll_mult = 8; break;
        }

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, CLKRC, &reg);
        int clk_rc;

        if (reg & 0x40) {
            clk_rc = 1;
        } else {
            clk_rc = (reg & 0x3F) + 1;
        }

        int exposure =
            IM_MAX(IM_MIN(((exposure_us * ((((OV7690_XCLK_FREQ / clk_rc) * pll_mult) / pll_div) / 1000000)) / t_pclk) / t_line,
                          0xFFFF), 0x0000);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, AECL, ((exposure >> 0) & 0xFF));
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, AECH, ((exposure >> 8) & 0xFF));
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us) {
    uint8_t aec_l, aec_h, reg;
    int ret = 0;

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, AECL, &aec_l);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, AECH, &aec_h);

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG12, &reg);
    int t_line = (reg & 0x40) ? (320 + 456) : (640 + 136);

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG3E, &reg);
    int t_pclk = (reg & 0x10) ? 2 : 1;

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, PLL, &reg);
    int pll_div = reg >> 6, pll_mult = 1;

    switch ((reg >> 4) & 0x3) {
        case 0: pll_div = 1; break;
        case 1: pll_mult = 4; break;
        case 2: pll_mult = 6; break;
        case 3: pll_mult = 8; break;
    }

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, CLKRC, &reg);
    int clk_rc;

    if (reg & 0x40) {
        clk_rc = 1;
    } else {
        clk_rc = (reg & 0x3F) + 1;
    }

    *exposure_us =
        (((aec_h << 8) + (aec_l << 0)) * t_line * t_pclk) / ((((OV7690_XCLK_FREQ / clk_rc) * pll_mult) / pll_div) / 1000000);

    return ret;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG13, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG13, (reg & 0xFD) | ((enable != 0) << 1));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))
        && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) && (!isinff(b_gain_db))) {

        int r_gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((r_gain_db / 20.0) * fast_log(10.0))), 255), 0);
        int g_gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((g_gain_db / 20.0) * fast_log(10.0))), 255), 0);
        int b_gain = IM_MAX(IM_MIN(fast_roundf(fast_expf((b_gain_db / 20.0) * fast_log(10.0))), 255), 0);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, BGAIN, b_gain);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, RGAIN, r_gain);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, GGAIN, g_gain);
    }

    return ret;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    uint8_t blue, red, green;
    int ret = 0;

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, BGAIN, &blue);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, RGAIN, &red);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, GGAIN, &green);

    *r_gain_db = 20.0 * (fast_log(red) / fast_log(10.0));
    *g_gain_db = 20.0 * (fast_log(green) / fast_log(10.0));
    *b_gain_db = 20.0 * (fast_log(blue) / fast_log(10.0));

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG0C, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG0C, (reg & 0xBF) | ((enable == 0) << 6));

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG0C, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG0C, (reg & 0x7F) | ((enable == 0) << 7));
    // Apply new vertical flip setting.
    ret |= set_framesize(sensor, sensor->framesize);

    return ret;
}

static int set_special_effect(sensor_t *sensor, sde_t sde) {
    int ret = 0;

    switch (sde) {
        case SDE_NEGATIVE:
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, 0x28, 0x80);
            break;
        case SDE_NORMAL:
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, 0x28, 0x00);
            break;
        default:
            return -1;
    }

    return ret;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef) {
    int ret = 0;

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LCC0, (enable != 0) << 4);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LCC1, radi);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LCC4, coef);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LCC5, coef);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LCC6, coef);

    return ret;
}

int ov7690_init(sensor_t *sensor) {
    // Initialize sensor structure.
    sensor->reset = reset;
    sensor->sleep = sleep;
    sensor->read_reg = read_reg;
    sensor->write_reg = write_reg;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_contrast = set_contrast;
    sensor->set_brightness = set_brightness;
    sensor->set_saturation = set_saturation;
    sensor->set_gainceiling = set_gainceiling;
    sensor->set_quality = set_quality;
    sensor->set_colorbar = set_colorbar;
    sensor->set_auto_gain = set_auto_gain;
    sensor->get_gain_db = get_gain_db;
    sensor->set_auto_exposure = set_auto_exposure;
    sensor->get_exposure_us = get_exposure_us;
    sensor->set_auto_whitebal = set_auto_whitebal;
    sensor->get_rgb_gain_db = get_rgb_gain_db;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;
    sensor->set_special_effect = set_special_effect;
    sensor->set_lens_correction = set_lens_correction;

    // Set sensor flags
    sensor->hw_flags.vsync = 1;
    sensor->hw_flags.hsync = 0;
    sensor->hw_flags.pixck = 1;
    sensor->hw_flags.fsync = 0;
    sensor->hw_flags.jpege = 0;
    sensor->hw_flags.gs_bpp = 2;
    sensor->hw_flags.rgb_swap = 1;

    return 0;
}
#endif //(OMV_ENABLE_OV7690 == 1)
