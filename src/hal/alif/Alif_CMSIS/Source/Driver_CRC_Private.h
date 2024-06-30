/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DRIVER_CRC_PRIVATE_H_
#define DRIVER_CRC_PRIVATE_H_

/* System includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#if (RTE_CRC0_DMA_ENABLE || RTE_CRC1_DMA_ENABLE)
#define CRC_DMA_ENABLE  1
#else
#define CRC_DMA_ENABLE  0
#endif

#if CRC_DMA_ENABLE
#include <DMA_Common.h>
#endif

/* Project includes */
#include "crc.h"
#include "Driver_CRC.h"

#define CRC_8_BIT_SIZE           0          /* To select the 8 bit algorithm size  */
#define CRC_16_BIT_SIZE          1          /* To select the 16 bit algorithm size */
#define CRC_32_BIT_SIZE          2          /* To select the 32 bit algorithm size */

#define CRC_DMA_MIN_TRANSFER_LEN 900
/**
 @brief   : CRC Driver states
 */
typedef volatile struct _CRC_DRIVER_STATE {
    uint32_t initialized : 1;                    /* Driver Initialized    */
    uint32_t powered     : 1;                    /* Driver powered        */
    uint32_t reserved    : 30;                   /* Reserved              */
} CRC_DRIVER_STATE;

/**
 @brief   : Access structure for the saving the CRC Setting and status
 */
typedef struct _CRC_RESOURCES
{
    ARM_CRC_SignalEvent_t   cb_event;           /* CRC application callback event */
    CRC_Type                *regs;              /* CRC register address           */
    CRC_DRIVER_STATE        state;              /* CRC Driver state               */
    uint8_t                 busy;               /* CRC compute busy flag          */
    crc_transfer_t          transfer;           /* CRC transfer params            */
#if CRC_DMA_ENABLE
    const bool              dma_enable;         /* DMA enable                     */
    const uint32_t          dma_irq_priority;   /* DMA IRQ priority number        */
    ARM_DMA_SignalEvent_t   dma_cb;             /* CRC DMA Callback               */
    DMA_PERIPHERAL_CONFIG   dma_cfg;            /* DMA configuration              */
    volatile uint32_t       dma_event;          /* Result of DMA operation        */
#endif
}CRC_RESOURCES;

#endif /* DRIVER_CRC_PRIVATE_H_ */
