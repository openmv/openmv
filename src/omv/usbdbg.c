/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * USB debug support.
 *
 */
#include "mp.h"
#include "imlib.h"
#include "sensor.h"
#include "framebuffer.h"
#include "ff.h"
#include "usbdbg.h"
#include "nlr.h"
#include "lexer.h"
#include "parse.h"
#include "compile.h"
#include "runtime.h"
#include "omv_boardconfig.h"

static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;

static volatile bool script_ready;
static volatile bool script_running;
static vstr_t script_buf;
static mp_obj_t mp_const_ide_interrupt = MP_OBJ_NULL;

extern void usbd_cdc_tx_buf_flush();
extern uint32_t usbd_cdc_tx_buf_len();
extern uint8_t *usbd_cdc_tx_buf(uint32_t bytes);
extern const char *ffs_strerror(FRESULT res);

void usbdbg_init()
{
    script_ready=false;
    script_running=false;
    vstr_init(&script_buf, 32);
    mp_const_ide_interrupt = mp_obj_new_exception_msg(&mp_type_Exception, "IDE interrupt");
}

bool usbdbg_script_ready()
{
    return script_ready;
}

vstr_t *usbdbg_get_script()
{
    return &script_buf;
}

void usbdbg_set_script_running(bool running)
{
    script_running = running;
}

inline void usbdbg_set_irq_enabled(bool enabled)
{
    if (enabled) {
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    } else {
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
    }
    __DSB(); __ISB();
}

void usbdbg_data_in(void *buffer, int length)
{
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
            uint32_t tx_buf_len = usbd_cdc_tx_buf_len();
            memcpy(buffer, &tx_buf_len, sizeof(tx_buf_len));
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_TX_BUF: {
            uint8_t *tx_buf = usbd_cdc_tx_buf(length);
            memcpy(buffer, tx_buf, length);
            if (xfer_bytes == xfer_length) {
                cmd = USBDBG_NONE;
            }
            break;
        }

        case USBDBG_FRAME_SIZE:
            // Return 0 if FB is locked or not ready.
            ((uint32_t*)buffer)[0] = 0;
            // Try to lock FB. If header size == 0 frame is not ready
            if (mutex_try_lock(&JPEG_FB()->lock, MUTEX_TID_IDE)) {
                // If header size == 0 frame is not ready
                if (JPEG_FB()->size == 0) {
                    // unlock FB
                    mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
                } else {
                    // Return header w, h and size/bpp
                    ((uint32_t*)buffer)[0] = JPEG_FB()->w;
                    ((uint32_t*)buffer)[1] = JPEG_FB()->h;
                    ((uint32_t*)buffer)[2] = JPEG_FB()->size;
                }
            }
            cmd = USBDBG_NONE;
            break;

        case USBDBG_FRAME_DUMP:
            if (xfer_bytes < xfer_length) {
                memcpy(buffer, JPEG_FB()->pixels+xfer_bytes, length);
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    cmd = USBDBG_NONE;
                    JPEG_FB()->w = 0; JPEG_FB()->h = 0; JPEG_FB()->size = 0;
                    mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
                }
            }
            break;

        case USBDBG_ARCH_STR: {
            snprintf((char *) buffer, 64, "%s [%s:%08X%08X%08X]",
                    OMV_ARCH_STR, OMV_BOARD_TYPE,
                    *((unsigned int *) (OMV_UNIQUE_ID_ADDR + 8)),
                    *((unsigned int *) (OMV_UNIQUE_ID_ADDR + 4)),
                    *((unsigned int *) (OMV_UNIQUE_ID_ADDR + 0)));
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

extern int py_image_descriptor_from_roi(image_t *image, const char *path, rectangle_t *roi);

void usbdbg_data_out(void *buffer, int length)
{
    switch (cmd) {
        case USBDBG_SCRIPT_EXEC:
            // check if GC is locked before allocating memory for vstr. If GC was locked
            // at least once before the script is fully uploaded xfer_bytes will be less
            // than the total length (xfer_length) and the script will Not be executed.
            if (!script_running && !gc_is_locked()) {
                vstr_add_strn(&script_buf, buffer, length);
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    // Set script ready flag
                    script_ready = true;

                    // Set script running flag
                    script_running = true;

                    // Disable IDE IRQ (re-enabled by pyexec or main).
                    usbdbg_set_irq_enabled(false);

                    // Clear interrupt traceback
                    mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
                    // Interrupt running REPL
                    // Note: setting pendsv explicitly here because the VM is probably
                    // waiting in REPL and the soft interrupt flag will not be checked.
                    pendsv_nlr_jump_hard(mp_const_ide_interrupt);
                }
            }
            break;

        case USBDBG_TEMPLATE_SAVE: {
            image_t image ={
                .w = MAIN_FB()->w,
                .h = MAIN_FB()->h,
                .bpp = MAIN_FB()->bpp,
                .pixels = MAIN_FB()->pixels
            };

            // null terminate the path
            length = (length == 64) ? 63:length;
            ((char*)buffer)[length] = 0;

            rectangle_t *roi = (rectangle_t*)buffer;
            char *path = (char*)buffer+sizeof(rectangle_t);

            imlib_save_image(&image, path, roi, 50);
            // raise a flash IRQ to flush image
            //NVIC->STIR = FLASH_IRQn;
            break;
        }

        case USBDBG_DESCRIPTOR_SAVE: {
            image_t image ={
                .w = MAIN_FB()->w,
                .h = MAIN_FB()->h,
                .bpp = MAIN_FB()->bpp,
                .pixels = MAIN_FB()->pixels
            };

            // null terminate the path
            length = (length == 64) ? 63:length;
            ((char*)buffer)[length] = 0;

            rectangle_t *roi = (rectangle_t*)buffer;
            char *path = (char*)buffer+sizeof(rectangle_t);

            py_image_descriptor_from_roi(&image, path, roi);
            break;
        }
        default: /* error */
            break;
    }
}

void usbdbg_control(void *buffer, uint8_t request, uint32_t length)
{
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
                // Set script running flag
                script_running = false;

                // Disable IDE IRQ (re-enabled by pyexec or main).
                usbdbg_set_irq_enabled(false);

                // interrupt running code by raising an exception
                mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
                pendsv_nlr_jump_hard(mp_const_ide_interrupt);
            }
            cmd = USBDBG_NONE;
            break;

        case USBDBG_SCRIPT_SAVE:
            /* save running script */
            // TODO
            break;

        case USBDBG_SCRIPT_RUNNING:
            xfer_bytes = 0;
            xfer_length =length;
            break;

        case USBDBG_TEMPLATE_SAVE:
        case USBDBG_DESCRIPTOR_SAVE:
            /* save template */
            xfer_bytes = 0;
            xfer_length =length;
            break;

        case USBDBG_ATTR_WRITE: {
            /* write sensor attribute */
            int16_t attr= *((int16_t*)buffer);
            int16_t val = *((int16_t*)buffer+1);
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
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SYS_RESET:
            NVIC_SystemReset();
            break;

        case USBDBG_FB_ENABLE: {
            int16_t enable = *((int16_t*)buffer);
            JPEG_FB()->enabled = enable;
            if (enable == 0) {
                // When disabling framebuffer, the IDE might still be holding FB lock.
                // If the IDE is not the current lock owner, this operation is ignored.
                mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
            }
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_TX_BUF:
        case USBDBG_TX_BUF_LEN:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}


