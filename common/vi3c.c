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

#include "vi3c.h"
#include "omv_common.h"
#include "omv_i3c.h"

#define VI3C_FRAME_REG          (0x1800)
#define VI3C_CMD_REG            (0x0400)
// Commands are cleared once done.
#define VI3C_CMD_ACK_FRAME      (0x06)
#define VI3C_CMD_DSS_MAP        (0x0A) // Maps the DSS indexes at the frame register.
#define VI3C_CMD_DSS_UNMAP      (0x0B)

// The poll byte fills a cache line, which is invalidated after each read.
#define VI3C_BUFFER_SIZE        (OMV_DMA_ALIGNMENT)

typedef enum {
    VI3C_FLAG_STREAM  = (1 << 0),
    VI3C_FLAG_CAPTURE = (1 << 1),
    VI3C_FLAG_PENDING = (1 << 2), // A frame was signalled while busy.
} vi3c_flags_t;

// Each frame is read, then its DSS indexes if needed, then acknowledged.
// Each command is polled until cleared.
typedef enum {
    VI3C_STEP_IDLE,
    VI3C_STEP_READ_FRAME,
    VI3C_STEP_READ_DSS,
    VI3C_STEP_CMD,
    VI3C_STEP_POLL,
} vi3c_step_t;

typedef struct _vi3c_state {
    framebuffer_t *fb;
    vbuffer_t *buffer;
    omv_i2c_t *i2c;
    uint8_t slv_addr;
    uint8_t cmd;
    size_t frame_size;
    size_t dss_size;
    volatile vi3c_step_t step;
    volatile uint32_t flags;
} vi3c_state_t;

static vi3c_state_t vi3c;
static uint8_t OMV_ATTR_SEC_ALIGN(vi3c_buf[VI3C_BUFFER_SIZE], ".dma_buffer", OMV_DMA_ALIGNMENT);

static void vi3c_callback(omv_i2c_t *i2c, void *userdata, void *buf);

static int vi3c_read(uint16_t reg_addr, void *buf, uint32_t size) {
    uint8_t addr[] = { reg_addr >> 8, reg_addr };
    omv_i3c_transfer_t xfer = {
        .slv_addr = vi3c.slv_addr,
        .txbuf = addr,
        .txsize = sizeof(addr),
        .rxbuf = buf,
        .rxsize = size,
        .flags = OMV_I3C_XFER_DMA,
        .callback = vi3c_callback,
    };
    return omv_i3c_transfer_start(vi3c.i2c, &xfer);
}

static int vi3c_command(uint8_t cmd) {
    uint8_t buf[] = { VI3C_CMD_REG >> 8, VI3C_CMD_REG & 0xFF, cmd };
    omv_i3c_transfer_t xfer = {
        .slv_addr = vi3c.slv_addr,
        .txbuf = buf,
        .txsize = sizeof(buf),
        .flags = OMV_I3C_XFER_DMA,
        .callback = vi3c_callback,
    };
    vi3c.cmd = cmd;
    vi3c.step = VI3C_STEP_CMD;
    return omv_i3c_transfer_start(vi3c.i2c, &xfer);
}

static int vi3c_next_frame(void) {
    if ((vi3c.flags & VI3C_FLAG_CAPTURE) &&
        (vi3c.buffer = framebuffer_acquire(vi3c.fb, FB_FLAG_FREE | FB_FLAG_PEEK))) {
        vi3c.step = VI3C_STEP_READ_FRAME;
        return vi3c_read(VI3C_FRAME_REG, vi3c.buffer->data, vi3c.frame_size);
    }

    // No free buffer, so the frame is dropped.
    vi3c.flags &= ~VI3C_FLAG_CAPTURE;
    return vi3c_command(VI3C_CMD_ACK_FRAME);
}

static int vi3c_release_frame(void) {
    // Replaces the newest frame if it's the last free buffer.
    framebuffer_release(vi3c.fb, FB_FLAG_FREE | FB_FLAG_CHECK_LAST);
    return vi3c_command(VI3C_CMD_ACK_FRAME);
}

// Runs once the current command is done.
static int vi3c_command_done(void) {
    switch (vi3c.cmd) {
        case VI3C_CMD_DSS_MAP: {
            vi3c.step = VI3C_STEP_READ_DSS;
            return vi3c_read(VI3C_FRAME_REG, vi3c.buffer->data + vi3c.frame_size, vi3c.dss_size);
        }
        case VI3C_CMD_DSS_UNMAP: {
            return vi3c_release_frame();
        }
        default: {
            vi3c.step = VI3C_STEP_IDLE;
            if (vi3c.flags & VI3C_FLAG_PENDING) {
                vi3c.flags &= ~VI3C_FLAG_PENDING;
                return vi3c_next_frame();
            }
            return 0;
        }
    }
}

static void vi3c_callback(omv_i2c_t *i2c, void *userdata, void *buf) {
    int ret = 0;

    if (!(vi3c.flags & VI3C_FLAG_STREAM)) {
        return;
    }

    if (i2c->xfer_flags & OMV_I3C_XFER_FAILED) {
        vi3c_abort();
        return;
    }

    switch (vi3c.step) {
        case VI3C_STEP_READ_FRAME: {
            ret = vi3c.dss_size ? vi3c_command(VI3C_CMD_DSS_MAP) : vi3c_release_frame();
            break;
        }
        case VI3C_STEP_READ_DSS: {
            ret = vi3c_command(VI3C_CMD_DSS_UNMAP);
            break;
        }
        case VI3C_STEP_CMD: {
            vi3c.step = VI3C_STEP_POLL;
            ret = vi3c_read(VI3C_CMD_REG, vi3c_buf, 1);
            break;
        }
        case VI3C_STEP_POLL: {
            if (vi3c_buf[0] != 0) {
                ret = vi3c_read(VI3C_CMD_REG, vi3c_buf, 1);
            } else {
                ret = vi3c_command_done();
            }
            break;
        }
        default: {
            break;
        }
    }

    if (ret != 0) {
        vi3c_abort();
    }
}

static void vi3c_ibi_callback(void *data) {
    if (!(vi3c.flags & VI3C_FLAG_STREAM)) {
        return;
    }

    if (vi3c.step != VI3C_STEP_IDLE) {
        vi3c.flags |= VI3C_FLAG_PENDING;
    } else if (vi3c_next_frame() != 0) {
        vi3c_abort();
    }
}

// The DSS indexes, if any, are read to the frame buffer after the frame.
int vi3c_init(omv_i2c_t *i2c, uint8_t slv_addr, size_t frame_size, size_t dss_size, framebuffer_t *fb) {
    memset(&vi3c, 0, sizeof(vi3c_state_t));
    vi3c.fb = fb;
    vi3c.i2c = i2c;
    vi3c.slv_addr = slv_addr;
    vi3c.frame_size = frame_size;
    vi3c.dss_size = dss_size;
    return 0;
}

int vi3c_deinit(void) {
    return vi3c_abort();
}

bool vi3c_active(void) {
    return (vi3c.flags & VI3C_FLAG_CAPTURE) &&
           (vi3c.flags & VI3C_FLAG_STREAM);
}

int vi3c_abort(void) {
    vi3c.flags = 0;
    if (vi3c.i2c) {
        omv_i3c_ibi_enable(vi3c.i2c, false);
        omv_i3c_transfer_abort(vi3c.i2c);
    }
    vi3c.step = VI3C_STEP_IDLE;
    return 0;
}

void vi3c_restart(void) {
    // Resume capturing.
    vi3c.flags |= VI3C_FLAG_CAPTURE;

    if (!(vi3c.flags & VI3C_FLAG_STREAM)) {
        // Restart the stream. A frame signalled meanwhile is still flagged,
        // so it's read once enabled.
        vi3c.flags |= VI3C_FLAG_STREAM;
        omv_i3c_ibi_register(vi3c.i2c, vi3c.slv_addr, vi3c_ibi_callback, NULL);
        omv_i3c_ibi_enable(vi3c.i2c, true);
    }
}
#endif // OMV_VL53L9CX_ENABLE
