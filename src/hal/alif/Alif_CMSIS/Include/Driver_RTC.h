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
 * @file     Driver_RTC.h
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     18-Feb-2021
 * @brief    RTC(Real Time Clock) driver definitions.
 ******************************************************************************/

#ifndef DRIVER_RTC_H_
#define DRIVER_RTC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes --------------------------------------------------------------------------- */
#include "Driver_Common.h"

#define ARM_RTC_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/*----- RTC Control Codes -----*/
#define ARM_RTC_SET_PRESCALER             (0x01UL)    ///< RTC Set Prescaler; arg = Prescaler Value
#define ARM_RTC_SET_ALARM                 (0x02UL)    ///< RTC Set Alarm;     arg = Alarm Value

/****** RTC Event *****/
#define ARM_RTC_EVENT_ALARM_TRIGGER       (1UL << 0)  ///< Alarm Triggered;


// Function documentation
/**
  \fn          ARM_DRIVER_VERSION GetVersion (void)
  \brief       Get RTC driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_RTC_CAPABILITIES GetCapabilities (void)
  \brief       Get RTC driver capabilities
  \return      \ref RTC_CAPABILITIES

  \fn          int32_t Initialize (ARM_RTC_SignalEvent_t cb_event)
  \brief       Initialize RTC Interface.
  \param[in]   cb_event  : Pointer to RTC Event \ref ARM_RTC_SignalEvent
  \return      \ref execution_status

  \fn          int32_t Uninitialize (void)
  \brief       De-initialize RTC Interface.
  \return      \ref execution_status

  \fn          int32_t PowerControl (ARM_POWER_STATE state)
  \brief       Control RTC Interface Power.
  \param[in]   state : Power state
  \return      \ref execution_status

  \fn          int32_t Control (uint32_t control, uint32_t arg)
  \brief       Control RTC Interface.
  \param[in]   control :  Operation
  \param[in]   arg     :  Argument of operation (optional)
  \return      common \ref execution_status

  \fn          int32_t ReadCounter (uint32_t *val)
  \brief       Read the current counter value.
  \param[out]  val : Pointer to the address where current rtc counter value
                     needs to be copied to
  \return      \ref execution_status
*/

typedef void (*ARM_RTC_SignalEvent_t) (uint32_t event);   ///< Pointer to \ref RTC_SignalEvent : Signal RTC Event.

/**
\brief RTC Device Driver Capabilities.
*/
typedef struct _ARM_RTC_CAPABILITIES {
  uint32_t alarm              : 1;      ///< supports RTC Alarm Event Callback
  uint32_t reserved           : 31;     ///< Reserved (must be zero)
} ARM_RTC_CAPABILITIES;


/**
\brief Access structure of the RTC Driver.
*/
typedef struct _ARM_DRIVER_RTC {
  ARM_DRIVER_VERSION     (*GetVersion)      (void);                            ///< Pointer to \ref RTC_GetVersion : Get driver version.
  ARM_RTC_CAPABILITIES   (*GetCapabilities) (void);                            ///< Pointer to \ref RTC_GetCapabilities : Get driver capabilities.
  int32_t                (*Initialize)      (ARM_RTC_SignalEvent_t cb_event);  ///< Pointer to \ref RTC_Initialize : Initialize RTC Interface.
  int32_t                (*Uninitialize)    (void);                            ///< Pointer to \ref RTC_Uninitialize : De-initialize RTC Interface.
  int32_t                (*PowerControl)    (ARM_POWER_STATE state);           ///< Pointer to \ref RTC_PowerControl : Control RTC Interface Power.
  int32_t                (*Control)         (uint32_t control, uint32_t arg);  ///< Pointer to \ref RTC_Control : Control RTC Interface.
  int32_t                (*ReadCounter)     (uint32_t *val);                   ///< Pointer to \ref RTC_ReadCounter : Read the RTC current counter.
  int32_t                (*LoadCounter)     (uint32_t loadval);                ///< Pointer to \ref RTC_LoadCounter : Load  new RTC counter Value.
} const ARM_DRIVER_RTC;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_RTC_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
