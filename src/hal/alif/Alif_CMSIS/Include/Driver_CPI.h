/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     Driver_CPI.h
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     03-May-2023
 * @brief    Camera Controller Driver definitions.
 ******************************************************************************/

#ifndef DRIVER_CPI_H_
#define DRIVER_CPI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "Driver_Common.h"

#define ARM_CPI_API_VERSION                                        ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/****** CPI Control Codes *****/
#define CPI_SOFTRESET                                              (0x01UL) ///< CPI Software Reset; arg: 0=disable, 1=enable
#define CPI_CAMERA_SENSOR_CONFIGURE                                (0x02UL) ///< CAMERA SENSOR configure; arg: 0=disable, 1=enable
#define CPI_EVENTS_CONFIGURE                                       (0x03UL) ///< CAMERA EVENTS configure; arg: list of events to enable (ARM_CPI_EVENT_*)
#define CPI_CAMERA_SENSOR_GAIN                                     (0x04UL) ///< CAMERA SENSOR gain set; arg: 0x10000 * gain, 0=read only. Returns current/updated gain if no error.
#define CPI_CONFIGURE                                              (0x05UL) ///< CPI configure
#define CPI_CAMERA_SENSOR_AE                                       (0x06UL) ///< CAMERA SENSOR AE; arg: 0=disable, 1=enable
#define CPI_CAMERA_SENSOR_AE_TARGET_LUMA                           (0x07UL) ///< CAMERA SENSOR AE Tagret LUMA; arg: Value for target luminance

/****** CPI Events *****/
#define ARM_CPI_EVENT_CAMERA_CAPTURE_STOPPED                       (1UL << 0) ///< Camera Capture Stopped
#define ARM_CPI_EVENT_CAMERA_FRAME_HSYNC_DETECTED                  (1UL << 1) ///< Camera Frame VSYNC Detected for incoming frame
#define ARM_CPI_EVENT_CAMERA_FRAME_VSYNC_DETECTED                  (1UL << 2) ///< Camera Frame VSYNC Detected for incoming frame
#define ARM_CPI_EVENT_ERR_CAMERA_INPUT_FIFO_OVERRUN                (1UL << 3) ///< Camera FIFO over run Error
#define ARM_CPI_EVENT_ERR_CAMERA_OUTPUT_FIFO_OVERRUN               (1UL << 4) ///< Camera FIFO under run Error
#define ARM_CPI_EVENT_ERR_HARDWARE                                 (1UL << 5) ///< Hardware Bus Error
#define ARM_CPI_EVENT_MIPI_CSI2_ERROR                              (1UL << 6) ///< MIPI CSI2 Error

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION GetVersion (void)
  \brief       Get CPI driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_CPI_CAPABILITIES GetCapabilities (void)
  \brief       Get CPI driver capabilities
  \return      \ref ARM_CPI_CAPABILITIES

  \fn          int32_t Initialize (CAMERA_RESOLUTION camera_resolution,
                                   ARM_CPI_SignalEvent_t cb_event)
  \brief       Initialize CPI and Camera Sensor Device Interface.
  \param[in]   cb_event          : Pointer to \ref ARM_CPI_SignalEvent_t
  \return      \ref execution_status

  \fn          int32_t Uninitialize (void)
  \brief       De-initialize CPI and Camera Sensor Device Interface.
  \return      \ref execution_status

  \fn          int32_t PowerControl (ARM_POWER_STATE state)
  \brief       Control CPI Interface Power.
  \param[in]   state : Power state
  \return      \ref execution_status

  \fn          int32_t CaptureFrame (void *framebuffer_startaddr)
  \brief       Start CPI in Snapshot mode and Camera Sensor Device Interface.
                In Snapshot mode, CPI will capture one frame then it gets stop.
  \param[in]   framebuffer_startaddr : Pointer to frame buffer start address, where camera captured image will be stored.
  \return      \ref execution_status

  \fn          int32_t CaptureVideo (void *framebuffer_startaddr)
  \brief       Start CPI in Video mode and Camera Sensor Device Interface.
                In Video mode, CPI will capture video data continuously.
  \param[in]   framebuffer_startaddr : Pointer to frame buffer start address, where camera captured video data will be stored.
  \return      \ref execution_status


  \fn          int32_t Stop (void)
  \brief       Stop CPI and Camera Sensor Device Interface.
  \return      \ref execution_status

  \fn          int32_t Control (uint32_t control, uint32_t arg)
  \brief       Control CPI and Camera Sensor Device Interface.
  \param[in]   control : Operation
  \param[in]   arg     : Argument of operation (optional)
  \return      common \ref execution_status
*/

typedef void (*ARM_CPI_SignalEvent_t) (uint32_t event);  ///< Pointer to \ref ARM_CPI_SignalEvent_t : Signal CPI Event.

/**
\brief CPI Driver Capabilities.
*/
typedef struct _ARM_CPI_CAPABILITIES {
  uint32_t snapshot           : 1;        ///< Supports CPI Snapshot mode, In this mode CPI will capture one frame then it gets stop.
  uint32_t video              : 1;        ///< Supports CPI video mode
  uint32_t reserved           : 30;       ///< Reserved (must be zero)
} ARM_CPI_CAPABILITIES;


/**
\brief Access structure of the CPI Driver.
*/
typedef struct _ARM_DRIVER_CPI{
  ARM_DRIVER_VERSION                  (*GetVersion)      (void);                                           ///< Pointer to \ref CPI_GetVersion      : Get driver version.
  ARM_CPI_CAPABILITIES                (*GetCapabilities) (void);                                           ///< Pointer to \ref CPI_GetCapabilities : Get driver capabilities.
  int32_t                             (*Initialize)      (ARM_CPI_SignalEvent_t cb_event);                 ///< Pointer to \ref CPI_Initialize      : Initialize CPI Interface.
  int32_t                             (*Uninitialize)    (void);                                           ///< Pointer to \ref CPI_Uninitialize    : De-initialize CPI Interface.
  int32_t                             (*PowerControl)    (ARM_POWER_STATE state);                          ///< Pointer to \ref CPI_PowerControl    : Control CPI Interface Power.
  int32_t                             (*CaptureFrame)    (void *framebuffer_startaddr);                    ///< Pointer to \ref CPI_StartSnapshot   : Start CPI Interface in Snapshot mode.
  int32_t                             (*CaptureVideo)    (void *framebuffer_startaddr);                    ///< Pointer to \ref CPI_CaptureVideo    : Start CPI Interface in Video mode.
  int32_t                             (*Stop)            (void);                                           ///< Pointer to \ref CPI_Stop            : Stop  CPI Interface.
  int32_t                             (*Control)         (uint32_t control, uint32_t arg);                 ///< Pointer to \ref CPI_Control         : Control CPI Interface.
} const ARM_DRIVER_CPI;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_CPI_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
