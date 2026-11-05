/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * VL53L9CX driver.
 */
#include "board_config.h"
#if (OMV_VL53L9CX_ENABLE == 1)

#include <string.h>
#include "py/mphal.h"
#include "py/runtime.h"

#include "omv_common.h"
#include "omv_csi.h"
#include "omv_gpio.h"
#include "omv_i3c.h"
#include "framebuffer.h"
#include "imlib.h"
#include "fmath.h"
#include "umalloc.h"
#include "vi3c.h"

#include "platform.h"
#include "vl53l9.h"
#include "vl53l9cx.h"

#define VL53L9CX_POWER_DELAY_MS     (50)
#define VL53L9CX_SNAPSHOT_TIMEOUT   (1000)
#define VL53L9CX_FRAME_BUFFERS      (3)

// Frame readout, see vl53l9_get_frame().
#define VL53L9CX_REG_FRAME          (0x1800)
#define VL53L9CX_REG_STATUS         (0x0028)
#define VL53L9CX_REG_COMMAND        (0x0400)
#define VL53L9CX_CMD_ACK_FRAME      (0x06)
#define VL53L9CX_FSM_STANDBY        (2)
#define VL53L9CX_FSM_STREAMING      (3)
#define VL53L9CX_STATE_TIMEOUT      (100)

// Bit 15 of each depth sample is a flag, not part of the distance.
#define VL53L9CX_DEPTH_MASK         (0x7FFF)

// Calibration data returned by vl53l9_get_calib_data(). The distance
// offsets of the 54x42 zones are 6-bit signed values in mm, stored column
// by column, four per three bytes, in three blocks.
#define VL53L9CX_CALIB_SIZE         (2332)
#define VL53L9CX_CALIB_GLOBAL       (0x120)
#define VL53L9CX_CALIB_W            (54)
#define VL53L9CX_CALIB_H            (42)

#define VL53L9CX_MIN_MM_DEFAULT     (0)
#define VL53L9CX_MAX_MM_DEFAULT     (4000)
#define VL53L9CX_EXPOSURE_DEFAULT   (10)
#define VL53L9CX_EXPOSURE_MIN       (1)
#define VL53L9CX_EXPOSURE_MAX       (30)
#define VL53L9CX_FPS_DEFAULT        (30)
#define VL53L9CX_FPS_MIN            (1)
#define VL53L9CX_FPS_MAX            (100)

// Readout size and the rows cropped from it for each binning.
typedef struct {
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

typedef struct {
    vl53l9_omv_dev_t dev;
    const vl53l9cx_binning_t *binning;
    vl53l9_context_t context;
    uint16_t exposure_ms;
    uint32_t fps;
    uint32_t planes;
    int32_t min_mm;
    int32_t max_mm;
    bool triggered;
    bool trigger_pending;
    bool booted;
    bool streaming;
    vbuffer_t *frame;
    // Offsets in mm, per zone and resampled for the current binning, which
    // are allocated once and kept across soft-resets.
    int8_t *calib;
    int8_t *offsets;
    int16_t calib_global;
} vl53l9cx_state_t;

static vl53l9cx_state_t vl53l9cx;

static size_t vl53l9cx_plane_size(void) {
    return vl53l9cx.binning->w * vl53l9cx.binning->h;
}

// Returns a plane of the held frame, starting at the first uncropped row.
static const uint16_t *vl53l9cx_plane(size_t index) {
    const uint16_t *data = (const uint16_t *) vl53l9cx.frame->data;
    return data + vl53l9cx_plane_size() * index + vl53l9cx.binning->crop_y * vl53l9cx.binning->w;
}

// Waits for an FSM state, releasing the frames the sensor holds meanwhile.
static int vl53l9cx_wait_state(uint8_t state) {
    void *dev = &vl53l9cx.dev;

    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ; mp_hal_delay_ms(1)) {
        vl53l9_status_t status;
        uint8_t ready = 0;

        if (vl53l9_get_status(dev, &status) != 0) {
            return -1;
        } else if (status.fsm == state) {
            return 0;
        } else if ((mp_hal_ticks_ms() - tick_start) > VL53L9CX_STATE_TIMEOUT) {
            return -1;
        }

        if (vl53l9_poll_frame(dev, &ready) == 0 && ready) {
            vl53l9_get_frame(dev, NULL, 0);
        }
    }
}

static int vl53l9cx_stop(void) {
    int ret = 0;
    if (vl53l9cx.streaming) {
        uint8_t ready = 0;
        vi3c_stop();
        // The sensor won't stop while a frame is waiting to be acknowledged.
        if (vl53l9_poll_frame(&vl53l9cx.dev, &ready) == 0 && ready) {
            vl53l9_get_frame(&vl53l9cx.dev, NULL, 0);
        }
        if (vl53l9_stop(&vl53l9cx.dev) != 0 || vl53l9cx_wait_state(VL53L9CX_FSM_STANDBY) != 0) {
            ret = -1;
        }
        vl53l9cx.streaming = false;
        vl53l9cx.trigger_pending = false;
        vl53l9cx.frame = NULL;
    }
    return ret;
}

static int vl53l9cx_start(void) {
    if (vl53l9cx.streaming) {
        return 0;
    }

    if (vl53l9_start(&vl53l9cx.dev) != 0 || vl53l9cx_wait_state(VL53L9CX_FSM_STREAMING) != 0) {
        return -1;
    }

    vl53l9cx.streaming = true;
    return vi3c_start();
}

// Must be called in standby.
static int vl53l9cx_read_calibration(void) {
    size_t zones = VL53L9CX_CALIB_W * VL53L9CX_CALIB_H;

    if (vl53l9cx.calib == NULL) {
        vl53l9cx.calib = uma_malloc(zones * 2, UMA_PERSIST | UMA_MAYBE);
        if (vl53l9cx.calib == NULL) {
            return -1;
        }
        vl53l9cx.offsets = vl53l9cx.calib + zones;
    }

    uint8_t *buf = uma_malloc(VL53L9CX_CALIB_SIZE, UMA_FAST | UMA_MAYBE);

    if (buf == NULL) {
        return -1;
    }

    if (vl53l9_get_calib_data(&vl53l9cx.dev, buf) != 0) {
        uma_free(buf);
        return -1;
    }

    vl53l9cx.calib_global = (int16_t) (buf[VL53L9CX_CALIB_GLOBAL] | (buf[VL53L9CX_CALIB_GLOBAL + 1] << 8));

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
                vl53l9cx.calib[y * VL53L9CX_CALIB_W + x] = offset;
                n++;
            }
        }
    }

    uma_free(buf);
    return 0;
}

// Resamples the zone offsets to the readout of the current binning.
static void vl53l9cx_update_offsets(void) {
    const vl53l9cx_binning_t *binning = vl53l9cx.binning;
    float sx = (float) VL53L9CX_CALIB_W / binning->w;
    float sy = (float) VL53L9CX_CALIB_H / binning->h;

    for (int y = 0; y < binning->h; y++) {
        float fy = IM_CLAMP((y + 0.5f) * sy - 0.5f, 0.0f, VL53L9CX_CALIB_H - 1.0f);
        int y0 = fy;
        int y1 = IM_MIN(y0 + 1, VL53L9CX_CALIB_H - 1);
        float wy = fy - y0;

        for (int x = 0; x < binning->w; x++) {
            float fx = IM_CLAMP((x + 0.5f) * sx - 0.5f, 0.0f, VL53L9CX_CALIB_W - 1.0f);
            int x0 = fx;
            int x1 = IM_MIN(x0 + 1, VL53L9CX_CALIB_W - 1);
            float wx = fx - x0;

            const int8_t *row0 = &vl53l9cx.calib[y0 * VL53L9CX_CALIB_W];
            const int8_t *row1 = &vl53l9cx.calib[y1 * VL53L9CX_CALIB_W];
            float top = row0[x0] + (row0[x1] - row0[x0]) * wx;
            float bottom = row1[x0] + (row1[x1] - row1[x0]) * wx;
            float offset = top + (bottom - top) * wy;

            vl53l9cx.offsets[y * binning->w + x] = fast_roundf(offset);
        }
    }
}

// Applies the calibration offsets to the depth of the held frame.
static void vl53l9cx_correct_frame(void) {
    uint16_t *depth = (uint16_t *) vl53l9cx.frame->data;

    for (size_t i = 0; i < vl53l9cx_plane_size(); i++) {
        int32_t mm = depth[i] & VL53L9CX_DEPTH_MASK;
        if (mm) {
            mm = IM_CLAMP(mm + vl53l9cx.offsets[i] + vl53l9cx.calib_global, 0, VL53L9CX_DEPTH_MASK);
            depth[i] = (depth[i] & ~VL53L9CX_DEPTH_MASK) | mm;
        }
    }
}

// Must be called in standby, so with streaming stopped.
static int vl53l9cx_configure(void) {
    void *dev = &vl53l9cx.dev;
    vl53l9_hw_config_t hw_config;
    vl53l9_sync_mode_t sync = vl53l9cx.triggered ? VL53L9_SYNC_MANUAL : VL53L9_SYNC_AUTONOMOUS;

    if (vl53l9_get_hw_config(dev, &hw_config) != 0) {
        return -1;
    }

    // Signal frames on the interrupt pad, driven push-pull.
    hw_config.output_interface = VL53L9_OUTPUT_I3C;
    hw_config.signaling_mode = true;
    hw_config.interrupt_pad_mode = false;

    if (vl53l9_set_hw_config(dev, hw_config) != 0 ||
        vl53l9_set_sync_mode(dev, sync) != 0 ||
        vl53l9_set_power_mode(dev, VL53L9_POWER_REGULAR) != 0 ||
        vl53l9_set_context(dev, vl53l9cx.context) != 0 ||
        vl53l9_set_binning(dev, vl53l9cx.context, vl53l9cx.binning->binning) != 0 ||
        vl53l9_set_exposure(dev, vl53l9cx.context, vl53l9cx.exposure_ms) != 0) {
        return -1;
    }

    vl53l9cx_update_offsets();

    // The frame period only applies when free running.
    if (!vl53l9cx.triggered && vl53l9_set_frame_period(dev, 1000000 / vl53l9cx.fps) != 0) {
        return -1;
    }

    vi3c_config_t config = {
        .bus = vl53l9cx.dev.bus,
        .addr = vl53l9cx.dev.addr,
        .irq_pin = OMV_VL53L9CX_INT_PIN,
        .segments = {
            { VL53L9CX_REG_FRAME, vl53l9cx_plane_size() * vl53l9cx.planes * sizeof(uint16_t) },
            { VL53L9CX_REG_STATUS, VL53L9_STATUS_SIZE },
        },
        .n_segments = 2,
        .cmd_reg = VL53L9CX_REG_COMMAND,
        .ack_cmd = VL53L9CX_CMD_ACK_FRAME,
        .n_buffers = VL53L9CX_FRAME_BUFFERS,
    };
    return vi3c_init(&config);
}

static int vl53l9cx_reconfigure(void) {
    if (vl53l9cx_stop() != 0 || vl53l9cx_configure() != 0) {
        return -1;
    }
    return 0;
}

// Boots the sensor from power-on. The power cycle and DAA disturb legacy
// targets on the bus, so this runs before they are configured, or only to
// recover a sensor that lost power.
static int vl53l9cx_boot(omv_csi_t *csi, bool power_cycle) {
    uint8_t addrs[1];
    uint64_t pids[1];

    vl53l9cx.booted = false;

    if (power_cycle) {
        // The module routes the power-down pin to XSHUT only.
        #if defined(OMV_CSI_POWER_PIN)
        omv_gpio_write(OMV_CSI_POWER_PIN, csi->power_pol);
        mp_hal_delay_ms(VL53L9CX_POWER_DELAY_MS);
        omv_gpio_write(OMV_CSI_POWER_PIN, !csi->power_pol);
        mp_hal_delay_ms(VL53L9CX_POWER_DELAY_MS);
        #endif

        // Power cycling clears the dynamic address.
        if (omv_i3c_daa(csi->i2c, addrs, pids, 1) != 1 || (pids[0] >> 16) != VL53L9CX_PID) {
            return -1;
        }
        csi->slv_addr = addrs[0];
    }

    vl53l9cx.dev.bus = csi->i2c;
    vl53l9cx.dev.addr = csi->slv_addr;
    vl53l9cx.dev.ext_clock = omv_csi_get_clk_frequency(csi, false);

    if (vl53l9_init(&vl53l9cx.dev) != 0 || vl53l9cx_read_calibration() != 0) {
        return -1;
    }

    vl53l9cx.booted = true;
    return 0;
}

static bool vl53l9cx_is_booted(void) {
    vl53l9_status_t status;
    return vl53l9cx.booted &&
           vl53l9_get_status(&vl53l9cx.dev, &status) == 0 &&
           status.fsm >= VL53L9CX_FSM_STANDBY;
}

static int reset(omv_csi_t *csi) {
    vi3c_deinit();
    vl53l9cx_stop();

    if (!vl53l9cx_is_booted() && vl53l9cx_boot(csi, true) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    vl53l9cx.binning = &vl53l9cx_binnings[0];
    vl53l9cx.context = VL53L9_CONTEXT_SHORT;
    vl53l9cx.exposure_ms = VL53L9CX_EXPOSURE_DEFAULT;
    vl53l9cx.fps = VL53L9CX_FPS_DEFAULT;
    vl53l9cx.planes = OMV_CSI_VL53L9CX_PLANES_ALL;
    vl53l9cx.min_mm = VL53L9CX_MIN_MM_DEFAULT;
    vl53l9cx.max_mm = VL53L9CX_MAX_MM_DEFAULT;
    vl53l9cx.triggered = false;

    if (vl53l9cx_configure() != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

static int sleep(omv_csi_t *csi, int enable) {
    return enable ? vl53l9cx_stop() : 0;
}

static int match(omv_csi_t *csi, size_t id) {
    return id == VL53L9CX_ID;
}

static int config(omv_csi_t *csi, omv_csi_config_t config) {
    if (config == OMV_CSI_CONFIG_INIT && reset(csi) != 0) {
        return OMV_CSI_ERROR_CSI_INIT_FAILED;
    }
    return 0;
}

static int _abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    // Stopping talks to the sensor, which can't be done from an IRQ.
    return in_irq ? 0 : vl53l9cx_stop();
}

static int set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    return (pixformat == PIXFORMAT_GRAYSCALE || pixformat == PIXFORMAT_RGB565) ? 0 : -1;
}

// Picks the lowest resolution that covers the frame size, which is also
// the fastest. Larger frame sizes are scaled up from the full resolution.
static int set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    uint16_t w = csi->resolution[framesize][csi->transpose ? 1 : 0];
    uint16_t h = csi->resolution[framesize][csi->transpose ? 0 : 1];
    const vl53l9cx_binning_t *binning = &vl53l9cx_binnings[0];

    for (size_t i = 0; i < OMV_ARRAY_SIZE(vl53l9cx_binnings); i++) {
        if (vl53l9cx_binnings[i].w >= w && vl53l9cx_binnings[i].crop_h >= h) {
            binning = &vl53l9cx_binnings[i];
        }
    }

    if (binning == vl53l9cx.binning) {
        return 0;
    }

    vl53l9cx.binning = binning;
    return vl53l9cx_reconfigure();
}

static int set_framerate(omv_csi_t *csi, int framerate) {
    if (framerate < VL53L9CX_FPS_MIN || framerate > VL53L9CX_FPS_MAX) {
        return -1;
    }
    vl53l9cx.fps = framerate;
    csi->framerate = framerate;
    return vl53l9cx_reconfigure();
}

static int set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    if (enable) {
        return -1;
    }
    if (exposure_us >= 0) {
        vl53l9cx.exposure_ms = IM_CLAMP(exposure_us / 1000, VL53L9CX_EXPOSURE_MIN, VL53L9CX_EXPOSURE_MAX);
    }
    return vl53l9cx_reconfigure();
}

static int get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    *exposure_us = vl53l9cx.exposure_ms * 1000;
    return 0;
}

static int set_hmirror(omv_csi_t *csi, int enable) {
    return 0;
}

static int set_vflip(omv_csi_t *csi, int enable) {
    return 0;
}

// Maps depth to 1-255, near is bright. Invalid pixels are 0.
static void vl53l9cx_fill_image(omv_csi_t *csi, image_t *image) {
    const vl53l9cx_binning_t *binning = vl53l9cx.binning;
    const uint16_t *depth = vl53l9cx_plane(0);
    const uint16_t *amplitude = (vl53l9cx.planes >= OMV_CSI_VL53L9CX_PLANES_AMPLITUDE) ? vl53l9cx_plane(1) : NULL;
    int32_t range = IM_MAX(vl53l9cx.max_mm - vl53l9cx.min_mm, 1);

    for (int y = 0; y < binning->crop_h; y++) {
        int sy = csi->vflip ? (binning->crop_h - 1 - y) : y;
        for (int x = 0; x < binning->w; x++) {
            int sx = csi->hmirror ? (binning->w - 1 - x) : x;
            int32_t mm = depth[y * binning->w + x] & VL53L9CX_DEPTH_MASK;
            uint8_t value = 0;

            if (mm && (!amplitude || amplitude[y * binning->w + x])) {
                int32_t scaled = ((mm - vl53l9cx.min_mm) * 254) / range;
                value = 255 - IM_CLAMP(scaled, 0, 254);
            }

            if (csi->transpose) {
                IMAGE_PUT_GRAYSCALE_PIXEL(image, sy, sx, value);
            } else {
                IMAGE_PUT_GRAYSCALE_PIXEL(image, sx, sy, value);
            }
        }
    }
}

static int snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    framebuffer_t *fb = csi->fb;

    if (!vl53l9cx.binning) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    vl53l9cx.frame = NULL;
    for (mp_uint_t tick_start = mp_hal_ticks_ms(); ; mp_event_handle_nowait()) {
        // Restart the stream if it was stopped or failed.
        if (!vi3c_active()) {
            vl53l9cx.trigger_pending = false;
            if (vl53l9cx_stop() != 0 || vl53l9cx_start() != 0) {
                return OMV_CSI_ERROR_IO_ERROR;
            }
        }

        // In triggered mode, each snapshot starts a new frame.
        if (vl53l9cx.triggered && !vl53l9cx.trigger_pending) {
            while (vi3c_get_frame()) {
            }
            if (vl53l9_trigger_frame(&vl53l9cx.dev) != 0) {
                return OMV_CSI_ERROR_IO_ERROR;
            }
            vl53l9cx.trigger_pending = true;
        }

        if ((vl53l9cx.frame = vi3c_get_frame())) {
            vl53l9cx.trigger_pending = false;
            vl53l9cx_correct_frame();
            break;
        }

        if (flags & OMV_CSI_FLAG_NON_BLOCK) {
            return OMV_CSI_ERROR_WOULD_BLOCK;
        }

        if ((mp_hal_ticks_ms() - tick_start) > VL53L9CX_SNAPSHOT_TIMEOUT) {
            vl53l9cx.trigger_pending = false;
            vl53l9cx_stop();
            return OMV_CSI_ERROR_CAPTURE_TIMEOUT;
        }
    }

    if (!framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK)) {
        return OMV_CSI_ERROR_FRAMEBUFFER_ERROR;
    }

    fb->w = csi->transpose ? fb->v : fb->u;
    fb->h = csi->transpose ? fb->u : fb->v;
    fb->pixfmt = csi->pixformat;
    framebuffer_release(fb, FB_FLAG_FREE);

    image_t fb_image;
    framebuffer_to_image(fb, &fb_image);

    image_t temp = {
        .w = csi->transpose ? vl53l9cx.binning->crop_h : vl53l9cx.binning->w,
        .h = csi->transpose ? vl53l9cx.binning->w : vl53l9cx.binning->crop_h,
        .pixfmt = PIXFORMAT_GRAYSCALE,
        .data = uma_malloc(vl53l9cx.binning->w * vl53l9cx.binning->crop_h, UMA_CACHE),
    };

    vl53l9cx_fill_image(csi, &temp);

    imlib_draw_image(&fb_image, &temp, 0, 0, 1.0f, 1.0f, NULL, -1, 255,
                     (csi->pixformat == PIXFORMAT_RGB565) ? csi->color_palette : NULL, NULL,
                     IMAGE_HINT_BILINEAR | IMAGE_HINT_CENTER | IMAGE_HINT_SCALE_ASPECT_EXPAND,
                     NULL, NULL, NULL, NULL);

    uma_free(temp.data);
    framebuffer_to_image(fb, image);
    return 0;
}

static int ioctl(omv_csi_t *csi, int request, void *arg) {
    vl53l9cx_ioctl_arg_t *a = arg;

    switch (request) {
        case OMV_CSI_IOCTL_SET_TRIGGERED_MODE:
            vl53l9cx.triggered = a->ivalue;
            return vl53l9cx_reconfigure();
        case OMV_CSI_IOCTL_GET_TRIGGERED_MODE:
            a->ivalue = vl53l9cx.triggered;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_SET_RANGE:
            if (a->range.min < 0 || a->range.max <= a->range.min) {
                return OMV_CSI_ERROR_INVALID_ARGUMENT;
            }
            vl53l9cx.min_mm = a->range.min;
            vl53l9cx.max_mm = a->range.max;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_GET_RANGE:
            a->range.min = vl53l9cx.min_mm;
            a->range.max = vl53l9cx.max_mm;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_SET_MODE:
            if (a->ivalue != OMV_CSI_VL53L9CX_MODE_PRECISION && a->ivalue != OMV_CSI_VL53L9CX_MODE_RANGE) {
                return OMV_CSI_ERROR_INVALID_ARGUMENT;
            }
            vl53l9cx.context = (a->ivalue == OMV_CSI_VL53L9CX_MODE_RANGE) ?
                               VL53L9_CONTEXT_LONG : VL53L9_CONTEXT_SHORT;
            return vl53l9cx_reconfigure();
        case OMV_CSI_IOCTL_VL53L9CX_GET_MODE:
            a->ivalue = (vl53l9cx.context == VL53L9_CONTEXT_LONG) ?
                        OMV_CSI_VL53L9CX_MODE_RANGE : OMV_CSI_VL53L9CX_MODE_PRECISION;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_SET_PLANES:
            if (a->ivalue < OMV_CSI_VL53L9CX_PLANES_DEPTH || a->ivalue > OMV_CSI_VL53L9CX_PLANES_ALL) {
                return OMV_CSI_ERROR_INVALID_ARGUMENT;
            }
            vl53l9cx.planes = a->ivalue;
            return vl53l9cx_reconfigure();
        case OMV_CSI_IOCTL_VL53L9CX_GET_PLANES:
            a->ivalue = vl53l9cx.planes;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_GET_BINNING:
            a->ivalue = vl53l9cx.binning->binning;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_GET_RESOLUTION:
            a->range.min = vl53l9cx.binning->w;
            a->range.max = vl53l9cx.binning->crop_h;
            return 0;
        case OMV_CSI_IOCTL_VL53L9CX_GET_FRAME_COUNTER:
        case OMV_CSI_IOCTL_VL53L9CX_GET_DEPTH:
        case OMV_CSI_IOCTL_VL53L9CX_GET_AMPLITUDE:
        case OMV_CSI_IOCTL_VL53L9CX_GET_AMBIENT: {
            // These read the frame returned by the last snapshot.
            if (!vl53l9cx.frame) {
                return OMV_CSI_ERROR_CTL_FAILED;
            }
            if (request == OMV_CSI_IOCTL_VL53L9CX_GET_FRAME_COUNTER) {
                // The status line follows the planes.
                const uint8_t *status = vl53l9cx.frame->data + vl53l9cx_plane_size() * vl53l9cx.planes * sizeof(uint16_t);
                a->ivalue = status[0] | (status[1] << 8) | (status[2] << 16) | ((uint32_t) status[3] << 24);
                return 0;
            }
            size_t index = request - OMV_CSI_IOCTL_VL53L9CX_GET_DEPTH;
            if (index >= vl53l9cx.planes) {
                return OMV_CSI_ERROR_CTL_FAILED;
            }
            a->plane.data = vl53l9cx_plane(index);
            a->plane.w = vl53l9cx.binning->w;
            a->plane.h = vl53l9cx.binning->crop_h;
            a->plane.mask = (index == 0) ? VL53L9CX_DEPTH_MASK : 0xFFFF;
            return 0;
        }
        default:
            return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }
}

int vl53l9cx_init(omv_csi_t *csi) {
    csi->reset = reset;
    csi->sleep = sleep;
    csi->config = config;
    csi->abort = _abort;
    csi->shutdown = NULL;
    csi->match = match;
    csi->snapshot = snapshot;
    csi->set_pixformat = set_pixformat;
    csi->set_framesize = set_framesize;
    csi->set_framerate = set_framerate;
    csi->set_auto_exposure = set_auto_exposure;
    csi->get_exposure_us = get_exposure_us;
    csi->set_hmirror = set_hmirror;
    csi->set_vflip = set_vflip;
    csi->ioctl = ioctl;

    csi->auxiliary = 1;
    csi->mono_bpp = 1;
    // Frame sync belongs to the main sensor.
    csi->fsync_pin = NULL;

    // Boot while the other sensors on the bus are still unconfigured. On
    // failure, reset() retries with a power cycle.
    vl53l9cx_boot(csi, false);
    return 0;
}
#endif // (OMV_VL53L9CX_ENABLE == 1)
