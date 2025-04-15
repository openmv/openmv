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
 * @file     Driver_Touch_Screen.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     5-January-23
 * @brief    Touch screen driver header.
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef DRIVER_TOUCH_SCREEN_H_
#define DRIVER_TOUCH_SCREEN_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "Driver_Common.h"

#define ARM_TOUCH_SCREEN_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) /* API version */

/**
\brief Touch Screen Coordinates
*/
typedef struct _ARM_TOUCH_COORDINATES{
	int16_t x;          ///< Data of the X coordinate
	int16_t y;          ///< Data of the X coordinate
} ARM_TOUCH_COORDINATES;

/**
\brief Touch Screen state
*/
typedef struct _ARM_TOUCH_STATE{
	int32_t numtouches;                                          ///< Number touch happened.
	ARM_TOUCH_COORDINATES coordinates[RTE_ACTIVE_TOUCH_POINTS];  ///< Variable to store Touch coordinates.
} ARM_TOUCH_STATE;

// Function documentation
/*
  \fn          ARM_DRIVER_VERSION ARM_TOUCH_SCREEN_GetVersion (void)
  \brief       Get touch screen driver version.
  \return      \ref ARM_DRIVER_VERSION

  \fn          ARM_TOUCH_SCREEN_CAPABILITIES ARM_TOUCH_SCREEN_GetCapabilities (void)
  \brief       Get touch screen driver capabilities.
  \return      \ref ARM_TOUCH_SCREEN_CAPABILITIES

  \fn          int32_t  ARM_TOUCH_SCREEN_Initialize (void)
  \brief       Initialize touch screen Interface.
  \param[in]   none.
  \return      \ref execution_status

  \fn          int32_t ARM_TOUCH_SCREEN_Uninitialize (void)
  \brief       uninitialize touch screen Interface.
  \return      \ref execution_status

  \fn          int32_t ARM_TOUCH_SCREEN_PowerControl (ARM_POWER_STATE state)
  \brief       Control touch screen Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status

  \fn          int32_t ARM_TOUCH_SCREEN_GetState (ARM_TOUCH_STATE *state)
  \brief       Get touch screen touch state..
  \param[in]   state pointer to ARM_TOUCH_STATE.
  \return      \ref execution_status
*/

/**
  \brief Touch Screen capabilities.
  */
typedef struct {
	uint32_t reentrant_operation         :1;    ///< Support for reentrant calls
	uint32_t multi_touch_points          :1;    ///< Support Multiple touch points
	uint32_t reserved                    :30;   ///< Reserved (must be zero)
}ARM_TOUCH_SCREEN_CAPABILITIES;

/**
  \brief Touch screen Device Operations.
  */
typedef struct _ARM_DRIVER_TOUCH_SCREEN{
	ARM_DRIVER_VERSION             (*GetVersion)      (void);                    ///< Pointer to \ref ARM_TOUCH_SCREEN_GetVersion : Get driver version.
	ARM_TOUCH_SCREEN_CAPABILITIES  (*GetCapabilities) (void);                    ///< Pointer to \ref ARM_TOUCT_SCREEN_GetCapabilities : Get driver capabilities.
	int32_t                        (*Initialize)      (void);                    ///< Pointer to \ref ARM_TOUCH_SCREEN_Initialize : Initialize touch screen Interface.
	int32_t                        (*Uninitialize)    (void);                    ///< Pointer to \ref ARM_TOUCH_SCREEN_Uninitialize : Un-initialize touch screen Interface.
	int32_t                        (*PowerControl)    (ARM_POWER_STATE state);   ///< Pointer to \ref ARM_TOUCH_SCREEN_PowerControl : Control touch screen Interface Power.
	int32_t                        (*GetState)        (ARM_TOUCH_STATE *state);  ///< Pointer to \ref ARM_TOUCH_SCREEN_GetState : Get touch screen touch state.
} const ARM_DRIVER_TOUCH_SCREEN;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_TOUCH_SCREEN_H_ */
