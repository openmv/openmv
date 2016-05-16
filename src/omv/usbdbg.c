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
#include "core_cm4.h"
#include "usbdbg.h"
#include "nlr.h"
#include "lexer.h"
#include "parse.h"
#include "compile.h"
#include "runtime.h"

static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;

static volatile bool script_ready;
static volatile bool script_running;
static volatile bool irq_enabled;
static vstr_t script_buf;
mp_obj_t mp_const_ide_interrupt = MP_OBJ_NULL;

extern void usbd_cdc_tx_buf_flush();
extern uint32_t usbd_cdc_tx_buf_len();
extern uint8_t *usbd_cdc_tx_buf(uint32_t bytes);
extern const char *ffs_strerror(FRESULT res);

void usbdbg_init()
{
    irq_enabled=false;
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

inline bool usbdbg_get_irq_enabled()
{
    return irq_enabled;
}

inline void usbdbg_set_irq_enabled(bool enabled)
{
    irq_enabled=enabled; __DSB();
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
            if (!fb->ready) {
                // Frame not ready return 0
                ((uint32_t*)buffer)[0] = 0;
            } else {
                // Frame ready return header
                ((uint32_t*)buffer)[0] = fb->w;
                ((uint32_t*)buffer)[1] = fb->h;
                ((uint32_t*)buffer)[2] = fb->bpp;
            }
            fb->request = 1;
            cmd = USBDBG_NONE;
            break;

        case USBDBG_FRAME_DUMP:
            if (xfer_bytes < xfer_length) {
                memcpy(buffer, fb->pixels+xfer_bytes, length);
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    fb->request = 0;
                    cmd = USBDBG_NONE;
                }
            }
            break;

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
            if (!script_running && usbdbg_get_irq_enabled() && !gc_is_locked()) {
                vstr_add_strn(&script_buf, buffer, length);
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    // Set script ready flag
                    script_ready = true;

                    // Set script running flag
                    script_running = true;

                    // Disable IDE IRQ (re-enabled by pyexec or main).
                    usbdbg_set_irq_enabled(false);

                    // interrupt running script/REPL
                    mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
                    pendsv_nlr_jump_hard(mp_const_ide_interrupt);
                }
            }
            break;

        case USBDBG_TEMPLATE_SAVE: {
            image_t image ={
                .w = fb->w,
                .h = fb->h,
                .bpp = fb->bpp,
                .pixels = fb->pixels
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
                .w = fb->w,
                .h = fb->h,
                .bpp = fb->bpp,
                .pixels = fb->pixels
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

        case USBDBG_SCRIPT_EXEC:
            xfer_bytes = 0;
            xfer_length =length;
            vstr_reset(&script_buf);
            break;

        case USBDBG_SCRIPT_STOP:
            if (usbdbg_get_irq_enabled()) {
                // Set script running flag
                script_running = false;

                // Disable IDE IRQ (re-enabled by pyexec or main).
                usbdbg_set_irq_enabled(false);

                // We can safely disable FS IRQ here
                HAL_NVIC_DisableIRQ(OTG_FS_IRQn); __DSB(); __ISB();

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

        case USBDBG_JPEG_ENABLE: {
            int16_t enable= *((int16_t*)buffer);
            sensor_enable_jpeg(enable);
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


