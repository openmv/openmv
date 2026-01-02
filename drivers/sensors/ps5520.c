/*
 * Copyright (C) 2025 OpenMV, LLC.
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
 * PixArt PS5520 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_PS5520_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "omv_i2c.h"
#include "omv_csi.h"
#include "py/mphal.h"

#define REG_BANK                (0xEF)

// Bank #0
#define CMD_OUTGEN              (0xBF)
// Bank #1
#define CMD_LPF_H               (0x0A)
#define CMD_LPF_L               (0x0B)
#define CMD_OFFNY1_H            (0x0C)
#define CMD_OFFNY1_L            (0x0D)
#define CMD_GAIN_IDX            (0x83)
#define CMD_HFLIP               (0x1B)
#define CMD_HSIZE_E1            (0x1C)
#define CMD_VFLIP               (0x1D)
#define CMD_NP                  (0xAB)
#define R_ISP_TESTMODE          (0x92)
#define SENSOR_UPDATE           (0x09)

#define PIX_CLK                 (160000000L)

#define VTS_5M_30               (1980)
#define HTS_5M_30               (2700)
#define CONST1                  (HTS_5M_30 - 1600)

static int g_div = 1;
#define ConvertL2T(line)        ((((line * HTS_5M_30) + CONST1) * (g_div) + (PIX_CLK / 2000000)) / (PIX_CLK / 1000000))
#define ConvertT2L(t)           (((t / (g_div)) * (PIX_CLK / 1000000) - CONST1 + (HTS_5M_30 / 2)) / HTS_5M_30)
#define ConvertT2LineBase(t)    (((t) * (PIX_CLK / 1000000) - CONST1 + (HTS_5M_30 / 2)) / HTS_5M_30)

#define PS5520_MIN_INT          (1) // ExpLine min vlaue
#define PS5520_MAX_INT          (ConvertT2LineBase(200000))  // 200ms
#define PS5520_MIN_AGAIN_REG    (20) // 1.25f * 16
#define PS5520_MAX_AGAIN_REG    (512) // 32.0f * 16

#define PS5520_GAIN_SCALE       (16)
#define PS5520_GAIN_SCALE_F     (16.0f)

#define PS5520_GAIN(idx)        ((16 + ((idx & 0x0F))) << (idx >> 4))
#define PS5520_MIN_GAIN_IDX     (4)
#define PS5520_MAX_GAIN_IDX     (80)

#define PS5520_WIDTH_ALIGN      (8)
#define PS5520_FPS_MAX          (30)

#define PS5520_DEF_GAIN         (PS5520_MIN_GAIN_IDX)
#define PS5520_DEF_GAINCEILING  (PS5520_MAX_GAIN_IDX)
#define PS5520_DEF_EXP          (VTS_5M_30 - 3)
#define PS5520_DEF_EXP_CEILING  (VTS_5M_30 - 3)

#define PS5520_L_TARGET         (80)
#define PS5520_L_AGC_DIFF_DIV   (10)
#define PS5520_L_AEC_DIFF_MUL   (10)

static bool enable_agc = true;
static int32_t agc_gain = PS5520_DEF_GAIN;
static int32_t agc_gain_ceiling = PS5520_DEF_GAINCEILING;

static bool enable_aec = true;
static int32_t aec_exposure = PS5520_DEF_EXP;
static int32_t aec_exposure_ceiling = PS5520_MAX_INT;

static const uint8_t sw_reset_regs[][2] = {
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0xED, 0x01 },
    { 0xFF, 0x35 }, // delay more than 1 frame
    { 0xEF, 0x01 },
    { 0x03, 0x02 },
    { 0xFF, 0x35 }, // delay more than 1 frame
    { 0x00, 0x00 }, // End
};

static const uint8_t stream_on_regs[][2] = {
    { 0xEF, 0x01 },
    { 0xF5, 0x10 },
    { 0x09, 0x01 },
    { 0xEF, 0x05 },
    { 0x42, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x09, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0x7F },
    { 0xFF, 0x35 }, // Delay more than 1 frame
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0x00, 0x00 }, // End
};

static const uint8_t stream_off_regs[][2] = {
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0xED, 0x01 },
    { 0xFF, 0x35 }, // Delay more than 1 frame
    { 0xEF, 0x05 },
    { 0x3B, 0x01 },
    { 0x42, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0F },
    { 0xF5, 0x00 },
    { 0x09, 0x01 },
    { 0x00, 0x00 }, // End
};

static const uint8_t res_640x480_regs[][2] = {
    // PS5520_640x480x30fps_24MHz_2Lane_RAW10_840Mbps_20190408_C10A.asc
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0x43, 0x02 },
    { 0x44, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0xF5, 0x01 },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x10, 0x80 },
    { 0x11, 0x80 },
    { 0x35, 0x01 },
    { 0x36, 0x0F },
    { 0x37, 0x0F },
    { 0x38, 0xE0 },
    { 0x5F, 0xC2 },
    { 0x60, 0x2A },
    { 0x61, 0x54 },
    { 0x62, 0x29 },
    { 0x69, 0x10 },
    { 0x6A, 0x40 },
    { 0x85, 0x22 },
    { 0x98, 0x02 },
    { 0x9E, 0x00 },
    { 0xA0, 0x02 },
    { 0xA2, 0x0A },
    { 0xD8, 0x10 },
    { 0xDF, 0x24 },
    { 0xE2, 0x05 },
    { 0xE3, 0x24 },
    { 0xE6, 0x05 },
    { 0xF3, 0xC1 },
    { 0xF8, 0x0A },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x0D, 0x03 },
    { 0x1A, 0x02 },
    { 0x1D, 0x47 },
    { 0x1E, 0xAA },
    { 0x20, 0x00 },
    { 0x2A, 0x56 },
    { 0x37, 0x2C },
    { 0x39, 0x36 },
    { 0x3F, 0xA6 },
    { 0x40, 0x8C },
    { 0x42, 0xF4 },
    { 0x43, 0xD6 },
    { 0x51, 0x28 },
    { 0x5C, 0x1E },
    { 0x5D, 0x0A },
    { 0x68, 0xFA },
    { 0x69, 0xC8 },
    { 0x75, 0x56 },
    { 0x84, 0x10 },
    { 0x85, 0x40 },
    { 0xA3, 0x00 },
    { 0xA4, 0x12 },
    { 0xA5, 0x01 },
    { 0xA6, 0xE0 },
    { 0xA7, 0x00 },
    { 0xA8, 0x06 },
    { 0xA9, 0x02 },
    { 0xAA, 0x80 },
    { 0xAE, 0x50 },
    { 0xB0, 0x50 },
    { 0xC4, 0x54 },
    { 0xC6, 0x10 },
    { 0xC9, 0x55 },
    { 0xCE, 0x30 },
    { 0xD0, 0x02 },
    { 0xD1, 0x50 },
    { 0xD3, 0x01 },
    { 0xD4, 0x04 },
    { 0xD5, 0x61 },
    { 0xD8, 0xA0 },
    { 0xDD, 0x42 },
    { 0xE2, 0x0A },
    { 0xF0, 0x8D },
    { 0xF1, 0x16 },
    { 0xF5, 0x19 },
    { 0x09, 0x01 },
    { 0xEF, 0x02 },
    { 0x2E, 0x04 },
    { 0x33, 0x84 },
    { 0x3C, 0xFA },
    { 0x4E, 0x02 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x06, 0x64 },
    { 0x09, 0x09 },
    { 0x0A, 0x05 },
    { 0x0D, 0x5E },
    { 0x0E, 0x01 },
    { 0x0F, 0x00 },
    { 0x10, 0x02 },
    { 0x11, 0x01 },
    { 0x15, 0x07 },
    { 0x17, 0x06 },
    { 0x18, 0x05 },
    { 0x3B, 0x00 },
    { 0x40, 0x16 },
    { 0x41, 0x28 },
    { 0x43, 0x02 },
    { 0x44, 0x01 },
    { 0x49, 0x01 },
    { 0x4F, 0x01 },
    { 0x5B, 0x10 },
    { 0x94, 0x04 },
    { 0xB0, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x06 },
    { 0x00, 0x0C },
    { 0x02, 0x13 },
    { 0x06, 0x02 },
    { 0x09, 0x02 },
    { 0x0A, 0x15 },
    { 0x0B, 0x90 },
    { 0x0C, 0x90 },
    { 0x0D, 0x90 },
    { 0x0F, 0x1B },
    { 0x10, 0x20 },
    { 0x11, 0x1B },
    { 0x12, 0x20 },
    { 0x18, 0x40 },
    { 0x1A, 0x40 },
    { 0x28, 0x03 },
    { 0x2B, 0x20 },
    { 0x2D, 0x00 },
    { 0x2E, 0x20 },
    { 0x2F, 0x20 },
    { 0x4A, 0x40 },
    { 0x4B, 0x40 },
    { 0x98, 0x05 },
    { 0x99, 0x23 },
    { 0x9A, 0x88 },
    { 0x9E, 0x02 },
    { 0x9F, 0x41 },
    { 0xA3, 0x10 },
    { 0xF1, 0x01 },
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0xFB },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x11, 0x00 },
    { 0xFF, 0x02 }, // Delay 2ms
    { 0x00, 0x00 }, // End
};

static const uint8_t res_1280x720_regs[][2] = {
    // PS5520_1280x720x30fps_24MHz_2Lane_RAW10_840Mbps_20190408_C10A.asc
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0x43, 0x02 },
    { 0x44, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0xF5, 0x01 },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x10, 0x80 },
    { 0x11, 0x80 },
    { 0x35, 0x01 },
    { 0x36, 0x0F },
    { 0x37, 0x0F },
    { 0x38, 0xE0 },
    { 0x5F, 0xC2 },
    { 0x60, 0x2A },
    { 0x61, 0x54 },
    { 0x62, 0x29 },
    { 0x69, 0x10 },
    { 0x6A, 0x40 },
    { 0x85, 0x22 },
    { 0x98, 0x02 },
    { 0x9E, 0x00 },
    { 0xA0, 0x02 },
    { 0xA2, 0x0A },
    { 0xD8, 0x10 },
    { 0xDF, 0x24 },
    { 0xE2, 0x05 },
    { 0xE3, 0x24 },
    { 0xE6, 0x05 },
    { 0xF3, 0xC1 },
    { 0xF8, 0x0A },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x0D, 0x03 },
    { 0x1A, 0x01 },
    { 0x1D, 0x27 },
    { 0x1E, 0xAA },
    { 0x20, 0x00 },
    { 0x2A, 0x56 },
    { 0x37, 0x2C },
    { 0x39, 0x36 },
    { 0x3F, 0xA6 },
    { 0x40, 0x8C },
    { 0x42, 0xF4 },
    { 0x43, 0xD6 },
    { 0x51, 0x28 },
    { 0x5C, 0x1E },
    { 0x5D, 0x0A },
    { 0x68, 0xFA },
    { 0x69, 0xC8 },
    { 0x75, 0x56 },
    { 0x84, 0x10 },
    { 0x85, 0x40 },
    { 0xA3, 0x00 },
    { 0xA4, 0x90 },
    { 0xA5, 0x02 },
    { 0xA6, 0xD0 },
    { 0xA7, 0x00 },
    { 0xA8, 0x0C },
    { 0xA9, 0x05 },
    { 0xAA, 0x00 },
    { 0xAE, 0x50 },
    { 0xB0, 0x50 },
    { 0xC4, 0x54 },
    { 0xC6, 0x10 },
    { 0xC9, 0x55 },
    { 0xCE, 0x30 },
    { 0xD0, 0x02 },
    { 0xD1, 0x50 },
    { 0xD3, 0x01 },
    { 0xD4, 0x04 },
    { 0xD5, 0x61 },
    { 0xD8, 0xA0 },
    { 0xDD, 0x42 },
    { 0xE2, 0x0A },
    { 0xF0, 0x8D },
    { 0xF1, 0x16 },
    { 0xF5, 0x19 },
    { 0x09, 0x01 },
    { 0xEF, 0x02 },
    { 0x2E, 0x04 },
    { 0x33, 0x84 },
    { 0x3C, 0xFA },
    { 0x4E, 0x02 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x06, 0x64 },
    { 0x09, 0x09 },
    { 0x0A, 0x05 },
    { 0x0D, 0x5E },
    { 0x0E, 0x01 },
    { 0x0F, 0x00 },
    { 0x10, 0x02 },
    { 0x11, 0x01 },
    { 0x15, 0x07 },
    { 0x17, 0x06 },
    { 0x18, 0x05 },
    { 0x3B, 0x00 },
    { 0x40, 0x16 },
    { 0x41, 0x28 },
    { 0x43, 0x02 },
    { 0x44, 0x01 },
    { 0x49, 0x01 },
    { 0x4F, 0x01 },
    { 0x5B, 0x10 },
    { 0x94, 0x04 },
    { 0xB0, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x06 },
    { 0x00, 0x0C },
    { 0x02, 0x13 },
    { 0x06, 0x02 },
    { 0x09, 0x02 },
    { 0x0A, 0x15 },
    { 0x0B, 0x90 },
    { 0x0C, 0x90 },
    { 0x0D, 0x90 },
    { 0x0F, 0x1B },
    { 0x10, 0x20 },
    { 0x11, 0x1B },
    { 0x12, 0x20 },
    { 0x18, 0x40 },
    { 0x1A, 0x40 },
    { 0x28, 0x03 },
    { 0x2B, 0x20 },
    { 0x2D, 0x00 },
    { 0x2E, 0x20 },
    { 0x2F, 0x20 },
    { 0x4A, 0x40 },
    { 0x4B, 0x40 },
    { 0x98, 0x05 },
    { 0x99, 0x23 },
    { 0x9A, 0x88 },
    { 0x9E, 0x02 },
    { 0x9F, 0x40 },
    { 0xA3, 0x10 },
    { 0xF1, 0x01 },
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0xFB },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x11, 0x00 },
    { 0xFF, 0x02 }, // Delay 2ms
    { 0x00, 0x00 }, // End
};

static const uint8_t res_1280x960_regs[][2] = {
    // PS5520_1280x960x30fps_24MHz_2Lane_RAW10_840Mbps_20190408_C10A.asc
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0x43, 0x02 },
    { 0x44, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0xF5, 0x01 },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x10, 0x80 },
    { 0x11, 0x80 },
    { 0x35, 0x01 },
    { 0x36, 0x0F },
    { 0x37, 0x0F },
    { 0x38, 0xE0 },
    { 0x5F, 0xC2 },
    { 0x60, 0x2A },
    { 0x61, 0x54 },
    { 0x62, 0x29 },
    { 0x69, 0x10 },
    { 0x6A, 0x40 },
    { 0x85, 0x22 },
    { 0x98, 0x02 },
    { 0x9E, 0x00 },
    { 0xA0, 0x02 },
    { 0xA2, 0x0A },
    { 0xD8, 0x10 },
    { 0xDF, 0x24 },
    { 0xE2, 0x05 },
    { 0xE3, 0x24 },
    { 0xE6, 0x05 },
    { 0xF3, 0xC1 },
    { 0xF8, 0x0A },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x0D, 0x03 },
    { 0x1A, 0x01 },
    { 0x1D, 0x27 },
    { 0x1E, 0xAA },
    { 0x20, 0x00 },
    { 0x2A, 0x56 },
    { 0x37, 0x2C },
    { 0x39, 0x36 },
    { 0x3F, 0xA6 },
    { 0x40, 0x8C },
    { 0x42, 0xF4 },
    { 0x43, 0xD6 },
    { 0x51, 0x28 },
    { 0x5C, 0x1E },
    { 0x5D, 0x0A },
    { 0x68, 0xFA },
    { 0x69, 0xC8 },
    { 0x75, 0x56 },
    { 0x84, 0x10 },
    { 0x85, 0x40 },
    { 0xA3, 0x00 },
    { 0xA4, 0x18 },
    { 0xA5, 0x03 },
    { 0xA6, 0xC0 },
    { 0xA7, 0x00 },
    { 0xA8, 0x0C },
    { 0xA9, 0x05 },
    { 0xAA, 0x00 },
    { 0xAE, 0x50 },
    { 0xB0, 0x50 },
    { 0xC4, 0x54 },
    { 0xC6, 0x10 },
    { 0xC9, 0x55 },
    { 0xCE, 0x30 },
    { 0xD0, 0x02 },
    { 0xD1, 0x50 },
    { 0xD3, 0x01 },
    { 0xD4, 0x04 },
    { 0xD5, 0x61 },
    { 0xD8, 0xA0 },
    { 0xDD, 0x42 },
    { 0xE2, 0x0A },
    { 0xF0, 0x8D },
    { 0xF1, 0x16 },
    { 0xF5, 0x19 },
    { 0x09, 0x01 },
    { 0xEF, 0x02 },
    { 0x2E, 0x04 },
    { 0x33, 0x84 },
    { 0x3C, 0xFA },
    { 0x4E, 0x02 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x06, 0x64 },
    { 0x09, 0x09 },
    { 0x0A, 0x05 },
    { 0x0D, 0x5E },
    { 0x0E, 0x01 },
    { 0x0F, 0x00 },
    { 0x10, 0x02 },
    { 0x11, 0x01 },
    { 0x15, 0x07 },
    { 0x17, 0x06 },
    { 0x18, 0x05 },
    { 0x3B, 0x00 },
    { 0x40, 0x16 },
    { 0x41, 0x28 },
    { 0x43, 0x02 },
    { 0x44, 0x01 },
    { 0x49, 0x01 },
    { 0x4F, 0x01 },
    { 0x5B, 0x10 },
    { 0x94, 0x04 },
    { 0xB0, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x06 },
    { 0x00, 0x0C },
    { 0x02, 0x13 },
    { 0x06, 0x02 },
    { 0x09, 0x02 },
    { 0x0A, 0x15 },
    { 0x0B, 0x90 },
    { 0x0C, 0x90 },
    { 0x0D, 0x90 },
    { 0x0F, 0x1B },
    { 0x10, 0x20 },
    { 0x11, 0x1B },
    { 0x12, 0x20 },
    { 0x18, 0x40 },
    { 0x1A, 0x40 },
    { 0x28, 0x03 },
    { 0x2B, 0x20 },
    { 0x2D, 0x00 },
    { 0x2E, 0x20 },
    { 0x2F, 0x20 },
    { 0x4A, 0x40 },
    { 0x4B, 0x40 },
    { 0x98, 0x05},
    { 0x99, 0x23 },
    { 0x9A, 0x88 },
    { 0x9E, 0x02 },
    { 0x9F, 0x40 },
    { 0xA3, 0x10 },
    { 0xF1, 0x01 },
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0xFB },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x11, 0x00 },
    { 0xFF, 0x02 }, // Delay 2ms
    { 0x00, 0x00 }, // End
};

static const uint8_t res_1920x1080_regs[][2] = {
    // PS5520_1920x1080x30fps_24MHz_2Lane_RAW10_840Mbps_20190408_C10A.asc
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0x43, 0x02 },
    { 0x44, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0xF5, 0x01 },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x10, 0x80 },
    { 0x11, 0x80 },
    { 0x35, 0x01 },
    { 0x36, 0x0F },
    { 0x37, 0x0F },
    { 0x38, 0xE0 },
    { 0x5F, 0xC2 },
    { 0x60, 0x2A },
    { 0x61, 0x54 },
    { 0x62, 0x29 },
    { 0x69, 0x10 },
    { 0x6A, 0x40 },
    { 0x85, 0x22 },
    { 0x98, 0x02 },
    { 0x9E, 0x00 },
    { 0xA0, 0x02 },
    { 0xA2, 0x0A },
    { 0xD8, 0x10 },
    { 0xDF, 0x24 },
    { 0xE2, 0x05 },
    { 0xE3, 0x24 },
    { 0xE6, 0x05 },
    { 0xF3, 0xC1 },
    { 0xF8, 0x0A },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x0D, 0x03 },
    { 0x1A, 0x00 },
    { 0x1D, 0x07 },
    { 0x1E, 0xAA },
    { 0x20, 0x02 },
    { 0x2A, 0x56 },
    { 0x37, 0x2C },
    { 0x39, 0x36 },
    { 0x3F, 0xA6 },
    { 0x40, 0x8C },
    { 0x42, 0xF4 },
    { 0x43, 0xD6 },
    { 0x51, 0x28 },
    { 0x5C, 0x1E },
    { 0x5D, 0x0A },
    { 0x68, 0xFA },
    { 0x69, 0xC8 },
    { 0x75, 0x56 },
    { 0x84, 0x12 },
    { 0x85, 0xC0 },
    { 0xA3, 0x01 },
    { 0xA4, 0xC6 },
    { 0xA5, 0x04 },
    { 0xA6, 0x38 },
    { 0xA7, 0x01 },
    { 0xA8, 0x58 },
    { 0xA9, 0x07 },
    { 0xAA, 0x80 },
    { 0xAE, 0x50 },
    { 0xB0, 0x50 },
    { 0xC4, 0x54 },
    { 0xC6, 0x10 },
    { 0xC9, 0x55 },
    { 0xCE, 0x30 },
    { 0xD0, 0x02 },
    { 0xD1, 0x50 },
    { 0xD3, 0x01 },
    { 0xD4, 0x04 },
    { 0xD5, 0x61 },
    { 0xD8, 0xA0 },
    { 0xDD, 0x42 },
    { 0xE2, 0x0A },
    { 0xF0, 0x8D },
    { 0xF1, 0x16 },
    { 0xF5, 0x19 },
    { 0x09, 0x01 },
    { 0xEF, 0x02 },
    { 0x2E, 0x04 },
    { 0x33, 0x84 },
    { 0x3C, 0xFA },
    { 0x4E, 0x02 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x06, 0x64 },
    { 0x09, 0x09 },
    { 0x0A, 0x05 },
    { 0x0D, 0x5E },
    { 0x0E, 0x01 },
    { 0x0F, 0x00 },
    { 0x10, 0x02 },
    { 0x11, 0x01 },
    { 0x15, 0x07 },
    { 0x17, 0x06 },
    { 0x18, 0x05 },
    { 0x3B, 0x00 },
    { 0x40, 0x16 },
    { 0x41, 0x28 },
    { 0x43, 0x02 },
    { 0x44, 0x01 },
    { 0x49, 0x01 },
    { 0x4F, 0x01 },
    { 0x5B, 0x10 },
    { 0x94, 0x04 },
    { 0xB0, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x06 },
    { 0x00, 0x0C },
    { 0x02, 0x13 },
    { 0x06, 0x02 },
    { 0x09, 0x02 },
    { 0x0A, 0x15 },
    { 0x0B, 0x90 },
    { 0x0C, 0x90 },
    { 0x0D, 0x90 },
    { 0x0F, 0x1B },
    { 0x10, 0x20 },
    { 0x11, 0x1B },
    { 0x12, 0x20 },
    { 0x18, 0x40 },
    { 0x1A, 0x40 },
    { 0x28, 0x03 },
    { 0x2B, 0x20 },
    { 0x2D, 0x00 },
    { 0x2E, 0x20 },
    { 0x2F, 0x20 },
    { 0x4A, 0x40 },
    { 0x4B, 0x40 },
    { 0x98, 0x05 },
    { 0x99, 0x23 },
    { 0x9A, 0x88 },
    { 0x9E, 0x42 },
    { 0x9F, 0x44 },
    { 0xF1, 0x01 },
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0xFB },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x11, 0x00 },
    { 0xFF, 0x02 }, // Delay 2 ms
    { 0x00, 0x00 }, // End
};

static const uint8_t res_2560x1440_regs[][2] = {
    // PS5520_2560x1440x30fps_24MHz_2Lane_RAW10_840Mbps_20190408_C10A.asc
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0x43, 0x02 },
    { 0x44, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0xF5, 0x01 },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x10, 0x80 },
    { 0x11, 0x80 },
    { 0x35, 0x01 },
    { 0x36, 0x0F },
    { 0x37, 0x0F },
    { 0x38, 0xE0 },
    { 0x5F, 0xC2 },
    { 0x60, 0x2A },
    { 0x61, 0x54 },
    { 0x62, 0x29 },
    { 0x69, 0x10 },
    { 0x6A, 0x40 },
    { 0x85, 0x22 },
    { 0x98, 0x02 },
    { 0x9E, 0x00 },
    { 0xA0, 0x02 },
    { 0xA2, 0x0A },
    { 0xD8, 0x10 },
    { 0xDF, 0x24 },
    { 0xE2, 0x05 },
    { 0xE3, 0x24 },
    { 0xE6, 0x05 },
    { 0xF3, 0xC1 },
    { 0xF8, 0x0A },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x0D, 0x03 },
    { 0x1A, 0x00 },
    { 0x1D, 0x07 },
    { 0x1E, 0xAA },
    { 0x20, 0x02 },
    { 0x2A, 0x56 },
    { 0x37, 0x2C },
    { 0x39, 0x36 },
    { 0x3F, 0xA6 },
    { 0x40, 0x8C },
    { 0x42, 0xF4 },
    { 0x43, 0xD6 },
    { 0x51, 0x28 },
    { 0x5C, 0x1E },
    { 0x5D, 0x0A },
    { 0x68, 0xFA },
    { 0x69, 0xC8 },
    { 0x75, 0x56 },
    { 0x84, 0x12 },
    { 0x85, 0xC0 },
    { 0xA3, 0x01 },
    { 0xA4, 0x12 },
    { 0xA5, 0x05 },
    { 0xA6, 0xA0 },
    { 0xA7, 0x00 },
    { 0xA8, 0x18 },
    { 0xA9, 0x07 },
    { 0xAA, 0x00 },
    { 0xAE, 0x50 },
    { 0xB0, 0x50 },
    { 0xC4, 0x54 },
    { 0xC6, 0x10 },
    { 0xC9, 0x55 },
    { 0xCE, 0x30 },
    { 0xD0, 0x02 },
    { 0xD1, 0x50 },
    { 0xD3, 0x01 },
    { 0xD4, 0x04 },
    { 0xD5, 0x61 },
    { 0xD8, 0xA0 },
    { 0xDD, 0x42 },
    { 0xE2, 0x0A },
    { 0xF0, 0x8D },
    { 0xF1, 0x16 },
    { 0xF5, 0x19 },
    { 0x09, 0x01 },
    { 0xEF, 0x02 },
    { 0x2E, 0x04 },
    { 0x33, 0x84 },
    { 0x3C, 0xFA },
    { 0x4E, 0x02 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x06, 0x64 },
    { 0x09, 0x09 },
    { 0x0A, 0x05 },
    { 0x0D, 0x5E },
    { 0x0E, 0x01 },
    { 0x0F, 0x00 },
    { 0x10, 0x02 },
    { 0x11, 0x01 },
    { 0x15, 0x07 },
    { 0x17, 0x06 },
    { 0x18, 0x05 },
    { 0x3B, 0x00 },
    { 0x40, 0x16 },
    { 0x41, 0x28 },
    { 0x43, 0x02 },
    { 0x44, 0x01 },
    { 0x49, 0x01 },
    { 0x4F, 0x01 },
    { 0x5B, 0x10 },
    { 0x94, 0x04 },
    { 0xB0, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x06 },
    { 0x00, 0x0C },
    { 0x02, 0x13 },
    { 0x06, 0x02 },
    { 0x09, 0x02 },
    { 0x0A, 0x15 },
    { 0x0B, 0x90 },
    { 0x0C, 0x90 },
    { 0x0D, 0x90 },
    { 0x0F, 0x1B },
    { 0x10, 0x20 },
    { 0x11, 0x1B },
    { 0x12, 0x20 },
    { 0x18, 0x40 },
    { 0x1A, 0x40 },
    { 0x28, 0x03 },
    { 0x2B, 0x20 },
    { 0x2D, 0x00 },
    { 0x2E, 0x20 },
    { 0x2F, 0x20 },
    { 0x4A, 0x40 },
    { 0x4B, 0x40 },
    { 0x98, 0x05 },
    { 0x99, 0x23 },
    { 0x9A, 0x88 },
    { 0x9E, 0x42 },
    { 0x9F, 0x44 },
    { 0xF1, 0x01 },
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0xFB },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x11, 0x00 },
    { 0xFF, 0x02 }, // Delay 2 ms
    { 0x00, 0x00 }, // End
};

static const uint8_t res_2592x1944_regs[][2] = {
    // PS5520_2592x1944x30fps_24MHz_2Lane_RAW10_840Mbps_20190408_C10A.asc
    { 0xEF, 0x05 },
    { 0x0F, 0x00 },
    { 0x43, 0x02 },
    { 0x44, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0xF5, 0x01 },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x10, 0x80 },
    { 0x11, 0x80 },
    { 0x35, 0x01 },
    { 0x36, 0x0F },
    { 0x37, 0x0F },
    { 0x38, 0xE0 },
    { 0x5F, 0xC2 },
    { 0x60, 0x2A },
    { 0x61, 0x54 },
    { 0x62, 0x29 },
    { 0x69, 0x10 },
    { 0x6A, 0x40 },
    { 0x85, 0x22 },
    { 0x98, 0x02 },
    { 0x9E, 0x00 },
    { 0xA0, 0x02 },
    { 0xA2, 0x0A },
    { 0xD8, 0x10 },
    { 0xDF, 0x24 },
    { 0xE2, 0x05 },
    { 0xE3, 0x24 },
    { 0xE6, 0x05 },
    { 0xF3, 0xC1 },
    { 0xF8, 0x0A },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x05, 0x0B },
    { 0x0D, 0x03 },
    { 0x1A, 0x00 },
    { 0x1D, 0x07 },
    { 0x1E, 0xAA },
    { 0x20, 0x02 },
    { 0x2A, 0x56 },
    { 0x37, 0x2C },
    { 0x39, 0x36 },
    { 0x3F, 0xA6 },
    { 0x40, 0x8C },
    { 0x42, 0xF4 },
    { 0x43, 0xD6 },
    { 0x51, 0x28 },
    { 0x5C, 0x1E },
    { 0x5D, 0x0A },
    { 0x68, 0xFA },
    { 0x69, 0xC8 },
    { 0x75, 0x56 },
    { 0x84, 0x12 },
    { 0x85, 0xC0 },
    { 0xA3, 0x00 },
    { 0xA4, 0x16 },
    { 0xA5, 0x07 },
    { 0xA6, 0x98 },
    { 0xA7, 0x00 },
    { 0xA8, 0x08 },
    { 0xA9, 0x0A },
    { 0xAA, 0x20 },
    { 0xAE, 0x50 },
    { 0xB0, 0x50 },
    { 0xC4, 0x54 },
    { 0xC6, 0x10 },
    { 0xC9, 0x55 },
    { 0xCE, 0x30 },
    { 0xD0, 0x02 },
    { 0xD1, 0x50 },
    { 0xD3, 0x01 },
    { 0xD4, 0x04 },
    { 0xD5, 0x61 },
    { 0xD8, 0xA0 },
    { 0xDD, 0x42 },
    { 0xE2, 0x0A },
    { 0xF0, 0x8D },
    { 0xF1, 0x16 },
    { 0xF5, 0x19 },
    { 0x09, 0x01 },
    { 0xEF, 0x02 },
    { 0x2E, 0x04 },
    { 0x33, 0x84 },
    { 0x3C, 0xFA },
    { 0x4E, 0x02 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x06, 0x64 },
    { 0x09, 0x09 },
    { 0x0A, 0x05 },
    { 0x0D, 0x5E },
    { 0x0E, 0x01 },
    { 0x0F, 0x00 },
    { 0x10, 0x02 },
    { 0x11, 0x01 },
    { 0x15, 0x07 },
    { 0x17, 0x06 },
    { 0x18, 0x05 },
    { 0x3B, 0x00 },
    { 0x40, 0x16 },
    { 0x41, 0x28 },
    { 0x43, 0x02 },
    { 0x44, 0x01 },
    { 0x49, 0x01 },
    { 0x4F, 0x01 },
    { 0x5B, 0x10 },
    { 0x94, 0x04 },
    { 0xB0, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x06 },
    { 0x00, 0x0C },
    { 0x02, 0x13 },
    { 0x06, 0x02 },
    { 0x09, 0x02 },
    { 0x0A, 0x15 },
    { 0x0B, 0x90 },
    { 0x0C, 0x90 },
    { 0x0D, 0x90 },
    { 0x0F, 0x1B },
    { 0x10, 0x20 },
    { 0x11, 0x1B },
    { 0x12, 0x20 },
    { 0x18, 0x40 },
    { 0x1A, 0x40 },
    { 0x28, 0x03 },
    { 0x2B, 0x20 },
    { 0x2D, 0x00 },
    { 0x2E, 0x20 },
    { 0x2F, 0x20 },
    { 0x4A, 0x40 },
    { 0x4B, 0x40 },
    { 0x98, 0x05 },
    { 0x99, 0x23 },
    { 0x9A, 0x88 },
    { 0x9E, 0x42 },
    { 0x9F, 0x44 },
    { 0xF1, 0x01 },
    { 0xEF, 0x05 },
    { 0x3B, 0x00 },
    { 0xED, 0x01 },
    { 0xEF, 0x05 },
    { 0x0F, 0x01 },
    { 0xED, 0x01 },
    { 0xEF, 0x01 },
    { 0x02, 0xFB },
    { 0x09, 0x01 },
    { 0xEF, 0x00 },
    { 0x11, 0x00 },
    { 0xFF, 0x02 }, // Delay 2 ms
    { 0x00, 0x00 }, // End
};

#define EXP_LPF_MIN     (VTS_5M_30)
#define EXP_LPF_MAX     (3952 - 1)

typedef struct exp_tbl {
    uint32_t idx;
    uint16_t lpf;
    uint8_t np;
    uint8_t div;
} exp_tbl_t;

static exp_tbl_t gu16ExpTbl[] = {
    // IDX              LPF,                    NP  DIV
    {EXP_LPF_MIN,       EXP_LPF_MIN,            1,  1},     // 30fps
    {EXP_LPF_MAX,       EXP_LPF_MAX,            1,  1},     // 15fps
    {EXP_LPF_MIN * 2,   EXP_LPF_MIN,            2,  2},     // 15fps
    {EXP_LPF_MIN * 3,   EXP_LPF_MIN * 3 / 2,    2,  2},     // 10fps
    {EXP_LPF_MIN * 3,   EXP_LPF_MIN,            3,  3},     // 10fps
    {EXP_LPF_MIN * 5,   EXP_LPF_MIN * 5 / 3,    3,  3},     // 6fps
    {EXP_LPF_MIN * 5,   EXP_LPF_MIN,            5,  5},     // 6fps
    {EXP_LPF_MIN * 8,   EXP_LPF_MIN * 8 / 5,    11, 5},     // 3.75fps
    {EXP_LPF_MIN * 8,   EXP_LPF_MIN,            12, 8},     // 3.75fps
    {EXP_LPF_MIN * 10,  EXP_LPF_MIN * 10 / 8,   12, 8},     // 3fps
    {EXP_LPF_MIN * 10,  EXP_LPF_MIN,            13, 10},    // 3fps
    {EXP_LPF_MAX * 10,  EXP_LPF_MAX,            13, 10},    // 1.5fps
    {0xFFFFFF,          EXP_LPF_MAX,            13, 10},    // End
};

#define __INTERPOLATE__(lpf, off) ({                                   \
        uint32_t idx_l = gu16ExpTbl[off - 1].idx;                      \
        uint32_t lpf_l = gu16ExpTbl[off - 1].lpf;                      \
        uint32_t idx_h = gu16ExpTbl[off].idx;                          \
        uint32_t lpf_h = gu16ExpTbl[off].lpf;                          \
        lpf_l + (((lpf - idx_l) * (lpf_h - lpf_l)) / (idx_h - idx_l)); \
    })

#define EXP_TBL_INTERPOLATE(lpf, cnt) ({                   \
        (gu16ExpTbl[cnt].lpf == gu16ExpTbl[cnt - 1].lpf) ? \
        gu16ExpTbl[cnt].lpf : __INTERPOLATE__(lpf, cnt);   \
    })

static int write_registers(omv_csi_t *csi, const uint8_t(*regs)[2]);
static int get_exposure_us(omv_csi_t *csi, int *exposure_us);

static int reset(omv_csi_t *csi) {
    int ret = 0;
    uint8_t exposure_line_h;
    uint8_t exposure_line_l;
    int16_t lpf;

    // Set resolution
    ret |= write_registers(csi, res_640x480_regs);

    enable_agc = true;
    agc_gain = PS5520_DEF_GAIN;
    agc_gain_ceiling = PS5520_DEF_GAINCEILING;

    enable_aec = true;
    aec_exposure = PS5520_DEF_EXP;
    aec_exposure_ceiling = PS5520_DEF_EXP_CEILING;

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_LPF_H, 1, &exposure_line_h, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_LPF_L, 1, &exposure_line_l, 1);
    lpf = (exposure_line_h << 8) + exposure_line_l; // Cmd_Lpf

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);

    // Set default gain
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_GAIN_IDX, 1, agc_gain, 1);

    // Set default exposure
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_H, 1, (lpf - 1 - aec_exposure) >> 8, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_L, 1, (lpf - 1 - aec_exposure) & 0xFF, 1);

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);

    return ret;
}

static int sleep(omv_csi_t *csi, int enable) {
    return write_registers(csi, enable ? stream_off_regs : stream_on_regs);
}

static int read_reg(omv_csi_t *csi, uint16_t reg) {
    uint8_t reg_data;
    if (omv_i2c_read_reg(csi->i2c, csi->slv_addr, reg, 1, &reg_data, 1) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg, uint16_t reg_data) {
    return omv_i2c_write_reg(csi->i2c, csi->slv_addr, reg, 1, reg_data, 1);
}

static int write_registers(omv_csi_t *csi, const uint8_t(*regs)[2]) {
    int ret = 0;

    for (int i = 0; !ret; i++) {
        if (!regs[i][0] && !regs[i][1]) {
            break;
        } else if (regs[i][0] == 0xFF) {
            mp_hal_delay_ms(regs[i][1]);
        } else {
            ret |= write_reg(csi, regs[i][0], regs[i][1]);
            if (ret) {
                ret = 0;
                printf("failed to write reg: 0x%x value: 0x%x\n", regs[i][0], regs[i][1]);
            }
        }
    }

    #if 0
    for (int i = 0; !ret; i++) {
        if (!regs[i][0] && !regs[i][1]) {
            break;
        } else if (regs[i][0] != 0xFF) {
            write_reg(csi, regs[i][0], regs[i][1]);
            int reg = read_reg(csi, regs[i][0]);
            if (ret) {
                printf("failed to read reg: 0x%x\n", regs[i][0]);
                ret = 0;
            }
            if (reg != regs[i][1]) {
                printf("reg: 0x%x exp: 0x%x got: 0x%x\n", regs[i][0], regs[i][1], reg);
            }
        }
    }
    #endif

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

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    int ret = 0;
    const uint8_t(*regs)[2];

    switch (framesize) {
        case OMV_CSI_FRAMESIZE_VGA:
            regs = res_640x480_regs;
            break;
        case OMV_CSI_FRAMESIZE_HD:
            regs = res_1280x720_regs;
            break;
        case OMV_CSI_FRAMESIZE_SXGAM:
            regs = res_1280x960_regs;
            break;
        case OMV_CSI_FRAMESIZE_FHD:
            regs = res_1920x1080_regs;
            break;
        case OMV_CSI_FRAMESIZE_QHD:
            regs = res_2560x1440_regs;
            break;
        case OMV_CSI_FRAMESIZE_WQXGA2:
            regs = res_2592x1944_regs;
            break;
        default:
            return -1;
    }

    // SW reset
    ret |= write_registers(csi, sw_reset_regs);

    // Set resolution
    ret |= write_registers(csi, regs);
    return ret;
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    int ret = 0;
    int cnt;
    int exposure_us;
    int32_t lpf;
    int8_t np;
    bool flg_stall = 0;

    framerate = IM_MIN(framerate, PS5520_FPS_MAX);

    ret |= get_exposure_us(csi, &exposure_us);

    lpf = (PIX_CLK / HTS_5M_30) / framerate; // LPF (Cmd_Lpf+1)
    lpf = IM_CLAMP(lpf, VTS_5M_30, EXP_LPF_MAX * 10); // LPF (Cmd_Lpf+1)

    for (cnt = 0; gu16ExpTbl[cnt].idx < 0xFFFFFF; cnt++) {
        if (lpf < gu16ExpTbl[cnt].idx) {
            break;
        }
    }

    if (cnt == 2) {
        lpf = EXP_LPF_MIN * 2;
    }
    lpf = EXP_TBL_INTERPOLATE(lpf, cnt) - 1; // Cmd_Lpf
    np = gu16ExpTbl[cnt].np; // Cmd_Np

    if (g_div != gu16ExpTbl[cnt].div) {
        g_div = gu16ExpTbl[cnt].div;
        flg_stall = 1;
    }

    int16_t exposure_line = ConvertT2L(exposure_us);

    exposure_line = IM_CLAMP(exposure_line, PS5520_MIN_INT, (lpf - 2));

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_LPF_H, 1, lpf >> 8, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_LPF_L, 1, lpf & 0xFF, 1);

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_H, 1, (lpf - exposure_line) >> 8, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_L, 1, (lpf - exposure_line) & 0xFF, 1);

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_NP, 1, np & 0xFF, 1);

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);

    if (flg_stall == 1) {
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x05, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, 0x25, 1, 0x01, 1);
        mp_hal_delay_ms(35 * g_div);  // delay over 1 frame time
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, 0x25, 1, 0x00, 1);
    }

    return ret;
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    int ret = -1; // No AGC function of PS5520

    return ret;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    int ret = 0;
    uint8_t reg;

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x00, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_OUTGEN, 1, &reg, 1);

    reg = enable ? (reg | 0x60) : (reg & 0x8F);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OUTGEN, 1, reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, R_ISP_TESTMODE, 1, &reg, 1);

    reg = enable ? (reg | 0x06) : (reg & 0xE0);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, R_ISP_TESTMODE, 1, reg, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);

    return ret;
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    int ret = 0;
    int idx = 0;

    enable_agc = enable;

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        int gain = fast_roundf(expf((gain_db / 20.0f) * M_LN10) * PS5520_GAIN_SCALE_F);
        gain = IM_CLAMP(gain, PS5520_MIN_AGAIN_REG, PS5520_MAX_AGAIN_REG);

        for (idx = 0; gain >> (4 + idx); idx++) {
        }

        agc_gain = ((idx - 1) << 4) + ((gain >> (idx - 1)) & 0x0F);

        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_GAIN_IDX, 1, agc_gain, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        int gain = fast_roundf(expf((gain_db_ceiling / 20.0f) * M_LN10) * PS5520_GAIN_SCALE_F);
        gain = IM_CLAMP(gain, PS5520_MIN_AGAIN_REG, PS5520_MAX_AGAIN_REG);

        for (idx = 0; gain >> (4 + idx); idx++) {
        }

        agc_gain_ceiling = ((idx - 1) << 4) + ((gain >> (idx - 1)) & 0x0F);
    }

    return ret;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    uint8_t gain;
    int ret = omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_GAIN_IDX, 1, &gain, 1);

    *gain_db = 20.0f * log10f(PS5520_GAIN((int) gain) / PS5520_GAIN_SCALE_F);

    return ret;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    int ret = 0;
    int16_t lpf;
    uint8_t exposure_line_h;
    uint8_t exposure_line_l;

    enable_aec = enable;

    if ((enable == 0) && (exposure_us >= 0)) {
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);

        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_LPF_H, 1, &exposure_line_h, 1);
        ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_LPF_L, 1, &exposure_line_l, 1);
        lpf = (exposure_line_h << 8) + exposure_line_l; // Cmd_Lpf

        int32_t exposure_line = ConvertT2L(exposure_us);
        aec_exposure = IM_CLAMP(exposure_line, PS5520_MIN_INT, (lpf - 2));

        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_H, 1, (lpf - aec_exposure) >> 8, 1);
        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_L, 1, (lpf - aec_exposure) & 0xFF, 1);

        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);
    } else if ((enable != 0) && (exposure_us >= 0)) {
        aec_exposure_ceiling = ConvertT2LineBase(exposure_us);
        aec_exposure_ceiling = IM_CLAMP(aec_exposure_ceiling, PS5520_MIN_INT, PS5520_MAX_INT);
    }

    return ret;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    int ret = 0;
    uint16_t lpf;
    uint8_t exposure_line_h;
    uint8_t exposure_line_l;

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_LPF_H, 1, &exposure_line_h, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_LPF_L, 1, &exposure_line_l, 1);
    lpf = (exposure_line_h << 8) + exposure_line_l; // Cmd_Lpf

    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_H, 1, &exposure_line_h, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_L, 1, &exposure_line_l, 1);

    int16_t exposure_line = lpf - ((exposure_line_h << 8) + exposure_line_l);
    *exposure_us = ConvertL2T(exposure_line);

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
    uint8_t reg1, reg2;
    uint16_t hsize;

    int ret = omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_HFLIP, 1, &reg1, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_HSIZE_E1, 1, &reg2, 1);

    hsize = ((reg1 & 0x7F) << 8) + reg2 - (reg1 >> 7) + (enable?0x7FFF:0);
    reg1 = hsize >> 8;
    reg2 = hsize & 0xFF;
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_HFLIP, 1, reg1, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_HSIZE_E1, 1, reg2, 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);

    return ret;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    uint8_t reg1;

    int ret = omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);
    ret |= omv_i2c_read_reg(csi->i2c, csi->slv_addr, CMD_VFLIP, 1, &reg1, 1);

    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_VFLIP, 1, (reg1 & 0x7F) | ((enable & 0x01) << 7), 1);
    ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);

    return ret;
}

static int update_agc_aec(omv_csi_t *csi, int luminance) {
    int ret = 0;
    int diff = PS5520_L_TARGET - luminance;

    if (abs(diff) > 0) {
        bool aec_exposure_in = ((diff > 0) && (aec_exposure < aec_exposure_ceiling)) ||
                               ((diff < 0) && (agc_gain <= PS5520_MIN_GAIN_IDX));

        ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);

        if (enable_aec && aec_exposure_in) {
            // Long exposure first for better SNR
            int32_t lpf;
            uint8_t np;
            bool flg_stall = 0;
            int32_t exposure_line;

            aec_exposure += diff * PS5520_L_AEC_DIFF_MUL;
            aec_exposure = IM_CLAMP(aec_exposure, PS5520_MIN_INT, aec_exposure_ceiling);

            if (aec_exposure > (VTS_5M_30 - 1 - 2)) {
                int cnt = 0;

                lpf = aec_exposure + 2 + 1;

                for (cnt = 0; gu16ExpTbl[cnt].idx < 0xFFFFFF; cnt++) {
                    if (lpf < gu16ExpTbl[cnt].idx) {
                        break;
                    }
                }

                if (cnt == 2) {
                    lpf = EXP_LPF_MIN * 2;
                }
                lpf = EXP_TBL_INTERPOLATE(lpf, cnt) - 1; // Cmd_Lpf
                np = gu16ExpTbl[cnt].np; // Cmd_Np
                exposure_line = lpf - 2;

                if (g_div != gu16ExpTbl[cnt].div) {
                    g_div = gu16ExpTbl[cnt].div;
                    flg_stall = 1;
                }
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_NP, 1, np & 0xFF, 1);

            } else {
                lpf = VTS_5M_30 - 1;
                exposure_line = aec_exposure;
                exposure_line = IM_CLAMP(exposure_line, PS5520_MIN_INT, (lpf - 2));
            }
            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_LPF_H, 1, lpf >> 8, 1);
            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_LPF_L, 1, lpf & 0xFF, 1);

            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_H, 1, (lpf - exposure_line) >> 8, 1);
            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_OFFNY1_L, 1, (lpf - exposure_line) & 0xFF, 1);

            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);

            if (flg_stall == 1) {
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x05, 1);
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, 0x25, 1, 0x01, 1);
                mp_hal_delay_ms(35 * g_div);  // delay over 1 frame time
                ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, 0x25, 1, 0x00, 1);
            }

        } else if (enable_agc) {
            agc_gain += diff / PS5520_L_AGC_DIFF_DIV;
            agc_gain = IM_CLAMP(agc_gain, PS5520_MIN_GAIN_IDX, agc_gain_ceiling);

            //ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, REG_BANK, 1, 0x01, 1);
            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, CMD_GAIN_IDX, 1, agc_gain, 1);
            ret |= omv_i2c_write_reg(csi->i2c, csi->slv_addr, SENSOR_UPDATE, 1, 0x01, 1);
        }
    }

    return ret;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;

    switch (request) {
        case OMV_CSI_IOCTL_UPDATE_AGC_AEC:
            ret = update_agc_aec(csi, va_arg(ap, int));
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

int ps5520_init(omv_csi_t *csi) {
    // Initialize csi flags.
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->mono_bpp = 1;
    csi->raw_output = 1;
    csi->cfa_format = SUBFORMAT_ID_BGGR;
    csi->mipi_if = 1;
    csi->mipi_brate = 850;

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

    return 0;
}
#endif // (OMV_PS5520_ENABLE == 1)
