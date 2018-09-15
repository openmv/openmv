/* Configuration 1 */
.usb_configuration = {
  .bLength = USB_LEN_CFG_DESC,                    // 9
  .bDescriptorType = USB_DESC_TYPE_CONFIGURATION, // 2
  .wTotalLength = sizeof(struct usbd_uvc_cfg),
  .bNumInterfaces = 0x02,                         // 2
  .bConfigurationValue = 0x01,                    // 1 ID of this configuration
  .iConfiguration = 0x00,                         // 0 no description available
  .bmAttributes = USB_CONFIG_BUS_POWERED ,        // 0x80 Bus Powered
  .bMaxPower = USB_CONFIG_POWER_MA(100),          // 100 mA
},
