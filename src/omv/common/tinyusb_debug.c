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
#if (OMV_TUSBDBG_ENABLE == 1)
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "py/ringbuf.h"
#include "pendsv.h"

#include "tusb.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"
#include "omv_common.h"

#define DEBUG_BAUDRATE_SLOW     (921600)
#define DEBUG_BAUDRATE_FAST     (12000000)
#define DEBUG_EP_SIZE           (TUD_OPT_HIGH_SPEED ? 512 : 64)

void NORETURN __fatal_error(const char *msg);

typedef struct __attribute__((packed)) {
    uint8_t cmd;
    uint8_t request;
    uint32_t xfer_length;
}
usbdbg_cmd_t;

static uint8_t tx_array[OMV_TUSBDBG_BUFFER];
static ringbuf_t tx_ringbuf = { tx_array, sizeof(tx_array) };
static volatile bool tinyusb_debug_mode = false;

uint32_t usb_cdc_buf_len() {
    return ringbuf_avail(&tx_ringbuf);
}

uint32_t usb_cdc_get_buf(uint8_t *buf, uint32_t len) {
    // Read all of the request bytes.
    if (ringbuf_get_bytes(&tx_ringbuf, buf, len) == 0) {
        return len;
    }
    // Try to read as much data as possible.
    uint32_t bytes = 0;
    for (; bytes < len; bytes++) {
        int c = ringbuf_get(&tx_ringbuf);
        if (c == -1) {
            break;
        }
        buf[bytes] = c;
    }
    return bytes;
}

void usb_cdc_reset_buffers() {

}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding) {
    tx_ringbuf.iget = 0;
    tx_ringbuf.iput = 0;

    if (0) {
        #if defined(MICROPY_BOARD_ENTER_BOOTLOADER)
    } else if (coding->bit_rate == 1200) {
        MICROPY_BOARD_ENTER_BOOTLOADER(0, 0);
        #endif
    } else if (coding->bit_rate == DEBUG_BAUDRATE_SLOW
               || coding->bit_rate == DEBUG_BAUDRATE_FAST) {
        tinyusb_debug_mode = true;
    } else {
        tinyusb_debug_mode = false;
    }
}

bool tinyusb_debug_enabled(void) {
    return tinyusb_debug_mode;
}

extern void __real_mp_usbd_task(void);
void __wrap_mp_usbd_task(void) {
    if (!tinyusb_debug_enabled()) {
        __real_mp_usbd_task();
    }
}

extern void __real_tud_cdc_rx_cb(uint8_t itf);
void __wrap_tud_cdc_rx_cb(uint8_t itf) {
    if (!tinyusb_debug_enabled()) {
        __real_tud_cdc_rx_cb(itf);
    }
}

extern uintptr_t __real_mp_hal_stdio_poll(uintptr_t poll_flags);
uintptr_t __wrap_mp_hal_stdio_poll(uintptr_t poll_flags) {
    if (!tinyusb_debug_enabled()) {
        return __real_mp_hal_stdio_poll(poll_flags);
    }
    return 0;
}

extern mp_uint_t __real_mp_hal_stdout_tx_strn(const char *str, mp_uint_t len);
mp_uint_t __wrap_mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    if (tinyusb_debug_enabled()) {
        if (tud_cdc_connected()) {
            NVIC_DisableIRQ(PendSV_IRQn);
            for (int i = 0; i < len; i++) {
                // The ring buffer overflows occasionally, espcially when using a slow poll
                // rate and fast print rate. When this happens, reset the buffer and start
                // over, if this string fits entirely in the buffer. This helps the ring buffer
                // self-recover from broken strings.
                if (ringbuf_put(&tx_ringbuf, str[i]) == -1 && len <= tx_ringbuf.size) {
                    tx_ringbuf.iget = 0;
                    tx_ringbuf.iput = 0;
                }
            }
            NVIC_EnableIRQ(PendSV_IRQn);
        }
        return len;
    } else {
        return __real_mp_hal_stdout_tx_strn(str, len);
    }
}

static void tinyusb_debug_task(void) {
    tud_task_ext(0, false);

    if (!tinyusb_debug_enabled() || !tud_cdc_connected() || tud_cdc_available() < 6) {
        return;
    }

    usbdbg_cmd_t cmd;
    tud_cdc_read(&cmd, sizeof(cmd));

    if (cmd.cmd != 0x30) {
        // TODO: Try to recover from this state but for now, call __fatal_error.
        __fatal_error("Invalid USB CMD received.");
    }

    uint8_t request = cmd.request;
    uint32_t xfer_length = cmd.xfer_length;
    usbdbg_control(NULL, request, xfer_length);

    while (xfer_length) {
        // && tud_cdc_connected())
        if (tud_task_event_ready()) {
            tud_task_ext(0, false);
        }
        if (request & 0x80) {
            // Device-to-host data phase
            int size = OMV_MIN(xfer_length, tud_cdc_write_available());
            if (size) {
                xfer_length -= size;
                usbdbg_data_in(size, tud_cdc_write);
            }
            if (size > 0 && size < DEBUG_EP_SIZE) {
                tud_cdc_write_flush();
            }
        } else {
            // Host-to-device data phase
            int size = OMV_MIN(xfer_length, tud_cdc_available());
            if (size) {
                xfer_length -= size;
                usbdbg_data_out(size, tud_cdc_read);
            }
        }
    }
}

// For the mimxrt, and nrf ports this replaces the weak USB IRQ handlers.
// For the RP2 port, this handler is installed in main.c
void OMV_USB1_IRQ_HANDLER(void) {
    dcd_int_handler(0);
    // If there are any event to process, schedule a call to cdc loop.
    if (tud_task_event_ready()) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_CDC, tinyusb_debug_task);
    }
}

#if defined(OMV_USB2_IRQ_HANDLER)
void OMV_USB2_IRQ_HANDLER(void) {
    dcd_int_handler(1);
    // If there are any event to process, schedule a call to cdc loop.
    if (tud_task_event_ready()) {
        pendsv_schedule_dispatch(PENDSV_DISPATCH_CDC, tinyusb_debug_task);
    }
}
#endif

#endif //OMV_TINYUSB_DEBUG
