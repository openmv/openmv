/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
/**************************************************************************//**
 * @file     Driver_MIPI_CSI2.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     24-March-2022
 * @brief    Driver Specific Header file for MIPI CSI2 Driver.
 ******************************************************************************/

#ifndef DRIVER_MIPI_CSI2_H_
#define DRIVER_MIPI_CSI2_H_

#include "Driver_Common.h"

#define ARM_MIPI_CSI2_API_VERSION     ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /* API version */

/**
\brief MIPI CSI2 event types
*/
#define CSI2_EVENT_PHY_FATAL          (1U << 0)                           ///< PHY Packet discarded event
#define CSI2_EVENT_PKT_FATAL          (1U << 1)                           ///< Packet fatal
#define CSI2_EVENT_PHY                (1U << 2)                           ///< PHY events
#define CSI2_EVENT_LINE               (1U << 3)                           ///< Line events
#define CSI2_EVENT_IPI_FATAL          (1U << 4)                           ///< Image pixel interface events
#define CSI2_EVENT_BNDRY_FRAME_FATAL  (1U << 5)                           ///< Boundary frame events
#define CSI2_EVENT_SEQ_FRAME_FATAL    (1U << 6)                           ///< Frame sequance events
#define CSI2_EVENT_CRC_FRAME_FATAL    (1U << 7)                           ///< CRC event
#define CSI2_EVENT_PLD_CRC_FATAL      (1U << 8)                           ///< Payload CRC event
#define CSI2_EVENT_DATA_ID            (1u << 9)                           ///< Pixel Data ID event
#define CSI2_EVENT_ECC_CORRECT        (1U << 10)                          ///< ECC event
// Function documentation
/*
  \fn          ARM_DRIVER_VERSION MIPI_CSI2_GetVersion (void)
  \brief       Get MIPI CSI2 driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_MIPI_CSI2_CAPABILITIES MIPI_CSI2_GetCapabilities (void)
  \brief       Get MIPI CSI2 driver capabilities.
  \return      \ref ARM_MIPI_DPHY_CAPABILITIES

  \fn          int32_t  MIPI_CSI2_Initialize (ARM_MIPI_CSI2_SignalEvent_t cb_event)
  \brief       Initialize MIPI CSI2 Interface.
  \param[in]   cb_event Pointer to ARM_MIPI_CSI2_SignalEvent_t
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_Uninitialize (void)
  \brief       uninitialize MIPI CSI2 Interface.
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_PowerControl (ARM_POWER_STATE state)
  \brief       Control CSI2 Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_ConfigureHost (uint32_t int_event)
  \brief       Configure CSI2 Host Interface.
  \param[in]   int_event interrupt event to be enabled.
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_ConfigureIPI (void)
  \brief       Configure CSI2 IPI Interface.
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_StartPHY (void)
  \brief       Configure CSI2 PHY Interface.
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_StartIPI(void)
  \brief       Enable CSI2 IPI Interface.
  \return      \ref execution_status

  \fn          int32_t MIPI_CSI2_StopIPI(void)
  \brief       Disable CSI2 IPI Interface.
  \return      \ref execution_status

  \fn          void ARM_MIPI_CSI2_Event_Callback (uint32_t int_event)
  \brief       Signal MIPI CSI2 Events.
  \param[in]   int_event   \ref MIPI CSI2 event types.
  \return      none.
*/

/** \brief MIPI CSI2 signal event */
typedef void (*ARM_MIPI_CSI2_SignalEvent_t) (uint32_t  int_event);    ///< Pointer to \ref ARM_MIPI_CSI2_SignalEvent : Signal MIPI CSI2 Event.

/** \brief MIPI CSI2 driver capabilities */
typedef struct {
    uint32_t reentrant_operation         :1;    ///< Support for reentrant calls
    uint32_t ipi_interface               :1;    ///< Support Image Pixel Interface
    uint32_t idi_interface               :1;    ///< Support Image Data Interface
    uint32_t reserved                    :29;   ///< Reserved (must be zero)
}ARM_MIPI_CSI2_CAPABILITIES;

/** \brief Access structure of the MIPI CSI2 Driver */
typedef struct {
    ARM_DRIVER_VERSION                  (*GetVersion)      (void);                                                     ///< Pointer to \ref MIPI_CSI2_GetVersion : Get driver version.
    ARM_MIPI_CSI2_CAPABILITIES          (*GetCapabilities) (void);                                                     ///< Pointer to \ref MIPI_CSI2_GetCapabilities : Get MIPI CSI2 driver capabilities.
    int32_t                             (*Initialize)      (ARM_MIPI_CSI2_SignalEvent_t cb_event);                     ///< Pointer to \ref MIPI_CSI2_Initialize : Initialize MIPI CSI2 Interface.
    int32_t                             (*Uninitialize)    (void);                                                     ///< Pointer to \ref MIPI_CSI2_Uninitialize : Uninitialize MIPI CSI2 Interface.
    int32_t                             (*PowerControl)    (ARM_POWER_STATE state);                                    ///< Pointer to \ref MIPI_CSI2_PowerControl : Control CSI2 Interface Power.
    int32_t                             (*ConfigureHost)   (uint32_t int_event);                                       ///< Pointer to \ref MIPI_CSI2_ConfigureHost : Configure CSI2 Host Interface.
    int32_t                             (*ConfigureIPI)    (void);                                                     ///< Pointer to \ref MIPI_CSI2_StartPHY : Configure CSI2 PHY Interface.
    int32_t                             (*StartIPI)        (void);                                                     ///< Pointer to \ref MIPI_CSI2_StartIPI : Enable CSI2 IPI Interface.Get driver version.
    int32_t                             (*StopIPI)         (void);                                                     ///< Pointer to \ref MIPI_CSI2_StartIPI : Disable CSI2 IPI Interface.
}ARM_DRIVER_MIPI_CSI2;

#endif /* DRIVER_MIPI_CSI2_H_ */
