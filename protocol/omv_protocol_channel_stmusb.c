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
#if OMV_USB_STACK_STMUSB
#include "py/stream.h"
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "py/runtime.h"

#include "omv_common.h"
#include "cmsis_gcc.h"
#include "omv_protocol.h"
#include "omv_boardconfig.h"
#undef MIN
#undef MAX
#include "usbd_cdc_msc_hid.h"
#include "usbd_cdc_interface.h"

#ifndef OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS
#define OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS (1500)
#endif

static bool usb_channel_active = false;

static size_t usb_channel_size(const omv_protocol_channel_t *channel) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    return usbd_cdc_rx_num(cdc);
}

static bool usb_channel_is_active(const omv_protocol_channel_t *channel) {
    return usb_channel_active;
}

static int usb_channel_read(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, void *data) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    return usbd_cdc_rx(cdc, data, size, OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS);
}

static int usb_channel_write(const omv_protocol_channel_t *channel, uint32_t offset, size_t size, const void *data) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    return usbd_cdc_tx(cdc, data, size, OMV_PROTOCOL_USB_CHANNEL_TIMEOUT_MS);
}

static void usb_channel_task(mp_sched_node_t *node) {
    if (usb_channel_active) {
        omv_protocol_task();
    }
}

int __real_mp_os_dupterm_rx_chr(void);
int __wrap_mp_os_dupterm_rx_chr(void) {
    if (usb_channel_active) {
        return -1;
    }
    return __real_mp_os_dupterm_rx_chr();
}

int8_t __real_usbd_cdc_receive(usbd_cdc_state_t *cdc_in, size_t len);
int8_t __wrap_usbd_cdc_receive(usbd_cdc_state_t *cdc_in, size_t len) {
    static mp_sched_node_t usb_channel_node;

    // Ensure the CDC is detached from REPL, if the protocol is active,
    // before calling receive as it raises exceptions on interrupt chars.
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    if (usb_channel_active && cdc->attached_to_repl) {
        cdc->attached_to_repl = false;
        cdc->flow |= USBD_CDC_FLOWCONTROL_RTS | USBD_CDC_FLOWCONTROL_CTS;
    }

    int8_t ret = __real_usbd_cdc_receive(cdc_in, len);
    if (usb_channel_active) {
        mp_sched_schedule_node(&usb_channel_node, usb_channel_task);
    }
    return ret;
}

int8_t __real_usbd_cdc_control(usbd_cdc_state_t *cdc_in, uint8_t cmd, uint8_t *pbuf, uint16_t length);
int8_t __wrap_usbd_cdc_control(usbd_cdc_state_t *cdc_in, uint8_t cmd, uint8_t *pbuf, uint16_t length) {
    usbd_cdc_itf_t *cdc = usb_vcp_get(0);
    int8_t ret = __real_usbd_cdc_control(cdc_in, cmd, pbuf, length);

    if (cdc->bitrate != OMV_PROTOCOL_MAGIC_BAUDRATE ||
        cdc->connect_state == USBD_CDC_CONNECT_STATE_DISCONNECTED) {
        // Reattach to REPL
        cdc->rx_buf_put = 0;
        cdc->rx_buf_get = 0;
        cdc->rx_buf_full = false;
        cdc->tx_need_empty_packet = 0;
        cdc->attached_to_repl = true;
        cdc->flow &= ~USBD_CDC_FLOWCONTROL_CTS;
    } else if (cdc->bitrate == OMV_PROTOCOL_MAGIC_BAUDRATE) {
        // Detach from REPL
        cdc->attached_to_repl = false;
        cdc->flow |= USBD_CDC_FLOWCONTROL_RTS | USBD_CDC_FLOWCONTROL_CTS;
    }

    usb_channel_active = !cdc->attached_to_repl;

    return ret;
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
    .is_active = usb_channel_is_active
};
#endif // OMV_USB_STACK_STM
