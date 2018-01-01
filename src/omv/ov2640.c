/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV2640 driver.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include STM32_HAL_H
#include "cambus.h"
#include "ov2640.h"
#include "systick.h"
#include "ov2640_regs.h"
#include "omv_boardconfig.h"

#define SVGA_HSIZE     (800)
#define SVGA_VSIZE     (600)

#define UXGA_HSIZE     (1600)
#define UXGA_VSIZE     (1200)

static const uint8_t default_regs[][2] = {
    { BANK_SEL, BANK_SEL_DSP },
    { 0x2c,     0xff },
    { 0x2e,     0xdf },
    { BANK_SEL, BANK_SEL_SENSOR },
    { 0x3c,     0x32 },
    { CLKRC,    0x80 }, /* Set PCLK divider */
    { COM2,     COM2_OUT_DRIVE_3x }, /* Output drive x2 */
#ifdef OPENMV2
    { REG04,    0xF8}, /* Mirror/VFLIP/AEC[1:0] */
#else
    { REG04_SET(REG04_HREF_EN)},
#endif
    { COM8,     COM8_SET(COM8_BNDF_EN | COM8_AGC_EN | COM8_AEC_EN) },
    { COM9,     COM9_AGC_SET(COM9_AGC_GAIN_8x)},
    { 0x2c,     0x0c },
    { 0x33,     0x78 },
    { 0x3a,     0x33 },
    { 0x3b,     0xfb },
    { 0x3e,     0x00 },
    { 0x43,     0x11 },
    { 0x16,     0x10 },
    { 0x39,     0x02 },
    { 0x35,     0x88 },
    { 0x22,     0x0a },
    { 0x37,     0x40 },
    { 0x23,     0x00 },
    { ARCOM2,   0xa0 },
    { 0x06,     0x02 },
    { 0x06,     0x88 },
    { 0x07,     0xc0 },
    { 0x0d,     0xb7 },
    { 0x0e,     0x01 },
    { 0x4c,     0x00 },
    { 0x4a,     0x81 },
    { 0x21,     0x99 },
    { AEW,      0x40 },
    { AEB,      0x38 },
    /* AGC/AEC fast mode operating region */
    { VV,       VV_AGC_TH_SET(0x08, 0x02) },
    { COM19,    0x00 }, /* Zoom control 2 MSBs */
    { ZOOMS,    0x00 }, /* Zoom control 8 MSBs */
    { 0x5c,     0x00 },
    { 0x63,     0x00 },
    { FLL,      0x00 },
    { FLH,      0x00 },

    /* Set banding filter */
    { COM3,     COM3_BAND_SET(COM3_BAND_AUTO) },
    { REG5D,    0x55 },
    { REG5E,    0x7d },
    { REG5F,    0x7d },
    { REG60,    0x55 },
    { HISTO_LOW,   0x70 },
    { HISTO_HIGH,  0x80 },
    { 0x7c,     0x05 },
    { 0x20,     0x80 },
    { 0x28,     0x30 },
    { 0x6c,     0x00 },
    { 0x6d,     0x80 },
    { 0x6e,     0x00 },
    { 0x70,     0x02 },
    { 0x71,     0x94 },
    { 0x73,     0xc1 },
    { 0x3d,     0x34 },
    //{ COM7,   COM7_RES_UXGA | COM7_ZOOM_EN },
    { 0x5a,     0x57 },
    { BD50,     0xbb },
    { BD60,     0x9c },

    { BANK_SEL, BANK_SEL_DSP },
    { 0xe5,     0x7f },
    { MC_BIST,  MC_BIST_RESET | MC_BIST_BOOT_ROM_SEL },
    { 0x41,     0x24 },
    { RESET,    RESET_JPEG | RESET_DVP },
    { 0x76,     0xff },
    { 0x33,     0xa0 },
    { 0x42,     0x20 },
    { 0x43,     0x18 },
    { 0x4c,     0x00 },
    { CTRL3,    CTRL3_BPC_EN | CTRL3_WPC_EN | 0x10 },
    { 0x88,     0x3f },
    { 0xd7,     0x03 },
    { 0xd9,     0x10 },
    { R_DVP_SP , R_DVP_SP_AUTO_MODE | 0x2 },
    { 0xc8,     0x08 },
    { 0xc9,     0x80 },
    { BPADDR,   0x00 },
    { BPDATA,   0x00 },
    { BPADDR,   0x03 },
    { BPDATA,   0x48 },
    { BPDATA,   0x48 },
    { BPADDR,   0x08 },
    { BPDATA,   0x20 },
    { BPDATA,   0x10 },
    { BPDATA,   0x0e },
    { 0x90,     0x00 },
    { 0x91,     0x0e },
    { 0x91,     0x1a },
    { 0x91,     0x31 },
    { 0x91,     0x5a },
    { 0x91,     0x69 },
    { 0x91,     0x75 },
    { 0x91,     0x7e },
    { 0x91,     0x88 },
    { 0x91,     0x8f },
    { 0x91,     0x96 },
    { 0x91,     0xa3 },
    { 0x91,     0xaf },
    { 0x91,     0xc4 },
    { 0x91,     0xd7 },
    { 0x91,     0xe8 },
    { 0x91,     0x20 },
    { 0x92,     0x00 },
    { 0x93,     0x06 },
    { 0x93,     0xe3 },
    { 0x93,     0x03 },
    { 0x93,     0x03 },
    { 0x93,     0x00 },
    { 0x93,     0x02 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x93,     0x00 },
    { 0x96,     0x00 },
    { 0x97,     0x08 },
    { 0x97,     0x19 },
    { 0x97,     0x02 },
    { 0x97,     0x0c },
    { 0x97,     0x24 },
    { 0x97,     0x30 },
    { 0x97,     0x28 },
    { 0x97,     0x26 },
    { 0x97,     0x02 },
    { 0x97,     0x98 },
    { 0x97,     0x80 },
    { 0x97,     0x00 },
    { 0x97,     0x00 },
    { 0xa4,     0x00 },
    { 0xa8,     0x00 },
    { 0xc5,     0x11 },
    { 0xc6,     0x51 },
    { 0xbf,     0x80 },
    { 0xc7,     0x10 },
    { 0xb6,     0x66 },
    { 0xb8,     0xA5 },
    { 0xb7,     0x64 },
    { 0xb9,     0x7C },
    { 0xb3,     0xaf },
    { 0xb4,     0x97 },
    { 0xb5,     0xFF },
    { 0xb0,     0xC5 },
    { 0xb1,     0x94 },
    { 0xb2,     0x0f },
    { 0xc4,     0x5c },
    { 0xa6,     0x00 },
    { 0xa7,     0x20 },
    { 0xa7,     0xd8 },
    { 0xa7,     0x1b },
    { 0xa7,     0x31 },
    { 0xa7,     0x00 },
    { 0xa7,     0x18 },
    { 0xa7,     0x20 },
    { 0xa7,     0xd8 },
    { 0xa7,     0x19 },
    { 0xa7,     0x31 },
    { 0xa7,     0x00 },
    { 0xa7,     0x18 },
    { 0xa7,     0x20 },
    { 0xa7,     0xd8 },
    { 0xa7,     0x19 },
    { 0xa7,     0x31 },
    { 0xa7,     0x00 },
    { 0xa7,     0x18 },
    { 0x7f,     0x00 },
    { 0xe5,     0x1f },
    { 0xe1,     0x77 },
    { 0xdd,     0x7f },
    { CTRL0,    CTRL0_YUV422 | CTRL0_YUV_EN | CTRL0_RGB_EN },
    { 0x00,     0x00 }
};

static const uint8_t svga_regs[][2] = {
        { BANK_SEL, BANK_SEL_SENSOR },
        /* DSP input image resoultion and window size control */
        { COM7,    COM7_RES_SVGA},
        { COM1,    0x0F }, /* UXGA=0x0F, SVGA=0x0A, CIF=0x06 */
        { REG32,   0x09 }, /* UXGA=0x36, SVGA/CIF=0x09 */

        { HSTART,  0x11 }, /* UXGA=0x11, SVGA/CIF=0x11 */
        { HSTOP,   0x43 }, /* UXGA=0x75, SVGA/CIF=0x43 */

        { VSTART,  0x00 }, /* UXGA=0x01, SVGA/CIF=0x00 */
        { VSTOP,   0x4b }, /* UXGA=0x97, SVGA/CIF=0x4b */
        { 0x3d,    0x38 }, /* UXGA=0x34, SVGA/CIF=0x38 */

        { 0x35,    0xda },
        { 0x22,    0x1a },
        { 0x37,    0xc3 },
        { 0x34,    0xc0 },
        { 0x06,    0x88 },
        { 0x0d,    0x87 },
        { 0x0e,    0x41 },
        { 0x42,    0x03 },

        /* Set DSP input image size and offset.
           The sensor output image can be scaled with OUTW/OUTH */
        { BANK_SEL, BANK_SEL_DSP },
        { R_BYPASS, R_BYPASS_DSP_BYPAS },

        { RESET,   RESET_DVP },
        { HSIZE8,  (SVGA_HSIZE>>3)}, /* Image Horizontal Size HSIZE[10:3] */
        { VSIZE8,  (SVGA_VSIZE>>3)}, /* Image Vertiacl Size VSIZE[10:3] */

        /* {HSIZE[11], HSIZE[2:0], VSIZE[2:0]} */
        { SIZEL,   ((SVGA_HSIZE>>6)&0x40) | ((SVGA_HSIZE&0x7)<<3) | (SVGA_VSIZE&0x7)},

        { XOFFL,   0x00 }, /* OFFSET_X[7:0] */
        { YOFFL,   0x00 }, /* OFFSET_Y[7:0] */
        { HSIZE,   ((SVGA_HSIZE>>2)&0xFF) }, /* H_SIZE[7:0]= HSIZE/4 */
        { VSIZE,   ((SVGA_VSIZE>>2)&0xFF) }, /* V_SIZE[7:0]= VSIZE/4 */

        /* V_SIZE[8]/OFFSET_Y[10:8]/H_SIZE[8]/OFFSET_X[10:8] */
        { VHYX,    ((SVGA_VSIZE>>3)&0x80) | ((SVGA_HSIZE>>7)&0x08) },
        { TEST,    (SVGA_HSIZE>>4)&0x80}, /* H_SIZE[9] */

        { CTRL2,   CTRL2_DCW_EN | CTRL2_SDE_EN |
          CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN },

        /* H_DIVIDER/V_DIVIDER */
        { CTRLI,   CTRLI_LP_DP | 0x00},
        /* DVP prescalar */
        { R_DVP_SP, R_DVP_SP_AUTO_MODE},

        { R_BYPASS, R_BYPASS_DSP_EN },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t uxga_regs[][2] = {
        { BANK_SEL, BANK_SEL_SENSOR },
        /* DSP input image resoultion and window size control */
        { COM7,    COM7_RES_UXGA},
        { COM1,    0x0F }, /* UXGA=0x0F, SVGA=0x0A, CIF=0x06 */
        { REG32,   0x36 }, /* UXGA=0x36, SVGA/CIF=0x09 */

        { HSTART,  0x11 }, /* UXGA=0x11, SVGA/CIF=0x11 */
        { HSTOP,   0x75 }, /* UXGA=0x75, SVGA/CIF=0x43 */

        { VSTART,  0x01 }, /* UXGA=0x01, SVGA/CIF=0x00 */
        { VSTOP,   0x97 }, /* UXGA=0x97, SVGA/CIF=0x4b */
        { 0x3d,    0x34 }, /* UXGA=0x34, SVGA/CIF=0x38 */

        { 0x35,    0x88 },
        { 0x22,    0x0a },
        { 0x37,    0x40 },
        { 0x34,    0xa0 },
        { 0x06,    0x02 },
        { 0x0d,    0xb7 },
        { 0x0e,    0x01 },
        { 0x42,    0x83 },

        /* Set DSP input image size and offset.
           The sensor output image can be scaled with OUTW/OUTH */
        { BANK_SEL, BANK_SEL_DSP },
        { R_BYPASS, R_BYPASS_DSP_BYPAS },

        { RESET,   RESET_DVP },
        { HSIZE8,  (UXGA_HSIZE>>3)}, /* Image Horizontal Size HSIZE[10:3] */
        { VSIZE8,  (UXGA_VSIZE>>3)}, /* Image Vertiacl Size VSIZE[10:3] */

        /* {HSIZE[11], HSIZE[2:0], VSIZE[2:0]} */
        { SIZEL,   ((UXGA_HSIZE>>6)&0x40) | ((UXGA_HSIZE&0x7)<<3) | (UXGA_VSIZE&0x7)},

        { XOFFL,   0x00 }, /* OFFSET_X[7:0] */
        { YOFFL,   0x00 }, /* OFFSET_Y[7:0] */
        { HSIZE,   ((UXGA_HSIZE>>2)&0xFF) }, /* H_SIZE[7:0] real/4 */
        { VSIZE,   ((UXGA_VSIZE>>2)&0xFF) }, /* V_SIZE[7:0] real/4 */

        /* V_SIZE[8]/OFFSET_Y[10:8]/H_SIZE[8]/OFFSET_X[10:8] */
        { VHYX,    ((UXGA_VSIZE>>3)&0x80) | ((UXGA_HSIZE>>7)&0x08) },
        { TEST,    (UXGA_HSIZE>>4)&0x80}, /* H_SIZE[9] */

        { CTRL2,   CTRL2_DCW_EN | CTRL2_SDE_EN |
            CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN },

        /* H_DIVIDER/V_DIVIDER */
        { CTRLI,   CTRLI_LP_DP | 0x00},
        /* DVP prescalar */
        { R_DVP_SP, R_DVP_SP_AUTO_MODE | 0x04},

        { R_BYPASS, R_BYPASS_DSP_EN },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t yuv422_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { IMAGE_MODE, IMAGE_MODE_YUV422 },
        { 0xD7,     0x01 },
        { 0xE1,     0x67 },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t rgb565_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { IMAGE_MODE, IMAGE_MODE_RGB565 },
        { 0xD7,     0x03 },
        { 0xE1,     0x77 },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t jpeg_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { IMAGE_MODE, IMAGE_MODE_JPEG_EN|IMAGE_MODE_RGB565 },
        { 0xD7,     0x03 },
        { 0xE1,     0x77 },
        { QS,       0x0C },
        { RESET,    0x00 },
        {0, 0},
};

#define NUM_BRIGHTNESS_LEVELS (5)
static const uint8_t brightness_regs[NUM_BRIGHTNESS_LEVELS + 1][5] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA },
    { 0x00, 0x04, 0x09, 0x00, 0x00 }, /* -2 */
    { 0x00, 0x04, 0x09, 0x10, 0x00 }, /* -1 */
    { 0x00, 0x04, 0x09, 0x20, 0x00 }, /*  0 */
    { 0x00, 0x04, 0x09, 0x30, 0x00 }, /* +1 */
    { 0x00, 0x04, 0x09, 0x40, 0x00 }, /* +2 */
};

#define NUM_CONTRAST_LEVELS (5)
static const uint8_t contrast_regs[NUM_CONTRAST_LEVELS + 1][7] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA, BPDATA, BPDATA },
    { 0x00, 0x04, 0x07, 0x20, 0x18, 0x34, 0x06 }, /* -2 */
    { 0x00, 0x04, 0x07, 0x20, 0x1c, 0x2a, 0x06 }, /* -1 */
    { 0x00, 0x04, 0x07, 0x20, 0x20, 0x20, 0x06 }, /*  0 */
    { 0x00, 0x04, 0x07, 0x20, 0x24, 0x16, 0x06 }, /* +1 */
    { 0x00, 0x04, 0x07, 0x20, 0x28, 0x0c, 0x06 }, /* +2 */
};

#define NUM_SATURATION_LEVELS (5)
static const uint8_t saturation_regs[NUM_SATURATION_LEVELS + 1][5] = {
    { BPADDR, BPDATA, BPADDR, BPDATA, BPDATA },
    { 0x00, 0x02, 0x03, 0x28, 0x28 }, /* -2 */
    { 0x00, 0x02, 0x03, 0x38, 0x38 }, /* -1 */
    { 0x00, 0x02, 0x03, 0x48, 0x48 }, /*  0 */
    { 0x00, 0x02, 0x03, 0x58, 0x58 }, /* +1 */
    { 0x00, 0x02, 0x03, 0x58, 0x58 }, /* +2 */
};

static int reset(sensor_t *sensor)
{
    int i=0;
    const uint8_t (*regs)[2];

    /* Reset all registers */
    cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    cambus_writeb(sensor->slv_addr, COM7, COM7_SRST);

    /* delay n ms */
    systick_sleep(10);

    i = 0;
    regs = default_regs;
    /* Write initial regsiters */
    while (regs[i][0]) {
        cambus_writeb(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    i = 0;
    regs = svga_regs;
    /* Write DSP input regsiters */
    while (regs[i][0]) {
        cambus_writeb(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int i=0;
    const uint8_t (*regs)[2]=NULL;

    /* read pixel format reg */
    switch (pixformat) {
        case PIXFORMAT_RGB565:
            regs = rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            regs = yuv422_regs;
            break;
        case PIXFORMAT_JPEG:
            regs = jpeg_regs;
            break;
        default:
            return -1;
    }

    /* Write initial regsiters */
    while (regs[i][0]) {
        cambus_writeb(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    /* delay n ms */
    systick_sleep(30);

    return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    uint8_t clkrc;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    int i=0;
    const uint8_t (*regs)[2];

    if ((w <= 800) && (h <= 600)) {
        clkrc =0x80;
        regs = svga_regs;
    } else {
        clkrc =0x81;
        regs = uxga_regs;
    }

    /* Disable DSP */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= cambus_writeb(sensor->slv_addr, R_BYPASS, R_BYPASS_DSP_BYPAS);

    /* Write output width */
    ret |= cambus_writeb(sensor->slv_addr, ZMOW, (w>>2)&0xFF); /* OUTW[7:0] (real/4) */
    ret |= cambus_writeb(sensor->slv_addr, ZMOH, (h>>2)&0xFF); /* OUTH[7:0] (real/4) */
    ret |= cambus_writeb(sensor->slv_addr, ZMHH, ((h>>8)&0x04)|((w>>10)&0x03)); /* OUTH[8]/OUTW[9:8] */

    /* Set CLKRC */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= cambus_writeb(sensor->slv_addr, CLKRC, clkrc);

    /* Write DSP input regsiters */
    while (regs[i][0]) {
        cambus_writeb(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    /* Enable DSP */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= cambus_writeb(sensor->slv_addr, R_BYPASS, R_BYPASS_DSP_EN);

    /* delay n ms */
    systick_sleep(30);

    return ret;
}

static int set_framerate(sensor_t *sensor, framerate_t framerate)
{
    return 0;
}

static int set_contrast(sensor_t *sensor, int level)
{
    int ret=0;

    level += (NUM_CONTRAST_LEVELS / 2 + 1);
    if (level < 0 || level > NUM_CONTRAST_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write contrast registers */
    for (int i=0; i<sizeof(contrast_regs[0])/sizeof(contrast_regs[0][0]); i++) {
        ret |= cambus_writeb(sensor->slv_addr, contrast_regs[0][i], contrast_regs[level][i]);
    }

    return ret;
}

static int set_brightness(sensor_t *sensor, int level)
{
    int ret=0;

    level += (NUM_BRIGHTNESS_LEVELS / 2 + 1);
    if (level < 0 || level > NUM_BRIGHTNESS_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write brightness registers */
    for (int i=0; i<sizeof(brightness_regs[0])/sizeof(brightness_regs[0][0]); i++) {
        ret |= cambus_writeb(sensor->slv_addr, brightness_regs[0][i], brightness_regs[level][i]);
    }

    return ret;
}

static int set_saturation(sensor_t *sensor, int level)
{
    int ret=0;

    level += (NUM_SATURATION_LEVELS / 2 + 1);
    if (level < 0 || level > NUM_SATURATION_LEVELS) {
        return -1;
    }

    /* Switch to DSP register bank */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write contrast registers */
    for (int i=0; i<sizeof(saturation_regs[0])/sizeof(saturation_regs[0][0]); i++) {
        ret |= cambus_writeb(sensor->slv_addr, saturation_regs[0][i], saturation_regs[level][i]);
    }

    return ret;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    int ret =0;

    /* Switch to SENSOR register bank */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Write gain ceiling register */
    ret |= cambus_writeb(sensor->slv_addr, COM9, COM9_AGC_SET(gainceiling));

    return ret;
}

static int set_quality(sensor_t *sensor, int qs)
{
    int ret=0;

    /* Switch to DSP register bank */
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write QS register */
    ret |= cambus_writeb(sensor->slv_addr, QS, qs);

    return ret;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    uint8_t reg;
    /* Switch to SENSOR register bank */
    int ret = cambus_writeb(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Update COM7 */
    ret |= cambus_readb(sensor->slv_addr, COM7, &reg);

    if (enable) {
        reg |= COM7_COLOR_BAR;
    } else {
        reg &= ~COM7_COLOR_BAR;
    }

    return cambus_writeb(sensor->slv_addr, COM7, reg) | ret;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
    uint8_t reg;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);
    ret |= cambus_readb(sensor->slv_addr, COM8, &reg);
    ret |= cambus_writeb(sensor->slv_addr, COM8, (reg & (~COM8_AGC_EN)) | ((enable != 0) ? COM8_AGC_EN : 0));

    if ((enable == 0) && (!isnanf(gain_db))) {
        float gain = IM_MAX(IM_MIN(fast_expf((gain_db / 20.0) * fast_log(10.0)), 32.0), 0.0);

        int gain_hi = 15;
        for (; gain_hi >= 0; gain_hi--) {
            if ((gain / (gain_hi + 1.0)) >= 1.0) break;
        }

        int gain_lo = 15;
        for (; gain_lo >= 0; gain_lo--) {
            if (((gain / (gain_hi + 1.0)) / (1.0 + (gain_lo / 16.0))) >= 1.0) break;
        }

        ret |= cambus_writeb(sensor->slv_addr, GAIN, (gain_hi << 4) | (gain_lo << 0));
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling))) {
        float gain_ceiling = IM_MAX(IM_MIN(fast_expf((gain_db_ceiling / 20.0) * fast_log(10.0)), 128.0), 2.0);

        ret |= cambus_readb(sensor->slv_addr, COM9, &reg);
        ret |= cambus_writeb(sensor->slv_addr, COM9, (reg & 0x1F) | ((fast_ceilf(fast_log2(gain_ceiling)) - 1) << 5));
    }

    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    uint8_t reg, gain;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);
    ret |= cambus_readb(sensor->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= cambus_writeb(sensor->slv_addr, COM8, reg & (~COM8_AGC_EN));
    // }
    // DISABLED

    ret |= cambus_readb(sensor->slv_addr, GAIN, &gain);

    // DISABLED
    // if (reg & COM8_AGC_EN) {
    //     ret |= cambus_writeb(sensor->slv_addr, COM8, reg | COM8_AGC_EN);
    // }
    // DISABLED

    *gain_db = 20.0 * (fast_log((((gain >> 4) & 0xF) + 1.0) * (1.0 + (((gain >> 0) & 0xF) / 16.0))) / fast_log(10.0));

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    uint8_t reg;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);
    ret |= cambus_readb(sensor->slv_addr, COM8, &reg);
    ret |= cambus_writeb(sensor->slv_addr, COM8, COM8_SET_AEC(reg, (enable != 0)));

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= cambus_readb(sensor->slv_addr, COM7, &reg);
        int t_line = 0;

        if (COM7_GET_RES(reg) == COM7_RES_UXGA) t_line = 1600 + 322;
        if (COM7_GET_RES(reg) == COM7_RES_SVGA) t_line = 800 + 390;
        if (COM7_GET_RES(reg) == COM7_RES_CIF) t_line = 400 + 195;

        ret |= cambus_readb(sensor->slv_addr, CLKRC, &reg);
        int pll_mult = (reg & CLKRC_DOUBLE) ? 2 : 1;
        int clk_rc = ((reg & CLKRC_DIVIDER_MASK) + 1) * 2;

        ret |= cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
        ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
        ret |= cambus_readb(sensor->slv_addr, IMAGE_MODE, &reg);
        int t_pclk = 0;

        if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_YUV422) t_pclk = 2;
        if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RAW10) t_pclk = 1;
        if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RGB565) t_pclk = 2;

        int exposure = IM_MAX(IM_MIN(((exposure_us*(((OMV_XCLK_FREQUENCY/clk_rc)*pll_mult)/1000000))/t_pclk)/t_line,0xFFFF),0x0000);

        ret |= cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
        ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);

        ret |= cambus_readb(sensor->slv_addr, REG04, &reg);
        ret |= cambus_writeb(sensor->slv_addr, REG04, (reg & 0xFC) | ((exposure >> 0) & 0x3));

        ret |= cambus_readb(sensor->slv_addr, AEC, &reg);
        ret |= cambus_writeb(sensor->slv_addr, AEC, (reg & 0x00) | ((exposure >> 2) & 0xFF));

        ret |= cambus_readb(sensor->slv_addr, REG04, &reg);
        ret |= cambus_writeb(sensor->slv_addr, REG04, (reg & 0xC0) | ((exposure >> 10) & 0x3F));
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    uint8_t reg, aec_10, aec_92, aec_1510;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);
    ret |= cambus_readb(sensor->slv_addr, COM8, &reg);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= cambus_writeb(sensor->slv_addr, COM8, reg & (~COM8_AEC_EN));
    // }
    // DISABLED

    ret |= cambus_readb(sensor->slv_addr, REG04, &aec_10);
    ret |= cambus_readb(sensor->slv_addr, AEC, &aec_92);
    ret |= cambus_readb(sensor->slv_addr, REG45, &aec_1510);

    // DISABLED
    // if (reg & COM8_AEC_EN) {
    //     ret |= cambus_writeb(sensor->slv_addr, COM8, reg | COM8_AEC_EN);
    // }
    // DISABLED

    ret |= cambus_readb(sensor->slv_addr, COM7, &reg);
    int t_line = 0;

    if (COM7_GET_RES(reg) == COM7_RES_UXGA) t_line = 1600 + 322;
    if (COM7_GET_RES(reg) == COM7_RES_SVGA) t_line = 800 + 390;
    if (COM7_GET_RES(reg) == COM7_RES_CIF) t_line = 400 + 195;

    ret |= cambus_readb(sensor->slv_addr, CLKRC, &reg);
    int pll_mult = (reg & CLKRC_DOUBLE) ? 2 : 1;
    int clk_rc = ((reg & CLKRC_DIVIDER_MASK) + 1) * 2;

    ret |= cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
    ret |= cambus_readb(sensor->slv_addr, IMAGE_MODE, &reg);
    int t_pclk = 0;

    if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_YUV422) t_pclk = 2;
    if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RAW10) t_pclk = 1;
    if (IMAGE_MODE_GET_FMT(reg) == IMAGE_MODE_RGB565) t_pclk = 2;

    uint16_t exposure = ((aec_1510 & 0x3F) << 10) + ((aec_92 & 0xFF) << 2) + ((aec_10 & 0x3) << 0);
    *exposure_us = (exposure*t_line*t_pclk)/(((OMV_XCLK_FREQUENCY/clk_rc)*pll_mult)/1000000);

    return ret;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    uint8_t reg;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
    ret |= cambus_readb(sensor->slv_addr, CTRL1, &reg);
    ret |= cambus_writeb(sensor->slv_addr, CTRL1, (reg & (~CTRL1_AWB)) | ((enable != 0) ? CTRL1_AWB : 0));

    if ((enable == 0) && (!isnanf(r_gain_db)) && (!isnanf(g_gain_db)) && (!isnanf(b_gain_db))) {
    }

    return ret;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    uint8_t reg;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg & (~BANK_SEL_SENSOR));
    ret |= cambus_readb(sensor->slv_addr, CTRL1, &reg);

    // DISABLED
    // if (reg & CTRL1_AWB) {
    //     ret |= cambus_writeb(sensor->slv_addr, CTRL1, reg & (~CTRL1_AWB));
    // }
    // DISABLED

    // DISABLED
    // if (reg & CTRL1_AWB) {
    //     ret |= cambus_writeb(sensor->slv_addr, CTRL1, reg | CTRL1_AWB);
    // }
    // DISABLED

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);
    ret |= cambus_readb(sensor->slv_addr, REG04, &reg);

    if (enable) {
        reg |= REG04_HFLIP_IMG;
    } else {
        reg &= ~REG04_HFLIP_IMG;
    }

    ret |= cambus_writeb(sensor->slv_addr, REG04, reg);

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb(sensor->slv_addr, BANK_SEL, &reg);
    ret |= cambus_writeb(sensor->slv_addr, BANK_SEL, reg | BANK_SEL_SENSOR);
    ret |= cambus_readb(sensor->slv_addr, REG04, &reg);

    if (enable) {
        reg |= REG04_VFLIP_IMG;
    } else {
        reg &= ~REG04_VFLIP_IMG;
    }

    ret |= cambus_writeb(sensor->slv_addr, REG04, reg);

    return ret;
}

int ov2640_init(sensor_t *sensor)
{
    // Initialize sensor structure.
    sensor->gs_bpp              = 2;
    sensor->reset               = reset;
    sensor->set_pixformat       = set_pixformat;
    sensor->set_framesize       = set_framesize;
    sensor->set_framerate       = set_framerate;
    sensor->set_contrast        = set_contrast;
    sensor->set_brightness      = set_brightness;
    sensor->set_saturation      = set_saturation;
    sensor->set_gainceiling     = set_gainceiling;
    sensor->set_quality         = set_quality;
    sensor->set_colorbar        = set_colorbar;
    sensor->set_auto_gain       = set_auto_gain;
    sensor->get_gain_db         = get_gain_db;
    sensor->set_auto_exposure   = set_auto_exposure;
    sensor->get_exposure_us     = get_exposure_us;
    sensor->set_auto_whitebal   = set_auto_whitebal;
    sensor->get_rgb_gain_db     = get_rgb_gain_db;
    sensor->set_hmirror         = set_hmirror;
    sensor->set_vflip           = set_vflip;

    // Set sensor flags
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 1);

    return 0;
}
