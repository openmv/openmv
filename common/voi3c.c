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
 * Video over I3C driver.
 *
 * Reads the frames of the VL53L9CX, which signals each frame with an in-band
 * interrupt and holds it until it's acknowledged.
 */
#include "board_config.h"
#if OMV_VL53L9CX_ENABLE

#include <stdint.h>
#include <string.h>

#include "py/mphal.h"

#include "voi3c.h"
#include "omv_common.h"
#include "omv_i3c.h"

#define VOI3C_FRAME_REG          (0x1800)
#define VOI3C_CMD_REG            (0x0400)
// Commands are cleared once done.
#define VOI3C_CMD_ACK_FRAME      (0x06)
#define VOI3C_CMD_DSS_MAP        (0x0A) // Maps the DSS indexes at the frame register.
#define VOI3C_CMD_DSS_UNMAP      (0x0B)
// A command the sensor doesn't clear in time aborts the frame.
#define VOI3C_CMD_TIMEOUT_MS     (30)

// The poll byte fills a cache line, which is invalidated after each read.
#define VOI3C_BUFFER_SIZE        (OMV_DMA_ALIGNMENT)

typedef enum {
    VOI3C_FLAG_STREAM  = (1 << 0),
    VOI3C_FLAG_CAPTURE = (1 << 1),
    VOI3C_FLAG_PENDING = (1 << 2), // A frame was signalled while busy.
} voi3c_flags_t;

// The transfer in flight. Each frame is read, then its DSS indexes if needed,
// then acknowledged. Each command is polled until the sensor clears it.
typedef enum {
    VOI3C_STEP_IDLE,
    VOI3C_STEP_READ_FRAME,
    VOI3C_STEP_MAP_DSS,
    VOI3C_STEP_POLL_MAP_DSS,
    VOI3C_STEP_READ_DSS,
    VOI3C_STEP_UNMAP_DSS,
    VOI3C_STEP_POLL_UNMAP_DSS,
    VOI3C_STEP_ACK_FRAME,
    VOI3C_STEP_POLL_ACK_FRAME,
} voi3c_step_t;

typedef struct _voi3c_state {
    framebuffer_t *fb;
    vbuffer_t *buffer;
    omv_i2c_t *i2c;
    uint8_t slv_addr;
    size_t frame_size;
    size_t dss_size;
    mp_uint_t cmd_start;
    volatile voi3c_step_t step;
    volatile uint32_t flags;
} voi3c_state_t;

static voi3c_state_t voi3c;
static uint8_t OMV_ATTR_SEC_ALIGN(voi3c_buf[VOI3C_BUFFER_SIZE], ".dma_buffer", OMV_DMA_ALIGNMENT);

static void voi3c_callback(omv_i2c_t *i2c, void *userdata, void *buf);

static int voi3c_read(voi3c_step_t step, uint16_t reg_addr, void *buf, uint32_t size) {
    uint8_t addr[] = { reg_addr >> 8, reg_addr };
    omv_i3c_transfer_t xfer = {
        .slv_addr = voi3c.slv_addr,
        .txbuf = addr,
        .txsize = sizeof(addr),
        .rxbuf = buf,
        .rxsize = size,
        .flags = OMV_I3C_XFER_DMA,
        .callback = voi3c_callback,
    };
    voi3c.step = step;
    return omv_i3c_transfer_start(voi3c.i2c, &xfer);
}

static int voi3c_command(voi3c_step_t step, uint8_t cmd) {
    uint8_t buf[] = { VOI3C_CMD_REG >> 8, VOI3C_CMD_REG & 0xFF, cmd };
    omv_i3c_transfer_t xfer = {
        .slv_addr = voi3c.slv_addr,
        .txbuf = buf,
        .txsize = sizeof(buf),
        .flags = OMV_I3C_XFER_DMA,
        .callback = voi3c_callback,
    };
    voi3c.step = step;
    voi3c.cmd_start = mp_hal_ticks_ms();
    return omv_i3c_transfer_start(voi3c.i2c, &xfer);
}

static int voi3c_poll(voi3c_step_t step) {
    if ((mp_hal_ticks_ms() - voi3c.cmd_start) > VOI3C_CMD_TIMEOUT_MS) {
        return -1;
    }
    return voi3c_read(step, VOI3C_CMD_REG, voi3c_buf, 1);
}

// Starts the next transfer once the current one is done, or once a frame is
// signalled while idle.
static int voi3c_next(void) {
    switch (voi3c.step) {
        case VOI3C_STEP_IDLE: {
            voi3c.buffer = NULL;
            if (voi3c.flags & VOI3C_FLAG_CAPTURE) {
                voi3c.buffer = framebuffer_acquire(voi3c.fb, FB_FLAG_FREE | FB_FLAG_PEEK);
            }
            if (voi3c.buffer) {
                return voi3c_read(VOI3C_STEP_READ_FRAME, VOI3C_FRAME_REG, voi3c.buffer->data, voi3c.frame_size);
            }
            // No free buffer, so the frame is dropped.
            voi3c.flags &= ~VOI3C_FLAG_CAPTURE;
            return voi3c_command(VOI3C_STEP_ACK_FRAME, VOI3C_CMD_ACK_FRAME);
        }
        case VOI3C_STEP_READ_FRAME: {
            if (voi3c.dss_size) {
                return voi3c_command(VOI3C_STEP_MAP_DSS, VOI3C_CMD_DSS_MAP);
            }
            // Replaces the newest frame if it's the last free buffer.
            framebuffer_release(voi3c.fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
            return voi3c_command(VOI3C_STEP_ACK_FRAME, VOI3C_CMD_ACK_FRAME);
        }
        case VOI3C_STEP_MAP_DSS: {
            return voi3c_poll(VOI3C_STEP_POLL_MAP_DSS);
        }
        case VOI3C_STEP_POLL_MAP_DSS: {
            if (voi3c_buf[0] != 0) {
                return voi3c_poll(VOI3C_STEP_POLL_MAP_DSS);
            }
            return voi3c_read(VOI3C_STEP_READ_DSS, VOI3C_FRAME_REG, voi3c.buffer->data + voi3c.frame_size, voi3c.dss_size);
        }
        case VOI3C_STEP_READ_DSS: {
            return voi3c_command(VOI3C_STEP_UNMAP_DSS, VOI3C_CMD_DSS_UNMAP);
        }
        case VOI3C_STEP_UNMAP_DSS: {
            return voi3c_poll(VOI3C_STEP_POLL_UNMAP_DSS);
        }
        case VOI3C_STEP_POLL_UNMAP_DSS: {
            if (voi3c_buf[0] != 0) {
                return voi3c_poll(VOI3C_STEP_POLL_UNMAP_DSS);
            }
            framebuffer_release(voi3c.fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
            return voi3c_command(VOI3C_STEP_ACK_FRAME, VOI3C_CMD_ACK_FRAME);
        }
        case VOI3C_STEP_ACK_FRAME: {
            return voi3c_poll(VOI3C_STEP_POLL_ACK_FRAME);
        }
        case VOI3C_STEP_POLL_ACK_FRAME: {
            if (voi3c_buf[0] != 0) {
                return voi3c_poll(VOI3C_STEP_POLL_ACK_FRAME);
            }
            voi3c.step = VOI3C_STEP_IDLE;
            if (voi3c.flags & VOI3C_FLAG_PENDING) {
                voi3c.flags &= ~VOI3C_FLAG_PENDING;
                return voi3c_next();
            }
            return 0;
        }
        default: {
            return 0;
        }
    }
}

// Called when a transfer is done.
static void voi3c_callback(omv_i2c_t *i2c, void *userdata, void *buf) {
    if (!(voi3c.flags & VOI3C_FLAG_STREAM)) {
        return;
    }

    if ((i2c->xfer_flags & OMV_I3C_XFER_FAILED) || voi3c_next() != 0) {
        voi3c_abort();
    }
}

// Called when a frame is signalled.
static void voi3c_ibi_callback(void *data) {
    if (!(voi3c.flags & VOI3C_FLAG_STREAM)) {
        return;
    }

    if (voi3c.step != VOI3C_STEP_IDLE) {
        voi3c.flags |= VOI3C_FLAG_PENDING;
    } else if (voi3c_next() != 0) {
        voi3c_abort();
    }
}

// The DSS indexes, if any, are read to the frame buffer after the frame.
int voi3c_init(omv_i2c_t *i2c, uint8_t slv_addr, size_t frame_size, size_t dss_size, framebuffer_t *fb) {
    memset(&voi3c, 0, sizeof(voi3c_state_t));
    voi3c.fb = fb;
    voi3c.i2c = i2c;
    voi3c.slv_addr = slv_addr;
    voi3c.frame_size = frame_size;
    voi3c.dss_size = dss_size;
    return 0;
}

// Stops capturing and releases the bus, so the framebuffer may be freed
// afterwards. voi3c_init() must be called before capturing again.
int voi3c_deinit(void) {
    voi3c_abort();

    if (voi3c.i2c) {
        omv_i3c_ibi_register(voi3c.i2c, 0, NULL, NULL);
    }

    return 0;
}

bool voi3c_active(void) {
    return (voi3c.flags & VOI3C_FLAG_CAPTURE) &&
           (voi3c.flags & VOI3C_FLAG_STREAM);
}

int voi3c_abort(void) {
    voi3c.flags = 0;
    if (voi3c.i2c) {
        omv_i3c_ibi_enable(voi3c.i2c, false);
        omv_i3c_transfer_abort(voi3c.i2c);
    }
    voi3c.step = VOI3C_STEP_IDLE;
    return 0;
}

void voi3c_restart(void) {
    // Resume capturing.
    voi3c.flags |= VOI3C_FLAG_CAPTURE;

    if (!(voi3c.flags & VOI3C_FLAG_STREAM)) {
        // Restart the stream. A frame signalled meanwhile is still flagged,
        // so it's read once enabled.
        voi3c.flags |= VOI3C_FLAG_STREAM;
        omv_i3c_ibi_register(voi3c.i2c, voi3c.slv_addr, voi3c_ibi_callback, NULL);
        omv_i3c_ibi_enable(voi3c.i2c, true);
    }
}
#endif // OMV_VL53L9CX_ENABLE
