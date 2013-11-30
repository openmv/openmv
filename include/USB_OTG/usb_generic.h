#ifndef __USB_DEV_H__
#define __USB_DEV_H__
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_ioreq.h"

#define MAX_DATA_PACKET_SIZE                    64   /* Endpoint IN & OUT Packet size */
#define MAX_CMD_PACKET_SIZE                      8    /* Control Endpoint Packet size */

struct usb_user_cb {
    void *user_data;
    void (*usb_data_in)(void *buffer, int *length, void *user_data);
    void (*usb_data_out)(void *buffer, int *length, void *user_data);
};

void usb_dev_init(struct usb_user_cb *cb);
void usb_dev_send(void *buffer, int length);
#endif  /* __USB_DEV_H__ */
