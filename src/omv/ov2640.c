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
#include "stm32f4xx_hal.h"
#include "sccb.h"
#include "ov2640.h"
#include "systick.h"
#include "ov2640_regs.h"

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

static const uint8_t cif_regs[][2] = {
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
    SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    SCCB_Write(sensor->slv_addr, COM7, COM7_SRST);

    /* delay n ms */
    systick_sleep(10);

    i = 0;
    regs = default_regs;
    /* Write initial regsiters */
    while (regs[i][0]) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    i = 0;
    regs = svga_regs;
    /* Write DSP input regsiters */
    while (regs[i][0]) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
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
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
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

    if (framesize <= FRAMESIZE_SVGA) {
        clkrc =0x80;
        regs = svga_regs;
    } else {
        clkrc =0x81;
        regs = uxga_regs;
    }

    /* Disable DSP */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= SCCB_Write(sensor->slv_addr, R_BYPASS, R_BYPASS_DSP_BYPAS);

    /* Write output width */
    ret |= SCCB_Write(sensor->slv_addr, ZMOW, (w>>2)&0xFF); /* OUTW[7:0] (real/4) */
    ret |= SCCB_Write(sensor->slv_addr, ZMOH, (h>>2)&0xFF); /* OUTH[7:0] (real/4) */
    ret |= SCCB_Write(sensor->slv_addr, ZMHH, ((h>>8)&0x04)|((w>>10)&0x03)); /* OUTH[8]/OUTW[9:8] */

    /* Set CLKRC */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= SCCB_Write(sensor->slv_addr, CLKRC, clkrc);

    /* Write DSP input regsiters */
    while (regs[i][0]) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    /* Enable DSP */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);
    ret |= SCCB_Write(sensor->slv_addr, R_BYPASS, R_BYPASS_DSP_EN);

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
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write contrast registers */
    for (int i=0; i<sizeof(contrast_regs[0])/sizeof(contrast_regs[0][0]); i++) {
        ret |= SCCB_Write(sensor->slv_addr, contrast_regs[0][i], contrast_regs[level][i]);
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
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write brightness registers */
    for (int i=0; i<sizeof(brightness_regs[0])/sizeof(brightness_regs[0][0]); i++) {
        ret |= SCCB_Write(sensor->slv_addr, brightness_regs[0][i], brightness_regs[level][i]);
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
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write contrast registers */
    for (int i=0; i<sizeof(saturation_regs[0])/sizeof(saturation_regs[0][0]); i++) {
        ret |= SCCB_Write(sensor->slv_addr, saturation_regs[0][i], saturation_regs[level][i]);
    }

    return ret;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    int ret =0;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Write gain ceiling register */
    ret |= SCCB_Write(sensor->slv_addr, COM9, COM9_AGC_SET(gainceiling));

    return ret;
}

static int set_quality(sensor_t *sensor, int qs)
{
    int ret=0;

    /* Switch to DSP register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Write QS register */
    ret |= SCCB_Write(sensor->slv_addr, QS, qs);

    return ret;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Update COM7 */
    reg = SCCB_Read(sensor->slv_addr, COM7);

    if (enable) {
        reg |= COM7_COLOR_BAR;
    } else {
        reg &= ~COM7_COLOR_BAR;
    }

    ret |= SCCB_Write(sensor->slv_addr, COM7, reg);
    return ret;
}

static int set_whitebal(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_DSP);

    /* Update CTRL1 */
    reg = SCCB_Read(sensor->slv_addr, CTRL1);

    if (enable) {
        reg |= CTRL1_AWB;
    } else {
        reg &= ~CTRL1_AWB;
    }

    ret |= SCCB_Write(sensor->slv_addr, CTRL1, reg);
    return ret;
}

static int set_gain_ctrl(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Update COM8 */
    reg = SCCB_Read(sensor->slv_addr, COM8);

    if (enable) {
        reg |= COM8_AGC_EN;
    } else {
        reg &= ~COM8_AGC_EN;
    }

    ret |= SCCB_Write(sensor->slv_addr, COM8, reg);
    return ret;
}

static int set_exposure_ctrl(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Update COM8 */
    reg = SCCB_Read(sensor->slv_addr, COM8);

    if (enable) {
        reg |= COM8_AEC_EN;
    } else {
        reg &= ~COM8_AEC_EN;
    }

    ret |= SCCB_Write(sensor->slv_addr, COM8, reg);
    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Update REG04 */
    reg = SCCB_Read(sensor->slv_addr, REG04);

    if (enable) {
        reg |= REG04_HFLIP_IMG;
    } else {
        reg &= ~REG04_HFLIP_IMG;
    }

    ret |= SCCB_Write(sensor->slv_addr, REG04, reg);
    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    /* Switch to SENSOR register bank */
    ret |= SCCB_Write(sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);

    /* Update REG04 */
    reg = SCCB_Read(sensor->slv_addr, REG04);

    if (enable) {
        reg |= REG04_VFLIP_IMG;
    } else {
        reg &= ~REG04_VFLIP_IMG;
    }

    ret |= SCCB_Write(sensor->slv_addr, REG04, reg);
    return ret;
}

int ov2640_init(sensor_t *sensor)
{
    /* set function pointers */
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_framerate = set_framerate;
    sensor->set_contrast  = set_contrast;
    sensor->set_brightness= set_brightness;
    sensor->set_saturation= set_saturation;
    sensor->set_gainceiling = set_gainceiling;
    sensor->set_quality = set_quality;
    sensor->set_colorbar = set_colorbar;
    sensor->set_gain_ctrl = set_gain_ctrl;
    sensor->set_exposure_ctrl = set_exposure_ctrl;
    sensor->set_whitebal = set_whitebal;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;

    // Set sensor flags
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HW_JPEG, 1);

    return 0;
}
