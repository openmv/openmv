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
 * auxiliary sensor whose frames are read over I3C with voi3c, not through
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
#include "voi3c.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "vl53l9.h"
#include "vl53l9_platform.h"

#define VL53L9CX_POWER_DELAY_MS         (50)
#define VL53L9CX_STATE_TIMEOUT_MS       (100)
#define VL53L9CX_SNAPSHOT_RETRY         (1)
#define VL53L9CX_SNAPSHOT_TIMEOUT_MS    (1000)
// Writes up to this size are sent from the stack.
#define VL53L9CX_WRITE_BUF_SIZE         (32)

// Sensor states, see vl53l9_status_t.
#define VL53L9CX_FSM_STANDBY            (2)
#define VL53L9CX_FSM_STREAMING          (3)
#define VL53L9CX_CMD_REG                (0x0400)
#define VL53L9CX_CMD_ACK_FRAME          (0x06)
#define VL53L9CX_CMD_DSS_UNMAP          (0x0B)
#define VL53L9CX_CMD_TIMEOUT_MS         (30)

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
// The planes the sensor outputs, in the order it outputs them.
#define VL53L9CX_PLANES_SENSOR          (OMV_CSI_TOF_PLANE_DEPTH | OMV_CSI_TOF_PLANE_SIGNAL | \
                                         OMV_CSI_TOF_PLANE_AMBIENT)
// The planes that need the SPADs of each zone.
#define VL53L9CX_PLANES_DSS             (OMV_CSI_TOF_PLANE_SIGNAL | OMV_CSI_TOF_PLANE_AMBIENT | \
                                         OMV_CSI_TOF_PLANE_SPADS)
// All planes, which are read by default.
#define VL53L9CX_PLANES_ALL             (VL53L9CX_PLANES_SENSOR | OMV_CSI_TOF_PLANE_SPADS)

#define VL53L9CX_MIN_MM_DEFAULT         (0)
#define VL53L9CX_MAX_MM_DEFAULT         (4000)
#define VL53L9CX_EXPOSURE_MS_MIN        (1)
#define VL53L9CX_EXPOSURE_MS_MAX        (30)
#define VL53L9CX_FPS_DEFAULT            (30)
#define VL53L9CX_FPS_MAX                (100)

// Readout size and the crop of it that frames show, for each binning, with
// the default exposure of each context. Larger zones collect more light and
// saturate at shorter exposures, reading surfaces at ~670 mm as far as 2.4 m.
typedef struct vl53l9cx_binning {
    uint8_t binning;
    uint8_t w;
    uint8_t h;
    uint8_t crop_x;
    uint8_t crop_y;
    uint8_t crop_w;
    uint8_t crop_h;
    uint8_t exposure_ms[2];
} vl53l9cx_binning_t;

static const vl53l9cx_binning_t vl53l9cx_binnings[] = {
    { 2,  54, 42, 0, 0, 54, 42, { 10, 10 } },
    { 4,  24, 24, 0, 2, 24, 20, { 10, 10 } },
    { 6,  18, 14, 0, 0, 18, 14, { 10, 10 } },
    { 8,  12, 10, 0, 0, 12, 10, { 7,  5  } },
    { 12, 8,  8,  0, 1, 8,  6,  { 3,  2  } },
    { 24, 4,  4,  0, 0, 4,  4,  { 1,  1  } },
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
    // The frame of the last snapshot, held in csi->fb until the next one.
    vbuffer_t *frame;
    // Allocated while streaming, and freed on abort.
    vl53l9cx_calib_t *calib;
} vl53l9cx_state_t;

// A region of the readout, see get_view().
typedef struct vl53l9cx_view {
    int32_t origin;
    int32_t x_stride;
    int32_t y_stride;
    int32_t w;
    int32_t h;
} vl53l9cx_view_t;

static vl53l9cx_state_t vl53l9cx = {};

// Bus callbacks for ST's driver. Registers have 16-bit addresses and
// little-endian values.
int vl53l9_read(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size) {
    omv_csi_t *csi = p_dev;
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

// Frames are read with voi3c instead.
int vl53l9_read_async(void *const p_dev, uint16_t address, volatile uint8_t *p_values, uint32_t size) {
    return vl53l9_read(p_dev, address, (uint8_t *) p_values, size);
}

int vl53l9_write(void *const p_dev, uint16_t address, uint8_t *p_values, uint32_t size) {
    omv_csi_t *csi = p_dev;
    uint8_t stack_buf[VL53L9CX_WRITE_BUF_SIZE];
    uint8_t *buf = stack_buf;
    int ret = VL53L9_ERROR_NONE;

    // The sensor takes the address and data in one write, which the I3C
    // controller sends from one buffer, so they're copied together. Only
    // the firmware patch doesn't fit on the stack.
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
    *ext_clock = omv_csi_get_clk_frequency(p_dev, false);
    return VL53L9_ERROR_NONE;
}

static int vl53l9cx_config(omv_csi_t *csi);

static int read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    uint8_t reg_data;
    if (vl53l9_read8(csi, reg_addr, &reg_data) != VL53L9_ERROR_NONE) {
        return -1;
    }
    return reg_data;
}

static int write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    return (vl53l9_write8(csi, reg_addr, reg_data) != VL53L9_ERROR_NONE) ? -1 : 0;
}

// The exposure set, or the binning's default if none is.
static uint16_t exposure_ms(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;

    return state->exposure_ms ? state->exposure_ms : state->binning->exposure_ms[state->context];
}

static size_t plane_size(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;

    return state->binning->w * state->binning->h;
}

// The sensor's planes are read from the depth, which frames are rendered
// from, up to the last one enabled.
static size_t plane_count(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    uint32_t mask = (state->planes & VL53L9CX_PLANES_SENSOR) | OMV_CSI_TOF_PLANE_DEPTH;
    return 32 - __builtin_clz(mask);
}

// Signal and ambient vary with the SPADs selected for each zone, which the
// DSS indexes read after the planes give, two per byte.
static size_t dss_size(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    return (state->planes & VL53L9CX_PLANES_DSS) ? plane_size(csi) / 2 : 0;
}

// Frames hold the planes, then the DSS indexes, and are captured into the
// framebuffers, which the core sizes for them.
static size_t get_framebuffer_size(omv_csi_t *csi) {
    return plane_size(csi) * plane_count(csi) * sizeof(uint16_t) + dss_size(csi);
}

// Returns the DSS index of a zone of the readout.
static uint8_t dss_index(omv_csi_t *csi, size_t i) {
    vl53l9cx_state_t *state = csi->priv;
    const uint8_t *dss = state->frame->data + plane_size(csi) * plane_count(csi) * sizeof(uint16_t);
    return (dss[i / 2] >> ((i & 1) * 4)) & 0xF;
}

static const uint16_t *plane(omv_csi_t *csi, size_t index) {
    vl53l9cx_state_t *state = csi->priv;
    return (const uint16_t *) state->frame->data + plane_size(csi) * index;
}

// Gets the view of the readout that a frame shows: the binning's crop,
// flipped, windowed, then transposed. Output pixel (x, y) is the readout
// zone origin + x * x_stride + y * y_stride, for the planes, the DSS
// indexes and the offsets alike.
static void get_view(omv_csi_t *csi, vl53l9cx_view_t *view) {
    vl53l9cx_state_t *state = csi->priv;
    const vl53l9cx_binning_t *binning = state->binning;
    int32_t stride = binning->w;
    int32_t col_stride = csi->hmirror ? -1 : 1;
    int32_t row_stride = csi->vflip ? -stride : stride;
    int32_t col = binning->crop_x + (csi->hmirror ? (binning->crop_w - 1 - csi->fb->x) : csi->fb->x);
    int32_t row = binning->crop_y + (csi->vflip ? (binning->crop_h - 1 - csi->fb->y) : csi->fb->y);

    view->origin = row * stride + col;
    view->x_stride = csi->transpose ? row_stride : col_stride;
    view->y_stride = csi->transpose ? col_stride : row_stride;
    view->w = csi->transpose ? csi->fb->v : csi->fb->u;
    view->h = csi->transpose ? csi->fb->u : csi->fb->v;
}

// Waits for an FSM state, releasing the frames the sensor holds meanwhile.
static int wait_state(omv_csi_t *csi, uint8_t fsm) {
    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        vl53l9_status_t status;
        uint8_t ready = 0;

        if (vl53l9_get_status(csi, &status) != 0) {
            return -1;
        } else if (status.fsm == fsm) {
            return 0;
        } else if ((mp_hal_ticks_ms() - tick_start) > VL53L9CX_STATE_TIMEOUT_MS) {
            return -1;
        }

        if (vl53l9_poll_frame(csi, &ready) == 0 && ready) {
            vl53l9_get_frame(csi, NULL, 0);
        }
    }
}

// Must be called in standby.
static int read_calibration(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    vl53l9cx_calib_t *calib = state->calib;
    uint8_t *buf = uma_malloc(VL53L9CX_CALIB_SIZE, UMA_FAST | UMA_MAYBE);

    if (buf == NULL) {
        return -1;
    }

    if (vl53l9_get_calib_data(csi, buf) != 0) {
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
static void update_offsets(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
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
static void correct_frame(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    const vl53l9cx_calib_t *calib = state->calib;
    uint16_t *depth = (uint16_t *) state->frame->data;

    for (size_t i = 0; i < plane_size(csi); i++) {
        int32_t mm = depth[i] & VL53L9CX_DEPTH_MASK;
        if (mm) {
            mm = IM_CLAMP(mm + calib->offsets[i] + calib->global, 0, VL53L9CX_DEPTH_MASK);
            depth[i] = (depth[i] & ~VL53L9CX_DEPTH_MASK) | mm;
        }
    }
}

// Sends a command and waits for the sensor to clear it, which it does even
// if the command fails.
static int command(omv_csi_t *csi, uint8_t cmd) {
    if (vl53l9_write8(csi, VL53L9CX_CMD_REG, cmd) != 0) {
        return -1;
    }

    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ;) {
        uint8_t value;

        if (vl53l9_read8(csi, VL53L9CX_CMD_REG, &value) != 0) {
            return -1;
        } else if (value == 0) {
            return 0;
        } else if ((mp_hal_ticks_ms() - tick_start) > VL53L9CX_CMD_TIMEOUT_MS) {
            return -1;
        }
    }
}

// Stops the sensor if it's streaming. The sensor won't stop while it holds a
// frame, which the frame ready flag doesn't show once the frame reader read
// it, or while the DSS indexes are mapped, so the frame reader may have left
// both if it was stopped mid-frame. Both are undone first, which the sensor
// rejects harmlessly if there's nothing to undo.
static int stop_streaming(omv_csi_t *csi) {
    vl53l9_status_t status;

    if (vl53l9_get_status(csi, &status) != 0) {
        return -1;
    } else if (status.fsm != VL53L9CX_FSM_STREAMING) {
        return 0;
    }

    command(csi, VL53L9CX_CMD_DSS_UNMAP);
    command(csi, VL53L9CX_CMD_ACK_FRAME);

    // A stop sent earlier takes effect once the frame is acknowledged.
    if (vl53l9_get_status(csi, &status) != 0) {
        return -1;
    } else if (status.fsm == VL53L9CX_FSM_STREAMING && vl53l9_stop(csi) != 0) {
        return -1;
    }

    return wait_state(csi, VL53L9CX_FSM_STANDBY);
}

// Stops streaming and frees the calibration. On soft reset, this runs through
// abort before the memory is reclaimed.
static int stop(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    int ret = 0;

    // Stop reading frames before the sensor is stopped.
    voi3c_deinit();

    // The sensor may be streaming even if the driver didn't start it, such
    // as after a start that failed, so its state is checked.
    if (state->booted && stop_streaming(csi) != 0) {
        ret = -1;
    }

    state->streaming = false;
    state->trigger_pending = false;

    if (state->calib) {
        uma_free(state->calib);
    }

    state->frame = NULL;
    state->calib = NULL;
    return ret;
}

// Starts streaming into the framebuffers. Must be called in standby.
static int start(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    size_t planes_size = plane_size(csi) * plane_count(csi) * sizeof(uint16_t);

    if (framebuffer_get_buffer_size(csi->fb) < get_framebuffer_size(csi) ||
        !(state->calib = uma_malloc(sizeof(vl53l9cx_calib_t), UMA_PERSIST | UMA_MAYBE)) ||
        read_calibration(csi) != 0) {
        stop(csi);
        return -1;
    }

    update_offsets(csi);
    voi3c_init(csi->i2c, csi->slv_addr, planes_size, dss_size(csi), csi->fb);

    if (vl53l9_start(csi) != 0 || wait_state(csi, VL53L9CX_FSM_STREAMING) != 0) {
        stop(csi);
        return -1;
    }

    state->streaming = true;
    voi3c_restart();
    return 0;
}

// Must be called in standby, so with streaming stopped.
static int vl53l9cx_config(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;
    vl53l9_hw_config_t hw_config;
    vl53l9_sync_mode_t sync = state->triggered ? VL53L9_SYNC_MANUAL : VL53L9_SYNC_AUTONOMOUS;

    if (vl53l9_get_hw_config(csi, &hw_config) != 0) {
        return -1;
    }

    // Signal frames with in-band interrupts.
    hw_config.output_interface = VL53L9_OUTPUT_I3C;
    hw_config.signaling_mode = false;

    if (vl53l9_set_hw_config(csi, hw_config) != 0 ||
        vl53l9_set_sync_mode(csi, sync) != 0 ||
        vl53l9_set_power_mode(csi, VL53L9_POWER_REGULAR) != 0 ||
        vl53l9_set_context(csi, state->context) != 0 ||
        vl53l9_set_binning(csi, state->context, state->binning->binning) != 0 ||
        vl53l9_set_exposure(csi, state->context, exposure_ms(csi)) != 0) {
        return -1;
    }

    uint16_t addr = (state->context == VL53L9_CONTEXT_SHORT) ? VL53L9CX_DSS_SPADS_SHORT : VL53L9CX_DSS_SPADS_LONG;
    uint8_t buf[VL53L9CX_DSS_INDEXES * 2];

    if (vl53l9_read(csi, addr, buf, sizeof(buf)) != 0) {
        return -1;
    }

    for (size_t i = 0; i < VL53L9CX_DSS_INDEXES; i++) {
        state->dss_spads[i] = buf[i * 2] | (buf[i * 2 + 1] << 8);
    }

    // The frame period only applies when free running.
    if (!state->triggered && vl53l9_set_frame_period(csi, 1000000 / state->framerate) != 0) {
        return -1;
    }

    return 0;
}

// Drops the frames captured with the old settings.
static int reconfigure(omv_csi_t *csi) {
    if (stop(csi) != 0 || vl53l9cx_config(csi) != 0) {
        return -1;
    }

    if (csi->fb) {
        framebuffer_flush(csi->fb);
    }
    return 0;
}

// Boots the sensor, uploading its firmware patch. Power cycling it first
// reassigns its address with a DAA, which disturbs legacy targets on the
// bus, so that's only done to recover a sensor that lost power.
static int boot(omv_csi_t *csi, bool power_cycle) {
    vl53l9cx_state_t *state = csi->priv;

    state->booted = false;

    if (power_cycle) {
        // The module routes the power-down pin to XSHUT only.
        #if defined(OMV_CSI_POWER_PIN)
        omv_gpio_write(OMV_CSI_POWER_PIN, csi->power_pol);
        mp_hal_delay_ms(VL53L9CX_POWER_DELAY_MS);
        omv_gpio_write(OMV_CSI_POWER_PIN, !csi->power_pol);
        mp_hal_delay_ms(VL53L9CX_POWER_DELAY_MS);
        #endif

        // Power cycling clears the dynamic address. The DAA readdresses all
        // the I3C targets on the bus, so every csi gets its new address.
        uint8_t addrs[OMV_CSI_MAX_DEVICES];
        uint64_t pids[OMV_CSI_MAX_DEVICES];
        int count = OMV_MIN(omv_i3c_daa(csi->i2c, addrs, pids, OMV_CSI_MAX_DEVICES), OMV_CSI_MAX_DEVICES);
        bool found = false;

        for (int i = 0; i < count; i++) {
            omv_csi_t *dev = omv_csi_get(pids[i] >> 16);

            if (dev) {
                dev->slv_addr = addrs[i];
                found |= (dev == csi);
            }
        }

        if (!found) {
            return -1;
        }
    }

    if (vl53l9_init(csi) != 0) {
        return -1;
    }

    state->booted = true;
    return 0;
}

// Retries the status read before the sensor is power-cycled for it.
static bool is_booted(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;

    for (size_t i = 0; state->booted && i < 2; i++) {
        vl53l9_status_t status;
        if (vl53l9_get_status(csi, &status) == 0) {
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

        if (binning->crop_w != w || binning->crop_h != h) {
            continue;
        }

        if (binning == state->binning) {
            return 0;
        }

        state->binning = binning;
        return reconfigure(csi);
    }

    return -1;
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    vl53l9cx_state_t *state = csi->priv;

    if (framerate > VL53L9CX_FPS_MAX) {
        return -1;
    }

    state->framerate = framerate ? framerate : VL53L9CX_FPS_DEFAULT;
    return reconfigure(csi);
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    vl53l9cx_state_t *state = csi->priv;

    if (enable) {
        return -1;
    }

    if (exposure_us >= 0) {
        state->exposure_ms = IM_CLAMP(exposure_us / 1000, VL53L9CX_EXPOSURE_MS_MIN, VL53L9CX_EXPOSURE_MS_MAX);
    }

    return reconfigure(csi);
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    *exposure_us = exposure_ms(csi) * 1000;
    return 0;
}

// The sensor can't flip, so frames are flipped when they're rendered or their
// planes are read, using the csi's hmirror and vflip.
static int set_hmirror(omv_csi_t *csi, int enable) {
    return 0;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    return 0;
}

// Copies the enabled planes of the frame, in bit order, to a float array of
// shape (h, w, enabled planes), laid out like the image.
static void read_planes(omv_csi_t *csi, float *data) {
    vl53l9cx_state_t *state = csi->priv;
    // The LUT counts the SPADs of a 2x2 binned zone, so it's scaled to the
    // zones of the binning.
    uint32_t zones = (state->binning->binning / 2) * (state->binning->binning / 2);
    size_t count = __builtin_popcount(state->planes);
    size_t index = 0;
    vl53l9cx_view_t view;

    get_view(csi, &view);

    for (uint32_t planes = state->planes; planes; planes &= planes - 1, index++) {
        uint32_t id = planes & -planes;
        // The sensor's planes are in bit order too. The SPADs aren't one.
        const uint16_t *src = (id & VL53L9CX_PLANES_SENSOR) ? plane(csi, __builtin_ctz(id)) : NULL;

        for (int32_t y = 0; y < view.h; y++) {
            for (int32_t x = 0; x < view.w; x++) {
                size_t j = view.origin + x * view.x_stride + y * view.y_stride;
                float value;

                if (id == OMV_CSI_TOF_PLANE_DEPTH) {
                    value = src[j] & VL53L9CX_DEPTH_MASK;
                } else {
                    uint32_t spads = state->dss_spads[dss_index(csi, j)] * zones;

                    if (id == OMV_CSI_TOF_PLANE_SPADS) {
                        value = spads;
                    } else {
                        value = spads ? (float) src[j] / spads : 0.0f;
                    }
                }

                data[(y * view.w + x) * count + index] = value;
            }
        }
    }
}

static int ioctl(omv_csi_t *csi, int request, void *arg) {
    vl53l9cx_state_t *state = csi->priv;
    omv_csi_tof_ioctl_arg_t *a = arg;
    int ret = 0;

    switch (request) {
        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE: {
            state->triggered = a->ivalue;
            ret = reconfigure(csi);
            break;
        }
        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE: {
            a->ivalue = state->triggered;
            break;
        }
        case OMV_CSI_IOCTL_TOF_SET_MODE: {
            if (a->ivalue != OMV_CSI_TOF_MODE_PRECISION && a->ivalue != OMV_CSI_TOF_MODE_RANGE) {
                ret = -1;
                break;
            }
            state->context = (a->ivalue == OMV_CSI_TOF_MODE_RANGE) ? VL53L9_CONTEXT_LONG : VL53L9_CONTEXT_SHORT;
            ret = reconfigure(csi);
            break;
        }
        case OMV_CSI_IOCTL_TOF_GET_MODE: {
            a->ivalue = (state->context == VL53L9_CONTEXT_LONG) ? OMV_CSI_TOF_MODE_RANGE : OMV_CSI_TOF_MODE_PRECISION;
            break;
        }
        case OMV_CSI_IOCTL_TOF_SET_RANGE: {
            if (a->range.min < 0 || a->range.max <= a->range.min) {
                ret = -1;
                break;
            }
            state->min_mm = a->range.min;
            state->max_mm = a->range.max;
            break;
        }
        case OMV_CSI_IOCTL_TOF_GET_RANGE: {
            a->range.min = state->min_mm;
            a->range.max = state->max_mm;
            break;
        }
        case OMV_CSI_IOCTL_TOF_GET_PLANES: {
            a->ivalue = VL53L9CX_PLANES_ALL;
            break;
        }
        case OMV_CSI_IOCTL_TOF_GET_ENABLED_PLANES: {
            a->ivalue = state->planes;
            break;
        }
        case OMV_CSI_IOCTL_TOF_SET_PLANES: {
            if (a->ivalue == 0 || (a->ivalue & ~VL53L9CX_PLANES_ALL)) {
                ret = -1;
                break;
            }
            state->planes = a->ivalue;

            // The frames change size with the planes.
            if ((ret = reconfigure(csi)) == 0 && csi->framesize != OMV_CSI_FRAMESIZE_INVALID) {
                ret = omv_csi_set_framebuffers(csi, -1);
            }
            break;
        }
        case OMV_CSI_IOCTL_TOF_READ_PLANES: {
            image_t image;
            if ((ret = omv_csi_snapshot(csi, &image, OMV_CSI_FLAG_NO_POST | OMV_CSI_FLAG_NO_UPDATE)) < 0) {
                break;
            }

            // The frame isn't rendered, so keep the IDE from showing it when
            // the next snapshot releases it.
            csi->fb->pixfmt = PIXFORMAT_INVALID;
            read_planes(csi, a->planes);
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }

    return ret;
}

// A frame acknowledged while none is held latches this error, after which
// the sensor streams no frames until it's power cycled.
static bool sof_error(omv_csi_t *csi) {
    vl53l9_status_t status;

    return vl53l9_get_status(csi, &status) != 0 || status.error.sof_outside_blanking;
}

static int sleep(omv_csi_t *csi, int enable) {
    return enable ? stop(csi) : 0;
}

static int reset(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;

    stop(csi);

    // Clear the state, but keep whether the sensor is booted, which
    // decides if it needs a power cycle below.
    bool booted = state->booted;
    memset(state, 0, sizeof(vl53l9cx_state_t));
    state->booted = booted;

    state->binning = &vl53l9cx_binnings[0];
    state->context = VL53L9_CONTEXT_SHORT;
    state->framerate = VL53L9CX_FPS_DEFAULT;
    state->planes = VL53L9CX_PLANES_ALL;
    state->min_mm = VL53L9CX_MIN_MM_DEFAULT;
    state->max_mm = VL53L9CX_MAX_MM_DEFAULT;

    // Reading planes needs no pixel format, snapshots render to grayscale by default.
    csi->pixformat = PIXFORMAT_GRAYSCALE;

    // Boot if needed, power cycling the sensor if it lost power and so
    // its address.
    if (!is_booted(csi) && boot(csi, false) != 0 && boot(csi, true) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // The sensor may still be streaming if it couldn't be stopped before,
    // or have latched an error, in which case it's power cycled.
    if ((stop_streaming(csi) != 0 || sof_error(csi)) && boot(csi, true) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    if (vl53l9cx_config(csi) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

static int _abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    // Stopping talks to the sensor, which can't be done from an IRQ.
    return in_irq ? 0 : stop(csi);
}

static int config(omv_csi_t *csi, omv_csi_config_t config) {
    // A failure here would fail the main sensor too, so reset() is left
    // to report it.
    if (config == OMV_CSI_CONFIG_INIT) {
        reset(csi);
    }
    return 0;
}

// Renders the depth of the frame. The distance over the range maps to 1-255,
// near is low like the tof module, and zones without a valid return are 0.
static void fill_image(omv_csi_t *csi, image_t *image) {
    vl53l9cx_state_t *state = csi->priv;
    const uint16_t *depth = plane(csi, 0);
    const uint16_t *signal = (plane_count(csi) > 1) ? plane(csi, 1) : NULL;
    int32_t range = IM_MAX(state->max_mm - state->min_mm, 1);
    const uint16_t *palette = csi->color_palette ? csi->color_palette : depth_table;
    vl53l9cx_view_t view;

    get_view(csi, &view);

    for (int32_t y = 0; y < view.h; y++) {
        for (int32_t x = 0; x < view.w; x++) {
            size_t j = view.origin + x * view.x_stride + y * view.y_stride;
            int32_t mm = depth[j] & VL53L9CX_DEPTH_MASK;
            uint8_t value = 0;

            if (mm && (!signal || signal[j])) {
                int32_t scaled = ((mm - state->min_mm) * 254) / range;
                value = 1 + IM_CLAMP(scaled, 0, 254);
            }

            if (image->pixfmt == PIXFORMAT_RGB565) {
                IMAGE_PUT_RGB565_PIXEL(image, x, y, palette[value]);
            } else {
                IMAGE_PUT_GRAYSCALE_PIXEL(image, x, y, value);
            }
        }
    }
}

// In triggered mode, each snapshot starts a new frame.
static int trigger_frame(omv_csi_t *csi) {
    vl53l9cx_state_t *state = csi->priv;

    if (state->triggered && !state->trigger_pending) {
        if (vl53l9_trigger_frame(csi) != 0) {
            return -1;
        }

        state->trigger_pending = true;
    }

    return 0;
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    vl53l9cx_state_t *state = csi->priv;
    size_t reset_retry = 0;
    framebuffer_t *fb = csi->fb;

    // The core released the frame of the last snapshot.
    state->frame = NULL;

    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        // Restart first (if needed) before returning the frame or timing out.
        if (!state->streaming && start(csi) != 0) {
            return OMV_CSI_ERROR_IO_ERROR;
        }

        if (!voi3c_active()) {
            voi3c_restart();
        }

        if (trigger_frame(csi) != 0) {
            return OMV_CSI_ERROR_IO_ERROR;
        }

        if ((state->frame = framebuffer_acquire(fb, FB_FLAG_USED | FB_FLAG_PEEK))) {
            state->trigger_pending = false;
            break;
        }

        if (flags & OMV_CSI_FLAG_NON_BLOCK) {
            return OMV_CSI_ERROR_WOULD_BLOCK;
        }

        // Restart the sensor on timeout, up to VL53L9CX_SNAPSHOT_RETRY times.
        if ((mp_hal_ticks_ms() - tick_start) > VL53L9CX_SNAPSHOT_TIMEOUT_MS) {
            stop(csi);

            if (reset_retry++ == VL53L9CX_SNAPSHOT_RETRY) {
                return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
            }

            tick_start = mp_hal_ticks_ms();
        }
    }

    correct_frame(csi);

    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;
    fb->pixfmt = csi->pixformat;
    framebuffer_to_image(fb, image);

    // Render the frame in place, through a temporary image since the image
    // overwrites the frame. Reading the planes skips rendering.
    if (!(flags & OMV_CSI_FLAG_NO_POST)) {
        image_t temp = {
            .w = image->w,
            .h = image->h,
            .pixfmt = image->pixfmt,
        };

        size_t size = image_size(&temp);
        temp.data = uma_malloc(size, UMA_CACHE);
        fill_image(csi, &temp);
        memcpy(image->data, temp.data, size);
        uma_free(temp.data);
    }

    return 0;
}

int vl53l9cx_init(omv_csi_t *csi) {
    csi->priv = &vl53l9cx;

    // Initialize csi ops.
    csi->reset = reset;
    csi->sleep = sleep;
    csi->config = config;
    csi->abort = _abort;
    csi->snapshot = snapshot;
    csi->get_framebuffer_size = get_framebuffer_size;
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
