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
 * Tinyusb CDC debugger helper code.
 */

#include "omv_boardconfig.h"

#if OMV_TUSBDBG_ENABLE
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "pendsv.h"

#include "tusb.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"
#include "omv_common.h"
#include "cmsis_gcc.h"

#define USBDBG_DATA_TIMEOUT     (1000)
#define DEBUG_EP_SIZE           (TUD_OPT_HIGH_SPEED ? 512 : 64)

typedef struct {
    uint32_t timestamp;
    uint8_t opcode;
    uint32_t length;
    bool debug_mode;
    mp_sched_node_t sched_node;
    ringbuf_t ringbuf;
    uint8_t rawbuf[OMV_TUSBDBG_BUFFER];
} debug_context_t;

static debug_context_t ctx;

uint32_t usb_cdc_buf_len() {
    return ringbuf_avail(&ctx.ringbuf);
}

uint32_t usb_cdc_get_buf(uint8_t *buf, uint32_t len) {
    // Read all of the request bytes.
    if (ringbuf_get_bytes(&ctx.ringbuf, buf, len) == 0) {
        return len;
    }

    // Try to read as much data as possible.
    uint32_t bytes = 0;
    for (; bytes < len; bytes++) {
        int c = ringbuf_get(&ctx.ringbuf);
        if (c == -1) {
            break;
        }
        buf[bytes] = c;
    }

    return bytes;
}

void usb_cdc_reset_buffers(void) {
    ctx.ringbuf.iget = 0;
    ctx.ringbuf.iput = 0;
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding) {
    usb_cdc_reset_buffers();

    #if defined(MICROPY_BOARD_ENTER_BOOTLOADER)
    if (coding->bit_rate == 1200) {
        MICROPY_BOARD_ENTER_BOOTLOADER(0, 0);
    }
    #endif

    ctx.debug_mode = (coding->bit_rate == USBDBG_BAUDRATE_SLOW ||
                      coding->bit_rate == USBDBG_BAUDRATE_FAST);
}

// Wrapped MicroPython functions.
void __wrap_mp_usbd_task(void) {
    extern void __real_mp_usbd_task(void);

    if (!tinyusb_debug_enabled()) {
        __real_mp_usbd_task();
    }
}

void __wrap_tud_cdc_rx_cb(uint8_t itf) {
    extern void __real_tud_cdc_rx_cb(uint8_t itf);

    if (!tinyusb_debug_enabled()) {
        __real_tud_cdc_rx_cb(itf);
    }
}

uintptr_t __wrap_mp_hal_stdio_poll(uintptr_t poll_flags) {
    extern uintptr_t __real_mp_hal_stdio_poll(uintptr_t poll_flags);

    if (!tinyusb_debug_enabled()) {
        return __real_mp_hal_stdio_poll(poll_flags);
    }
    return 0;
}

mp_uint_t __wrap_mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    extern mp_uint_t __real_mp_hal_stdout_tx_strn(const char *str, mp_uint_t len);

    if (!tinyusb_debug_enabled()) {
        return __real_mp_hal_stdout_tx_strn(str, len);
    } else if (tud_cdc_connected()) {
        for (int i = 0; i < len; i++) {
            // On overflow, reset the ring buffer, if this string fits
            // entirely in the buffer, to recover from broken strings.
            if (ringbuf_put(&ctx.ringbuf, str[i]) == -1 && len <= ctx.ringbuf.size) {
                usb_cdc_reset_buffers();
                ringbuf_put(&ctx.ringbuf, str[i]);
            }
        }
    }
    return len;
}

int tinyusb_debug_init(void) {
    ctx.opcode = 0;
    ctx.length = 0;
    if (!ctx.ringbuf.buf) {
        ctx.ringbuf = (ringbuf_t) { ctx.rawbuf, sizeof(ctx.rawbuf), 0, 0 };
    }
    return 0;
}

bool tinyusb_debug_enabled(void) {
    return ctx.debug_mode;
}

void tinyusb_debug_task(mp_sched_node_t *node) {
    tud_task_ext(0, false);

    if (!tinyusb_debug_enabled() || !tud_cdc_connected()) {
        return;
    }

    if (!ctx.length && tud_cdc_available() >= USBDBG_HEADER_SIZE) {
        uint8_t cmdbuf[USBDBG_HEADER_SIZE];
        tud_cdc_read(cmdbuf, sizeof(cmdbuf));

        if (cmdbuf[0] == 0x30) {
            ctx.opcode = cmdbuf[1];
            ctx.length = *((uint32_t*)(cmdbuf+2));
            usbdbg_control(NULL, ctx.opcode, ctx.length);
        }

        if (ctx.length > 0) {
            ctx.timestamp = mp_hal_ticks_ms();
        }
    }

    while (ctx.length) {
        uint32_t bytes = 0;

        if (tud_task_event_ready()) {
            tud_task_ext(0, false);
        }
        
        if (ctx.opcode & 0x80) {
            bytes = OMV_MIN(ctx.length, tud_cdc_write_available());
            if (bytes) {
                ctx.length -= bytes;
                usbdbg_data_in(bytes, tud_cdc_write);
            }
            if (bytes > 0 && bytes < DEBUG_EP_SIZE) {
                tud_cdc_write_flush();
            }
        } else {
            bytes = OMV_MIN(ctx.length, tud_cdc_available());
            if (bytes) {
                ctx.length -= bytes;
                usbdbg_data_out(bytes, tud_cdc_read);
            }
        }
        
        if (bytes) {
            ctx.timestamp = mp_hal_ticks_ms();
        } else if (__get_PRIMASK() & 1) {
            break;
        } else if (check_timeout_ms(ctx.timestamp, USBDBG_DATA_TIMEOUT)) {
            tinyusb_debug_init();
        }
    } 
}

// For the mimxrt, and nrf ports this replaces the weak USB IRQ handlers.
// For the RP2 port, this handler is installed in main.c
void OMV_USB1_IRQ_HANDLER(void) {
    dcd_int_handler(0);
    // If there are any event to process, schedule a call to cdc loop.
    if (tud_task_event_ready()) {
        mp_sched_schedule_node(&ctx.sched_node, tinyusb_debug_task);
    }
}

#if defined(OMV_USB2_IRQ_HANDLER)
void OMV_USB2_IRQ_HANDLER(void) {
    dcd_int_handler(1);
    // If there are any event to process, schedule a call to cdc loop.
    if (tud_task_event_ready()) {
        mp_sched_schedule_node(&ctx.sched_node, tinyusb_debug_task);
    }
}
#endif

#endif //OMV_TINYUSB_DEBUG
