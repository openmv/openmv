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
 * VL53L9CX driver.
 *
 * The VL53L9CX is a time-of-flight sensor with 54x42 zones. It's used as an
 * auxiliary sensor whose frames are read over I3C with vi3c, not through
 * the CSI interface. This is the OpenMV glue around ST's VL53L9 driver (see
 * drivers/vl53l9cx), which boots and configures the sensor.
 */
#include "board_config.h"
#if (OMV_VL53L9CX_ENABLE == 1)

#include <stdint.h>
#include <string.h>

#include "omv_common.h"
#include "omv_csi.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "omv_i3c.h"
#include "framebuffer.h"
#include "imlib.h"
#include "queue.h"
#include "umalloc.h"
#include "vi3c.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "vl53l9.h"
#include "vl53l9_platform.h"
#include "vl53l9cx.h"

#define VL53L9CX_POWER_DELAY_MS         (50)
#define VL53L9CX_STATE_TIMEOUT_MS       (100)
#define VL53L9CX_SNAPSHOT_RETRY         (1)
#define VL53L9CX_SNAPSHOT_TIMEOUT_MS    (1000)
#define VL53L9CX_FRAME_BUFFERS          (3)
// Writes up to this size are sent from the stack.
#define VL53L9CX_WRITE_BUF_SIZE         (32)

// Sensor states, see vl53l9_status_t.
#define VL53L9CX_FSM_STANDBY            (2)
#define VL53L9CX_FSM_STREAMING          (3)

// Bit 15 of each depth sample is a flag, not part of the distance.
#define VL53L9CX_DEPTH_MASK             (0x7FFF)

// Calibration data returned by vl53l9_get_calib_data(). The distance
// offsets of the 54x42 zones are 6-bit signed values in mm, stored column
// by column, four per three bytes, in three blocks.
#define VL53L9CX_CALIB_SIZE             (2332)
#define VL53L9CX_CALIB_GLOBAL           (0x120)
#define VL53L9CX_CALIB_W                (54)
#define VL53L9CX_CALIB_H                (42)

// Effective SPAD count for each DSS index, per context.
#define VL53L9CX_DSS_SPADS_SHORT        (0x0624)
#define VL53L9CX_DSS_SPADS_LONG         (0x06A4)
#define VL53L9CX_DSS_INDEXES            (16)
// Amplitude and ambient are read per SPAD, in 12.4 fixed point.
#define VL53L9CX_DSS_SCALE              (16)

// All planes, which are read by default.
#define VL53L9CX_PLANES_ALL             ((1 << OMV_CSI_VL53L9CX_PLANE_COUNT) - 1)

#define VL53L9CX_MIN_MM_DEFAULT         (0)
#define VL53L9CX_MAX_MM_DEFAULT         (4000)
#define VL53L9CX_EXPOSURE_MS_DEFAULT    (10)
#define VL53L9CX_EXPOSURE_MS_MIN        (1)
#define VL53L9CX_EXPOSURE_MS_MAX        (30)
#define VL53L9CX_FPS_DEFAULT            (30)
#define VL53L9CX_FPS_MAX                (100)

// Readout size and the rows cropped from it for each binning.
typedef struct vl53l9cx_binning {
    uint8_t binning;
    uint8_t w;
    uint8_t h;
    uint8_t crop_y;
    uint8_t crop_h;
} vl53l9cx_binning_t;

static const vl53l9cx_binning_t vl53l9cx_binnings[] = {
    { 2,  54, 42, 0, 42 },
    { 4,  24, 24, 2, 20 },
    { 6,  18, 14, 0, 14 },
    { 8,  12, 10, 0, 10 },
    { 12, 8,  8,  1, 6  },
    { 24, 4,  4,  0, 4  },
};

static const uint16_t vl53l9cx_calib_blocks[][2] = {
    { 0x124, 945 },
    { 0x55C, 315 },
    { 0x74C, 441 },
};

// The zone offsets and the offsets resampled for the current binning, in mm.
typedef struct vl53l9cx_calib {
    int16_t global;
    int8_t zones[VL53L9CX_CALIB_W * VL53L9CX_CALIB_H];
    int8_t offsets[VL53L9CX_CALIB_W * VL53L9CX_CALIB_H];
} vl53l9cx_calib_t;

typedef struct vl53l9cx_state {
    omv_csi_t *csi;     // Passed to ST's driver, which passes it to the callbacks.
    const vl53l9cx_binning_t *binning;
    vl53l9_context_t context;
    uint16_t exposure_ms;
    uint32_t framerate;
    uint32_t planes;
    uint16_t dss_spads[VL53L9CX_DSS_INDEXES];
    int32_t min_mm;
    int32_t max_mm;
    bool triggered;
    bool trigger_pending;
    bool booted;
    bool streaming;
    // Allocated while streaming, and freed on abort.
    framebuffer_t fb;
    vbuffer_t *frame;
    vl53l9cx_calib_t *calib;
} vl53l9cx_state_t;

static vl53l9cx_state_t vl53l9cx = {};

// Bus callbacks for ST's driver. Registers have 16-bit addresses and
// little-endian values.
int vl53l9_read(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size) {
    omv_csi_t *csi = ((vl53l9cx_state_t *) p_dev)->csi;
    uint8_t addr[] = { address >> 8, address };

    if (omv_i2c_write(csi->i2c, csi->slv_addr, addr, 2, OMV_I2C_XFER_NO_STOP) != 0 ||
        omv_i2c_read(csi->i2c, csi->slv_addr, p_values, size, OMV_I2C_XFER_NO_FLAGS) != 0) {
        return VL53L9_ERROR_PLATFORM;
    }
    return VL53L9_ERROR_NONE;
}

int vl53l9_read8(void *const p_dev, uint16_t address, uint8_t *p_value) {
    return vl53l9_read(p_dev, address, p_value, 1);
}

int vl53l9_read16(void *const p_dev, uint16_t address, uint16_t *p_value) {
    uint8_t buf[2];
    int ret = vl53l9_read(p_dev, address, buf, sizeof(buf));
    *p_value = buf[0] | (buf[1] << 8);
    return ret;
}

int vl53l9_read32(void *const p_dev, uint16_t address, uint32_t *p_value) {
    uint8_t buf[4];
    int ret = vl53l9_read(p_dev, address, buf, sizeof(buf));
    *p_value = buf[0] | (buf[1] << 8) | (buf[2] << 16) | ((uint32_t) buf[3] << 24);
    return ret;
}

// Frames are read with vi3c instead.
int vl53l9_read_async(void *const p_dev, uint16_t address, volatile uint8_t *p_values, uint32_t size) {
    return vl53l9_read(p_dev, address, (uint8_t *) p_values, size);
}

int vl53l9_write(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size) {
    omv_csi_t *csi = ((vl53l9cx_state_t *) p_dev)->csi;
    uint8_t stack_buf[VL53L9CX_WRITE_BUF_SIZE];
    uint8_t *buf = stack_buf;
    int ret = VL53L9_ERROR_NONE;

    // The address and data are sent as one write, so the firmware patch
    // is staged in a heap buffer.
    if (size + 2 > sizeof(stack_buf)) {
        buf = uma_malloc(size + 2, UMA_FAST | UMA_MAYBE);
        if (buf == NULL) {
            return VL53L9_ERROR_PLATFORM;
        }
    }

    buf[0] = address >> 8;
    buf[1] = address;
    memcpy(buf + 2, p_values, size);

    if (omv_i2c_write(csi->i2c, csi->slv_addr, buf, size + 2, OMV_I2C_XFER_NO_FLAGS) != 0) {
        ret = VL53L9_ERROR_PLATFORM;
    }

    if (buf != stack_buf) {
        uma_free(buf);
    }
    return ret;
}

int vl53l9_write8(void *const p_dev, uint16_t address, uint8_t value) {
    return vl53l9_write(p_dev, address, &value, 1);
}

int vl53l9_write16(void *const p_dev, uint16_t address, uint16_t value) {
    uint8_t buf[] = { value, value >> 8 };
    return vl53l9_write(p_dev, address, buf, sizeof(buf));
}

int vl53l9_write32(void *const p_dev, uint16_t address, uint32_t value) {
    uint8_t buf[] = { value, value >> 8, value >> 16, value >> 24 };
    return vl53l9_write(p_dev, address, buf, sizeof(buf));
}

int vl53l9_wait_ms(void *const p_dev, uint32_t delay_ms) {
    mp_hal_delay_ms(delay_ms);
    return VL53L9_ERROR_NONE;
}

int vl53l9_get_config_vddio(void *const p_dev, vl53l9_vddio_t *voltage) {
    *voltage = VDDIO_1V8;
    return VL53L9_ERROR_NONE;
}

int vl53l9_get_config_vdda(void *const p_dev, vl53l9_vdda_t *voltage) {
    *voltage = VDDA_2V8;
    return VL53L9_ERROR_NONE;
}

int vl53l9_get_config_ext_clock(void *const p_dev, uint32_t *ext_clock) {
    *ext_clock = omv_csi_get_clk_frequency(((vl53l9cx_state_t *) p_dev)->csi, false);
    return VL53L9_ERROR_NONE;
}

static int vl53l9cx_config(vl53l9cx_state_t *state);

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint8_t reg_data;
    if (vl53l9_read8(csi->priv, reg_addr, &reg_data) != VL53L9_ERROR_NONE) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return (vl53l9_write8(csi->priv, reg_addr, reg_data) != VL53L9_ERROR_NONE) ? -1 : 0;
}

static size_t plane_size(vl53l9cx_state_t *state) {
    return state->binning->w * state->binning->h;
}

// Planes are read from the depth up to the last one enabled.
static size_t plane_count(vl53l9cx_state_t *state) {
    return 32 - __builtin_clz(state->planes);
}

// Amplitude and ambient vary with the SPADs selected for each zone, which
// the DSS indexes read after the planes give, two per byte.
static size_t dss_size(vl53l9cx_state_t *state) {
    uint32_t mask = OMV_CSI_VL53L9CX_PLANE_AMPLITUDE | OMV_CSI_VL53L9CX_PLANE_AMBIENT;
    return (state->planes & mask) ? plane_size(state) / 2 : 0;
}

static uint8_t dss_index(vl53l9cx_state_t *state, size_t i) {
    const uint8_t *dss = state->frame->data + plane_size(state) * plane_count(state) * sizeof(uint16_t);
    i += state->binning->crop_y * state->binning->w;
    return (dss[i / 2] >> ((i & 1) * 4)) & 0xF;
}

// Returns a plane of the frame, starting at the first uncropped row.
static const uint16_t *plane(vl53l9cx_state_t *state, size_t index) {
    const uint16_t *data = (const uint16_t *) state->frame->data;
    return data + plane_size(state) * index + state->binning->crop_y * state->binning->w;
}

// Waits for an FSM state, releasing the frames the sensor holds meanwhile.
static int wait_state(vl53l9cx_state_t *state, uint8_t fsm) {
    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(1)) {
        vl53l9_status_t status;
        uint8_t ready = 0;

        if (vl53l9_get_status(state, &status) != 0) {
            return -1;
        } else if (status.fsm == fsm) {
            return 0;
        } else if ((mp_hal_ticks_ms() - tick_start) > VL53L9CX_STATE_TIMEOUT_MS) {
            return -1;
        }

        if (vl53l9_poll_frame(state, &ready) == 0 && ready) {
            vl53l9_get_frame(state, NULL, 0);
        }
    }
}

// Must be called in standby.
static int read_calibration(vl53l9cx_state_t *state) {
    vl53l9cx_calib_t *calib = state->calib;
    uint8_t *buf = uma_malloc(VL53L9CX_CALIB_SIZE, UMA_FAST | UMA_MAYBE);

    if (buf == NULL) {
        return -1;
    }

    if (vl53l9_get_calib_data(state, buf) != 0) {
        uma_free(buf);
        return -1;
    }

    calib->global = (int16_t) (buf[VL53L9CX_CALIB_GLOBAL] | (buf[VL53L9CX_CALIB_GLOBAL + 1] << 8));

    size_t n = 0;
    for (size_t i = 0; i < OMV_ARRAY_SIZE(vl53l9cx_calib_blocks); i++) {
        const uint8_t *block = buf + vl53l9cx_calib_blocks[i][0];

        for (size_t j = 0; j < vl53l9cx_calib_blocks[i][1]; j += 3) {
            uint32_t bits = block[j] | (block[j + 1] << 8) | (block[j + 2] << 16);

            for (size_t k = 0; k < 4; k++) {
                size_t x = n / VL53L9CX_CALIB_H;
                size_t y = n % VL53L9CX_CALIB_H;
                // Sign-extend the 6-bit value.
                int8_t offset = (int8_t) (((bits >> (k * 6)) & 0x3F) << 2) >> 2;
                calib->zones[y * VL53L9CX_CALIB_W + x] = offset;
                n++;
            }
        }
    }

    uma_free(buf);
    return 0;
}

// Maps a readout pixel's center to the zone grid, in 8.8 fixed point.
static int32_t calib_coord(int i, int size, int calib_size) {
    int32_t c = ((2 * i + 1) * calib_size * 256) / (2 * size) - 128;
    return IM_CLAMP(c, 0, (calib_size - 1) * 256);
}

// Resamples the zone offsets to the readout of the current binning.
static void update_offsets(vl53l9cx_state_t *state) {
    const vl53l9cx_binning_t *binning = state->binning;
    vl53l9cx_calib_t *calib = state->calib;

    for (int y = 0; y < binning->h; y++) {
        int32_t fy = calib_coord(y, binning->h, VL53L9CX_CALIB_H);
        int32_t wy = fy & 0xFF;
        const int8_t *row0 = &calib->zones[(fy >> 8) * VL53L9CX_CALIB_W];
        const int8_t *row1 = &calib->zones[IM_MIN((fy >> 8) + 1, VL53L9CX_CALIB_H - 1) * VL53L9CX_CALIB_W];

        for (int x = 0; x < binning->w; x++) {
            int32_t fx = calib_coord(x, binning->w, VL53L9CX_CALIB_W);
            int32_t wx = fx & 0xFF;
            int x0 = fx >> 8;
            int x1 = IM_MIN(x0 + 1, VL53L9CX_CALIB_W - 1);

            int32_t top = row0[x0] * 256 + (row0[x1] - row0[x0]) * wx;
            int32_t bottom = row1[x0] * 256 + (row1[x1] - row1[x0]) * wx;
            int32_t offset = top * 256 + (bottom - top) * wy;

            calib->offsets[y * binning->w + x] = (offset + 32768) >> 16;
        }
    }
}

// Applies the calibration offsets to the depth of the frame.
static void correct_frame(vl53l9cx_state_t *state) {
    const vl53l9cx_calib_t *calib = state->calib;
    uint16_t *depth = (uint16_t *) state->frame->data;

    for (size_t i = 0; i < plane_size(state); i++) {
        int32_t mm = depth[i] & VL53L9CX_DEPTH_MASK;
        if (mm) {
            mm = IM_CLAMP(mm + calib->offsets[i] + calib->global, 0, VL53L9CX_DEPTH_MASK);
            depth[i] = (depth[i] & ~VL53L9CX_DEPTH_MASK) | mm;
        }
    }
}

// Stops streaming and frees the frames. On soft reset, this runs through
// abort before the memory is reclaimed.
static int stop(vl53l9cx_state_t *state) {
    int ret = 0;

    if (state->streaming) {
        uint8_t ready = 0;
        vi3c_abort();

        // The sensor won't stop while a frame is waiting to be acknowledged.
        if (vl53l9_poll_frame(state, &ready) == 0 && ready) {
            vl53l9_get_frame(state, NULL, 0);
        }

        if (vl53l9_stop(state) != 0 || wait_state(state, VL53L9CX_FSM_STANDBY) != 0) {
            ret = -1;
        }

        state->streaming = false;
        state->trigger_pending = false;
    }

    if (state->fb.raw_base) {
        uma_free(state->fb.raw_base);
    }

    if (state->calib) {
        uma_free(state->calib);
    }

    framebuffer_init(&state->fb, NULL, 0, true, true);
    state->frame = NULL;
    state->calib = NULL;
    return ret;
}

// Allocates the frames and starts streaming. Must be called in standby.
static int start(vl53l9cx_state_t *state) {
    omv_csi_t *csi = state->csi;
    size_t frame_size = plane_size(state) * plane_count(state) * sizeof(uint16_t);

    framebuffer_init(&state->fb, NULL, 0, true, true);

    if (framebuffer_resize(&state->fb, VL53L9CX_FRAME_BUFFERS, frame_size + dss_size(state)) != 0 ||
        !(state->calib = uma_malloc(sizeof(vl53l9cx_calib_t), UMA_PERSIST | UMA_MAYBE)) ||
        read_calibration(state) != 0) {
        stop(state);
        return -1;
    }

    update_offsets(state);
    vi3c_init(csi->i2c, csi->slv_addr, frame_size, dss_size(state), &state->fb);

    if (vl53l9_start(state) != 0 || wait_state(state, VL53L9CX_FSM_STREAMING) != 0) {
        stop(state);
        return -1;
    }

    state->streaming = true;
    vi3c_restart();
    return 0;
}

// Must be called in standby, so with streaming stopped.
static int vl53l9cx_config(vl53l9cx_state_t *state) {
    vl53l9_hw_config_t hw_config;
    vl53l9_sync_mode_t sync = state->triggered ? VL53L9_SYNC_MANUAL : VL53L9_SYNC_AUTONOMOUS;

    if (vl53l9_get_hw_config(state, &hw_config) != 0) {
        return -1;
    }

    // Signal frames with in-band interrupts.
    hw_config.output_interface = VL53L9_OUTPUT_I3C;
    hw_config.signaling_mode = false;

    if (vl53l9_set_hw_config(state, hw_config) != 0 ||
        vl53l9_set_sync_mode(state, sync) != 0 ||
        vl53l9_set_power_mode(state, VL53L9_POWER_REGULAR) != 0 ||
        vl53l9_set_context(state, state->context) != 0 ||
        vl53l9_set_binning(state, state->context, state->binning->binning) != 0 ||
        vl53l9_set_exposure(state, state->context, state->exposure_ms) != 0) {
        return -1;
    }

    uint16_t addr = (state->context == VL53L9_CONTEXT_SHORT) ? VL53L9CX_DSS_SPADS_SHORT : VL53L9CX_DSS_SPADS_LONG;
    uint8_t buf[VL53L9CX_DSS_INDEXES * 2];

    if (vl53l9_read(state, addr, buf, sizeof(buf)) != 0) {
        return -1;
    }

    for (size_t i = 0; i < VL53L9CX_DSS_INDEXES; i++) {
        state->dss_spads[i] = buf[i * 2] | (buf[i * 2 + 1] << 8);
    }

    // The frame period only applies when free running.
    if (!state->triggered && vl53l9_set_frame_period(state, 1000000 / state->framerate) != 0) {
        return -1;
    }

    return 0;
}

static int reconfigure(vl53l9cx_state_t *state) {
    if (stop(state) != 0 || vl53l9cx_config(state) != 0) {
        return -1;
    }
    return 0;
}

// Boots the sensor from power-on. The power cycle and DAA disturb legacy
// targets on the bus, so this runs before they are configured, or only to
// recover a sensor that lost power.
static int boot(vl53l9cx_state_t *state, bool power_cycle) {
    omv_csi_t *csi = state->csi;
    uint8_t addrs[1];
    uint64_t pids[1];

    state->booted = false;

    if (power_cycle) {
        // The module routes the power-down pin to XSHUT only.
        #if defined(OMV_CSI_POWER_PIN)
        omv_gpio_write(OMV_CSI_POWER_PIN, csi->power_pol);
        mp_hal_delay_ms(VL53L9CX_POWER_DELAY_MS);
        omv_gpio_write(OMV_CSI_POWER_PIN, !csi->power_pol);
        mp_hal_delay_ms(VL53L9CX_POWER_DELAY_MS);
        #endif

        // Power cycling clears the dynamic address.
        if (omv_i3c_daa(csi->i2c, addrs, pids, 1) != 1 || (pids[0] >> 16) != VL53L9CX_ID) {
            return -1;
        }
        csi->slv_addr = addrs[0];
    }

    if (vl53l9_init(state) != 0) {
        return -1;
    }

    state->booted = true;
    return 0;
}

// Retries the status read before the sensor is power-cycled for it.
static bool is_booted(vl53l9cx_state_t *state) {
    for (size_t i = 0; state->booted && i < 2; i++) {
        vl53l9_status_t status;
        if (vl53l9_get_status(state, &status) == 0) {
            return status.fsm >= VL53L9CX_FSM_STANDBY;
        }
    }
    return false;
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    return ((pixformat != PIXFORMAT_GRAYSCALE) && (pixformat != PIXFORMAT_RGB565)) ? -1 : 0;
}

// The frame size selects the binning, so it must be one of its resolutions.
static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    vl53l9cx_state_t *state = csi->priv;
    uint16_t w = csi->resolution[framesize][csi->transpose ? 1 : 0];
    uint16_t h = csi->resolution[framesize][csi->transpose ? 0 : 1];

    if (framesize != OMV_CSI_FRAMESIZE_CUSTOM) {
        return -1;
    }

    for (size_t i = 0; i < OMV_ARRAY_SIZE(vl53l9cx_binnings); i++) {
        const vl53l9cx_binning_t *binning = &vl53l9cx_binnings[i];

        if (binning->w != w || binning->crop_h != h) {
            continue;
        }

        if (binning == state->binning) {
            return 0;
        }

        state->binning = binning;
        return reconfigure(state);
    }

    return -1;
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    vl53l9cx_state_t *state = csi->priv;

    if (framerate > VL53L9CX_FPS_MAX) {
        return -1;
    }

    state->framerate = framerate ? framerate : VL53L9CX_FPS_DEFAULT;
    return reconfigure(state);
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    vl53l9cx_state_t *state = csi->priv;

    if (enable) {
        return -1;
    }

    if (exposure_us >= 0) {
        state->exposure_ms = IM_CLAMP(exposure_us / 1000, VL53L9CX_EXPOSURE_MS_MIN, VL53L9CX_EXPOSURE_MS_MAX);
    }

    return reconfigure(state);
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    vl53l9cx_state_t *state = csi->priv;
    *exposure_us = state->exposure_ms * 1000;
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    return 0;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    return 0;
}

// Copies the enabled planes of the frame to an array of shape (h, w, 3).
// The planes that aren't enabled are left as they are.
static void read_planes(vl53l9cx_state_t *state, uint16_t *data) {
    size_t size = state->binning->w * state->binning->crop_h;
    const uint16_t *depth = plane(state, 0);

    if (state->planes & OMV_CSI_VL53L9CX_PLANE_DEPTH) {
        for (size_t j = 0; j < size; j++) {
            data[j * OMV_CSI_VL53L9CX_PLANE_COUNT] = depth[j] & VL53L9CX_DEPTH_MASK;
        }
    }

    for (size_t i = 1; i < OMV_CSI_VL53L9CX_PLANE_COUNT; i++) {
        if (!(state->planes & (1 << i))) {
            continue;
        }

        const uint16_t *src = plane(state, i);

        for (size_t j = 0; j < size; j++) {
            uint32_t spads = state->dss_spads[dss_index(state, j)];
            uint32_t value = spads ? (src[j] * VL53L9CX_DSS_SCALE) / spads : 0;
            data[j * OMV_CSI_VL53L9CX_PLANE_COUNT + i] = OMV_MIN(value, 0xFFFF);
        }
    }
}

static int ioctl(omv_csi_t *csi, int request, void *arg) {
    vl53l9cx_state_t *state = csi->priv;
    vl53l9cx_ioctl_arg_t *a = arg;
    int ret = 0;

    switch (request) {
        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE: {
            state->triggered = a->ivalue;
            ret = reconfigure(state);
            break;
        }
        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE: {
            a->ivalue = state->triggered;
            break;
        }
        case OMV_CSI_IOCTL_VL53L9CX_SET_MODE: {
            if (a->ivalue != OMV_CSI_VL53L9CX_MODE_PRECISION && a->ivalue != OMV_CSI_VL53L9CX_MODE_RANGE) {
                ret = -1;
                break;
            }
            state->context = (a->ivalue == OMV_CSI_VL53L9CX_MODE_RANGE) ? VL53L9_CONTEXT_LONG : VL53L9_CONTEXT_SHORT;
            ret = reconfigure(state);
            break;
        }
        case OMV_CSI_IOCTL_VL53L9CX_SET_RANGE: {
            if (a->range.min < 0 || a->range.max <= a->range.min) {
                ret = -1;
                break;
            }
            state->min_mm = a->range.min;
            state->max_mm = a->range.max;
            break;
        }
        case OMV_CSI_IOCTL_VL53L9CX_SET_PLANES: {
            if (a->ivalue == 0 || (a->ivalue & ~VL53L9CX_PLANES_ALL)) {
                ret = -1;
                break;
            }
            state->planes = a->ivalue;
            ret = reconfigure(state);
            break;
        }
        case OMV_CSI_IOCTL_VL53L9CX_READ_PLANES: {
            if (omv_csi_get_cropped(csi)) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }

            if (csi->transpose) {
                return OMV_CSI_ERROR_CAPTURE_FAILED;
            }

            // The planes are read at the sensor resolution, which must be the frame size.
            if (csi->resolution[csi->framesize][0] != state->binning->w ||
                csi->resolution[csi->framesize][1] != state->binning->crop_h) {
                return OMV_CSI_ERROR_INVALID_FRAMESIZE;
            }

            image_t image;
            if ((ret = omv_csi_snapshot(csi, &image, OMV_CSI_FLAG_NO_POST | OMV_CSI_FLAG_NO_UPDATE)) < 0) {
                break;
            }

            // Invalidate frame.
            csi->fb->pixfmt = PIXFORMAT_INVALID;
            read_planes(state, a->planes);
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

static int sleep(omv_csi_t *csi, int enable) {
    return enable ? stop(csi->priv) : 0;
}

static int match(omv_csi_t *csi, size_t id) {
    return id == VL53L9CX_ID;
}

static int reset(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;

    vi3c_deinit();
    stop(state);

    state->binning = &vl53l9cx_binnings[0];
    state->context = VL53L9_CONTEXT_SHORT;
    state->exposure_ms = VL53L9CX_EXPOSURE_MS_DEFAULT;
    state->framerate = VL53L9CX_FPS_DEFAULT;
    state->planes = VL53L9CX_PLANES_ALL;
    state->min_mm = VL53L9CX_MIN_MM_DEFAULT;
    state->max_mm = VL53L9CX_MAX_MM_DEFAULT;
    state->triggered = false;

    // Reading planes needs no pixel format, snapshots render to grayscale by default.
    csi->pixformat = PIXFORMAT_GRAYSCALE;

    // Boot again if the sensor lost power.
    if (!is_booted(state) && boot(state, true) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    if (vl53l9cx_config(state) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

static int _abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    // Stopping talks to the sensor, which can't be done from an IRQ.
    return in_irq ? 0 : stop(csi->priv);
}

static int config(omv_csi_t *csi, omv_csi_config_t config) {
    // A failure here would fail the main sensor too, so reset() is left
    // to report it.
    if (config == OMV_CSI_CONFIG_INIT) {
        reset(csi);
    }
    return 0;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    vl53l9cx_state_t *state = csi->priv;
    framebuffer_t *fb = csi->fb;
    size_t reset_retry = 0;

    // Release the frame of the last snapshot.
    if (state->frame) {
        framebuffer_release(&state->fb, FB_FLAG_USED);
        state->frame = NULL;
    }

    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ;) {
        // Restart first (if needed) before returning the frame or timing out.
        if (!state->streaming && start(state) != 0) {
            return OMV_CSI_ERROR_IO_ERROR;
        }

        if (!vi3c_active()) {
            vi3c_restart();
        }

        // In triggered mode, each snapshot starts a new frame.
        if (state->triggered && !state->trigger_pending) {
            while (framebuffer_release(&state->fb, FB_FLAG_USED)) {
            }
            if (vl53l9_trigger_frame(state) != 0) {
                return OMV_CSI_ERROR_IO_ERROR;
            }
            state->trigger_pending = true;
        }

        // Keep only the newest frame.
        while (queue_size(state->fb.used_queue) > 1) {
            framebuffer_release(&state->fb, FB_FLAG_USED);
        }

        if ((state->frame = framebuffer_acquire(&state->fb, FB_FLAG_USED | FB_FLAG_PEEK))) {
            state->trigger_pending = false;
            break;
        }

        if (flags & OMV_CSI_FLAG_NON_BLOCK) {
            return OMV_CSI_ERROR_WOULD_BLOCK;
        }

        // Restart the sensor on timeout, up to VL53L9CX_SNAPSHOT_RETRY times.
        mp_uint_t elapsed = mp_hal_ticks_ms() - tick_start;
        if (elapsed > VL53L9CX_SNAPSHOT_TIMEOUT_MS) {
            if (reset_retry++ == VL53L9CX_SNAPSHOT_RETRY) {
                stop(state);
                return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
            }

            stop(state);
            tick_start = mp_hal_ticks_ms();
            continue;
        }

        // Sleep until the next interrupt, which may be the frame's.
        mp_event_wait_ms(VL53L9CX_SNAPSHOT_TIMEOUT_MS - elapsed);
    }

    correct_frame(state);

    if (!framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK)) {
        return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
    }

    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;
    fb->pixfmt = csi->pixformat;
    framebuffer_release(fb, FB_FLAG_FREE);
    framebuffer_to_image(fb, image);
    return 0;
}

// Maps the distance over the range to 1-255, near is low like the tof module.
// Zones without a valid return are 0.
static void fill_image(omv_csi_t *csi, image_t *image) {
    vl53l9cx_state_t *state = csi->priv;
    const vl53l9cx_binning_t *binning = state->binning;
    const uint16_t *depth = plane(state, 0);
    const uint16_t *amplitude = NULL;
    int32_t range = IM_MAX(state->max_mm - state->min_mm, 1);

    if (plane_count(state) > 1) {
        amplitude = plane(state, 1);
    }

    for (int y = 0; y < binning->crop_h; y++) {
        int sy = csi->vflip ? (binning->crop_h - 1 - y) : y;
        for (int x = 0; x < binning->w; x++) {
            int sx = csi->hmirror ? (binning->w - 1 - x) : x;
            int32_t mm = depth[y * binning->w + x] & VL53L9CX_DEPTH_MASK;
            uint8_t value = 0;

            if (mm && (!amplitude || amplitude[y * binning->w + x])) {
                int32_t scaled = ((mm - state->min_mm) * 254) / range;
                value = 1 + IM_CLAMP(scaled, 0, 254);
            }

            if (csi->transpose) {
                IMAGE_PUT_GRAYSCALE_PIXEL(image, sy, sx, value);
            } else {
                IMAGE_PUT_GRAYSCALE_PIXEL(image, sx, sy, value);
            }
        }
    }
}

// Renders the depth of the frame, cropped to the window.
static int post_process(omv_csi_t *csi, image_t *image, uint32_t flags) {
    vl53l9cx_state_t *state = csi->priv;
    int x = csi->transpose ? csi->fb->y : csi->fb->x;
    int y = csi->transpose ? csi->fb->x : csi->fb->y;

    image_t temp = {
        .w = csi->transpose ? state->binning->crop_h : state->binning->w,
        .h = csi->transpose ? state->binning->w : state->binning->crop_h,
        .pixfmt = PIXFORMAT_GRAYSCALE,
        .data = uma_malloc(state->binning->w * state->binning->crop_h, UMA_CACHE),
    };

    fill_image(csi, &temp);

    imlib_draw_image(image, &temp, -x, -y, 1.0f, 1.0f, NULL, -1, 255,
                     (csi->pixformat == PIXFORMAT_RGB565) ? csi->color_palette : NULL, NULL,
                     0, NULL, NULL, NULL, NULL);

    uma_free(temp.data);
    return 0;
}

int vl53l9cx_init(omv_csi_t *csi) {
    vl53l9cx_state_t *state = &vl53l9cx;

    csi->priv = state;
    state->csi = csi;

    // Boot while the other sensors on the bus are still unconfigured. On
    // failure, reset() retries with a power cycle.
    boot(state, false);

    // Initialize csi ops.
    csi->reset = reset;
    csi->sleep = sleep;
    csi->config = config;
    csi->abort = _abort;
    csi->match = match;
    csi->snapshot = snapshot;
    csi->post_process = post_process;
    csi->read_reg = read_reg;
    csi->write_reg = write_reg;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_framerate = set_framerate;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->ioctl = ioctl;

    // Initialize csi flags.
    csi->auxiliary = 1;
    csi->mono_bpp = 1;
    // Frame sync belongs to the main sensor.
    csi->fsync_pin = NULL;

    return 0;
}
#endif // (OMV_VL53L9CX_ENABLE == 1)
