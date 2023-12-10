/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @brief          :
  ******************************************************************************
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_uvc_if.h"
#include "uvc_desc.h"

/* Define size for the receive and transmit buffer over UVC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  1024
#define APP_TX_DATA_SIZE  1024

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Send Data over USB UVC are stored in this buffer       */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USB handle declaration */
USBD_HandleTypeDef  *hUsbDevice_0;

extern USBD_HandleTypeDef hUsbDeviceFS;

//data array for Video Probe and Commit
__ALIGN_BEGIN struct uvc_streaming_control videoCommitControl __ALIGN_END =
{
  .bmHint = 0x00,
  .bFormatIndex = VS_FMT_INDEX(GREY),
  .bFrameIndex = 0x01,
  .dwFrameInterval = 0, 
  .wKeyFrameRate = 0,
  .wPFrameRate = 0,
  .wCompQuality = 0,
  .wCompWindowSize = 0,
  .wDelay = 0,
  .dwMaxVideoFrameSize = MAX_FRAME_SIZE(640, 480, 8),
  .dwMaxPayloadTransferSize = VIDEO_PACKET_SIZE,
  .dwClockFrequency = 0,
  .bmFramingInfo = 0,
  .bPreferedVersion = 0,
  .bMinVersion = 0,
  .bMaxVersion = 0,
};

__ALIGN_BEGIN struct uvc_streaming_control videoProbeControl __ALIGN_END =
{
  .bmHint = 0x00,
  .bFormatIndex = VS_FMT_INDEX(GREY),
  .bFrameIndex = 0x01,
  .dwFrameInterval = 0, 
  .wKeyFrameRate = 0,
  .wPFrameRate = 0,
  .wCompQuality = 0,
  .wCompWindowSize = 0,
  .wDelay = 0,
  .dwMaxVideoFrameSize = MAX_FRAME_SIZE(640, 480, 8),
  .dwMaxPayloadTransferSize = VIDEO_PACKET_SIZE,
  .dwClockFrequency = 0,
  .bmFramingInfo = 0,
  .bPreferedVersion = 0,
  .bMinVersion = 0,
  .bMaxVersion = 0,
};

static int8_t UVC_Init_FS (void);
static int8_t UVC_DeInit_FS (void);
static int8_t UVC_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t index, uint16_t value);
static int8_t UVC_VC_ControlGet (VC_TERMINAL_ID entity_id,
        uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t index, uint16_t value);
static int8_t UVC_VC_ControlSet (VC_TERMINAL_ID entity_id,
        uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t index, uint16_t value);
static int8_t UVC_VS_ControlGet(uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t index, uint16_t value);
static int8_t UVC_VS_ControlSet(uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t index, uint16_t value);
static int8_t UVC_Receive_FS  (uint8_t* pbuf, uint32_t *Len);

USBD_UVC_ItfTypeDef USBD_Interface_fops_FS = {
  .Init = UVC_Init_FS,
  .DeInit = UVC_DeInit_FS,
  .Control = UVC_Control,
  .VS_CtrlGet = UVC_VS_ControlGet,
  .VS_CtrlSet = UVC_VS_ControlSet,
  .ControlGet = UVC_VC_ControlGet,
  .ControlSet = UVC_VC_ControlSet,
  .Receive = UVC_Receive_FS,
};

/**
  * @brief  UVC_Init_FS
  *         Initializes the UVC media low layer over the FS USB IP
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t UVC_Init_FS(void)
{
  hUsbDevice_0 = &hUsbDeviceFS;
  /* Set Application Buffers */
  USBD_UVC_SetTxBuffer(hUsbDevice_0, UserTxBufferFS, 0);
  USBD_UVC_SetRxBuffer(hUsbDevice_0, UserRxBufferFS);
  return (USBD_OK);
}

/**
  * @brief  UVC_DeInit_FS
  *         DeInitializes the UVC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t UVC_DeInit_FS(void)
{
  return (USBD_OK);
}

static int8_t UVC_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t idx, uint16_t value)
{
  return USBD_OK;
}

/**
  * @brief  UVC_Control_FS
  *         Manage the UVC class requests
  * @param  cmd: Command code            
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t UVC_VC_ControlGet(VC_TERMINAL_ID entity_id, uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t idx, uint16_t value)
{ 
  uint8_t cs_value = (value >> 8) & 0xFF;
  (void) cs_value; //TODO

  switch (entity_id) {
  case VC_CONTROL_XU_LEP_AGC_ID:
  case VC_CONTROL_XU_LEP_OEM_ID:
  case VC_CONTROL_XU_LEP_RAD_ID:
  case VC_CONTROL_XU_LEP_RAD_2_ID:
  case VC_CONTROL_XU_LEP_SYS_ID:
  case VC_CONTROL_XU_LEP_VID_ID:
  case VC_CONTROL_XU_LEP_CUST_ID:
    memset(pbuf, 0, length);
    switch (cmd) {
    case UVC_GET_DEF:
    case UVC_GET_MIN:
      break;
    case UVC_GET_CUR:
      if (length > 1) {
        //VC_LEP_GetAttribute(entity_id, (cs_value - 1) << 2, pbuf, length);
      }
      break;
    case UVC_GET_MAX:
      //VC_LEP_GetMaxValue(entity_id, (cs_value - 1) << 2, pbuf, length);
      break;
    case UVC_GET_RES:
      pbuf[0] = 1;
      break;
    case UVC_GET_LEN:
      //VC_LEP_GetAttributeLen(entity_id, (cs_value - 1) << 2, (uint16_t*)pbuf);
      break;
    case UVC_GET_INFO:
      pbuf[0] = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET;
      break;
    default:
      return USBD_FAIL;
    }

    break;

  case VC_CONTROL_PU_ID:
    switch (cmd) {
    case UVC_GET_MIN:
      pbuf[0] = 0;
      break;
    case UVC_GET_DEF:
    case UVC_GET_CUR:
      pbuf[0] = 128;
      break;
    case UVC_GET_MAX:
      pbuf[0] = 255;
      break;
    case UVC_GET_RES:
      pbuf[0] = 1;
      break;
    case UVC_GET_LEN:
      pbuf[0] = 2;
      break;
    case UVC_GET_INFO:
      pbuf[0] = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_DISABLED;
      break;
    default:
      return USBD_FAIL;
    }

    break;

  default:
    return USBD_FAIL;
  }

  return (USBD_OK);
}

static int8_t UVC_VC_ControlSet(VC_TERMINAL_ID entity_id, uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t idx, uint16_t value)
{ 
  uint8_t cs_value = (value >> 8) & 0xFF;
  (void) cs_value;

  switch (cmd) {
  case UVC_SET_CUR:
    break;
  default:
    return USBD_FAIL;
  }

  switch (entity_id) {
  case VC_CONTROL_XU_LEP_AGC_ID:
  case VC_CONTROL_XU_LEP_OEM_ID:
  case VC_CONTROL_XU_LEP_RAD_ID:
  case VC_CONTROL_XU_LEP_RAD_2_ID:
  case VC_CONTROL_XU_LEP_SYS_ID:
  case VC_CONTROL_XU_LEP_VID_ID:
  case VC_CONTROL_XU_LEP_CUST_ID:
    if (length == 1) {
      //VC_LEP_RunCommand(entity_id, (cs_value - 1) << 2);
    } else {
      //VC_LEP_SetAttribute(entity_id, (cs_value - 1) << 2, pbuf, length);
    }
    break;
  case VC_CONTROL_PU_ID:
    break;
  default:
    return USBD_FAIL;
  }

  return (USBD_OK);
}


/**
  * @brief  UVC_Control_FS
  *         Manage the UVC class requests
  * @param  cmd: Command code            
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t UVC_VS_ControlGet(uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t idx, uint16_t value)
{ 
  uint8_t cs_value = (value >> 8) & 0xFF;

  switch (cmd) {
  case UVC_GET_DEF:
  case UVC_GET_CUR:
  case UVC_GET_MIN:
  case UVC_GET_MAX:
    if(cs_value == UVC_VS_PROBE_CONTROL || cs_value == UVC_VS_STILL_PROBE_CONTROL) {
      struct uvc_streaming_control *rtnBuf = (struct uvc_streaming_control*)pbuf;
      memset(rtnBuf, 0, length);
      rtnBuf->bFormatIndex = videoProbeControl.bFormatIndex;
      rtnBuf->bFrameIndex = videoProbeControl.bFrameIndex;
      rtnBuf->dwMaxPayloadTransferSize = VIDEO_PACKET_SIZE;
      rtnBuf->dwFrameInterval = INTERVAL;

      if (cmd == UVC_GET_DEF || cmd == UVC_GET_MIN || cmd == UVC_GET_MAX || cmd == UVC_GET_CUR) {
        struct uvc_vs_frames_formats_descriptor *frames_formats;
        struct UVC_FRAME_UNCOMPRESSED(1) *frame;
        frames_formats = &USBD_UVC_CfgFSDesc.uvc_vs_frames_formats_desc;

        switch (videoProbeControl.bFormatIndex) {
        case VS_FMT_INDEX(YUYV):
          frame = frames_formats->uvc_vs_frames_format_1.uvc_vs_frame;
          break;
        case VS_FMT_INDEX(GREY):
          frame = frames_formats->uvc_vs_frames_format_2.uvc_vs_frame;
          break;
        case VS_FMT_INDEX(RGB565):
          frame = frames_formats->uvc_vs_frames_format_3.uvc_vs_frame;
          break;
        default:
          return USBD_FAIL;
        }
        rtnBuf->dwMaxVideoFrameSize = frame[videoProbeControl.bFrameIndex - 1].dwMaxVideoFrameBufferSize;
      } else {
        return USBD_FAIL;
      }
    } else if (cs_value == UVC_VS_COMMIT_CONTROL || cs_value == UVC_VS_STILL_COMMIT_CONTROL) {
      // Commit Request
      memcpy(pbuf, &videoCommitControl, MIN(sizeof(struct uvc_streaming_control), length));
    } else {
      return USBD_FAIL;
    }
    break;

  default:
    return USBD_FAIL;
  }

  return (USBD_OK);
}

static int8_t UVC_VS_ControlSet(uint8_t cmd, uint8_t* pbuf, uint16_t length, uint16_t idx, uint16_t value)
{ 
  uint8_t cs_value = (value >> 8) & 0xFF;

  switch (cmd) {
  case UVC_SET_CUR: {
    struct uvc_streaming_control *rtnBuf = (struct uvc_streaming_control*)pbuf;
    if (rtnBuf->bFormatIndex > VS_NUM_FORMATS) {
      return USBD_FAIL;
    }

    if(cs_value == UVC_VS_PROBE_CONTROL || cs_value == UVC_VS_STILL_PROBE_CONTROL) {
      memcpy(&videoProbeControl, pbuf, MIN(sizeof(struct uvc_streaming_control), length));
    } else if (cs_value == UVC_VS_COMMIT_CONTROL || cs_value == UVC_VS_STILL_COMMIT_CONTROL) {
      memcpy(&videoCommitControl, pbuf, MIN(sizeof(struct uvc_streaming_control), length));
    } else {
      return USBD_FAIL;
    }
    break;
  }
  default:
    return USBD_FAIL;
  }

  return (USBD_OK);
}

/**
  * @brief  UVC_Receive_FS
  *         Data received over USB OUT endpoint are sent over UVC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         until exiting this function. If you exit this function before transfer
  *         is complete on UVC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t UVC_Receive_FS (uint8_t* Buf, uint32_t *Len)
{
  return (USBD_OK);
}

/**
  * @brief  UVC_Transmit_FS
  *         Data send over USB IN endpoint are sent over UVC interface 
  *         through this function.           
  *         @note
  *         
  *                 
  * @param  Buf: Buffer of data to be send
  * @param  Len: Number of data to be send (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t UVC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
  USBD_UVC_SetTxBuffer(hUsbDevice_0, Buf, Len);   
  result = USBD_UVC_TransmitPacket(hUsbDevice_0);
  HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
  return result;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
