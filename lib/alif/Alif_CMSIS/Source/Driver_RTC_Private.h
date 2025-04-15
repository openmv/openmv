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
 * @file     Driver_LPRTC_Private.h
 * @author   Tanay Rami, Manoj A Murudi
 * @email    tanay@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     23-March-2023
 * @brief    Device Specific Header file for RTC Driver.
 ******************************************************************************/

#ifndef DRIVER_RTC_PRIVATE_H_
#define DRIVER_RTC_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_RTC.h"
#include "rtc.h"

/** \brief LPRTC Driver states. */
typedef volatile struct _LPRTC_DRIVER_STATE {
    uint32_t initialized : 1; /* Driver Initialized*/
    uint32_t powered     : 1; /* Driver powered */
    uint32_t alarm       : 1; /* Driver set alarm */
    uint32_t reserved    : 29;/* Reserved */
} LPRTC_DRIVER_STATE;

/** \brief Resources for a LPRTC instance. */
typedef struct _LPRTC_RESOURCES {
    LPRTC_Type                *regs;           /* LPRTC register address             */
    ARM_RTC_SignalEvent_t     cb_event;        /* LPRTC Application Event callback   */
    LPRTC_DRIVER_STATE        state;           /* LPRTC Driver state                 */
    IRQn_Type                 irq_num;         /* LPRTC interrupt vector number      */
    uint16_t                  irq_priority;    /* LPRTC interrupt priority           */
}LPRTC_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_RTC_PRIVATE_H_ */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
