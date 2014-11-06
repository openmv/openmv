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
#include "py/py_file.h"
#include "core_cm4.h"
#include "usbdbg.h"

#define USB_TX_BUF_SIZE (64)
static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;

static vstr_t script;
static int script_ready=0;
mp_obj_t mp_const_ide_interrupt = MP_OBJ_NULL;

void usbdbg_init()
{
    vstr_init(&script, 64);
    mp_const_ide_interrupt = mp_obj_new_exception_msg(&mp_type_OSError, "IDEInterrupt");
}

int usbdbg_script_ready()
{
    return script_ready;
}

vstr_t *usbdbg_get_script()
{
    return &script;
}

void usbdbg_clr_script()
{
    script_ready =0;
    vstr_reset(&script);
    fb->lock_tried=0;
    mutex_unlock(&fb->lock);
}

void usbdbg_data_in(void *buffer, int length)
{
    switch (cmd) {
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
            vstr_add_strn(&script, buffer, length);
            xfer_bytes += length;
            if (xfer_bytes == xfer_length) {
                script_ready = 1;
                /* Interrupt REPL */
                mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
                pendsv_nlr_jump(mp_const_ide_interrupt);
            }
            break;

        case USBDBG_TEMPLATE_SAVE: {
            int res;
            image_t image;
            image.w = fb->w; image.h = fb->h; image.bpp = fb->bpp; image.pixels = fb->pixels;
            if ((res=imlib_save_image(&image, "1:/template.pgm", (rectangle_t*)buffer)) != FR_OK) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, ffs_strerror(res)));
            }
            // raise a flash IRQ to flush image
            //NVIC->STIR = FLASH_IRQn;
            break;
        }

        case USBDBG_DESCRIPTOR_SAVE: {
            image_t image;
            image.w = fb->w; image.h = fb->h; image.bpp = fb->bpp; image.pixels = fb->pixels;
            py_image_descriptor_from_roi(&image, "1:/desc.freak", (rectangle_t*)buffer);
            break;
        }
        default: /* error */
            break;
    }
}

void usbdbg_control(void *buffer, uint8_t request, uint16_t length)
{
    cmd = (enum usbdbg_cmd) request;
    switch (cmd) {
        case USBDBG_FRAME_SIZE:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_FRAME_DUMP:
            xfer_bytes = 0;
            xfer_length =((uint32_t)length)<<2;
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
            script_ready = 0;
            vstr_reset(&script);
            break;

        case USBDBG_SCRIPT_STOP:
            /* interrupt running code by raising an exception */
            mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
            pendsv_nlr_jump(mp_const_ide_interrupt);
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
            int val = (int8_t)(length&0xff);
            int attr= length>>8;
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

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}


