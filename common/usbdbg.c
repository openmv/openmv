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
 * USB debugger.
 */
#include <string.h>
#include <stdio.h>
#include "py/nlr.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/runtime.h"

#include "imlib.h"
#if MICROPY_PY_CSI || MICROPY_PY_CSI_NG
#include "omv_i2c.h"
#include "omv_csi.h"
#endif
#include "framebuffer.h"
#include "usbdbg.h"
#include "tinyusb_debug.h"
#include "omv_boardconfig.h"
#include "py_image.h"

static int xfer_offs;
static int xfer_size;
static enum usbdbg_cmd cmd;

static volatile bool script_ready;
static volatile bool script_running;
static volatile bool irq_enabled;
static vstr_t script_buf;

// These functions must be implemented by the stack.
extern uint32_t usb_cdc_buf_len();
extern uint32_t usb_cdc_get_buf(uint8_t *buf, uint32_t len);
extern void usb_cdc_reset_buffers(void);

void usbdbg_init() {
    cmd = USBDBG_NONE;
    script_ready = false;
    script_running = false;
    irq_enabled = false;
    vstr_init(&script_buf, 32);
    #if OMV_TUSBDBG_ENABLE
    tinyusb_debug_init();
    #endif
    #if OMV_PROFILER_ENABLE
    omv_profiler_init();
    #endif
}

bool usbdbg_script_ready() {
    return script_ready;
}

vstr_t *usbdbg_get_script() {
    return &script_buf;
}

bool usbdbg_is_busy() {
    return cmd != USBDBG_NONE;
}

void usbdbg_set_script_running(bool running) {
    script_running = running;
}

inline void usbdbg_set_irq_enabled(bool enabled) {
    if (enabled) {
        NVIC_EnableIRQ(OMV_USB_IRQN);
    } else {
        NVIC_DisableIRQ(OMV_USB_IRQN);
    }
    __DSB(); __ISB();
    irq_enabled = enabled;
}

static void usbdbg_interrupt_vm(bool ready) {
    // Set script ready flag
    script_ready = ready;

    // Set script running flag
    script_running = ready;

    // Disable IDE IRQ (re-enabled by pyexec or in main).
    usbdbg_set_irq_enabled(false);

    // Abort the VM.
    mp_sched_vm_abort();

    // When the VM runs again it will raise a KeyboardInterrupt.
    mp_sched_keyboard_interrupt();
}

bool usbdbg_get_irq_enabled() {
    return irq_enabled;
}

void usbdbg_data_in(uint32_t size, usbdbg_write_callback_t write_callback) {
    switch (cmd) {
        case USBDBG_FW_VERSION: {
            uint32_t buffer[3] = {
                FIRMWARE_VERSION_MAJOR,
                FIRMWARE_VERSION_MINOR,
                FIRMWARE_VERSION_PATCH,
            };
            cmd = USBDBG_NONE;
            write_callback(&buffer, sizeof(buffer));
            break;
        }

        case USBDBG_TX_BUF_LEN: {
            uint32_t tx_buf_len = usb_cdc_buf_len();
            cmd = USBDBG_NONE;
            write_callback(&tx_buf_len, sizeof(tx_buf_len));
            break;
        }

        case USBDBG_SENSOR_ID: {
            uint32_t buffer = 0xFF;
            #if MICROPY_PY_CSI
            omv_csi_t *csi = omv_csi_get(-1);
            if (omv_csi_is_detected(csi) == true) {
                buffer = omv_csi_get_id(csi);
            }
            #endif
            cmd = USBDBG_NONE;
            write_callback(&buffer, sizeof(buffer));
            break;
        }

        case USBDBG_TX_BUF: {
            uint8_t buffer[size];
            size = usb_cdc_get_buf(buffer, size);
            xfer_offs += size;
            if (xfer_offs == xfer_size) {
                cmd = USBDBG_NONE;
            }
            if (size) {
                write_callback(buffer, size);
            }
            break;
        }

        case USBDBG_FRAME_SIZE: {
            // Return 0 if FB is locked or not ready.
            uint32_t buffer[3] = { 0 };
            // Try to lock FB. If header size == 0 frame is not ready
            framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
            if (mutex_try_lock(&fb->lock, MUTEX_TID_IDE)) {
                // If header size == 0 frame is not ready
                if (fb->size == 0) {
                    // unlock FB
                    mutex_unlock(&fb->lock, MUTEX_TID_IDE);
                } else {
                    // Return header w, h and size/bpp
                    buffer[0] = fb->w;
                    buffer[1] = fb->h;
                    buffer[2] = fb->size;
                }
            }
            cmd = USBDBG_NONE;
            write_callback(&buffer, sizeof(buffer));
            break;
        }

        case USBDBG_FRAME_DUMP:
            if (xfer_offs < xfer_size) {
                framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
                write_callback(fb->raw_base + sizeof(framebuffer_header_t) + xfer_offs, size);
                xfer_offs += size;
                if (xfer_offs == xfer_size) {
                    cmd = USBDBG_NONE;
                    fb->w = 0; fb->h = 0; fb->size = 0;
                    mutex_unlock(&fb->lock, MUTEX_TID_IDE);
                }
            }
            break;

        case USBDBG_ARCH_STR: {
            uint8_t buffer[64];
            unsigned int uid[3] = {
                #if (OMV_BOARD_UID_SIZE == 2)
                0U,
                #else
                *((unsigned int *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 2)),
                #endif
                *((unsigned int *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 1)),
                *((unsigned int *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 0)),
            };
            snprintf((char *) buffer, 64, "%s [%s:%08X%08X%08X]",
                     OMV_BOARD_ARCH, OMV_BOARD_TYPE, uid[0], uid[1], uid[2]);
            cmd = USBDBG_NONE;
            write_callback(&buffer, sizeof(buffer));
            break;
        }

        case USBDBG_SCRIPT_RUNNING: {
            cmd = USBDBG_NONE;
            uint32_t buffer = script_running;
            write_callback(&buffer, sizeof(buffer));
            break;
        }

        case USBDBG_GET_STATE: {
            // IDE will request 63/511 bytes of data to avoid ZLP packets.
            // 64/512 packets still work too but generate ZLP packets.
            size = OMV_MIN(size, 512);
            uint8_t byte_buffer[size];
            memset(byte_buffer, 0, size);
            uint32_t *buffer = (uint32_t *) byte_buffer;
            static uint32_t last_update_ms = 0;

            // Set script running flag
            if (script_running) {
                buffer[0] |= USBDBG_FLAG_SCRIPT_RUNNING;
            }

            // Set text buf valid flag.
            uint32_t tx_buf_len = usb_cdc_buf_len();
            if (tx_buf_len) {
                buffer[0] |= USBDBG_FLAG_TEXTBUF_NOTEMPTY;
            }

            // Set code profiling flags
            #if OMV_PROFILER_ENABLE
            buffer[0] |= USBDBG_FLAG_PROFILE_ENABLED;
            #if __PMU_PRESENT
            buffer[0] |= USBDBG_FLAG_PROFILE_HAS_PMU;
            #endif // __PMU_PRESENT
            #endif // OMV_PROFILER_ENABLE

            // Limit the frames sent over USB to 20Hz.
            framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
            if (check_timeout_ms(last_update_ms, 50) &&
                mutex_try_lock_fair(&fb->lock, MUTEX_TID_IDE)) {
                // If header size == 0 frame is not ready
                if (fb->size == 0) {
                    // unlock FB
                    mutex_unlock(&fb->lock, MUTEX_TID_IDE);
                } else {
                    // Set valid frame flag.
                    buffer[0] |= USBDBG_FLAG_FRAMEBUF_LOCKED;

                    // Set frame width, height and size/bpp
                    buffer[1] = fb->w;
                    buffer[2] = fb->h;
                    buffer[3] = fb->size;
                    last_update_ms = mp_hal_ticks_ms();
                }
            }

            // The rest of this packet is packed with text buffer.
            if (tx_buf_len) {
                const uint32_t hdr = 16;
                tx_buf_len = OMV_MIN(tx_buf_len, (size - hdr - 1));
                usb_cdc_get_buf(byte_buffer + hdr, tx_buf_len);
                byte_buffer[hdr + tx_buf_len] = 0; // Null-terminate
            }

            cmd = USBDBG_NONE;
            write_callback(&byte_buffer, size);
            break;
        }

        case USBDBG_PROFILE_SIZE: {
            // Return 0 if the profiling data is locked.
            uint32_t buffer[3] = { 0 };
            #if OMV_PROFILER_ENABLE
            if (mutex_try_lock(omv_profiler_lock(), MUTEX_TID_IDE)) {
                size_t size = omv_profiler_get_size();
                buffer[0] = size / sizeof(omv_profiler_data_t);
                buffer[1] = sizeof(omv_profiler_data_t);
                buffer[2] = __PMU_NUM_EVENTCNT;
            }
            #endif
            cmd = USBDBG_NONE;
            write_callback(&buffer, sizeof(buffer));
            break;
        }

        #if OMV_PROFILER_ENABLE
        case USBDBG_PROFILE_DUMP:
            if (xfer_offs < xfer_size) {
                const char *data = omv_profiler_get_data();
                write_callback(data + xfer_offs, size);
                xfer_offs += size;
                if (xfer_offs == xfer_size) {
                    cmd = USBDBG_NONE;
                    mutex_unlock(omv_profiler_lock(), MUTEX_TID_IDE);
                }
            }
            break;
        #endif

        default: /* error */
            break;
    }
}

void usbdbg_data_out(uint32_t size, usbdbg_read_callback_t read_callback) {
    switch (cmd) {
        case USBDBG_FB_ENABLE: {
            uint32_t enabled = 0;
            framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
            read_callback(&enabled, 4);
            framebuffer_set_enabled(fb, enabled);
            if (fb->enabled == 0) {
                // When disabling framebuffer, the IDE might still be holding FB lock.
                // If the IDE is not the current lock owner, this operation is ignored.
                mutex_unlock(&fb->lock, MUTEX_TID_IDE);
            }
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SCRIPT_EXEC:
            // check if GC is locked before allocating memory for vstr. If GC was locked
            // at least once before the script is fully uploaded xfer_offs will be less
            // than the total size (xfer_size) and the script will Not be executed.
            if (!script_running) {
                nlr_buf_t nlr;
                if (!gc_is_locked() && nlr_push(&nlr) == 0) {
                    char *buffer = vstr_add_len(&script_buf, size);
                    read_callback(buffer, size);
                    nlr_pop();
                }
                xfer_offs += size;
                if (xfer_offs == xfer_size) {
                    cmd = USBDBG_NONE;
                    // Schedule the IDE exception to interrupt the VM.
                    usbdbg_interrupt_vm(true);
                }
            }
            break;

        case USBDBG_PROFILE_MODE: {
            uint32_t buffer[1];
            read_callback(&buffer, sizeof(buffer));
            #if OMV_PROFILER_ENABLE
            omv_profiler_set_mode(buffer[0]);
            #endif
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_PROFILE_EVENT: {
            uint32_t buffer[2];
            read_callback(&buffer, sizeof(buffer));
            #if OMV_PROFILER_ENABLE
            omv_profiler_set_event(buffer[0], buffer[1]);
            #endif
            cmd = USBDBG_NONE;
            break;
        }

        default: /* error */
            size = OMV_MIN(size, 512);
            uint8_t byte_buffer[size];
            read_callback(&byte_buffer, size);
            break;
    }
}

void usbdbg_control(void *buffer, uint8_t request, uint32_t size) {
    cmd = (enum usbdbg_cmd) request;
    switch (cmd) {
        case USBDBG_FW_VERSION:
        case USBDBG_FRAME_SIZE:
        case USBDBG_FRAME_DUMP:
        case USBDBG_ARCH_STR:
        case USBDBG_FB_ENABLE:
        case USBDBG_TX_BUF:
        case USBDBG_TX_BUF_LEN:
        case USBDBG_SENSOR_ID:
        case USBDBG_GET_STATE:
        case USBDBG_PROFILE_SIZE:
        case USBDBG_PROFILE_DUMP:
        case USBDBG_PROFILE_MODE:
        case USBDBG_PROFILE_EVENT:
        case USBDBG_SCRIPT_EXEC:
            xfer_offs = 0;
            xfer_size = size;
            break;

        case USBDBG_PROFILE_RESET: {
            #if OMV_PROFILER_ENABLE
            omv_profiler_reset();
            #endif
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SCRIPT_RUNNING:
            vstr_reset(&script_buf);
            xfer_offs = 0;
            xfer_size = size;
            break;
        
        case USBDBG_SCRIPT_STOP:
            if (script_running) {
                // Reset CDC buffers.
                usb_cdc_reset_buffers();

                // Schedule the IDE exception to interrupt the VM.
                usbdbg_interrupt_vm(false);
            }
            cmd = USBDBG_NONE;
            break;

        case USBDBG_SYS_RESET:
            #if defined(OMV_BOARD_RESET)
            OMV_BOARD_RESET();
            #else
            NVIC_SystemReset();
            #endif
            break;

        case USBDBG_SYS_RESET_TO_BL: {
            #if defined(MICROPY_BOARD_ENTER_BOOTLOADER)
            MICROPY_BOARD_ENTER_BOOTLOADER(0, 0);
            #else
            NVIC_SystemReset();
            #endif
            break;
        }

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}
