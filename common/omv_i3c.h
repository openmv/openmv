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
 * I3C bus abstraction layer.
 *
 * An I3C bus is an omv_i2c_t created with omv_i2c_init() on an I3C bus ID.
 * Legacy I2C targets keep using the omv_i2c API on it, while I3C targets
 * are addressed by their dynamic address with the functions below. Like
 * omv_i2c, all addresses are 7-bit addresses shifted left by one.
 */
#ifndef __OMV_I3C_H__
#define __OMV_I3C_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "omv_i2c.h"

// Message flags.
#define OMV_I3C_MSG_READ    (1 << 0)

typedef struct _omv_i3c_msg {
    uint8_t *buf;
    uint32_t len;
    uint32_t flags;
} omv_i3c_msg_t;

// Called from IRQ context when an asynchronous transfer completes.
typedef void (*omv_i3c_callback_t) (omv_i2c_t *bus, int status, void *arg);

// Returns true if the bus is driven by an I3C controller.
bool omv_i3c_is_i3c(omv_i2c_t *bus);

// Resets all dynamic addresses and runs ENTDAA. Returns the number of
// targets assigned, storing up to count addresses and 48-bit PIDs.
int omv_i3c_daa(omv_i2c_t *bus, uint8_t *addrs, uint64_t *pids, size_t count);

// Sends a frame of private messages to one target, with repeated starts
// in between. The data of all read messages is stored back-to-back, so
// their buffers must be contiguous.
int omv_i3c_transfer(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count);

// Same as above using DMA. Writes are copied and may be released on return,
// reads must stay valid until the callback runs.
int omv_i3c_transfer_async(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count,
                           omv_i3c_callback_t callback, void *arg);

// Aborts a pending asynchronous transfer without calling its callback.
void omv_i3c_abort(omv_i2c_t *bus);

// Port functions backing omv_i2c on I3C bus IDs.
int omv_i3c_init(omv_i2c_t *bus, uint32_t bus_id, uint32_t speed);
int omv_i3c_deinit(omv_i2c_t *bus);
int omv_i3c_scan(omv_i2c_t *bus, uint8_t *list, uint8_t size);
int omv_i3c_transfer_i2c(omv_i2c_t *bus, uint8_t addr, omv_i3c_msg_t *msgs, size_t count);

#endif // __OMV_I3C_H__
