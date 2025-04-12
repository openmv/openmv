/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DRIVER_CMP_PRIVATE_H_
#define DRIVER_CMP_PRIVATE_H_

#ifdef  __cplusplus
extern "C"
{
#endif

/* System includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#define CMP_CTRL_BASE               CMP0_BASE

#include "sys_ctrl_cmp.h"

/**
 @brief   : CMP Driver states
 */
typedef volatile struct _CMP_DRIVER_STATE {
    uint32_t initialized : 1;                    /* Driver Initialized    */
    uint32_t powered     : 1;                    /* Driver powered        */
    uint32_t reserved    : 30;                   /* Reserved              */
} CMP_DRIVER_STATE;

/**
 * struct CMP_RESOURCES: structure representing a Analog comparator device
 * @regs           : Register address of the Comparator
 * @drv_instance   : Driver instance
 * @state          : Comparator driver state
 * @irq_num        : Comparator interrupt number
 * @config         : Comparator configuration information
 * @irq_priority   : Comparator interrupt Priority
 */
typedef struct _CMP_RESOURCES{
    ARM_Comparator_SignalEvent_t  cb_event;        /* Comparator application event callback */
    CMP_Type                     *regs;            /* Comparator register base address      */
    CMP_INSTANCE                  drv_instance;    /* Driver instance                       */
    CMP_DRIVER_STATE              state;           /* Comparator Driver state               */
    IRQn_Type                     irq_num;         /* Comparator interrupt number           */
    uint32_t                      config;          /* Comparator configuration information  */
    uint32_t                      irq_priority;    /* Comparator interrupt Priority         */
}CMP_RESOURCES;

#endif /* DRIVER_CMP_PRIVATE_H_ */
