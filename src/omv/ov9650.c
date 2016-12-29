/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV9650 driver.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include STM32_HAL_H
#include "sccb.h"
#include "ov9650.h"
#include "systick.h"
#include "ov9650_regs.h"

#define NUM_BR_LEVELS       7

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
    {REG_EDGE,   0xa6}, /* Edge enhancement treshhold and factor */
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

static int reset(sensor_t *sensor)
{
    int i=0;
    const uint8_t (*regs)[2]=default_regs;

    /* Reset all registers */
    SCCB_Write(sensor->slv_addr, REG_COM7, 0x80);

    /* delay n ms */
    systick_sleep(10);

    /* Write initial regsiters */
    while (regs[i][0]) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int i=0;
    const uint8_t (*regs)[2];
    uint8_t com7=0; /* framesize/RGB */

    /* read pixel format reg */
    com7 = SCCB_Read(sensor->slv_addr, REG_COM7);

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
    SCCB_Write(sensor->slv_addr, REG_COM7, com7);

    /* Write pixel format registers */
    while (regs[i][0]) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
        i++;
    }

    return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    uint8_t com7=0; /* framesize/RGB */
    uint8_t com1=0; /* Skip option */

    /* read COM7 RGB bit */
    com7 = (SCCB_Read(sensor->slv_addr, REG_COM7) & REG_COM7_RGB);

    switch (framesize) {
        case FRAMESIZE_QQCIF:
            com7 |= REG_COM7_QCIF;
            com1 |= REG_COM1_QQCIF|REG_COM1_SKIP2;
            break;
        case FRAMESIZE_QQVGA:
            com7 |= REG_COM7_QVGA;
            com1 |= REG_COM1_QQVGA|REG_COM1_SKIP2;
            break;
        case FRAMESIZE_QCIF:
            com7 |= REG_COM7_QCIF;
            break;
        default:
            return -1;
    }

    /* write the frame size registers */
    SCCB_Write(sensor->slv_addr, REG_COM1, com1);
    SCCB_Write(sensor->slv_addr, REG_COM7, com7);

    return 0;
}

static int set_framerate(sensor_t *sensor, framerate_t framerate)
{
    /* Write framerate register */
    SCCB_Write(sensor->slv_addr, REG_CLKRC, framerate);
    return 0;
}

static int set_brightness(sensor_t *sensor, int level)
{
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

    for (i=0; i<3; i++) {
        SCCB_Write(sensor->slv_addr, regs[0][i], regs[level][i]);
    }

    return 0;
}

//static int set_exposure(sensor_t *sensor, int exposure)
//{
//   uint8_t val;
//   val = SCCB_Read(sensor->slv_addr, REG_COM1);

//   /* exposure [1:0] */
//   SCCB_Write(sensor->slv_addr, REG_COM1, val | (exposure&0x03));

//   /* exposure [9:2] */
//   SCCB_Write(sensor->slv_addr, REG_AECH, ((exposure>>2)&0xFF));

//   /* exposure [15:10] */
//   SCCB_Write(sensor->slv_addr, REG_AECHM, ((exposure>>10)&0x3F));

//   return 0;
//}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    /* Write gain ceiling register */
    SCCB_Write(sensor->slv_addr, REG_COM9, (gainceiling<<4));
    return 0;
}

static int set_auto_gain(sensor_t *sensor, int enable, int gain)
{
   uint8_t val;
   val = SCCB_Read(sensor->slv_addr, REG_COM8);

   SCCB_Write(sensor->slv_addr, REG_COM8,
              enable ? (val | REG_COM8_AGC) : (val & ~REG_COM8_AGC));

   return 0;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure)
{
   uint8_t val;
   val = SCCB_Read(sensor->slv_addr, REG_COM8);

   SCCB_Write(sensor->slv_addr, REG_COM8,
              enable ? (val | REG_COM8_AEC) : (val & ~REG_COM8_AEC));

   return 0;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, int r_gain, int g_gain, int b_gain)
{
   uint8_t val;
   val = SCCB_Read(sensor->slv_addr, REG_COM8);

   SCCB_Write(sensor->slv_addr, REG_COM8,
              enable ? (val | REG_COM8_AWB) : (val & ~REG_COM8_AWB));

   return 0;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
   uint8_t val;
   val = SCCB_Read(sensor->slv_addr, REG_MVFP);

   SCCB_Write(sensor->slv_addr, REG_MVFP,
              enable ? (val | REG_MVFP_HMIRROR) : (val & ~REG_MVFP_HMIRROR));

   return 0;
}

static int set_vflip(sensor_t *sensor, int enable)
{
   uint8_t val;
   val = SCCB_Read(sensor->slv_addr, REG_MVFP);

   SCCB_Write(sensor->slv_addr, REG_MVFP,
              enable ? (val | REG_MVFP_VFLIP) : (val & ~REG_MVFP_VFLIP));

   return 0;
}

int ov9650_init(sensor_t *sensor)
{
    // Set function pointers
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_framerate = set_framerate;
    sensor->set_brightness= set_brightness;
    sensor->set_gainceiling = set_gainceiling;
    sensor->set_auto_gain = set_auto_gain;
    sensor->set_auto_exposure = set_auto_exposure;
    sensor->set_auto_whitebal = set_auto_whitebal;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;

    // Set sensor flags
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
