/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9M114 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_ENABLE_MT9M114 == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cambus.h"
#include "sensor.h"
#include "mt9m114.h"
#include "mt9m114_regs.h"
#include "py/mphal.h"

#define DUMMY_LINES             8
#define DUMMY_COLUMNS           8

#define SENSOR_WIDTH            1296
#define SENSOR_HEIGHT           976

#define ACTIVE_SENSOR_WIDTH     (SENSOR_WIDTH - (2 * DUMMY_COLUMNS))
#define ACTIVE_SENSOR_HEIGHT    (SENSOR_HEIGHT - (2 * DUMMY_LINES))

#define DUMMY_WIDTH_BUFFER      8
#define DUMMY_HEIGHT_BUFFER     8

static int16_t readout_x = 0;
static int16_t readout_y = 0;

static uint16_t readout_w = ACTIVE_SENSOR_WIDTH;
static uint16_t readout_h = ACTIVE_SENSOR_HEIGHT;

static const uint16_t default_regs[][2] = {
    // Sensor Optimization
    {0x316A, 0x8270},
    {0x316C, 0x8270},
    {0x3ED0, 0x2305},
    {0x3ED2, 0x77CF},
    {0x316E, 0x8202},
    {0x3180, 0x87FF},
    {0x30D4, 0x6080},
    {0xA802, 0x0008},

    // Errata 1 (Column Bands)
    {0x3E14, 0xFF39}
};

static const uint16_t patch_0202[] = {
    0x70cf, 0xffff, 0xc5d4, 0x903a, 0x2144, 0x0c00, 0x2186, 0x0ff3, 0xb844, 0xb948, 0xe082, 0x20cc,
    0x80e2, 0x21cc, 0x80a2, 0x21cc, 0x80e2, 0xf404, 0xd801, 0xf003, 0xd800, 0x7ee0, 0xc0f1, 0x08ba,
    0x0600, 0xc1a1, 0x76cf, 0xffff, 0xc130, 0x6e04, 0xc040, 0x71cf, 0xffff, 0xc790, 0x8103, 0x77cf,
    0xffff, 0xc7c0, 0xe001, 0xa103, 0xd800, 0x0c6a, 0x04e0, 0xb89e, 0x7508, 0x8e1c, 0x0809, 0x0191,
    0xd801, 0xae1d, 0xe580, 0x20ca, 0x0022, 0x20cf, 0x0522, 0x0c5c, 0x04e2, 0x21ca, 0x0062, 0xe580,
    0xd901, 0x79c0, 0xd800, 0x0be6, 0x04e0, 0xb89e, 0x70cf, 0xffff, 0xc8d4, 0x9002, 0x0857, 0x025e,
    0xffdc, 0xe080, 0x25cc, 0x9022, 0xf225, 0x1700, 0x108a, 0x73cf, 0xff00, 0x3174, 0x9307, 0x2a04,
    0x103e, 0x9328, 0x2942, 0x7140, 0x2a04, 0x107e, 0x9349, 0x2942, 0x7141, 0x2a04, 0x10be, 0x934a,
    0x2942, 0x714b, 0x2a04, 0x10be, 0x130c, 0x010a, 0x2942, 0x7142, 0x2250, 0x13ca, 0x1b0c, 0x0284,
    0xb307, 0xb328, 0x1b12, 0x02c4, 0xb34a, 0xed88, 0x71cf, 0xff00, 0x3174, 0x9106, 0xb88f, 0xb106,
    0x210a, 0x8340, 0xc000, 0x21ca, 0x0062, 0x20f0, 0x0040, 0x0b02, 0x0320, 0xd901, 0x07f1, 0x05e0,
    0xc0a1, 0x78e0, 0xc0f1, 0x71cf, 0xffff, 0xc7c0, 0xd840, 0xa900, 0x71cf, 0xffff, 0xd02c, 0xd81e,
    0x0a5a, 0x04e0, 0xda00, 0xd800, 0xc0d1, 0x7ee0
};

static const uint16_t patch_0302[] = {
    0x70cf, 0xffff, 0xc5d4, 0x903a, 0x2144, 0x0c00, 0x2186, 0x0ff3, 0xb844, 0x262f, 0xf008, 0xb948,
    0x21cc, 0x8021, 0xd801, 0xf203, 0xd800, 0x7ee0, 0xc0f1, 0x71cf, 0xffff, 0xc610, 0x910e, 0x208c,
    0x8014, 0xf418, 0x910f, 0x208c, 0x800f, 0xf414, 0x9116, 0x208c, 0x800a, 0xf410, 0x9117, 0x208c,
    0x8807, 0xf40c, 0x9118, 0x2086, 0x0ff3, 0xb848, 0x080d, 0x0090, 0xffea, 0xe081, 0xd801, 0xf203,
    0xd800, 0xc0d1, 0x7ee0, 0x78e0, 0xc0f1, 0x71cf, 0xffff, 0xc610, 0x910e, 0x208c, 0x800a, 0xf418,
    0x910f, 0x208c, 0x8807, 0xf414, 0x9116, 0x208c, 0x800a, 0xf410, 0x9117, 0x208c, 0x8807, 0xf40c,
    0x9118, 0x2086, 0x0ff3, 0xb848, 0x080d, 0x0090, 0xffd9, 0xe080, 0xd801, 0xf203, 0xd800, 0xf1df,
    0x9040, 0x71cf, 0xffff, 0xc5d4, 0xb15a, 0x9041, 0x73cf, 0xffff, 0xc7d0, 0xb140, 0x9042, 0xb141,
    0x9043, 0xb142, 0x9044, 0xb143, 0x9045, 0xb147, 0x9046, 0xb148, 0x9047, 0xb14b, 0x9048, 0xb14c,
    0x9049, 0x1958, 0x0084, 0x904a, 0x195a, 0x0084, 0x8856, 0x1b36, 0x8082, 0x8857, 0x1b37, 0x8082,
    0x904c, 0x19a7, 0x009c, 0x881a, 0x7fe0, 0x1b54, 0x8002, 0x78e0, 0x71cf, 0xffff, 0xc350, 0xd828,
    0xa90b, 0x8100, 0x01c5, 0x0320, 0xd900, 0x78e0, 0x220a, 0x1f80, 0xffff, 0xd4e0, 0xc0f1, 0x0811,
    0x0051, 0x2240, 0x1200, 0xffe1, 0xd801, 0xf006, 0x2240, 0x1900, 0xffde, 0xd802, 0x1a05, 0x1002,
    0xfff2, 0xf195, 0xc0f1, 0x0e7e, 0x05c0, 0x75cf, 0xffff, 0xc84c, 0x9502, 0x77cf, 0xffff, 0xc344,
    0x2044, 0x008e, 0xb8a1, 0x0926, 0x03e0, 0xb502, 0x9502, 0x952e, 0x7e05, 0xb5c2, 0x70cf, 0xffff,
    0xc610, 0x099a, 0x04a0, 0xb026, 0x0e02, 0x0560, 0xde00, 0x0a12, 0x0320, 0xb7c4, 0x0b36, 0x03a0,
    0x70c9, 0x9502, 0x7608, 0xb8a8, 0xb502, 0x70cf, 0x0000, 0x5536, 0x7860, 0x2686, 0x1ffb, 0x9502,
    0x78c5, 0x0631, 0x05e0, 0xb502, 0x72cf, 0xffff, 0xc5d4, 0x923a, 0x73cf, 0xffff, 0xc7d0, 0xb020,
    0x9220, 0xb021, 0x9221, 0xb022, 0x9222, 0xb023, 0x9223, 0xb024, 0x9227, 0xb025, 0x9228, 0xb026,
    0x922b, 0xb027, 0x922c, 0xb028, 0x1258, 0x0101, 0xb029, 0x125a, 0x0101, 0xb02a, 0x1336, 0x8081,
    0xa836, 0x1337, 0x8081, 0xa837, 0x12a7, 0x0701, 0xb02c, 0x1354, 0x8081, 0x7fe0, 0xa83a, 0x78e0,
    0xc0f1, 0x0dc2, 0x05c0, 0x7608, 0x09bb, 0x0010, 0x75cf, 0xffff, 0xd4e0, 0x8d21, 0x8d00, 0x2153,
    0x0003, 0xb8c0, 0x8d45, 0x0b23, 0x0000, 0xea8f, 0x0915, 0x001e, 0xff81, 0xe808, 0x2540, 0x1900,
    0xffde, 0x8d00, 0xb880, 0xf004, 0x8d00, 0xb8a0, 0xad00, 0x8d05, 0xe081, 0x20cc, 0x80a2, 0xdf00,
    0xf40a, 0x71cf, 0xffff, 0xc84c, 0x9102, 0x7708, 0xb8a6, 0x2786, 0x1ffe, 0xb102, 0x0b42, 0x0180,
    0x0e3e, 0x0180, 0x0f4a, 0x0160, 0x70c9, 0x8d05, 0xe081, 0x20cc, 0x80a2, 0xf429, 0x76cf, 0xffff,
    0xc84c, 0x082d, 0x0051, 0x70cf, 0xffff, 0xc90c, 0x8805, 0x09b6, 0x0360, 0xd908, 0x2099, 0x0802,
    0x9634, 0xb503, 0x7902, 0x1523, 0x1080, 0xb634, 0xe001, 0x1d23, 0x1002, 0xf00b, 0x9634, 0x9503,
    0x6038, 0xb614, 0x153f, 0x1080, 0xe001, 0x1d3f, 0x1002, 0xffa4, 0x9602, 0x7f05, 0xd800, 0xb6e2,
    0xad05, 0x0511, 0x05e0, 0xd800, 0xc0f1, 0x0cfe, 0x05c0, 0x0a96, 0x05a0, 0x7608, 0x0c22, 0x0240,
    0xe080, 0x20ca, 0x0f82, 0x0000, 0x190b, 0x0c60, 0x05a2, 0x21ca, 0x0022, 0x0c56, 0x0240, 0xe806,
    0x0e0e, 0x0220, 0x70c9, 0xf048, 0x0896, 0x0440, 0x0e96, 0x0400, 0x0966, 0x0380, 0x75cf, 0xffff,
    0xd4e0, 0x8d00, 0x084d, 0x001e, 0xff47, 0x080d, 0x0050, 0xff57, 0x0841, 0x0051, 0x8d04, 0x9521,
    0xe064, 0x790c, 0x702f, 0x0ce2, 0x05e0, 0xd964, 0x72cf, 0xffff, 0xc700, 0x9235, 0x0811, 0x0043,
    0xff3d, 0x080d, 0x0051, 0xd801, 0xff77, 0xf025, 0x9501, 0x9235, 0x0911, 0x0003, 0xff49, 0x080d,
    0x0051, 0xd800, 0xff72, 0xf01b, 0x0886, 0x03e0, 0xd801, 0x0ef6, 0x03c0, 0x0f52, 0x0340, 0x0dba,
    0x0200, 0x0af6, 0x0440, 0x0c22, 0x0400, 0x0d72, 0x0440, 0x0dc2, 0x0200, 0x0972, 0x0440, 0x0d3a,
    0x0220, 0xd820, 0x0bfa, 0x0260, 0x70c9, 0x0451, 0x05c0, 0x78e0, 0xd900, 0xf00a, 0x70cf, 0xffff,
    0xd520, 0x7835, 0x8041, 0x8000, 0xe102, 0xa040, 0x09f1, 0x8114, 0x71cf, 0xffff, 0xd4e0, 0x70cf,
    0xffff, 0xc594, 0xb03a, 0x7fe0, 0xd800, 0x0000, 0x0000, 0x0500, 0x0500, 0x0200, 0x0330, 0x0000,
    0x0000, 0x03cd, 0x050d, 0x01c5, 0x03b3, 0x00e0, 0x01e3, 0x0280, 0x01e0, 0x0109, 0x0080, 0x0500,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0xffff, 0xc9b4, 0xffff, 0xd324, 0xffff, 0xca34, 0xffff, 0xd3ec
};

static const uint16_t patch_0402[] = {
    0xc0f1, 0x0bba, 0x05c0, 0xc1a2, 0x75cf, 0xffff, 0xc7c0, 0xde00, 0x1c05, 0x3382, 0xc661, 0x08c3,
    0x0040, 0xe1d1, 0xf217, 0xe1d4, 0xf45b, 0x8d01, 0x08b3, 0x001e, 0x1c00, 0x3f80, 0x000c, 0x3500,
    0x718b, 0x0fbe, 0x04e0, 0x70c9, 0x0806, 0x0520, 0x70c9, 0x9525, 0x70cf, 0xff00, 0x0000, 0xb039,
    0xf047, 0x8d01, 0x0827, 0x00df, 0x8d02, 0x081d, 0x001e, 0x8d02, 0xad01, 0x8d01, 0xb8a3, 0xad01,
    0x70cf, 0xff00, 0x0000, 0x9019, 0xb505, 0xf005, 0x8d01, 0xb8a0, 0xad01, 0x8d01, 0x0811, 0x00de,
    0x8d01, 0xb8a3, 0xad01, 0x0d02, 0x04a0, 0xd801, 0x8d01, 0x0853, 0x001e, 0x8501, 0x0b5e, 0x05e0,
    0x218a, 0x041f, 0x9524, 0x2905, 0x003e, 0x1c00, 0x3e40, 0x718b, 0x0f4e, 0x04e0, 0xd800, 0x0f9a,
    0x04e0, 0xd800, 0xd800, 0x0fb6, 0x04e0, 0xd901, 0x8d01, 0x0823, 0x005f, 0x208a, 0x001c, 0x0be2,
    0x04e0, 0xd900, 0xd80d, 0xb80a, 0x0bd6, 0x04e0, 0xd900, 0xd890, 0x0bea, 0x04e0, 0xd901, 0xd800,
    0x02f5, 0x05e0, 0xc0a2, 0x78e0, 0xc0f1, 0x73cf, 0xffff, 0xc7c0, 0x8b41, 0x0a0d, 0x00de, 0x8b41,
    0x0a0f, 0x005e, 0x72cf, 0x0000, 0x403e, 0x7a40, 0xf002, 0xd800, 0xc0d1, 0x7ee0, 0xc0f1, 0xc5e1,
    0x75cf, 0xffff, 0xc7c0, 0xd900, 0xf009, 0x70cf, 0xffff, 0xd740, 0x7835, 0x8041, 0x8000, 0xe102,
    0xa040, 0x09f3, 0x8094, 0x71cf, 0xffff, 0xd6d0, 0xd803, 0x0d1e, 0x04a0, 0xda00, 0x70cf, 0xffff,
    0xd530, 0x0c22, 0x0300, 0xe887, 0x70cf, 0xffff, 0xd6e8, 0x094a, 0x0240, 0x218a, 0x0a0f, 0xb524,
    0xd904, 0xad23, 0x1d04, 0x1f80, 0x016e, 0x3600, 0x0279, 0x05c0, 0xc0f1, 0x71cf, 0xffff, 0xc350,
    0x1109, 0x00c0, 0x73cf, 0xffff, 0xc164, 0xe0d2, 0x72cf, 0xffff, 0xc7c0, 0xf412, 0x8b08, 0x0821,
    0x03d1, 0x8a01, 0x0819, 0x00df, 0xd854, 0xa90b, 0xd800, 0xa908, 0x8a21, 0xb983, 0xaa21, 0xaa0c,
    0x0be6, 0x0480, 0xf1b2, 0x78e0, 0xc0f1, 0x70cf, 0xffff, 0xc7c0, 0x8801, 0xb8e0, 0x0fb4, 0xffc2,
    0x0e9a, 0x0240, 0xf1a6, 0x78e0, 0xc0f1, 0x71cf, 0xffff, 0xc7c0, 0x8961, 0x72cf, 0xffff, 0xc350,
    0x0b45, 0x00de, 0x0841, 0x0811, 0x890c, 0xe001, 0xa90c, 0x896c, 0x8903, 0x0b0d, 0x0003, 0x8902,
    0x082d, 0x009e, 0x8902, 0x0813, 0x009e, 0x8902, 0xb8a2, 0xa902, 0x8901, 0xb882, 0xf003, 0x8901,
    0xb8a2, 0xa901, 0xd850, 0xaa0b, 0xd900, 0xaa2a, 0xaa28, 0x0cca, 0x02e0, 0x8200, 0xd800, 0xf17a,
    0xffff, 0xcad0, 0xffff, 0xd610
};

static const uint16_t patch_0502[] = {
    0x72cf, 0xffff, 0xc7c0, 0x0827, 0x0040, 0xe1d1, 0xf40f, 0x8a01, 0x081b, 0x00df, 0x70cf, 0xffff,
    0xc644, 0x8808, 0xb8e4, 0x8a0e, 0x20cf, 0x0062, 0x20d0, 0x0061, 0xaa0e, 0x7fe0, 0xd800, 0x78e0,
    0xc0f1, 0x096e, 0x05c0, 0x75cf, 0xffff, 0xc7c0, 0x70cf, 0x0000, 0x5536, 0x7860, 0xc1a1, 0x95ca,
    0x77cf, 0xffff, 0xc8d4, 0x9f1c, 0x0827, 0x0012, 0xd908, 0x0d56, 0x0320, 0xda05, 0x9f3c, 0x0913,
    0x0f82, 0x0000, 0x0b00, 0xb82a, 0x7e0c, 0x762f, 0xf00f, 0x7e0c, 0x2941, 0x728e, 0xf00b, 0x7813,
    0xd908, 0x0d32, 0x0320, 0xda05, 0x7108, 0x0952, 0x05e0, 0x70c9, 0x7608, 0x8d0f, 0x0825, 0x001e,
    0x70cf, 0xffff, 0xc82f, 0x0d1e, 0x0400, 0x2805, 0x03be, 0x70cf, 0xffff, 0xc914, 0x9033, 0xb924,
    0x092a, 0x05e0, 0x702f, 0x7608, 0x8d0e, 0x0883, 0x005e, 0x8d01, 0x087b, 0x00de, 0x8d0e, 0x77cf,
    0xffff, 0xc84c, 0xb8a0, 0xad0e, 0x8f27, 0x8f49, 0x2202, 0x8040, 0x0008, 0x0003, 0x2102, 0x0080,
    0x780e, 0xc040, 0x8f08, 0x782c, 0x702f, 0xe063, 0x08fe, 0x05e0, 0xd964, 0x9720, 0x780d, 0xb9c1,
    0x2142, 0x8002, 0x22ca, 0x0062, 0xc300, 0x7259, 0x0817, 0x00e3, 0xd900, 0x70cf, 0xffff, 0xd8ba,
    0x8800, 0xe080, 0x22cc, 0x9022, 0xf202, 0xd901, 0x262f, 0xf047, 0xf208, 0x8d0e, 0xb880, 0xad0e,
    0xa5c4, 0x8d02, 0xb882, 0xad02, 0x70cf, 0xffff, 0xd8ba, 0xa840, 0xf002, 0xa5c4, 0x0079, 0x05e0,
    0xc0a1, 0x78e0, 0xc0f1, 0xc5e1, 0x75cf, 0xffff, 0xc7c0, 0xd900, 0xf009, 0x70cf, 0xffff, 0xd8bc,
    0x7835, 0x8041, 0x8000, 0xe102, 0xa040, 0x09f3, 0x8094, 0x70cf, 0xffff, 0xd748, 0x09de, 0x0300,
    0x218a, 0x0014, 0xb52a, 0x0051, 0x05c0, 0x0000, 0xffff, 0xcb54, 0xffff, 0xd778
};

static const uint16_t awb_ccm[] = {
    0x0267, 0xFF1A, 0xFFB3, 0xFF80, 0x0166, 0x0003, 0xFF9A, 0xFEB4, 0x024D, 0x01BF, 0xFF01, 0xFFF3,
    0xFF75, 0x0198, 0xFFFD, 0xFF9A, 0xFEE7, 0x02A8, 0x01D9, 0xFF26, 0xFFF3, 0xFFB3, 0x0132, 0xFFE8,
    0xFFDA, 0xFECD, 0x02C2
};

static const uint16_t awb_weights[] = {
    0x0000, 0x0000, 0x0000, 0xE724, 0x1583, 0x2045, 0x03FF, 0x007C
};

static const uint16_t cpipe_regs_8_bit_a[] = {
    0xC92A, // CAM_LL_START_SATURATION
    0xC92B, // CAM_LL_END_SATURATION
    0xC92C, // CAM_LL_START_DESATURATION
    0xC92D, // CAM_LL_END_DESATURATION
    0xC92E, // CAM_LL_START_DEMOSAIC
    0xC92F, // CAM_LL_START_AP_GAIN
    0xC930, // CAM_LL_START_AP_THRESH
    0xC931, // CAM_LL_STOP_DEMOSAIC
    0xC932, // CAM_LL_STOP_AP_GAIN
    0xC933, // CAM_LL_STOP_AP_THRESH
    0xC934, // CAM_LL_START_NR_RED
    0xC935, // CAM_LL_START_NR_GREEN
    0xC936, // CAM_LL_START_NR_BLUE
    0xC937, // CAM_LL_START_NR_THRESH
    0xC938, // CAM_LL_STOP_NR_RED
    0xC939, // CAM_LL_STOP_NR_GREEN
    0xC93A, // CAM_LL_STOP_NR_BLUE
    0xC93B, // CAM_LL_STOP_NR_THRESH
    0xC942, // CAM_LL_START_CONTRAST_GRADIENT
    0xC943, // CAM_LL_STOP_CONTRAST_GRADIENT
    0xC944, // CAM_LL_START_CONTRAST_LUMA_PERCENTAGE
    0xC945, // CAM_LL_STOP_CONTRAST_LUMA_PERCENTAGE
    0xC950, // CAM_LL_CLUSTER_DC_GATE_PERCENTAGE
    0xC951, // CAM_LL_SUMMING_SENSITIVITY_FACTOR
    0xC87B, // CAM_AET_TARGET_AVERAGE_LUMA_DARK
    0xC878, // CAM_AET_AEMODE
    0xB42A, // CCM_DELTA_GAIN
    0xA80A, // AE_TRACK_AE_TRACKING_DAMPENING_SPEED
};

static const uint16_t cpipe_regs_8_bit_d[] = {
    0x80, // CAM_LL_START_SATURATION
    0x4B, // CAM_LL_END_SATURATION
    0x00, // CAM_LL_START_DESATURATION
    0xFF, // CAM_LL_END_DESATURATION
    0x3C, // CAM_LL_START_DEMOSAIC
    0x02, // CAM_LL_START_AP_GAIN
    0x06, // CAM_LL_START_AP_THRESH
    0x64, // CAM_LL_STOP_DEMOSAIC
    0x01, // CAM_LL_STOP_AP_GAIN
    0x0C, // CAM_LL_STOP_AP_THRESH
    0x3C, // CAM_LL_START_NR_RED
    0x3C, // CAM_LL_START_NR_GREEN
    0x3C, // CAM_LL_START_NR_BLUE
    0x0F, // CAM_LL_START_NR_THRESH
    0x64, // CAM_LL_STOP_NR_RED
    0x64, // CAM_LL_STOP_NR_GREEN
    0x64, // CAM_LL_STOP_NR_BLUE
    0x32, // CAM_LL_STOP_NR_THRESH
    0x38, // CAM_LL_START_CONTRAST_GRADIENT
    0x30, // CAM_LL_STOP_CONTRAST_GRADIENT
    0x50, // CAM_LL_START_CONTRAST_LUMA_PERCENTAGE
    0x19, // CAM_LL_STOP_CONTRAST_LUMA_PERCENTAGE
    0x05, // CAM_LL_CLUSTER_DC_GATE_PERCENTAGE
    0x40, // CAM_LL_SUMMING_SENSITIVITY_FACTOR
    0x1B, // CAM_AET_TARGET_AVERAGE_LUMA_DARK
    0x00, // CAM_AET_AEMODE (was 0x0E)
    0x05, // CCM_DELTA_GAIN
    0x20  // AE_TRACK_AE_TRACKING_DAMPENING_SPEED
};

static const uint16_t cpipe_regs_16_bit[][2] = {
    {0xC926, 0x0020}, // CAM_LL_START_BRIGHTNESS
    {0xC928, 0x009A}, // CAM_LL_STOP_BRIGHTNESS
    {0xC946, 0x0070}, // CAM_LL_START_GAIN_METRIC
    {0xC948, 0x00F3}, // CAM_LL_STOP_GAIN_METRIC
    {0xC952, 0x0020}, // CAM_LL_START_TARGET_LUMA_BM
    {0xC954, 0x009A}, // CAM_LL_STOP_TARGET_LUMA_BM
    {0xC93C, 0x0020}, // CAM_LL_START_CONTRAST_BM
    {0xC93E, 0x009A}, // CAM_LL_STOP_CONTRAST_BM
    {0xC940, 0x00DC}, // CAM_LL_GAMMA
    {0xC94A, 0x0230}, // CAM_LL_START_FADE_TO_BLACK_LUMA
    {0xC94C, 0x0010}, // CAM_LL_STOP_FADE_TO_BLACK_LUMA
    {0xC94E, 0x01CD}, // CAM_LL_CLUSTER_DC_TH_BM
    {0xC890, 0x0080}, // CAM_AET_TARGET_GAIN
    {0xC886, 0x0100}, // CAM_AET_AE_MAX_VIRT_AGAIN
    {0xC87C, 0x005A}  // CAM_AET_BLACK_CLIPPING_TARGET
};

static int host_command(sensor_t *sensor, uint16_t command)
{
    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_HOST_COMMAND, (command | MT9M114_HC_OK)) != 0) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(MT9M114_HC_DELAY)) {
        uint16_t reg_data;

        if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_HOST_COMMAND, &reg_data) != 0) {
            return -1;
        }

        if ((reg_data & command) == 0) {
            return (reg_data & MT9M114_HC_OK) ? 0 : -1;
        }

        if ((mp_hal_ticks_ms() - start) >= MT9M114_HC_TIMEOUT) {
            return -1;
        }
    }

    return 0;
}

static int load_array(sensor_t *sensor, uint16_t address, const uint16_t *array, size_t array_len)
{
    uint16_t address_rev = __REV16(address);
    int ret = cambus_write_bytes(&sensor->bus, sensor->slv_addr,
            (uint8_t *) &address_rev, sizeof(uint16_t), CAMBUS_XFER_SUSPEND);

    for (size_t i = 0; i < array_len; i++) {
        uint16_t reg_data = __REV16(array[i]);
        ret |= cambus_write_bytes(&sensor->bus, sensor->slv_addr,
                (uint8_t *) &reg_data, sizeof(uint16_t),
                (i != (array_len - 1)) ? CAMBUS_XFER_SUSPEND : CAMBUS_XFER_NO_FLAGS);
    }

    return ret;
}

static int load_and_apply_patch(sensor_t *sensor, uint16_t patch_address, const uint16_t *patch, size_t patch_len,
                                uint16_t patch_loader_address, uint16_t patch_id, uint32_t patch_firmware_id)
{
    int ret = 0;

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_XMDA_ACCESS_CTL_STAT, patch_address >> 15);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_XMDA_PHYSICAL_ADDRESS_ACCESS, patch_address & 0x7FFF);
    ret |= load_array(sensor, patch_address, patch, patch_len);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_XMDA_LOGIC_ADDRESS_ACCESS, 0x0000);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_PATCHLDR_LOADER_ADDRESS, patch_loader_address);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_PATCHLDR_PATCH_ID, patch_id);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_PATCHLDR_FIRMWARE_ID_HI, patch_firmware_id >> 16);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_PATCHLDR_FIRMWARE_ID_LO, patch_firmware_id);

    ret |= host_command(sensor, MT9M114_HC_APPLY_PATCH);

    uint8_t reg_data;
    ret |= cambus_readb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_PATCHLDR_APPLY_STATUS, &reg_data);
    ret |= (reg_data == 0) ? 0: -1;

    return ret;
}

static int load_and_apply_patch_0202(sensor_t *sensor) // Black level correction fix
{
    return load_and_apply_patch(sensor, 0xd000, patch_0202, sizeof(patch_0202) / sizeof(patch_0202[0]),
            0x010c, 0x0202, 0x41030202);
}

static int load_and_apply_patch_0302(sensor_t *sensor) // Adaptive Sensitivity
{
    return load_and_apply_patch(sensor, 0xd12c, patch_0302, sizeof(patch_0302) / sizeof(patch_0302[0]),
            0x04b4, 0x0302, 0x41030202);
}

static int load_and_apply_patch_0402(sensor_t *sensor) // System Idle Control (auto wakeup from standby)
{
    return load_and_apply_patch(sensor, 0xd530, patch_0402, sizeof(patch_0402) / sizeof(patch_0402[0]),
            0x0634, 0x0402, 0x41030202);
}

static int load_and_apply_patch_0502(sensor_t *sensor) // Ambient light sensor
{
    return load_and_apply_patch(sensor, 0xd748, patch_0502, sizeof(patch_0502) / sizeof(patch_0502[0]),
            0x0884, 0x0502, 0x41030202);
}

static int load_awb_cmm(sensor_t *sensor)
{
    return load_array(sensor, 0xC892, awb_ccm, sizeof(awb_ccm) / sizeof(awb_ccm[0]));
}

static int load_awb(sensor_t *sensor)
{
    int ret = 0;

    ret |= cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AWB_XSCALE, 0x03);
    ret |= cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AWB_YSCALE, 0x02);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AWB_Y_SHIFT_PRE_ADJ, 0x003C);

    ret |= load_array(sensor, 0xC8F4, awb_weights, sizeof(awb_weights) / sizeof(awb_weights[0]));

    for (int i = 0xC90C; i <= 0xC911; i++) {
        ret |= cambus_writeb2(&sensor->bus, sensor->slv_addr, i, 0x80);
    }

    return ret;
}

static int load_cpipe(sensor_t *sensor)
{
    int ret = 0;

    int a_size = sizeof(cpipe_regs_8_bit_a) / sizeof(cpipe_regs_8_bit_a[0]);
    int d_size = sizeof(cpipe_regs_8_bit_d) / sizeof(cpipe_regs_8_bit_d[0]);

    for (int i = 0, ii = IM_MIN(a_size, d_size); i < ii; i++) {
        ret |= cambus_writeb2(&sensor->bus, sensor->slv_addr, cpipe_regs_8_bit_a[i], cpipe_regs_8_bit_d[i]);
    }

    for (int i = 0; i < (sizeof(cpipe_regs_16_bit) / sizeof(cpipe_regs_16_bit[0])); i++) {
        ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, cpipe_regs_16_bit[i][0], cpipe_regs_16_bit[i][1]);
    }

    return ret;
}

static int set_system_state(sensor_t *sensor, uint8_t state)
{
    if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SYSMGR_NEXT_STATE, state) != 0) {
        return -1;
    }

    return (host_command(sensor, MT9M114_HC_SET_STATE) == 0) ? 0 : -1;
}

static int change_config(sensor_t *sensor)
{
    return set_system_state(sensor, MT9M114_SS_ENTER_CONFIG_CHANGE);
}

static int refresh(sensor_t *sensor)
{
    int ret = host_command(sensor, MT9M114_HC_REFRESH);

    if (ret == 0) {
        return ret;
    }

    uint8_t reg_data;

    if (cambus_readb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SEQ_ERROR_CODE, &reg_data) != 0) {
        return -1;
    }

    return -reg_data;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat);
static int set_framesize(sensor_t *sensor, framesize_t framesize);
static int reset(sensor_t *sensor)
{
    int ret = 0;
    readout_x = 0;
    readout_y = 0;

    readout_w = ACTIVE_SENSOR_WIDTH;
    readout_h = ACTIVE_SENSOR_HEIGHT;

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SYSCTL, MT9M114_SYSCTL_SOFT_RESET);
    mp_hal_delay_ms(1);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SYSCTL, 0);
    mp_hal_delay_ms(45);

    for (mp_uint_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(MT9M114_HC_DELAY)) {
        uint16_t reg_data;

        if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_HOST_COMMAND, &reg_data) != 0) {
            return -1;
        }

        if ((reg_data & MT9M114_HC_SET_STATE) != 0) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= MT9M114_HC_TIMEOUT) {
            return -1;
        }
    }

    for (mp_uint_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(MT9M114_HC_DELAY)) {
        uint8_t reg_data;

        if (cambus_readb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SYSMGR_CURRENT_STATE, &reg_data) != 0) {
            return -1;
        }

        if (reg_data != MT9M114_SS_STANDBY) {
            break;
        }

        if ((mp_hal_ticks_ms() - start) >= MT9M114_HC_TIMEOUT) {
            return -1;
        }
    }

    // Errata 2 (Black Frame Output)
    uint16_t reg;
    ret |= cambus_readw2(&sensor->bus, sensor->slv_addr, 0x301A, &reg);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, 0x301A, reg | (1 << 9));

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_SYSCTL_PLL_DIVIDER_M_N,
            (sensor_get_xclk_frequency() == MT9M114_XCLK_FREQ)
            ? 0x120 // xclk=24MHz, m=32, n=1, sensor=48MHz, bus=76.8MHz
            : 0x448); // xclk=25MHz, m=72, n=4, sensor=45MHz, bus=72MHz

    for (int i = 0; i < (sizeof(default_regs) / sizeof(default_regs[0])); i++) {
        ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    set_framesize(sensor, FRAMESIZE_SXGAM);

    ret |= load_and_apply_patch_0202(sensor);
    ret |= load_and_apply_patch_0302(sensor);
    ret |= load_and_apply_patch_0402(sensor);
    ret |= load_and_apply_patch_0502(sensor);

    ret |= load_awb_cmm(sensor);
    ret |= load_awb(sensor);
    ret |= load_cpipe(sensor);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_PORT_OUTPUT_CONTROL, 0x8008);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_PAD_SLEW, 0x0777);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE,
            MT9M114_SENSOR_CONTROL_READ_MODE_HMIRROR | MT9M114_SENSOR_CONTROL_READ_MODE_VFLIP);

    ret |= change_config(sensor);

    return ret;
}

static int sleep(sensor_t *sensor, int enable)
{
    uint8_t state = MT9M114_SS_LEAVE_STANDBY;
    uint8_t new_state = MT9M114_SS_STREAMING;

    if (enable) {
        state = MT9M114_SS_ENTER_STANDBY;
        new_state = MT9M114_SS_STANDBY;
    }

    if (set_system_state(sensor, state) != 0) {
        return -1;
    }

    for (mp_uint_t start = mp_hal_ticks_ms();; mp_hal_delay_ms(MT9M114_HC_DELAY)) {
        uint8_t reg_data;

        if (cambus_readb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SYSMGR_CURRENT_STATE, &reg_data) != 0) {
            return -1;
        }

        if (reg_data == new_state) {
            return 0;
        }

        if ((mp_hal_ticks_ms() - start) >= MT9M114_HC_TIMEOUT) {
            return -1;
        }
    }
}


static int read_reg(sensor_t *sensor, uint16_t reg_addr)
{
    uint16_t reg_data;

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }

    return reg_data;
}

static int write_reg(sensor_t *sensor, uint16_t reg_addr, uint16_t reg_data)
{
    return cambus_writew2(&sensor->bus, sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    uint16_t reg = 0;

    switch (pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_YUV422:
            reg = MT9M114_OUTPUT_FORMAT_YUV | MT9M114_OUTPUT_FORMAT_SWAP_BYTES;
            break;
        case PIXFORMAT_RGB565:
            reg = MT9M114_OUTPUT_FORMAT_RGB | MT9M114_OUTPUT_FORMAT_RGB565 | MT9M114_OUTPUT_FORMAT_SWAP_BYTES;
            break;
        case PIXFORMAT_BAYER:
            if ((sensor->framesize != FRAMESIZE_INVALID)
            && (sensor->framesize != FRAMESIZE_VGA)
            && (sensor->framesize != FRAMESIZE_SXGAM)) {
                return -1;
            }
            reg = MT9M114_OUTPUT_FORMAT_BAYER | MT9M114_OUTPUT_FORMAT_RAW_BAYER_10;
            break;
        default:
            return -1;
    }

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_OUTPUT_FORMAT, reg) != 0) {
        return -1;
    }

    return change_config(sensor);
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret = 0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    if ((sensor->pixformat == PIXFORMAT_BAYER) && ((framesize != FRAMESIZE_VGA) && (framesize != FRAMESIZE_SXGAM))) {
        return -1;
    }

    if ((w > ACTIVE_SENSOR_WIDTH) || (h > ACTIVE_SENSOR_HEIGHT)) {
        return -1;
    }

    // Step 0: Clamp readout settings.

    readout_w = IM_MAX(readout_w, w);
    readout_h = IM_MAX(readout_h, h);

    int readout_x_max = (ACTIVE_SENSOR_WIDTH - readout_w) / 2;
    int readout_y_max = (ACTIVE_SENSOR_HEIGHT - readout_h) / 2;
    readout_x = IM_MAX(IM_MIN(readout_x, readout_x_max), -readout_x_max);
    readout_y = IM_MAX(IM_MIN(readout_y, readout_y_max), -readout_y_max);

    // Step 1: Determine readout area and subsampling amount.

    uint16_t read_mode;

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE, &read_mode) != 0) {
        return -1;
    }

    int read_mode_div = 1;
    read_mode &= ~(MT9M114_SENSOR_CONTROL_READ_MODE_HBIN_MASK | MT9M114_SENSOR_CONTROL_READ_MODE_VBIN_MASK);

    if ((w <= (readout_w / 2)) && (h <= (readout_h / 2))) {
        read_mode_div = 2;
        read_mode |= MT9M114_SENSOR_CONTROL_READ_MODE_HBIN | MT9M114_SENSOR_CONTROL_READ_MODE_VBIN;
    }

    // Step 2: Determine horizontal and vertical start and end points.

    uint16_t sensor_w = readout_w + DUMMY_WIDTH_BUFFER; // camera hardware needs dummy pixels to sync
    uint16_t sensor_h = readout_h + DUMMY_HEIGHT_BUFFER; // camera hardware needs dummy lines to sync

    uint16_t sensor_ws = IM_MAX(IM_MIN((((ACTIVE_SENSOR_WIDTH - sensor_w) / 4) + (readout_x / 2)) * 2,
            ACTIVE_SENSOR_WIDTH - sensor_w),
            -(DUMMY_WIDTH_BUFFER / 2)) + DUMMY_COLUMNS; // must be multiple of 2
    uint16_t sensor_we = sensor_ws + sensor_w - 1;

    int sensor_ws_mod = sensor_ws % (read_mode_div * 4); // multiple 4/8
    if (sensor_ws_mod) {
        sensor_ws -= sensor_ws_mod;
        sensor_we += read_mode_div;
    }

    uint16_t sensor_hs = IM_MAX(IM_MIN((((ACTIVE_SENSOR_HEIGHT - sensor_h) / 4) - (readout_y / 2)) * 2,
            ACTIVE_SENSOR_HEIGHT - sensor_h),
            -(DUMMY_HEIGHT_BUFFER / 2)) + DUMMY_LINES; // must be multiple of 2
    uint16_t sensor_he = sensor_hs + sensor_h - 1;

    int sensor_hs_mod = sensor_hs % (read_mode_div * 4); // multiple 4/8
    if (sensor_hs_mod) {
        sensor_hs -= sensor_hs_mod;
        sensor_he += read_mode_div;
    }

    // Step 3: Determine scaling window offset.

    float ratio = IM_MIN((readout_w / read_mode_div) / ((float) w), (readout_h / read_mode_div) / ((float) h));

    uint16_t w_mul = w * ratio;
    uint16_t h_mul = h * ratio;
    uint16_t x_off = ((readout_w / read_mode_div) - w_mul) / 2;
    uint16_t y_off = ((readout_h / read_mode_div) - h_mul) / 2;

    // Step 4: Write regs.

    uint16_t frame_length_lines = (readout_h / read_mode_div) + ((read_mode_div == 2) ? 40 : 39) + DUMMY_HEIGHT_BUFFER;
    uint16_t line_length_pck = (readout_w / read_mode_div) + ((read_mode_div == 2) ? 534 : 323) + DUMMY_WIDTH_BUFFER;

    // Errata 4 (Cam_port_clock_slowdown issues/limitations)
    if (!(line_length_pck % 5)) {
        line_length_pck += 1;
    }

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_Y_ADDR_START, sensor_hs);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_X_ADDR_START, sensor_ws);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_Y_ADDR_END, sensor_he);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_X_ADDR_END, sensor_we);

    int pixclk = (sensor_get_xclk_frequency() == MT9M114_XCLK_FREQ) ? 48000000 : 45000000;

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_PIXCLK, pixclk >> 16);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_PIXCLK + 2, pixclk);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_FINE_INTEG_TIME_MIN,
            (read_mode_div == 2) ? 451 : 219); // figured this out by checking register wizard against datasheet
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_FINE_INTEG_TIME_MAX,
            (read_mode_div == 2) ? 947 : 1480); // figured this out by checking register wizard against datasheet
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_FRAME_LENGTH_LINES,
            frame_length_lines); // figured this out by checking register wizard against datasheet
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_LINE_LENGTH_PCK,
            line_length_pck); // figured this out by checking register wizard against datasheet
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_FINE_CORRECTION,
            (read_mode_div == 2) ? 224 : 96); // figured this out by checking register wizard against datasheet
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CFG_CPIPE_LAST_ROW,
            (readout_h / read_mode_div) + 3); // figured this out by checking register wizard against datasheet

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE, read_mode);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CROP_WINDOW_X_OFFSET, x_off);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CROP_WINDOW_Y_OFFSET, y_off);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CROP_WINDOW_WIDTH, w_mul);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CROP_WINDOW_HEIGHT, h_mul);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_OUTPUT_WIDTH, w);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_OUTPUT_HEIGHT, h);

    float rate = (((float) pixclk) / (frame_length_lines * line_length_pck)) * 256;

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AET_MAX_FRAME_RATE, fast_ceilf(rate));
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AET_MIN_FRAME_RATE, fast_floorf(rate / 2.f));

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AWB_CLIP_WINDOW_X_START, 0);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AWB_CLIP_WINDOW_Y_START, 0);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AWB_CLIP_WINDOW_X_END, w - 1);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AWB_CLIP_WINDOW_Y_END, h - 1);

    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AE_INITIAL_WINDOW_X_START, 0);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AE_INITIAL_WINDOW_Y_START, 0);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AE_INITIAL_WINDOW_X_END, (w / 5) - 1);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AE_INITIAL_WINDOW_Y_END, (h / 5) - 1);

    ret |= cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_AUTO_BINNING_MODE, 0);
    ret |= cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_LL_ALGO, 0);

    return change_config(sensor);
}

static int set_framerate(sensor_t *sensor, int framerate)
{
    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AET_MAX_FRAME_RATE, framerate * 256) != 0) {
        return -1;
    }

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_AET_MIN_FRAME_RATE, framerate * 128) != 0) {
        return -1;
    }

    return change_config(sensor);
}

static int set_contrast(sensor_t *sensor, int level) // -16 to +16
{
    int new_level = (level > 0) ? (level * 2) : level;

    if ((new_level < -16) || (32 < new_level)) {
        return -1;
    }

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_CONTRAST_CONTROL, new_level + 32) != 0) {
        return -1;
    }

    return refresh(sensor);
}

static int set_brightness(sensor_t *sensor, int level) // -16 to +16
{
    int new_level = level * 2;

    if ((new_level < -32) || (32 < new_level)) {
        return -1;
    }

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_BRIGHTNESS_CONTROL, new_level + 55) != 0) {
        return -1;
    }

    return refresh(sensor);
}

static int set_saturation(sensor_t *sensor, int level) // -16 to +16
{
    int new_level = level * 8;

    if ((new_level < -128) || (128 < new_level)) {
        return -1;
    }

    new_level = IM_MIN(new_level, 127);

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_SATURATION_CONTROL, new_level + 128) != 0) {
        return -1;
    }

    return refresh(sensor);
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
    return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_MODE_SELECT, enable ? 2 : 0) != 0) {
        return -1;
    }

    return change_config(sensor);
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
    if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_AE_MODE_CONTROL, enable ? 0x2 : 0x1) != 0) {
        return -1;
    }

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        int gain = IM_MAX(IM_MIN(fast_expf((gain_db / 20.f) * fast_log(10.f)) * 32.f, 0xffff), 0x0000);

        if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_GAIN_CONTROL, gain) != 0) {
            return -1;
        }
    }

    return refresh(sensor);
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
    uint16_t gain;

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_GAIN_CONTROL, &gain) != 0) {
        return -1;
    }

    *gain_db = 20.f * (fast_log(gain / 32.f) / fast_log(10.f));

    return 0;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
    if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_AE_MODE_CONTROL, enable ? 0x2 : 0x1) != 0) {
        return -1;
    }

    if ((enable == 0) && (exposure_us >= 0)) {
        if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_MANUAL_EXPOSURE_CONFIG, 0x1) != 0) {
            return -1;
        }

        int exposure_100_us = exposure_us / 100;

        if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_EXPOSURE_TIME_ABSOLUTE_CTRL,
                exposure_100_us >> 16) != 0) {
            return -1;
        }

        if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_EXPOSURE_TIME_ABSOLUTE_CTRL + 2,
                exposure_100_us) != 0) {
            return -1;
        }
    }

    return refresh(sensor);
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
    uint16_t reg_h, reg_l;

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_EXPOSURE_TIME_ABSOLUTE_CTRL, &reg_h) != 0) {
        return -1;
    }

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_EXPOSURE_TIME_ABSOLUTE_CTRL + 2, &reg_l) != 0) {
        return -1;
    }

    *exposure_us = ((reg_h << 16) | reg_l) * 100;

    return 0;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
    if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_UVC_WHITE_BALANCE_AUTO_CONTROL, enable ? 0x1 : 0x0) != 0) {
        return -1;
    }

    return 0;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
    *r_gain_db = 0;
    *g_gain_db = 0;
    *b_gain_db = 0;

    return 0;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    uint16_t reg_data;

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE, &reg_data) != 0) {
        return -1;
    }

    reg_data = (reg_data & (~MT9M114_SENSOR_CONTROL_READ_MODE_HMIRROR)) |
            (enable ? 0x0 : MT9M114_SENSOR_CONTROL_READ_MODE_HMIRROR); // inverted

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE, reg_data) != 0) {
        return -1;
    }

    return change_config(sensor);
}

static int set_vflip(sensor_t *sensor, int enable)
{
    uint16_t reg_data;

    if (cambus_readw2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE, &reg_data) != 0) {
        return -1;
    }

    reg_data = (reg_data & (~MT9M114_SENSOR_CONTROL_READ_MODE_VFLIP)) |
            (enable ? 0x0 : MT9M114_SENSOR_CONTROL_READ_MODE_VFLIP); // inverted

    if (cambus_writew2(&sensor->bus, sensor->slv_addr, MT9M114_REG_SENSOR_CONTROL_READ_MODE, reg_data) != 0) {
        return -1;
    }

    return change_config(sensor);
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
{
    switch (sde) {
        case SDE_NEGATIVE:
            if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_SFX_CONTROL, 0x3) != 0) {
                return -1;
            }
            break;
        case SDE_NORMAL:
            if (cambus_writeb2(&sensor->bus, sensor->slv_addr, MT9M114_REG_CAM_SFX_CONTROL, 0x0) != 0) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    return refresh(sensor);
}

static int ioctl(sensor_t *sensor, int request, va_list ap)
{
    int ret = 0;

    switch (request) {
        case IOCTL_SET_READOUT_WINDOW: {
            int tmp_readout_x = va_arg(ap, int);
            int tmp_readout_y = va_arg(ap, int);
            int tmp_readout_w = IM_MAX(IM_MIN(va_arg(ap, int), ACTIVE_SENSOR_WIDTH), resolution[sensor->framesize][0]);
            int tmp_readout_h = IM_MAX(IM_MIN(va_arg(ap, int), ACTIVE_SENSOR_HEIGHT), resolution[sensor->framesize][1]);
            int readout_x_max = (ACTIVE_SENSOR_WIDTH - tmp_readout_w) / 2;
            int readout_y_max = (ACTIVE_SENSOR_HEIGHT - tmp_readout_h) / 2;
            tmp_readout_x = IM_MAX(IM_MIN(tmp_readout_x, readout_x_max), -readout_x_max);
            tmp_readout_y = IM_MAX(IM_MIN(tmp_readout_y, readout_y_max), -readout_y_max);
            bool changed = (tmp_readout_x != readout_x) || (tmp_readout_y != readout_y) ||
                           (tmp_readout_w != readout_w) || (tmp_readout_h != readout_h);
            readout_x = tmp_readout_x;
            readout_y = tmp_readout_y;
            readout_w = tmp_readout_w;
            readout_h = tmp_readout_h;
            if (changed && (sensor->framesize != FRAMESIZE_INVALID)) {
                ret |= set_framesize(sensor, sensor->framesize);
            }
            break;
        }
        case IOCTL_GET_READOUT_WINDOW: {
            *va_arg(ap, int *) = readout_x;
            *va_arg(ap, int *) = readout_y;
            *va_arg(ap, int *) = readout_w;
            *va_arg(ap, int *) = readout_h;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int mt9m114_init(sensor_t *sensor)
{
    // Initialize sensor structure.
    sensor->reset               = reset;
    sensor->sleep               = sleep;
    sensor->read_reg            = read_reg;
    sensor->write_reg           = write_reg;
    sensor->set_pixformat       = set_pixformat;
    sensor->set_framesize       = set_framesize;
    sensor->set_framerate       = set_framerate;
    sensor->set_contrast        = set_contrast;
    sensor->set_brightness      = set_brightness;
    sensor->set_saturation      = set_saturation;
    sensor->set_gainceiling     = set_gainceiling;
    sensor->set_colorbar        = set_colorbar;
    sensor->set_auto_gain       = set_auto_gain;
    sensor->get_gain_db         = get_gain_db;
    sensor->set_auto_exposure   = set_auto_exposure;
    sensor->get_exposure_us     = get_exposure_us;
    sensor->set_auto_whitebal   = set_auto_whitebal;
    sensor->get_rgb_gain_db     = get_rgb_gain_db;
    sensor->set_hmirror         = set_hmirror;
    sensor->set_vflip           = set_vflip;
    sensor->set_special_effect  = set_special_effect;
    sensor->ioctl               = ioctl;

    // Set sensor flags
    sensor->hw_flags.vsync      = 0;
    sensor->hw_flags.hsync      = 0;
    sensor->hw_flags.pixck      = 0;
    sensor->hw_flags.fsync      = 0;
    sensor->hw_flags.jpege      = 0;
    sensor->hw_flags.gs_bpp     = 2;
    sensor->hw_flags.rgb_swap   = 0;
    sensor->hw_flags.bayer      = SENSOR_HW_FLAGS_BAYER_GBRG;

    return 0;
}

#endif // (OMV_ENABLE_MT9M114 == 1)
