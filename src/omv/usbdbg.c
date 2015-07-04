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

#define USB_TX_BUF_SIZE (64)
static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;

static vstr_t script_buf;
static mp_obj_t script;
static int script_ready=0;
mp_obj_t mp_const_ide_interrupt = MP_OBJ_NULL;

extern void usbd_cdc_tx_buf_flush();
extern uint32_t usbd_cdc_tx_buf_len();
extern uint8_t *usbd_cdc_tx_buf(uint32_t bytes);
extern const char *ffs_strerror(FRESULT res);

void usbdbg_init()
{
    vstr_init(&script_buf, 64);
    mp_const_ide_interrupt = mp_obj_new_exception_msg(&mp_type_OSError, "IDEInterrupt");
}

int usbdbg_script_ready()
{
    return script_ready;
}

mp_obj_t usbdbg_get_script()
{
    return script;
}

void usbdbg_clr_script()
{
    script_ready =0;
    fb->lock_tried=0;
    mutex_unlock(&fb->lock);
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
            memcpy(buffer, &tx_buf_len, length);
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
            memcpy(buffer, fb, length);
            cmd = USBDBG_NONE;
            break;

        case USBDBG_FRAME_LOCK:
            // try to lock FB, return fb hdr if locked
            if (fb->ready && mutex_try_lock(&fb->lock)) {
                fb->lock_tried = 0;
                memcpy(buffer, fb, length);
            } else {
                // no valid frame or failed to lock, return 0
                fb->lock_tried = 1;
                ((uint32_t*)buffer)[0] = 0;
            }
            cmd = USBDBG_NONE;
            break;

        case USBDBG_FRAME_DUMP:
            if (xfer_bytes < xfer_length) {
                memcpy(buffer, fb->pixels+xfer_bytes, length);
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    mutex_unlock(&fb->lock);
                    cmd = USBDBG_NONE;
                }
            }
            break;

        default: /* error */
            break;
    }
}

extern int py_image_descriptor_from_roi(image_t *image, const char *path, rectangle_t *roi);

void usbdbg_data_out(void *buffer, int length)
{
    switch (cmd) {
        case USBDBG_SCRIPT_EXEC:
            vstr_add_strn(&script_buf, buffer, length);
            xfer_bytes += length;
            if (xfer_bytes == xfer_length) {
                // set script ready flag
                script_ready = 1;

                // parse and compile script
                mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_,
                        vstr_str(&script_buf), vstr_len(&script_buf), 0);
                mp_parse_node_t pn = mp_parse(lex, MP_PARSE_FILE_INPUT);
                script = mp_compile(pn, lex->source_name, MP_EMIT_OPT_NONE, false);

                // interrupt running script/REPL
                mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
                pendsv_nlr_jump_hard(mp_const_ide_interrupt);
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

            int res=imlib_save_image(&image, path, roi);
            if (res != FR_OK) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
            }
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

        case USBDBG_FRAME_LOCK:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_FRAME_UPDATE:
            sensor_snapshot(NULL);
            cmd = USBDBG_NONE;
            break;

        case USBDBG_SCRIPT_EXEC:
            xfer_bytes = 0;
            xfer_length =length;
            vstr_reset(&script_buf);
            break;

        case USBDBG_SCRIPT_STOP:
            /* interrupt running code by raising an exception */
            mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
            pendsv_nlr_jump_hard(mp_const_ide_interrupt);
            cmd = USBDBG_NONE;
            break;

        case USBDBG_SCRIPT_SAVE:
            /* save running script */
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

        case USBDBG_BOOT:
            *((uint32_t *)0x20002000) = 0xDEADBEEF;
            NVIC_SystemReset();
            break;

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


