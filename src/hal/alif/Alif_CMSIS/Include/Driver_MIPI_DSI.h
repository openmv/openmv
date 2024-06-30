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
 * @file     Driver_MIPI_DSI.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     24-March-2022
 * @brief    Driver Specific Header file for MIPI DSI Driver.
 ******************************************************************************/
#ifndef DRIVER_MIPI_DSI_H_
#define DRIVER_MIPI_DSI_H_

#include "Driver_Common.h"

#define ARM_MIPI_DSI_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/**
\brief MIPI DSI event types
*/
#define DSI_PHY_ERROR_EVENT          (1U << 0)    ///< PHY error event
#define DSI_ACK_ERROR_EVENT          (1U << 1)    ///< Acknowledge error report event
#define DSI_PKT_ERROR_EVENT          (1U << 2)    ///< Packet error event
#define DSI_DPI_ERROR_EVENT          (1U << 3)    ///< DPI error event

// Function documentation
/**
  \fn          ARM_DRIVER_VERSION ARM_MIPI_DSI_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION.

  \fn          ARM_MIPI_DSI_CAPABILITIES MIPI_DSI_GetCapabilities (void)
  \brief       Get MIPI DSI driver capabilities.
  \return      \ref ARM_MIPI_DPHY_CAPABILITIES.

  \fn          int32_t  ARM_MIPI_DSI_Initialize (uint32_t frequency)
  \brief       Initialize MIPI DSI Interface.
  \param[in]   cb_event Pointer to ARM_MIPI_DSI_SignalEvent_t.
  \param[in]   frequency to configure DPHY PLL.
  \return      \ref execution_status.

  \fn          int32_t ARM_MIPI_DSI_Uninitialize (void)
  \brief       uninitialize MIPI DSI Interface.
  \return      \ref execution_status.

  \fn          int32_t ARM_MIPI_DSI_PowerControl (ARM_POWER_STATE state)
  \brief       Control MIPI DSI Interface Power.
  \param[in]   state  Power state.
  \return      \ref execution_status.

  \fn          int32_t ARM_MIPI_DSI_Control (ARM_MIPI_DSI_CONTROL control)
  \brief       Control DSI Interface.
  \param[in]   control DSI host and DPI Configuration.
  \param[in]   arg Argument of operation (optional)
  \return      \ref execution_status

  \fn          int32_t  ARM_MIPI_DSI_StartCommandMode (void)
  \brief       Configure DSI to start Command mode.
  \return      \ref execution_status.

  \fn          int32_t  ARM_MIPI_DSI_StartVideoMode (void)
  \brief       Configure DSI to start Video mode.
  \return      \ref execution_status.

  \fn          int32_t  ARM_MIPI_DSI_Stop (void)
  \brief       Shutdown DSI.
  \return      \ref execution_status

  \fn          void ARM_MIPI_DSI_SignalEvent (uint32_t int_event)
  \brief       Signal MIPI DSI Events.
  \param[in]   int_event  \ref MIPI DSI event types.
  \return      none.
*/

/**
\brief DSI Configuration control
*/
typedef enum _ARM_MIPI_DSI_CONTROL {
    DSI_CONFIGURE_HOST,
    DSI_CONFIGURE_DPI,
} ARM_MIPI_DSI_CONTROL;

/**
\brief MIPI DSI signal event.
*/
typedef void (*ARM_MIPI_DSI_SignalEvent_t) (uint32_t  int_event); ///< Pointer to \ref ARM_MIPI_DSI_SignalEvent : Signal MIPI DSI Event.

/**
\brief MIPI DSI driver capabilities.
*/
typedef struct {
    uint32_t reentrant_operation         :1;    ///< Support for reentrant calls
    uint32_t dpi_interface               :1;    ///< Support video mode Interface
    uint32_t dbi_interface               :1;    ///< Support command mode Interface
    uint32_t reserved                    :29;   ///< Reserved (must be zero)
}ARM_MIPI_DSI_CAPABILITIES;

/**
\brief Access structure of the MIPI DSI Driver.
*/
typedef struct {
    ARM_DRIVER_VERSION                  (*GetVersion)      (void);                                        ///< Pointer to \ref ARM_MIPI_DSI_GetVersion : Get driver version.
    ARM_MIPI_DSI_CAPABILITIES           (*GetCapabilities) (void);                                        ///< Pointer to \ref ARM_MIPI_DSI_GetCapabilities : Get MIPI DSI driver capabilities.
    int32_t                             (*Initialize)      (ARM_MIPI_DSI_SignalEvent_t cb_event);         ///< Pointer to \ref ARM_MIPI_DSI_Initialize : Initialize MIPI DSI Interface.
    int32_t                             (*Uninitialize)    (void);                                        ///< Pointer to \ref ARM_MIPI_DSI_Uninitialize : Uninitialize MIPI DSI Interface.
    int32_t                             (*PowerControl)    (ARM_POWER_STATE state);                       ///< Pointer to \ref ARM_MIPI_DSI_PowerControl : Control MIPI DSI Interface Power.
    int32_t                             (*Control)         (ARM_MIPI_DSI_CONTROL control, uint32_t arg);  ///< Pointer to \ref ARM_MIPI_DSI_Control: Control DSI Interface.
    int32_t                             (*StartCommandMode)(void);                                        ///< Pointer to \ref ARM_MIPI_DSI_StartCommandMode : Configure DSI to start Command mode.
    int32_t                             (*StartVideoMode)  (void);                                        ///< Pointer to \ref ARM_MIPI_DSI_StartVideoMode : Configure DSI to start Video mode.
    int32_t                             (*Stop)            (void);                                        ///< Pointer to \ref ARM_MIPI_DSI_Stop: Shutdown DSI.
}ARM_DRIVER_MIPI_DSI;

#endif /* DRIVER_MIPI_DSI_H_ */
