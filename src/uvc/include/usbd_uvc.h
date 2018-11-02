/**
  ******************************************************************************
  * @file    usbd_cdc.h
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   header file for the usbd_cdc.c file.
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
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_UVC_H
#define __USB_UVC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"
#include "uvc.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_cdc.c
  * @{
  */ 


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */ 

// USB Video device class specification version 1.10
#ifdef UVC_1_1
#define UVC_VERSION                             0x0110      // UVC 1.1
#else
#define UVC_VERSION                             0x0100      // UVC 1.0
#endif

#define UVC_IN_EP                               0x81  /* EP1 for data IN */
#define UVC_CMD_EP                              0x82  /* EP2 for UVC commands */
#define VIDEO_PACKET_SIZE                       ((unsigned int)(960 + 2))
#define VIDEO_MAX_SETUP_PACKET_SIZE             ((unsigned int)(1024))
#define CMD_PACKET_SIZE                         ((unsigned int)(8))

#define CAM_FPS                                 120

enum CUST_COMTROL_IDS {
	CUST_CONTROL_COMMAND=0,
	CUST_CONTROL_GET,
	CUST_CONTROL_SET,
	CUST_CONTROL_RUN,
	CUST_CONTROL_DIRECT_WRITE,
	CUST_CONTROL_DIRECT_READ,
	CUST_CONTROL_END
};

enum _vs_fmt_indexes {
  VS_FMT_INDEX_GREY = 1,
  VS_FMT_INDEX_YUYV,
  VS_FMT_INDEX_RGB565,
};

#define VS_NUM_FORMATS 3

enum _vs_fmt_size {
  VS_FMT_SIZE_YUYV   = 16,
  VS_FMT_SIZE_GREY   =  8,
  VS_FMT_SIZE_RGB565 = 16,
};

#define VS_FMT_GUID_NONE \
    '0',  '0',  '0',  '0', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#define VS_FMT_GUID_GREY \
    'Y',  '8',  ' ',  ' ', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71

#define VS_FMT_GUID_Y16 \
    'Y',  '1',  '6',  ' ', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71

#define VS_FMT_GUID_YUYV \
    'U',  'Y',  'V',  'Y', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71

#define VS_FMT_GUID_NV12 \
    'N',  'V',  '1',  '2', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71

#define VS_FMT_GUID_YU12 \
    'I',  '4',  '2',  '0', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71

#define VS_FMT_GUID_BGR3 \
    0x7d, 0xeb, 0x36, 0xe4, 0x4f, 0x52, 0xce, 0x11, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70

#define VS_FMT_GUID_RGB565 \
    'R',  'G',  'B',  'P', 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71

#define VS_FMT_INDEX(NAME) VS_FMT_INDEX_ ## NAME
#define VS_FMT_GUID(NAME) { VS_FMT_GUID_ ## NAME }
#define VS_FMT_SIZE(NAME) VS_FMT_SIZE_ ## NAME

enum _vs_frame_indexes {
  VS_FRAME_INDEX_1 = 1,
  VS_FRAME_INDEX_2 = 2,
  VS_FRAME_INDEX_3 = 3,
  VS_FRAME_INDEX_4 = 4,
};

#define VS_FRAME_INDEX(NAME) VS_FRAME_INDEX_ ## NAME

#define USB_UVC_VCIF_NUM                              0
#define USB_UVC_VSIF_NUM                              (char)1

#define VIDEO_TOTAL_IF_NUM                            2
#define WINDOWS_MAX_CONTROLS 31

typedef enum _vs_terminal_id {
  VC_INPUT_TERMINAL_ID = 1,
  VC_CONTROL_PU_ID,
  VC_CONTROL_XU_LEP_AGC_ID,
  VC_CONTROL_XU_LEP_OEM_ID,
  VC_CONTROL_XU_LEP_RAD_ID,
  VC_CONTROL_XU_LEP_SYS_ID,
  VC_CONTROL_XU_LEP_VID_ID,
  VC_CONTROL_XU_LEP_AGC_2_ID = VC_CONTROL_XU_LEP_AGC_ID + 0x10,
  VC_CONTROL_XU_LEP_OEM_2_ID,
  VC_CONTROL_XU_LEP_RAD_2_ID,
  VC_CONTROL_XU_LEP_SYS_2_ID,
  VC_CONTROL_XU_LEP_VID_2_ID,
  VC_CONTROL_XU_LEP_CUST_ID = 0xfe,
  VC_OUTPUT_TERMINAL_ID = 0xff
} VC_TERMINAL_ID;

/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */


typedef struct _USBD_UVC_Itf
{
  int8_t (* Init)          (void);
  int8_t (* DeInit)        (void);
  int8_t (* Control)       (uint8_t, uint8_t * , uint16_t, uint16_t, uint16_t);
  int8_t (* VS_CtrlGet)    (uint8_t, uint8_t * , uint16_t, uint16_t, uint16_t);
  int8_t (* VS_CtrlSet)    (uint8_t, uint8_t * , uint16_t, uint16_t, uint16_t);
  int8_t (* ControlGet)    (VC_TERMINAL_ID, uint8_t, uint8_t * , uint16_t, uint16_t, uint16_t);
  int8_t (* ControlSet)    (VC_TERMINAL_ID, uint8_t, uint8_t * , uint16_t, uint16_t, uint16_t);
  int8_t (* Receive)       (uint8_t *, uint32_t *);  

}USBD_UVC_ItfTypeDef;


typedef struct
{
  uint32_t data[VIDEO_MAX_SETUP_PACKET_SIZE/4];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint16_t CmdLength;    
  uint16_t CmdIndex;
  uint16_t CmdValue;
  uint8_t  *RxBuffer;  
  uint8_t  *TxBuffer;   
  uint32_t RxLength;
  uint32_t TxLength;    
  
  __IO uint32_t TxState;     
  __IO uint32_t RxState;    
}
USBD_UVC_HandleTypeDef; 

#if 0
struct uvc_input_terminal_desc {
  uint8_t bLength;
  uint8_t bDescriptorType; /* : CS_INTERFACE */
  uint8_t bDescriptorSubtype; /* VC_INPUT_TERMINAL descriptor subtype */
  uint8_t bTerminalID; /* Constant uniquely identifying the Terminal within the video function.
    This value is used in all requests to address this Terminal. */
  uint8_t[2] wTerminalType; /* ITT_MEDIA_TRANSPORT_INPUT */
  uint8_t bAssocTerminal; /* ID of the Output Terminal to which this Input  Terminal is associated. */
  uint8_t iTerminal; /* */
  uint8_t bControlSize; /* Size of the bmControls field, in bytes: n=1 */
  uint8_t[1] bmControls; /*
    A bit set to 1 indicates that the mentioned Control is supported for the video stream.
    D0: Transport Control
    D1: Absolute Track Number Control
    D2: Media Information
    D3: Time Code Information D4...(n*8-1): Reserved
  */
  uint8_t bTransportModeSize; /* Size of the bmTransportModes field, in bytes: m=4 */
  uint8_t[4] bmTransportModes; /*
    A bit set to 1 indicates that the mentioned Transport mode is supported.
    D0: Play Forward
    D1: Pause
    D2: Rewind
    D3: Fast Forward
    D4: High Speed Rewind
    D5: Stop
    D6: Eject
    D7: Play Next Frame
    D8: Play Slowest Forward
    D9: Play Slow Forward 4
    D10: Play Slow Forward 3
    D11: Play Slow Forward 2
    D12: Play Slow Forward 1
    D13: Play X1
    D14: Play Fast Forward 1
    D15: Play Fast Forward 2
    D16: Play Fast Forward 3
    D17: Play Fast Forward 4
    D18: Play Fastest Forward
    D19: Play Previous Frame
    D20: Play Slowest Reverse
    D21: Play Slow Reverse 4
    D22: Play Slow Reverse 3
    D23: Play Slow Reverse 2
    D24: Play Slow Reverse 1
    D25: Play X1 Reverse
    D26: Play Fast Reverse 1
    D27: Play Fast Reverse 2
    D28: Play Fast Reverse 3
    D29: Play Fast Reverse 4
    D30: Play Fastest Reverse
    D31: Record StateStart
    */
};
#endif

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

#define SIZEOF_M(type, member) sizeof(((type *)0)->member)

#define UVC_DATA_HS_IN_PACKET_SIZE                    VIDEO_PACKET_SIZE
#define UVC_DATA_HS_OUT_PACKET_SIZE                   VIDEO_PACKET_SIZE

#define UVC_DATA_FS_IN_PACKET_SIZE                    VIDEO_PACKET_SIZE
#define UVC_DATA_FS_OUT_PACKET_SIZE                   VIDEO_PACKET_SIZE

#define UVC_LEN_IF_ASSOCIATION_DESC                   (char)8

#define UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(a,b)  (char) (13+a*b)

#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE           8
#define USB_OTG_DESCRIPTOR_TYPE                       9
#define USB_DEBUG_DESCRIPTOR_TYPE                     10
#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE     11

/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                       0xC0
#define USB_CONFIG_BUS_POWERED                        0x80

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                       ((mA)/2)
#define MAX_FRAME_SIZE(width, height, bits_per_pixel) ((unsigned long)((width*height*bits_per_pixel)>>3))
#define MIN_BIT_RATE(width, height, bits_per_pixel)   ((unsigned long)(width*height*bits_per_pixel*CAM_FPS))
#define MAX_BIT_RATE(width, height, bits_per_pixel)   ((unsigned long)(width*height*bits_per_pixel*CAM_FPS))
#define INTERVAL                                      ((unsigned long)(10000000/CAM_FPS))

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK                   0x80
#define USB_ENDPOINT_OUT(addr)                        ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                         ((addr) | 0x80)

#define UVC_FORMAT_UNCOMPRESSED_DESCRIPTOR(FMT_NAME, NUM_FRAME_DESCS) { \
  .bLength = UVC_DT_FORMAT_UNCOMPRESSED_SIZE, \
  .bDescriptorType = UVC_CS_INTERFACE,  /* CS_INTERFACE */ \
  .bDescriptorSubType = UVC_VS_FORMAT_UNCOMPRESSED, /* VS_FORMAT_UNCOMPRESSED subtype */ \
  .bFormatIndex = VS_FMT_INDEX(FMT_NAME), /* First format descriptor */ \
  .bNumFrameDescriptors = NUM_FRAME_DESCS, /* One frame descriptor for this format follows. */ \
  .guidFormat = VS_FMT_GUID(FMT_NAME),  /* */ \
  .bBitsPerPixel = VS_FMT_SIZE(FMT_NAME), /* Number of bits per pixel used to specify color in the decoded video frame - 16 for yuy2, 12 for nv12... */ \
  .bDefaultFrameIndex = 0x01,           /* Default frame index is 1. */ \
  .bAspectRatioX = 0x00,                /* Non-interlaced stream not required. */ \
  .bAspectRatioY = 0x00,                /* Non-interlaced stream not required. */ \
  .bmInterlaceFlags = 0x00,             /* Non-interlaced stream */ \
  .bCopyProtect = 0x00,                 /* No restrictions imposed on the duplication of this video stream. */ \
}

#define UVC_FRAME_FORMAT(FRAME_INDEX, FMT_NAME, WIDTH, HEIGHT) { \
  .bLength = UVC_DT_FRAME_UNCOMPRESSED_SIZE(1), \
  .bDescriptorType = UVC_CS_INTERFACE,  /* CS_INTERFACE */ \
  .bDescriptorSubType = UVC_VS_FRAME_UNCOMPRESSED, /* VS_FRAME_UNCOMPRESSED */ \
  .bFrameIndex = FRAME_INDEX,           /* First (and only) frame descriptor */ \
  .bmCapabilities = 0x02,               /* Still images using capture method 0 are supported at this frame setting.D1: Fixed frame-rate. */ \
  .wWidth = WIDTH,                      /* Width of frame is 128 pixels. */ \
  .wHeight = HEIGHT,                    /* Height of frame is 64 pixels. */ \
  .dwMinBitRate = MIN_BIT_RATE(WIDTH,HEIGHT,VS_FMT_SIZE(FMT_NAME)), /* Min bit rate in bits/s  */ \
  .dwMaxBitRate = MAX_BIT_RATE(WIDTH,HEIGHT,VS_FMT_SIZE(FMT_NAME)), /* Max bit rate in bits/s  */ \
  .dwMaxVideoFrameBufferSize = MAX_FRAME_SIZE(WIDTH,HEIGHT,VS_FMT_SIZE(FMT_NAME)), /* Maximum video or still frame size, in bytes. */ \
  .dwDefaultFrameInterval = INTERVAL,   /* */ \
  .bFrameIntervalType = 0x01,           /* Continuous frame interval */ \
  .dwFrameInterval = { INTERVAL },      /* 1,000,000 ns  *100ns -> 10 FPS */ \
}

#define UVC_COLOR_MATCHING_DESCRIPTOR() { \
  .bLength = UVC_DT_COLOR_MATCHING_SIZE, \
  .bDescriptorType = UVC_CS_INTERFACE,  /* CS_INTERFACE */ \
  .bDescriptorSubType = UVC_VS_COLORFORMAT, /* VS_COLORFORMAT */ \
  .bColorPrimaries = 0x01,              /* 1: BT.709, sRGB (default) */ \
  .bTransferCharacteristics = 0x01,     /* 1: BT.709 (default) */ \
  .bMatrixCoefficients = 0x04,          /* 1: SMPTE 170M (BT.601) */ \
}

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_UVC;
#define USBD_UVC_CLASS    &USBD_UVC
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_UVC_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_UVC_ItfTypeDef *fops);

uint8_t  USBD_UVC_SetTxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff,
                                      uint16_t length);

uint8_t  USBD_UVC_SetRxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff);
  
uint8_t  USBD_UVC_ReceivePacket      (USBD_HandleTypeDef *pdev);

uint8_t  USBD_UVC_TransmitPacket     (USBD_HandleTypeDef *pdev);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_UVC_H */
 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
