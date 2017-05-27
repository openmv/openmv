/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV7725 driver.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include STM32_HAL_H
#include "sccb.h"
#include "ov7725.h"
#include "systick.h"
#include "ov7725_regs.h"

static const uint8_t default_regs[][2] = {
    {COM3,          COM3_SWAP_YUV},
    {COM7,          COM7_RES_VGA | COM7_FMT_RGB565 | COM7_FMT_RGB},

    {COM4,          0x81}, /* PLL x6 */
    {CLKRC,         0xC0}, /* Res/Bypass pre-scalar */

    // VGA Window Size
    {HSTART,        0x23},
    {HSIZE,         0xA0},
    {VSTART,        0x07},
    {VSIZE,         0xF0},
    {HREF,          0x00},

    // Scale down to QVGA Resoultion
    {HOUTSIZE,      0x50},
    {VOUTSIZE,      0x78},

    {COM12,         0x03},
    {EXHCH,         0x00},
    {TGT_B,         0x7F},
    {FIXGAIN,       0x09},
    {AWB_CTRL0,     0xE0},
    {DSP_CTRL1,     0xFF},

    {DSP_CTRL2,     DSP_CTRL2_VDCW_EN | DSP_CTRL2_HDCW_EN | DSP_CTRL2_HZOOM_EN | DSP_CTRL2_VZOOM_EN},

    {DSP_CTRL3,     0x00},
    {DSP_CTRL4,     0x00},
    {DSPAUTO,       0xFF},

    {COM8,          0xF0},
    {COM6,          0xC5},
    {COM9,          0x21},
    {BDBASE,        0x7F},
    {BDSTEP,        0x03},
    {AEW,           0x96},
    {AEB,           0x64},
    {VPT,           0xA1},
    {EXHCL,         0x00},
    {AWB_CTRL3,     0xAA},
    {COM8,          0xFF},

    //Gamma
    {GAM1,          0x0C},
    {GAM2,          0x16},
    {GAM3,          0x2A},
    {GAM4,          0x4E},
    {GAM5,          0x61},
    {GAM6,          0x6F},
    {GAM7,          0x7B},
    {GAM8,          0x86},
    {GAM9,          0x8E},
    {GAM10,         0x97},
    {GAM11,         0xA4},
    {GAM12,         0xAF},
    {GAM13,         0xC5},
    {GAM14,         0xD7},
    {GAM15,         0xE8},

    {SLOP,          0x20},
    {EDGE1,         0x05},
    {EDGE2,         0x03},
    {EDGE3,         0x00},
    {DNSOFF,        0x01},

    {MTX1,          0xB0},
    {MTX2,          0x9D},
    {MTX3,          0x13},
    {MTX4,          0x16},
    {MTX5,          0x7B},
    {MTX6,          0x91},
    {MTX_CTRL,      0x1E},

    {BRIGHTNESS,    0x08},
    {CONTRAST,      0x20},
    {UVADJ0,        0x81},
    {SDE,           (SDE_CONT_BRIGHT_EN | SDE_SATURATION_EN)},

    // For 30 fps/60Hz
    {DM_LNL,        0x00},
    {DM_LNH,        0x00},
    {BDBASE,        0x7F},
    {BDSTEP,        0x03},

    // Lens Correction, should be tuned with real camera module
    {LC_CTR,        0x01}, // Enable LC and use 1 coefficient for all 3 channels
    {LC_RADI,       0x30}, // The radius of the circle where no compensation applies
    {LC_COEF,       0x30}, // RGB Lens correction coefficient

    // Frame reduction in night mode.
    {COM5,          0xD5},

    {0x00,          0x00},
};

#define NUM_BRIGHTNESS_LEVELS (9)
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

#define NUM_CONTRAST_LEVELS (9)
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

#define NUM_SATURATION_LEVELS (9)
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

static int reset(sensor_t *sensor)
{
    int i=0;
    const uint8_t (*regs)[2];

    // Reset all registers
    SCCB_Write(sensor->slv_addr, COM7, COM7_RESET);

    // Delay 10 ms
    systick_sleep(10);

    // Write default regsiters
    for (i=0, regs = default_regs; regs[i][0]; i++) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
    }

    // Delay
    systick_sleep(30);

    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret=0;
    // Read register COM7
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM7);

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            reg = COM7_SET_FMT(reg, COM7_FMT_RGB);
            ret = SCCB_Write(sensor->slv_addr, DSP_CTRL4, 0);
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            reg = COM7_SET_FMT(reg, COM7_FMT_YUV);
            ret = SCCB_Write(sensor->slv_addr, DSP_CTRL4, 0);
            break;
        case PIXFORMAT_BAYER:
            reg = COM7_SET_FMT(reg, COM7_FMT_P_BAYER);
            ret = SCCB_Write(sensor->slv_addr, DSP_CTRL4, DSP_CTRL4_RAW8);
            break;

        default:
            return -1;
    }

    // Write back register COM7
    ret |= SCCB_Write(sensor->slv_addr, COM7, reg);

    // Delay
    systick_sleep(30);

    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    // Write MSBs
    ret |= SCCB_Write(sensor->slv_addr, HOUTSIZE, w>>2);
    ret |= SCCB_Write(sensor->slv_addr, VOUTSIZE, h>>1);

    // Write LSBs
    ret |= SCCB_Write(sensor->slv_addr, EXHCH, ((w&0x3) | ((h&0x1) << 2)));

    if ((w <= 320) && (h <= 240)) {
        // Set QVGA Resolution
        uint8_t reg = SCCB_Read(sensor->slv_addr, COM7);
        reg = COM7_SET_RES(reg, COM7_RES_QVGA);
        ret |= SCCB_Write(sensor->slv_addr, COM7, reg);

        // Set QVGA Window Size
        ret |= SCCB_Write(sensor->slv_addr, HSTART, 0x3F);
        ret |= SCCB_Write(sensor->slv_addr, HSIZE,  0x50);
        ret |= SCCB_Write(sensor->slv_addr, VSTART, 0x03);
        ret |= SCCB_Write(sensor->slv_addr, VSIZE,  0x78);
        ret |= SCCB_Write(sensor->slv_addr, HREF,   0x00);

        // Enable auto-scaling/zooming factors
        ret |= SCCB_Write(sensor->slv_addr, DSPAUTO, 0xFF);
    } else {
        // Set VGA Resolution
        uint8_t reg = SCCB_Read(sensor->slv_addr, COM7);
        reg = COM7_SET_RES(reg, COM7_RES_VGA);
        ret |= SCCB_Write(sensor->slv_addr, COM7, reg);

        // Set VGA Window Size
        ret |= SCCB_Write(sensor->slv_addr, HSTART, 0x23);
        ret |= SCCB_Write(sensor->slv_addr, HSIZE,  0xA0);
        ret |= SCCB_Write(sensor->slv_addr, VSTART, 0x07);
        ret |= SCCB_Write(sensor->slv_addr, VSIZE,  0xF0);
        ret |= SCCB_Write(sensor->slv_addr, HREF,   0x00);

        // Disable auto-scaling/zooming factors
        ret |= SCCB_Write(sensor->slv_addr, DSPAUTO, 0xF3);

        // Clear auto-scaling/zooming factors
        ret |= SCCB_Write(sensor->slv_addr, SCAL0, 0x00);
        ret |= SCCB_Write(sensor->slv_addr, SCAL1, 0x00);
        ret |= SCCB_Write(sensor->slv_addr, SCAL2, 0x00);
    }

    // Delay
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

    level += (NUM_CONTRAST_LEVELS / 2);
    if (level < 0 || level >= NUM_CONTRAST_LEVELS) {
        return -1;
    }

    ret |= SCCB_Write(sensor->slv_addr, CONTRAST, contrast_regs[level][0]);
    return ret;
}

static int set_brightness(sensor_t *sensor, int level)
{
    int ret=0;

    level += (NUM_BRIGHTNESS_LEVELS / 2);
    if (level < 0 || level >= NUM_BRIGHTNESS_LEVELS) {
        return -1;
    }

    ret |= SCCB_Write(sensor->slv_addr, BRIGHTNESS, brightness_regs[level][0]);
    ret |= SCCB_Write(sensor->slv_addr, SIGN_BIT,   brightness_regs[level][1]);
    return ret;
}

static int set_saturation(sensor_t *sensor, int level)
{
    int ret=0;

    level += (NUM_SATURATION_LEVELS / 2 );
    if (level < 0 || level >= NUM_SATURATION_LEVELS) {
        return -1;
    }

    ret |= SCCB_Write(sensor->slv_addr, USAT, saturation_regs[level][0]);
    ret |= SCCB_Write(sensor->slv_addr, VSAT, saturation_regs[level][1]);
    return ret;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    // Read register COM9
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM9);

    // Set gain ceiling
    reg = COM9_SET_AGC(reg, gainceiling);

    // Write back register COM9
    return SCCB_Write(sensor->slv_addr, COM9, reg);
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    int ret=0;
    uint8_t reg;

    // Read reg COM3
    reg = SCCB_Read(sensor->slv_addr, COM3);
    // Enable colorbar test pattern output
    reg = COM3_SET_CBAR(reg, enable);
    // Write back COM3
    ret |= SCCB_Write(sensor->slv_addr, COM3, reg);

    // Read reg DSP_CTRL3
    reg = SCCB_Read(sensor->slv_addr, DSP_CTRL3);
    // Enable DSP colorbar output
    reg = DSP_CTRL3_SET_CBAR(reg, enable);
    // Write back DSP_CTRL3
    ret |= SCCB_Write(sensor->slv_addr, DSP_CTRL3, reg);

    return ret;
}

static int set_auto_gain(sensor_t *sensor, int enable, int gain)
{
    int ret=0;
    // Read register COM8
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM8);

    // Set AGC on/off
    reg = COM8_SET_AGC(reg, enable);

    // Write back register COM8
    ret |= SCCB_Write(sensor->slv_addr, COM8, reg);

    if (enable == 0 && gain >= 0) {
        // Set value manually.
        ret |= SCCB_Write(sensor->slv_addr, GAIN, gain);
    }

    return ret;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure)
{
    int ret=0;
    // Read register COM8
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM8);

    // Set AEC on/off
    reg = COM8_SET_AEC(reg, enable);

    // Write back register COM8
    ret |= SCCB_Write(sensor->slv_addr, COM8, reg);

    if (enable == 0 && exposure >= 0) {
        // Set value manually.
        ret |= SCCB_Write(sensor->slv_addr, AEC, (exposure&0xFF));
        ret |= SCCB_Write(sensor->slv_addr, AECH, ((exposure>>8)&0xFF));
    }

    return ret;

}

static int set_auto_whitebal(sensor_t *sensor, int enable, int r_gain, int g_gain, int b_gain)
{
    int ret=0;

    // Read register COM8
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM8);

    // Set AWB on/off
    reg = COM8_SET_AWB(reg, enable);

    // Write back register COM8
    ret |= SCCB_Write(sensor->slv_addr, COM8, reg);

    if (enable == 0 && r_gain >= 0 && g_gain >=0 && b_gain >=0) {
        // Set value manually.
        ret |= SCCB_Write(sensor->slv_addr, RED, r_gain);
        ret |= SCCB_Write(sensor->slv_addr, GREEN, g_gain);
        ret |= SCCB_Write(sensor->slv_addr, BLUE, b_gain);
    }

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    // Read register COM3
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM3);

    // Set mirror on/off
    reg = COM3_SET_MIRROR(reg, enable);

    // Write back register COM3
    return SCCB_Write(sensor->slv_addr, COM3, reg);
}

static int set_vflip(sensor_t *sensor, int enable)
{
    // Read register COM3
    uint8_t reg = SCCB_Read(sensor->slv_addr, COM3);

    // Set mirror on/off
    reg = COM3_SET_FLIP(reg, enable);

    // Write back register COM3
    return SCCB_Write(sensor->slv_addr, COM3, reg);
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
{
    int ret=0;

    switch (sde) {
        case SDE_NEGATIVE:
            ret |= SCCB_Write(sensor->slv_addr, SDE, 0x46);
            break;
        case SDE_NORMAL:
            ret |= SCCB_Write(sensor->slv_addr, SDE, 0x06);
            ret |= SCCB_Write(sensor->slv_addr, UFIX, 0x80);
            ret |= SCCB_Write(sensor->slv_addr, UFIX, 0x80);
            break;
        default:
            return -1;
    }

    return ret;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef)
{
    int ret=0;

    ret |= SCCB_Write(sensor->slv_addr, LC_CTR, (enable&0x01));
    ret |= SCCB_Write(sensor->slv_addr, LC_RADI, radi);
    ret |= SCCB_Write(sensor->slv_addr, LC_COEF, coef);
    return ret;
}


int ov7725_init(sensor_t *sensor)
{
    // Set function pointers
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_framerate = set_framerate;
    sensor->set_contrast  = set_contrast;
    sensor->set_brightness= set_brightness;
    sensor->set_saturation= set_saturation;
    sensor->set_gainceiling = set_gainceiling;
    sensor->set_colorbar = set_colorbar;
    sensor->set_auto_gain = set_auto_gain;
    sensor->set_auto_exposure = set_auto_exposure;
    sensor->set_auto_whitebal = set_auto_whitebal;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;
    sensor->set_special_effect = set_special_effect;
    sensor->set_lens_correction = set_lens_correction;

    // Set sensor flags
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
