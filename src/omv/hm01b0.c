/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HM01B0 driver.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include STM32_HAL_H
#include "cambus.h"
#include "hm01b0.h"
#include "hm01b0_regs.h"
#include "systick.h"
#include "omv_boardconfig.h"
#define HIMAX_BOOT_RETRY        (10)

#define HIMAX_LINE_LEN_PCK 0x172
#define HIMAX_FRAME_LENGTH 0x232
#if (OMV_ENABLE_HM01B0 == 1)
static const uint16_t default_regs[][2] = {
    {BLC_TGT,              0x08},          //  BLC target :8  at 8 bit mode
    {BLC2_TGT,             0x08},          //  BLI target :8  at 8 bit mode
    {0x3044,               0x0A},          //  Increase CDS time for settling
    {0x3045,               0x00},          //  Make symetric for cds_tg and rst_tg
    {0x3047,               0x0A},          //  Increase CDS time for settling
    {0x3050,               0xC0},          //  Make negative offset up to 4x
    {0x3051,               0x42},
    {0x3052,               0x50},
    {0x3053,               0x00},
    {0x3054,               0x03},          //  tuning sf sig clamping as lowest
    {0x3055,               0xF7},          //  tuning dsun
    {0x3056,               0xF8},          //  increase adc nonoverlap clk
    {0x3057,               0x29},          //  increase adc pwr for missing code
    {0x3058,               0x1F},          //  turn on dsun
    {0x3059,               0x1E},
    {0x3064,               0x00},
    {0x3065,               0x04},          //  pad pull 0
   
    {BLC_CFG,              0x43},          //  BLC_on, IIR
   
    {0x1001,               0x43},          //  BLC dithering en
    {0x1002,               0x43},          //  blc_darkpixel_thd
    {0x0350,               0x7F},          //  Dgain Control
    {BLI_EN,               0x01},          //  BLI enable
    {0x1003,               0x00},          //  BLI Target [Def: 0x20]
   
    {DPC_CTRL,             0x01},          //  DPC option 0: DPC off   1 : mono   3 : bayer1   5 : bayer2
    {0x1009,               0xA0},          //  cluster hot pixel th
    {0x100A,               0x60},          //  cluster cold pixel th
    {SINGLE_THR_HOT,       0x90},          //  single hot pixel th
    {SINGLE_THR_COLD,      0x40},          //  single cold pixel th
    {0x1012,               0x00},          //  Sync. shift disable
    {0x2000,               0x07},
    {0x2003,               0x00},
    {0x2004,               0x1C},
    {0x2007,               0x00},
    {0x2008,               0x58},
    {0x200B,               0x00},
    {0x200C,               0x7A},
    {0x200F,               0x00},
    {0x2010,               0xB8},
    {0x2013,               0x00},
    {0x2014,               0x58},
    {0x2017,               0x00},
    {0x2018,               0x9B},
   
    {AE_CTRL,              0x01},          //Automatic Exposure
    {AE_TARGET_MEAN,       0x3C},          //AE target mean          [Def: 0x3C]
    {AE_MIN_MEAN,          0x0A},          //AE min target mean      [Def: 0x0A]
    {CONVERGE_IN_TH,       0x03},          //Converge in threshold   [Def: 0x03]
    {CONVERGE_OUT_TH,      0x05},          //Converge out threshold  [Def: 0x05]
    {MAX_INTG_H,           0x01},          //Maximum INTG High Byte  [Def: 0x01]
    {MAX_INTG_L,           0x54},          //Maximum INTG Low Byte   [Def: 0x54]
    {MAX_AGAIN_FULL,       0x03},          //Maximum Analog gain in full frame mode [Def: 0x03]
    {MAX_AGAIN_BIN2,       0x04},          //Maximum Analog gain in bin2 mode       [Def: 0x04]
    {MAX_DGAIN,            0xC0},
   
    {INTEGRATION_H,        0x01},          //Integration H           [Def: 0x01]
    {INTEGRATION_L,        0x08},          //Integration L           [Def: 0x08]
    {ANALOG_GAIN,          0x00},          //Analog Global Gain      [Def: 0x00]
    {DAMPING_FACTOR,       0x20},          //Damping Factor          [Def: 0x20]
    {DIGITAL_GAIN_H,       0x01},          //Digital Gain High       [Def: 0x01]
    {DIGITAL_GAIN_L,       0x00},          //Digital Gain Low        [Def: 0x00]
   
    {FS_CTRL,              0x00},          //Flicker Control
   
    {FS_60HZ_H,            0x00},
    {FS_60HZ_L,            0x3C},
    {FS_50HZ_H,            0x00},
    {FS_50HZ_L,            0x32},

    {MD_CTRL,              0x30},
    {FRAME_LEN_LINES_H,    HIMAX_FRAME_LENGTH>>8},
    {FRAME_LEN_LINES_L,    HIMAX_FRAME_LENGTH&0xFF},
    {LINE_LEN_PCK_H,       HIMAX_LINE_LEN_PCK>>8},
    {LINE_LEN_PCK_L,       HIMAX_LINE_LEN_PCK&0xFF},
    {0x3010,               0x00},          // no full frame
    {0x0383,               0x01},
    {0x0387,               0x01},
    {0x0390,               0x00},
    {0x3011,               0x70},
    {0x3059,               0x02},
    {0x3060,               0x0B},
    {IMG_ORIENTATION,      0x00},          // change the orientation
    {0x0104,               0x01},

    //============= End of regs marker ==================
    {0x0000,            0x00},
};

static int reset(sensor_t *sensor)
{
    // Reset sensor.
    uint8_t reg=0xff;
    for (int retry=HIMAX_BOOT_RETRY; reg != HIMAX_MODE_STANDBY; retry--) {
        if (retry == 0) {
            return -1;
        }
        if (cambus_writeb2(&sensor->i2c, sensor->slv_addr, SW_RESET, HIMAX_RESET) != 0) {
            return -1;
        }
        // Delay for 1ms.
        systick_sleep(1);
        if (cambus_readb2(&sensor->i2c, sensor->slv_addr, MODE_SELECT, &reg) != 0) {
            return -1;
        }
    }

    // Write default regsiters
    int ret = 0;
    for (int i=0; default_regs[i][0] && ret == 0; i++) {
        ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Set PCLK polarity.
    ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE));
    
    // Set mode to streaming
    ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, MODE_SELECT, HIMAX_MODE_STREAMING);

    return ret;
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint8_t reg_data;
    if (cambus_readb2(&sensor->i2c, sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data)
{
    return cambus_writeb2(&sensor->i2c, sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret = 0;
    switch (pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            break;
        default:
            return -1;
    }

    return ret;
}

static const uint16_t QVGA_regs[][2] = {
    {0x0383,            0x01},
    {0x0387,            0x01},
    {0x0390,            0x00},
        //============= End of regs marker ==================
    {0x0000,            0x00},

};

static const uint16_t QQVGA_regs[][2] = {
    {0x0383,            0x03},
    {0x0387,            0x03},
    {0x0390,            0x03},
        //============= End of regs marker ==================
    {0x0000,            0x00},

};

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    switch (framesize) {
        case FRAMESIZE_QVGA:
            for (int i=0; QVGA_regs[i][0] && ret == 0; i++) {
                ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, QVGA_regs[i][0], QVGA_regs[i][1]);
            }
            break;
        case FRAMESIZE_QQVGA:
            for (int i=0; QQVGA_regs[i][0] && ret == 0; i++) {
                ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, QQVGA_regs[i][0], QQVGA_regs[i][1]);
            }
            break;
        default: 
            if (w>320 || h>320) 
                ret = -1;
            
    }

    return ret;
}

static int set_contrast(sensor_t *sensor, int level)
{
    return 0;
}

static int set_brightness(sensor_t *sensor, int level)
{
    return 0;
}

static int set_saturation(sensor_t *sensor, int level)
{
    return 0;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    return 0;
}

static int set_quality(sensor_t *sensor, int quality)
{
    return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
{
    return 0;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
    return 0;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    return 0;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    int ret=0;

    if (enable) {
        ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, AE_CTRL, 1);
    } else {
        int coarse_int = exposure_us*(OMV_XCLK_FREQUENCY/1000000)/LINE_LEN_PCK_H;
        if (coarse_int<2) coarse_int = 2;
        if (coarse_int>HIMAX_FRAME_LENGTH-2) coarse_int = HIMAX_FRAME_LENGTH-2;
        ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, AE_CTRL, 0);
        ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, INTEGRATION_H, coarse_int>>8);
        ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, INTEGRATION_L, coarse_int&0xff);
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    return 0;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    return 0;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    return 0;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(&sensor->i2c, sensor->slv_addr, IMG_ORIENTATION, &reg);
    ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, IMG_ORIENTATION, HIMAX_SET_HMIRROR(reg, enable)) ;
    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(&sensor->i2c, sensor->slv_addr, IMG_ORIENTATION, &reg);
    ret |= cambus_writeb2(&sensor->i2c, sensor->slv_addr, IMG_ORIENTATION, HIMAX_SET_VMIRROR(reg, enable)) ;
    return ret;
}

int hm01b0_init(sensor_t *sensor)
{
    // Initialize sensor structure.
    sensor->gs_bpp              = 1;
    sensor->reset               = reset;
    sensor->read_reg            = read_reg;
    sensor->write_reg           = write_reg;
    sensor->set_pixformat       = set_pixformat;
    sensor->set_framesize       = set_framesize;
    sensor->set_contrast        = set_contrast;
    sensor->set_brightness      = set_brightness;
    sensor->set_saturation      = set_saturation;
    sensor->set_gainceiling     = set_gainceiling;
    sensor->set_quality         = set_quality;
    sensor->set_colorbar        = set_colorbar;
    sensor->set_special_effect  = set_special_effect;
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
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
#endif //(OMV_ENABLE_HM01B0 == 1)
