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


/*------------------ VideoControl Interface descriptor --------------------*/

/* Standard VC Interface Descriptor  = interface 0 */
.uvc_vc_if_desc = {
  .bLength = USB_LEN_IF_DESC,                       // 9
  .bDescriptorType = USB_DESC_TYPE_INTERFACE,       // 4
  .bInterfaceNumber = USB_UVC_VCIF_NUM,             // 0 index of this interface (VC)
  .bAlternateSetting = 0x00,                        // 0 index of this setting
  .bNumEndpoints = 0x01,                            // 1 endpoint
  .bInterfaceClass = UVC_CC_VIDEO,                  // 14 Video
  .bInterfaceSubClass = UVC_SC_VIDEOCONTROL,        // 1 Video Control
  .bInterfaceProtocol = UVC_PC_PROTOCOL_UNDEFINED,  // 0 (protocol undefined)
  .iInterface = 0x00,
},

/* Class-specific VC Interface Descriptor */
.ucv_vc_header = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, ucv_vc_header),
  .bDescriptorType = UVC_CS_INTERFACE,     // 36 (INTERFACE)
  .bDescriptorSubType = UVC_VC_HEADER,     // 1 (HEADER)
  .bcdUVC = UVC_VERSION,                   // 1.10 or 1.00
  .wTotalLength =
    SIZEOF_M(struct usbd_uvc_cfg, ucv_vc_header) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_input_terminal) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_processing_unit) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_agc) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_oem) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_rad) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_sys) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_vid) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_rad_2) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_cust) +
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_output_terminal), // header+units+terminals
  .dwClockFrequency = 0x005B8D80,          // 6.000000 MHz
  .bInCollection = 0x01,                   // 1 one streaming interface
  .baInterfaceNr = { 0x01 },               // 1 VS interface 1 belongs to this VC interface
},

/* Input Terminal Descriptor (Camera) */
.uvc_vc_input_terminal = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_input_terminal), // Descriptor size
  .bDescriptorType = UVC_CS_INTERFACE,       // 36 (INTERFACE)
  .bDescriptorSubType = UVC_VC_INPUT_TERMINAL, // 2 (INPUT_TERMINAL)
  .bTerminalID = VC_INPUT_TERMINAL_ID,   // 1 ID of this Terminal
  .wTerminalType = UVC_ITT_CAMERA,           // 0x0201 Camera Sensor
  .bAssocTerminal = 0x00,                    // 0 no Terminal associated
  .iTerminal = 0x00,                         // 0 no description available
  .wObjectiveFocalLengthMin = 0x0000,        // 0
  .wObjectiveFocalLengthMax = 0x0000,        // 0
  .wOcularFocalLength = 0x0000,              // 0
  .bControlSize = 0x03,                      // 2
  .bmControls = { 0x00, 0x00, 0x00 },        // 0x0000 no controls supported
},

/* Processing Unit Descriptor */
.uvc_vc_processing_unit = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_processing_unit), // Descriptor size
  .bDescriptorType = 0x24,                   // Class specific interface desc type
  .bDescriptorSubType = 0x05,                // Processing Unit Descriptor type
  .bUnitID = VC_CONTROL_PU_ID,               // ID of this terminal
  .bSourceID = 0x01,                         // Source ID : 1 : Connected to input terminal
  .wMaxMultiplier = 0,                       // Digital multiplier
  .bControlSize = 0x03,                      // Size of controls field for this terminal : 2 bytes
  .bmControls = { 0x03, 0x00, 0x00 },        // Brightness and contrast
  .iProcessing = 0x00,                       // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_agc = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_agc), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_AGC_ID,     // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    'a','g','c','-',
    '0','0','0','0'
  },
  .bNumControls = 0x14,                        // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x04,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { 0xff, 0xff, 0x0f, 0x00 },    // Registers 0x00 to 0x48
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_oem = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_oem), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_OEM_ID,     // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    'o','e','m','-',
    '0','0','0','0'
  },
  .bNumControls = 0x1e,                        // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x04,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { 0xbf, 0xff, 0xff, 0x7f },    // Registers 0x00 to 0x48
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_rad = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_rad), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_RAD_ID,     // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    'r','a','d','-',
    '0','0','0','0'
  },
  .bNumControls = 48,                          // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x08,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { 0xFF, 0xFF, 0xFF, 0x81, 0xFC, 0xCF, 0xFF, 0x03 },    // Registers 0x00 to 0x48
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_rad_2 = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_rad_2), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_RAD_2_ID,       // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    'r','a','d','2',
    '0','0','0','0'
  },
  .bNumControls = 27,                        // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x04,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { 0xf9, 0x9f, 0xff, 0x07 },    // Registers 0x00 to 0x48
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_sys = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_sys), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_SYS_ID,     // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    's','y','s','-',
    '0','0','0','0'
  },
  .bNumControls = 0x17,                        // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x04,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { 0xFF, 0xFF, 0x7F, 0x00 },    // Registers 0x00 to 0x48
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_vid = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_vid), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_VID_ID,     // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    'v','i','d','-',
    '0','0','0','0'
  },
  .bNumControls = 0x0e,                        // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x04,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { 0xFF, 0x3F, 0x00, 0x00 },    // Registers 0x00 to 0x48
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Extension Unit Descriptor */
.uvc_vc_xu_lep_cust = {
  .bLength =
    SIZEOF_M(struct usbd_uvc_cfg, uvc_vc_xu_lep_cust), // Descriptor size
  .bDescriptorType = 0x24,                     // Class specific interface desc type
  .bDescriptorSubType = 0x06,                  // Extension Unit Descriptor type
  .bUnitID = VC_CONTROL_XU_LEP_CUST_ID,     // ID of this terminal
  .guidExtensionCode = {                       // 16 byte GUID
    'p','t','1','-',
    'l','e','p','-',
    'c','u','s','t',
    '0','0','0','0'
  },
  .bNumControls = CUST_CONTROL_END,                           // Number of controls in this terminal
  .bNrInPins = 0x01,                           // Number of input pins in this terminal
  .baSourceID = { 0x02 },                      // Source ID : 2 : Connected to Proc Unit
  .bControlSize = 0x04,                        // Size of controls field for this terminal : 1 byte
  .bmControls = { (1 << CUST_CONTROL_END) - 1, 0, 0, 0 },    //
  .iExtension = 0x00,                          // String desc index : Not used
},

/* Output Terminal Descriptor */
.uvc_vc_output_terminal = {
  .bLength = UVC_DT_OUTPUT_TERMINAL_SIZE,      // 9
  .bDescriptorType = UVC_CS_INTERFACE,         // 36 (INTERFACE)
  .bDescriptorSubType = UVC_VC_OUTPUT_TERMINAL, // 3 (OUTPUT_TERMINAL)
  .bTerminalID = VC_OUTPUT_TERMINAL_ID,    // 2 ID of this Terminal
  .wTerminalType = UVC_TT_STREAMING,           // 0x0101 USB streaming terminal
  .bAssocTerminal = 0x00,                      // 0 no Terminal assiciated
  .bSourceID = 0x03,                           // 1 input pin connected to output pin unit 1
  .iTerminal = 0x00,                           // 0 no description available
},

/* Standard Interrupt Endpoint Descriptor */
.uvc_vc_ep = {
  .bLength = USB_DT_ENDPOINT_SIZE,              // 7
  .bDescriptorType = USB_DESC_TYPE_ENDPOINT,    // 5 (ENDPOINT)
  .bEndpointAddress = UVC_CMD_EP,               // 0x82 EP 2 IN
  .bmAttributes = USBD_EP_TYPE_INTR,            // interrupt
  .wMaxPacketSize = CMD_PACKET_SIZE,            // 8 bytes status
  .bInterval = 0x20,                            // poll at least every 32ms
},

/* Class-specific Interrupt Endpoint Descriptor */
.uvc_vc_cs_ep = {
  .bLength = UVC_DT_CONTROL_ENDPOINT_SIZE,
  .bDescriptorType = UVC_CS_ENDPOINT,
  .bDescriptorSubType = USBD_EP_TYPE_INTR,
  .wMaxTransferSize = CMD_PACKET_SIZE,
},
