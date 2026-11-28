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
 * An I3C controller drives I3C targets, by the dynamic addresses assigned
 * with omv_i3c_daa(), and legacy I2C targets on the same bus.
 */
#ifndef __OMV_I3C_H__
#define __OMV_I3C_H__
#include <stdint.h>
#include <stdbool.h>
#include "port_config.h"

typedef enum {
    OMV_I3C_XFER_DMA      = (1 << 0),
    OMV_I3C_XFER_BLOCKING = (1 << 1),
    OMV_I3C_XFER_FAILED   = (1 << 3),
    OMV_I3C_XFER_COMPLETE = (1 << 4),
} omv_i3c_xfer_flags_t;

typedef struct _omv_i3c omv_i3c_t;
typedef void (*omv_i3c_callback_t) (omv_i3c_t *i3c, void *userdata, void *buf);
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

typedef struct _omv_i3c {
    uint32_t id;
    uint32_t speed;
    bool initialized;
    volatile omv_i3c_xfer_flags_t xfer_flags;
    #ifdef OMV_I3C_PORT_BITS
    // Additional port-specific fields like device base pointer,
    // dma handles, more I/Os etc... are included directly here,
    // so that they can be accessible from this struct.
    OMV_I3C_PORT_BITS
    #endif
} omv_i3c_t;

// Re-initializing keeps the dynamic addresses assigned by the last DAA, as the
// targets keep them.
int omv_i3c_init(omv_i3c_t *i3c, uint32_t bus_id, uint32_t speed);
int omv_i3c_deinit(omv_i3c_t *i3c);
// Returns the initialized controller of an I3C bus ID, or NULL.
omv_i3c_t *omv_i3c_get(uint32_t bus_id);
int omv_i3c_scan(omv_i3c_t *i3c, uint8_t *list, uint8_t size);
// Assigns dynamic addresses with ENTDAA, returning them with the targets' PIDs.
int omv_i3c_daa(omv_i3c_t *i3c, uint8_t *list, uint64_t *pids, uint8_t size);
// Transfers with the omv_i2c transfer flags. Writes without a STOP are cached
// and sent with the next transfer, as one frame with a repeated START.
int omv_i3c_read(omv_i3c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags);
int omv_i3c_write(omv_i3c_t *i3c, uint8_t slv_addr, uint8_t *buf, uint32_t len, uint32_t flags);
// Writes are copied, so only rxbuf must stay valid until a DMA transfer completes.
int omv_i3c_transfer_start(omv_i3c_t *i3c, omv_i3c_transfer_t *xfer);
int omv_i3c_transfer_abort(omv_i3c_t *i3c);
// Calls the callback, from an IRQ, on the target's in-band interrupts once enabled.
int omv_i3c_ibi_register(omv_i3c_t *i3c, uint8_t slv_addr, omv_i3c_ibi_callback_t callback, void *data);
int omv_i3c_ibi_enable(omv_i3c_t *i3c, bool enable);

#endif // __OMV_I3C_H__
