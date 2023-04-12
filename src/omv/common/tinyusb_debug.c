/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2022 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2022 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Tinyusb CDC debugger helper code.
 */

#include "omv_boardconfig.h"
#if (OMV_ENABLE_TUSBDBG == 1)
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "pendsv.h"

#include "tusb.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"

#define DEBUG_MAX_PACKET        (OMV_TUSBDBG_PACKET)
#define DEBUG_BAUDRATE_SLOW     (921600)
#define DEBUG_BAUDRATE_FAST     (12000000)

extern void __fatal_error();

typedef struct __attribute__((packed)) {
    uint8_t cmd;
    uint8_t request;
    uint32_t xfer_length;
} usbdbg_cmd_t;

static uint8_t debug_ringbuf_array[512];
static volatile bool  tinyusb_debug_mode = false;
ringbuf_t debug_ringbuf = { debug_ringbuf_array, sizeof(debug_ringbuf_array) };

uint32_t usb_cdc_buf_len()
{
    return ringbuf_avail((ringbuf_t*) &debug_ringbuf);
}

uint32_t usb_cdc_get_buf(uint8_t *buf, uint32_t len)
{
    for (int i=0; i<len; i++) {
        buf[i] = ringbuf_get((ringbuf_t*) &debug_ringbuf);
    }
    return len;
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding)
{
    debug_ringbuf.iget = 0;
    debug_ringbuf.iput = 0;

    if (0) {
    #if defined(MICROPY_RESET_TO_BOOTLOADER)
    } else if (coding->bit_rate == 1200) {
        MICROPY_RESET_TO_BOOTLOADER();
    #endif
    } else if (coding->bit_rate == DEBUG_BAUDRATE_SLOW
            || coding->bit_rate == DEBUG_BAUDRATE_FAST) {
        tinyusb_debug_mode = true;
    } else {
        tinyusb_debug_mode = false;
    }
}

bool tinyusb_debug_enabled(void)
{
    return tinyusb_debug_mode;
}

void tinyusb_debug_tx_strn(const char *str, mp_uint_t len)
{
    // TODO can be faster.
    if (tinyusb_debug_enabled() && tud_cdc_connected()) {
        for (int i=0; i<len; i++) {
            NVIC_DisableIRQ(PendSV_IRQn);
            ringbuf_put((ringbuf_t*)&debug_ringbuf, str[i]);
            NVIC_EnableIRQ(PendSV_IRQn);
        }
    }
}

static void tinyusb_debug_task(void)
{
    tud_task();

    uint8_t dbg_buf[DEBUG_MAX_PACKET];
    if (tud_cdc_connected() && tud_cdc_available() >= 6) {
        uint32_t count = tud_cdc_read(dbg_buf, 6);
        if (count < 6 || dbg_buf[0] != 0x30) {
            // Maybe we should try to recover from this state
            // but for now, call __fatal_error which doesn't
            // return.
            __fatal_error();
            return;
        }
        usbdbg_cmd_t *cmd = (usbdbg_cmd_t *) dbg_buf;
        uint8_t request = cmd->request;
        uint32_t xfer_length = cmd->xfer_length;
        usbdbg_control(NULL, request, xfer_length);

        while (xfer_length) {   // && tud_cdc_connected())
            if (tud_task_event_ready()) {
                tud_task();
            }
            if (request & 0x80) {
                // Device-to-host data phase
                int bytes = MIN(xfer_length, DEBUG_MAX_PACKET);
                if (bytes <= tud_cdc_write_available()) {
                    xfer_length -= bytes;
                    usbdbg_data_in(dbg_buf, bytes);
                    tud_cdc_write(dbg_buf, bytes);
                }
                tud_cdc_write_flush();
            } else {
                // Host-to-device data phase
                int bytes = MIN(xfer_length, DEBUG_MAX_PACKET);
                uint32_t count = tud_cdc_read(dbg_buf, bytes);
                if (count == bytes) {
                    xfer_length -= count;
                    usbdbg_data_out(dbg_buf, count);
                }
            }
        }
    }
}

// For the mimxrt, and nrf ports this replaces the weak USB IRQ handlers.
// For the RP2 port, this handler is installed in main.c
void OMV_USB1_IRQ_HANDLER(void)
{
    dcd_int_handler(0);
    // If there are any event to process, schedule a call to cdc loop.
    if (tinyusb_debug_enabled()) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_CDC, tinyusb_debug_task);
    }
}

#if defined(OMV_USB2_IRQ_HANDLER)
void OMV_USB2_IRQ_HANDLER(void)
{
    dcd_int_handler(1);
    // If there are any event to process, schedule a call to cdc loop.
    if (tinyusb_debug_enabled()) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_CDC, tinyusb_debug_task);
    }
}
#endif

#endif //OMV_TINYUSB_DEBUG
