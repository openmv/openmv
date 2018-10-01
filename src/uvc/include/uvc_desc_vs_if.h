/* Standard VS Interface Descriptor  = interface 1 */
// alternate setting 1 = operational setting
.uvc_vs_if_alt1_desc = {
  .bLength = USB_DT_INTERFACE_SIZE,                // 9
  .bDescriptorType = USB_DESC_TYPE_INTERFACE,      // 4
  .bInterfaceNumber = USB_UVC_VSIF_NUM,            // 1 index of this interface
  .bAlternateSetting = 0x01,                       // 1 index of this setting
  .bNumEndpoints = 0x01,                           // 1 one EP used
  .bInterfaceClass = UVC_CC_VIDEO,                 // 14 Video
  .bInterfaceSubClass = UVC_SC_VIDEOSTREAMING,     // 2 Video Streaming
  .bInterfaceProtocol = UVC_PC_PROTOCOL_UNDEFINED, // 0 (protocol undefined)
  .iInterface = 0x00,                              // 0 no description available
},

/* Standard VS Isochronous Video data Endpoint Descriptor */
.uvc_vs_if_alt1_ep = {
  .bLength = USB_DT_ENDPOINT_SIZE,              // 7
  .bDescriptorType = USB_DESC_TYPE_ENDPOINT,    // 5 (ENDPOINT)
  .bEndpointAddress = UVC_IN_EP,                // 0x83 EP 3 IN
  .bmAttributes = USBD_EP_TYPE_ISOC,            // 1 isochronous transfer type
  .wMaxPacketSize = VIDEO_PACKET_SIZE,
  .bInterval = 0x01,                            // 1 one frame interval
},
