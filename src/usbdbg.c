#include <string.h>
#include "libmp.h"
#include "framebuffer.h"
#include "usbdbg.h"

#define USB_TX_BUF_SIZE (64)
static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;
mp_obj_t mp_const_ide_interrupt = MP_OBJ_NULL;

void usbdbg_init()
{
    mp_const_ide_interrupt = mp_obj_new_exception_msg(&mp_type_OSError, "IDEInterrupt");
}

void usbdbg_data_in(void *buffer, int *length)
{
    switch (cmd) {
        case USBDBG_FB_SIZE:  /* dump framebuffer */
            memcpy(buffer, fb, xfer_length);
            cmd = USBDBG_NONE;
            break;
        case USBDBG_DUMP_FB:  /* dump framebuffer */
            if (xfer_bytes < xfer_length) {
                memcpy(buffer, fb->pixels+xfer_bytes, *length);
                xfer_bytes += *length;
            } else {
                *length = 0;
                cmd = USBDBG_NONE;
            }
            break;

        default: /* error */
            *length = 0;
            break;
    }
}

void usbdbg_data_out(void *buffer, int length)
{
    switch (cmd) {
        case USBDBG_EXEC_SCRIPT: /* execute script */
            vstr_add_strn(libmp_get_line(), buffer, length);
            xfer_bytes += length;
            if (xfer_bytes == xfer_length) {
                /* interrupt do_repl */
                libmp_int_repl();
            }
            break;

        default: /* error */
            break;
    }
}

void usbdbg_control(uint8_t request, int length)
{
    cmd = (enum usbdbg_cmd) request;
    switch (cmd) {
        case USBDBG_FB_SIZE:     /* read framebuffer size */
            xfer_bytes = 0;
            xfer_length = 12;
            break;

        case USBDBG_DUMP_FB:     /* dump framebuffer */
            /* reset bytes counter */
            xfer_bytes = 0;
            xfer_length = fb->w*fb->h*fb->bpp;
            break;

        case USBDBG_EXEC_SCRIPT: /* execute script */
            /* reset bytes counter */
            xfer_bytes = 0;
            xfer_length =length;
            break;

        case USBDBG_STOP_SCRIPT:
            if (libmp_code_running()) {
                /* Stop any running code by raising an exception */
                mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
                pendsv_nlr_jump(mp_const_ide_interrupt);
            }
            break;

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}
