/*----------------- Video Association (Control + stream) ------------------*/

/* Interface Association Descriptor */
.usb_uvc_association = {
  .bLength = USB_DT_INTERFACE_ASSOC_SIZE,                       // 8
  .bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE, // 11
  .bFirstInterface = 0x00,                                      // 0
  .bInterfaceCount = 0x02,                                      // 2
  .bFunctionClass = UVC_CC_VIDEO,                               // 14 Video
  .bFunctionSubClass = UVC_SC_VIDEO_INTERFACE_COLLECTION,       // 3 Video Interface Collection
  .bFunctionProtocol = UVC_PC_PROTOCOL_UNDEFINED,              // 0 (protocol undefined)
  .iFunction = 0x02,                                            // 2
},
