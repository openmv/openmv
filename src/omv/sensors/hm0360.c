/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * HM0360 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_ENABLE_HM0360 == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "omv_i2c.h"
#include "sensor.h"
#include "hm0360.h"
#include "hm0360_regs.h"
#include "py/mphal.h"

#define HIMAX_BOOT_RETRY            (10)
#define HIMAX_LINE_LEN_PCK_VGA      0x300
#define HIMAX_FRAME_LENGTH_VGA      0x214

#define HIMAX_LINE_LEN_PCK_QVGA     0x178
#define HIMAX_FRAME_LENGTH_QVGA     0x109

#define HIMAX_LINE_LEN_PCK_QQVGA    0x178
#define HIMAX_FRAME_LENGTH_QQVGA    0x084

#define HIMAX_MD_ROI_VGA_W          40
#define HIMAX_MD_ROI_VGA_H          30

#define HIMAX_MD_ROI_QVGA_W         20
#define HIMAX_MD_ROI_QVGA_H         15

#define HIMAX_MD_ROI_QQVGA_W        10
#define HIMAX_MD_ROI_QQVGA_H        8

static const uint16_t default_regs[][2] = {
    {SW_RESET,          0x00},
    {MONO_MODE,         0x00},
    {MONO_MODE_ISP,     0x01},
    {MONO_MODE_SEL,     0x01},

    // BLC control
    {0x1000,            0x01},
    {0x1003,            0x04},
    {BLC_TGT,           0x04},
    {0x1007,            0x01},
    {0x1008,            0x04},
    {BLC2_TGT,          0x04},
    {MONO_CTRL,         0x01},

    // Output format control
    {OPFM_CTRL,         0x0C},

    // Reserved regs
    {0x101D,            0x00},
    {0x101E,            0x01},
    {0x101F,            0x00},
    {0x1020,            0x01},
    {0x1021,            0x00},

    {CMPRS_CTRL,        0x00},
    {CMPRS_01,          0x09},
    {CMPRS_02,          0x12},
    {CMPRS_03,          0x23},
    {CMPRS_04,          0x31},
    {CMPRS_05,          0x3E},
    {CMPRS_06,          0x4B},
    {CMPRS_07,          0x56},
    {CMPRS_08,          0x5E},
    {CMPRS_09,          0x65},
    {CMPRS_10,          0x72},
    {CMPRS_11,          0x7F},
    {CMPRS_12,          0x8C},
    {CMPRS_13,          0x98},
    {CMPRS_14,          0xB2},
    {CMPRS_15,          0xCC},
    {CMPRS_16,          0xE6},

    {0x3112,            0x00},  // PCLKO_polarity falling

    {PLL1_CONFIG,       0x08},  // Core = 24MHz PCLKO = 24MHz I2C = 12MHz
    {PLL2_CONFIG,       0x0A},  // MIPI pre-dev (default)
    {PLL3_CONFIG,       0x77},  // PMU/MIPI pre-dev (default)

    {PMU_CFG_3,         0x08},  // Disable context switching
    {PAD_REGISTER_07,   0x00},  // PCLKO_polarity falling

    {AE_CTRL,           0x5F},  // Automatic Exposure (NOTE: Auto framerate enabled)
    {AE_CTRL1,          0x00},
    {T_DAMPING,         0x20},  // AE T damping factor
    {N_DAMPING,         0x00},  // AE N damping factor
    {AE_TARGET_MEAN,    0x64},  // AE target
    {AE_MIN_MEAN,       0x0A},  // AE min target mean
    {AE_TARGET_ZONE,    0x23},  // AE target zone
    {CONVERGE_IN_TH,    0x03},  // AE converge in threshold
    {CONVERGE_OUT_TH,   0x05},  // AE converge out threshold
    {MAX_INTG_H,        (HIMAX_FRAME_LENGTH_QVGA-4)>>8},
    {MAX_INTG_L,        (HIMAX_FRAME_LENGTH_QVGA-4)&0xFF},

    {MAX_AGAIN,         0x04},  // Maximum analog gain
    {MAX_DGAIN_H,       0x03},
    {MAX_DGAIN_L,       0x3F},
    {INTEGRATION_H,     0x01},
    {INTEGRATION_L,     0x08},

    {MD_CTRL,           0x6A},
    {MD_TH_MIN,         0x01},
    {MD_BLOCK_NUM_TH,   0x01},
    {MD_CTRL1,          0x06},
    {PULSE_MODE,        0x00},  // Interrupt in level mode.
    {ROI_START_END_V,   0xF0},
    {ROI_START_END_H,   0xF0},

    {FRAME_LEN_LINES_H, HIMAX_FRAME_LENGTH_QVGA>>8},
    {FRAME_LEN_LINES_L, HIMAX_FRAME_LENGTH_QVGA&0xFF},
    {LINE_LEN_PCK_H,    HIMAX_LINE_LEN_PCK_QVGA>>8},
    {LINE_LEN_PCK_L,    HIMAX_LINE_LEN_PCK_QVGA&0xFF},
    {H_SUBSAMPLE,       0x01},
    {V_SUBSAMPLE,       0x01},
    {BINNING_MODE,      0x00},
    {WIN_MODE,          0x00},
    {IMG_ORIENTATION,   0x00},
    {COMMAND_UPDATE,    0x01},

    /// SYNC function config.
    {0x3010,            0x00},
    {0x3013,            0x01},
    {0x3019,            0x00},
    {0x301A,            0x00},
    {0x301B,            0x20},
    {0x301C,            0xFF},

    // PREMETER config.
    {0x3026,            0x03},
    {0x3027,            0x81},
    {0x3028,            0x01},
    {0x3029,            0x00},
    {0x302A,            0x30},
    {0x302E,            0x00},
    {0x302F,            0x00},

    // Magic regs 🪄.
    {0x302B,            0x2A},
    {0x302C,            0x00},
    {0x302D,            0x03},
    {0x3031,            0x01},
    {0x3051,            0x00},
    {0x305C,            0x03},
    {0x3060,            0x00},
    {0x3061,            0xFA},
    {0x3062,            0xFF},
    {0x3063,            0xFF},
    {0x3064,            0xFF},
    {0x3065,            0xFF},
    {0x3066,            0xFF},
    {0x3067,            0xFF},
    {0x3068,            0xFF},
    {0x3069,            0xFF},
    {0x306A,            0xFF},
    {0x306B,            0xFF},
    {0x306C,            0xFF},
    {0x306D,            0xFF},
    {0x306E,            0xFF},
    {0x306F,            0xFF},
    {0x3070,            0xFF},
    {0x3071,            0xFF},
    {0x3072,            0xFF},
    {0x3073,            0xFF},
    {0x3074,            0xFF},
    {0x3075,            0xFF},
    {0x3076,            0xFF},
    {0x3077,            0xFF},
    {0x3078,            0xFF},
    {0x3079,            0xFF},
    {0x307A,            0xFF},
    {0x307B,            0xFF},
    {0x307C,            0xFF},
    {0x307D,            0xFF},
    {0x307E,            0xFF},
    {0x307F,            0xFF},
    {0x3080,            0x01},
    {0x3081,            0x01},
    {0x3082,            0x03},
    {0x3083,            0x20},
    {0x3084,            0x00},
    {0x3085,            0x20},
    {0x3086,            0x00},
    {0x3087,            0x20},
    {0x3088,            0x00},
    {0x3089,            0x04},
    {0x3094,            0x02},
    {0x3095,            0x02},
    {0x3096,            0x00},
    {0x3097,            0x02},
    {0x3098,            0x00},
    {0x3099,            0x02},
    {0x309E,            0x05},
    {0x309F,            0x02},
    {0x30A0,            0x02},
    {0x30A1,            0x00},
    {0x30A2,            0x08},
    {0x30A3,            0x00},
    {0x30A4,            0x20},
    {0x30A5,            0x04},
    {0x30A6,            0x02},
    {0x30A7,            0x02},
    {0x30A8,            0x01},
    {0x30A9,            0x00},
    {0x30AA,            0x02},
    {0x30AB,            0x34},
    {0x30B0,            0x03},
    {0x30C4,            0x10},
    {0x30C5,            0x01},
    {0x30C6,            0xBF},
    {0x30C7,            0x00},
    {0x30C8,            0x00},
    {0x30CB,            0xFF},
    {0x30CC,            0xFF},
    {0x30CD,            0x7F},
    {0x30CE,            0x7F},
    {0x30D3,            0x01},
    {0x30D4,            0xFF},
    {0x30D5,            0x00},
    {0x30D6,            0x40},
    {0x30D7,            0x00},
    {0x30D8,            0xA7},
    {0x30D9,            0x05},
    {0x30DA,            0x01},
    {0x30DB,            0x40},
    {0x30DC,            0x00},
    {0x30DD,            0x27},
    {0x30DE,            0x05},
    {0x30DF,            0x07},
    {0x30E0,            0x40},
    {0x30E1,            0x00},
    {0x30E2,            0x27},
    {0x30E3,            0x05},
    {0x30E4,            0x47},
    {0x30E5,            0x30},
    {0x30E6,            0x00},
    {0x30E7,            0x27},
    {0x30E8,            0x05},
    {0x30E9,            0x87},
    {0x30EA,            0x30},
    {0x30EB,            0x00},
    {0x30EC,            0x27},
    {0x30ED,            0x05},
    {0x30EE,            0x00},
    {0x30EF,            0x40},
    {0x30F0,            0x00},
    {0x30F1,            0xA7},
    {0x30F2,            0x05},
    {0x30F3,            0x01},
    {0x30F4,            0x40},
    {0x30F5,            0x00},
    {0x30F6,            0x27},
    {0x30F7,            0x05},
    {0x30F8,            0x07},
    {0x30F9,            0x40},
    {0x30FA,            0x00},
    {0x30FB,            0x27},
    {0x30FC,            0x05},
    {0x30FD,            0x47},
    {0x30FE,            0x30},
    {0x30FF,            0x00},
    {0x3100,            0x27},
    {0x3101,            0x05},
    {0x3102,            0x87},
    {0x3103,            0x30},
    {0x3104,            0x00},
    {0x3105,            0x27},
    {0x3106,            0x05},
    {0x310B,            0x10},
    {0x3113,            0xA0},
    {0x3114,            0x67},
    {0x3115,            0x42},
    {0x3116,            0x10},
    {0x3117,            0x0A},
    {0x3118,            0x3F},
    {0x311C,            0x10},
    {0x311D,            0x06},
    {0x311E,            0x0F},
    {0x311F,            0x0E},
    {0x3120,            0x0D},
    {0x3121,            0x0F},
    {0x3122,            0x00},
    {0x3123,            0x1D},
    {0x3126,            0x03},
    {0x3128,            0x57},
    {0x312A,            0x11},
    {0x312B,            0x41},
    {0x312E,            0x00},
    {0x312F,            0x00},
    {0x3130,            0x0C},
    {0x3141,            0x2A},
    {0x3142,            0x9F},
    {0x3147,            0x18},
    {0x3149,            0x18},
    {0x314B,            0x01},
    {0x3150,            0x50},
    {0x3152,            0x00},
    {0x3156,            0x2C},
    {0x315A,            0x0A},
    {0x315B,            0x2F},
    {0x315C,            0xE0},
    {0x315F,            0x02},
    {0x3160,            0x1F},
    {0x3163,            0x1F},
    {0x3164,            0x7F},
    {0x3165,            0x7F},
    {0x317B,            0x94},
    {0x317C,            0x00},
    {0x317D,            0x02},
    {0x318C,            0x00},

    {COMMAND_UPDATE,    0x01},
    {0x0000,            0x00},
};

static int reset(sensor_t *sensor)
{
    // Reset sensor.
    uint8_t reg=0xff;
    for (int retry=HIMAX_BOOT_RETRY; retry >= 0 && reg != HIMAX_MODE_STANDBY; retry--) {
        if (omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, SW_RESET, HIMAX_RESET) != 0) {
            return -1;
        }

        mp_hal_delay_ms(1);

        if (omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, MODE_SELECT, &reg) != 0) {
            return -1;
        }

        if (reg == HIMAX_MODE_STANDBY) {
            break;
        } else if (retry == 0) {
            return -1;
        }

        mp_hal_delay_ms(10);
    }

    // Write default regsiters
    int ret = 0;
    for (int i=0; default_regs[i][0] && ret == 0; i++) {
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Set mode to streaming
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MODE_SELECT, HIMAX_MODE_STREAMING);

    return ret;
}

static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint8_t reg_data;
    if (omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data)
{
    return omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    switch (pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            return 0;
        default:
            return -1;
    }
}

static const uint16_t VGA_regs[][2] = {
    {PLL1_CONFIG,           0x08},  // Core = 24MHz PCLKO = 24MHz I2C = 12MHz
    {H_SUBSAMPLE,           0x00},
    {V_SUBSAMPLE,           0x00},
    {BINNING_MODE,          0x00},
    {WIN_MODE,              0x00},
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_VGA-4)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_VGA-4)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_VGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_VGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_VGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_VGA&0xFF)},
    {ROI_START_END_H,       0xF0},
    {ROI_START_END_V,       0xE0},
    {COMMAND_UPDATE,        0x01},
    {0x0000,                0x00},
};

static const uint16_t QVGA_regs[][2] = {
    {PLL1_CONFIG,           0x09},  // Core = 12MHz PCLKO = 24MHz I2C = 12MHz
    {H_SUBSAMPLE,           0x01},
    {V_SUBSAMPLE,           0x01},
    {BINNING_MODE,          0x00},
    {WIN_MODE,              0x00},
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QVGA-4)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QVGA-4)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QVGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QVGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QVGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QVGA&0xFF)},
    {ROI_START_END_H,       0xF0},
    {ROI_START_END_V,       0xE0},
    {COMMAND_UPDATE,        0x01},
    {0x0000,                0x00},
};

static const uint16_t QQVGA_regs[][2] = {
    {PLL1_CONFIG,           0x09},  // Core = 12MHz PCLKO = 24MHz I2C = 12MHz
    {H_SUBSAMPLE,           0x02},
    {V_SUBSAMPLE,           0x02},
    {BINNING_MODE,          0x00},
    {WIN_MODE,              0x00},
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QQVGA-4)>>8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QQVGA-4)&0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QQVGA>>8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QQVGA&0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QQVGA>>8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QQVGA&0xFF)},
    {ROI_START_END_H,       0xF0},
    {ROI_START_END_V,       0xD0},
    {COMMAND_UPDATE,        0x01},
    {0x0000,                0x00},
};

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    switch (framesize) {
        case FRAMESIZE_VGA:
            for (int i=0; VGA_regs[i][0] && ret == 0; i++) {
                ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, VGA_regs[i][0], VGA_regs[i][1]);
            }
            break;
        case FRAMESIZE_QVGA:
            for (int i=0; QVGA_regs[i][0] && ret == 0; i++) {
                ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, QVGA_regs[i][0], QVGA_regs[i][1]);
            }
            break;
        case FRAMESIZE_QQVGA:
            for (int i=0; QQVGA_regs[i][0] && ret == 0; i++) {
                ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, QQVGA_regs[i][0], QQVGA_regs[i][1]);
            }
            break;
        default:
            ret = -1;
    }
    return ret;
}

static int set_framerate(sensor_t *sensor, int framerate)
{
    int ret = 0;
    uint8_t pll_cfg = 0;
    uint8_t osc_div = 0;
    bool    highres = false;

    if (sensor->framesize == FRAMESIZE_VGA) {
        highres = true;
    }

    if (framerate <= 10) {
        osc_div = (highres == true) ? 0x03 : 0x03;
    } else if (framerate <= 15) {
        osc_div = (highres == true) ? 0x02 : 0x03;
    } else if (framerate <= 30) {
        osc_div = (highres == true) ? 0x01 : 0x02;
    } else {
        // Set to the max possible FPS at this resolution.
        osc_div = (highres == true) ? 0x00 : 0x01;
    }

    ret |= omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, PLL1_CONFIG, &pll_cfg);
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, PLL1_CONFIG, (pll_cfg & 0xFC) | osc_div);
    return ret;
}

static int set_brightness(sensor_t *sensor, int level)
{
    uint8_t ae_mean;
    // Simulate brightness levels by setting AE loop target mean.
    switch (level) {
        case 1:
            ae_mean = 150;
            break;
        case 2:
            ae_mean = 200;
            break;
        case 3:
            ae_mean = 250;
            break;
        case 0:
        default:
            ae_mean = 100;
            break;
    }
    return omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, AE_TARGET_MEAN, ae_mean);
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    int ret = 0;
    int gain = 0x0;
    switch (gainceiling) {
        case GAINCEILING_2X:
            gain = 0x01;
            break;
        case GAINCEILING_4X:
            gain = 0x02;
            break;
        case GAINCEILING_8X:
            gain = 0x03;
            break;
        case GAINCEILING_16X:
            gain = 0x04;
            break;
        default:
            return -1;
    }
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MAX_AGAIN, (gain & 0x07));
    return ret;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    return omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, TEST_PATTERN_MODE, enable & 0x1);
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
    uint8_t ae_ctrl = 0;
    int ret = omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, AE_CTRL, &ae_ctrl);
    if (!enable && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        gain_db = IM_MAX(IM_MIN(gain_db, 24.0f), 0.0f);
        uint8_t gain = fast_ceilf(fast_log2(fast_expf((gain_db / 20.0f) * fast_log(10.0f))));
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, AE_CTRL, (ae_ctrl & 0xFE));
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, ANALOG_GAIN, ((gain & 0x7)<<4));
    } else if (enable && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        gain_db_ceiling = IM_MAX(IM_MIN(gain_db_ceiling, 24.0f), 0.0f);
        uint8_t gain = fast_ceilf(fast_log2(fast_expf((gain_db_ceiling / 20.0f) * fast_log(10.0f))));
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MAX_AGAIN, (gain & 0x07));
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, AE_CTRL, (ae_ctrl | 0x01));
    }
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, COMMAND_UPDATE, 0x01);
    return ret;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    uint8_t gain;
    if (omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, ANALOG_GAIN, &gain) != 0) {
        return -1;
    }
    *gain_db = fast_floorf(fast_log(1 << (gain>>4)) / fast_log(10.0f) * 20.0f);
    return 0;
}

static int get_vt_pix_clk(sensor_t *sensor, uint32_t *vt_pix_clk)
{
    uint8_t reg;
    if (omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, PLL1_CONFIG, &reg) != 0) {
        return -1;
    }
    // 00 -> MCLK / 1
    // 01 -> MCLK / 2
    // 10 -> MCLK / 4
    // 11 -> MCLK / 8
    uint32_t vt_sys_div = (1 << (reg & 0x03));

    // vt_pix_clk = MCLK / vt_sys_div
    *vt_pix_clk = OMV_XCLK_FREQUENCY / vt_sys_div;
    return 0;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    uint8_t ae_ctrl = 0;
    int ret = omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, AE_CTRL, &ae_ctrl);

    if (enable) {
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, AE_CTRL, (ae_ctrl | 0x01));
    } else {
        uint32_t line_len;
        uint32_t frame_len;
        uint32_t coarse_int;
        uint32_t vt_pix_clk = 0;

        switch (sensor->framesize) {
            case FRAMESIZE_VGA:
                line_len = HIMAX_LINE_LEN_PCK_VGA;
                frame_len = HIMAX_FRAME_LENGTH_VGA;
                break;
            case FRAMESIZE_QVGA:
                line_len = HIMAX_LINE_LEN_PCK_QVGA;
                frame_len = HIMAX_FRAME_LENGTH_QVGA;
                break;
            case FRAMESIZE_QQVGA:
                line_len = HIMAX_LINE_LEN_PCK_QQVGA;
                frame_len = HIMAX_FRAME_LENGTH_QQVGA;
                break;
            default:
                return -1;
        }

        ret |= get_vt_pix_clk(sensor, &vt_pix_clk);
        coarse_int = fast_roundf(exposure_us * (vt_pix_clk / 1000000.0f) / line_len);

        if (coarse_int < 2) {
            coarse_int = 2;
        } else if (coarse_int > (frame_len-4)) {
            coarse_int = frame_len-4;
        }

        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, AE_CTRL, (ae_ctrl & 0xFE));
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, INTEGRATION_H, coarse_int>>8);
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, INTEGRATION_L, coarse_int&0xff);
        ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, COMMAND_UPDATE, 0x01);
    }

    return ret;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    int ret = 0;
    uint32_t line_len;
    uint32_t coarse_int = 0;
    uint32_t vt_pix_clk = 0;
    switch (sensor->framesize) {
        case FRAMESIZE_VGA:
            line_len = HIMAX_LINE_LEN_PCK_VGA;
            break;
        case FRAMESIZE_QVGA:
            line_len = HIMAX_LINE_LEN_PCK_QVGA;
            break;
        case FRAMESIZE_QQVGA:
            line_len = HIMAX_LINE_LEN_PCK_QQVGA;
            break;
        default:
            return -1;
    }
    ret |= get_vt_pix_clk(sensor, &vt_pix_clk);
    ret |= omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, INTEGRATION_H, &((uint8_t*)&coarse_int)[1]);
    ret |= omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, INTEGRATION_L, &((uint8_t*)&coarse_int)[0]);
    *exposure_us = fast_roundf(coarse_int * line_len / (vt_pix_clk / 1000000.0f));
    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, IMG_ORIENTATION, &reg);
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, IMG_ORIENTATION, HIMAX_SET_HMIRROR(reg, enable)) ;
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, COMMAND_UPDATE, 0x01);
    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint8_t reg;
    int ret = omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, IMG_ORIENTATION, &reg);
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, IMG_ORIENTATION, HIMAX_SET_VMIRROR(reg, enable)) ;
    ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, COMMAND_UPDATE, 0x01);
    return ret;
}

static int ioctl(sensor_t *sensor, int request, va_list ap)
{
    int ret = 0;

    switch (request) {
        case IOCTL_HIMAX_OSC_ENABLE: {
            break;
        }

        case IOCTL_HIMAX_MD_ENABLE: {
            int ret = 0;
            uint8_t md_ctrl = 0;
            uint32_t enable = va_arg(ap, uint32_t) & 0x01;
            ret |= omv_i2c_readb2(&sensor->i2c_bus, sensor->slv_addr, MD_CTRL, &md_ctrl);
            ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MD_CTRL, (md_ctrl & 0xFE) | enable);
            break;
        }

        case IOCTL_HIMAX_MD_WINDOW: {
            int ret = 0;
            int32_t roi_w = 0;
            int32_t roi_h = 0;
            int32_t roi_max_h = 14;

            int32_t x1 = va_arg(ap, int32_t);
            int32_t y1 = va_arg(ap, int32_t);
            int32_t x2 = va_arg(ap, int32_t) + x1;
            int32_t y2 = va_arg(ap, int32_t) + y1;

            switch (sensor->framesize) {
                case FRAMESIZE_VGA:
                    roi_w = HIMAX_MD_ROI_VGA_W;
                    roi_h = HIMAX_MD_ROI_VGA_H;
                    roi_max_h = 14;
                    break;
                case FRAMESIZE_QVGA:
                    roi_w = HIMAX_MD_ROI_QVGA_W;
                    roi_h = HIMAX_MD_ROI_QVGA_H;
                    roi_max_h = 14;
                    break;
                case FRAMESIZE_QQVGA:
                    roi_w = HIMAX_MD_ROI_QQVGA_W;
                    roi_h = HIMAX_MD_ROI_QQVGA_H;
                    roi_max_h = 13;
                    break;
                default:
                    return -1;
            }

            x1 = MAX((x1 / roi_w - 1), 0);
            y1 = MAX((y1 / roi_h - 1), 0);
            x2 = MIN((x2 / roi_w) + !!(x2 % roi_w), 0xF);
            y2 = MIN((y2 / roi_h) + !!(y2 % roi_h), roi_max_h);
            ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, ROI_START_END_H, ((x2 & 0xF) << 4) |  (x1 & 0x0F));
            ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, ROI_START_END_V, ((y2 & 0xF) << 4) |  (y1 & 0x0F));
            break;
        }

        case IOCTL_HIMAX_MD_THRESHOLD: {
            uint32_t threshold = va_arg(ap, uint32_t) & 0x3F;
            ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MD_TH_STR_L, threshold);
            ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MD_TH_STR_H, threshold);
            ret |= omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, MD_LIGHT_COEF, threshold);
            break;
        }

        case IOCTL_HIMAX_MD_CLEAR: {
            ret = omv_i2c_writeb2(&sensor->i2c_bus, sensor->slv_addr, INT_CLEAR, (1 << 3));
            break;
        }

        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int hm0360_init(sensor_t *sensor)
{
    // Initialize sensor structure.
    sensor->reset               = reset;
    sensor->read_reg            = read_reg;
    sensor->write_reg           = write_reg;
    sensor->set_pixformat       = set_pixformat;
    sensor->set_framesize       = set_framesize;
    sensor->set_framerate       = set_framerate;
    sensor->set_brightness      = set_brightness;
    sensor->set_gainceiling     = set_gainceiling;
    sensor->set_colorbar        = set_colorbar;
    sensor->set_auto_gain       = set_auto_gain;
    sensor->get_gain_db         = get_gain_db;
    sensor->set_auto_exposure   = set_auto_exposure;
    sensor->get_exposure_us     = get_exposure_us;
    sensor->set_hmirror         = set_hmirror;
    sensor->set_vflip           = set_vflip;
    sensor->ioctl               = ioctl;

    // Set sensor flags
    sensor->hw_flags.vsync      = 0;
    sensor->hw_flags.hsync      = 0;
    sensor->hw_flags.pixck      = 0;
    sensor->hw_flags.fsync      = 0;
    sensor->hw_flags.jpege      = 0;
    sensor->hw_flags.gs_bpp     = 1;

    return 0;
}
#endif //(OMV_ENABLE_HM0360 == 1)
