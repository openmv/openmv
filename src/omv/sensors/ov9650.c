/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV9650 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_ENABLE_OV9650 == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "omv_i2c.h"
#include "sensor.h"
#include "ov9650.h"
#include "py/mphal.h"
#include "ov9650_regs.h"

#define NUM_BR_LEVELS    7

static const uint8_t default_regs[][2] = {
    /* See Implementation Guide */
    {REG_COM2,   0x01},  /*  Output drive x2 */
    {REG_COM5,   0x00},  /*  System clock  */
    {REG_CLKRC,  0x81},  /*  Clock control 30 FPS*/
    {REG_MVFP,   0x10},  /*  Mirror/VFlip */

    {REG_OFON,   0x43},  /*  Power down register  */
    {REG_ACOM38, 0x12},  /*  reserved  */
    {REG_ADC,    0x91},  /*  reserved  */
    {REG_RSVD35, 0x81},  /*  reserved  */

    /* Default QQVGA-RGB565 */
    {REG_COM7,   0x14},  /*  QVGA/RGB565 */
    {REG_COM1,   0x24},  /*  QQVGA/Skip Option */
    {REG_COM3,   0x04},  /*  Vario Pixels */
    {REG_COM4,   0x80},  /*  Vario Pixels */
    {REG_COM15,  0xD0},  /*  Output range 0x00-0xFF/RGB565*/

    /* YUV fmt /Special Effects Controls */
    {REG_TSLB,   0x01},  /*  YUYV/DBLC Enable/Bitwise reverse*/
    {REG_MANU,   0x80},  /*  Manual U */
    {REG_MANV,   0x80},  /*  Manual V */

    /* Dummy pixels settings */
    {REG_EXHCH,  0x00},  /*  Dummy Pixel Insert MSB */
    {REG_EXHCL,  0x00},  /*  Dummy Pixel Insert LSB */

    {REG_ADVFH,  0x00},  /*  Dummy Pixel Insert MSB */
    {REG_ADVFL,  0x00},  /*  Dummy Pixel Insert LSB */

    /* See Implementation Guide Section 3.4.1.2 */
    {REG_COM8,   0xA7}, /* Enable Fast Mode AEC/Enable Banding Filter/AGC/AWB/AEC */
    {0x60,       0x8C}, /* Normal AWB, 0x0C for Advanced AWB */
    {REG_AEW,    0x70}, /* AGC/AEC Threshold Upper Limit */
    {REG_AEB,    0x64}, /* AGC/AEC Threshold Lower Limit */
    {REG_VPT,    0xC3}, /* Fast AEC operating region */


    /* See OV9650 Implementation Guide */
    {REG_COM11,  0x01}, /* Night Mode-Automatic/Manual Banding Filter */
    {REG_MBD,    0x1a}, /* MBD[7:0] Manual banding filter LSB */
    {REG_HV,     0x0A}, /* HV[0]    Manual banding filter MSB */
    {REG_COM12,  0x04}, /* HREF options/ UV average  */
    {REG_COM9,   0x20}, /* Gain ceiling [6:4]/Over-Exposure */
    {REG_COM16,  0x02}, /* Color matrix coeff double option */
    {REG_COM13,  0x10}, /* Gamma/Colour Matrix/UV delay */
    {REG_COM23,  0x00}, /* Disable Color bar/Analog Color Gain */
    {REG_PSHFT,  0x00}, /* Pixel delay after HREF  */
    {REG_COM10,  0x00}, /* Slave mode, HREF vs HSYNC, signals negate */
    {REG_EDGE,   0xa6}, /* Edge enhancement threshold and factor */
    {REG_COM6,   0x43}, /* HREF & ADBLC options */
    {REG_COM22,  0x20}, /* Edge enhancement/Denoising */

    /* Some registers discovered with probing */
    {REG_COM21,  0x00}, /* COM21[3] Digital Zoom */
    {REG_GRCOM,  0x24}, /* Enable Internal Regulator */
    {0xaa,       0x00}, /* some edge effect 0x80 */
    {0xab,       0x00}, /* makes image blurry 0x40 */

#if 0
    /* When AEC is not used */
    {REG_AECH,   0x00}, /* Exposure Value MSB */
    {REG_AECHM,  0x00}, /* Exposure Value LSB */
#endif

#if 0
    /* Windowing Settings */
    {REG_HSTART, 0x1d},  /*  Horiz start high bits  */
    {REG_HSTOP,  0xbd},  /*  Horiz stop high bits  */
    {REG_HREF,   0xbf},  /*  HREF pieces  */

    {REG_VSTART, 0x00},  /*  Vert start high bits  */
    {REG_VSTOP,  0x80},  /*  Vert stop high bits  */
    {REG_VREF,   0x12},  /*  Pieces of GAIN, VSTART, VSTOP  */
#endif

#if 1
    /* gamma curve p */
    {0x6c,  0x40},
    {0x6d,  0x30},
    {0x6e,  0x4b},
    {0x6f,  0x60},
    {0x70,  0x70},
    {0x71,  0x70},
    {0x72,  0x70},
    {0x73,  0x70},
    {0x74,  0x60},
    {0x75,  0x60},
    {0x76,  0x50},
    {0x77,  0x48},
    {0x78,  0x3a},
    {0x79,  0x2e},
    {0x7a,  0x28},
    {0x7b,  0x22},

    /* Gamma curve T */
    {0x7c,  0x04},
    {0x7d,  0x07},
    {0x7e,  0x10},
    {0x7f,  0x28},
    {0x80,  0x36},
    {0x81,  0x44},
    {0x82,  0x52},
    {0x83,  0x60},
    {0x84,  0x6c},
    {0x85,  0x78},
    {0x86,  0x8c},
    {0x87,  0x9e},
    {0x88,  0xbb},
    {0x89,  0xd2},
    {0x8a,  0xe6},

    /* Reserved Registers, see OV965x App Note */
    {0x16,  0x07},
    {0x96,  0x04},
    {0x8e,  0x00},
    {0x94,  0x88},
    {0x95,  0x88},
    {0x5c,  0x96},
    {0x5d,  0x96},
    {0x5e,  0x10},
    {0x59,  0xeb},
    {0x5a,  0x9c},
    {0x5b,  0x55},
#endif

    /* NULL reg */
    {0x00,  0x00}
};

static const uint8_t rgb565_regs[][2] = {
    /* See Implementation Guide */
    {REG_COM3,   0x04},  /*  Vario Pixels */
    {REG_COM4,   0x80},  /*  Vario Pixels */
    {REG_COM15,  0xD0},  /*  Output range 0x00-0xFF/RGB565*/

    /* RGB color matrix */
    {REG_MTX1,   0x71},
    {REG_MTX2,   0x3e},
    {REG_MTX3,   0x0c},

    {REG_MTX4,   0x33},
    {REG_MTX5,   0x72},
    {REG_MTX6,   0x00},

    {REG_MTX7,   0x2b},
    {REG_MTX8,   0x66},
    {REG_MTX9,   0xd2},
    {REG_MTXS,   0x65},

    /* NULL reg */
    {0x00,  0x00}
};

static const uint8_t yuv422_regs[][2] = {
    /* See Implementation Guide */
    {REG_COM3,   0x04},  /*  Vario Pixels */
    {REG_COM4,   0x80},  /*  Vario Pixels */
    {REG_COM15,  0xC0},  /*  Output range 0x00-0xFF  */

    /* YUV color matrix */
    {REG_MTX1,   0x3a},
    {REG_MTX2,   0x3d},
    {REG_MTX3,   0x03},

    {REG_MTX4,   0x12},
    {REG_MTX5,   0x26},
    {REG_MTX6,   0x38},

    {REG_MTX7,   0x40},
    {REG_MTX8,   0x40},
    {REG_MTX9,   0x40},
    {REG_MTXS,   0x0d},

    /* NULL reg */
    {0x00,  0x00}
};

static int reset(sensor_t *sensor) {
    int i = 0;
    const uint8_t(*regs)[2] = default_regs;

    /* Reset all registers */
    omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, 0x80);

    /* delay n ms */
    mp_hal_delay_ms(10);

    /* Write initial registers */
    while (regs[i][0]) {
        omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat) {
    int i = 0;
    const uint8_t(*regs)[2];
    uint8_t com7 = 0; /* framesize/RGB */

    /* read pixel format reg */
    omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, &com7);

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            com7 |= REG_COM7_RGB;
            regs = rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
            com7 &= (~REG_COM7_RGB);
            regs = yuv422_regs;
            break;
        case PIXFORMAT_GRAYSCALE:
            com7 &= (~REG_COM7_RGB);
            regs = yuv422_regs;
            break;
        default:
            return -1;
    }

    /* Set pixel format */
    omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, com7);

    /* Write pixel format registers */
    while (regs[i][0]) {
        omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize) {
    uint8_t com7 = 0; /* framesize/RGB */
    uint8_t com1 = 0; /* Skip option */

    /* read COM7 RGB bit */
    omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, &com7);
    com7 &= REG_COM7_RGB;

    switch (framesize) {
        case FRAMESIZE_QQCIF:
            com7 |= REG_COM7_QCIF;
            com1 |= REG_COM1_QQCIF | REG_COM1_SKIP2;
            break;
        case FRAMESIZE_QQVGA:
            com7 |= REG_COM7_QVGA;
            com1 |= REG_COM1_QQVGA | REG_COM1_SKIP2;
            break;
        case FRAMESIZE_QCIF:
            com7 |= REG_COM7_QCIF;
            break;
        default:
            return -1;
    }

    /* write the frame size registers */
    omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM1, com1);
    omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, com7);

    return 0;
}

static int set_brightness(sensor_t *sensor, int level) {
    int i;

    static uint8_t regs[NUM_BR_LEVELS + 1][3] = {
        { REG_AEW, REG_AEB, REG_VPT },
        { 0x1c, 0x12, 0x50 }, /* -3 */
        { 0x3d, 0x30, 0x71 }, /* -2 */
        { 0x50, 0x44, 0x92 }, /* -1 */
        { 0x70, 0x64, 0xc3 }, /*  0 */
        { 0x90, 0x84, 0xd4 }, /* +1 */
        { 0xc4, 0xbf, 0xf9 }, /* +2 */
        { 0xd8, 0xd0, 0xfa }, /* +3 */
    };

    level += (NUM_BR_LEVELS / 2 + 1);
    if (level < 0 || level > NUM_BR_LEVELS) {
        return -1;
    }

    for (i = 0; i < 3; i++) {
        omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, regs[0][i], regs[level][i]);
    }

    return 0;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling) {
    /* Write gain ceiling register */
    omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM9, (gainceiling << 4));
    return 0;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, &reg);
    ret |=
        omv_i2c_writeb(&sensor->i2c_bus,
                       sensor->slv_addr,
                       REG_COM8,
                       (reg & (~REG_COM8_AGC)) | ((enable != 0) ? REG_COM8_AGC : 0));

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinf(gain_db))) {
        float gain = IM_MAX(IM_MIN(expf((gain_db / 20.0f) * M_LN10), 128.0f), 1.0f);

        int gain_temp = fast_ceilf(logf(IM_MAX(gain / 2.0f, 1.0f)) / M_LN2);
        int gain_hi = 0x3F >> (6 - gain_temp);
        int gain_lo = IM_MIN(fast_roundf(((gain / (1 << gain_temp)) - 1.0f) * 16.0f), 15);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_GAIN, ((gain_hi & 0x0F) << 4) | (gain_lo << 0));
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_VREF, &reg);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_VREF, ((gain_hi & 0x30) << 2) | (reg & 0x3F));
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinf(gain_db_ceiling))) {
        float gain_ceiling = IM_MAX(IM_MIN(expf((gain_db_ceiling / 20.0f) * M_LN10), 128.0f), 2.0f);

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM9, &reg);
        ret |=
            omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM9,
                           (reg & 0x8F) | ((fast_ceilf(logf(gain_ceiling) / M_LN2) - 1) << 4));
    }

    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db) {
    uint8_t reg, gain_lo, gain_hi;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, &reg);

    // DISABLED
    // if (reg & REG_COM8_AGC) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, reg & (~REG_COM8_AGC));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_GAIN, &gain_lo);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_VREF, &gain_hi);

    // DISABLED
    // if (reg & REG_COM8_AGC) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, reg | REG_COM8_AGC);
    // }
    // DISABLED

    int gain = ((gain_hi & 0xC0) << 2) | gain_lo;
    int hi_gain = 1 <<
                  (((gain >>
                     9) & 1) +
                   ((gain >> 8) & 1) + ((gain >> 7) & 1) + ((gain >> 6) & 1) + ((gain >> 5) & 1) + ((gain >> 4) & 1));
    float lo_gain = 1.0f + (((gain >> 0) & 0xF) / 16.0f);
    *gain_db = 20.0f * log10f(hi_gain * lo_gain);

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, &reg);
    ret |=
        omv_i2c_writeb(&sensor->i2c_bus,
                       sensor->slv_addr,
                       REG_COM8,
                       (reg & (~REG_COM8_AEC)) | ((enable != 0) ? REG_COM8_AEC : 0));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, &reg);
        int t_line = 0, t_pclk = (reg & REG_COM7_RGB) ? 2 : 1;

        if (reg & REG_COM7_VGA) {
            t_line = 640 + 160;
        }
        if (reg & REG_COM7_CIF) {
            t_line = 352 + 168;
        }
        if (reg & REG_COM7_QVGA) {
            t_line = 320 + 80;
        }
        if (reg & REG_COM7_QCIF) {
            t_line = 176 + 84;
        }

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_CLKRC, &reg);
        int pll_mult = (reg & REG_CLKRC_DOUBLE) ? 2 : 1;
        int clk_rc = ((reg & REG_CLKRC_DIVIDER_MASK) + 1) * 2;

        int exposure =
            IM_MAX(IM_MIN(((exposure_us * (((OMV_XCLK_FREQUENCY / clk_rc) * pll_mult) / 1000000)) / t_pclk) / t_line, 0xFFFF),
                   0x0000);

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM1, &reg);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM1, (reg & 0xFC) | ((exposure >> 0) & 0x3));

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_AECH, &reg);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_AECH, (reg & 0x00) | ((exposure >> 2) & 0xFF));

        ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_AECHM, &reg);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_AECHM, (reg & 0xC0) | ((exposure >> 10) & 0x3F));
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us) {
    uint8_t reg, aec_10, aec_92, aec_1510;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, &reg);

    // DISABLED
    // if (reg & REG_COM8_AEC) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, reg & (~REG_COM8_AEC));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM1, &aec_10);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_AECH, &aec_92);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_AECHM, &aec_1510);

    // DISABLED
    // if (reg & REG_COM8_AEC) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, reg | REG_COM8_AEC);
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM7, &reg);
    int t_line = 0, t_pclk = (reg & REG_COM7_RGB) ? 2 : 1;

    if (reg & REG_COM7_VGA) {
        t_line = 640 + 160;
    }
    if (reg & REG_COM7_CIF) {
        t_line = 352 + 168;
    }
    if (reg & REG_COM7_QVGA) {
        t_line = 320 + 80;
    }
    if (reg & REG_COM7_QCIF) {
        t_line = 176 + 84;
    }

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_CLKRC, &reg);
    int pll_mult = (reg & REG_CLKRC_DOUBLE) ? 2 : 1;
    int clk_rc = ((reg & REG_CLKRC_DIVIDER_MASK) + 1) * 2;

    uint16_t exposure = ((aec_1510 & 0x3F) << 10) + ((aec_92 & 0xFF) << 2) + ((aec_10 & 0x3) << 0);
    *exposure_us = (exposure * t_line * t_pclk) / (((OMV_XCLK_FREQUENCY / clk_rc) * pll_mult) / 1000000);

    return ret;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    uint8_t reg;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, &reg);
    ret |=
        omv_i2c_writeb(&sensor->i2c_bus,
                       sensor->slv_addr,
                       REG_COM8,
                       (reg & (~REG_COM8_AWB)) | ((enable != 0) ? REG_COM8_AWB : 0));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))
        && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) && (!isinff(b_gain_db))) {
        int r_gain = IM_MAX(IM_MIN(fast_roundf(expf((r_gain_db / 20.0f) * M_LN10) * 128.0f), 255), 0);
        int b_gain = IM_MAX(IM_MIN(fast_roundf(expf((b_gain_db / 20.0f) * M_LN10) * 128.0f), 255), 0);

        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_BLUE, b_gain);
        ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_RED, r_gain);
    }

    return ret;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    uint8_t reg, blue, red;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, &reg);

    // DISABLED
    // if (reg & REG_COM8_AWB) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, reg & (~REG_COM8_AWB));
    // }
    // DISABLED

    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_BLUE, &blue);
    ret |= omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_RED, &red);

    // DISABLED
    // if (reg & REG_COM8_AWB) {
    //     ret |= omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_COM8, reg | REG_COM8_AWB);
    // }
    // DISABLED

    *r_gain_db = 20.0f * log10f(red / 128.0f);
    *b_gain_db = 20.0f * log10f(blue / 128.0f);

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable) {
    uint8_t val;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_MVFP, &val);
    ret |=
        omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_MVFP,
                       enable ? (val | REG_MVFP_HMIRROR) : (val & (~REG_MVFP_HMIRROR)));

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable) {
    uint8_t val;
    int ret = omv_i2c_readb(&sensor->i2c_bus, sensor->slv_addr, REG_MVFP, &val);
    ret |=
        omv_i2c_writeb(&sensor->i2c_bus, sensor->slv_addr, REG_MVFP,
                       enable ? (val | REG_MVFP_VFLIP) : (val & (~REG_MVFP_VFLIP)));

    return ret;
}

int ov9650_init(sensor_t *sensor) {
    // Initialize sensor structure.
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_brightness = set_brightness;
    sensor->set_gainceiling = set_gainceiling;
    sensor->set_auto_gain = set_auto_gain;
    sensor->get_gain_db = get_gain_db;
    sensor->set_auto_exposure = set_auto_exposure;
    sensor->get_exposure_us = get_exposure_us;
    sensor->set_auto_whitebal = set_auto_whitebal;
    sensor->get_rgb_gain_db = get_rgb_gain_db;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;

    // Set sensor flags
    sensor->hw_flags.vsync = 1;
    sensor->hw_flags.hsync = 0;
    sensor->hw_flags.pixck = 1;
    sensor->hw_flags.fsync = 0;
    sensor->hw_flags.jpege = 0;
    sensor->hw_flags.gs_bpp = 2;
    sensor->hw_flags.rgb_swap = 1;
    sensor->hw_flags.yuv_order = SENSOR_HW_FLAGS_YVU422;

    return 0;
}
#endif // (OMV_ENABLE_OV9650 == 1)
