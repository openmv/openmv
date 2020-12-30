/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB UVC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as UVC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                UVC Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as UVC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *      
  *  @endverbatim
  *                                  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_uvc.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_types.h"
#include "uvc_desc.h"
#include "cambus.h"
#include "sensor.h"
extern sensor_t sensor;

static USBD_UVC_HandleTypeDef USBD_UVC_Handle;
volatile uint8_t g_uvc_stream_status = 0;

static uint8_t USBD_UVC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_UVC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_UVC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_UVC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_UVC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_UVC_SOF(USBD_HandleTypeDef *pdev);
static uint8_t USBD_UVC_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_UVC_GetFSCfgDesc(uint16_t *length);
uint8_t  *USBD_UVC_GetDeviceQualifierDescriptor(uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_UVC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/* UVC interface class callbacks structure */
USBD_ClassTypeDef  USBD_UVC = {
  USBD_UVC_Init,
  USBD_UVC_DeInit,
  USBD_UVC_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_UVC_EP0_RxReady,
  USBD_UVC_DataIn,
  USBD_UVC_DataOut,
  USBD_UVC_SOF,
  NULL,
  NULL,     
  USBD_UVC_GetFSCfgDesc,  
  USBD_UVC_GetFSCfgDesc,    
  USBD_UVC_GetFSCfgDesc, 
  USBD_UVC_GetDeviceQualifierDescriptor,
};

/* USB UVC device Configuration Descriptor */
__ALIGN_BEGIN struct usbd_uvc_cfg USBD_UVC_CfgFSDesc __ALIGN_END = {
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
    
    /*-------------- Video Streaming (VS) Interface Descriptor ----------------*/
    
    /* Standard VS Interface Descriptor  = interface 1 */
    // alternate setting 0 = Zero Bandwidth
    .uvc_vs_if_alt0_desc = {
      .bLength = USB_DT_INTERFACE_SIZE,                // 9
      .bDescriptorType = USB_DESC_TYPE_INTERFACE,      // 4
      .bInterfaceNumber = USB_UVC_VSIF_NUM,            // 1 index of this interface
      .bAlternateSetting = 0x00,                       // 0 index of this setting
      .bNumEndpoints = 0x00,                           // 0 no EP used
      .bInterfaceClass = UVC_CC_VIDEO,                 // 14 Video
      .bInterfaceSubClass = UVC_SC_VIDEOSTREAMING,     // 2 Video Streaming
      .bInterfaceProtocol = UVC_PC_PROTOCOL_UNDEFINED, // 0 (protocol undefined)
      .iInterface = 0x00,                              // 0 no description available
    },
    
    /* Class-specific VS Header Descriptor (Input) */
    .uvc_vs_input_header_desc = {
      .bLength =
        SIZEOF_M(struct usbd_uvc_cfg, uvc_vs_input_header_desc),
      .bDescriptorType = UVC_CS_INTERFACE,       // 36 (INTERFACE)
      .bDescriptorSubType = UVC_VS_INPUT_HEADER, // 1 (INPUT_HEADER)
      .bNumFormats = VS_NUM_FORMATS,             // number of format descriptors
      .wTotalLength =
        SIZEOF_M(struct usbd_uvc_cfg, uvc_vs_input_header_desc) +
        SIZEOF_M(struct usbd_uvc_cfg, uvc_vs_frames_formats_desc),
      .bEndpointAddress = UVC_IN_EP,             // 0x83 EP 3 IN
      .bmInfo = 0x00,                            // 0 no dynamic format change supported
      .bTerminalLink = VC_OUTPUT_TERMINAL_ID,    // 2 supplies terminal ID 2 (Output terminal)
      .bStillCaptureMethod = 0x01,               // 1 Host captures from video stream
      .bTriggerSupport = 0x00,                   // 0 HW trigger supported for still image capture
      .bTriggerUsage = 0x00,                     // 0 HW trigger initiate a still image capture
      .bControlSize = 0x01,                      // 1 one byte bmaControls field size
      .bmaControls = {
        { 0x00 },                                // bmaControls(0)           0 no VS specific controls
        { 0x00 },                                // bmaControls(1)           0 no VS specific controls
        { 0x00 },                                // bmaControls(2)           0 no VS specific controls
      },
    },

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

};

static struct uvc_vs_frames_formats_descriptor uvc_vs_frames_formats_desc = {
    .uvc_vs_frames_format_1 =
      {
        .uvc_vs_format = UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(GREY, 4),
        .uvc_vs_frame  = { UVC_FRAME_FORMAT(VS_FRAME_INDEX_1, GREY, 80, 60),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_2, GREY, 160, 120),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_3, GREY, 320, 240),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_4, GREY, 640, 480)},
        .uvc_vs_color  = UVC_COLOR_MATCHING_DESCRIPTOR(),
      },
    .uvc_vs_frames_format_2 =
      {
        .uvc_vs_format = UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(YUYV, 3),
        .uvc_vs_frame  = { UVC_FRAME_FORMAT(VS_FRAME_INDEX_1, YUYV, 80, 60),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_2, YUYV, 160, 120),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_3, YUYV, 320, 240)},
        .uvc_vs_color  = UVC_COLOR_MATCHING_DESCRIPTOR(),
      },
    .uvc_vs_frames_format_3 =
      {
        .uvc_vs_format = UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(RGB565, 3),
        .uvc_vs_frame  = { UVC_FRAME_FORMAT(VS_FRAME_INDEX_1, RGB565, 80, 60),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_2, RGB565, 160, 120),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_3, RGB565, 320, 240)},
        .uvc_vs_color  = UVC_COLOR_MATCHING_DESCRIPTOR(),
      },
};

static struct uvc_vs_frames_formats_descriptor uvc_vs_frames_formats_desc_gs = {
    .uvc_vs_frames_format_1 =
      {
        .uvc_vs_format = UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(  GREY, 4),
        .uvc_vs_frame  = { UVC_FRAME_FORMAT(VS_FRAME_INDEX_1, GREY, 80, 60),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_2, GREY, 160, 120),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_3, GREY, 320, 240),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_4, GREY, 640, 480)},
        .uvc_vs_color  = UVC_COLOR_MATCHING_DESCRIPTOR(),
      },
    .uvc_vs_frames_format_2 =
      {
        .uvc_vs_format = UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(  GREY, 3),
        .uvc_vs_frame  = { UVC_FRAME_FORMAT(VS_FRAME_INDEX_1, GREY, 80, 60),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_2, GREY, 160, 120),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_3, GREY, 320, 240)},
        .uvc_vs_color  = UVC_COLOR_MATCHING_DESCRIPTOR(),
      },
    .uvc_vs_frames_format_3 =
      {
        .uvc_vs_format = UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(  GREY, 3),
        .uvc_vs_frame  = { UVC_FRAME_FORMAT(VS_FRAME_INDEX_1, GREY, 80, 60),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_2, GREY, 160, 120),
                           UVC_FRAME_FORMAT(VS_FRAME_INDEX_3, GREY, 320, 240)},
        .uvc_vs_color  = UVC_COLOR_MATCHING_DESCRIPTOR(),
      },
};

/**
  * @brief  USBD_UVC_Init
  *         Initialize the UVC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_UVC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = 0;
  USBD_UVC_HandleTypeDef   *hcdc;
  
  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
                 UVC_IN_EP,
                 USBD_EP_TYPE_ISOC,
                 VIDEO_PACKET_SIZE);

  USBD_LL_OpenEP(pdev,
                 UVC_CMD_EP,
                 USBD_EP_TYPE_INTR,
                 CMD_PACKET_SIZE);


  pdev->pClassData = &USBD_UVC_Handle;
  hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  /* Init  physical Interface components */
  ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->Init();
  
  /* Init Xfer states */
  hcdc->TxState =0;
  hcdc->RxState =0;
  return ret;
}

/**
  * @brief  USBD_UVC_Init
  *         DeInitialize the UVC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_UVC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Open EP IN */
  USBD_LL_CloseEP(pdev, UVC_IN_EP);

  USBD_LL_CloseEP(pdev, UVC_CMD_EP);

  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL) {
    ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->DeInit();
    //USBD_free(pdev->pClassData); //TODO
    pdev->pClassData = NULL;
  }
  
  return ret;
}

/**
  * @brief  USBD_UVC_Setup
  *         Handle the UVC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_UVC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t ret = USBD_OK;
  static uint8_t ifalt = 0;
  USBD_UVC_HandleTypeDef *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
    
  switch (req->bmRequest & USB_REQ_TYPE_MASK) {
  case USB_REQ_TYPE_CLASS :
    if (req->wLength) {
      // TODO: what to do with recipient as endpoint?
      uint8_t address = (req->wIndex >> 0) & 0xff;
      VC_TERMINAL_ID entity_id = (VC_TERMINAL_ID)((req->wIndex >> 8) & 0xff);
      if (req->bmRequest & USB_REQ_READ_MASK) {
        USBD_LL_FlushEP (pdev,USB_ENDPOINT_OUT(0));
        if (address == USB_UVC_VSIF_NUM) {
          ret = ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->VS_CtrlGet(req->bRequest,
                  (uint8_t *)hcdc->data, req->wLength, req->wIndex, req->wValue);
        } else {
        ret = ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->ControlGet(entity_id, req->bRequest,
                (uint8_t *)hcdc->data, req->wLength, req->wIndex, req->wValue);
        }

        if (ret == USBD_OK) {
          USBD_CtlSendData (pdev, (uint8_t *)hcdc->data, req->wLength);
        } else {
          USBD_CtlError (pdev, req);
        }
      } else {
        hcdc->CmdOpCode = req->bRequest;
        hcdc->CmdLength = req->wLength;
        hcdc->CmdIndex = req->wIndex;
        hcdc->CmdValue = req->wValue;
        USBD_CtlPrepareRx (pdev,  (uint8_t *)hcdc->data, req->wLength);
      }
    } else {
      ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest, (uint8_t*)req, 0, 0, 0);
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest) {
    case USB_REQ_GET_DESCRIPTOR: 
      if( (req->wValue >> 8) == UVC_CS_DEVICE) {
        USBD_CtlSendData (pdev, (uint8_t *)&USBD_UVC_CfgFSDesc.uvc_vc_if_desc, MIN(sizeof(USBD_UVC_CfgFSDesc), req->wLength));
      } else {
        USBD_CtlSendData (pdev, (uint8_t *)hcdc->data, req->wLength);
      }
      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev, &ifalt, 1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) < VIDEO_TOTAL_IF_NUM) {
        ifalt = (uint8_t)(req->wValue);
        // TODO: refactor this to callback user code instead of doing this here
        if (ifalt == 1) {
          g_uvc_stream_status = 1;
        } else {
          g_uvc_stream_status = 0;
        }
      } else  {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
        return USBD_FAIL;
      }
      break;
    default:
        return USBD_FAIL;
    }
    break;
 
  default: 
    return USBD_FAIL;
    break;
  }
  return ret;
}

/**
  * @brief  USBD_UVC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_UVC_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_UVC_HandleTypeDef   *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL) {
    hcdc->TxState = 0;
    return USBD_OK;
  } else {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_UVC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_UVC_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  USBD_UVC_HandleTypeDef   *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  /* Get the received data length */
  hcdc->RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
  
  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the application Xfer */
  if(pdev->pClassData != NULL) {
    ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->Receive(hcdc->RxBuffer, &hcdc->RxLength);
    return USBD_OK;
  } else {
    return USBD_FAIL;
  }
}

static uint8_t  USBD_UVC_SOF (USBD_HandleTypeDef *pdev)
{     
  if (g_uvc_stream_status == 1) {
      USBD_LL_FlushEP(pdev, USB_ENDPOINT_IN(1));
      USBD_LL_Transmit(pdev, USB_ENDPOINT_IN(1), (uint8_t*)0x0002, 2);//header
      g_uvc_stream_status = 2;
  }
  return USBD_OK;
}


/**
  * @brief  USBD_UVC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_UVC_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  uint8_t ret = USBD_OK;
  USBD_UVC_HandleTypeDef   *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  if((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFF))
  {
    uint8_t address = (hcdc->CmdIndex >> 0) & 0xff;
    uint8_t entity_id = (hcdc->CmdIndex >> 8) & 0xff;

    if (address == USB_UVC_VSIF_NUM) {
      ret = ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->VS_CtrlSet(hcdc->CmdOpCode,
              (uint8_t *)hcdc->data, hcdc->CmdLength, hcdc->CmdIndex, hcdc->CmdValue);
    } else {
    ret = ((USBD_UVC_ItfTypeDef *)pdev->pUserData)->ControlSet(entity_id, hcdc->CmdOpCode,
            (uint8_t *)hcdc->data, hcdc->CmdLength, hcdc->CmdIndex, hcdc->CmdValue);
    }

    hcdc->CmdOpCode = 0xFF;
  }
  return ret;
}

/**
  * @brief  USBD_UVC_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_UVC_GetFSCfgDesc (uint16_t *length)
{
  if (sensor_get_id(&sensor) != MT9V034_ID) {
      USBD_UVC_CfgFSDesc.uvc_vs_frames_formats_desc = uvc_vs_frames_formats_desc;
  } else {
      USBD_UVC_CfgFSDesc.uvc_vs_frames_formats_desc = uvc_vs_frames_formats_desc_gs;
  }
  *length = sizeof(USBD_UVC_CfgFSDesc);
  return (uint8_t*)&USBD_UVC_CfgFSDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_UVC_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_UVC_DeviceQualifierDesc);
  return USBD_UVC_DeviceQualifierDesc;
}

/**
* @brief  USBD_UVC_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_UVC_RegisterInterface  (USBD_HandleTypeDef   *pdev, USBD_UVC_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;
  if(fops != NULL) {
    pdev->pUserData= fops;
    ret = USBD_OK;    
  }
  return ret;
}

/**
  * @brief  USBD_UVC_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t  USBD_UVC_SetTxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff,
                                uint16_t length)
{
  USBD_UVC_HandleTypeDef   *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  hcdc->TxBuffer = pbuff;
  hcdc->TxLength = length;  
  
  return USBD_OK;  
}


/**
  * @brief  USBD_UVC_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_UVC_SetRxBuffer  (USBD_HandleTypeDef   *pdev,
                                   uint8_t  *pbuff)
{
  USBD_UVC_HandleTypeDef   *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  hcdc->RxBuffer = pbuff;
  
  return USBD_OK;
}

/**
  * @brief  USBD_UVC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  USBD_UVC_TransmitPacket(USBD_HandleTypeDef *pdev)
{      
  USBD_UVC_HandleTypeDef   *hcdc = (USBD_UVC_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
    if(hcdc->TxState == 0)
    {
      /* Tx Transfer in progress */
      hcdc->TxState = 1;
      
      /* Transmit next packet */
      USBD_LL_Transmit(pdev,
                       UVC_IN_EP,
                       hcdc->TxBuffer,
                       hcdc->TxLength);
      
      return USBD_OK;
    }
    else
    {
      return USBD_BUSY;
    }
  }
  else
  {
    return USBD_FAIL;
  }
}


/**
  * @brief  USBD_UVC_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_UVC_ReceivePacket(USBD_HandleTypeDef *pdev)
{      
  /* Suspend or Resume USB Out process */
  if(pdev->pClassData != NULL)
  {
    /* Prepare Out endpoint to receive next packet */
    // USBD_LL_PrepareReceive(pdev,
    //                        UVC_OUT_EP,
    //                        hcdc->RxBuffer,
    //                        UVC_DATA_FS_OUT_PACKET_SIZE);
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
