/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2022-2024 OpenMV, LLC.
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
 * OpenMV Protocol USB Channel.
 */
#if OMV_USB_STACK_TINYUSB
#include "py/stream.h"
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "py/runtime.h"

#include "omv_common.h"
#include "cmsis_gcc.h"
#include "omv_protocol.h"
#include "omv_boardconfig.h"
#include "tusb.h"

#ifndef OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS
#define OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS (1500)
#endif

static bool usb_channel_active;

static size_t usb_channel_size(const omv_protocol_channel_t *channel) {
    if (tud_task_event_ready()) {
        tud_task_ext(0, false);
    }
    return tud_cdc_available();
}

static int usb_channel_flush(const omv_protocol_channel_t *channel) {
    if (tud_task_event_ready()) {
        tud_task_ext(0, false);
    }
    return tud_cdc_write_flush();
}

static bool usb_channel_is_active(const omv_protocol_channel_t *channel) {
    return usb_channel_active;
}

static int usb_channel_read(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, void *data) {
    size_t bytes = 0;
    uint32_t start_ms = mp_hal_ticks_ms();
    while (bytes < size && !check_timeout_ms(start_ms, OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS)) {
        bytes += tud_cdc_read((uint8_t *) data + bytes, size - bytes);
        if (tud_task_event_ready()) {
            tud_task_ext(0, false);
        }
        if (bytes < size) {
            mp_event_handle_nowait();
        }
    }

    return bytes;
}

static int usb_channel_write(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, const void *data) {
    size_t bytes = 0;
    uint32_t start_ms = mp_hal_ticks_ms();
    while (bytes < size && !check_timeout_ms(start_ms, OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS)) {
        bytes += tud_cdc_write((uint8_t *) data + bytes, size - bytes);
        if (tud_task_event_ready()) {
            tud_task_ext(0, false);
        }
        if (bytes < size) {
            mp_event_handle_nowait();
        }
    }

    return bytes;
}

static void usb_channel_task(mp_sched_node_t *node) {
    tud_task_ext(0, false);
    if (omv_protocol_is_active()) {
        omv_protocol_task();
    }
}

// Wrap MicroPython TinyUSB functions to run our own task.
void tud_cdc_rx_cb(uint8_t itf) {
    extern void __mp_tud_cdc_rx_cb(uint8_t itf);

    if (!omv_protocol_is_active()) {
        __mp_tud_cdc_rx_cb(itf);
    }
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding) {
    usb_channel_active = (coding->bit_rate == OMV_PROTOCOL_MAGIC_BAUDRATE);
}

void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts) {
    extern void __mp_tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts);

    cdc_line_coding_t coding;
    tud_cdc_get_line_coding(&coding);

    if (!dtr) {
        usb_channel_active = false;
    } else {
        usb_channel_active = (coding.bit_rate == OMV_PROTOCOL_MAGIC_BAUDRATE);
    }

    __mp_tud_cdc_line_state_cb(instance, dtr, rts);
}

void tud_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr) {
    static mp_sched_node_t usb_channel_node;
    extern void __mp_tud_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr);

    if (!omv_protocol_is_active()) {
        __mp_tud_event_hook_cb(rhport, eventid, in_isr);
    } else if (tud_task_event_ready()) {
        mp_sched_schedule_node(&usb_channel_node, usb_channel_task);
    }
}

// USB Channel
const omv_protocol_channel_t omv_usb_channel = {
    .priv = NULL,
    .id = OMV_PROTOCOL_CHANNEL_ID_TRANSPORT,
    .name = "usb",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_PHYSICAL,
    .size = usb_channel_size,
    .read = usb_channel_read,
    .write = usb_channel_write,
    .flush = usb_channel_flush,
    .is_active = usb_channel_is_active
};
#endif // OMV_USB_STACK_TINYUSB
