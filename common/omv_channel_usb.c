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
#include "py/stream.h"
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "py/runtime.h"

#include "omv_common.h"
#include "cmsis_gcc.h"
#include "omv_protocol.h"
#include "omv_boardconfig.h"
#if OMV_TUSBDBG_ENABLE
#include "tusb.h"
#else
#undef MIN
#undef MAX
#include "usbd_cdc_msc_hid.h"
#include "usbd_cdc_interface.h"
#include "usb.h"
#endif

#if OMV_TUSBDBG_ENABLE
static int usb_channel_init(const omv_protocol_channel_t *channel) {
    return 0;
}

static size_t usb_channel_size(const omv_protocol_channel_t *channel) {
    return tud_cdc_available();
}

static int usb_channel_flush(const omv_protocol_channel_t *channel) {
    return tud_cdc_write_flush();
}

static int usb_channel_phy_read(const omv_protocol_channel_t *channel,
                              size_t size, void *data, uint32_t timeout_ms) {
    return tud_cdc_read(data, size);
}

static int usb_channel_phy_write(const omv_protocol_channel_t *channel,
                               size_t size, const void *data, uint32_t timeout_ms) {
    return tud_cdc_write(data, size);
}

static void usb_channel_task(mp_sched_node_t *node) {
    tud_task_ext(0, false);
    if (omv_protocol_active() && tud_cdc_connected()) {
        omv_protocol_task();
    }
}

// Wrap MicroPython TinyUSB functions to run our own task.
void __real_tud_cdc_rx_cb(uint8_t itf);
void __wrap_tud_cdc_rx_cb(uint8_t itf) {
    if (!omv_protocol_active()) {
        __real_tud_cdc_rx_cb(itf);
    }
}


void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding) {
    if (coding->bit_rate == OMV_PROTOCOL_MAGIC_BAUDRATE) {
        omv_protocol_init(NULL, NULL);
    }
}

void __real_tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts);
void __wrap_tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts) {
    cdc_line_coding_t coding;
    tud_cdc_get_line_coding(&coding);

    if (!dtr) {
        omv_protocol_deinit();
    } else if (coding.bit_rate == OMV_PROTOCOL_MAGIC_BAUDRATE) {
        omv_protocol_init(NULL, NULL);
    }

    __real_tud_cdc_line_state_cb(instance, dtr, rts);
}

// For the mimxrt, and nrf ports this replaces the weak USB IRQ handlers.
// For the RP2 port, this handler is installed in main.c
void OMV_USB1_IRQ_HANDLER(void) {
    static mp_sched_node_t usb_channel_sched;

    dcd_int_handler(0);
    // If there are any event to process, schedule a call to channel task.
    if (tud_task_event_ready()) {
        mp_sched_schedule_node(&usb_channel_sched, usb_channel_task);
    }
}

#ifdef OMV_USB2_IRQ_HANDLER
void OMV_USB2_IRQ_HANDLER(void) {
    static mp_sched_node_t usb_channel_sched;

    dcd_int_handler(1);
    // If there are any event to process, schedule a call to channel task.
    if (tud_task_event_ready()) {
        mp_sched_schedule_node(&usb_channel_sched, usb_channel_task);
    }
}
#endif  // OMV_USB2_IRQ_HANDLER
#else   // STM stack
static int usb_channel_init(const omv_protocol_channel_t *channel) {
    return 0;
}

static size_t usb_channel_size(const omv_protocol_channel_t *channel) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    return usbd_cdc_rx_num(cdc);
}

static int usb_channel_flush(const omv_protocol_channel_t *channel) {
    return 0;
}

static int usb_channel_phy_read(const omv_protocol_channel_t *channel,
                              size_t size, void *data, uint32_t timeout_ms) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    return usbd_cdc_rx(cdc, data, size, timeout_ms);
}

static int usb_channel_phy_write(const omv_protocol_channel_t *channel,
                               size_t size, const void *data, uint32_t timeout_ms) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    return usbd_cdc_tx(cdc, data, size, timeout_ms);
}

static void usb_channel_task(mp_sched_node_t *node) {
    if (omv_protocol_active()) {
        omv_protocol_task();
    }
}

int __real_mp_os_dupterm_rx_chr(void);
int __wrap_mp_os_dupterm_rx_chr(void) {
    if (omv_protocol_active()) {
        return -1;
    }
    return __real_mp_os_dupterm_rx_chr();
}

int8_t __real_usbd_cdc_receive(usbd_cdc_state_t *cdc_in, size_t len);
int8_t __wrap_usbd_cdc_receive(usbd_cdc_state_t *cdc_in, size_t len) {
    static mp_sched_node_t usb_channel_sched;
    int8_t ret = __real_usbd_cdc_receive(cdc_in, len);
    if (omv_protocol_active()) {
        mp_sched_schedule_node(&usb_channel_sched, usb_channel_task);
    }
    return ret;
}

int8_t __real_usbd_cdc_control(usbd_cdc_state_t *cdc_in, uint8_t cmd, uint8_t *pbuf, uint16_t length);
int8_t __wrap_usbd_cdc_control(usbd_cdc_state_t *cdc_in, uint8_t cmd, uint8_t *pbuf, uint16_t length) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    int8_t ret = __real_usbd_cdc_control(cdc_in, cmd, pbuf, length);

    if (cdc->bitrate != OMV_PROTOCOL_MAGIC_BAUDRATE) {
        // Reattach to REPL
        cdc->attached_to_repl = true;
        cdc->flow &= ~USBD_CDC_FLOWCONTROL_CTS;
        omv_protocol_deinit();
    } else {
        // Detach from REPL
        cdc->attached_to_repl = false;
        cdc->flow |= USBD_CDC_FLOWCONTROL_RTS | USBD_CDC_FLOWCONTROL_CTS;
        omv_protocol_init(NULL, NULL);
    }

    return ret;
}
#endif  // OMV_TINYUSB_DEBUG

// USB Channel
const omv_protocol_channel_t omv_usb_channel = {
    .id = OMV_PROTOCOL_CHANNEL_ID_TRANSPORT,
    .name = "usb",
    .flags = OMV_PROTOCOL_CHANNEL_FLAG_PHYSICAL,
    .priv = NULL,
    .init = usb_channel_init,
    .deinit = NULL,
    .lock = NULL,
    .unlock = NULL,
    .size = usb_channel_size,
    .shape = NULL,
    .read = NULL,
    .write = NULL,
    .readp = NULL,
    .flush = usb_channel_flush,
    .ioctl = NULL,
    .phy_read = usb_channel_phy_read,
    .phy_write = usb_channel_phy_write
};
