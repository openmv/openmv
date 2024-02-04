/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV7725 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_OV7725_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "omv_i2c.h"
#include "sensor.h"
#include "ov7725.h"
#include "ov7725_regs.h"
#include "py/mphal.h"

static const uint8_t default_regs[][2] = {

// From App Note.

    {COM12,         0x03},
    {HSTART,        0x22},
    {HSIZE,         0xa4},
    {VSTART,        0x07},
    {VSIZE,         0xf0},
    {HREF,          0x00},
    {HOUTSIZE,      0xa0},
    {VOUTSIZE,      0xf0},
    {EXHCH,         0x00},
    {CLKRC,         0xC0}, // {CLKRC, 0x01},

    {TGT_B,         0x7f},
    {FIXGAIN,       0x09},
    {AWB_CTRL0,     0xe0},
    {DSP_CTRL1,     0xff},
    {DSP_CTRL2,     0x20 | DSP_CTRL2_VDCW_EN | DSP_CTRL2_HDCW_EN | DSP_CTRL2_VZOOM_EN | DSP_CTRL2_HZOOM_EN},
    {DSP_CTRL3,     0x00},
    {DSP_CTRL4,     0x48},

    {COM8,          0xf0},
    {COM4,          OMV_OV7725_PLL_CONFIG}, // {COM4, 0x41},
    {COM6,          0xc5},
    {COM9,          0x11},
    {BDBASE,        0x7f},
    {BDSTEP,        0x03},
    {AEW,           0x40},
    {AEB,           0x30},
    {VPT,           0xa1},
    {EXHCL,         0x00},
    {AWB_CTRL3,     0xaa},
    {COM8,          0xff},

    {EDGE1,         0x05},
    {DNSOFF,        0x01},
    {EDGE2,         0x03},
    {EDGE3,         0x00},
    {MTX1,          0xb0},
    {MTX2,          0x9d},
    {MTX3,          0x13},
    {MTX4,          0x16},
    {MTX5,          0x7b},
    {MTX6,          0x91},
    {MTX_CTRL,      0x1e},
    {BRIGHTNESS,    0x08},
    {CONTRAST,      0x20},
    {UVADJ0,        0x81},
    {SDE,           SDE_CONT_BRIGHT_EN | SDE_SATURATION_EN},

    {GAM1,          0x0c},
    {GAM2,          0x16},
    {GAM3,          0x2a},
    {GAM4,          0x4e},
    {GAM5,          0x61},
    {GAM6,          0x6f},
    {GAM7,          0x7b},
    {GAM8,          0x86},
    {GAM9,          0x8e},
    {GAM10,         0x97},
    {GAM11,         0xa4},
    {GAM12,         0xaf},
    {GAM13,         0xc5},
    {GAM14,         0xd7},
    {GAM15,         0xe8},
    {SLOP,          0x20},

    {DM_LNL,        0x00},
    {BDBASE,        OMV_OV7725_BANDING}, // {BDBASE, 0x7f}
    {BDSTEP,        0x03},

    {LC_RADI,       0x10},
    {LC_COEF,       0x10},
    {LC_COEFB,      0x14},
    {LC_COEFR,      0x17},
    {LC_CTR,        0x01}, // {LC_CTR, 0x05},

    {COM5,          0xf5}, // {COM5, 0x65},

    // OpenMV Custom.
    {COM7,          COM7_FMT_RGB565},

    // End.
    {0x00,          0x00},
};

#define NUM_BRIGHTNESS_LEVELS    (9)
static const uint8_t brightness_regs[NUM_BRIGHTNESS_LEVELS][2] = {
    {0x38, 0x0e}, /* -4 */
    {0x28, 0x0e}, /* -3 */
    {0x18, 0x0e}, /* -2 */
    {0x08, 0x0e}, /* -1 */
    {0x08, 0x06}, /*  0 */
    {0x18, 0x06}, /* +1 */
    {0x28, 0x06}, /* +2 */
    {0x38, 0x06}, /* +3 */
    {0x48, 0x06}, /* +4 */
};

#define NUM_CONTRAST_LEVELS    (9)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS][1] = {
    {0x10}, /* -4 */
    {0x14}, /* -3 */
    {0x18}, /* -2 */
    {0x1C}, /* -1 */
    {0x20}, /*  0 */
    {0x24}, /* +1 */
    {0x28}, /* +2 */
    {0x2C}, /* +3 */
    {0x30}, /* +4 */
};

#define NUM_SATURATION_LEVELS    (9)
static const uint8_t saturation_regs[NUM_SATURATION_LEVELS][2] = {
    {0x00, 0x00}, /* -4 */
    {0x10, 0x10}, /* -3 */
    {0x20, 0x20}, /* -2 */
    {0x30, 0x30}, /* -1 */
    {0x40, 0x40}, /*  0 */
    {0x50, 0x50}, /* +1 */
    {0x60, 0x60}, /* +2 */
    {0x70, 0x70}, /* +3 */
    {0x80, 0x80}, /* +4 */
};

static int reset(sensor_t *sensor) {
    // Reset all registers
    int ret = omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM7, COM7_RESET);

    // Delay 2 ms
    mp_hal_delay_ms(2);

    // Write default registers
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
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM2, &reg);

    if (enable) {
        reg |= COM2_SOFT_SLEEP;
    } else {
        reg &= ~COM2_SOFT_SLEEP;
    }

    // Write back register
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM2, reg) | ret;
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
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM7, &reg);

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            reg = COM7_SET_FMT(reg, COM7_FMT_RGB);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, DSP_CTRL4, DSP_CTRL4_YUV_RGB);
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            reg = COM7_SET_FMT(reg, COM7_FMT_YUV);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, DSP_CTRL4, DSP_CTRL4_YUV_RGB);
            break;
        case PIXFORMAT_BAYER:
            reg = COM7_SET_FMT(reg, COM7_FMT_P_BAYER);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, DSP_CTRL4, DSP_CTRL4_RAW8);
            break;
        default:
            return -1;
    }

    // Write back register
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM7, reg) | ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize) {
    uint8_t reg;
    int ret = 0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];
    bool vflip;

    if ((w > 640) || (h > 480)) {
        return -1;
    }

    // Write MSBs
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, HOUTSIZE, w >> 2);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VOUTSIZE, h >> 1);

    // Write LSBs
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, EXHCH, ((w & 0x3) | ((h & 0x1) << 2)));

    // Sample VFLIP
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM3, &reg);
    vflip = reg & COM3_VFLIP;
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, HREF, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, HREF, (reg & 0xBF) | (vflip ? 0x40 : 0x00));

    if ((w <= 320) && (h <= 240)) {
        // Set QVGA Resolution
        uint8_t reg;
        int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM7, &reg);
        reg = COM7_SET_RES(reg, COM7_RES_QVGA);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM7, reg);

        // Set QVGA Window Size
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, HSTART, 0x3F);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, HSIZE,  0x50);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VSTART, 0x03 - vflip);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VSIZE,  0x78);

        // Enable auto-scaling/zooming factors
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, DSPAUTO, 0xFF);
    } else {
        // Set VGA Resolution
        uint8_t reg;
        int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM7, &reg);
        reg = COM7_SET_RES(reg, COM7_RES_VGA);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM7, reg);

        // Set VGA Window Size
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, HSTART, 0x23);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, HSIZE,  0xA0);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VSTART, 0x07 - vflip);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VSIZE,  0xF0);

        // Disable auto-scaling/zooming factors
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, DSPAUTO, 0xF3);

        // Clear auto-scaling/zooming factors
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, SCAL0, 0x00);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, SCAL1, 0x40);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, SCAL2, 0x40);
    }

    return ret;
}

static int set_contrast(sensor_t *sensor, int level) {
    level += (NUM_CONTRAST_LEVELS / 2);
    if (level < 0 || level >= NUM_CONTRAST_LEVELS) {
        return -1;
    }

    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, CONTRAST, contrast_regs[level][0]);
}

static int set_brightness(sensor_t *sensor, int level) {
    int ret = 0;
    level += (NUM_BRIGHTNESS_LEVELS / 2);
    if (level < 0 || level >= NUM_BRIGHTNESS_LEVELS) {
        return -1;
    }

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, BRIGHTNESS, brightness_regs[level][0]);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, SIGN_BIT,   brightness_regs[level][1]);
    return ret;
}

static int set_saturation(sensor_t *sensor, int level) {
    int ret = 0;
    level += (NUM_SATURATION_LEVELS / 2);
    if (level < 0 || level >= NUM_SATURATION_LEVELS) {
        return -1;
    }

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, USAT, saturation_regs[level][0]);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VSAT, saturation_regs[level][1]);
    return ret;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM9, &reg);

    // Set gain ceiling
    reg = COM9_SET_AGC(reg, gainceiling);
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM9, reg) | ret;
}

static int set_colorbar(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM3, &reg);

    // Enable colorbar test pattern output
    reg = COM3_SET_CBAR(reg, enable);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM3, reg);

    // Enable DSP colorbar output
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, DSP_CTRL3, &reg);
    reg = DSP_CTRL3_SET_CBAR(reg, enable);
    return omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, DSP_CTRL3, reg) | ret;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM8, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AGC(reg, (enable != 0)));

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        float gain = IM_MAX(IM_MIN(expf((gain_db / 20.0f) * M_LN10), 32.0f), 1.0f);

        int gain_temp = fast_ceilf(logf(IM_MAX(gain / 2.0f, 1.0f)) / M_LN2);
        int gain_hi = 0xF >> (4 - gain_temp);
        int gain_lo = IM_MIN(fast_roundf(((gain / (1 << gain_temp)) - 1.0f) * 16.0f), 15);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, GAIN, (gain_hi << 4) | (gain_lo << 0));
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        float gain_ceiling = IM_MAX(IM_MIN(expf((gain_db_ceiling / 20.0f) * M_LN10), 32.0f), 2.0f);

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM9, &reg);
        ret |=
            omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM9,
                           (reg & 0x8F) | ((fast_ceilf(logf(gain_ceiling) / M_LN2) - 1) << 4));
    }

    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db) {
    uint8_t reg, gain;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AGC(reg, 0));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, GAIN, &gain);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AGC(reg, 1));
    // }
    // DISABLED

    int hi_gain = 1 << (((gain >> 7) & 1) + ((gain >> 6) & 1) + ((gain >> 5) & 1) + ((gain >> 4) & 1));
    float lo_gain = 1.0f + (((gain >> 0) & 0xF) / 16.0f);
    *gain_db = 20.0f * log10f(hi_gain * lo_gain);

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM8, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AEC(reg, (enable != 0)));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM7, &reg);

        int t_line = (reg & COM7_RES_QVGA) ? (320 + 256) : (640 + 144);
        int t_pclk = (COM7_GET_FMT(reg) == COM7_FMT_P_BAYER) ? 1 : 2;

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM4, &reg);
        int pll_mult = 0;

        if (COM4_GET_PLL(reg) == COM4_PLL_BYPASS) {
            pll_mult = 1;
        }
        if (COM4_GET_PLL(reg) == COM4_PLL_4x) {
            pll_mult = 4;
        }
        if (COM4_GET_PLL(reg) == COM4_PLL_6x) {
            pll_mult = 6;
        }
        if (COM4_GET_PLL(reg) == COM4_PLL_8x) {
            pll_mult = 8;
        }

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, CLKRC, &reg);
        int clk_rc = 0;

        if (reg & CLKRC_NO_PRESCALE) {
            clk_rc = 1;
        } else {
            clk_rc = ((reg & CLKRC_PRESCALER) + 1) * 2;
        }

        int exposure =
            IM_MAX(IM_MIN(((exposure_us * (((OMV_CSI_XCLK_FREQUENCY / clk_rc) * pll_mult) / 1000000)) / t_pclk) / t_line,
                          0xFFFF), 0x0000);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, AEC, ((exposure >> 0) & 0xFF));
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, AECH, ((exposure >> 8) & 0xFF));
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us) {
    uint8_t reg, aec_l, aec_h;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AEC(reg, 0));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, AEC, &aec_l);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, AECH, &aec_h);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AEC(reg, 1));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM7, &reg);

    int t_line = (reg & COM7_RES_QVGA) ? (320 + 256) : (640 + 144);
    int t_pclk = (COM7_GET_FMT(reg) == COM7_FMT_P_BAYER) ? 1 : 2;

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM4, &reg);
    int pll_mult = 0;

    if (COM4_GET_PLL(reg) == COM4_PLL_BYPASS) {
        pll_mult = 1;
    }
    if (COM4_GET_PLL(reg) == COM4_PLL_4x) {
        pll_mult = 4;
    }
    if (COM4_GET_PLL(reg) == COM4_PLL_6x) {
        pll_mult = 6;
    }
    if (COM4_GET_PLL(reg) == COM4_PLL_8x) {
        pll_mult = 8;
    }

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, CLKRC, &reg);
    int clk_rc = 0;

    if (reg & CLKRC_NO_PRESCALE) {
        clk_rc = 1;
    } else {
        clk_rc = ((reg & CLKRC_PRESCALER) + 1) * 2;
    }

    *exposure_us =
        (((aec_h << 8) + (aec_l << 0)) * t_line * t_pclk) / (((OMV_CSI_XCLK_FREQUENCY / clk_rc) * pll_mult) / 1000000);

    return ret;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM8, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AWB(reg, (enable != 0)));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))
        && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) && (!isinff(b_gain_db))) {
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, AWB_CTRL1, &reg);
        float gain_div = (reg & 0x2) ? 64.0f : 128.0f;

        int r_gain = IM_MAX(IM_MIN(fast_roundf(expf((r_gain_db / 20.0f) * M_LN10) * gain_div), 255), 0);
        int g_gain = IM_MAX(IM_MIN(fast_roundf(expf((g_gain_db / 20.0f) * M_LN10) * gain_div), 255), 0);
        int b_gain = IM_MAX(IM_MIN(fast_roundf(expf((b_gain_db / 20.0f) * M_LN10) * gain_div), 255), 0);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, BLUE, b_gain);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, RED, r_gain);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, GREEN, g_gain);
    }

    return ret;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    uint8_t reg, blue, red, green;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AWB_EN) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AWB(reg, 0));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, BLUE, &blue);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, RED, &red);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, GREEN, &green);

    // DISABLED
    // if (reg & COM8_AWB_EN) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM8, COM8_SET_AWB(reg, 1));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, AWB_CTRL1, &reg);
    float gain_div = (reg & 0x2) ? 64.0f : 128.0f;

    *r_gain_db = 20.0f * log10f(red / gain_div);
    *g_gain_db = 20.0f * log10f(green / gain_div);
    *b_gain_db = 20.0f * log10f(blue / gain_div);

    return ret;
}

static int set_auto_blc(sensor_t *sensor, int enable, int *regs) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM13, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM13, COM13_SET_BLC(reg, (enable != 0)));

    if ((enable == 0) && (regs != NULL)) {
        for (uint32_t i = 0; i < sensor->hw_flags.blc_size; i++) {
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, ADOFF_B + i, regs[i]);
        }
    }

    return ret;
}

static int get_blc_regs(sensor_t *sensor, int *regs) {
    int ret = 0;

    for (uint32_t i = 0; i < sensor->hw_flags.blc_size; i++) {
        uint8_t reg;
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, ADOFF_B + i, &reg);
        regs[i] = reg;
    }

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM3, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM3, COM3_SET_MIRROR(reg, enable));

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM3, &reg);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM3, COM3_SET_FLIP(reg, enable));
    // Apply new vertical flip setting.
    ret |= set_framesize(sensor, sensor->framesize);

    return ret;
}

static int set_special_effect(sensor_t *sensor, sde_t sde) {
    int ret = 0;

    switch (sde) {
        case SDE_NEGATIVE:
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, SDE, 0x46);
            break;
        case SDE_NORMAL:
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, SDE, 0x06);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, UFIX, 0x80);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, VFIX, 0x80);
            break;
        default:
            return -1;
    }

    return ret;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef) {
    int ret = 0;

    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LC_CTR, (enable & 0x01));
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LC_RADI, radi);
    ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, LC_COEF, coef);

    return ret;
}

static int ioctl(sensor_t *sensor, int request, va_list ap) {
    int ret = 0;
    uint8_t reg;

    switch (request) {
        case IOCTL_SET_NIGHT_MODE: {
            int enable = va_arg(ap, int);
            ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM5, &reg);
            ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, COM5, COM5_SET_AFR(reg, (enable != 0)));
            if (enable == 0) {
                ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, ADVFL, 0);
                ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, ADVFH, 0);
            }
            break;
        }
        case IOCTL_GET_NIGHT_MODE: {
            int *enable = va_arg(ap, int *);
            ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, COM5, &reg);
            if (ret >= 0) {
                *enable = reg & COM5_AFR;
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

int ov7725_init(sensor_t *sensor) {
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
    sensor->set_colorbar = set_colorbar;
    sensor->set_auto_gain = set_auto_gain;
    sensor->get_gain_db = get_gain_db;
    sensor->set_auto_exposure = set_auto_exposure;
    sensor->get_exposure_us = get_exposure_us;
    sensor->set_auto_whitebal = set_auto_whitebal;
    sensor->get_rgb_gain_db = get_rgb_gain_db;
    sensor->set_auto_blc = set_auto_blc;
    sensor->get_blc_regs = get_blc_regs;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;
    sensor->set_special_effect = set_special_effect;
    sensor->set_lens_correction = set_lens_correction;
    sensor->ioctl = ioctl;

    // Set sensor flags
    sensor->hw_flags.vsync = 1;
    sensor->hw_flags.hsync = 0;
    sensor->hw_flags.pixck = 1;
    sensor->hw_flags.fsync = 0;
    sensor->hw_flags.jpege = 0;
    sensor->hw_flags.gs_bpp = 2;
    sensor->hw_flags.rgb_swap = 1;
    sensor->hw_flags.yuv_order = SENSOR_HW_FLAGS_YVU422;
    sensor->hw_flags.blc_size = 8;

    return 0;
}
#endif // (OMV_OV7725_ENABLE == 1)
