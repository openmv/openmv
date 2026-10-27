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
 * An I3C bus is an omv_i2c_t initialized with an I3C bus ID. The omv_i2c
 * functions reach both legacy I2C targets and I3C targets on it, the latter
 * by the dynamic addresses assigned with omv_i3c_daa().
 */
#ifndef __OMV_I3C_H__
#define __OMV_I3C_H__
#include <stdint.h>
#include <stdbool.h>
#include "omv_i2c.h"

typedef enum {
    OMV_I3C_XFER_DMA      = (1 << 0),
    OMV_I3C_XFER_BLOCKING = (1 << 1),
    OMV_I3C_XFER_FAILED   = (1 << 3),
    OMV_I3C_XFER_COMPLETE = (1 << 4),
} omv_i3c_xfer_flags_t;

typedef void (*omv_i3c_callback_t) (omv_i2c_t *i2c, void *userdata, void *buf);
typedef void (*omv_i3c_ibi_callback_t) (void *data);

// Writes txbuf, then reads rxbuf after a repeated START, as one frame.
typedef struct _omv_i3c_transfer {
    uint8_t slv_addr;
    void *txbuf;
    uint32_t txsize;
    void *rxbuf;
    uint32_t rxsize;
    omv_i3c_xfer_flags_t flags;
    void *userdata;
    omv_i3c_callback_t callback;
} omv_i3c_transfer_t;

int omv_i3c_init(omv_i2c_t *i2c, uint32_t bus_id, uint32_t speed);
int omv_i3c_deinit(omv_i2c_t *i2c);
int omv_i3c_scan(omv_i2c_t *i2c, uint8_t *list, uint8_t size);
// Assigns dynamic addresses with ENTDAA, returning them with the targets' PIDs.
int omv_i3c_daa(omv_i2c_t *i2c, uint8_t *list, uint64_t *pids, uint8_t size);
int omv_i3c_read(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags);
int omv_i3c_write(omv_i2c_t *i2c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags);
// Writes are copied, so only rxbuf must stay valid until a DMA transfer completes.
int omv_i3c_transfer_start(omv_i2c_t *i2c, omv_i3c_transfer_t *xfer);
int omv_i3c_transfer_abort(omv_i2c_t *i2c);
// Calls the callback, from an IRQ, on the target's in-band interrupts once enabled.
int omv_i3c_ibi_register(omv_i2c_t *i2c, uint8_t slv_addr, omv_i3c_ibi_callback_t callback, void *data);
int omv_i3c_ibi_enable(omv_i2c_t *i2c, bool enable);

#endif // __OMV_I3C_H__
