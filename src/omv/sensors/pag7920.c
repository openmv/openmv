/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2023 Lake Fu <lake_fu@pixart.com> for PixArt Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * PAG7920 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_PAG7920_ENABLE == 1)

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "py/mphal.h"

#include "omv_csi.h"
#include "framebuffer.h"
#include "omv_i2c.h"

#include "pag7920.h"
#include "pag7920_reg.h"

#define SENSOR_I2C_DEBUG (0)

#define US (1000000)  // microsecond
#ifndef M_LN29
#define M_LN29  (3.36729582998647402718)
#endif

// Grayscale/QVGA
const uint16_t default_regs [][2] = {
    {0xEF, 0x00},
    {0x1D, 0x41},
    {0x64, 0x01},
    {0x45, 0x44},
    {0x12, 0x20},
    {0x0C, 0xC8},
    {0x11, 0xC8},
    {0x42, 0xC8},
    {0x43, 0xC8},
    {0x44, 0xC8},
    {0xAF, 0x31},
    {0x69, 0x14},
    {0xEF, 0x00},
    {0x4C, 0x00},
    {0x4D, 0x35},
    {0x4E, 0x0C},
    {0x4F, 0x00},
    {0xEF, 0x02},
    {0x02, 0x64},
    {0x03, 0x08},
    {0xEF, 0x04},
    {0x2C, 0xC6},
    {0x2D, 0x2D},
    {0x2E, 0x00},
    {0x30, 0x31},
    {0x31, 0x80},
    {0x40, 0x1D},
    {0x41, 0x00},
    {0x42, 0xD0},
    {0x43, 0x01},
    {0x44, 0xF0},
    {0x45, 0x00},
    {0x46, 0x00},
    {0x47, 0x00},
    {0x48, 0xF0},
    {0x49, 0x0D},
    {0x4A, 0x0C},
    {0x4B, 0x00},
    {0x51, 0x1D},
    {0x52, 0x00},
    {0x53, 0xC0},
    {0x54, 0xD4},
    {0x55, 0x01},
    {0x56, 0x00},
    {0xEF, 0x01},
    {0xC4, 0x02},
    {0xC6, 0x40},
    {0xC7, 0x01},
    {0xC8, 0xF0},
    {0xC9, 0x00},
    {0xEF, 0x02},
    {0x11, 0x03},
    {0x19, 0xFC},
    {0x1A, 0x00},
    {0x21, 0x40},
    {0x22, 0x01},
    {0x23, 0xF0},
    {0x24, 0x00},
    {0x27, 0x0C},
    {0x28, 0x00},
    {0x2D, 0xF0},
    {0x2E, 0x00},
    {0x56, 0x40},
    {0x57, 0x01},
    {0x58, 0xFE},
    {0x59, 0x00},
    {0xEF, 0x01},
    {0x33, 0x46},
    {0x3B, 0x30},
    {0x40, 0x96},
    {0xD9, 0x32},
    {0xDB, 0x64},
    {0xDD, 0x64},
    {0xEF, 0x02},
    {0xA5, 0x00},
    {0xA6, 0x96},
    {0xEF, 0x00},
    {0x09, 0x10},
    {0x18, 0x01},
    {0x2F, 0x44},
    {0x37, 0x04},
    {0x38, 0x06},
    {0x3F, 0x01},
    {0x55, 0x01},
    {0x66, 0x01},
    {0xEF, 0x01},
    {0x03, 0x00},
    {0x04, 0xAB},
    {0x07, 0x02},
    {0x0A, 0x00},
    {0x0B, 0x40},
    {0x0F, 0x0B},
    {0x11, 0x0A},
    {0x13, 0x0C},
    {0x16, 0x00},
    {0x17, 0xA9},
    {0x36, 0x00},
    {0x37, 0x02},
    {0x4D, 0x02},
    {0x56, 0xB8},
    {0x57, 0x01},
    {0x58, 0xB8},
    {0x59, 0x01},
    {0x62, 0x00},
    {0x63, 0x06},
    {0x69, 0x1C},
    {0x6A, 0x1D},
    {0x6B, 0x68},
    {0x6C, 0x67},
    {0x76, 0x06},
    {0x77, 0x09},
    {0x78, 0x02},
    {0x79, 0x03},
    {0x84, 0x19},
    {0x86, 0x14},
    {0x87, 0x19},
    {0x89, 0x14},
    {0x8A, 0x23},
    {0x8C, 0x1E},
    {0x8D, 0x23},
    {0x8F, 0x1E},
    {0x9D, 0x0A},
    {0xA0, 0x00},
    {0xD1, 0xC8},
    {0xD2, 0x00},
    {0xEF, 0x02},
    {0x92, 0x11},
    {0x93, 0x01},
    {0xC3, 0xC8},
    {0xC4, 0x00},
    {0xC5, 0xC8},
    {0xC6, 0x00},
    {0xD1, 0x45},
    {0xEF, 0x00},
    {0xEB, 0x80},
    {0xEF, 0x00},
    {0x30, 0x01},
    // OpenMV typical end token.
    {0x00, 0x00},
};

typedef union {
    struct {
        uint8_t b0 : 1;
        uint8_t b1 : 1;
        uint8_t b2 : 1;
        uint8_t b3 : 1;
        uint8_t b4 : 1;
        uint8_t b5 : 1;
        uint8_t b6 : 1;
        uint8_t b7 : 1;
    } bits;
    uint8_t byte_val;
} bitplane;

extern uint8_t _line_buf;
static int8_t g_bank_cache = -1;

static bool g_f_hflip = false;
static bool g_f_vflip = false;

static int switch_bank(omv_csi_t *csi, uint8_t bank) {
    if (g_bank_cache != bank) {
        debug_printf("W Reg: 0x%02X 0x%02X\r\n", REG_BANK, bank);
        int res = omv_i2c_writeb(&csi->i2c_bus, csi->slv_addr, REG_BANK, bank);
        if (res) {
            g_bank_cache = -1;  // Encountered an unknown error, clear cache.
            printf("switch bank failed, res = %d\n", res);
        } else {
            g_bank_cache = bank;
        }
        return res;
    }
    return 0;
}

static int write_reg_w_bank(omv_csi_t *csi, uint8_t bank, uint8_t addr,
                            uint8_t val) {
    if (switch_bank(csi, bank)) {
        printf("write_reg failed.\n");
        return -1;
    }
    if (addr == REG_BANK) {
        g_bank_cache = -1;
    }

    debug_printf("W Reg: 0x%02X 0x%02X\r\n", addr, val);
    return omv_i2c_writeb(&csi->i2c_bus, csi->slv_addr, addr, val);
}

static int read_reg_w_bank(omv_csi_t *csi, uint8_t bank, uint8_t addr,
                           uint8_t *p_val) {
    if (switch_bank(csi, bank)) {
        printf("read_reg_w_bank() failed.\n");
        return -1;
    }

    debug_printf("R Reg: 0x%02X\r\n", addr);
    return omv_i2c_readb(&csi->i2c_bus, csi->slv_addr, addr, p_val);
}

static int init_csi(omv_csi_t *csi) {
    write_reg_w_bank(csi, BANK_0, R_GLOBAL_RESET, V_GLOBAL_RESET);
    mp_hal_delay_ms(50);
#define ta_seq default_regs
    for (int i = 0; ta_seq[i][0]; i++) {
        debug_printf("W Reg: 0x%02X 0x%02X\r\n", ta_seq[i][0], ta_seq[i][1]);
        int res = omv_i2c_writeb(&csi->i2c_bus, csi->slv_addr, ta_seq[i][0],
                                 ta_seq[i][1]);
        if (res) {
            return res;
        }
    }
#undef ta_seq
    return 0;
}

static int reset(omv_csi_t *csi) {
    // Reset internal flag.
    g_f_hflip = g_f_vflip = false;
    g_bank_cache = -1;
    return init_csi(csi);
}

static int sleep(omv_csi_t *csi, int enable) {
    int ret = write_reg_w_bank(csi, 0, TG_En,
                               enable == 1 ? TG_En_V2 : TG_En_V1);
    if (ret) {
        printf("sleep() failed.\n");
    }
    return ret;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint8_t val = 0;
    if (omv_i2c_readb(&csi->i2c_bus, csi->slv_addr, (uint8_t) reg_addr, &val)) {
        return -1;
    }
    return val;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    int ret;
    ret = omv_i2c_writeb(&csi->i2c_bus, csi->slv_addr, (uint8_t) reg_addr,
                         (uint8_t) reg_data);
    if (ret == 0 && reg_addr == REG_BANK) {
        g_bank_cache = (uint8_t) reg_data;
    }

    return ret;
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    switch (pixformat) {
        case PIXFORMAT_GRAYSCALE:
            break;
        default:
            return -1;
    }
    return 0;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    int res = 0;
    uint8_t val_R0_15, val_R0_16, val_R0_1C,
            val_R0_3E_B2, val_R1_4B_B0,
            val_R1_4B_B4, val_R1_70, val_R1_C2,
            val_R_VSize, val_R_ABC_2_Start, val_R_WOI_VSize,
            val_R_WOI_VStart, val_R_ABC_grad1, val_R2_D9_B0, val_R2_D9_B1,
            val_R_AE_Size__div4_X, val_R_AE_Size__div4_Y, val_R009_H;
    switch (framesize) {
        case OMV_CSI_FRAMESIZE_QVGA:
            val_R0_15 = 0;
            val_R0_16 = 0;
            val_R0_1C = 0;
            val_R0_3E_B2 = 0;
            val_R1_4B_B0 = 0;
            val_R1_4B_B4 = 0;
            val_R1_70 = 0;
            val_R1_C2 = g_f_vflip ? V_R1C2_Q_V_ON : V_R1C2_Q_V_OFF;
            val_R_VSize = 240;
            val_R_ABC_2_Start = 252;
            val_R_WOI_VSize = 240;
            val_R_WOI_VStart = 12;
            val_R_ABC_grad1 = 240;
            val_R2_D9_B0 = 0;
            val_R009_H = g_f_hflip ? V_R009_Q_H_ON : V_R009_Q_H_OFF;
            val_R2_D9_B1 = g_f_hflip ? V_R2D9_Q_H_ON : V_R2D9_Q_H_OFF;
            val_R_AE_Size__div4_X = 80;
            val_R_AE_Size__div4_Y = 60;
            break;
        case OMV_CSI_FRAMESIZE_QQVGA:
            val_R0_15 = 1;
            val_R0_16 = 1;
            val_R0_1C = 1;
            val_R0_3E_B2 = 1;
            val_R1_4B_B0 = 1;
            val_R1_4B_B4 = 1;
            val_R1_70 = 1;
            val_R1_C2 = g_f_vflip?V_R1C2_QQ_V_ON:V_R1C2_QQ_V_OFF;
            val_R_VSize = 120;
            val_R_ABC_2_Start = 129;
            val_R_WOI_VSize = 120;
            val_R_WOI_VStart = 9;
            val_R_ABC_grad1 = 120;
            val_R2_D9_B0 = 1;
            val_R009_H = g_f_hflip ? V_R009_QQ_H_ON : V_R009_QQ_H_OFF;
            val_R2_D9_B1 = 0;
            val_R_AE_Size__div4_X = 40;
            val_R_AE_Size__div4_Y = 30;
            break;
        default:
            return -1;
    }
    bitplane tmp = {.byte_val = 0};
    res |= read_reg_w_bank(csi, BANK_0, R0_15, &tmp.byte_val);
    tmp.bits.b5 = val_R0_15;
    res |= write_reg_w_bank(csi, BANK_0, R0_15, tmp.byte_val);

    res |= read_reg_w_bank(csi, BANK_0, R0_16, &tmp.byte_val);
    tmp.bits.b0 = val_R0_16;
    res |= write_reg_w_bank(csi, BANK_0, R0_16, tmp.byte_val);

    res |= read_reg_w_bank(csi, BANK_0, R0_1C, &tmp.byte_val);
    tmp.bits.b1 = val_R0_1C;
    res |= write_reg_w_bank(csi, BANK_0, R0_1C, tmp.byte_val);

    res |= read_reg_w_bank(csi, BANK_0, R0_3E_B2, &tmp.byte_val);
    tmp.bits.b2 = val_R0_3E_B2;
    //tmp.bits.b5 = val_R0_3E_B5;
    res |= write_reg_w_bank(csi, BANK_0, R0_3E_B2, tmp.byte_val);

    res |= write_reg_w_bank(csi, BANK_0, R0_09_B1, val_R009_H);

    res |= read_reg_w_bank(csi, BANK_1, R1_4B_B0, &tmp.byte_val);
    tmp.bits.b0 = val_R1_4B_B0;
    tmp.bits.b4 = val_R1_4B_B4 & 0x01;
    tmp.bits.b5 = (val_R1_4B_B4 >> 1) & 0x01;
    res |= write_reg_w_bank(csi, BANK_1, R1_4B_B0, tmp.byte_val);

    res |= read_reg_w_bank(csi, BANK_1, R1_70, &tmp.byte_val);
    tmp.bits.b4 = val_R1_70;
    res |= write_reg_w_bank(csi, BANK_1, R1_70, tmp.byte_val);

    tmp.byte_val = val_R1_C2;
    res |= write_reg_w_bank(csi, BANK_1, R1_C2, tmp.byte_val);

    tmp.byte_val = val_R_VSize;
    res |= write_reg_w_bank(csi, BANK_1, R1_C8, tmp.byte_val);

    tmp.byte_val = val_R_ABC_2_Start;
    res |= write_reg_w_bank(csi, BANK_2, R2_19, tmp.byte_val);

    tmp.byte_val = val_R_WOI_VSize;
    res |= write_reg_w_bank(csi, BANK_2, R2_23, tmp.byte_val);

    tmp.byte_val = val_R_WOI_VStart;
    res |= write_reg_w_bank(csi, BANK_2, R2_27, tmp.byte_val);

    tmp.byte_val = val_R_ABC_grad1;
    res |= write_reg_w_bank(csi, BANK_2, R2_2D, tmp.byte_val);

    res |= read_reg_w_bank(csi, BANK_2, R2_D9_B0, &tmp.byte_val);
    tmp.bits.b0 = val_R2_D9_B0;
    tmp.bits.b1 = val_R2_D9_B1;
    res |= write_reg_w_bank(csi, BANK_2, R2_D9_B0, tmp.byte_val);

    res |=
        write_reg_w_bank(csi, BANK_4, R_AE_Size__div4_X, val_R_AE_Size__div4_X);

    res |=
        write_reg_w_bank(csi, BANK_4, R_AE_Size__div4_Y, val_R_AE_Size__div4_Y);

    res |= write_reg_w_bank(csi, BANK_0, R_UPDATE_FLAG, V_UPDATE_VALUE);
    res |= write_reg_w_bank(csi, BANK_0, TG_En, 1);

    return res;
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    return 0;
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db,
                         float gain_db_ceiling) {
    static const uint32_t digital = (OMV_PAG7920_CLK_FREQ / US) / 2;
    uint32_t frame_time;
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_0, (uint8_t *) &frame_time);
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_1, (uint8_t *) (&frame_time) + 1);
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_2, (uint8_t *) (&frame_time) + 2);
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_3, (uint8_t *) (&frame_time) + 3);
    uint16_t gain_code;
    if (!enable && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        float exponent = gain_db / 20.0f;
        gain_code = IM_MIN(fast_roundf(expf(M_LN29 + (exponent * M_LN10))), 464);
        write_reg_w_bank(csi, BANK_4, R_AE_Gain_manual_L,
                         *((uint8_t *) &gain_code));
        write_reg_w_bank(csi, BANK_4, R_AE_Gain_manual_H,
                         *((uint8_t *) &gain_code + 1));
        write_reg_w_bank(csi, BANK_0, R_UPDATE_FLAG, V_UPDATE_VALUE);

        // AE disable.
        write_reg_w_bank(csi, BANK_4, R_AE_EnH, 0x32);
        // Delay a frame time.
        mp_hal_delay_ms((frame_time / digital) / 1000);
        write_reg_w_bank(csi, BANK_4, R_AE_MinGain_L, *((uint8_t *) &gain_code));
        write_reg_w_bank(csi, BANK_4, R_AE_MinGain_H, *((uint8_t *) &gain_code + 1));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxGain_L, *((uint8_t *) &gain_code));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxGain_H, *((uint8_t *) &gain_code + 1));
    } else if (enable && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        float exponent = gain_db_ceiling / 20.0f;
        gain_code = IM_MIN(fast_roundf(expf(M_LN29 + (exponent * M_LN10))), 464);
        // Min gain code = 29
        write_reg_w_bank(csi, BANK_4, R_AE_MinGain_L, 0x1D);
        write_reg_w_bank(csi, BANK_4, R_AE_MinGain_H, 0);
        write_reg_w_bank(csi, BANK_4, R_AE_MaxGain_L, *((uint8_t *) &gain_code));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxGain_H, *((uint8_t *) &gain_code + 1));
    }

    // AE enable.
    write_reg_w_bank(csi, BANK_4, R_AE_EnH, 0x31);

    return 0;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    uint16_t gain_code;
    read_reg_w_bank(csi, BANK_4, AE_Total_Gain_L, (uint8_t *) &gain_code);
    read_reg_w_bank(csi, BANK_4, AE_Total_Gain_H, (uint8_t *) &gain_code + 1);
    gain_code &= 0x07ff;
    *gain_db = 20.0f * log10f(gain_code / 29.0f);
    return 0;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int expo_us) {
    const uint32_t digital = (OMV_PAG7920_CLK_FREQ / US) / 2;
    uint32_t frame_time, expo_max_us, expo_min_us;
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_0, (uint8_t *) &frame_time);
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_1, (uint8_t *) (&frame_time) + 1);
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_2, (uint8_t *) (&frame_time) + 2);
    read_reg_w_bank(csi, BANK_0, R_Frame_Time_3, (uint8_t *) (&frame_time) + 3);
    uint32_t expo_max_factor = (frame_time - 10000);
    const uint32_t expo_min_factor = 240;
    expo_max_us = expo_max_factor / digital;
    expo_min_us = expo_min_factor / digital;

    if (!enable) {
        uint32_t expo_manual_factor;
        if (expo_us > expo_max_us) {
            expo_manual_factor = expo_max_factor;
        } else if (expo_us < expo_min_us) {
            expo_manual_factor = expo_min_factor;
        } else {
            expo_manual_factor = expo_us * digital;
        }
        write_reg_w_bank(csi, BANK_4, R_AE_Expo_manual_0,
                         *((uint8_t *) &expo_manual_factor));
        write_reg_w_bank(csi, BANK_4, R_AE_Expo_manual_1,
                         *((uint8_t *) &expo_manual_factor + 1));
        write_reg_w_bank(csi, BANK_4, R_AE_Expo_manual_2,
                         *((uint8_t *) &expo_manual_factor + 2));
        write_reg_w_bank(csi, BANK_4, R_AE_Expo_manual_3,
                         *((uint8_t *) &expo_manual_factor + 3));

        write_reg_w_bank(csi, BANK_0, R_UPDATE_FLAG, V_UPDATE_VALUE);
        // AE disable.
        write_reg_w_bank(csi, BANK_4, R_AE_EnH, 0x32);
        // Delay a frame time.
        mp_hal_delay_ms(((frame_time / digital) / 1000));

        // Bundle AE upbound and lowbound.
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_0, *((uint8_t *) &expo_manual_factor));
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_1,
                         *((uint8_t *) &expo_manual_factor + 1));
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_2,
                         *((uint8_t *) &expo_manual_factor + 2));
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_3,
                         *((uint8_t *) &expo_manual_factor + 3));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_0, *((uint8_t *) &expo_manual_factor));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_1,
                         *((uint8_t *) &expo_manual_factor + 1));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_2,
                         *((uint8_t *) &expo_manual_factor + 2));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_3,
                         *((uint8_t *) &expo_manual_factor + 3));
        // AE enable.
        write_reg_w_bank(csi, BANK_4, R_AE_EnH, 0x31);
    } else {
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_0, *((uint8_t *) &expo_min_factor));
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_1,
                         *((uint8_t *) &expo_min_factor + 1));
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_2,
                         *((uint8_t *) &expo_min_factor + 2));
        write_reg_w_bank(csi, BANK_4, R_AE_MinExpo_3,
                         *((uint8_t *) &expo_min_factor + 3));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_0, *((uint8_t *) &expo_max_factor));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_1,
                         *((uint8_t *) &expo_max_factor + 1));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_2,
                         *((uint8_t *) &expo_max_factor + 2));
        write_reg_w_bank(csi, BANK_4, R_AE_MaxExpo_3,
                         *((uint8_t *) &expo_max_factor + 3));
    }
    return 0;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    static const uint32_t digital = (OMV_PAG7920_CLK_FREQ / US) / 2;
    uint32_t clk_num;

    read_reg_w_bank(csi, BANK_4, Reg_ExpPxclkNum_0, (uint8_t *) &clk_num);
    read_reg_w_bank(csi, BANK_4, Reg_ExpPxclkNum_1, (uint8_t *) (&clk_num) + 1);
    read_reg_w_bank(csi, BANK_4, Reg_ExpPxclkNum_2, (uint8_t *) (&clk_num) + 2);
    read_reg_w_bank(csi, BANK_4, Reg_ExpPxclkNum_3, (uint8_t *) (&clk_num) + 3);
    clk_num &= 0x0fffffff;
    debug_printf("Reg_ExpPxclkNum = %ld\n", clk_num);
    *exposure_us = (int) clk_num / digital;
    return 0;
}

static int set_auto_whitebal(omv_csi_t *csi, int enable, float r_gain_db,
                             float g_gain_db, float b_gain_db) {
    return 0;
}

static int get_rgb_gain_db(omv_csi_t *csi, float *r_gain_db, float *g_gain_db,
                           float *b_gain_db) {
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    int res = 0;

    switch (csi->framesize) {
        case OMV_CSI_FRAMESIZE_QVGA:
            res |= write_reg_w_bank(csi, BANK_0, R0_09_B1,
                                    enable?V_R009_Q_H_ON:V_R009_Q_H_OFF);
            res |= write_reg_w_bank(csi, BANK_2, R2_D9_B1,
                                    enable?V_R2D9_Q_H_ON:V_R2D9_Q_H_OFF);
            break;
        case OMV_CSI_FRAMESIZE_QQVGA:
            res |= write_reg_w_bank(csi, BANK_0, R0_09_B1,
                                    enable?V_R009_QQ_H_ON:V_R009_QQ_H_OFF);
            break;
        default:
            return -1;
    }

    res |= write_reg_w_bank(csi, BANK_0, R_UPDATE_FLAG, V_UPDATE_VALUE);
    res |= write_reg_w_bank(csi, BANK_0, TG_En, 1);

    if (res == 0) {
        g_f_hflip = enable ? true : false;
    }
    return res;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    int res = 0;

    switch (csi->framesize) {
        case OMV_CSI_FRAMESIZE_QVGA:
            res |= write_reg_w_bank(csi, BANK_1, R1_C2,
                                    enable?V_R1C2_Q_V_ON:V_R1C2_Q_V_OFF);
            break;
        case OMV_CSI_FRAMESIZE_QQVGA:
            res |= write_reg_w_bank(csi, BANK_1, R1_C2,
                                    enable?V_R1C2_QQ_V_ON:V_R1C2_QQ_V_OFF);
            break;
        default:
            return -1;
    }

    res |= write_reg_w_bank(csi, BANK_1, R1_CB, enable?0xF3:0x04);
    bitplane tmp = {.byte_val = 0};
    res |= read_reg_w_bank(csi, BANK_1, R1_CE_B2, &tmp.byte_val);
    tmp.bits.b2 = enable ? 1 : 0;
    res |= write_reg_w_bank(csi, BANK_1, R1_CE_B2, tmp.byte_val);

    res |= write_reg_w_bank(csi, BANK_0, R_UPDATE_FLAG, V_UPDATE_VALUE);
    res |= write_reg_w_bank(csi, BANK_0, TG_En, 1);

    if (res == 0) {
        g_f_vflip = enable ? true : false;
    }
    return res;
}

int pag7920_init(omv_csi_t *csi) {
    // Initialize csi structure.
    csi->reset = reset;
    csi->sleep = sleep;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_gainceiling = set_gainceiling;
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_auto_whitebal = set_auto_whitebal;

    csi->get_rgb_gain_db = get_rgb_gain_db;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;

    // Set csi flags
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->frame_sync = 1;
    csi->mono_bpp = 1;

    init_csi(csi);

    return 0;
}
#endif
