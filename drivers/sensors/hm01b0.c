/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * HM01B0 driver.
 */
#include "omv_boardconfig.h"
#if (OMV_HM01B0_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "py/mphal.h"

#include "omv_i2c.h"
#include "omv_csi.h"
#include "hm01b0.h"

#define HIMAX_BOOT_RETRY            (10)
#define HIMAX_LINE_LEN_PCK_FULL     0x178
#define HIMAX_FRAME_LENGTH_FULL     0x109

#define HIMAX_LINE_LEN_PCK_QVGA     0x178
#define HIMAX_FRAME_LENGTH_QVGA     0x104

#define HIMAX_LINE_LEN_PCK_QQVGA    0x178
#define HIMAX_FRAME_LENGTH_QQVGA    0x084

static const uint16_t default_regs[][2] = {
    {BLC_TGT,              0x08},          //  BLC target :8  at 8 bit mode
    {BLC2_TGT,             0x08},          //  BLI target :8  at 8 bit mode
    {0x3044,               0x0A},          //  Increase CDS time for settling
    {0x3045,               0x00},          //  Make symmetric for cds_tg and rst_tg
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
    {ANA_Register_17,      0x00},          //  Disable internal oscillator

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
    {STATISTIC_CTRL,       0x07},          //  AE stat en | MD LROI stat en | magic
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
    {AE_TARGET_MEAN,       0x64},          //AE target mean          [Def: 0x3C]
    {AE_MIN_MEAN,          0x0A},          //AE min target mean      [Def: 0x0A]
    {CONVERGE_IN_TH,       0x03},          //Converge in threshold   [Def: 0x03]
    {CONVERGE_OUT_TH,      0x05},          //Converge out threshold  [Def: 0x05]
    {MAX_INTG_H,           (HIMAX_FRAME_LENGTH_QVGA - 2) >> 8},          //Maximum INTG High Byte  [Def: 0x01]
    {MAX_INTG_L,           (HIMAX_FRAME_LENGTH_QVGA - 2) & 0xFF},        //Maximum INTG Low Byte   [Def: 0x54]
    {MAX_AGAIN_FULL,       0x04},          //Maximum Analog gain in full frame mode [Def: 0x03]
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

    {MD_CTRL,              0x00},
    {FRAME_LEN_LINES_H,    HIMAX_FRAME_LENGTH_QVGA >> 8},
    {FRAME_LEN_LINES_L,    HIMAX_FRAME_LENGTH_QVGA & 0xFF},
    {LINE_LEN_PCK_H,       HIMAX_LINE_LEN_PCK_QVGA >> 8},
    {LINE_LEN_PCK_L,       HIMAX_LINE_LEN_PCK_QVGA & 0xFF},
    {QVGA_WIN_EN,          0x01},          // Enable QVGA window readout
    {0x0383,               0x01},
    {0x0387,               0x01},
    {0x0390,               0x00},
    {0x3011,               0x70},
    {0x3059,               0x02},
    {OSC_CLK_DIV,          0x0B},
    {IMG_ORIENTATION,      0x00},          // change the orientation
    {0x0104,               0x01},

    //============= End of regs marker ==================
    {0x0000,            0x00},
};

static int reset(omv_csi_t *csi) {
    // Reset csi.
    uint8_t reg = 0xff;
    for (int retry = HIMAX_BOOT_RETRY; retry >= 0 && reg != HIMAX_MODE_STANDBY; retry--) {
        if (omv_i2c_writeb2(csi->i2c, csi->slv_addr, SW_RESET, HIMAX_RESET) != 0) {
            return -1;
        }

        mp_hal_delay_ms(1);

        if (omv_i2c_readb2(csi->i2c, csi->slv_addr, MODE_SELECT, &reg) != 0) {
            return -1;
        }

        if (reg == HIMAX_MODE_STANDBY) {
            break;
        } else if (retry == 0) {
            return -1;
        }

        mp_hal_delay_ms(10);
    }

    // Write default registers
    int ret = 0;
    for (int i = 0; default_regs[i][0] && ret == 0; i++) {
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Set PCLK polarity.
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, PCLK_POLARITY, (0x20 | PCLK_FALLING_EDGE));

    // Set mode to streaming
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MODE_SELECT, HIMAX_MODE_STREAMING);

    return ret;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint8_t reg_data;
    if (omv_i2c_readb2(csi->i2c, csi->slv_addr, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return omv_i2c_writeb2(csi->i2c, csi->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
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

static const uint16_t FULL_regs[][2] = {
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {QVGA_WIN_EN,           0x00},// Disable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_FULL - 2) >> 8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_FULL - 2) & 0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_FULL >> 8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_FULL & 0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_FULL >> 8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_FULL & 0xFF)},
    {GRP_PARAM_HOLD,        0x01},
    //============= End of regs marker ==================
    {0x0000,            0x00},

};

static const uint16_t QVGA_regs[][2] = {
    {0x0383,                0x01},
    {0x0387,                0x01},
    {0x0390,                0x00},
    {QVGA_WIN_EN,           0x01},// Enable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QVGA - 2) >> 8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QVGA - 2) & 0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QVGA >> 8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QVGA & 0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QVGA >> 8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QVGA & 0xFF)},
    {GRP_PARAM_HOLD,        0x01},
    //============= End of regs marker ==================
    {0x0000,            0x00},

};

static const uint16_t QQVGA_regs[][2] = {
    {0x0383,                0x03},
    {0x0387,                0x03},
    {0x0390,                0x03},
    {QVGA_WIN_EN,           0x01},// Enable QVGA window readout
    {MAX_INTG_H,            (HIMAX_FRAME_LENGTH_QQVGA - 2) >> 8},
    {MAX_INTG_L,            (HIMAX_FRAME_LENGTH_QQVGA - 2) & 0xFF},
    {FRAME_LEN_LINES_H,     (HIMAX_FRAME_LENGTH_QQVGA >> 8)},
    {FRAME_LEN_LINES_L,     (HIMAX_FRAME_LENGTH_QQVGA & 0xFF)},
    {LINE_LEN_PCK_H,        (HIMAX_LINE_LEN_PCK_QQVGA >> 8)},
    {LINE_LEN_PCK_L,        (HIMAX_LINE_LEN_PCK_QQVGA & 0xFF)},
    {GRP_PARAM_HOLD,        0x01},
    //============= End of regs marker ==================
    {0x0000,            0x00},
};

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    int ret = 0;
    uint16_t w = resolution[framesize][0];
    uint16_t h = resolution[framesize][1];

    switch (framesize) {
        case OMV_CSI_FRAMESIZE_QVGA:
            for (int i = 0; QVGA_regs[i][0] && ret == 0; i++) {
                ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, QVGA_regs[i][0], QVGA_regs[i][1]);
            }
            break;
        case OMV_CSI_FRAMESIZE_QQVGA:
            for (int i = 0; QQVGA_regs[i][0] && ret == 0; i++) {
                ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, QQVGA_regs[i][0], QQVGA_regs[i][1]);
            }
            break;
        case OMV_CSI_FRAMESIZE_CUSTOM:
        case OMV_CSI_FRAMESIZE_320X320:
            if (w != 320 || h != 320) {
                ret = -1;
                break;
            }
            for (int i = 0; FULL_regs[i][0] && ret == 0; i++) {
                ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, FULL_regs[i][0], FULL_regs[i][1]);
            }
            break;
        default:
            ret = -1;
    }

    return ret;
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    uint8_t osc_div = 0;
    bool highres = false;

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID
        || csi->framesize == OMV_CSI_FRAMESIZE_QVGA
        || csi->framesize == OMV_CSI_FRAMESIZE_320X320) {
        highres = true;
    }

    if (framerate <= 15) {
        osc_div = (highres == true) ? 0x01 : 0x00;
    } else if (framerate <= 30) {
        osc_div = (highres == true) ? 0x02 : 0x01;
    } else if (framerate <= 60) {
        osc_div = (highres == true) ? 0x03 : 0x02;
    } else {
        osc_div = 0x03; // Set to the max possible FPS at this resolution.
    }
    return omv_i2c_writeb2(csi->i2c, csi->slv_addr, OSC_CLK_DIV, 0x08 | osc_div);
}

static int set_brightness(omv_csi_t *csi, int level) {
    uint8_t ae_mean;
    // Simulate brightness levels by setting AE loop target mean.
    switch (level) {
        case 0:
            ae_mean = 60;
            break;
        case 1:
            ae_mean = 80;
            break;
        case 2:
            ae_mean = 100;
            break;
        case 3:
            ae_mean = 127;
            break;
        default:
            ae_mean = 60;
    }
    return omv_i2c_writeb2(csi->i2c, csi->slv_addr, AE_TARGET_MEAN, ae_mean);
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    int ret = 0;
    int gain = 0x0;
    switch (gainceiling) {
        case OMV_CSI_GAINCEILING_2X:
            gain = 0x01;
            break;
        case OMV_CSI_GAINCEILING_4X:
            gain = 0x02;
            break;
        case OMV_CSI_GAINCEILING_8X:
            gain = 0x03;
            break;
        case OMV_CSI_GAINCEILING_16X:
            gain = 0x04;
            break;
        default:
            return -1;
    }
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MAX_AGAIN_FULL, gain);
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MAX_AGAIN_BIN2, gain);
    return ret;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    return omv_i2c_writeb2(csi->i2c, csi->slv_addr, TEST_PATTERN_MODE, enable & 0x1);
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    int ret = 0;
    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        gain_db = IM_CLAMP(gain_db, 0.0f, 24.0f);
        int gain = fast_ceilf(logf(expf((gain_db / 20.0f) * M_LN10)) / M_LN2);
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, AE_CTRL, 0); // Must disable AE
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, ANALOG_GAIN, ((gain & 0x7) << 4));
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, GRP_PARAM_HOLD, 0x01);
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        gain_db_ceiling = IM_CLAMP(gain_db_ceiling, 0.0f, 24.0f);
        int gain = fast_ceilf(logf(expf((gain_db_ceiling / 20.0f) * M_LN10) / M_LN2));
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MAX_AGAIN_FULL, (gain & 0x7));
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MAX_AGAIN_BIN2, (gain & 0x7));
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, AE_CTRL, 1);
    }
    return ret;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    uint8_t gain;
    if (omv_i2c_readb2(csi->i2c, csi->slv_addr, ANALOG_GAIN, &gain) != 0) {
        return -1;
    }
    *gain_db = fast_floorf(log10f(1 << (gain >> 4)) * 20.0f);
    return 0;
}

static int get_vt_pix_clk(omv_csi_t *csi, uint32_t *vt_pix_clk) {
    uint8_t reg;
    if (omv_i2c_readb2(csi->i2c, csi->slv_addr, OSC_CLK_DIV, &reg) != 0) {
        return -1;
    }
    // 00 -> MCLK / 8
    // 01 -> MCLK / 4
    // 10 -> MCLK / 2
    // 11 -> MCLK / 1
    uint32_t vt_sys_div = 8 / (1 << (reg & 0x03));

    // vt_pix_clk = MCLK / vt_sys_div
    *vt_pix_clk = OMV_CSI_CLK_FREQUENCY / vt_sys_div;
    return 0;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    int ret = 0;

    if (enable) {
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, AE_CTRL, 1);
    } else {
        uint32_t line_len;
        uint32_t frame_len;
        uint32_t coarse_int;
        uint32_t vt_pix_clk = 0;

        switch (csi->framesize) {
            case OMV_CSI_FRAMESIZE_320X320:
                line_len = HIMAX_LINE_LEN_PCK_FULL;
                frame_len = HIMAX_FRAME_LENGTH_FULL;
                break;
            case OMV_CSI_FRAMESIZE_QVGA:
                line_len = HIMAX_LINE_LEN_PCK_QVGA;
                frame_len = HIMAX_FRAME_LENGTH_QVGA;
                break;
            case OMV_CSI_FRAMESIZE_QQVGA:
                line_len = HIMAX_LINE_LEN_PCK_QQVGA;
                frame_len = HIMAX_FRAME_LENGTH_QQVGA;
                break;
            default:
                return -1;
        }

        ret |= get_vt_pix_clk(csi, &vt_pix_clk);
        coarse_int = fast_roundf(exposure_us * (vt_pix_clk / 1000000.0f) / line_len);

        if (coarse_int < 2) {
            coarse_int = 2;
        } else if (coarse_int > (frame_len - 2)) {
            coarse_int = frame_len - 2;
        }

        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, AE_CTRL, 0);
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, INTEGRATION_H, coarse_int >> 8);
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, INTEGRATION_L, coarse_int & 0xff);
        ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, GRP_PARAM_HOLD, 0x01);
    }

    return ret;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    int ret = 0;
    uint32_t line_len;
    uint32_t coarse_int = 0;
    uint32_t vt_pix_clk = 0;
    if (csi->framesize == OMV_CSI_FRAMESIZE_QVGA) {
        line_len = HIMAX_LINE_LEN_PCK_QVGA;
    } else {
        line_len = HIMAX_LINE_LEN_PCK_QQVGA;
    }
    ret |= get_vt_pix_clk(csi, &vt_pix_clk);
    ret |= omv_i2c_readb2(csi->i2c, csi->slv_addr, INTEGRATION_H, &((uint8_t *) &coarse_int)[1]);
    ret |= omv_i2c_readb2(csi->i2c, csi->slv_addr, INTEGRATION_L, &((uint8_t *) &coarse_int)[0]);
    *exposure_us = fast_roundf(coarse_int * line_len / (vt_pix_clk / 1000000.0f));
    return ret;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb2(csi->i2c, csi->slv_addr, IMG_ORIENTATION, &reg);
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, IMG_ORIENTATION, HIMAX_SET_HMIRROR(reg, enable));
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, GRP_PARAM_HOLD, 0x01);
    return ret;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    uint8_t reg;
    int ret = omv_i2c_readb2(csi->i2c, csi->slv_addr, IMG_ORIENTATION, &reg);
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, IMG_ORIENTATION, HIMAX_SET_VMIRROR(reg, enable));
    ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, GRP_PARAM_HOLD, 0x01);
    return ret;
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    int ret = 0;

    switch (request) {
        case OMV_CSI_IOCTL_HIMAX_OSC_ENABLE: {
            uint32_t enable = va_arg(ap, uint32_t);
            ret = omv_i2c_writeb2(csi->i2c, csi->slv_addr, ANA_Register_17, enable ? 1:0);
            mp_hal_delay_ms(100);
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_ENABLE: {
            uint32_t enable = va_arg(ap, uint32_t);
            ret = omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_CTRL, enable ? 1:0);
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_WINDOW: {
            uint32_t x1 = va_arg(ap, uint32_t);
            uint32_t y1 = va_arg(ap, uint32_t);
            uint32_t x2 = va_arg(ap, uint32_t) + x1;
            uint32_t y2 = va_arg(ap, uint32_t) + y1;
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_X_START_H, (x1 >> 8));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_X_START_L, (x1 & 0xff));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_Y_START_H, (y1 >> 8));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_Y_START_L, (y1 & 0xff));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_X_END_H,   (x2 >> 8));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_X_END_L,   (x2 & 0xff));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_Y_END_H,   (y2 >> 8));
            ret |= omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_LROI_Y_END_L,   (y2 & 0xff));
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_THRESHOLD: {
            uint32_t threshold = va_arg(ap, uint32_t);
            ret = omv_i2c_writeb2(csi->i2c, csi->slv_addr, MD_THL, threshold);
            break;
        }

        case OMV_CSI_IOCTL_HIMAX_MD_CLEAR: {
            ret = omv_i2c_writeb2(csi->i2c, csi->slv_addr, I2C_CLEAR, 1);
            break;
        }

        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int hm01b0_init(omv_csi_t *csi) {
    // Initialize csi structure.
    csi->reset = reset;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_framerate = set_framerate;
    csi->set_brightness = set_brightness;
    csi->set_gainceiling = set_gainceiling;
    csi->set_colorbar = set_colorbar;
    csi->set_auto_gain = set_auto_gain;
    csi->get_gain_db = get_gain_db;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->ioctl = ioctl;

    // Set csi flags
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 0;
    csi->frame_sync = 0;
    csi->mono_bpp = 1;

    return 0;
}
#endif //(OMV_HM01B0_ENABLE == 1)
