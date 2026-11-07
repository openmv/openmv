/*
 * Copyright (C) 2026 OpenMV, LLC.
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
 * ST VB1943/VB5943 driver.
 *
 * Both modules use the same sensor die. The VB1943 is the RGB-NIR (color)
 * variant and the VB5943 is the monochrome variant. This is the OpenMV
 * glue around ST's VD1943 driver (see drivers/vd1943), which boots,
 * configures and starts the sensor.
 */
#include "board_config.h"
#if (OMV_VBX943_ENABLE == 1)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "omv_i2c.h"
#include "omv_csi.h"
#include "omv_gpio.h"
#include "py/mphal.h"
#include "vd1943.h"

#define SENSOR_WIDTH            VD1943_MAX_WIDTH
#define SENSOR_HEIGHT           VD1943_MAX_HEIGHT

// Registers ST's driver has no accessors for.
#define REG_SYSTEM_PLL_CLK      (0x0228)
#define REG_ROI_X               (0x090C)
#define REG_ROI_Y               (0x090E)
#define REG_ROI_W               (0x0910)
#define REG_ROI_H               (0x0912)
#define REG_LINE_LENGTH         (0x0934)
#define REG_PWL_X               (0x0940)
#define REG_PWL_Y               (0x09C0)
#define REG_PWL_GRADIENT        (0x0A40)
#define REG_PWL_BIAS            (0x0AC0)
#define REG_FRAME_LENGTH        (0x0B46)
#define REG_PWL_SEL             (0x0B4B)
#define REG_PEDESTAL            (0x0B6E)
#define REG_EXPOSURE            (0x0C7A)
#define REG_EXPOSURE_SHORT      (0x0C7E)
#define REG_DIGITAL_GAIN_IR     (0x0C86)

#define VBX943_MIPI_BRATE       (1300) // Mbps per lane
#define VBX943_WRITE_CHUNK      (128)

// The CSI-2 link (2 lanes at 1300 Mbps) and the rows the sensor needs between
// frames limit the frame rate. A smaller readout window is faster. The limit
// is rounded down to a multiple of 10 FPS, which is 40 FPS when the sensor
// reads out all of its rows. At 40 FPS the 120 Hz flicker of mains lighting
// is the same in every frame, and at the 43 to 46 FPS the sensor can do, it
// isn't.
#define VBX943_FPS_DEFAULT      (30)
#define VBX943_FPS_MAX          (40)
#define VBX943_FPS_STEP         (10)
#define VBX943_FPS_LIMIT        (1000)
#define VBX943_VBLANK_MIN       (130) // rows

// The VB5943 reads out faster when it only outputs every 4th pixel and line.
#define VBX943_FAST_SCALE       (4)
#define VBX943_FAST_FPS_MAX     (90)

// The exposure is in lines and must end before the frame does.
#define VBX943_EXP_MARGIN       (100)
#define VBX943_EXP_MIN          (4)

// HDR merges a long and a short exposure of the same frame. The short one is
// the long one divided by the ratio, and has to fit in the rows between
// frames less the ones the sensor needs. The line is 1/20th longer too.
#define VBX943_HDR_RATIO        (16)
#define VBX943_HDR_SHORT_MIN    (2)
#define VBX943_HDR_SHORT_MAX    (396)
#define VBX943_HDR_VBLANK_MIN   (64)
#define VBX943_HDR_LINE_MARGIN  (20)

// The sensor compresses the 18-bit HDR pixels to 10 bits with a curve of 32
// segments. The slope of a segment is (256 + mantissa) * 2^(exponent - bias).
#define VBX943_PWL_POINTS       (32)
#define VBX943_PWL_BIAS         (12)
#define VBX943_PWL_SLOPE(e, m)  (((e) << 8) | (m))
#define VBX943_PWL_CUSTOM       (1)

// The curve is linear over the long exposure's range, which is 1/16th of the
// input range and gets 3/4 of the output range. Above that the slope halves
// every time the input doubles.
#define VBX943_PWL_LINEAR       (16) // segments
#define VBX943_PWL_LINEAR_X     (1024)
#define VBX943_PWL_LINEAR_Y     (12288)
#define VBX943_PWL_LINEAR_SLOPE VBX943_PWL_SLOPE(7, 128) // 12
#define VBX943_PWL_OCTAVE       (4) // segments
#define VBX943_PWL_OCTAVE_X     (4096)
#define VBX943_PWL_OCTAVE_Y     (4096)
#define VBX943_PWL_OCTAVE_SLOPE VBX943_PWL_SLOPE(4, 0) // 1

// Total gain is analog gain (1x-4x) times digital gain (1x-32x), tracked
// in 8.8 fixed-point. Analog gain is 16 / (16 - reg) and the digital gain
// registers are 5.8 fixed-point, so the 32x ST's driver allows wraps to 0.
#define VBX943_GAIN_SCALE       (256)
#define VBX943_DGAIN_MAX        (VD1943_DIGITAL_GAIN_MAX - 1)
#define VBX943_GAIN_MIN         (VBX943_GAIN_SCALE)
#define VBX943_GAIN_MAX         (VBX943_DGAIN_MAX * 4)
#define VBX943_DEF_GAINCEILING  (OMV_CSI_GAINCEILING_16X)

#define VBX943_L_TARGET         (80)
#define VBX943_L_AGC_DIFF_MIN   (4)
#define VBX943_L_FAST_THRESH    (60)
#define VBX943_L_EMA_DIV        (8)
#define VBX943_DAMP_FAST        (2)
#define VBX943_DAMP_SLOW        (4)

typedef struct vbx943_roi {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} vbx943_roi_t;

typedef struct vbx943_state {
    VD1943_Ctx_t ctx;   // Must be first, ST's driver passes it to the callbacks.
    VD1943_Config_t config;
    vbx943_roi_t roi;
    omv_csi_t *csi;
    bool mono;
    bool streaming;
    bool triggered;
    bool strobe;
    int framerate;
    omv_csi_window_t readout;
    uint32_t line_time_ns;
    bool enable_agc;
    int32_t agc_gain;
    int32_t agc_gain_ceiling;
    bool enable_aec;
    int32_t aec_exposure;
    int32_t aec_exposure_ceiling;
    int32_t aec_exposure_max;
    int32_t aec_exposure_short_max;
    int luminance_ema;
} vbx943_state_t;

static vbx943_state_t vbx943_state = {};

// The widths of the readout windows ST's driver supports.
static const uint16_t vbx943_widths[] = {
    [VD1943_RES_QVGA_320_240] = 320,
    [VD1943_RES_VGA_640_480] = 640,
    [VD1943_RES_SVGA_800_600] = 800,
    [VD1943_RES_XGA_1024_768] = 1024,
    [VD1943_RES_720P_1280_720] = 1280,
    [VD1943_RES_SXGA_1280_1024] = 1280,
    [VD1943_RES_1080P_1920_1080] = 1920,
    [VD1943_RES_QXGA_2048_1536] = 2048,
    [VD1943_RES_2560_1920] = 2560,
    [VD1943_RES_FULL_2560_1984] = 2560,
};

// The Bayer pattern of the VB1943 for each readout orientation.
static const uint8_t vbx943_cfa_formats[] = {
    [VD1943_MIRROR_FLIP_NONE] = SUBFORMAT_ID_GBRG,
    [VD1943_FLIP] = SUBFORMAT_ID_RGGB,
    [VD1943_MIRROR] = SUBFORMAT_ID_BGGR,
    [VD1943_MIRROR_FLIP] = SUBFORMAT_ID_GBRG,
};

#ifdef OMV_CSI_HW_CCM_ENABLE
// Color correction matrices of the VB1943 with an IR cut lens, tuned on the real ISP output.
// Sorted warmest to coolest.
static const omv_csi_ccm_t vbx943_ccm[] = {
    {   // A (incandescent, ~2856K)
        .avg = { 84.598f, 80.000f, 32.184f },
        .coeffs = { {  1.215f,  0.125f, -0.340f },
                    { -0.656f,  2.009f, -0.354f },
                    {  0.794f, -1.547f,  1.752f } },
    },
    {   // TL84 (fluorescent, ~4000K)
        .avg = { 69.167f, 80.000f, 41.667f },
        .coeffs = { {  1.459f, -0.112f, -0.347f },
                    { -0.481f,  1.774f, -0.293f },
                    {  0.468f, -1.255f,  1.787f } },
    },
    {   // D65 (daylight, ~6500K)
        .avg = { 53.333f, 80.000f, 57.931f },
        .coeffs = { {  1.325f, -0.404f,  0.079f },
                    { -0.235f,  1.510f, -0.275f },
                    {  0.229f, -1.117f,  1.888f } },
    },
};
#endif // OMV_CSI_HW_CCM_ENABLE

// The rolling shutter runs in HDR on the VB5943.
static bool hdr_mode(vbx943_state_t *state) {
    return state->config.image_processing_mode == VD1943_RS_HDR_NATIVE_10;
}

static bool global_shutter(vbx943_state_t *state) {
    return !hdr_mode(state) && state->config.image_processing_mode != VD1943_RS_SDR_RGB_10;
}

static int fit_timings(vbx943_state_t *state, uint16_t *frame_length);

// Bus callbacks for ST's driver. Registers are little-endian.
static int vbx943_read8(VD1943_Ctx_t *ctx, uint16_t addr, uint8_t *value) {
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    return omv_i2c_read_reg(csi->i2c, csi->slv_addr, addr, 2, value, 1);
}

static int vbx943_read16(VD1943_Ctx_t *ctx, uint16_t addr, uint16_t *value) {
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, addr, 2, value, 2);
    *value = __builtin_bswap16(*value);
    return ret;
}

static int vbx943_read32(VD1943_Ctx_t *ctx, uint16_t addr, uint32_t *value) {
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    int ret = omv_i2c_read_reg(csi->i2c, csi->slv_addr, addr, 2, value, 4);
    *value = __builtin_bswap32(*value);
    return ret;
}

static int vbx943_write8(VD1943_Ctx_t *ctx, uint16_t addr, uint8_t value) {
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    return omv_i2c_write_reg(csi->i2c, csi->slv_addr, addr, 2, value, 1);
}

static int vbx943_write16(VD1943_Ctx_t *ctx, uint16_t addr, uint16_t value) {
    vbx943_state_t *state = (vbx943_state_t *) ctx;
    omv_csi_t *csi = state->csi;

    // ST's driver only knows of its own readout windows and frame length
    // limits. So, the ones to use replace what it writes when it sets up
    // the sensor to start streaming.
    switch (addr) {
        case REG_ROI_X:
            value = state->roi.x;
            break;
        case REG_ROI_Y:
            value = state->roi.y;
            break;
        case REG_ROI_W:
            value = state->roi.w;
            break;
        case REG_ROI_H:
            value = state->roi.h;
            break;
        case REG_LINE_LENGTH:
            // ST's driver makes the line just long enough for the link to
            // output a row per line. In HDR the sensor can't buffer rows
            // the link is late with, and it stops streaming when it is.
            if (hdr_mode(state)) {
                value = (value + (value / VBX943_HDR_LINE_MARGIN) + 3) & ~3;
            }
            break;
        case REG_FRAME_LENGTH:
            if (fit_timings(state, &value) != 0) {
                return -1;
            }
            break;
        default:
            break;
    }

    return omv_i2c_write_reg(csi->i2c, csi->slv_addr, addr, 2, __builtin_bswap16(value), 2);
}

static int vbx943_write32(VD1943_Ctx_t *ctx, uint16_t addr, uint32_t value) {
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    return omv_i2c_write_reg(csi->i2c, csi->slv_addr, addr, 2, __builtin_bswap32(value), 4);
}

static int vbx943_write_array(VD1943_Ctx_t *ctx, uint16_t addr, uint8_t *data, int data_len) {
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    uint8_t buf[2 + VBX943_WRITE_CHUNK];
    int ret = 0;

    while (data_len && ret == 0) {
        int chunk = IM_MIN(data_len, VBX943_WRITE_CHUNK);
        buf[0] = addr >> 8;
        buf[1] = addr;
        memcpy(buf + 2, data, chunk);
        ret = omv_i2c_write(csi->i2c, csi->slv_addr, buf, chunk + 2, OMV_I2C_XFER_NO_FLAGS);
        addr += chunk;
        data += chunk;
        data_len -= chunk;
    }

    return ret;
}

static void vbx943_delay(VD1943_Ctx_t *ctx, uint32_t delay_in_ms) {
    mp_hal_delay_ms(delay_in_ms);
}

static void vbx943_shutdown_pin(VD1943_Ctx_t *ctx, int value) {
    #if defined(OMV_CSI_RESET_PIN)
    omv_csi_t *csi = ((vbx943_state_t *) ctx)->csi;
    omv_gpio_write(OMV_CSI_RESET_PIN, value ? !csi->reset_pol : csi->reset_pol);
    #endif
}

// Split the total gain into the largest analog gain that does not exceed
// it, with the remainder applied as digital gain.
static int apply_gain(omv_csi_t *csi, int32_t gain) {
    vbx943_state_t *state = csi->priv;
    int32_t again = 16 - ((16 * VBX943_GAIN_SCALE + gain - 1) / gain);
    again = IM_CLAMP(again, VD1943_ANALOG_GAIN_MIN, VD1943_ANALOG_GAIN_MAX);

    int32_t dgain = (gain * (16 - again)) / 16;
    dgain = IM_CLAMP(dgain, VD1943_DIGITAL_GAIN_MIN, VBX943_DGAIN_MAX);

    int ret = VD1943_Hold(&state->ctx, 1);
    ret |= VD1943_SetAnalogGain(&state->ctx, again);
    ret |= VD1943_SetDigitalGain(&state->ctx, dgain);

    // A quarter of the VB5943's pixels use the gain of the sensor's NIR
    // channel, which ST's driver doesn't set in HDR.
    if (state->mono) {
        ret |= vbx943_write16(&state->ctx, REG_DIGITAL_GAIN_IR, dgain);
    }

    ret |= VD1943_Hold(&state->ctx, 0);
    return ret;
}

// ST's driver converts exposure times using a fixed line time, which is
// only right for the shortest line length. So, the exposure is programmed
// here, in lines, using the line time of the running configuration.
static int write_exposure(vbx943_state_t *state, int32_t lines) {
    int ret = vbx943_write16(&state->ctx, REG_EXPOSURE, lines);

    // Only HDR uses the short exposure. Without HDR the limit keeps it at
    // the value the sensor needs it to have.
    int32_t lines_short = IM_CLAMP(lines / VBX943_HDR_RATIO, VBX943_HDR_SHORT_MIN, state->aec_exposure_short_max);
    return ret | vbx943_write16(&state->ctx, REG_EXPOSURE_SHORT, lines_short);
}

static int apply_exposure(omv_csi_t *csi, int32_t lines) {
    vbx943_state_t *state = csi->priv;
    int ret = VD1943_Hold(&state->ctx, 1);
    ret |= write_exposure(state, lines);
    ret |= VD1943_Hold(&state->ctx, 0);
    return ret;
}

// In HDR, the shortest short exposure sets the shortest long exposure.
static int32_t exposure_min_lines(omv_csi_t *csi) {
    vbx943_state_t *state = csi->priv;
    return hdr_mode(state) ? (VBX943_HDR_SHORT_MIN * VBX943_HDR_RATIO) : VBX943_EXP_MIN;
}

static int32_t exposure_us_to_lines(omv_csi_t *csi, int exposure_us) {
    vbx943_state_t *state = csi->priv;
    int32_t exposure_min = exposure_min_lines(csi);
    int32_t lines = ((int64_t) exposure_us * 1000) / state->line_time_ns;
    return IM_CLAMP(lines, exposure_min, state->aec_exposure_max);
}

// Boot the sensor. It derives its timings from the frequency of its clock,
// so it's given the exact frequency rather than the nominal one.
static int boot(omv_csi_t *csi) {
    vbx943_state_t *state = csi->priv;

    state->streaming = false;
    state->config.ext_clock_freq_in_hz = omv_csi_get_clk_frequency(csi, false);
    if (VD1943_Init(&state->ctx, &state->config) != 0) {
        return -1;
    }

    // Black is zero without the pedestal the sensor adds to its output.
    return vbx943_write16(&state->ctx, REG_PEDESTAL, 0);
}

// ST's driver writes the frame length last, once the line length is set,
// and then starts streaming. So, this is where the frame length and the
// exposure are fit to the new timings. The sensor can stop streaming, with
// no error, when they break its limits.
static int fit_timings(vbx943_state_t *state, uint16_t *frame_length) {
    omv_csi_t *csi = state->csi;
    uint32_t pll_clk = 0;
    uint16_t line_length = 0;

    int ret = vbx943_read32(&state->ctx, REG_SYSTEM_PLL_CLK, &pll_clk);
    ret |= vbx943_read16(&state->ctx, REG_LINE_LENGTH, &line_length);
    if (ret != 0 || !pll_clk || !line_length) {
        return -1;
    }

    // The sensor needs time between frames, and reads out two rows per
    // line with its global shutter.
    uint32_t frame_length_min = (state->roi.h + VBX943_VBLANK_MIN) / (global_shutter(state) ? 2 : 1);

    // Round the fastest frame rate down.
    uint32_t frame_rate_max = pll_clk / (4 * line_length * frame_length_min);
    if (frame_rate_max >= VBX943_FPS_STEP) {
        frame_rate_max -= frame_rate_max % VBX943_FPS_STEP;
        frame_length_min = pll_clk / (4 * line_length * frame_rate_max);
    }
    *frame_length = IM_MAX(*frame_length, frame_length_min);

    // A line is 4 PLL clocks per pixel long.
    uint32_t line_time_ns = ((uint64_t) line_length * 4 * 1000000000) / pll_clk;
    int32_t exposure_max = *frame_length - VBX943_EXP_MARGIN;

    if (!hdr_mode(state)) {
        state->aec_exposure_short_max = VBX943_HDR_SHORT_MIN;
    } else {
        // Both exposures have to fit in the frame, and the short one in the
        // rows between frames too. The sensor scales its output by the short
        // exposure, so the long one can't be longer than the ratio allows.
        int32_t short_max = *frame_length - state->roi.h - VBX943_HDR_VBLANK_MIN;
        state->aec_exposure_short_max = IM_MIN(short_max, VBX943_HDR_SHORT_MAX);
        exposure_max = IM_MIN(exposure_max - state->aec_exposure_short_max,
                              state->aec_exposure_short_max * VBX943_HDR_RATIO);
    }

    // The exposure ceiling tracks the frame length unless the user lowered it.
    if (state->aec_exposure_ceiling == state->aec_exposure_max) {
        state->aec_exposure_ceiling = exposure_max;
    } else if (state->line_time_ns) {
        state->aec_exposure_ceiling = ((int64_t) state->aec_exposure_ceiling * state->line_time_ns +
                                       line_time_ns / 2) / line_time_ns;
    }

    // Keep the exposure time when the sensor mode changes the line time.
    if (state->line_time_ns) {
        state->aec_exposure = ((int64_t) state->aec_exposure * state->line_time_ns +
                               line_time_ns / 2) / line_time_ns;
    }

    state->line_time_ns = line_time_ns;
    state->aec_exposure_max = exposure_max;
    state->aec_exposure_ceiling = IM_MIN(state->aec_exposure_ceiling, state->aec_exposure_max);
    state->aec_exposure = IM_CLAMP(state->aec_exposure, exposure_min_lines(csi), state->aec_exposure_ceiling);
    return write_exposure(state, state->aec_exposure);
}

// The sensor's HDR curve is close to logarithmic and the receiver is meant
// to undo it, which the ISP can't. So, the sensor is given a curve that keeps
// most pixels linear and only compresses the highlights. The sensor forgets
// the curve when it changes modes.
static int write_tone_curve(vbx943_state_t *state) {
    uint32_t x[VBX943_PWL_POINTS], y[VBX943_PWL_POINTS], gradient[VBX943_PWL_POINTS];
    uint32_t x_next = 0, y_next = 0;

    for (int i = 0; i < VBX943_PWL_POINTS; i++) {
        int octave = (i - VBX943_PWL_LINEAR) / VBX943_PWL_OCTAVE;
        x[i] = x_next;
        y[i] = y_next;

        if (i < VBX943_PWL_LINEAR) {
            gradient[i] = VBX943_PWL_LINEAR_SLOPE;
            x_next += VBX943_PWL_LINEAR_X;
            y_next += VBX943_PWL_LINEAR_Y;
        } else {
            gradient[i] = VBX943_PWL_OCTAVE_SLOPE - VBX943_PWL_SLOPE(octave, 0);
            x_next += VBX943_PWL_OCTAVE_X << octave;
            y_next += VBX943_PWL_OCTAVE_Y;
        }
    }

    int ret = vbx943_write_array(&state->ctx, REG_PWL_X, (uint8_t *) x, sizeof(x));
    ret |= vbx943_write_array(&state->ctx, REG_PWL_Y, (uint8_t *) y, sizeof(y));
    ret |= vbx943_write_array(&state->ctx, REG_PWL_GRADIENT, (uint8_t *) gradient, sizeof(gradient));
    ret |= vbx943_write32(&state->ctx, REG_PWL_BIAS, VBX943_PWL_BIAS);
    ret |= vbx943_write8(&state->ctx, REG_PWL_SEL, VBX943_PWL_CUSTOM);
    return ret;
}

static int start(omv_csi_t *csi) {
    vbx943_state_t *state = csi->priv;

    if (hdr_mode(state) && write_tone_curve(state) != 0) {
        return -1;
    }

    if (VD1943_Start(&state->ctx) != 0) {
        return -1;
    }
    state->streaming = true;

    return apply_gain(csi, state->agc_gain);
}

// ST's driver only takes a configuration when it boots the sensor, and a
// reboot glitches the CSI-2 lanes. So, to change the configuration of a
// running sensor, update the driver's copy of it while streaming is off.
static int reconfigure(omv_csi_t *csi) {
    vbx943_state_t *state = csi->priv;

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    if (state->streaming && VD1943_Stop(&state->ctx) != 0) {
        return -1;
    }
    state->streaming = false;

    state->ctx.ctx.config_save = state->config;
    return start(csi);
}

// The ISP has to follow the Bayer pattern of the VB1943, which changes with
// the readout orientation.
static int reconfigure_isp(omv_csi_t *csi) {
    vbx943_state_t *state = csi->priv;

    if (state->mono || csi->pixformat == PIXFORMAT_INVALID) {
        return 0;
    }
    return omv_csi_config(csi, OMV_CSI_CONFIG_PIXFORMAT);
}

// Pick the sensor mode, readout window and frame rate.
static int select_config(omv_csi_t *csi, pixformat_t pixformat, omv_csi_framesize_t framesize,
                         VD1943_Config_t *config, vbx943_roi_t *roi) {
    vbx943_state_t *state = csi->priv;
    uint32_t w = state->readout.w;
    uint32_t h = state->readout.h;
    uint32_t scale = 1;
    int framerate_max = VBX943_FPS_LIMIT;

    // Triggered frames and the strobe need the global shutter. Otherwise the
    // sensor runs with its rolling shutter.
    bool global = state->triggered || state->strobe;

    #ifdef OMV_CSI_HW_SCALE_ENABLE
    bool scaled = (pixformat != PIXFORMAT_BAYER);
    #else
    bool scaled = false;
    #endif // OMV_CSI_HW_SCALE_ENABLE

    // The sensor's GPIO0 is wired to the frame sync pin on OpenMV boards. In
    // triggered mode the sensor starts a frame on the rising edge of the pin,
    // and in strobe mode it drives the pin high while it exposes a frame.
    config->sync_mode = state->triggered ? VD1943_SLAVE : VD1943_MASTER;
    config->gpios[0].gpio_ctrl = state->triggered ? VD1943_GPIO_FSYNC_IN :
                                 state->strobe ? VD1943_GPIO_STROBE : VD1943_GPIO_IN;
    config->gpios[0].enable = state->triggered || state->strobe;

    if (state->mono) {
        config->image_processing_mode = global ? VD1943_GS_SS1_NATIVE_10 : VD1943_RS_HDR_NATIVE_10;
    } else {
        // The sensor converts its RGB-NIR pattern to a Bayer pattern. HDR isn't used as its
        // curve compresses highlights ahead of the ISP's white balance, which turns them pink.
        config->image_processing_mode = global ? VD1943_GS_SS1_RGB_10 : VD1943_RS_SDR_RGB_10;
    }

    if (framesize != OMV_CSI_FRAMESIZE_INVALID) {
        uint32_t frame_w = csi->resolution[framesize][0];
        uint32_t frame_h = csi->resolution[framesize][1];

        if (frame_w > SENSOR_WIDTH || frame_h > SENSOR_HEIGHT) {
            return -1;
        }

        if (!scaled) {
            // The ISP can't scale the frame, so the sensor reads out a window of its size.
            if ((frame_w | frame_h) & 3) {
                return -1;
            }
            w = frame_w;
            h = frame_h;
        } else if (state->mono && state->framerate > VBX943_FPS_MAX &&
                   w == SENSOR_WIDTH && h == SENSOR_HEIGHT &&
                   frame_w <= (SENSOR_WIDTH / VBX943_FAST_SCALE) && frame_h <= (SENSOR_HEIGHT / VBX943_FAST_SCALE)) {
            // Small frames can trade the full resolution readout for a
            // higher frame rate, with the global shutter.
            config->image_processing_mode = VD1943_GS_SS4_MONO_10;
            scale = VBX943_FAST_SCALE;
            framerate_max = VBX943_FAST_FPS_MAX;
        } else {
            // The ISP can only scale the readout window down to the frame.
            w = (IM_MAX(w, frame_w) + 3) & ~3;
            h = (IM_MAX(h, frame_h) + 3) & ~3;
        }
    }

    // ST's driver sets the line length for one of its windows, so it's
    // given the narrowest one the readout window fits in.
    config->resolution = 0;
    while (vbx943_widths[config->resolution] < w) {
        config->resolution++;
    }

    // The readout window is centered unless it was moved.
    int32_t x_max = (SENSOR_WIDTH - w) / 2;
    int32_t y_max = (SENSOR_HEIGHT - h) / 2;
    roi->x = (x_max + IM_CLAMP(state->readout.x, -x_max, x_max)) & ~3;
    roi->y = (y_max + IM_CLAMP(state->readout.y, -y_max, y_max)) & ~3;
    roi->w = w / scale;
    roi->h = h / scale;

    config->frame_rate = IM_CLAMP(state->framerate, 1, framerate_max);

    #ifdef OMV_CSI_HW_SCALE_ENABLE
    // The ISP scales the sensor's output down to the frame size.
    csi->src_w = roi->w;
    csi->src_h = roi->h;
    #endif // OMV_CSI_HW_SCALE_ENABLE
    return 0;
}

static int configure(omv_csi_t *csi, pixformat_t pixformat, omv_csi_framesize_t framesize) {
    vbx943_state_t *state = csi->priv;
    VD1943_Config_t config = state->config;
    vbx943_roi_t roi = state->roi;

    if (select_config(csi, pixformat, framesize, &config, &roi) != 0) {
        return -1;
    }

    if (!memcmp(&config, &state->config, sizeof(config)) && !memcmp(&roi, &state->roi, sizeof(roi))) {
        return 0;
    }

    state->config = config;
    state->roi = roi;
    return reconfigure(csi);
}

// Stop driving the frame sync pin while the sensor outputs its strobe on it.
static void fsync_pin_config(omv_csi_t *csi, bool strobe) {
    if (csi->fsync_pin) {
        omv_gpio_config(csi->fsync_pin, strobe ? OMV_GPIO_MODE_INPUT : OMV_GPIO_MODE_OUTPUT,
                        OMV_GPIO_PULL_NONE, OMV_GPIO_SPEED_LOW, -1);
        if (!strobe) {
            omv_gpio_write(csi->fsync_pin, 0);
        }
    }
}

// The host's trigger and the sensor's strobe share the frame sync pin, so
// only one of them can be on.
static int set_frame_sync(omv_csi_t *csi, bool triggered, bool strobe) {
    vbx943_state_t *state = csi->priv;

    if (state->triggered == triggered && state->strobe == strobe) {
        return 0;
    }

    state->triggered = triggered;
    state->strobe = strobe;
    fsync_pin_config(csi, strobe);
    return configure(csi, csi->pixformat, csi->framesize);
}

static void reset_state(omv_csi_t *csi) {
    vbx943_state_t *state = csi->priv;

    state->triggered = false;
    state->strobe = false;
    state->framerate = VBX943_FPS_DEFAULT;
    state->readout.x = 0;
    state->readout.y = 0;
    state->readout.w = SENSOR_WIDTH;
    state->readout.h = SENSOR_HEIGHT;
    state->config.flip_mirror_mode = VD1943_MIRROR_FLIP_NONE;
    state->config.patgen = VD1943_PATGEN_DISABLE;
    csi->cfa_format = vbx943_cfa_formats[VD1943_MIRROR_FLIP_NONE];
    select_config(csi, PIXFORMAT_INVALID, OMV_CSI_FRAMESIZE_INVALID, &state->config, &state->roi);

    state->enable_agc = true;
    state->agc_gain = VBX943_GAIN_MIN;
    state->agc_gain_ceiling = VBX943_GAIN_SCALE << (VBX943_DEF_GAINCEILING + 1);
    csi->gainceiling = VBX943_DEF_GAINCEILING;

    // Start from the longest exposure.
    state->enable_aec = true;
    state->line_time_ns = 0;
    state->aec_exposure = INT32_MAX;
    state->aec_exposure_ceiling = 0;
    state->aec_exposure_max = 0;
    state->aec_exposure_short_max = VBX943_HDR_SHORT_MIN;
    state->luminance_ema = VBX943_L_TARGET;
}

static int reset(omv_csi_t *csi) {
    fsync_pin_config(csi, false);
    reset_state(csi);

    if (boot(csi) != 0) {
        return -1;
    }
    return start(csi);
}

static int sleep(omv_csi_t *csi, int enable) {
    vbx943_state_t *state = csi->priv;
    int ret = 0;

    if (enable && state->streaming) {
        ret = VD1943_Stop(&state->ctx);
        state->streaming = false;
    } else if (!enable && !state->streaming) {
        ret = start(csi);
    }

    return ret;
}

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    vbx943_state_t *state = csi->priv;
    uint8_t reg_data;

    if (vbx943_read8(&state->ctx, reg_addr, &reg_data) != 0) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    vbx943_state_t *state = csi->priv;
    return vbx943_write8(&state->ctx, reg_addr, reg_data);
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    vbx943_state_t *state = csi->priv;

    switch (pixformat) {
        case PIXFORMAT_RGB565:
        case PIXFORMAT_BAYER:
            if (state->mono) {
                return -1;
            }
            break;
        case PIXFORMAT_GRAYSCALE:
            break;
        default:
            return -1;
    }

    return configure(csi, pixformat, csi->framesize);
}

static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    return configure(csi, csi->pixformat, framesize);
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    vbx943_state_t *state = csi->priv;
    state->framerate = framerate;
    return configure(csi, csi->pixformat, csi->framesize);
}

static int set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    vbx943_state_t *state = csi->priv;
    int32_t gain = VBX943_GAIN_SCALE << (gainceiling + 1);

    state->agc_gain_ceiling = IM_MIN(gain, VBX943_GAIN_MAX);
    return 0;
}

static int set_colorbar(omv_csi_t *csi, int enable) {
    vbx943_state_t *state = csi->priv;
    state->config.patgen = enable ? VD1943_PATGEN_DIAGONAL_GRAYSCALE : VD1943_PATGEN_DISABLE;
    return reconfigure(csi);
}

static int set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    vbx943_state_t *state = csi->priv;
    int ret = 0;

    state->enable_agc = enable;

    if ((enable == 0) && (!isnanf(gain_db)) && (!isinff(gain_db))) {
        int32_t gain = fast_roundf(expf((gain_db / 20.0f) * M_LN10) * VBX943_GAIN_SCALE);
        state->agc_gain = IM_CLAMP(gain, VBX943_GAIN_MIN, VBX943_GAIN_MAX);
        ret |= apply_gain(csi, state->agc_gain);
    } else if ((enable != 0) && (!isnanf(gain_db_ceiling)) && (!isinff(gain_db_ceiling))) {
        int32_t gain = fast_roundf(expf((gain_db_ceiling / 20.0f) * M_LN10) * VBX943_GAIN_SCALE);
        state->agc_gain_ceiling = IM_CLAMP(gain, VBX943_GAIN_MIN, VBX943_GAIN_MAX);
    }

    return ret;
}

static int get_gain_db(omv_csi_t *csi, float *gain_db) {
    vbx943_state_t *state = csi->priv;
    *gain_db = 20.0f * log10f((float) state->agc_gain / VBX943_GAIN_SCALE);
    return 0;
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    vbx943_state_t *state = csi->priv;
    int ret = 0;

    state->enable_aec = enable;

    // The exposure is in lines, which have no duration until the sensor starts.
    if (!state->line_time_ns) {
        return -1;
    }

    if ((enable == 0) && (exposure_us >= 0)) {
        state->aec_exposure = exposure_us_to_lines(csi, exposure_us);
        ret |= apply_exposure(csi, state->aec_exposure);
    } else if ((enable != 0) && (exposure_us >= 0)) {
        state->aec_exposure_ceiling = exposure_us_to_lines(csi, exposure_us);
    }

    return ret;
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    vbx943_state_t *state = csi->priv;
    *exposure_us = ((int64_t) state->aec_exposure * state->line_time_ns) / 1000;
    return 0;
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

static int set_orientation(omv_csi_t *csi, bool hmirror, bool vflip) {
    vbx943_state_t *state = csi->priv;

    if (hmirror && vflip) {
        state->config.flip_mirror_mode = VD1943_MIRROR_FLIP;
    } else if (hmirror) {
        state->config.flip_mirror_mode = VD1943_MIRROR;
    } else if (vflip) {
        state->config.flip_mirror_mode = VD1943_FLIP;
    } else {
        state->config.flip_mirror_mode = VD1943_MIRROR_FLIP_NONE;
    }

    csi->cfa_format = vbx943_cfa_formats[state->config.flip_mirror_mode];
    int ret = reconfigure(csi);
    ret |= reconfigure_isp(csi);
    return ret;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    return set_orientation(csi, enable, csi->vflip);
}

static int set_vflip(omv_csi_t *csi, int enable) {
    return set_orientation(csi, csi->hmirror, enable);
}

static int update_agc_aec(omv_csi_t *csi, int luminance) {
    vbx943_state_t *state = csi->priv;
    int32_t exposure_min = exposure_min_lines(csi);
    int ret = 0;

    // The ceilings may have been lowered below the current values.
    if (state->enable_aec && state->aec_exposure > state->aec_exposure_ceiling) {
        state->aec_exposure = state->aec_exposure_ceiling;
        ret |= apply_exposure(csi, state->aec_exposure);
    }
    if (state->enable_agc && state->agc_gain > state->agc_gain_ceiling) {
        state->agc_gain = state->agc_gain_ceiling;
        ret |= apply_gain(csi, state->agc_gain);
    }

    // EMA-smooth luminance to absorb scene noise.
    state->luminance_ema = (state->luminance_ema * (VBX943_L_EMA_DIV - 1) + luminance) / VBX943_L_EMA_DIV;
    luminance = state->luminance_ema;
    int diff = VBX943_L_TARGET - luminance;

    // Dead zone: ignore small luminance deviations.
    if (abs(diff) <= VBX943_L_AGC_DIFF_MIN) {
        return ret;
    }

    // Prefer raising exposure over gain when the scene is too dark, and
    // lowering gain before exposure when it's too bright.
    bool aec_exposure_in = ((diff > 0) && (state->aec_exposure < state->aec_exposure_ceiling)) ||
                           ((diff < 0) && (state->agc_gain <= VBX943_GAIN_MIN));

    int32_t damp = (abs(diff) >= VBX943_L_FAST_THRESH) ? VBX943_DAMP_FAST : VBX943_DAMP_SLOW;

    if (state->enable_aec && aec_exposure_in) {
        // Geometric convergence toward current_exp * L_TARGET / luminance.
        int32_t target = state->aec_exposure_ceiling;

        if (luminance > 0) {
            target = ((int64_t) state->aec_exposure * VBX943_L_TARGET) / luminance;
        }

        target = IM_CLAMP(target, exposure_min, state->aec_exposure_ceiling);
        int32_t step = (target - state->aec_exposure) / damp;

        if (!step && target != state->aec_exposure) {
            step = (target > state->aec_exposure) ? 1 : -1;
        }

        if (step) {
            state->aec_exposure += step;
            ret |= apply_exposure(csi, state->aec_exposure);
        }
    } else if (state->enable_agc) {
        // Geometric convergence toward current_gain * L_TARGET / luminance.
        int32_t target = state->agc_gain_ceiling;

        if (luminance > 0) {
            target = ((int64_t) state->agc_gain * VBX943_L_TARGET) / luminance;
        }

        target = IM_CLAMP(target, VBX943_GAIN_MIN, state->agc_gain_ceiling);
        int32_t step = (target - state->agc_gain) / damp;

        if (!step && target != state->agc_gain) {
            step = (target > state->agc_gain) ? 1 : -1;
        }

        if (step) {
            state->agc_gain += step;
            ret |= apply_gain(csi, state->agc_gain);
        }
    }

    return ret;
}

static int ioctl(omv_csi_t *csi, int request, void *arg) {
    vbx943_state_t *state = csi->priv;
    omv_csi_ioctl_arg_t *a = arg;
    int ret = 0;

    switch (request) {
        case OMV_CSI_IOCTL_UPDATE_AGC_AEC:
            ret = update_agc_aec(csi, a->ivalue);
            break;
        case OMV_CSI_IOCTL_SET_READOUT_WINDOW:
            // The window's position is relative to the center of the sensor.
            state->readout.x = a->window.x;
            state->readout.y = a->window.y;
            state->readout.w = (IM_CLAMP(a->window.w, 4, SENSOR_WIDTH) + 3) & ~3;
            state->readout.h = (IM_CLAMP(a->window.h, 4, SENSOR_HEIGHT) + 3) & ~3;
            ret = configure(csi, csi->pixformat, csi->framesize);
            break;
        case OMV_CSI_IOCTL_GET_READOUT_WINDOW:
            a->window = state->readout;
            break;
        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE:
            ret = set_frame_sync(csi, a->ivalue, a->ivalue ? false : state->strobe);
            break;
        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE:
            a->ivalue = state->triggered;
            break;
        case OMV_CSI_IOCTL_SET_STROBE_MODE:
            ret = set_frame_sync(csi, a->ivalue ? false : state->triggered, a->ivalue);
            break;
        case OMV_CSI_IOCTL_GET_STROBE_MODE:
            a->ivalue = state->strobe;
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

int vbx943_init(omv_csi_t *csi) {
    vbx943_state_t *state = &vbx943_state;

    csi->priv = state;
    state->csi = csi;

    state->ctx.shutdown_pin = vbx943_shutdown_pin;
    state->ctx.read8 = vbx943_read8;
    state->ctx.read16 = vbx943_read16;
    state->ctx.read32 = vbx943_read32;
    state->ctx.write8 = vbx943_write8;
    state->ctx.write16 = vbx943_write16;
    state->ctx.write32 = vbx943_write32;
    state->ctx.write_array = vbx943_write_array;
    state->ctx.delay = vbx943_delay;

    // OpenMV boards wire the sensor's clock and data lanes straight to
    // the host, with no lane or polarity swaps.
    state->config = (VD1943_Config_t) {
        .out_itf = {
            .data_rate_in_mps = VBX943_MIPI_BRATE * 1000000,
            .datalane_nb = 2,
            .logic_lane_mapping = { 0, 1, 2, 3 },
        },
    };

    for (int i = 0; i < VD1943_GPIO_NB; i++) {
        state->config.gpios[i].gpio_ctrl = VD1943_GPIO_IN;
    }

    // Boot the sensor in the monochrome mode, which both variants support,
    // to find out which one this is. Only the VB5943 reports no Bayer pattern.
    state->mono = true;
    reset_state(csi);

    if (boot(csi) != 0) {
        return -1;
    }

    state->mono = (state->ctx.bayer == VD1943_BAYER_NONE);
    csi->chip_id = state->mono ? VB5943_ID : VB1943_ID;
    reset_state(csi);

    #ifdef OMV_CSI_HW_CCM_ENABLE
    if (!state->mono) {
        csi->ccm = vbx943_ccm;
        csi->ccm_count = OMV_ARRAY_SIZE(vbx943_ccm);
    }
    #endif // OMV_CSI_HW_CCM_ENABLE

    // Initialize csi flags.
    csi->vsync_pol = 0;
    csi->hsync_pol = 0;
    csi->pixck_pol = 1;
    csi->mono_bpp = 1;
    csi->raw_output = 1;
    csi->cfa_mono = state->mono;
    csi->mipi_if = 1;
    csi->mipi_brate = VBX943_MIPI_BRATE;
    csi->halt_req = 1;

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

    // Override the WQXGA resolution with the sensor's native resolution.
    csi->resolution[OMV_CSI_FRAMESIZE_WQXGA][0] = SENSOR_WIDTH;
    csi->resolution[OMV_CSI_FRAMESIZE_WQXGA][1] = SENSOR_HEIGHT;

    return 0;
}
#endif // (OMV_VBX943_ENABLE == 1)
