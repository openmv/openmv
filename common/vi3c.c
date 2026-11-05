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
 */
#include "board_config.h"
#if OMV_VI3C_ENABLE || (OMV_VL53L9CX_ENABLE == 1)

#include <stdint.h>
#include <string.h>

#include "py/mphal.h"

#include "omv_common.h"
#include "omv_i3c.h"
#include "queue.h"
#include "umalloc.h"
#include "vi3c.h"

typedef enum {
    VI3C_STATE_IDLE,
    VI3C_STATE_READ,
    VI3C_STATE_ACK,
    VI3C_STATE_POLL,
} vi3c_state_t;

typedef struct {
    vi3c_config_t config;
    framebuffer_t fb;
    vbuffer_t *buffer;
    volatile vi3c_state_t state;
    volatile bool active;
    bool held;
} vi3c_t;

static vi3c_t vi3c;

// The polled command byte is read with DMA, so it gets its own cache line.
static uint8_t OMV_ATTR_SEC_ALIGN(vi3c_cmd[OMV_DMA_ALIGNMENT], ".dma_buffer", OMV_DMA_ALIGNMENT);

static void vi3c_callback(omv_i2c_t *bus, int status, void *arg);

static int vi3c_read_frame(void) {
    omv_i3c_msg_t msgs[VI3C_MAX_SEGMENTS * 2];
    uint8_t regs[VI3C_MAX_SEGMENTS][2];
    uint8_t *data = vi3c.buffer->data;

    // Each segment is a register address write followed by a read.
    for (size_t i = 0; i < vi3c.config.n_segments; i++) {
        const vi3c_segment_t *seg = &vi3c.config.segments[i];
        omv_i3c_msg_t *msg = &msgs[i * 2];

        regs[i][0] = seg->reg >> 8;
        regs[i][1] = seg->reg;
        msg[0].buf = regs[i];
        msg[0].len = 2;
        msg[0].flags = 0;
        msg[1].buf = data;
        msg[1].len = seg->size;
        msg[1].flags = OMV_I3C_MSG_READ;
        data += seg->size;
    }

    return omv_i3c_transfer_async(vi3c.config.bus, vi3c.config.addr, msgs,
                                  vi3c.config.n_segments * 2, vi3c_callback, NULL);
}

static int vi3c_write_ack(void) {
    uint8_t buf[] = { vi3c.config.cmd_reg >> 8, vi3c.config.cmd_reg, vi3c.config.ack_cmd };
    omv_i3c_msg_t msg = { buf, sizeof(buf), 0 };
    return omv_i3c_transfer_async(vi3c.config.bus, vi3c.config.addr, &msg, 1, vi3c_callback, NULL);
}

static int vi3c_poll_cmd(void) {
    uint8_t reg[] = { vi3c.config.cmd_reg >> 8, vi3c.config.cmd_reg };
    omv_i3c_msg_t msgs[] = {
        { reg, sizeof(reg), 0 },
        { vi3c_cmd, 1, OMV_I3C_MSG_READ },
    };
    return omv_i3c_transfer_async(vi3c.config.bus, vi3c.config.addr, msgs, 2, vi3c_callback, NULL);
}

static int vi3c_next_frame(void) {
    vi3c.buffer = framebuffer_acquire(&vi3c.fb, FB_FLAG_FREE | FB_FLAG_PEEK);

    if (vi3c.buffer) {
        vi3c.state = VI3C_STATE_READ;
        return vi3c_read_frame();
    }

    // No free buffer, so drop this frame.
    vi3c.state = VI3C_STATE_ACK;
    return vi3c_write_ack();
}

static void vi3c_callback(omv_i2c_t *bus, int status, void *arg) {
    int ret = status;

    if (ret == 0) {
        switch (vi3c.state) {
            case VI3C_STATE_READ:
                vi3c.state = VI3C_STATE_ACK;
                ret = vi3c_write_ack();
                break;
            case VI3C_STATE_ACK:
                vi3c.state = VI3C_STATE_POLL;
                ret = vi3c_poll_cmd();
                break;
            case VI3C_STATE_POLL:
                // The sensor clears the command once the frame is released.
                if (vi3c_cmd[0] != 0) {
                    ret = vi3c_poll_cmd();
                    break;
                }
                if (vi3c.buffer) {
                    framebuffer_release(&vi3c.fb, FB_FLAG_FREE);
                    vi3c.buffer = NULL;
                }
                vi3c.state = VI3C_STATE_IDLE;
                // The pin is edge-triggered, so catch a frame signalled meanwhile.
                if (!omv_gpio_read(vi3c.config.irq_pin)) {
                    ret = vi3c_next_frame();
                }
                break;
            default:
                break;
        }
    }

    if (ret != 0) {
        vi3c.buffer = NULL;
        vi3c.state = VI3C_STATE_IDLE;
        vi3c.active = false;
    }
}

static void vi3c_irq_callback(void *data) {
    if (vi3c.active && vi3c.state == VI3C_STATE_IDLE && vi3c_next_frame() != 0) {
        vi3c.buffer = NULL;
        vi3c.state = VI3C_STATE_IDLE;
        vi3c.active = false;
    }
}

int vi3c_init(const vi3c_config_t *config) {
    size_t frame_size = 0;

    vi3c_deinit();

    if (config->n_segments == 0 || config->n_segments > VI3C_MAX_SEGMENTS) {
        return -1;
    }

    for (size_t i = 0; i < config->n_segments; i++) {
        frame_size += config->segments[i].size;
    }

    vi3c.config = *config;
    framebuffer_init(&vi3c.fb, NULL, 0, true, true);
    if (framebuffer_resize(&vi3c.fb, config->n_buffers, frame_size) != 0) {
        return -1;
    }

    omv_gpio_config(config->irq_pin, OMV_GPIO_MODE_IT_FALL, OMV_GPIO_PULL_UP, OMV_GPIO_SPEED_LOW, -1);
    omv_gpio_irq_register(config->irq_pin, vi3c_irq_callback, NULL);
    return 0;
}

void vi3c_deinit(void) {
    if (vi3c.config.bus) {
        vi3c_stop();
        omv_gpio_deinit(vi3c.config.irq_pin);
    }

    if (vi3c.fb.raw_base) {
        uma_free(vi3c.fb.raw_base);
    }

    memset(&vi3c, 0, sizeof(vi3c));
}

int vi3c_start(void) {
    if (!vi3c.config.bus) {
        return -1;
    }

    vi3c_stop();
    vi3c.active = true;
    omv_gpio_irq_enable(vi3c.config.irq_pin, true);

    // A frame may already be pending.
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    if (!omv_gpio_read(vi3c.config.irq_pin)) {
        vi3c_irq_callback(NULL);
    }
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    return vi3c.active ? 0 : -1;
}

void vi3c_stop(void) {
    if (!vi3c.config.bus) {
        return;
    }

    vi3c.active = false;
    omv_gpio_irq_enable(vi3c.config.irq_pin, false);
    omv_i3c_abort(vi3c.config.bus);

    vi3c.buffer = NULL;
    vi3c.state = VI3C_STATE_IDLE;
    vi3c.held = false;
    framebuffer_flush(&vi3c.fb);
}

bool vi3c_active(void) {
    return vi3c.active;
}

vbuffer_t *vi3c_get_frame(void) {
    // Release the frame held since the last call.
    if (vi3c.held) {
        framebuffer_release(&vi3c.fb, FB_FLAG_USED);
        vi3c.held = false;
    }

    // Drop all but the newest frame.
    while (queue_size(vi3c.fb.used_queue) > 1) {
        framebuffer_release(&vi3c.fb, FB_FLAG_USED);
    }

    vbuffer_t *buffer = framebuffer_acquire(&vi3c.fb, FB_FLAG_USED | FB_FLAG_PEEK);
    vi3c.held = (buffer != NULL);
    return buffer;
}
#endif // OMV_VI3C_ENABLE || (OMV_VL53L9CX_ENABLE == 1)
