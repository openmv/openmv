/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * ST VD66GY (colour) / VD56G3 (mono) 1.5 MP global-shutter sensor driver for OpenMV.
 *
 * The boot sequence (firmware patch, VT patch, PLL setup) is done by ST's BSD-licensed
 * VD6G driver from STMicroelectronics/stm32-mw-camera (drivers/sensors/vd6g). This file
 * adds the OpenMV glue: MIPI CSI-2 output, arbitrary centred ROI with optional digital
 * binning, frame length (frame rate / frame time in microseconds), manual exposure and
 * analog gain, mirror/flip, test pattern and the sensor GPIO modes (strobe output etc).
 *
 * Output is 2-lane MIPI CSI-2 RAW10 at 804 Mbps/lane. The sensor produces exactly the
 * requested frame size, so the ISP scaler is not used.
 *
 * Register widths and little-endian byte order follow ST's driver. The analog gain
 * coding (gain = 32 / (32 - code)) and the minimum vertical blanking (110 lines) follow
 * the Linux vd56g3 driver and are best-effort values; the minimum blanking can be
 * changed with OMV_CSI_IOCTL_VD66GY_SET_VBLANK and measured with cashew.sweep().
 */
#include "board_config.h"
#if (OMV_VD66GY_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "omv_i2c.h"
#include "omv_csi.h"
#include "omv_gpio.h"
#include "py/mphal.h"
#include "vd6g/vd6g.h"

#ifndef OMV_VD66GY_CLK_FREQ
#define OMV_VD66GY_CLK_FREQ         (12000000)
#endif

// Registers (see drivers/sensors/vd6g/vd6g.c).
#define REG_MODEL_ID                (0x0000)
#define REG_OPTICAL_REVISION        (0x001a)
#define REG_SYSTEM_FSM              (0x0028)
#define FSM_SW_STBY                 (0x02)
#define FSM_STREAMING               (0x03)
#define REG_STBY                    (0x0201)
#define REG_STREAMING               (0x0202)
#define CMD_ACK                     (0x00)
#define CMD_START_STREAM            (0x01)
#define CMD_STOP_STREAM             (0x01)
#define REG_LINE_LENGTH             (0x0300)
#define REG_ORIENTATION             (0x0302)
#define REG_FORMAT_CTRL             (0x030a)
#define REG_OIF_CTRL                (0x030c)
#define REG_OIF_IMG_CTRL            (0x030f)
#define RAW10_DATA_TYPE             (0x2b)
#define REG_OIF_CSI_BITRATE         (0x0312)
#define REG_DUSTER_CTRL             (0x0318)
#define REG_DARKCAL_CTRL            (0x0340)
#define REG_PATGEN_CTRL             (0x0400)
#define PATGEN_VER_COLOR_BAR        (0x0021)
#define REG_AE_COMPILER_CONTROL     (0x0430)
#define REG_EXP_MODE                (0x044c)
#define EXP_AUTO                    (0)
#define EXP_MANUAL                  (2)
#define REG_MANUAL_ANALOG_GAIN      (0x044d)
#define REG_MANUAL_COARSE_EXP       (0x044e)
#define REG_MANUAL_DIGITAL_GAIN(c)  (0x0450 + 2 * (c))
#define REG_FRAME_LENGTH            (0x0458)
#define REG_OUT_ROI_X_START         (0x045e)
#define REG_OUT_ROI_X_END           (0x0460)
#define REG_OUT_ROI_Y_START         (0x0462)
#define REG_OUT_ROI_Y_END           (0x0464)
#define REG_GPIO_CTRL(i)            (0x0467 + (i))
#define REG_READOUT_CTRL            (0x047e)
#define REG_EXP_COARSE_INTG_MARGIN  (0x0946)
#define REG_MAX_AG_CODED            (0x0960)
#define REG_MIN_AG_CODED            (0x097E)

#define VD66GY_PIXEL_CLOCK          (160800000U)
#define VD66GY_CSI_MBPS             (804)
#define VD66GY_ARRAY_X0             (2)     // First active column (ST full-resolution mode).
#define VD66GY_MIN_EXP_LINES        (21)
#define VD66GY_EXP_OFFSET           (7)
#define VD66GY_DEF_VBLANK           (110)
#define VD66GY_DEF_EXPOSURE_US      (1000)
#define VD66GY_GAIN_SCALE           (32.0f)

typedef struct {
    VD6G_Ctx_t st;              // ST driver context (boot only).
    omv_csi_t *csi;
    bool streaming;
    bool mono;
    bool expo_auto;             // Sensor AE is used only when gain and exposure are both auto.
    bool gain_auto;
    bool colorbar;
    bool hmirror;
    bool vflip;
    uint8_t bin_req;            // 0 = auto, 1 = none, 2 = digital x2, 4 = digital x4.
    uint8_t bin;                // Binning factor in use.
    uint8_t again;              // Manual analog gain code.
    uint8_t gpio[VD6G_GPIO_NB]; // GPIO control register values.
    uint16_t line_len;          // Line length in pixel clocks (read from sensor).
    uint16_t line_len_req;      // 0 = sensor default.
    uint16_t vblank;            // Minimum vertical blanking in lines.
    uint16_t rows;              // Sensor rows read out per frame (ROI height before binning).
    uint16_t frame_len;         // Frame length in lines.
    uint32_t frame_us;          // Requested frame time (0 = derived from the frame rate).
    uint32_t expo_us;           // Manual exposure in us.
    int framerate;
} vd66gy_state_t;

static vd66gy_state_t vd66gy_state;

// ---------------------------------------------------------------------------
// I2C access: 16-bit big-endian register address, little-endian data.
// ---------------------------------------------------------------------------
static int vd_read(omv_csi_t *csi, uint16_t reg, uint8_t *buf, size_t len) {
    uint8_t addr[2] = { reg >> 8, reg & 0xFF };
    int ret = omv_i2c_write(csi->i2c, csi->slv_addr, addr, 2, OMV_I2C_XFER_NO_STOP);
    ret |= omv_i2c_read(csi->i2c, csi->slv_addr, buf, len, OMV_I2C_XFER_NO_FLAGS);
    return ret;
}

static int vd_write(omv_csi_t *csi, uint16_t reg, const uint8_t *buf, size_t len) {
    // Chunked so large patch arrays never need a big stack buffer.
    uint8_t tmp[2 + 64];
    int ret = 0;
    while (len && ret == 0) {
        size_t n = len > 64 ? 64 : len;
        tmp[0] = reg >> 8;
        tmp[1] = reg & 0xFF;
        memcpy(&tmp[2], buf, n);
        ret = omv_i2c_write(csi->i2c, csi->slv_addr, tmp, n + 2, OMV_I2C_XFER_NO_FLAGS);
        reg += n;
        buf += n;
        len -= n;
    }
    return ret;
}

static int rd8(omv_csi_t *csi, uint16_t reg, uint8_t *v) {
    return vd_read(csi, reg, v, 1);
}

static int rd16(omv_csi_t *csi, uint16_t reg, uint16_t *v) {
    uint8_t b[2] = { 0 };
    int ret = vd_read(csi, reg, b, 2);
    *v = b[0] | (b[1] << 8);
    return ret;
}

static int wr8(omv_csi_t *csi, uint16_t reg, uint8_t v) {
    return vd_write(csi, reg, &v, 1);
}

static int wr16(omv_csi_t *csi, uint16_t reg, uint16_t v) {
    uint8_t b[2] = { v & 0xFF, v >> 8 };
    return vd_write(csi, reg, b, 2);
}

// ---------------------------------------------------------------------------
// ST driver callbacks.
// ---------------------------------------------------------------------------
static void st_shutdown_pin(VD6G_Ctx_t *ctx, int value) {
    #if defined(OMV_CSI_RESET_PIN)
    omv_gpio_write(OMV_CSI_RESET_PIN, value ? 1 : 0);
    #endif
}

static int st_read8(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t *value) {
    return rd8(vd66gy_state.csi, addr, value);
}

static int st_read16(VD6G_Ctx_t *ctx, uint16_t addr, uint16_t *value) {
    return rd16(vd66gy_state.csi, addr, value);
}

static int st_read32(VD6G_Ctx_t *ctx, uint16_t addr, uint32_t *value) {
    uint8_t b[4] = { 0 };
    int ret = vd_read(vd66gy_state.csi, addr, b, 4);
    *value = b[0] | (b[1] << 8) | (b[2] << 16) | ((uint32_t) b[3] << 24);
    return ret;
}

static int st_write8(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t value) {
    return wr8(vd66gy_state.csi, addr, value);
}

static int st_write16(VD6G_Ctx_t *ctx, uint16_t addr, uint16_t value) {
    return wr16(vd66gy_state.csi, addr, value);
}

static int st_write32(VD6G_Ctx_t *ctx, uint16_t addr, uint32_t value) {
    uint8_t b[4] = { value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, value >> 24 };
    return vd_write(vd66gy_state.csi, addr, b, 4);
}

static int st_write_array(VD6G_Ctx_t *ctx, uint16_t addr, uint8_t *data, int data_len) {
    return vd_write(vd66gy_state.csi, addr, data, data_len);
}

static void st_delay(VD6G_Ctx_t *ctx, uint32_t delay_in_ms) {
    mp_hal_delay_ms(delay_in_ms);
}

// ---------------------------------------------------------------------------
// Helpers.
// ---------------------------------------------------------------------------
static int poll8(omv_csi_t *csi, uint16_t reg, uint8_t val, uint32_t timeout_ms) {
    uint32_t start = mp_hal_ticks_ms();
    uint8_t v = 0xFF;
    do {
        if (rd8(csi, reg, &v) != 0) {
            return -1;
        }
        if (v == val) {
            return 0;
        }
        mp_hal_delay_ms(1);
    } while ((mp_hal_ticks_ms() - start) < timeout_ms);
    return -1;
}

static uint32_t line_time_ns(vd66gy_state_t *s) {
    return (uint32_t) (((uint64_t) s->line_len * 1000000000ULL) / VD66GY_PIXEL_CLOCK);
}

static uint16_t min_frame_len(vd66gy_state_t *s) {
    uint32_t fl = (uint32_t) s->rows + s->vblank;
    return (fl > 0xFFFF) ? 0xFFFF : fl;
}

// Frame length in lines for the requested frame time or frame rate.
static uint16_t compute_frame_len(vd66gy_state_t *s) {
    uint64_t fl;
    if (s->frame_us) {
        fl = ((uint64_t) s->frame_us * VD66GY_PIXEL_CLOCK) / (1000000ULL * s->line_len);
    } else if (s->framerate > 0) {
        fl = VD66GY_PIXEL_CLOCK / ((uint64_t) s->line_len * s->framerate);
    } else {
        fl = 0; // As fast as possible.
    }
    if (fl < min_frame_len(s)) {
        fl = min_frame_len(s);
    }
    if (fl > 0xFFFF) {
        fl = 0xFFFF;
    }
    return fl;
}

// Exposure in lines, clamped to what the current frame length allows.
static uint16_t compute_expo_lines(omv_csi_t *csi, vd66gy_state_t *s) {
    uint16_t margin = 68;
    rd16(csi, REG_EXP_COARSE_INTG_MARGIN, &margin);
    uint32_t lt = line_time_ns(s);
    uint32_t lines = lt ? (uint32_t) (((uint64_t) s->expo_us * 1000 + lt - 1) / lt) : VD66GY_MIN_EXP_LINES;
    int32_t max_lines = (int32_t) s->frame_len - margin - VD66GY_EXP_OFFSET;
    if (max_lines < VD66GY_MIN_EXP_LINES) {
        max_lines = VD66GY_MIN_EXP_LINES;
    }
    if (lines < VD66GY_MIN_EXP_LINES) {
        lines = VD66GY_MIN_EXP_LINES;
    }
    if ((int32_t) lines > max_lines) {
        lines = max_lines;
    }
    return lines;
}

static int write_exposure(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    int ret = 0;
    if (s->expo_auto && s->gain_auto) {
        ret |= wr8(csi, REG_EXP_MODE, EXP_AUTO);
    } else {
        ret |= wr8(csi, REG_MANUAL_ANALOG_GAIN, s->again);
        ret |= wr16(csi, REG_MANUAL_COARSE_EXP, compute_expo_lines(csi, s));
        for (int c = 0; c < 4; c++) {
            ret |= wr16(csi, REG_MANUAL_DIGITAL_GAIN(c), 0x100); // 1.0x (8.8 fixed point)
        }
        ret |= wr8(csi, REG_EXP_MODE, EXP_MANUAL);
    }
    return ret;
}

static int write_frame_len(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    uint16_t old = s->frame_len;
    s->frame_len = compute_frame_len(s);
    bool manual = !(s->expo_auto && s->gain_auto);
    int ret = 0;
    // A manual exposure must always fit in the frame: when the frame gets shorter, shorten
    // the exposure first; when it gets longer, lengthen the frame first.
    if (manual && s->frame_len < old) {
        ret |= wr16(csi, REG_MANUAL_COARSE_EXP, compute_expo_lines(csi, s));
    }
    ret |= wr16(csi, REG_FRAME_LENGTH, s->frame_len);
    if (manual && s->frame_len >= old) {
        ret |= wr16(csi, REG_MANUAL_COARSE_EXP, compute_expo_lines(csi, s));
    }
    return ret;
}

static void update_cfa(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    // Pattern for the sensor's readout orientation (ST vd6g.c VD6G_SetBayerType).
    if (s->hmirror && s->vflip) {
        csi->cfa_format = SUBFORMAT_ID_GBRG;
    } else if (s->hmirror) {
        csi->cfa_format = SUBFORMAT_ID_RGGB;
    } else if (s->vflip) {
        csi->cfa_format = SUBFORMAT_ID_BGGR;
    } else {
        csi->cfa_format = SUBFORMAT_ID_GRBG;
    }
}

static int stream_stop(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    uint8_t fsm = 0;
    rd8(csi, REG_SYSTEM_FSM, &fsm);
    if (fsm != FSM_STREAMING) {
        s->streaming = false;
        return 0;
    }
    int ret = wr8(csi, REG_STREAMING, CMD_STOP_STREAM);
    ret |= poll8(csi, REG_STREAMING, CMD_ACK, 500);
    ret |= poll8(csi, REG_SYSTEM_FSM, FSM_SW_STBY, 500);
    s->streaming = false;
    return ret;
}

static int stream_start(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    int ret = wr8(csi, REG_STBY, CMD_START_STREAM);
    ret |= poll8(csi, REG_STBY, CMD_ACK, 500);
    ret |= poll8(csi, REG_SYSTEM_FSM, FSM_STREAMING, 500);
    s->streaming = (ret == 0);
    return ret;
}

// Full (re)configuration in software standby, then restart streaming.
static int configure(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    int ret = 0;

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return 0;
    }

    uint32_t w = csi->resolution[csi->framesize][0];
    uint32_t h = csi->resolution[csi->framesize][1];
    if (!w || !h || (w & 1) || (h & 1)) {
        return -1;
    }

    // Choose the digital binning factor.
    uint32_t bin = s->bin_req;
    if (bin == 0) {
        bin = ((2 * w) <= VD6G_MAX_WIDTH && (2 * h) <= VD6G_MAX_HEIGHT) ? 2 : 1;
    }
    if ((bin != 1 && bin != 2 && bin != 4) || (w * bin) > VD6G_MAX_WIDTH || (h * bin) > VD6G_MAX_HEIGHT) {
        return -1;
    }
    s->bin = bin;

    // Centred ROI on the array, start coordinates even to keep the CFA phase.
    uint32_t cw = w * bin;
    uint32_t ch = h * bin;
    uint32_t x0 = (VD66GY_ARRAY_X0 + (VD6G_MAX_WIDTH - cw) / 2) & ~1u;
    uint32_t y0 = ((VD6G_MAX_HEIGHT - ch) / 2) & ~1u;
    s->rows = ch;

    ret |= stream_stop(csi);

    if (s->line_len_req) {
        ret |= wr16(csi, REG_LINE_LENGTH, s->line_len_req);
    }
    ret |= rd16(csi, REG_LINE_LENGTH, &s->line_len);
    if (!s->line_len) {
        return -1;
    }

    // MIPI CSI-2 output: 2 lanes, RAW10, lane swaps as used by ST for the STEVAL modules.
    const uint16_t oif_ctrl = (1 << 9) |    // data lane 1 swap
                              (1 << 7) |    // !data_lanes_mapping_swap
                              (1 << 6) |    // data lane 0 swap
                              (0 << 4) |    // data_lanes_mapping_swap
                              (1 << 3) |    // clock lane swap
                              (2 << 0);     // 2 data lanes
    ret |= wr8(csi, REG_FORMAT_CTRL, 10);
    ret |= wr16(csi, REG_OIF_CTRL, oif_ctrl);
    ret |= wr16(csi, REG_OIF_CSI_BITRATE, VD66GY_CSI_MBPS);
    ret |= wr8(csi, REG_OIF_IMG_CTRL, RAW10_DATA_TYPE);

    // Readout window and binning.
    ret |= wr8(csi, REG_READOUT_CTRL, (bin == 4) ? 2 : (bin == 2) ? 1 : 0);
    ret |= wr16(csi, REG_OUT_ROI_X_START, x0);
    ret |= wr16(csi, REG_OUT_ROI_X_END, x0 + cw - 1);
    ret |= wr16(csi, REG_OUT_ROI_Y_START, y0);
    ret |= wr16(csi, REG_OUT_ROI_Y_END, y0 + ch - 1);

    // Orientation and test pattern.
    ret |= wr8(csi, REG_ORIENTATION, (s->hmirror ? 1 : 0) | (s->vflip ? 2 : 0));
    if (s->colorbar) {
        ret |= wr8(csi, REG_DUSTER_CTRL, 0);
        ret |= wr8(csi, REG_DARKCAL_CTRL, 2);
    }
    ret |= wr16(csi, REG_PATGEN_CTRL, s->colorbar ? PATGEN_VER_COLOR_BAR : 0);

    // No flicker avoidance (strobe lighting).
    ret |= wr16(csi, REG_AE_COMPILER_CONTROL, 0);

    // GPIO modes (strobe output, inputs, ...).
    for (int i = 0; i < VD6G_GPIO_NB; i++) {
        ret |= wr8(csi, REG_GPIO_CTRL(i), s->gpio[i]);
    }

    // Frame length, then exposure (which is clamped to the frame length).
    ret |= write_frame_len(csi);
    ret |= write_exposure(csi);

    #ifdef OMV_CSI_HW_SCALE_ENABLE
    csi->src_w = w;
    csi->src_h = h;
    #endif
    update_cfa(csi);

    ret |= stream_start(csi);
    return ret;
}

// ---------------------------------------------------------------------------
// OpenMV sensor ops.
// ---------------------------------------------------------------------------
static int reset(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;

    s->streaming = false;
    s->expo_auto = true;
    s->gain_auto = true;
    s->colorbar = false;
    s->hmirror = false;
    s->vflip = false;
    s->bin_req = 0;
    s->line_len_req = 0;
    s->vblank = VD66GY_DEF_VBLANK;
    s->frame_us = 0;
    s->expo_us = VD66GY_DEF_EXPOSURE_US;
    s->framerate = 0;
    s->line_len = 1236;
    s->rows = VD6G_MAX_HEIGHT;
    s->frame_len = 0;
    for (int i = 0; i < VD6G_GPIO_NB; i++) {
        s->gpio[i] = VD6G_GPIO_GPIO_IN;
    }

    // Boot the sensor with ST's driver: reset pulse, firmware/VT patches and PLL setup.
    VD6G_Config_t config = {
        .ext_clock_freq_in_hz = OMV_VD66GY_CLK_FREQ,
        .resolution = VD6G_RES_QVGA_320_240,
        .frame_rate = 30,
        .flip_mirror_mode = VD6G_MIRROR_FLIP_NONE,
        .line_len = 0,
        .patgen = VD6G_PATGEN_DISABLE,
        .flicker = VD6G_FLICKER_FREE_NONE,
        .pixel_depth = 10,
        .out_itf = {
            .datalane_nb = 2,
            .clock_lane_swap_enable = 1,
            .data_lane0_swap_enable = 1,
            .data_lane1_swap_enable = 1,
            .data_lanes_mapping_swap_enable = 0,
        },
        .exposure_mode = VD6G_EXPOSURE_AUTO,
    };
    for (int i = 0; i < VD6G_GPIO_NB; i++) {
        config.gpio_ctrl[i] = VD6G_GPIO_GPIO_IN;
    }

    memset(&s->st, 0, sizeof(s->st));
    s->st.shutdown_pin = st_shutdown_pin;
    s->st.read8 = st_read8;
    s->st.read16 = st_read16;
    s->st.read32 = st_read32;
    s->st.write8 = st_write8;
    s->st.write16 = st_write16;
    s->st.write32 = st_write32;
    s->st.write_array = st_write_array;
    s->st.delay = st_delay;
    s->st.log = NULL;

    if (VD6G_Init(&s->st, &config) != 0) {
        return -1;
    }

    s->mono = (s->st.bayer == VD6G_BAYER_NONE);
    csi->raw_output = !s->mono;
    rd16(csi, REG_LINE_LENGTH, &s->line_len);

    // Default manual analog gain: minimum code (1x).
    uint8_t ag_min = 0;
    rd8(csi, REG_MIN_AG_CODED, &ag_min);
    s->again = ag_min & 0x1f;

    update_cfa(csi);
    return 0;
}

static int sleep(omv_csi_t *csi, int enable) {
    vd66gy_state_t *s = csi->priv;
    if (enable) {
        return stream_stop(csi);
    }
    if (!s->streaming && csi->framesize != OMV_CSI_FRAMESIZE_INVALID) {
        return stream_start(csi);
    }
    return 0;
}

static int read_reg(omv_csi_t *csi, uint16_t reg) {
    uint8_t v;
    if (rd8(csi, reg, &v) != 0) {
        return -1;
    }
    return v;
}

static int write_reg(omv_csi_t *csi, uint16_t reg, uint16_t reg_data) {
    return wr8(csi, reg, reg_data);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    vd66gy_state_t *s = csi->priv;
    switch (pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_RGB565:
            if (s->mono) {
                return -1;
            }
            break;
        case PIXFORMAT_GRAYSCALE:
            break;
        default:
            return -1;
    }
    return 0;
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    uint32_t w = csi->resolution[framesize][0];
    uint32_t h = csi->resolution[framesize][1];
    if (w > VD6G_MAX_WIDTH || h > VD6G_MAX_HEIGHT) {
        return -1;
    }
    // omv_csi sets csi->framesize after this returns; configure with the new size.
    omv_csi_framesize_t old = csi->framesize;
    csi->framesize = framesize;
    int ret = configure(csi);
    if (ret != 0) {
        csi->framesize = old;
    }
    return ret;
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    vd66gy_state_t *s = csi->priv;
    s->framerate = framerate;
    s->frame_us = 0;
    if (!s->streaming) {
        return 0;
    }
    return write_frame_len(csi);
}

// Restart the sensor stream with new settings; capture is aborted first so no frame is
// cut in half, and the ISP debayer is refreshed when the CFA pattern changed.
static int reconfigure(omv_csi_t *csi) {
    vd66gy_state_t *s = csi->priv;
    if (!s->streaming) {
        update_cfa(csi);
        return 0;
    }
    uint32_t cfa = csi->cfa_format;
    omv_csi_abort(csi, true, false);
    int ret = configure(csi);
    if (ret == 0 && csi->cfa_format != cfa && csi->pixformat != PIXFORMAT_BAYER &&
        csi->pixformat != PIXFORMAT_INVALID) {
        ret = omv_csi_config(csi, OMV_CSI_CONFIG_PIXFORMAT);
    }
    return ret;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    vd66gy_state_t *s = csi->priv;
    s->colorbar = enable;
    return reconfigure(csi);
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    vd66gy_state_t *s = csi->priv;
    s->gain_auto = enable;
    if (!enable && !isnanf(gain_db) && !isinff(gain_db)) {
        uint8_t ag_min = 0, ag_max = 28;
        rd8(csi, REG_MIN_AG_CODED, &ag_min);
        rd8(csi, REG_MAX_AG_CODED, &ag_max);
        ag_min &= 0x1f;
        ag_max &= 0x1f;
        float gain = expf((gain_db / 20.0f) * M_LN10);
        float code = VD66GY_GAIN_SCALE - (VD66GY_GAIN_SCALE / ((gain < 1.0f) ? 1.0f : gain));
        int c = (int) (code + 0.5f);
        s->again = (c < ag_min) ? ag_min : (c > ag_max) ? ag_max : c;
    }
    return write_exposure(csi);
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    vd66gy_state_t *s = csi->priv;
    uint8_t code = s->again;
    if (s->gain_auto && s->expo_auto) {
        rd8(csi, REG_MANUAL_ANALOG_GAIN, &code);
    }
    code &= 0x1f;
    *gain_db = 20.0f * log10f(VD66GY_GAIN_SCALE / (VD66GY_GAIN_SCALE - code));
    return 0;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    vd66gy_state_t *s = csi->priv;
    s->expo_auto = enable;
    if (!enable && exposure_us >= 0) {
        s->expo_us = exposure_us;
    }
    return write_exposure(csi);
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    vd66gy_state_t *s = csi->priv;
    uint16_t lines = 0;
    int ret = rd16(csi, REG_MANUAL_COARSE_EXP, &lines);
    *exposure_us = ((uint64_t) lines * line_time_ns(s)) / 1000;
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
    vd66gy_state_t *s = csi->priv;
    s->hmirror = enable;
    return reconfigure(csi);
}

static int set_vflip(omv_csi_t *csi, int enable) {
    vd66gy_state_t *s = csi->priv;
    s->vflip = enable;
    return reconfigure(csi);
}

static int ioctl(omv_csi_t *csi, int request, va_list ap) {
    vd66gy_state_t *s = csi->priv;
    int ret = 0;

    switch (request) {
        case OMV_CSI_IOCTL_SET_FRAME_TIME_US: {
            // Frame time in microseconds; 0 returns to the frame-rate setting.
            s->frame_us = va_arg(ap, int);
            ret = s->streaming ? write_frame_len(csi) : 0;
            break;
        }
        case OMV_CSI_IOCTL_GET_FRAME_TIME_US: {
            uint16_t fl = s->frame_len;
            rd16(csi, REG_FRAME_LENGTH, &fl);
            *va_arg(ap, int *) = (int) (((uint64_t) fl * s->line_len * 1000000ULL) / VD66GY_PIXEL_CLOCK);
            break;
        }
        case OMV_CSI_IOCTL_VD66GY_SET_GPIO: {
            // gpio index 0-7, control value = mode | level | polarity (see vd6g.h).
            int idx = va_arg(ap, int);
            int val = va_arg(ap, int);
            if (idx < 0 || idx >= VD6G_GPIO_NB) {
                return -1;
            }
            s->gpio[idx] = val;
            // GPIO modes are applied in standby (capture already aborted by the ioctl flag).
            ret = s->streaming ? configure(csi) : 0;
            break;
        }
        case OMV_CSI_IOCTL_VD66GY_SET_BINNING: {
            int bin = va_arg(ap, int);
            if (bin != 0 && bin != 1 && bin != 2 && bin != 4) {
                return -1;
            }
            uint8_t old = s->bin_req;
            s->bin_req = bin;
            if ((ret = configure(csi)) != 0) {
                // Size not possible with this binning: keep the previous mode running.
                s->bin_req = old;
                configure(csi);
            }
            break;
        }
        case OMV_CSI_IOCTL_VD66GY_SET_LINE_LEN: {
            // 0 keeps the current sensor value (power-cycle / reset restores the default).
            int ll = va_arg(ap, int);
            if (ll != 0 && (ll < 1000 || ll > 0xFFFF)) {
                return -1;
            }
            uint16_t old = s->line_len_req;
            s->line_len_req = ll;
            if ((ret = configure(csi)) != 0) {
                s->line_len_req = old;
                configure(csi);
            }
            break;
        }
        case OMV_CSI_IOCTL_VD66GY_SET_VBLANK: {
            int vb = va_arg(ap, int);
            if (vb < 1 || vb > 4000) {
                return -1;
            }
            s->vblank = vb;
            ret = s->streaming ? write_frame_len(csi) : 0;
            break;
        }
        case OMV_CSI_IOCTL_VD66GY_GET_INFO: {
            // line_len, frame_len, rows, bin, line time ns, min frame time us.
            int *info = va_arg(ap, int *);
            uint16_t fl = s->frame_len;
            rd16(csi, REG_FRAME_LENGTH, &fl);
            info[0] = s->line_len;
            info[1] = fl;
            info[2] = s->rows;
            info[3] = s->bin;
            info[4] = line_time_ns(s);
            info[5] = (int) (((uint64_t) min_frame_len(s) * s->line_len * 1000000ULL) / VD66GY_PIXEL_CLOCK);
            break;
        }
        default:
            ret = -1;
            break;
    }

    return ret;
}

int vd66gy_init(omv_csi_t *csi) {
    memset(&vd66gy_state, 0, sizeof(vd66gy_state));
    vd66gy_state.csi = csi;
    csi->priv = &vd66gy_state;

    // Initialize csi flags.
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->mono_bpp = 1;
    csi->raw_output = 1;
    csi->cfa_format = SUBFORMAT_ID_GRBG;
    csi->mipi_if = 1;
    csi->mipi_brate = VD66GY_CSI_MBPS;

    // Initialize csi ops.
    csi->reset = reset;
    csi->sleep = sleep;
    csi->ioctl = ioctl;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_framerate = set_framerate;
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
#endif // (OMV_VD66GY_ENABLE == 1)
