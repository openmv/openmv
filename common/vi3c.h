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
 * Streams frames from register-mapped I3C sensors that signal a ready frame
 * on an interrupt pin. Each frame is read with DMA into a private queue, then
 * acknowledged with a command that the sensor clears once done.
 */
#ifndef __VI3C_H__
#define __VI3C_H__
#include <stdint.h>
#include <stdbool.h>
#include "omv_i2c.h"
#include "omv_gpio.h"
#include "framebuffer.h"

#define VI3C_MAX_SEGMENTS   (2)

typedef struct {
    uint16_t reg;
    uint16_t size;
} vi3c_segment_t;

typedef struct {
    omv_i2c_t *bus;
    uint8_t addr;
    // Frame-ready pin, active low.
    omv_gpio_t irq_pin;
    // Register ranges read back-to-back into each frame.
    vi3c_segment_t segments[VI3C_MAX_SEGMENTS];
    size_t n_segments;
    // Command register, the ack command and the number of frame buffers.
    uint16_t cmd_reg;
    uint8_t ack_cmd;
    size_t n_buffers;
} vi3c_config_t;

int vi3c_init(const vi3c_config_t *config);
void vi3c_deinit(void);
int vi3c_start(void);
void vi3c_stop(void);
bool vi3c_active(void);
// Returns the newest frame, held until the next call or vi3c_stop().
vbuffer_t *vi3c_get_frame(void);
#endif // __VI3C_H__
