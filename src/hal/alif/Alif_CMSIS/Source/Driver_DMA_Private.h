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
 * @file     Driver_DMA_Private.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     04-Nov-2020
 * @brief    DMA Driver Private header
 * @bug      None
 * @Note     None
 ******************************************************************************/
#ifndef DRIVER_DMA_PRIVATE_H

#define DRIVER_DMA_PRIVATE_H
/* Includes ------------------------------------------------------------------*/
#include "Driver_DMA.h"
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include <dma_ctrl.h>
#include "sys_ctrl_dma.h"

#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

/** \brief DMA Driver states. */
typedef struct _DMA_DRIVER_STATE {
    uint32_t initialized : 1; /* Driver Initialized*/
    uint32_t powered     : 1; /* Device powered */
    uint32_t reserved    : 30;/* Reserved */
} DMA_DRIVER_STATE;

typedef union _DMA_DRV_STATUS {
    uint32_t status;                               /*!< DMA Driver status */
    ARM_DMA_STATUS status_b;                       /*!< DMA Driver status bits */
} DMA_DRV_STATUS;

typedef struct _DMA_RESOURCES {
    DMA_Type                 *regs;                   /*!< DMA register map               */
    ARM_DMA_SignalEvent_t    cb_event[DMA_MAX_EVENTS];   /*!< DMA Application Event Callback */
    dma_config_info_t        cfg;                     /*!< DMA Controller configuration   */
    DMA_SECURE_STATE         ns_iface;                /*!< DMA interface to be used       */
    DMA_DRV_STATUS           drv_status;              /*!< DMA Driver Status              */
    DMA_DRIVER_STATE         state;                   /*!< DMA Driver State               */
    uint8_t                  consumer_cnt;            /*!< DMA Consumer Counter           */
    const IRQn_Type          irq_start;               /*!< DMA irq0 start index           */
    const uint32_t           abort_irq_priority;      /*!< DMA Abort IRQ priority         */
    const DMA_INSTANCE       instance;                /*!< DMA controller instance        */
} DMA_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_DMA_PRIVATE_H */
