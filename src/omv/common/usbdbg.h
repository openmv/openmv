/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * USB debug support.
 */
#ifndef __USBDBG_H__
#define __USBDBG_H__

/**
 * Firmware version (major, minor and patch numbers).
 *
 */
#define FIRMWARE_VERSION_MAJOR    (4)
#define FIRMWARE_VERSION_MINOR    (6)
#define FIRMWARE_VERSION_PATCH    (0)

/**
 * To add a new debugging command, increment the last command value used.
 * Set the MSB of the value if the request has a device-to-host data phase.
 * Add the command to usr/openmv.py using the same value.
 * Handle the command control and data in/out (if any) phases in usbdbg.c.
 *
 * See usbdbg.c for examples.
 */
enum usbdbg_cmd {
    USBDBG_NONE            = 0x00,
    USBDBG_FW_VERSION      = 0x80,
    USBDBG_FRAME_SIZE      = 0x81,
    USBDBG_FRAME_DUMP      = 0x82,
    USBDBG_ARCH_STR        = 0x83,
    USBDBG_SCRIPT_EXEC     = 0x05,
    USBDBG_SCRIPT_STOP     = 0x06,
    USBDBG_SCRIPT_SAVE     = 0x07,
    USBDBG_SCRIPT_RUNNING  = 0x87,
    USBDBG_TEMPLATE_SAVE   = 0x08,
    USBDBG_DESCRIPTOR_SAVE = 0x09,
    USBDBG_ATTR_READ       = 0x8A,
    USBDBG_ATTR_WRITE      = 0x0B,
    USBDBG_SYS_RESET       = 0x0C,
    USBDBG_SYS_RESET_TO_BL = 0x0E,
    USBDBG_FB_ENABLE       = 0x0D,
    USBDBG_TX_BUF_LEN      = 0x8E,
    USBDBG_TX_BUF          = 0x8F,
    USBDBG_SENSOR_ID       = 0x90,
    USBDBG_TX_INPUT        = 0x11,
    USBDBG_SET_TIME        = 0x12,
    USBDBG_GET_STATE       = 0x93,
};

enum usbdbg_state_flags {
    USBDBG_STATE_FLAGS_SCRIPT   = (1 << 0),
    USBDBG_STATE_FLAGS_TEXT     = (1 << 1),
    USBDBG_STATE_FLAGS_FRAME    = (1 << 2),
};

typedef uint32_t (*usbdbg_read_callback_t) (void *buf, uint32_t len);
typedef uint32_t (*usbdbg_write_callback_t) (const void *buf, uint32_t len);

void usbdbg_init();
void usbdbg_wait_for_command(uint32_t timeout);
bool usbdbg_script_ready();
vstr_t *usbdbg_get_script();
bool usbdbg_is_busy();
bool usbdbg_get_irq_enabled();
void usbdbg_set_irq_enabled(bool enabled);
void usbdbg_set_script_running(bool running);
void usbdbg_data_in(uint32_t size, usbdbg_write_callback_t write_callback);
void usbdbg_data_out(uint32_t size, usbdbg_read_callback_t read_callback);
void usbdbg_control(void *buffer, uint8_t brequest, uint32_t wlength);

#endif /* __USBDBG_H__ */
