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
 * @file     Driver_MRAM_Private.h
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     01-June-2023
 * @brief    Device Specific Header file for MRAM (On-chip NVM (Non-Volatile Memory))
 *            Driver.
 ******************************************************************************/

#ifndef DRIVER_MRAM_PRIVATE_H
#define DRIVER_MRAM_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* Project Includes */
#include "Driver_MRAM.h"

/**
\brief MRAM(On-Chip NVM) Total Size
*/
#define MRAM_USER_SIZE            RTE_MRAM_SIZE

/**
\brief MRAM Driver states.
*/
typedef volatile struct _MRAM_DRIVER_STATE
{
  uint32_t initialized    : 1; /* Driver initialized    */
  uint32_t powered        : 1; /* Driver powered        */
  uint32_t reserved       : 30;/* Reserved              */
} MRAM_DRIVER_STATE;

/**
\brief MRAM(On-Chip NVM) Device Resources
*/
typedef struct _MRAM_RESOURCES
{
  MRAM_DRIVER_STATE  state;    /* MRAM driver state  */
} MRAM_RESOURCES;

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MRAM_PRIVATE_H */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
