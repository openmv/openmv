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

#if (OMV_SENSOR_HM01B0 == 1)
static const uint16_t default_regs[][2] = {
    {BLC_TGT,           0x08},          //  BLC target :8  at 8 bit mode
    {BLC2_TGT,          0x08},          //  BLI target :8  at 8 bit mode
    {0x3044,            0x0A},          //  Increase CDS time for settling
    {0x3045,            0x00},          //  Make symetric for cds_tg and rst_tg
    {0x3047,            0x0A},          //  Increase CDS time for settling
    {0x3050,            0xC0},          //  Make negative offset up to 4x
    {0x3051,            0x42},
    {0x3052,            0x50},
    {0x3053,            0x00},
    {0x3054,            0x03},          //  tuning sf sig clamping as lowest
    {0x3055,            0xF7},          //  tuning dsun
    {0x3056,            0xF8},          //  increase adc nonoverlap clk
    {0x3057,            0x29},          //  increase adc pwr for missing code
    {0x3058,            0x1F},          //  turn on dsun
    {0x3059,            0x1E},
    {0x3064,            0x00},
    {0x3065,            0x04},          //  pad pull 0

    {BLC_CFG,           0x43},          //  BLC_on, IIR

    {0x1001,            0x43},          //  BLC dithering en
    {0x1002,            0x43},          //  blc_darkpixel_thd
    {0x0350,            0x00},          //  Dgain Control
    {BLI_EN,            0x01},          //  BLI enable
    {0x1003,            0x00},          //  BLI Target [Def: 0x20]

    {DPC_CTRL,          0x01},          //  DPC option 0: DPC off   1 : mono   3 : bayer1   5 : bayer2
    {0x1009,            0xA0},          //  cluster hot pixel th
    {0x100A,            0x60},          //  cluster cold pixel th
    {SINGLE_THR_HOT,    0x90},          //  single hot pixel th
    {SINGLE_THR_COLD,   0x40},          //  single cold pixel th
    {0x1012,            0x00},          //  Sync. shift disable
    {0x2000,            0x07},
    {0x2003,            0x00},
    {0x2004,            0x1C},
    {0x2007,            0x00},
    {0x2008,            0x58},
    {0x200B,            0x00},
    {0x200C,            0x7A},
    {0x200F,            0x00},
    {0x2010,            0xB8},
    {0x2013,            0x00},
    {0x2014,            0x58},
    {0x2017,            0x00},
    {0x2018,            0x9B},

    {AE_CTRL,           0x01},          //Automatic Exposure
    {AE_TARGET_MEAN,    0x3C},          //AE target mean          [Def: 0x3C]
    {AE_MIN_MEAN,       0x0A},          //AE min target mean      [Def: 0x0A]

    {INTEGRATION_H,     0x00},          //Integration H           [Def: 0x01]
    {INTEGRATION_L,     0x60},          //Integration L           [Def: 0x08]
    {ANALOG_GAIN,       0x00},          //Analog Global Gain      [Def: 0x00]
    {DAMPING_FACTOR,    0x20},          //Damping Factor          [Def: 0x20]
    {DIGITAL_GAIN_H,    0x01},          //Digital Gain High       [Def: 0x01]
    {DIGITAL_GAIN_L,    0x00},          //Digital Gain Low        [Def: 0x00]

    {CONVERGE_IN_TH,    0x03},          //Converge in threshold   [Def: 0x03]
    {CONVERGE_OUT_TH,   0x05},          //Converge out threshold  [Def: 0x05]
    {MAX_INTG_H,        0x01},          //Maximum INTG High Byte  [Def: 0x01]
    {MAX_INTG_L,        0x54},          //Maximum INTG Low Byte   [Def: 0x54]
    {MAX_AGAIN_FULL,    0x03},          //Maximum Analog gain in full frame mode [Def: 0x03]
    {MAX_AGAIN_BIN2,    0x04},          //Maximum Analog gain in bin2 mode       [Def: 0x04]

    {0x210B,            0xC0},
    {0x210E,            0x00},          //Flicker Control
    {0x210F,            0x00},
    {0x2110,            0x3C},
    {0x2111,            0x00},
    {0x2112,            0x32},

    {0x2150,            0x30},
    {0x0340,            0x02},
    {0x0341,            0x16},
    {0x0342,            0x01},
    {0x0343,            0x78},
    {0x3010,            0x01},
    {0x0383,            0x01},
    {0x0387,            0x01},
    {0x0390,            0x00},
    {0x3011,            0x70},
    {0x3059,            0x02},
    {0x3060,            0x01},
    {IMG_ORIENTATION,   0x01},          // change the orientation
    {0x0104,            0x01},

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
        if (cambus_writeb2(sensor->slv_addr, SW_RESET, HIMAX_RESET) != 0) {
            return -1;
        }
        // Delay for 1ms.
        systick_sleep(1);
        if (cambus_readb2(sensor->slv_addr, MODE_SELECT, &reg) != 0) {
            return -1;
        }
    }

    // Write default regsiters
    int ret = 0;
    for (int i=0; default_regs[i][0] && ret == 0; i++) {
        ret |= cambus_writeb2(sensor->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Set PCLK polarity.
    ret |= cambus_writeb2(sensor->slv_addr, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE));
    
    // Set mode to streaming
    ret |= cambus_writeb2(sensor->slv_addr, MODE_SELECT, HIMAX_MODE_STREAMING);

    return ret;
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint8_t reg_data;
    if (cambus_readb2(sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data)
{
    return cambus_writeb2(sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret = 0;
    switch (pixformat) {
        case PIXFORMAT_BAYER:
            break;
        default:
            return -1;
    }

    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    if ((w != 320) || (h != 240)) {
        ret = -1;
    }

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, IMG_ORIENTATION, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, IMG_ORIENTATION, HIMAX_SET_HMIRROR(reg, enable)) ;
    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = cambus_readb2(sensor->slv_addr, IMG_ORIENTATION, &reg);
    ret |= cambus_writeb2(sensor->slv_addr, IMG_ORIENTATION, HIMAX_SET_VMIRROR(reg, enable)) ;
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
    sensor->set_hmirror         = set_hmirror;
    sensor->set_vflip           = set_vflip;

    // Set sensor flags
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 0);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 1);
    SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);

    return 0;
}
#endif //(OMV_SENSOR_HM01B0 == 1)
