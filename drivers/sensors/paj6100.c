/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2021 Pixart Inc.
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
 * Author: Lake Fu at <lake_fu@pixart.com>
 */
#include "omv_boardconfig.h"
#if (OMV_PAJ6100_ENABLE == 1)
#include <stdio.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "omv_csi.h"
#include "framebuffer.h"
#include "omv_gpio.h"

#include "paj6100.h"
#include "paj6100_reg.h"
#include "pixspi.h"

#define CACHE_BANK

#ifdef DEBUG
// For dump initial result before connect to IDE,
// we use a global variable to store it.
static int8_t init_res;
#endif

static int16_t bank_cache = -1;

// Exposure time related parameters +
#define QVGA_MAX_EXPO_PA         85161
#define QQVGA_MAX_EXPO_PA        25497

#define R_AE_MinGain             1
#define R_AE_MaxGain             8

static float R_FrameTime = 6000000 / 30;
static long R_AE_MaxExpoTime;
static long R_AE_MinExpoTime = 120 / 6 + 0.5f;

#define CAL_MAX_EXPO_TIME(PA)    (fast_roundf(((float) (R_FrameTime - PA)) / 6))

#define L_TARGET                 127
#define AE_LOCK_RANGE_IN         8
#define AE_LOCK_RANGE_OUT        16

static uint32_t l_total = 0;
static uint32_t l_target_total = 0;
static uint32_t lt_lockrange_in_ubound, lt_lockrange_in_lbound;
static uint32_t lt_lockrange_out_ubound, lt_lockrange_out_lbound;
static bool ae_converged_flag = false;

static uint8_t skip_frame = 0;

static bool is_ae_enabled = true;
static int exp_us_cache = -1;
// Exposure time related parameters -

static int set_auto_gain(omv_csi_t *csi, int enable,
                         float gain_db, float gain_db_ceiling);
static int get_gain_db(omv_csi_t *csi, float *gain_db);

static int bank_switch(uint8_t bank) {
    #ifdef CACHE_BANK
    if (bank_cache == bank) {
        return 0;
    }
    #endif

    if (pixspi_regs_write(REG_BANK_SWITCH, &bank, 1)) {
        printf("Bank switch failed.\n");
        return -1;
    }
    bank_cache = bank;
    return 0;
}

static int read_regs_w_bank(uint8_t bank, uint8_t addr, uint8_t *buff, uint16_t len) {
    if (bank_switch(bank)) {
        return -1;
    }
    return pixspi_regs_read(addr, buff, len);
}

static int write_regs_w_bank(uint8_t bank, uint8_t addr, uint8_t *buff, uint16_t len) {
    if (bank_switch(bank)) {
        return -1;
    }
    return pixspi_regs_write(addr, buff, len);
}

static int init_sensor(omv_csi_t *csi) {
#define ta_seq    low_active_R_ABC_Avg_UBx50_T_BLACINV_EnHx1_T_SIG_REFx1

    int arr_size = sizeof(ta_seq) / sizeof(ta_seq[0]);
    for (int i = 0; i < arr_size; i += 2) {
        #ifndef DEBUG
        int
        #endif
        init_res = pixspi_regs_write(ta_seq[i], &ta_seq[i + 1], 1);
        if (init_res) {
            return init_res;
        }
    }

    // Clear bank cache.
    bank_cache = -1;
    return 0;
}
//-----------------------------------------------------------------
static int set_exposure(omv_csi_t *csi, int exp_us, bool protected) {
    // 3642T
    static const uint32_t EXPOSURE_BASE = 3642 / (OMV_PAJ6100_CLK_FREQ / 1000000.0f) + 0.5f;
    int ret;
    uint32_t cmd_expo /* 24-bit available */;
    uint16_t exp_offset;

    #ifdef DEBUG_AE
    printf("set_exposure() = %d\n", exp_us);
    #endif
    if (exp_us == 0) {
        return 0;
    }

    if (exp_us >= EXPOSURE_BASE) {
        int param;
        if (csi->framesize == OMV_CSI_FRAMESIZE_QVGA) {
            param = 88803;
        } else if (csi->framesize == OMV_CSI_FRAMESIZE_QQVGA) {
            param = 29139; // QQVGA
        } else {
            param = 88803;
        }

        int max_cmd_expo = R_FrameTime - param;
        cmd_expo = (exp_us - EXPOSURE_BASE) * ((float) OMV_PAJ6100_CLK_FREQ / 1000000) + 0.5f;
        if (protected) {
            if (cmd_expo > max_cmd_expo) {
                cmd_expo = max_cmd_expo;
                exp_us = (cmd_expo + 3642) / ((float) OMV_PAJ6100_CLK_FREQ / 1000000) + 0.5f;
                printf("Exposure time overflow, reset to %dus.\n", exp_us);
            }
        }
        exp_offset = 0;
    } else {
        cmd_expo = 0;
        exp_offset = (EXPOSURE_BASE - exp_us) * ((float) OMV_PAJ6100_CLK_FREQ / 1000000) + 0.5;
    }
    #ifdef DEBUG_AE
    printf("target - cmd_exp: %ld, exp_offset: %d\n", cmd_expo, exp_offset);
    #endif
    uint8_t buff[3] = {
        (uint8_t) (cmd_expo & 0xff),
        (uint8_t) ((cmd_expo >> 8) & 0xff),
        (uint8_t) ((cmd_expo >> 16) & 0xff),
    };
    ret = write_regs_w_bank(BANK_0, REG_CMD_EXPO_L, buff, 3);

    buff[0] = (uint8_t) (exp_offset & 0xff);
    buff[1] = (uint8_t) ((exp_offset >> 8) & 0xff);
    ret |= write_regs_w_bank(BANK_0, REG_EXP_OFFSET_L, buff, 2);
    buff[0] = 1;
    ret |= write_regs_w_bank(BANK_1, REG_ISP_UPDATE, buff, 1);

    if (ret) {
        printf("Failed to write cmd_expo or exp_offset.\n");
        return -1;
    }
    exp_us_cache = exp_us;
    return ret;
}

static int get_exposure(omv_csi_t *csi) {
    int ret;
    uint8_t buff[3] = {};
    uint32_t cmd_expo /* 24-bit available */;
    uint16_t exp_offset;

    if (exp_us_cache >= 0) {
        return exp_us_cache;
    }

    ret = read_regs_w_bank(BANK_0, REG_CMD_EXPO_L, buff, 3);
    if (ret) {
        printf("Read cmd_expo failed.\n");
        return -1;
    }
    cmd_expo = buff[0] + (((uint32_t) buff[1]) << 8) + (((uint32_t) buff[2]) << 16);

    ret = read_regs_w_bank(BANK_0, REG_EXP_OFFSET_L, buff, 2);
    if (ret) {
        printf("Read exp_offset failed.\n");
        return -1;
    }
    exp_offset = buff[0] + (((uint16_t) buff[1]) << 8);

    #ifdef DEBUG_AE
    printf("cmd_exp: %ld, exp_offset: %d\n", cmd_expo, exp_offset);
    #endif

    if (exp_offset == 0) {
        exp_us_cache = (cmd_expo + 3642) / (OMV_PAJ6100_CLK_FREQ / 1000000);
    } else {
        exp_us_cache = (3642 - exp_offset) / (OMV_PAJ6100_CLK_FREQ / 1000000);
    }

    return exp_us_cache;
}

static void blc_freeze(bool enable) {
    uint8_t tmp;
    tmp = enable?2:0;
    write_regs_w_bank(0, 0x0E, &tmp, 1);
    tmp = 1;
    write_regs_w_bank(1, 0x00, &tmp, 1); // Update flag
}

static void auto_exposure(omv_csi_t *csi) {
    if (!is_ae_enabled) {
        return;
    }

    if (skip_frame) {
        --skip_frame;
        return;
    }

    if ((lt_lockrange_in_lbound <= l_total) && (l_total <= lt_lockrange_in_ubound)) {
        ae_converged_flag = true;
    } else if ((l_total > lt_lockrange_out_ubound) || (l_total < lt_lockrange_out_lbound)) {
        ae_converged_flag = false;
    }
    blc_freeze(ae_converged_flag);

    if (ae_converged_flag == false) {
        float gain_db = 0;
        if (get_gain_db(csi, &gain_db)) {
            printf("Get Gain DB failed.\n");
            return;
        }
        uint32_t gain;
        if (gain_db < 6) {
            // 1x gain
            gain = 1;
        } else if (gain_db < 12) {
            // 2x gain
            gain = 2;
        } else if (gain_db < 18) {
            // 4x gain
            gain = 4;
        } else {
            // 8x gain
            gain = 8;
        }

        int expo = get_exposure(csi);
        if (expo < 0) {
            printf("Get exposure failed.\n");
            return;
        }
        #ifdef DEBUG_AE
        printf("Current Gain: %ldx, Expo: %d\n", gain, expo);
        #endif

        uint32_t GEP = gain * expo;
        float l_ratio = ((float) l_target_total) / l_total;
        float GEP_target = GEP * l_ratio;
        uint32_t expo_target = IM_CLAMP(fast_roundf(GEP_target / R_AE_MinGain), R_AE_MinExpoTime, R_AE_MaxExpoTime);

        #ifdef DEBUG_AE
        printf("GEP: %ld ", GEP);
        printf("GEP_target: %d (x1000), L_Ratio: %d (x1000)\n",
               (int) (GEP_target * 1000), (int) (l_ratio * 1000));
        printf("1st Expo Target: %ld (max: %ld) ", expo_target, R_AE_MaxExpoTime);
        #endif
        uint32_t gain_target;
        float tmp_gain = GEP_target / expo_target;
        if (tmp_gain <= 1) {
            gain_target = 1;
        } else if (tmp_gain <= 2) {
            gain_target = 2;
        } else if (tmp_gain <= 4) {
            gain_target = 4;
        } else {
            gain_target = 8;
        }
        //gain_target = IM_CLAMP(fast_ceilf(GEP_target/expo_target), R_AE_MinGain, R_AE_MaxGain);
        gain_db = 20 * log10f((float) gain_target);
        expo_target = IM_CLAMP(fast_roundf(GEP_target / gain_target), R_AE_MinExpoTime, R_AE_MaxExpoTime);
        #ifdef DEBUG_AE
        printf("Gain Target: %ld (%d DB (x1000))\n", gain_target, (int) (gain_db * 1000));
        printf("Final Expo Target: %ld (max: %ld) \n", expo_target, R_AE_MaxExpoTime);
        #endif
        set_exposure(csi, expo_target, false);
        set_auto_gain(csi, false, gain_db, 0);
        skip_frame = 0;
    }
}
//-----------------------------------------------------------------
static int sleep(omv_csi_t *csi, int enable) {
    int ret;
    uint8_t val;

    if (enable) {
        ret = read_regs_w_bank(BANK_1, REG_CMD_SENSOR_MODE, &val, 1);
        if (ret) {
            printf("Failed to read REG_CMD_SENSOR_MODE.\n");
            return -1;
        }
        val |= (1 << 7);
        ret = write_regs_w_bank(BANK_1, REG_CMD_SENSOR_MODE, &val, 1);
        if (ret) {
            printf("Failed to write REG_CMD_SENSOR_MODE.\n");
            return -1;
        }
        // Sleep 30ms
        mp_hal_delay_ms(30);
        ret = read_regs_w_bank(BANK_1, REG_CMD_LPM_ENH, &val, 1);
        if (ret) {
            printf("Failed to read REG_CMD_SENSOR_MODE.\n");
            return -1;
        }
        val |= (1 << 6);
        ret = write_regs_w_bank(BANK_1, REG_CMD_LPM_ENH, &val, 1);
        if (ret) {
            printf("Failed to write REG_CMD_LPM_ENH.\n");
            return -1;
        }
    } else {
        ret = read_regs_w_bank(BANK_1, REG_CMD_SENSOR_MODE, &val, 1);
        if (ret) {
            #ifdef DEBUG
            printf("Failed to read REG_CMD_SENSOR_MODE.\n");
            #endif
            return -1;
        }
        val &= ~(1 << 7);
        val &= ~(1 << 6);
        ret = write_regs_w_bank(BANK_1, REG_CMD_SENSOR_MODE, &val, 1);
        if (ret) {
            #ifdef DEBUG
            printf("Failed to write REG_CMD_SENSOR_MODE.\n");
            #endif
            return -1;
        }
    }
    return 0;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint8_t data;

    if (pixspi_regs_read((uint8_t) reg_addr, &data, 1)) {
        return -1;
    }
    return data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    uint8_t data = reg_data;

    // Clear bank cache.
    bank_cache = -1;

    return pixspi_regs_write((uint8_t) reg_addr, &data, 1);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    if (pixformat != PIXFORMAT_GRAYSCALE) {
        return -1;
    }
    return 0;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    int ret = 0;

    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    uint8_t aavg_VnH, abc_start_line, voffset, abc_sample_size;
    ret |= read_regs_w_bank(BANK_0, REG_CMD_AAVG_V /* REG_CMD_AAVG_H */, &aavg_VnH, 1);
    ret |= read_regs_w_bank(BANK_0, REG_ABC_START_LINE, &abc_start_line, 1);
    ret |= read_regs_w_bank(BANK_1, REG_ABC_SAMPLE_SIZE, &abc_sample_size, 1);

    if (ret) {
        printf("Failed to read Average mode registers.\n");
        return -1;
    }

    switch (framesize) {
        case OMV_CSI_FRAMESIZE_QVGA:
            aavg_VnH &= ~((1 << 3) | (1 << 2));
            abc_start_line = (abc_start_line & ~(0x07 << 1)) | (2 << 1);
            voffset = 2;
            abc_sample_size = (abc_sample_size & ~(0x07)) | 6;

            R_AE_MaxExpoTime = CAL_MAX_EXPO_TIME(QVGA_MAX_EXPO_PA);
            break;
        case OMV_CSI_FRAMESIZE_QQVGA:
            aavg_VnH |= (1 << 3) | (1 << 2);
            abc_start_line = (abc_start_line & ~(0x07 << 1)) | (1 << 1);
            voffset = 1;
            abc_sample_size = (abc_sample_size & ~(0x07)) | 5;

            R_AE_MaxExpoTime = CAL_MAX_EXPO_TIME(QQVGA_MAX_EXPO_PA);
            break;
        default:
            printf("PAJ6100 only support QVGA & QQVGA now.\n");
            return -1;
    }

    ret |= write_regs_w_bank(BANK_0, REG_CMD_AAVG_V /* REG_CMD_AAVG_H */, &aavg_VnH, 1);
    ret |= write_regs_w_bank(BANK_0, REG_ABC_START_LINE, &abc_start_line, 1);
    ret |= write_regs_w_bank(BANK_1, REG_CP_WOI_VOFFSET, &voffset, 1);
    ret |= write_regs_w_bank(BANK_1, REG_ABC_SAMPLE_SIZE, &abc_sample_size, 1);

    if (ret) {
        printf("Failed to write Average mode registers.\n");
        return -1;
    }

    l_target_total = L_TARGET * w * h;
    lt_lockrange_in_ubound = (L_TARGET + AE_LOCK_RANGE_IN) * w * h;
    lt_lockrange_in_lbound = (L_TARGET - AE_LOCK_RANGE_IN) * w * h;
    lt_lockrange_out_ubound = (L_TARGET + AE_LOCK_RANGE_OUT) * w * h;
    lt_lockrange_out_lbound = (L_TARGET - AE_LOCK_RANGE_OUT) * w * h;

    return 0;
}

static int set_contrast(omv_csi_t *csi, int level) {
    return 0;
}

static int set_brightness(omv_csi_t *csi, int level) {
    return 0;
}

static int set_saturation(omv_csi_t *csi, int level) {
    return 0;
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    return 0;
}

static int set_special_effect(omv_csi_t *csi, omv_csi_sde_t sde) {
    return 0;
}

static int set_auto_gain(omv_csi_t *csi, int enable,
                         float gain_db, float gain_db_ceiling) {
    if (enable) {
        return 0;
    }
    int ret = 0;
    uint8_t val, fgh, ggh;

    if (gain_db < 6) {
        // 1x gain
        fgh = 0;
        ggh = 0;
    } else if (gain_db < 12) {
        // 2x gain
        fgh = 0;
        ggh = 1;
    } else if (gain_db < 18) {
        // 4x gain
        fgh = 2;
        ggh = 1;
    } else {
        // 8x gain
        fgh = 3;
        ggh = 1;
    }

    ret = read_regs_w_bank(BANK_0, REG_FGH, &val, 1);
    fgh = (val & ~(3 << 4)) | (fgh << 4); // fgh[5:4] = 0
    ret |= read_regs_w_bank(BANK_0, REG_GGH, &val, 1);
    ggh = (val & ~(1 << 7)) | (ggh << 7); // ggh[7] = 1
    if (ret) {
        printf("Failed to read FGH or GGH.\n");
    } else {
        write_regs_w_bank(BANK_0, REG_FGH, &fgh, 1);
        write_regs_w_bank(BANK_0, REG_GGH, &ggh, 1);
        val = 1;
        write_regs_w_bank(BANK_1, REG_ISP_UPDATE, &val, 1);
    }

    return 0;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    int ret = 0;
    uint8_t val;
    uint8_t fgh, ggh;

    ret |= read_regs_w_bank(BANK_0, REG_FGH, &val, 1);
    fgh = (val >> 4) & 0x03;
    ret |= read_regs_w_bank(BANK_0, REG_GGH, &val, 1);
    ggh = (val >> 7) & 0x01;

    if (ret) {
        printf("Failed to read FGH or GGH.\n");
        return -1;
    }

    if (ggh == 0 && fgh == 0) {
        *gain_db = 0;
    } else if (ggh == 1) {
        switch (fgh) {
            case 0:
                *gain_db = 6;
                break;
            case 2:
                *gain_db = 12;
                break;
            case 3:
                *gain_db = 18;
                break;
            default: {
                printf("Read a undefined FGH value (%d).\n", fgh);
                return -1;
            }
        }
    }

    return 0;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    if (enable) {
        is_ae_enabled = true;
        return 0;
    } else {
        is_ae_enabled = false;
        return set_exposure(csi, exposure_us, false);
    }
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    int ret = get_exposure(csi);
    if (ret >= 0) {
        *exposure_us = ret;
        return 0;
    }
    return ret;
}

static int set_auto_whitebal(omv_csi_t *csi, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    return 0;
}

static int get_rgb_gain_db(omv_csi_t *csi, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    int ret;
    uint8_t val;
    ret = read_regs_w_bank(BANK_0, REG_CMD_HSYNC_INV, &val, 1);
    if (ret) {
        printf("Failed to read REG_CMD_HSYNC_INV.\n");
        return -1;
    }

    val = enable == 1?val | (0x01):val & ~(0x01);

    ret = write_regs_w_bank(BANK_0, REG_CMD_HSYNC_INV, &val, 1);
    ret = write_regs_w_bank(BANK_1, REG_ISP_UPDATE, &val, 1);
    if (ret) {
        printf("Failed to write REG_CMD_HSYNC_INV.\n");
    }
    return ret;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    int ret;
    uint8_t val;
    ret = read_regs_w_bank(BANK_0, REG_CMD_VSYNC_INV, &val, 1);
    if (ret) {
        printf("Failed to read REG_CMD_VSYNC_INV.\n");
        return -1;
    }

    val = enable == 1?val | (0x01 << 1):val & ~(0x01 << 1);

    ret = write_regs_w_bank(BANK_0, REG_CMD_VSYNC_INV, &val, 1);
    ret = write_regs_w_bank(BANK_1, REG_ISP_UPDATE, &val, 1);
    if (ret) {
        printf("Failed to write REG_CMD_VSYNC_INV.\n");
    }
    return ret;
}

static int set_lens_correction(omv_csi_t *csi, int enable, int radi, int coef) {
    return 0;
}

static int reset(omv_csi_t *csi) {
    int ret;
    bank_cache = -1;
    exp_us_cache = -1;

    #ifdef DEBUG
    uint8_t part_id_l = 0, part_id_h = 0;
    read_regs_w_bank(0, 0x00, &part_id_l, 1);
    read_regs_w_bank(0, 0x01, &part_id_h, 1);
    printf("Part ID 0x%x 0x%x\n", part_id_l, part_id_h);
    printf("init_res: %d\n", init_res);
    #endif

    // Re-init csi every time.
    init_sensor(csi);

    // Fetch default R_Frame_Time
    uint8_t buff[3] = {};
    ret = read_regs_w_bank(BANK_0, REG_FRAME_TIME_L, buff, 3);
    if (ret) {
        printf("Read cmd_expo failed.\n");
    } else {
        R_FrameTime = buff[0] + (((uint32_t) buff[1]) << 8) + (((uint32_t) buff[2]) << 16);
    }

    // Default Gain 2x
    uint8_t val, fgh, ggh;
    ret = read_regs_w_bank(BANK_0, REG_FGH, &val, 1);
    fgh = (val & ~(3 << 4)); // fgh[5:4] = 0
    ret |= read_regs_w_bank(BANK_0, REG_GGH, &val, 1);
    ggh = (val & ~(1 << 7)) | (0x01 << 7); // ggh[7] = 1
    if (ret) {
        printf("Failed to read FGH or GGH.\n");
    } else {
        write_regs_w_bank(BANK_0, REG_FGH, &fgh, 1);
        write_regs_w_bank(BANK_0, REG_GGH, &ggh, 1);
        val = 1;
        write_regs_w_bank(BANK_1, REG_ISP_UPDATE, &val, 1);
    }
    return 0;
}

static int paj6100_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    int res = omv_csi_snapshot(csi, image, flags);
    if (res == 0) {
        l_total = 0;
        int iml = image->h * image->w;
        for (int i = 0; i < iml; ++i) {
            l_total += image->data[i];
        }
        // PAJ6100 doesn't support HW auto-exposure,
        // we provide a software implementation.
        auto_exposure(csi);
    }

    return res;
}

int paj6100_init(omv_csi_t *csi) {
    // Initialize csi structure.
    csi->reset = reset;
    csi->sleep = sleep;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_contrast = set_contrast;
    csi->set_brightness = set_brightness;
    csi->set_saturation = set_saturation;
    csi->set_gainceiling = set_gainceiling;
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_auto_whitebal = set_auto_whitebal;

    csi->get_rgb_gain_db = get_rgb_gain_db;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->set_special_effect = set_special_effect;
    csi->set_lens_correction = set_lens_correction;

    csi->snapshot = paj6100_snapshot;

    // Set csi flags
    csi->vsync_pol = 1;
    csi->hsync_pol = 1;
    csi->pixck_pol = 1;
    csi->frame_sync = 0;
    csi->mono_bpp = 1;

    init_sensor(csi);
    return 0;
}

bool paj6100_detect(omv_csi_t *csi) {
    int ret = 0;
    uint8_t part_id_l, part_id_h;

    omv_gpio_write(OMV_CSI_RESET_PIN, 1);
    mp_hal_delay_ms(10);

    if (!pixspi_init()) {
        printf("Initial pixspi failed.\n");
        return false;
    }

    ret |= pixspi_regs_read(0x00, &part_id_l, 1);
    ret |= pixspi_regs_read(0x01, &part_id_h, 1);
    #ifdef DEBUG
    printf("Part ID 0x%x 0x%x\n", part_id_l, part_id_h);
    #endif
    if (ret == 0 && (part_id_l == 0x00 && part_id_h == 0x61)) {
        return true; // Got you.
    }

    pixspi_release();
    return false;
}
#endif //(OMV_PAJ6100_ENABLE == 1)
