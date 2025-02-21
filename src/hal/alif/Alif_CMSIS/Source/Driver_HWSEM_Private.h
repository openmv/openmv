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
 * @file     Driver_HWSEM_Private.h
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     16-June-2022
 * @brief    Device Specific Header file for hardware semaphore driver.
 ******************************************************************************/

#ifndef DRIVER_HWSEM_PRIVATE_H_
#define DRIVER_HWSEM_PRIVATE_H_

/* System Includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include "Driver_Common.h"
#include "Driver_HWSEM.h"

#include "hwsem.h"

#include CMSIS_device_header

#ifdef  __cplusplus
extern "C"
{
#endif

/* Hardware Sem IDs */
typedef enum _HWSEM_ID
{
    HWSEMID0,
    HWSEMID1,
    HWSEMID2,
    HWSEMID3,
    HWSEMID4,
    HWSEMID5,
    HWSEMID6,
    HWSEMID7,
    HWSEMID8,
    HWSEMID9,
    HWSEMID10,
    HWSEMID11,
    HWSEMID12,
    HWSEMID13,
    HWSEMID14,
    HWSEMID15
}HWSEM_ID;

/** \brief Hw Semaphore driver state */
typedef volatile struct _HWSEM_DRIVER_STATE {
    uint32_t initialized : 1;                    /**< Driver Initialized */
    uint32_t reserved    : 31;                   /**< Reserved           */
} HWSEM_DRIVER_STATE;

/** \brief Representation of Hw Semaphore. */
typedef struct {
    HWSEM_Type *regs;                   /**< IOMEM base address of the Hw Semaphore module */
    ARM_HWSEM_SignalEvent_t cb_event;   /**< Registered callback function */
    IRQn_Type irq;                      /**< IRQ number of the HSEM instance */
    HWSEM_DRIVER_STATE state;           /**< HSEM driver state */
    uint8_t irq_priority;               /**< IRQ priority can be modified by user */
    uint8_t sem_id;                     /**< HWSEM ID */
} HWSEM_RESOURCES;

#ifdef  __cplusplus
}
#endif
#endif /* DRIVER_HWSEM_PRIVATE_H_ */
