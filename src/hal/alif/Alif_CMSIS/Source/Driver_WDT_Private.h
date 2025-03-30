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
 * @file     WDT_Private.h
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     27-April-2021
 * @brief    Private Header file for the CMSIS Wachdog Driver.
 ******************************************************************************/

#ifndef WDT_PRIVATE_H_
#define WDT_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes --------------------------------------------------------------------------- */
/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include <wdt.h>

/* Watchdog state definitions */
#define WATCHDOG_INITIALIZED       (1ul << 0)
#define WATCHDOG_STARTED           (1ul << 1)
#define WATCHDOG_POWERED           (1ul << 2)

/*
 * struct WDT_RESOURCES: structure representing a Watchdog device
 * @regs            : Address map for the watchdog device
 * @flags           : Watchdog driver flags
 * @timeout         : Timeout in msec, must be > 0.
 */
typedef struct _WDT_RESOURCES {
    WDT_CTRL_Type          *regs;             /* Watchdog physical address             */
    uint8_t                flags;             /* Watchdog Driver Flags                 */
    uint32_t               timeout;           /* Watchdog Timeout to reset in msec   */
} WDT_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* WDT_PRIVATE_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
