/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * PixArt PAG7936 driver.
 */
#include "board_config.h"
#if (OMV_PAG7936_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "omv_i2c.h"
#include "omv_csi.h"
#include "py/mphal.h"

#define FRAME_TIME_20_16            (0x004E)
#define FRAME_TIME_15_8             (0x004D)
#define FRAME_TIME_7_0              (0x004C)
#define AE_START_DIV_4_X_7_0        (0x0482)
#define AE_START_DIV_4_X_8_7        (0x0483)
#define AE_START_DIV_4_Y            (0x0484)
#define AE_SIZE_DIV_4_X_7_0         (0x0485)
#define AE_SIZE_DIV_4_X_8_7         (0x0486)
#define AE_SIZE_DIV_4_Y             (0x0487)
#define AE_EXPO_MANUAL              (0x1400)
#define AE_EXPO_MANUAL_AE_MANUAL_EN (0x10)
#define AE_EXPO_MANUAL_AE_ENH       (0x1)
#define AE_FREQ_60                  (0x1404)
#define AE_YTAR8BIT                 (0x1406)
#define AE_LOCKRANGE_IN             (0x1407)
#define AE_LOCKRANGE_OUT_LB         (0x1408)
#define AE_LOCKRANGE_OUT_UB         (0x1409)
#define AE_STABLE_7_0               (0x145B)
#define AE_MINGAIN_7_0              (0x140A)
#define AE_MINGAIN_10_8             (0x140B)
#define AE_MAXGAIN_7_0              (0x140C)
#define AE_MAXGAIN_10_8             (0x140D)
#define AE_MINEXPO_7_0              (0x140E)
#define AE_MINEXPO_15_8             (0x140F)
#define AE_MINEXPO_17_16            (0x1410)
#define AE_MAXEXPO_7_0              (0x1412)
#define AE_MAXEXPO_15_8             (0x1413)
#define AE_MAXEXPO_17_16            (0x1414)
#define AE_GAIN_MANUAL_7_0          (0x1423)
#define AE_GAIN_MANUAL_10_8         (0x1424)
#define AE_EXP_LINE_NUM_7_0         (0x1450)
#define AE_EXP_LINE_NUM_15_8        (0x1451)
#define AE_EXP_LINE_NUM_17_16       (0x1452)
#define TOTAL_GAIN_7_0              (0x1453)
#define TOTAL_GAIN_10_8             (0x1454)
#define AE_EXPO_MANUAL_17_16        (0x1427)
#define AE_EXPO_MANUAL_15_8         (0x1426)
#define AE_EXPO_MANUAL_7_0          (0x1425)
#define SENSOR_UPDATE               (0x00EB)
#define SENSOR_UPDATE_FLAG          (0x80)
#define INTERFACE_POLARITY          (0x0EAF)
#define SENSOR_OPMODE               (0x0008)
#define SENSOR_OPMODE_RUN           (0x83)
#define SENSOR_OPMODE_SUSPEND       (0x86)
#define SENSOR_TRIGGER_FRAMENUM     (0x002E)
#define SENSOR_TRIGGER_EN           (0x002F)
#define SENSOR_TRIGGER_EN_FLAG      (0x01)
#define SENSOR_TRIGGER_MODE_SOFT    (0x00)
#define SENSOR_TRIGGER_MODE_GPIO0   (0x10)
#define SENSOR_TRIGGER_MODE_GPIO1   (0x20)
#define SENSOR_TRIGGER_MODE_GPIO2   (0x30)
#define SENSOR_TG_EN                (0x0030)
#define SENSOR_TG_EN_FLAG           (0x01)
#define SENSOR_TRIGGER_MODE         (0x0031)
#define SENSOR_TRIGGER_MODE_0       (0x02)
#define SENSOR_TRIGGER_MODE_1       (0x03)
#define SENSOR_SOFTWARE_TRIGGER     (0x00EA)
#define ISP_EN_H                    (0x0800)
#define ISP_EN_H_EN                 (0x01)
#define ISP_TEST_MODE               (0x0801)
#define DENOISE_EN                  (0x0882)
#define ISP_TEST_MODE_RAMP          (0x04)
#define R_RGB1_GRAY0                (0x0E08)
#define ISP_WOI_EN                  (0x0E10)
#define ISP_WOI_HSIZE_L             (0x0E11)
#define ISP_WOI_HSIZE_H             (0x0E12)
#define ISP_WOI_VSIZE_L             (0x0E13)
#define ISP_WOI_VSIZE_H             (0x0E14)
#define ISP_WOI_HOFFSET_L           (0x0E15)
#define ISP_WOI_HOFFSET_H           (0x0E16)
#define ISP_WOI_VOFFSET_L           (0x0E17)
#define ISP_WOI_VOFFSET_H           (0x0E18)
#define TG_MONO_SENSOR              (0x01BF)
#define TG_FLIP                     (0x01CE)
#define TG_FLIP_SET_VFLIP(r, x)     ((r & 0xFB) | ((x & 1) << 2))
#define TG_FLIP_SET_HFLIP(r, x)     ((r & 0xF7) | ((x & 1) << 3))
#define AVERAGE_MODE                (0x01C0)
#define ROW_AVERAGE_MODE            (0x0166)
#define COL_AVERAGE_MODE            (0x016F)
#define HSIZE_L                     (0x01C6)
#define HSIZE_H                     (0x01C7)
#define VSIZE_L                     (0x01C8)
#define VSIZE_H                     (0x01C9)
#define WOI_HSIZE_L                 (0x0221)
#define WOI_HSIZE_H                 (0x0222)
#define WOI_VSIZE_L                 (0x0223)
#define WOI_VSIZE_H                 (0x0224)
#define WOI_HSTART_L                (0x0225)
#define WOI_HSTART_H                (0x0226)
#define WOI_VSTART_L                (0x0227)
#define WOI_VSTART_H                (0x0228)

#define RGB_STAT_B_VS               (0x04C9)
#define RGB_STAT_GB_VS              (0x04CD)
#define RGB_STAT_R_VS               (0x04D1)
#define RGB_STAT_GR_VS              (0x04D5)

#define FT_CLK                      (1000000)
#define FPS_MAX                     (120)

#define FMAX_720P_120_LINEAR        (1041)
#define LT_T_RATIO                  (1000000 / FPS_MAX / FMAX_720P_120_LINEAR / 8)    // around ~1.0 with current setting
#define ConvertL2T(line)            (line * LT_T_RATIO)   // 1-Line time = 8usec = 1T, return T
#define ConvertT2L(t)               ((t + (LT_T_RATIO / 2)) / LT_T_RATIO) // 1-Line time = 8usec = 1T, return Lines

#define PAG7936_MIN_INT             (ConvertT2L(10)) // ExpLine min vlaue
#define PAG7936_MAX_INT             ((FMAX_720P_120_LINEAR - 11)) // ExpLine max vlaue
#define PAG7936_MIN_AGAIN           (1472)  // AGain min value, unit is 1/1024x
#define PAG7936_MAX_AGAIN           (16384) // AGain max value, unit is 1/1024x
#define PAG7936_MIN_AGAIN_REG       ((PAG7936_MIN_AGAIN) / 64)
#define PAG7936_MAX_AGAIN_REG       ((PAG7936_MAX_AGAIN) / 64)

#define PAG7936_GAIN_SCALE          (16)
#define PAG7936_GAIN_SCALE_F        ((PAG7936_GAIN_SCALE) * 1.0f)

#define PAG7936_EXP_MARGIN          (80)
#define PAG7936_EXP_MIN             (80)
#define PAG7936_EXP_DIV             (8)

#define PAG7936_FRAME_TIME(h, m, l) ((((h) & 0x1f) << 16) | ((m) << 8) | (l))
#define PAG7936_FRAME_TIME_H(r, ft) (((r) & 0xe0) | (((ft) >> 16) & 0x1f))
#define PAG7936_FRAME_TIME_M(ft)    (((ft) >> 8) & 0xff)
#define PAG7936_FRAME_TIME_L(ft)    ((ft) & 0xff)

#define PAG7936_GAIN(h, l)          ((((h) & 0x7) << 8) | (l))
#define PAG7936_GAIN_H(r, gain)     (((r) & 0xf8) | (((gain) >> 8) & 0x7))
#define PAG7936_GAIN_L(gain)        ((gain) & 0xff)

#define PAG7936_EXPOSURE(h, m, l)   (((((h) & 0x3) << 16) | ((m) << 8) | (l)) * 8)
#define PAG7936_EXPOSURE_H(r, exp)  (((r) & 0xfc) | (((exp) >> 16) & 0x3))
#define PAG7936_EXPOSURE_M(exp)     (((exp) >> 8) & 0xff)
#define PAG7936_EXPOSURE_L(exp)     ((exp) & 0xff)

#if OMV_PAG7936_MIPI_CSI2
#define PAG7936_WIDTH_ALIGN         (8)
#define PAG7936_QVGA_FPS_MAX        (470)
#define PAG7936_VGA_FPS_MAX         (240)
#define PAG7936_HD_FPS_MAX          (120)
#else
#define PAG7936_WIDTH_ALIGN         (4)
#define PAG7936_QVGA_FPS_MAX        (240)
#define PAG7936_VGA_FPS_MAX         (120)
#define PAG7936_HD_FPS_MAX          (60)
#endif

typedef struct {
    bool gain_auto;
    bool expo_auto;
    int framerate;
} pag7936_state_t;

static pag7936_state_t pag7936_state = {};

static const uint16_t default_regs[][2] = {
    #if OMV_PAG7936_MIPI_CSI2
    { 0x004C,   0x8D },
    { 0x004D,   0x20 },
    { 0x004E,   0x00 },
    { 0x004F,   0x00 },
    { 0x110A,   0x00 },
    { 0x0905,   0x60 },
    { 0x0978,   0x01 },
    { 0x0979,   0x66 },
    { 0x097C,   0x30 },
    { 0x097D,   0x02 },
    { 0x0985,   0x06 },
    { 0x0989,   0x10 },
    { 0x098B,   0x91 },
    { 0x098E,   0x06 },
    { 0x0850,   0x03 },
    { 0x0851,   0x20 },
    { 0x0852,   0x40 },
    { 0x0853,   0x40 },
    { 0x0854,   0x40 },
    { 0x0855,   0x60 },
    { 0x0856,   0x80 },
    { 0x0857,   0x20 },
    { 0x0858,   0x40 },
    { 0x0859,   0x40 },
    { 0x085A,   0x40 },
    { 0x085B,   0x60 },
    { 0x085C,   0x80 },
    { 0x085D,   0x0C },
    { 0x085E,   0x0C },
    { 0x085F,   0x0C },
    { 0x0860,   0x0C },
    { 0x0861,   0x0C },
    { 0x0862,   0x0C },
    { 0x094A,   0x43 },
    { 0x09A0,   0x08 },
    { 0x0032,   0x00 },
    { 0x0033,   0x00 },
    { 0x0764,   0x00 },
    { 0x0304,   0x30 },
    { 0x0305,   0x03 },
    { 0x0307,   0x10 },
    { 0x0308,   0x05 },
    { 0x0306,   0x04 },
    { 0x0311,   0x00 },
    { 0x030F,   0x01 },
    { 0x0168,   0x6D },
    { 0x0730,   0x78 },
    { 0x0724,   0x20 },
    { 0x0188,   0x14 },
    { 0x02A5,   0xEF },
    { 0x0186,   0x40 },
    { 0x0A1A,   0x20 },
    { 0x0A32,   0x2A },
    { 0x0A33,   0x2A },
    { 0x0A34,   0x0D },
    { 0x0A35,   0x0D },
    { 0x000B,   0x01 },
    #else
    { 0x010C,   0x23 },
    { 0x010D,   0x05 },
    { 0x007B,   0x8C },
    { 0x007C,   0x8C },
    { 0x007D,   0x8C },
    { 0x007E,   0x8C },
    { 0x007F,   0x8C },
    { 0x0EAF,   0x31 }, // Interface polarity (Not default)
    { 0x004C,   0x1A }, // frame time
    { 0x004D,   0x41 }, // ftime
    { 0x004E,   0x00 }, // ftime
    { 0x004F,   0x00 },
    { 0x110A,   0x00 },
    { 0x0905,   0x60 },
    { 0x0978,   0x01 },
    { 0x0979,   0x64 },
    { 0x097C,   0xE6 },
    { 0x097D,   0x00 },
    { 0x097E,   0x08 },
    { 0x097F,   0x02 },
    { 0x0985,   0x10 },
    { 0x098B,   0x90 },
    { 0x098E,   0x06 },
    { 0x0850,   0x03 },
    { 0x0851,   0x20 },
    { 0x0852,   0x40 },
    { 0x0853,   0x40 },
    { 0x0854,   0x40 },
    { 0x0855,   0x60 },
    { 0x0856,   0x80 },
    { 0x0857,   0x20 },
    { 0x0858,   0x40 },
    { 0x0859,   0x40 },
    { 0x085A,   0x40 },
    { 0x085B,   0x60 },
    { 0x085C,   0x80 },
    { 0x085D,   0x0C },
    { 0x085E,   0x0C },
    { 0x085F,   0x0C },
    { 0x0860,   0x0C },
    { 0x0861,   0x0C },
    { 0x0862,   0x0C },
    { 0x094A,   0x43 },
    { 0x09A0,   0x08 },
    { 0x0724,   0x10 },
    { 0x0110,   0x1B },
    { 0x0111,   0x03 },
    { 0x0114,   0xF4 },
    { 0x0186,   0x40 },
    { 0x0188,   0x14 },
    { 0x02A5,   0xEF },
    { 0x016B,   0x5D },
    { 0x0634,   0xE0 },
    { 0x0635,   0x01 },
    { 0x0646,   0x12 },
    { 0x0647,   0x02 },
    { 0x064E,   0x3A },
    { 0x064F,   0x02 },
    { 0x0654,   0x30 },
    { 0x0655,   0x02 },
    { 0x065A,   0x3F },
    { 0x065B,   0x02 },
    { 0x066A,   0x7B },
    { 0x066B,   0x02 },
    { 0x066E,   0xD2 },
    { 0x0683,   0x9E },
    { 0x0684,   0x02 },
    { 0x0687,   0xF5 },
    { 0x06B5,   0xA8 },
    { 0x06B6,   0x02 },
    { 0x06B9,   0xFF },
    { 0x06C3,   0xA3 },
    { 0x06C4,   0x02 },
    { 0x06C7,   0xFA },
    { 0x06D1,   0x12 },
    { 0x06D2,   0x02 },
    { 0x06D7,   0x76 },
    { 0x06D8,   0x02 },
    { 0x06DB,   0xCD },
    { 0x0A1A,   0x20 },
    { 0x0A32,   0x2A },
    { 0x0A33,   0x2A },
    { 0x0A34,   0x0D },
    { 0x0A35,   0x0D },
    { 0x000B,   0x02 },
    #endif
    { 0x1400,   0x01 }, // AE auto mode (bit4=0), bit0 preserved at default
    { 0x140C,   0x00 },
    { 0x140D,   0x01 }, // AE_MAXGAIN = 256 (16x, hardware max)
    { 0x0801,   0x00 }, // Disable ramp test pattern
    { 0x0810,   0x01 },
    { 0x0814,   0xB3 }, //R_center_rx[10:0]=691
    { 0x0815,   0x02 }, //R_center_rx[10:0]=691
    { 0x0816,   0xBB }, //R_center_ry[9:0]=443
    { 0x0817,   0x01 }, //R_center_ry[9:0]=443
    { 0x0818,   0xA9 }, //R_center_gx[10:0]=681
    { 0x0819,   0x02 }, //R_center_gx[10:0]=681
    { 0x081A,   0xBB }, //R_center_gy[9:0]=443
    { 0x081B,   0x01 }, //R_center_gy[9:0]=443
    { 0x081C,   0xB0 }, //R_center_bx[10:0]=688
    { 0x081D,   0x02 }, //R_center_bx[10:0]=688
    { 0x081E,   0xBD }, //R_center_by[9:0]=445
    { 0x081F,   0x01 }, //R_center_by[9:0]=445
    { 0x0820,   0x61 }, //R_LSC_RS[7:0]=97
    { 0x0821,   0x5B }, //R_LSC_GS[7:0]=91
    { 0x0822,   0x5F }, //R_LSC_BS[7:0]=95
    { 0x0823,   0x41 }, //R_LSC_RQ[7:0]=65
    { 0x0824,   0x50 }, //R_LSC_GQ[7:0]=80
    { 0x0825,   0x7F }, //R_LSC_BQ[7:0]=127
    { 0x0826,   0x07 }, //R_LSC_SftRS[3:0]=7
    { 0x0827,   0x07 }, //R_LSC_SftGS[3:0]=7
    { 0x0828,   0x07 }, //R_LSC_SftBS[3:0]=7
    { 0x0829,   0x09 }, //R_LSC_SftRQ[3:0]=9
    { 0x082A,   0x09 }, //R_LSC_SftGQ[3:0]=9
    { 0x082B,   0x09 }, //R_LSC_SftBQ[3:0]=9
    { 0x082E,   0xD8 }, //R_LSC_LMaxR2R[15:0]=39640
    { 0x082F,   0x9A }, //R_LSC_LMaxR2R[15:0]=39640
    { 0x0830,   0xD8 }, //R_LSC_LMaxR2G[15:0]=39640
    { 0x0831,   0x9A }, //R_LSC_LMaxR2G[15:0]=39640
    { 0x0832,   0xD8 }, //R_LSC_LMaxR2B[15:0]=39640
    { 0x0833,   0x9A }, //R_LSC_LMaxR2B[15:0]=39640

    // WOI
    { 0x0E10,   0x01 },
    { 0x0E11,   0x00 },
    { 0x0E12,   0x05 },
    { 0x0E13,   0x20 },
    { 0x0E14,   0x03 },
    { 0x0E15,   0x10 },
    { 0x0E16,   0x00 },
    { 0x0E17,   0x10 },
    { 0x0E18,   0x00 },

    { 0x00EB,   0x80 },
    { 0x0030,   0x01 },
    { 0x0008,   0x83 },
    { 0x0000,   0x00 },
};

static const uint16_t qvga_regs[][2] = {
    { TG_MONO_SENSOR,       0x00 },
    { AVERAGE_MODE,         0x02 },
    { ROW_AVERAGE_MODE,     0x00 },
    { COL_AVERAGE_MODE,     0x00 },
    { ISP_WOI_EN,           0x01 },
    { HSIZE_L,              0x44 },
    { HSIZE_H,              0x01 },
    { VSIZE_L,              0xCC },
    { VSIZE_H,              0x00 },
    { WOI_HSIZE_L,          0x44 },
    { WOI_HSIZE_H,          0x01 },
    { WOI_VSIZE_L,          0xCC },
    { WOI_VSIZE_H,          0x00 },
    { WOI_HSTART_L,         0x00 },
    { WOI_HSTART_H,         0x00 },
    { WOI_VSTART_L,         0x00 },
    { WOI_VSTART_H,         0x00 },
    { ISP_WOI_HSIZE_L,      0x40 }, //320
    { ISP_WOI_HSIZE_H,      0x01 }, //320
    { ISP_WOI_VSIZE_L,      0xC8 }, //200
    { ISP_WOI_VSIZE_H,      0x00 }, //200
    { ISP_WOI_HOFFSET_L,    0x02 }, //2
    { ISP_WOI_HOFFSET_H,    0x00 }, //2
    { ISP_WOI_VOFFSET_L,    0x02 }, //2
    { ISP_WOI_VOFFSET_H,    0x00 }, //2
    { DENOISE_EN,           0x00 }, //Denoise off
    { R_RGB1_GRAY0,         0x01 }, //color
    { SENSOR_UPDATE,        0x80 },
    { 0x0000,               0x00 },
};

static const uint16_t vga_regs[][2] = {
    { TG_MONO_SENSOR,       0x00 },
    { AVERAGE_MODE,         0x01 },
    { ROW_AVERAGE_MODE,     0x00 },
    { COL_AVERAGE_MODE,     0x00 },
    { ISP_WOI_EN,           0x01 },
    { HSIZE_L,              0x88 },
    { HSIZE_H,              0x02 },
    { VSIZE_L,              0x98 },
    { VSIZE_H,              0x01 },
    { WOI_HSIZE_L,          0x88 },
    { WOI_HSIZE_H,          0x02 },
    { WOI_VSIZE_L,          0x98 },
    { WOI_VSIZE_H,          0x01 },
    { WOI_HSTART_L,         0x00 },
    { WOI_HSTART_H,         0x00 },
    { WOI_VSTART_L,         0x00 },
    { WOI_VSTART_H,         0x00 },
    { ISP_WOI_HSIZE_L,      0x80 }, //640
    { ISP_WOI_HSIZE_H,      0x02 }, //640
    { ISP_WOI_VSIZE_L,      0x90 }, //400
    { ISP_WOI_VSIZE_H,      0x01 }, //400
    { ISP_WOI_HOFFSET_L,    0x04 }, //4
    { ISP_WOI_HOFFSET_H,    0x00 }, //4
    { ISP_WOI_VOFFSET_L,    0x04 }, //4
    { ISP_WOI_VOFFSET_H,    0x00 }, //4
    { DENOISE_EN,           0x03 }, //Denoise on
    { R_RGB1_GRAY0,         0x01 }, //color
    { SENSOR_UPDATE,        0x80 },
    { 0x0000,               0x00 },
};

static const uint16_t hd_regs[][2] = {
    { TG_MONO_SENSOR,       0x00 },
    { AVERAGE_MODE,         0x00 },
    { ROW_AVERAGE_MODE,     0x00 },
    { COL_AVERAGE_MODE,     0x00 },
    { ISP_WOI_EN,           0x01 },
    { HSIZE_L,              0x10 },
    { HSIZE_H,              0x05 },
    { VSIZE_L,              0x30 },
    { VSIZE_H,              0x03 },
    { WOI_HSIZE_L,          0x10 },
    { WOI_HSIZE_H,          0x05 },
    { WOI_VSIZE_L,          0x30 },
    { WOI_VSIZE_H,          0x03 },
    { WOI_HSTART_L,         0x00 },
    { WOI_HSTART_H,         0x00 },
    { WOI_VSTART_L,         0x00 },
    { WOI_VSTART_H,         0x00 },
    { ISP_WOI_HSIZE_L,      0x00 }, //1280
    { ISP_WOI_HSIZE_H,      0x05 }, //1280
    { ISP_WOI_VSIZE_L,      0x20 }, //800
    { ISP_WOI_VSIZE_H,      0x03 }, //800
    { ISP_WOI_HOFFSET_L,    0x08 }, //8
    { ISP_WOI_HOFFSET_H,    0x00 }, //8
    { ISP_WOI_VOFFSET_L,    0x08 }, //8
    { ISP_WOI_VOFFSET_H,    0x00 }, //8
    { DENOISE_EN,           0x03 }, //Denoise on
    { R_RGB1_GRAY0,         0x01 }, //color
    { SENSOR_UPDATE,        0x80 },
    { 0x0000,               0x00 },
};

// Apply AE/AG hardware mode based on current manual/auto state.
// gain: register value to write (already clamped), or -1 to freeze from sensor readback.
// expo: exposure in sensor line units (already clamped), or -1 to freeze from sensor readback.
static int ae_apply(omv_csi_t *csi, int gain, int expo) {
    pag7936_state_t *state = csi->priv;
    uint8_t reg;
    uint8_t tmp;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL, 2, &reg, 1);

    if (state->gain_auto && state->expo_auto) {
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL, 2, reg & ~AE_EXPO_MANUAL_AE_MANUAL_EN, 1);
    } else {
        if (gain < 0) {
            uint8_t gainh, gainl;
            ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, TOTAL_GAIN_10_8, 2, &gainh, 1);
            ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, TOTAL_GAIN_7_0, 2, &gainl, 1);
            gain = PAG7936_GAIN(gainh, gainl);
        }

        if (expo < 0) {
            uint8_t exph, expm, expl;
            ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_17_16, 2, &exph, 1);
            ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_15_8, 2, &expm, 1);
            ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_7_0, 2, &expl, 1);
            expo = PAG7936_EXPOSURE(exph, expm, expl) / PAG7936_EXP_DIV;
        }

        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_GAIN_MANUAL_10_8, 2, &tmp, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_GAIN_MANUAL_10_8, 2, PAG7936_GAIN_H(tmp, gain), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_GAIN_MANUAL_7_0, 2, PAG7936_GAIN_L(gain), 1);

        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL_17_16, 2, &tmp, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL_17_16, 2, PAG7936_EXPOSURE_H(tmp, expo), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL_15_8, 2, PAG7936_EXPOSURE_M(expo), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL_7_0, 2, PAG7936_EXPOSURE_L(expo), 1);

        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_EXPO_MANUAL, 2, reg | AE_EXPO_MANUAL_AE_MANUAL_EN, 1);
    }

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 2, SENSOR_UPDATE_FLAG, 1);
    return ret;
}

static int reset(omv_csi_t *csi) {
    int ret = 0;
    pag7936_state_t *state = csi->priv;
    state->gain_auto = true;
    state->expo_auto = true;
    state->framerate = PAG7936_HD_FPS_MAX;
    csi->gainceiling = OMV_CSI_GAINCEILING_16X;
    // Write default registers
    for (int i = 0; default_regs[i][0] && ret == 0; i++) {
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, default_regs[i][0], 2, default_regs[i][1], 1);
    }
    return ret;
}

static int sleep(omv_csi_t *csi, int enable) {
    int ret = omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TG_EN, 2, SENSOR_TG_EN_FLAG, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_OPMODE, 2,
                             enable ? SENSOR_OPMODE_SUSPEND : SENSOR_OPMODE_RUN, 1);
    return ret;
}

static int read_reg(omv_csi_t *csi, uint16_t reg) {
    uint8_t reg_data;
    if (omv_i2c_read_reg(csi->i2c, csi->slv_addr, reg, 2, &reg_data, 1) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg, uint16_t reg_data) {
    return omv_i2c_write_reg(csi->i2c, csi->slv_addr, reg, 2, reg_data, 1);
}

static int read_reg_seq(omv_csi_t *csi, uint16_t addr, size_t size, uint8_t *buf) {
    int ret = 0;
    for (size_t i = 0; i < size; i++) {
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, addr + i, 2, &buf[i], 1);
    }
    return ret;
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    switch (pixformat) {
        case PIXFORMAT_RGB565:
        case PIXFORMAT_BAYER:
        case PIXFORMAT_GRAYSCALE:
            return 0;
        default:
            return -1;
    }
}

// Pick the largest native sensor mode that meets the requested framerate.
static omv_csi_framesize_t get_framesize(omv_csi_t *csi, omv_csi_framesize_t target, int framerate) {
    #ifndef OMV_CSI_HW_SCALE_ENABLE
    return target;
    #endif

    uint32_t w = csi->resolution[target][0];
    uint32_t h = csi->resolution[target][1];

    if (w > csi->resolution[OMV_CSI_FRAMESIZE_VGA][0] ||
        h > csi->resolution[OMV_CSI_FRAMESIZE_VGA][1] ||
        framerate <= PAG7936_HD_FPS_MAX) {
        return OMV_CSI_FRAMESIZE_HD;
    }

    if (w > csi->resolution[OMV_CSI_FRAMESIZE_QVGA][0] ||
        h > csi->resolution[OMV_CSI_FRAMESIZE_QVGA][1] ||
        framerate <= PAG7936_VGA_FPS_MAX) {
        return OMV_CSI_FRAMESIZE_VGA;
    }

    return OMV_CSI_FRAMESIZE_QVGA;
}

static int configure(omv_csi_t *csi, omv_csi_framesize_t target, int framerate) {
    pag7936_state_t *state = csi->priv;
    int ret = 0;
    const uint16_t(*regs)[2];
    uint8_t reg;
    omv_csi_framesize_t framesize = get_framesize(csi, target, framerate);

    switch (framesize) {
        case OMV_CSI_FRAMESIZE_HD:
            regs = hd_regs;
            framerate = IM_MIN(framerate, PAG7936_HD_FPS_MAX);
            break;
        case OMV_CSI_FRAMESIZE_VGA:
            regs = vga_regs;
            framerate = IM_MIN(framerate, PAG7936_VGA_FPS_MAX);
            break;
        case OMV_CSI_FRAMESIZE_QVGA:
            regs = qvga_regs;
            framerate = IM_MIN(framerate, PAG7936_QVGA_FPS_MAX);
            break;
        default:
            return -1;
    }

    for (int i = 0; regs[i][0] && ret == 0; i++) {
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, regs[i][0], 2, regs[i][1], 1);
    }

    #ifdef OMV_CSI_HW_SCALE_ENABLE
    csi->src_w = csi->resolution[framesize][0];
    csi->src_h = csi->resolution[framesize][1];
    #endif

    int32_t frame_time = FT_CLK / framerate;

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, FRAME_TIME_20_16, 2, &reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, FRAME_TIME_20_16, 2, PAG7936_FRAME_TIME_H(reg, frame_time), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, FRAME_TIME_15_8, 2, PAG7936_FRAME_TIME_M(frame_time), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, FRAME_TIME_7_0, 2, PAG7936_FRAME_TIME_L(frame_time), 1);

    if (state->gain_auto && state->expo_auto) {
        // Full auto: clamp AE_MAXEXPO ceiling to the new frame time so the AE engine
        // cannot pick an exposure longer than the frame allows, then commit.
        uint8_t exph, expm, expl;
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_MAXEXPO_17_16, 2, &exph, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_MAXEXPO_15_8, 2, &expm, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_MAXEXPO_7_0, 2, &expl, 1);
        int32_t max_expo = IM_CLAMP(PAG7936_EXPOSURE(exph, expm, expl), PAG7936_EXP_MIN,
                                    frame_time - PAG7936_EXP_MARGIN) / PAG7936_EXP_DIV;
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXEXPO_17_16, 2, PAG7936_EXPOSURE_H(exph, max_expo), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXEXPO_15_8, 2, PAG7936_EXPOSURE_M(max_expo), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXEXPO_7_0, 2, PAG7936_EXPOSURE_L(max_expo), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 2, SENSOR_UPDATE_FLAG, 1);
    } else {
        // Read actual current exposure and clamp to new frame time.
        // ae_apply commits the frame time change and updated manual exposure together.
        uint8_t exph, expm, expl;
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_17_16, 2, &exph, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_15_8, 2, &expm, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_7_0, 2, &expl, 1);
        int expo = IM_CLAMP(PAG7936_EXPOSURE(exph, expm, expl), PAG7936_EXP_MIN,
                            frame_time - PAG7936_EXP_MARGIN) / PAG7936_EXP_DIV;
        ret |= ae_apply(csi, -1, expo);
    }

    return ret;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    pag7936_state_t *state = csi->priv;
    uint32_t w = csi->resolution[framesize][0];
    uint32_t h = csi->resolution[framesize][1];

    if (w > csi->resolution[OMV_CSI_FRAMESIZE_HD][0] ||
        h > csi->resolution[OMV_CSI_FRAMESIZE_HD][1]) {
        return -1;
    }

    #ifndef OMV_CSI_HW_SCALE_ENABLE
    if (framesize != OMV_CSI_FRAMESIZE_HD &&
        framesize != OMV_CSI_FRAMESIZE_VGA &&
        framesize != OMV_CSI_FRAMESIZE_QVGA) {
        return -1;
    }
    #endif

    return configure(csi, framesize, state->framerate);
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    pag7936_state_t *state = csi->priv;
    state->framerate = framerate;

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return 0;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    return configure(csi, csi->framesize, framerate);
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    int new_gainceiling = PAG7936_GAIN_SCALE << (gainceiling + 1);
    if (new_gainceiling > PAG7936_MAX_AGAIN_REG) {
        return -1;
    }

    uint8_t reg;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_MAXGAIN_10_8, 2, &reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXGAIN_10_8, 2, PAG7936_GAIN_H(reg, new_gainceiling), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXGAIN_7_0, 2, PAG7936_GAIN_L(new_gainceiling), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 2, SENSOR_UPDATE_FLAG, 1);
    return ret;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, ISP_TEST_MODE, 2, &reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, ISP_TEST_MODE, 2,
                             (reg & ~ISP_TEST_MODE_RAMP) | (enable ? ISP_TEST_MODE_RAMP : 0), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 2, SENSOR_UPDATE_FLAG, 1);
    return ret;
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    pag7936_state_t *state = csi->priv;
    state->gain_auto = enable;
    int ret = 0;
    int gain = -1;

    if (enable && !isnanf(gain_db_ceiling) && !isinff(gain_db_ceiling)) {
        int gain_ceiling = fast_roundf(expf((gain_db_ceiling / 20.0f) * M_LN10) * PAG7936_GAIN_SCALE_F);
        gain_ceiling = IM_CLAMP(gain_ceiling, PAG7936_MIN_AGAIN_REG, PAG7936_MAX_AGAIN_REG);
        uint8_t reg;
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_MAXGAIN_10_8, 2, &reg, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXGAIN_10_8, 2, PAG7936_GAIN_H(reg, gain_ceiling), 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, AE_MAXGAIN_7_0, 2, PAG7936_GAIN_L(gain_ceiling), 1);
    }

    if (!enable && !isnanf(gain_db) && !isinff(gain_db)) {
        gain = fast_roundf(expf((gain_db / 20.0f) * M_LN10) * PAG7936_GAIN_SCALE_F);
        gain = IM_CLAMP(gain, PAG7936_MIN_AGAIN_REG, PAG7936_MAX_AGAIN_REG);
    }

    ret |= ae_apply(csi, gain, -1);
    return ret;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    uint8_t gainh, gainl;
    int ret = 0;

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, TOTAL_GAIN_10_8, 2, &gainh, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, TOTAL_GAIN_7_0, 2, &gainl, 1);

    *gain_db = 20.0f * log10f(PAG7936_GAIN(gainh, gainl) / PAG7936_GAIN_SCALE_F);

    return ret;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    pag7936_state_t *state = csi->priv;
    state->expo_auto = enable;
    int ret = 0;
    int expo = -1;

    if (!enable && exposure_us >= 0) {
        uint8_t ft_h, ft_m, ft_l;
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, FRAME_TIME_20_16, 2, &ft_h, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, FRAME_TIME_15_8, 2, &ft_m, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, FRAME_TIME_7_0, 2, &ft_l, 1);
        int32_t frame_time = PAG7936_FRAME_TIME(ft_h, ft_m, ft_l);
        expo = IM_CLAMP(exposure_us, PAG7936_EXP_MIN, frame_time - PAG7936_EXP_MARGIN) / PAG7936_EXP_DIV;
    }

    ret |= ae_apply(csi, -1, expo);
    return ret;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    uint8_t exposure_us_17_16, exposure_us_15_8, exposure_us_7_0;
    int ret = 0;

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_17_16, 2, &exposure_us_17_16, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_15_8, 2, &exposure_us_15_8, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, AE_EXP_LINE_NUM_7_0, 2, &exposure_us_7_0, 1);

    *exposure_us = PAG7936_EXPOSURE(exposure_us_17_16, exposure_us_15_8, exposure_us_7_0);

    return ret;
}

static int set_auto_whitebal(omv_csi_t *csi, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    csi->stats_enabled = enable;
    return 0;
}

static int get_rgb_gain_db(omv_csi_t *csi, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    uint32_t r, g, b;
    omv_csi_get_stats(csi, &r, &g, &b);

    *r_gain_db = 20.0f * log10f(IM_DIV((float) g, r));
    *g_gain_db = 0.0f;
    *b_gain_db = 20.0f * log10f(IM_DIV((float) g, b));
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, TG_FLIP, 2, &reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, TG_FLIP, 2, TG_FLIP_SET_HFLIP(reg, enable), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 2, SENSOR_UPDATE_FLAG, 1);
    return ret;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, TG_FLIP, 2, &reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, TG_FLIP, 2, TG_FLIP_SET_VFLIP(reg, enable), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 2, SENSOR_UPDATE_FLAG, 1);
    return ret;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;

    (void) read_reg_seq;

    switch (request) {
        case OMV_CSI_IOCTL_GET_RGB_STATS: {
            uint32_t rgb_stats[4];
            uint8_t buf[] = {(RGB_STAT_B_VS >> 8), RGB_STAT_B_VS & 0xFF };
            ret |= omv_i2c_write(csi->i2c, csi->slv_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
            ret |= omv_i2c_read(csi->i2c, csi->slv_addr, (uint8_t *) rgb_stats, sizeof(rgb_stats), 0);

            *va_arg(ap, uint32_t *) = rgb_stats[1];
            *va_arg(ap, uint32_t *) = rgb_stats[2];
            *va_arg(ap, uint32_t *) = rgb_stats[0];
            *va_arg(ap, uint32_t *) = rgb_stats[3];
            break;
        }
        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE: {
            int enable = va_arg(ap, int);
            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_OPMODE, 2, SENSOR_OPMODE_SUSPEND, 1);

            if (enable) {
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TG_EN, 2, 0, 1);
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TRIGGER_EN, 2, SENSOR_TRIGGER_EN_FLAG, 1);
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TRIGGER_MODE, 2, SENSOR_TRIGGER_MODE_1, 1);
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TRIGGER_EN, 2,
                                         SENSOR_TRIGGER_EN_FLAG | SENSOR_TRIGGER_MODE_GPIO0, 1);
            } else {
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TG_EN, 2, SENSOR_TG_EN_FLAG, 1);
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TRIGGER_EN, 2, 0, 1);
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_TRIGGER_MODE, 2, 0, 1);
            }

            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_OPMODE, 2, SENSOR_OPMODE_RUN, 1);

            // Skip past the first corrupt frames...
            if (!csi->disable_delays) {
                mp_hal_delay_ms(100);
            }
            break;
        }
        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE: {
            int *enable = va_arg(ap, int *);
            uint8_t reg;
            ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, SENSOR_TG_EN, 2, &reg, 1);
            if (ret >= 0) {
                *enable = (reg & SENSOR_TG_EN_FLAG) ? 0 : 1;
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

int pag7936_init(omv_csi_t *csi) {
    csi->priv = &pag7936_state;
    // Initialize csi flags.
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->mono_bpp = 1;
    csi->raw_output = 1;
    csi->cfa_format = SUBFORMAT_ID_BGGR;
    #if OMV_PAG7936_MIPI_CSI2
    csi->mipi_if = 1;
    csi->mipi_brate = 800;
    #endif

    // Initialize csi ops.
    csi->reset = reset;
    csi->sleep = sleep;
    csi->ioctl = ioctl;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_framerate = set_framerate;
    csi->set_gainceiling = set_gainceiling;
    csi->set_colorbar = set_colorbar;
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_auto_whitebal = set_auto_whitebal;
    csi->get_rgb_gain_db = get_rgb_gain_db;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;

    // Override standard resolutions
    csi->resolution[OMV_CSI_FRAMESIZE_HD][0] = 1280;
    csi->resolution[OMV_CSI_FRAMESIZE_HD][1] = 800;

    csi->resolution[OMV_CSI_FRAMESIZE_VGA][0] = 640;
    csi->resolution[OMV_CSI_FRAMESIZE_VGA][1] = 400;

    csi->resolution[OMV_CSI_FRAMESIZE_QVGA][0] = 320;
    csi->resolution[OMV_CSI_FRAMESIZE_QVGA][1] = 200;
    return 0;
}
#endif // (OMV_PAG7936_ENABLE == 1)
