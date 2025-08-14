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
 * OV2640 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_OV2640_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "omv_i2c.h"
#include "omv_csi.h"
#include "ov2640.h"
#include "py/mphal.h"

#define CIF_WIDTH      (400)
#define CIF_HEIGHT     (296)

#define SVGA_WIDTH     (800)
#define SVGA_HEIGHT    (600)

#define UXGA_WIDTH     (1600)
#define UXGA_HEIGHT    (1200)

static const uint8_t default_regs[][2] = {

// From Linux Driver.

    {BANK_SEL,      BANK_SEL_DSP},
    {0x2c,          0xff},
    {0x2e,          0xdf},
    {BANK_SEL,      BANK_SEL_SENSOR},
    {0x3c,          0x32},
    {CLKRC,         CLKRC_DOUBLE},
    {COM2,          COM2_OUT_DRIVE_3x},
    {REG04,         REG04_SET(REG04_HFLIP_IMG | REG04_VFLIP_IMG | REG04_VREF_EN | REG04_HREF_EN)},
    {COM8,          COM8_SET(COM8_BNDF_EN | COM8_AGC_EN | COM8_AEC_EN)},
    {COM9,          COM9_AGC_SET(COM9_AGC_GAIN_8x)},
    {0x2c,          0x0c},
    {0x33,          0x78},
    {0x3a,          0x33},
    {0x3b,          0xfb},
    {0x3e,          0x00},
    {0x43,          0x11},
    {0x16,          0x10},
    {0x39,          0x02},
    {0x35,          0x88},
    {0x22,          0x0a},
    {0x37,          0x40},
    {0x23,          0x00},
    {ARCOM2,        0xa0},
    {0x06,          0x02},
    {0x06,          0x88},
    {0x07,          0xc0},
    {0x0d,          0xb7},
    {0x0e,          0x01},
    {0x4c,          0x00},
    {0x4a,          0x81},
    {0x21,          0x99},
    {AEW,           0x40},
    {AEB,           0x38},
    {VV,            VV_AGC_TH_SET(0x08, 0x02)},
    {0x5c,          0x00},
    {0x63,          0x00},
    {FLL,           0x22},
    {COM3,          COM3_BAND_SET(COM3_BAND_AUTO)},
    {REG5D,         0x55},
    {REG5E,         0x7d},
    {REG5F,         0x7d},
    {REG60,         0x55},
    {HISTO_LOW,     0x70},
    {HISTO_HIGH,    0x80},
    {0x7c,          0x05},
    {0x20,          0x80},
    {0x28,          0x30},
    {0x6c,          0x00},
    {0x6d,          0x80},
    {0x6e,          0x00},
    {0x70,          0x02},
    {0x71,          0x94},
    {0x73,          0xc1},
    {0x3d,          0x34},
    {COM7,          COM7_RES_UXGA | COM7_ZOOM_EN},
    {0x5a,          0x57},
    {COM25,         0x00},
    {BD50,          0xbb},
    {BD60,          0x9c},
    {BANK_SEL,      BANK_SEL_DSP},
    {0xe5,          0x7f},
    {MC_BIST,       MC_BIST_RESET | MC_BIST_BOOT_ROM_SEL},
    {0x41,          0x24},
    {RESET,         RESET_JPEG | RESET_DVP},
    {0x76,          0xff},
    {0x33,          0xa0},
    {0x42,          0x20},
    {0x43,          0x18},
    {0x4c,          0x00},
    {CTRL3,         CTRL3_BPC_EN | CTRL3_WPC_EN | 0x10},
    {0x88,          0x3f},
    {0xd7,          0x03},
    {0xd9,          0x10},
    {R_DVP_SP,      R_DVP_SP_AUTO_MODE | 0x2},
    {0xc8,          0x08},
    {0xc9,          0x80},
    {BPADDR,        0x00},
    {BPDATA,        0x00},
    {BPADDR,        0x03},
    {BPDATA,        0x48},
    {BPDATA,        0x48},
    {BPADDR,        0x08},
    {BPDATA,        0x20},
    {BPDATA,        0x10},
    {BPDATA,        0x0e},
    {0x90,          0x00},
    {0x91,          0x0e},
    {0x91,          0x1a},
    {0x91,          0x31},
    {0x91,          0x5a},
    {0x91,          0x69},
    {0x91,          0x75},
    {0x91,          0x7e},
    {0x91,          0x88},
    {0x91,          0x8f},
    {0x91,          0x96},
    {0x91,          0xa3},
    {0x91,          0xaf},
    {0x91,          0xc4},
    {0x91,          0xd7},
    {0x91,          0xe8},
    {0x91,          0x20},
    {0x92,          0x00},
    {0x93,          0x06},
    {0x93,          0xe3},
    {0x93,          0x03},
    {0x93,          0x03},
    {0x93,          0x00},
    {0x93,          0x02},
    {0x93,          0x00},
    {0x93,          0x00},
    {0x93,          0x00},
    {0x93,          0x00},
    {0x93,          0x00},
    {0x93,          0x00},
    {0x93,          0x00},
    {0x96,          0x00},
    {0x97,          0x08},
    {0x97,          0x19},
    {0x97,          0x02},
    {0x97,          0x0c},
    {0x97,          0x24},
    {0x97,          0x30},
    {0x97,          0x28},
    {0x97,          0x26},
    {0x97,          0x02},
    {0x97,          0x98},
    {0x97,          0x80},
    {0x97,          0x00},
    {0x97,          0x00},
    {0xa4,          0x00},
    {0xa8,          0x00},
    {0xc5,          0x11},
    {0xc6,          0x51},
    {0xbf,          0x80},
    {0xc7,          0x10},  /* simple AWB */
    {0xb6,          0x66},
    {0xb8,          0xA5},
    {0xb7,          0x64},
    {0xb9,          0x7C},
    {0xb3,          0xaf},
    {0xb4,          0x97},
    {0xb5,          0xFF},
    {0xb0,          0xC5},
    {0xb1,          0x94},
    {0xb2,          0x0f},
    {0xc4,          0x5c},
    {0xa6,          0x00},
    {0xa7,          0x20},
    {0xa7,          0xd8},
    {0xa7,          0x1b},
    {0xa7,          0x31},
    {0xa7,          0x00},
    {0xa7,          0x18},
    {0xa7,          0x20},
    {0xa7,          0xd8},
    {0xa7,          0x19},
    {0xa7,          0x31},
    {0xa7,          0x00},
    {0xa7,          0x18},
    {0xa7,          0x20},
    {0xa7,          0xd8},
    {0xa7,          0x19},
    {0xa7,          0x31},
    {0xa7,          0x00},
    {0xa7,          0x18},
    {0x7f,          0x00},
    {0xe5,          0x1f},
    {0xe1,          0x77},
    {0xdd,          0x7f},
    {CTRL0,         CTRL0_YUV422 | CTRL0_YUV_EN | CTRL0_RGB_EN},

// OpenMV Custom.

    {BANK_SEL,      BANK_SEL_SENSOR},
    {0x0f,          0x4b},
    {COM1,          0x8f},

// End.

    {0x00,          0x00},
};

// Looks really bad.
//static const uint8_t cif_regs[][2] = {
//    {BANK_SEL,  BANK_SEL_SENSOR},
//    {COM7,      COM7_RES_CIF},
//    {COM1,      0x06 | 0x80},
//    {HSTART,    0x11},
//    {HSTOP,     0x43},
//    {VSTART,    0x01}, // 0x01 fixes issue with garbage pixels in the image...
//    {VSTOP,     0x97},
//    {REG32,     0x09},
//    {BANK_SEL,  BANK_SEL_DSP},
//    {RESET,     RESET_DVP},
//    {SIZEL,     SIZEL_HSIZE8_11_SET(CIF_WIDTH) | SIZEL_HSIZE8_SET(CIF_WIDTH) | SIZEL_VSIZE8_SET(CIF_HEIGHT)},
//    {HSIZE8,    HSIZE8_SET(CIF_WIDTH)},
//    {VSIZE8,    VSIZE8_SET(CIF_HEIGHT)},
//    {CTRL2,     CTRL2_DCW_EN | CTRL2_SDE_EN | CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN},
//    {0,         0},
//};

static const uint8_t svga_regs[][2] = {
    {BANK_SEL,  BANK_SEL_SENSOR},
    {COM7,      COM7_RES_SVGA},
    {COM1,      0x0A | 0x80},
    {HSTART,    0x11},
    {HSTOP,     0x43},
    {VSTART,    0x01}, // 0x01 fixes issue with garbage pixels in the image...
    {VSTOP,     0x97},
    {REG32,     0x09},
    {BANK_SEL,  BANK_SEL_DSP},
    {RESET,     RESET_DVP},
    {SIZEL,     SIZEL_HSIZE8_11_SET(SVGA_WIDTH) | SIZEL_HSIZE8_SET(SVGA_WIDTH) | SIZEL_VSIZE8_SET(SVGA_HEIGHT)},
    {HSIZE8,    HSIZE8_SET(SVGA_WIDTH)},
    {VSIZE8,    VSIZE8_SET(SVGA_HEIGHT)},
    {CTRL2,     CTRL2_DCW_EN | CTRL2_SDE_EN | CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN},
    {0,         0},
};

static const uint8_t uxga_regs[][2] = {
    {BANK_SEL,  BANK_SEL_SENSOR},
    {COM7,      COM7_RES_UXGA},
    {COM1,      0x0F | 0x80},
    {HSTART,    0x11},
    {HSTOP,     0x75},
    {VSTART,    0x01},
    {VSTOP,     0x97},
    {REG32,     0x36},
    {BANK_SEL,  BANK_SEL_DSP},
    {RESET,     RESET_DVP},
    {SIZEL,     SIZEL_HSIZE8_11_SET(UXGA_WIDTH) | SIZEL_HSIZE8_SET(UXGA_WIDTH) | SIZEL_VSIZE8_SET(UXGA_HEIGHT)},
    {HSIZE8,    HSIZE8_SET(UXGA_WIDTH)},
    {VSIZE8,    VSIZE8_SET(UXGA_HEIGHT)},
    {CTRL2,     CTRL2_DCW_EN | CTRL2_SDE_EN | CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN},
    {0,         0},
};

static const uint8_t yuv422_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_YUV422},
    {0xd7,          0x03},
    {0x33,          0xa0},
    {0xe5,          0x1f},
    {0xe1,          0x67},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static const uint8_t rgb565_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_RGB565},
    {0xd7,          0x03},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static const uint8_t bayer_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_RAW10},
    {0xd7,          0x03},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static const uint8_t jpeg_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_JPEG_EN},
    {0xd7,          0x03},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

#define NUM_BRIGHTNESS_LEVELS    (5)
static const uint8_t brightness_regs[NUM_BRIGHTNESS_LEVELS + 1][5] = {
    {BPADDR, BPDATA, BPADDR, BPDATA, BPDATA},
    {0x00, 0x04, 0x09, 0x00, 0x00}, /* -2 */
    {0x00, 0x04, 0x09, 0x10, 0x00}, /* -1 */
    {0x00, 0x04, 0x09, 0x20, 0x00}, /*  0 */
    {0x00, 0x04, 0x09, 0x30, 0x00}, /* +1 */
    {0x00, 0x04, 0x09, 0x40, 0x00}, /* +2 */
};

#define NUM_CONTRAST_LEVELS    (5)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS + 1][7] = {
    {BPADDR, BPDATA, BPADDR, BPDATA, BPDATA, BPDATA, BPDATA},
    {0x00, 0x04, 0x07, 0x20, 0x18, 0x34, 0x06}, /* -2 */
    {0x00, 0x04, 0x07, 0x20, 0x1c, 0x2a, 0x06}, /* -1 */
    {0x00, 0x04, 0x07, 0x20, 0x20, 0x20, 0x06}, /*  0 */
    {0x00, 0x04, 0x07, 0x20, 0x24, 0x16, 0x06}, /* +1 */
    {0x00, 0x04, 0x07, 0x20, 0x28, 0x0c, 0x06}, /* +2 */
};

#define NUM_SATURATION_LEVELS    (5)
static const uint8_t saturation_regs[NUM_SATURATION_LEVELS + 1][5] = {
    {BPADDR, BPDATA, BPADDR, BPDATA, BPDATA},
    {0x00, 0x02, 0x03, 0x28, 0x28}, /* -2 */
    {0x00, 0x02, 0x03, 0x38, 0x38}, /* -1 */
    {0x00, 0x02, 0x03, 0x48, 0x48}, /*  0 */
    {0x00, 0x02, 0x03, 0x58, 0x58}, /* +1 */
    {0x00, 0x02, 0x03, 0x68, 0x68}, /* +2 */
};

static int reset(omv_csi_t *csi) {
    // Reset all registers
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM7, COM7_SRST);

    // Delay 5 ms
    mp_hal_delay_ms(5);

    // Write default registers
    for (int i = 0; default_regs[i][0]; i++) {
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Delay 300 ms
    if (!csi->disable_delays) {
        mp_hal_delay_ms(300);
    }

    return ret;
}

static int sleep(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb(csi->i2c, csi->slv_addr, COM2, &reg);

    if (enable) {
        reg |= COM2_STDBY;
    } else {
        reg &= ~COM2_STDBY;
    }

    // Write back register
    return omv_i2c_writeb(csi->i2c, csi->slv_addr, COM2, reg) | ret;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint8_t reg_data;
    if (omv_i2c_readb(csi->i2c, csi->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return omv_i2c_writeb(csi->i2c, csi->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    const uint8_t(*regs)[2];
    int ret = 0;

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            regs = rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            regs = yuv422_regs;
            break;
        case PIXFORMAT_BAYER:
            regs = bayer_regs;
            break;
        case PIXFORMAT_JPEG:
            regs = jpeg_regs;
            break;
        default:
            return -1;
    }

    // Write registers
    for (int i = 0; regs[i][0]; i++) {
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, regs[i][0], regs[i][1]);
    }

    return ret;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    const uint8_t(*regs)[2];
    uint16_t sensor_w = 0;
    uint16_t sensor_h = 0;
    int ret = 0;
    uint16_t w = csi->resolution[framesize][0];
    uint16_t h = csi->resolution[framesize][1];

    if ((w % 4) || (h % 4) || (w > UXGA_WIDTH) || (h > UXGA_HEIGHT)) {
        // w/h must be divisible by 4
        return -1;
    }

    // Looks really bad.
    /* if ((w <= CIF_WIDTH) && (h <= CIF_HEIGHT)) {
        regs = cif_regs;
        sensor_w = CIF_WIDTH;
        sensor_h = CIF_HEIGHT;
       } else */if ((w <= SVGA_WIDTH) && (h <= SVGA_HEIGHT)) {
        regs = svga_regs;
        sensor_w = SVGA_WIDTH;
        sensor_h = SVGA_HEIGHT;
    } else {
        regs = uxga_regs;
        sensor_w = UXGA_WIDTH;
        sensor_h = UXGA_HEIGHT;
    }

    // Write setup registers
    for (int i = 0; regs[i][0]; i++) {
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, regs[i][0], regs[i][1]);
    }

    uint64_t tmp_div = IM_MIN(sensor_w / w, sensor_h / h);
    uint16_t log_div = IM_MIN(IM_LOG2(tmp_div) - 1, 3);
    uint16_t div = 1 << log_div;
    uint16_t w_mul = w * div;
    uint16_t h_mul = h * div;
    uint16_t x_off = (sensor_w - w_mul) / 2;
    uint16_t y_off = (sensor_h - h_mul) / 2;

    ret |=
        omv_i2c_writeb(csi->i2c, csi->slv_addr, CTRLI,
                       CTRLI_LP_DP | CTRLI_V_DIV_SET(log_div) | CTRLI_H_DIV_SET(log_div));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, HSIZE, HSIZE_SET(w_mul));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, VSIZE, VSIZE_SET(h_mul));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, XOFFL, XOFFL_SET(x_off));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, YOFFL, YOFFL_SET(y_off));
    ret |= omv_i2c_writeb(csi->i2c,
                          csi->slv_addr,
                          VHYX,
                          VHYX_HSIZE_SET(w_mul) | VHYX_VSIZE_SET(h_mul) | VHYX_XOFF_SET(x_off) | VHYX_YOFF_SET(y_off));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, TEST, TEST_HSIZE_SET(w_mul));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, ZMOW, ZMOW_OUTW_SET(w));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, ZMOH, ZMOH_OUTH_SET(h));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, ZMHH, ZMHH_OUTW_SET(w) | ZMHH_OUTH_SET(h));
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, R_DVP_SP, div);
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, RESET, 0x00);

    return ret;
}

static int set_contrast(omv_csi_t *csi, int level) {
    int ret = 0;

    level += (NUM_CONTRAST_LEVELS / 2) + 1;
    if (level <= 0 || level > NUM_CONTRAST_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write contrast registers */
    for (int i = 0; i < sizeof(contrast_regs[0]) / sizeof(contrast_regs[0][0]); i++) {
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, contrast_regs[0][i], contrast_regs[level][i]);
    }

    return ret;
}

static int set_brightness(omv_csi_t *csi, int level) {
    int ret = 0;

    level += (NUM_BRIGHTNESS_LEVELS / 2) + 1;
    if (level <= 0 || level > NUM_BRIGHTNESS_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write brightness registers */
    for (int i = 0; i < sizeof(brightness_regs[0]) / sizeof(brightness_regs[0][0]); i++) {
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, brightness_regs[0][i], brightness_regs[level][i]);
    }

    return ret;
}

static int set_saturation(omv_csi_t *csi, int level) {
    int ret = 0;

    level += (NUM_SATURATION_LEVELS / 2) + 1;
    if (level <= 0 || level > NUM_SATURATION_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write saturation registers */
    for (int i = 0; i < sizeof(saturation_regs[0]) / sizeof(saturation_regs[0][0]); i++) {
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, saturation_regs[0][i], saturation_regs[level][i]);
    }

    return ret;
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    int ret = 0;

    /* Switch to SENSOR register bank */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Write gain ceiling register */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM9, COM9_AGC_SET(gainceiling));

    return ret;
}

static int set_quality(omv_csi_t *csi, int qs) {
    int ret = 0;

    /* Switch to DSP register bank */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write QS register */
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, QS, qs);

    return ret;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM7, &reg);

    if (enable) {
        reg |= COM7_COLOR_BAR;
    } else {
        reg &= ~COM7_COLOR_BAR;
    }

    return omv_i2c_writeb(csi->i2c, csi->slv_addr, COM7, reg) | ret;
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM8, &reg);
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM8, (reg & (~COM8_AGC_EN)) | ((enable != 0) ? COM8_AGC_EN : 0));

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        float gain = IM_CLAMP(expf((gain_db / 20.0f) * M_LN10), 1.0f, 32.0f);

        int gain_temp = fast_ceilf(logf(IM_MAX(gain / 2.0f, 1.0f)) / M_LN2);
        int gain_hi = 0xF >> (4 - gain_temp);
        int gain_lo = IM_MIN(fast_roundf(((gain / (1 << gain_temp)) - 1.0f) * 16.0f), 15);

        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, GAIN, (gain_hi << 4) | (gain_lo << 0));
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        float gain_ceiling = IM_CLAMP(expf((gain_db_ceiling / 20.0f) * M_LN10), 2.0f, 128.0f);

        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM9, &reg);
        ret |=
            omv_i2c_writeb(csi->i2c, csi->slv_addr, COM9,
                           (reg & 0x1F) | ((fast_ceilf(logf(gain_ceiling) / M_LN2) - 1) << 5));
    }

    return ret;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    uint8_t reg, gain;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM8, reg & (~COM8_AGC_EN));
    // }
    // DISABLED

    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, GAIN, &gain);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM8, reg | COM8_AGC_EN);
    // }
    // DISABLED

    int hi_gain = 1 << (((gain >> 7) & 1) + ((gain >> 6) & 1) + ((gain >> 5) & 1) + ((gain >> 4) & 1));
    float lo_gain = 1.0f + (((gain >> 0) & 0xF) / 16.0f);
    *gain_db = 20.0f * log10f(hi_gain * lo_gain);

    return ret;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM8, &reg);
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM8, COM8_SET_AEC(reg, (enable != 0)));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM7, &reg);
        int t_line = 0;

        if (COM7_GET_RES(reg) == COM7_RES_UXGA) {
            t_line = 1600 + 322;
        }
        if (COM7_GET_RES(reg) == COM7_RES_SVGA) {
            t_line = 800 + 390;
        }
        if (COM7_GET_RES(reg) == COM7_RES_CIF) {
            t_line = 400 + 195;
        }

        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, CLKRC, &reg);
        int pll_mult = ((reg & CLKRC_DOUBLE) ? 2 : 1) * 3;
        int clk_rc = (reg & CLKRC_DIVIDER_MASK) + 2;

        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);
        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, IMAGE_MODE, &reg);
        int t_pclk = 0;

        if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_YUV422) {
            t_pclk = 2;
        }
        if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RAW10) {
            t_pclk = 1;
        }
        if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RGB565) {
            t_pclk = 2;
        }

        int exposure =
            __USAT(((exposure_us * (((OMV_CSI_CLK_FREQUENCY / clk_rc) * pll_mult) / 1000000)) / t_pclk) / t_line, 16);

        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, REG04, &reg);
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, REG04, (reg & 0xFC) | ((exposure >> 0) & 0x3));

        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, AEC, &reg);
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, AEC, (reg & 0x00) | ((exposure >> 2) & 0xFF));

        ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, REG45, &reg);
        ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, REG45, (reg & 0xC0) | ((exposure >> 10) & 0x3F));
    }

    return ret;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    uint8_t reg, aec_10, aec_92, aec_1510;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM8, reg & (~COM8_AEC_EN));
    // }
    // DISABLED

    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, REG04, &aec_10);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, AEC, &aec_92);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, REG45, &aec_1510);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, COM8, reg | COM8_AEC_EN);
    // }
    // DISABLED

    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, COM7, &reg);
    int t_line = 0;

    if (COM7_GET_RES(reg) == COM7_RES_UXGA) {
        t_line = 1600 + 322;
    }
    if (COM7_GET_RES(reg) == COM7_RES_SVGA) {
        t_line = 800 + 390;
    }
    if (COM7_GET_RES(reg) == COM7_RES_CIF) {
        t_line = 400 + 195;
    }

    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, CLKRC, &reg);
    int pll_mult = ((reg & CLKRC_DOUBLE) ? 2 : 1) * 3;
    int clk_rc = (reg & CLKRC_DIVIDER_MASK) + 2;

    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, IMAGE_MODE, &reg);
    int t_pclk = 0;

    if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_YUV422) {
        t_pclk = 2;
    }
    if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RAW10) {
        t_pclk = 1;
    }
    if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RGB565) {
        t_pclk = 2;
    }

    uint16_t exposure = ((aec_1510 & 0x3F) << 10) + ((aec_92 & 0xFF) << 2) + ((aec_10 & 0x3) << 0);
    *exposure_us = (exposure * t_line * t_pclk) / (((OMV_CSI_CLK_FREQUENCY / clk_rc) * pll_mult) / 1000000);

    return ret;
}

static int set_auto_whitebal(omv_csi_t *csi, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, CTRL1, &reg);
    ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, CTRL1, (reg & (~CTRL1_AWB)) | ((enable != 0) ? CTRL1_AWB : 0));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))
        && (!isinff(r_gain_db)) && (!isinff(g_gain_db)) && (!isinff(b_gain_db))) {
    }

    return ret;
}

static int get_rgb_gain_db(omv_csi_t *csi, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, CTRL1, &reg);

    // DISABLED
    // if (reg & CTRL1_AWB) {
    //     ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, CTRL1, reg & (~CTRL1_AWB));
    // }
    // DISABLED

    // DISABLED
    // if (reg & CTRL1_AWB) {
    //     ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, CTRL1, reg | CTRL1_AWB);
    // }
    // DISABLED

    *r_gain_db = NAN;
    *g_gain_db = NAN;
    *b_gain_db = NAN;

    return ret;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, REG04, &reg);

    if (!enable) {
        // Already mirrored.
        reg |= REG04_HFLIP_IMG;
    } else {
        reg &= ~REG04_HFLIP_IMG;
    }

    return omv_i2c_writeb(csi->i2c, csi->slv_addr, REG04, reg) | ret;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= omv_i2c_readb(csi->i2c, csi->slv_addr, REG04, &reg);

    if (!enable) {
        // Already flipped.
        reg |= REG04_VFLIP_IMG | REG04_VREF_EN;
    } else {
        reg &= ~(REG04_VFLIP_IMG | REG04_VREF_EN);
    }

    return omv_i2c_writeb(csi->i2c, csi->slv_addr, REG04, reg) | ret;
}

static int set_special_effect(omv_csi_t *csi, omv_csi_sde_t sde) {
    int ret = 0;

    switch (sde) {
        case OMV_CSI_SDE_NEGATIVE:
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPADDR, 0x00);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPDATA, 0x40);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPADDR, 0x05);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPDATA, 0x80);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPDATA, 0x80);
            break;
        case OMV_CSI_SDE_NORMAL:
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BANK_SEL, BANK_SEL_DSP);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPADDR, 0x00);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPDATA, 0x00);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPADDR, 0x05);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPDATA, 0x80);
            ret |= omv_i2c_writeb(csi->i2c, csi->slv_addr, BPDATA, 0x80);
            break;
        default:
            return -1;
    }

    return ret;
}

int ov2640_init(omv_csi_t *csi) {
    // Initialize csi structure.
    csi->reset = reset;
    csi->sleep = sleep;
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
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_auto_whitebal = set_auto_whitebal;
    csi->get_rgb_gain_db = get_rgb_gain_db;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->set_special_effect = set_special_effect;

    // Set csi flags
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->frame_sync = 0;
    csi->mono_bpp = 2;
    csi->rgb_swap = 0;
    csi->jpg_format = 3;
    csi->yuv_format = SUBFORMAT_ID_YVU422;

    return 0;
}
#endif // (OMV_OV2640_ENABLE == 1)
