#include <string.h>
#include "libmp.h"
#include "sensor.h"
#include "usbdbg.h"

#define USB_TX_BUF_SIZE (64)
static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd; 

extern struct sensor_dev sensor;

void usb_fb_data_in(void *buffer, int *length)
{
    struct frame_buffer *fb = &sensor.frame_buffer;

    switch (cmd) {
        case USBDBG_DUMP_FB:  /* dump framebuffer */
            if (xfer_bytes < (fb->width*fb->height*fb->bpp)) {
                memcpy(buffer, fb->pixels+xfer_bytes, *length);
                *length = USB_TX_BUF_SIZE;
                xfer_bytes += USB_TX_BUF_SIZE;
            } else {
                *length = 0;
            }
            break;

        default: /* error */
            break;
    }
}

void usb_fb_data_out(void *buffer, int length)
{
    switch (cmd) {
        case USBDBG_EXEC_SCRIPT: /* execute script */
            vstr_add_strn(libmp_get_line(), buffer, length);
            xfer_bytes += length;
            if (xfer_bytes == xfer_length) {
                /* interrupt do_repl */
                libmp_line_feed();
            }
            break;

        default: /* error */
            break;
    }
}

void usb_fb_control(uint8_t request, int length)
{
    cmd = (enum usbdbg_cmd) request;
    switch (cmd) {
        case USBDBG_DUMP_FB:     /* dump framebuffer */
            /* reset bytes counter */
            xfer_bytes = 0;
            break;

        case USBDBG_EXEC_SCRIPT: /* execute script */
            /* reset bytes counter */
            xfer_bytes = 0;
            xfer_length =length;
            break;

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}
