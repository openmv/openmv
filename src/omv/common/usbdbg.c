/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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
#include "pendsv.h"

#include "imlib.h"
#if MICROPY_PY_SENSOR
#include "omv_i2c.h"
#include "sensor.h"
#endif
#include "framebuffer.h"
#include "usbdbg.h"
#include "omv_boardconfig.h"
#include "py_image.h"

static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;

static volatile bool script_ready;
static volatile bool script_running;
static volatile bool irq_enabled;
static vstr_t script_buf;

static mp_obj_exception_t ide_exception;
static const MP_DEFINE_STR_OBJ(ide_exception_msg, "IDE interrupt");
static const mp_rom_obj_tuple_t ide_exception_args_obj = {
    {&mp_type_tuple}, 1, {MP_ROM_PTR(&ide_exception_msg)}
};


// These functions must be implemented in MicroPython CDC driver.
extern uint32_t usb_cdc_buf_len();
extern uint32_t usb_cdc_get_buf(uint8_t *buf, uint32_t len);
void __attribute__((weak)) usb_cdc_reset_buffers() {

}

void usbdbg_init() {
    cmd = USBDBG_NONE;
    script_ready = false;
    script_running = false;
    irq_enabled = false;

    vstr_init(&script_buf, 32);

    // Initialize the IDE exception object.
    ide_exception.base.type = &mp_type_Exception;
    ide_exception.traceback_alloc = 0;
    ide_exception.traceback_len = 0;
    ide_exception.traceback_data = NULL;
    ide_exception.args = (mp_obj_tuple_t *) &ide_exception_args_obj;
}

void usbdbg_wait_for_command(uint32_t timeout) {
    for (mp_uint_t ticks = mp_hal_ticks_ms();
         irq_enabled && ((mp_hal_ticks_ms() - ticks) < timeout) && (cmd != USBDBG_NONE); ) {
        ;
    }
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

    // Disable IDE IRQ (re-enabled by pyexec or main).
    usbdbg_set_irq_enabled(false);

    // Clear interrupt traceback
    mp_obj_exception_clear_traceback(&ide_exception);

    #if (__ARM_ARCH >= 7)
    // Remove the BASEPRI masking (if any)
    __set_BASEPRI(0);
    #endif

    // Interrupt running REPL
    // Note: setting pendsv explicitly here because the VM is probably
    // waiting in REPL and the soft interrupt flag will not be checked.
    pendsv_nlr_jump(&ide_exception);
}

bool usbdbg_get_irq_enabled() {
    return irq_enabled;
}

void usbdbg_data_in(void *buffer, int length) {
    switch (cmd) {
        case USBDBG_FW_VERSION: {
            uint32_t *ver_buf = buffer;
            ver_buf[0] = FIRMWARE_VERSION_MAJOR;
            ver_buf[1] = FIRMWARE_VERSION_MINOR;
            ver_buf[2] = FIRMWARE_VERSION_PATCH;
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_TX_BUF_LEN: {
            uint32_t tx_buf_len = usb_cdc_buf_len();
            memcpy(buffer, &tx_buf_len, sizeof(tx_buf_len));
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SENSOR_ID: {
            int sensor_id = 0xFF;
            #if MICROPY_PY_SENSOR
            if (sensor_is_detected() == true) {
                sensor_id = sensor_get_id();
            }
            #endif
            memcpy(buffer, &sensor_id, 4);
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_TX_BUF: {
            xfer_bytes += usb_cdc_get_buf(buffer, length);
            if (xfer_bytes == xfer_length) {
                cmd = USBDBG_NONE;
            }
            break;
        }

        case USBDBG_FRAME_SIZE:
            // Return 0 if FB is locked or not ready.
            ((uint32_t *) buffer)[0] = 0;
            // Try to lock FB. If header size == 0 frame is not ready
            if (mutex_try_lock_alternate(&JPEG_FB()->lock, MUTEX_TID_IDE)) {
                // If header size == 0 frame is not ready
                if (JPEG_FB()->size == 0) {
                    // unlock FB
                    mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
                } else {
                    // Return header w, h and size/bpp
                    ((uint32_t *) buffer)[0] = JPEG_FB()->w;
                    ((uint32_t *) buffer)[1] = JPEG_FB()->h;
                    ((uint32_t *) buffer)[2] = JPEG_FB()->size;
                }
            }
            cmd = USBDBG_NONE;
            break;

        case USBDBG_FRAME_DUMP:
            if (xfer_bytes < xfer_length) {
                memcpy(buffer, JPEG_FB()->pixels + xfer_bytes, length);
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    cmd = USBDBG_NONE;
                    JPEG_FB()->w = 0; JPEG_FB()->h = 0; JPEG_FB()->size = 0;
                    mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
                }
            }
            break;

        case USBDBG_ARCH_STR: {
            unsigned int uid[3] = {
                #if (OMV_UNIQUE_ID_SIZE == 2)
                0U,
                #else
                *((unsigned int *) (OMV_UNIQUE_ID_ADDR + OMV_UNIQUE_ID_OFFSET * 2)),
                #endif
                *((unsigned int *) (OMV_UNIQUE_ID_ADDR + OMV_UNIQUE_ID_OFFSET * 1)),
                *((unsigned int *) (OMV_UNIQUE_ID_ADDR + OMV_UNIQUE_ID_OFFSET * 0)),
            };
            snprintf((char *) buffer, 64, "%s [%s:%08X%08X%08X]",
                     OMV_ARCH_STR, OMV_BOARD_TYPE, uid[0], uid[1], uid[2]);
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SCRIPT_RUNNING: {
            uint32_t *buf = buffer;
            buf[0] = (uint32_t) script_running;
            cmd = USBDBG_NONE;
            break;
        }
        default: /* error */
            break;
    }
}

void usbdbg_data_out(void *buffer, int length) {
    switch (cmd) {
        case USBDBG_FB_ENABLE: {
            uint32_t enable = *((int32_t *) buffer);
            JPEG_FB()->enabled = enable;
            if (enable == 0) {
                // When disabling framebuffer, the IDE might still be holding FB lock.
                // If the IDE is not the current lock owner, this operation is ignored.
                mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
            }
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SCRIPT_EXEC:
            // check if GC is locked before allocating memory for vstr. If GC was locked
            // at least once before the script is fully uploaded xfer_bytes will be less
            // than the total length (xfer_length) and the script will Not be executed.
            if (!script_running) {
                nlr_buf_t nlr;
                if (!gc_is_locked() && nlr_push(&nlr) == 0) {
                    vstr_add_strn(&script_buf, buffer, length);
                    nlr_pop();
                }
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    cmd = USBDBG_NONE;
                    // Schedule the IDE exception to interrupt the VM.
                    usbdbg_interrupt_vm(true);
                }
            }
            break;

        case USBDBG_TEMPLATE_SAVE: {
            #if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
            image_t image;
            framebuffer_init_image(&image);

            // null terminate the path
            length = (length == 64) ? 63:length;
            ((char *) buffer)[length] = 0;

            rectangle_t *roi = (rectangle_t *) buffer;
            char *path = (char *) buffer + sizeof(rectangle_t);

            imlib_save_image(&image, path, roi, 50);

            // raise a flash IRQ to flush image
            //NVIC->STIR = FLASH_IRQn;
            #endif  //IMLIB_ENABLE_IMAGE_FILE_IO
            break;
        }

        case USBDBG_DESCRIPTOR_SAVE: {
            #if defined(IMLIB_ENABLE_IMAGE_FILE_IO) \
            && defined(IMLIB_ENABLE_KEYPOINTS)
            image_t image;
            framebuffer_init_image(&image);

            // null terminate the path
            length = (length == 64) ? 63:length;
            ((char *) buffer)[length] = 0;

            rectangle_t *roi = (rectangle_t *) buffer;
            char *path = (char *) buffer + sizeof(rectangle_t);

            py_image_descriptor_from_roi(&image, path, roi);
            #endif  //IMLIB_ENABLE_IMAGE_FILE_IO && IMLIB_ENABLE_KEYPOINTS
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_ATTR_WRITE: {
            #if MICROPY_PY_SENSOR
            /* write sensor attribute */
            int32_t attr = *((int32_t *) buffer);
            int32_t val = *((int32_t *) buffer + 1);
            switch (attr) {
                case ATTR_CONTRAST:
                    sensor_set_contrast(val);
                    break;
                case ATTR_BRIGHTNESS:
                    sensor_set_brightness(val);
                    break;
                case ATTR_SATURATION:
                    sensor_set_saturation(val);
                    break;
                case ATTR_GAINCEILING:
                    sensor_set_gainceiling(val);
                    break;
                default:
                    break;
            }
            #endif
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SET_TIME: {
            // TODO implement
            #if 0
            uint32_t *timebuf = (uint32_t *) buffer;
            timebuf[0];   // Year
            timebuf[1];   // Month
            timebuf[2];   // Day
            timebuf[3];   // Day of the week
            timebuf[4];   // Hour
            timebuf[5];   // Minute
            timebuf[6];   // Second
            timebuf[7];   // Milliseconds
            #endif
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_TX_INPUT: {
            // TODO implement
            #if 0
            uint32_t key = *((uint32_t *) buffer);
            #endif
            cmd = USBDBG_NONE;
            break;
        }

        default: /* error */
            break;
    }
}

void usbdbg_control(void *buffer, uint8_t request, uint32_t length) {
    cmd = (enum usbdbg_cmd) request;
    switch (cmd) {
        case USBDBG_FW_VERSION:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_FRAME_SIZE:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_FRAME_DUMP:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_ARCH_STR:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_SCRIPT_EXEC:
            xfer_bytes = 0;
            xfer_length = length;
            vstr_reset(&script_buf);
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

        case USBDBG_SCRIPT_SAVE:
            // TODO: save running script
            cmd = USBDBG_NONE;
            break;

        case USBDBG_SCRIPT_RUNNING:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_TEMPLATE_SAVE:
        case USBDBG_DESCRIPTOR_SAVE:
            /* save template */
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_ATTR_WRITE:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_SYS_RESET:
            NVIC_SystemReset();
            break;

        case USBDBG_SYS_RESET_TO_BL: {
            #if defined(MICROPY_BOARD_ENTER_BOOTLOADER)
            MICROPY_BOARD_ENTER_BOOTLOADER(0, 0);
            #else
            NVIC_SystemReset();
            #endif
            break;
        }

        case USBDBG_FB_ENABLE: {
            xfer_bytes = 0;
            xfer_length = length;
            break;
        }

        case USBDBG_TX_BUF:
        case USBDBG_TX_BUF_LEN:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_SENSOR_ID:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_SET_TIME:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_TX_INPUT:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}
