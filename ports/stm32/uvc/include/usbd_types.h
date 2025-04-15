#ifndef __USBD_TYPES_H
#define __USBD_TYPES_H

#define  USB_REQ_READ_MASK                              0x80

/* USB_DT_CONFIG: Configuration descriptor information. */
struct usb_config_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;

	uint16_t wTotalLength;
	uint8_t  bNumInterfaces;
	uint8_t  bConfigurationValue;
	uint8_t  iConfiguration;
	uint8_t  bmAttributes;
	uint8_t  bMaxPower;
} __attribute__ ((packed));

#define USB_DT_CONFIG_SIZE		9

/* USB_DT_INTERFACE_ASSOCIATION: groups interfaces */
struct usb_interface_assoc_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;

	uint8_t  bFirstInterface;
	uint8_t  bInterfaceCount;
	uint8_t  bFunctionClass;
	uint8_t  bFunctionSubClass;
	uint8_t  bFunctionProtocol;
	uint8_t  iFunction;
} __attribute__ ((packed));

#define USB_DT_INTERFACE_ASSOC_SIZE		8

/* USB_DT_INTERFACE: Interface descriptor */
struct usb_interface_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;

	uint8_t  bInterfaceNumber;
	uint8_t  bAlternateSetting;
	uint8_t  bNumEndpoints;
	uint8_t  bInterfaceClass;
	uint8_t  bInterfaceSubClass;
	uint8_t  bInterfaceProtocol;
	uint8_t  iInterface;
} __attribute__ ((packed));

#define USB_DT_INTERFACE_SIZE		9

/* USB_DT_ENDPOINT: Endpoint descriptor */
struct usb_endpoint_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;

	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} __attribute__ ((packed));

#define USB_DT_ENDPOINT_SIZE		7

#endif