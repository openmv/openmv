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
 * @file     Driver_CDC_Private.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     28-Sep-2023
 * @brief    CDC driver Specific Header file.
 ******************************************************************************/

#ifndef DRIVER_CDC_PRIVATE_H_
#define DRIVER_CDC_PRIVATE_H_

#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_CDC200.h"

#include "cdc.h"

#ifdef  __cplusplus
extern "C"
{
#endif

/** \brief CDC Driver states. */
typedef volatile struct _CDC_DRIVER_STATE {
    uint32_t initialized : 1;                    /**< Driver Initialized    */
    uint32_t powered     : 1;                    /**< Driver powered        */
    uint32_t configured  : 1;                    /**< Driver configured     */
    uint32_t reserved    : 29;                   /**< Reserved              */
} CDC_DRIVER_STATE;

/** \brief Resources for a CDC instance */
typedef struct _CDC_RESOURCES {
    CDC_Type                  *regs;                 /**< Pointer to regs                  */
    ARM_CDC200_SignalEvent_t  cb_event;              /**< Pointer to call back function    */
    cdc_backgnd_color_info_t  *bgc;                  /**< Pointer to CDC background color  */
    CDC_PIXEL_FORMAT          pixel_format;          /**< CDC pixel format                 */
    uint8_t                   const_alpha;           /**< Layer constant alpha             */
    CDC_BLEND_FACTOR          blend_factor;          /**< Layer blending factor            */
    uint32_t                  irq_priority;          /**< Interrupt priority               */
    CDC_DRIVER_STATE          state;                 /**< CDC driver status                */
} CDC_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_CDC_PRIVATE_H_ */
