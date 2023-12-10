/*
 * File: paj6100u6_reg.h
 * Created Date: Tuesday, May 25th 2021, 10:45:35 am
 * Author: Lake Fu
 * -----
 * Last Modified: Sunday May 30th 2021 2:39:45 pm
 * Modified By: Lake Fu at <lake_fu@pixart.com>
 * -----
 * MIT License
 *
 * Copyright (c) 2021 Pixart Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * -----
 * HISTORY:
 * Date         By	Comments
 * ----------	---	----------------------------------------------------------
 */

#include <stdint.h>

#define REG_BANK_SWITCH        0x7F

// =======================================
#define BANK_0                 0x00
// ---------------------------------------
#define REG_CMD_HSYNC_INV      0X07      // bit7
#define REG_CMD_VSYNC_INV      0X07      // bit6

#define REG_CMD_AAVG_V         0X07      // bit3
#define REG_CMD_AAVG_H         0X07      // bit2

#define REG_ABC_START_LINE     0x0D      // bit3:1


#define REG_FGH                0x11      // bit5:4
#define REG_GGH                0x12      // bit7

#define REG_EXP_OFFSET_L       0x18
#define REG_EXP_OFFSET_H       0x19
#define REG_CMD_EXPO_L         0x48
#define REG_CMD_EXPO_H         0x49
#define REG_CMD_EXPO_2H        0x4A

#define REG_FRAME_TIME_L       0x7A
#define REG_FRAME_TIME_H       0x7B
#define REG_FRAME_TIME_2H      0x7C
// =======================================
#define BANK_1                 0x01
// ---------------------------------------
#define REG_ISP_UPDATE         0x00

#define REG_CMD_SENSOR_MODE    0X23      // bit7
#define REG_CMD_LPM_ENH        0x23      // bit6

#define REG_CP_WOI_VOFFSET     0x45
#define REG_ABC_SAMPLE_SIZE    0x46      // bit2:0


// 20180504
static const uint8_t low_active_R_ABC_Avg_UBx50_T_BLACINV_EnHx1_T_SIG_REFx1[] = {
//address, value
    0x7F, 0x00,
    0x0C, 0x1D, // software reset
    0x7F, 0x00,
    0x07, 0x00, // low active
    0x08, 0x03,
    0x0B, 0x0E,
    0x0D, 0xC5,
    0x0F, 0x32, //R_ABC_Avg_UB=50
    0x11, 0x41, //R_global=1
    0x13, 0xD2,
    0x14, 0xFE,
    0x15, 0x00, // R_fg_fast=0
    0x17, 0x02, //03
    0x1A, 0x07,
    0x1B, 0x08,
    0x20, 0x00, //R_fast_powerdn_WakeupTime=0
    0x25, 0x78,
    0x29, 0x28,
    0x2B, 0x06,
    0x2F, 0x0E,
    0x30, 0x0E,
    0x34, 0x0F,
    0x35, 0x0F,
    0x3A, 0x28,
    0x45, 0x17,
    0x46, 0x17,
    0x48, 0x10, //EXP
    0x49, 0x27, //EXP
    0x4A, 0x00, //EXP
    0x4D, 0x0D,
    0x4E, 0x20,
    0x62, 0x12,
    0x64, 0x02,
    0x67, 0x0A,
    0x69, 0x0A,
    0x6C, 0x0B,
    0x6E, 0x0B,
    0x71, 0x0A,
    0x73, 0x1D,
    0x75, 0x1E,
    0x77, 0x0B,
    0x7F, 0x01,
    0x01, 0x14,
    0x02, 0x02, //
    0x04, 0x96,
    0x05, 0x03,
    0x06, 0x46,
    0x0D, 0x9F, //T_BLACINV_EnH=1
    0x0E, 0x11, //T_SIG_REF=1
    0x0F, 0x48,
    0x10, 0x10, //T_vcom_lvl=0
    0x11, 0x00,
    0x12, 0x05,
    0x15, 0x00,
    0x16, 0x01, //schmitt trigger
    0x17, 0x67,
    0x18, 0xD0, //T_vrt_lvl=T_vrb_lvl=2
    0x21, 0x14,
    0x22, 0x80, //
    0x2F, 0x30,
    0x35, 0x64,
    0x39, 0x03,
    0x3A, 0x03,
    0x46, 0x06,
    0x4A, 0x00, //FX2
    0x4B, 0x00, //FX2
    0x00, 0x01, //updated flag
};
