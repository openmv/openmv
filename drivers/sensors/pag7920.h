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
#include <stdint.h>

#define REG_BANK                    0xEF
// =======================================
#define BANK_0                      0x00
// ---------------------------------------
#define R_PART_ID_L                 0x00
#define R_PART_ID_H                 0x01
#define TG_En                       0x30
#define TG_En_V1                    0x01
#define TG_En_V2                    0x00
#define R0_09_B1                    0x09
#define R0_15                       0x15
#define R0_16                       0x16
#define R0_1C                       0x1C
#define R0_3E_B2                    0x3E
#define R0_3E_B5                    0x3E
#define R_Frame_Time_0              0x4C
#define R_Frame_Time_1              0x4D
#define R_Frame_Time_2              0x4E
#define R_Frame_Time_3              0x4F
#define R_UPDATE_FLAG               0xEB
#define V_UPDATE_VALUE              0x80
#define R_GLOBAL_RESET              0xEE
#define V_GLOBAL_RESET              0xFF
// =======================================
#define BANK_1                      0x01
// ---------------------------------------
#define R1_4B_B0                    0x4B
#define R1_4B_B4                    0x4B
#define R1_70                       0x70
#define R1_C2                       0xC2
#define R1_C3                       0xC3
#define R1_C8                       0xC8
#define R1_CB                       0xCB
#define R1_CE_B2                    0xCE
// =======================================
#define BANK_2                      0x02
// ---------------------------------------
#define R2_19                       0x19
#define R2_23                       0x23
#define R2_27                       0x27
#define R2_2D                       0x2D
#define R2_D9_B0                    0xD9
#define R2_D9_B1                    0xD9
// =======================================
#define BANK_4                      0x04
// ---------------------------------------
#define R_AE_EnH                    0x30
#define R_AE_manual_En              0x30
#define R_AE_Size__div4_X           0x3A
#define R_AE_Size__div4_Y           0x3B
#define R_AE_MinGain_L              0x40
#define R_AE_MinGain_H              0x41
#define R_AE_MaxGain_L              0x42
#define R_AE_MaxGain_H              0x43
#define R_AE_MinExpo_0              0x44
#define R_AE_MinExpo_1              0x45
#define R_AE_MinExpo_2              0x46
#define R_AE_MinExpo_3              0x47
#define R_AE_MaxExpo_0              0x48
#define R_AE_MaxExpo_1              0x49
#define R_AE_MaxExpo_2              0x4A
#define R_AE_MaxExpo_3              0x4B
#define R_AE_Gain_manual_L          0x51
#define R_AE_Gain_manual_H          0x52
#define R_AE_Expo_manual_0          0x53
#define R_AE_Expo_manual_1          0x54
#define R_AE_Expo_manual_2          0x55
#define R_AE_Expo_manual_3          0x56
#define AE_Total_Gain_L             0xA5
#define AE_Total_Gain_H             0xA6
#define Reg_ExpPxclkNum_0           0xA7
#define Reg_ExpPxclkNum_1           0xA8
#define Reg_ExpPxclkNum_2           0xA9
#define Reg_ExpPxclkNum_3           0xAA

#define V_R009_Q_H_ON               (0x02)
#define V_R009_Q_H_OFF              (0x10)
#define V_R2D9_Q_H_ON               (0x02)
#define V_R2D9_Q_H_OFF              (0x00)

#define V_R009_QQ_H_ON              (0x02)
#define V_R009_QQ_H_OFF             (0x00)

#define V_R1C2_Q_V_ON               (0xF7)
#define V_R1C2_Q_V_OFF              (0x00)
#define V_R1C2_QQ_V_ON              (0xF5)
#define V_R1C2_QQ_V_OFF             (0x02)
