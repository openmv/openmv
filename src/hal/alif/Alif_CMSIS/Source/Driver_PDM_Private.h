/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DRIVER_PDM_PRIVATE_H_
#define DRIVER_PDM_PRIVATE_H_

/* System includes */
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

/* include for PDM Driver */
#include "Driver_PDM.h"
#include "pdm.h"

#if(RTE_PDM_DMA_ENABLE || RTE_LPPDM_DMA_ENABLE)
#define PDM_DMA_ENABLE  1
#else
#define PDM_DMA_ENABLE  0
#endif

/* Verify if support for Blocking (Polling) mode is enabled */
#if(RTE_PDM_BLOCKING_MODE_ENABLE || RTE_LPPDM_BLOCKING_MODE_ENABLE)
#define PDM_BLOCKING_MODE_ENABLE   1
#else
#define PDM_BLOCKING_MODE_ENABLE   0
#endif

#if PDM_DMA_ENABLE
#include <DMA_Common.h>
#endif

/**
 * enum PDM_INSTANCE.
 * PDM instances.
 */
typedef enum _PDM_INSTANCE
{
    PDM_INSTANCE_PDM0,    /* PDM instance   */
    PDM_INSTANCE_LPPDM    /* LPPDM instance */
}PDM_INSTANCE;

/** \brief PDM Driver states. */
typedef volatile struct _PDM_DRIVER_STATE {
    uint32_t initialized : 1;              /* Driver Initialized*/
    uint32_t powered     : 1;              /* Driver powered */
    uint32_t reserved    : 30;             /* Reserved */
} PDM_DRIVER_STATE;

#if PDM_DMA_ENABLE
typedef struct _PDM_DMA_HW_CONFIG{
    DMA_PERIPHERAL_CONFIG dma_rx;          /* DMA rx interface */
}PDM_DMA_HW_CONFIG;
#endif

/**
 * Access structure for the saving the PDM Setting and status
 */
typedef struct _PDM_RESOURCES
{
    ARM_PDM_SignalEvent_t             cb_event;             /* PDM application event callback     */
    PDM_Type                         *regs;                 /* PDM register address               */
    pdm_transfer_t                    transfer;             /* To store PDM Capture Configuration */
    PDM_DRIVER_STATE                  state;                /* PDM Driver state                   */
    PDM_INSTANCE                      instance;             /* PDM Driver instance                */
    uint8_t                           fifo_watermark;       /* PDM fifo watermark value           */
    ARM_PDM_STATUS                    status;               /* PDM Driver status                  */
    #if PDM_DMA_ENABLE
    ARM_DMA_SignalEvent_t             dma_cb;               /* PDM DMA Callback                   */
    PDM_DMA_HW_CONFIG                *dma_cfg;              /* DMA controller configuration       */
    bool                              dma_enable;           /* PDM instance DMA enable            */
    uint8_t                           dma_irq_priority;     /* PDM instance DMA irq priority      */
#endif
#if PDM_BLOCKING_MODE_ENABLE
    bool                              blocking_mode;        /* PDM blocking mode transfer enable  */
#endif
    IRQn_Type                         error_irq;            /* PDM error IRQ number               */
    IRQn_Type                         warning_irq;          /* PDM warning IRQ number             */
    IRQn_Type                         audio_detect_irq;     /* PDM audio detect IRQ number        */
    uint32_t                          error_irq_priority;   /* PDM error IRQ priority             */
    uint32_t                          warning_irq_priority; /* PDM warning IRQ priority           */
    uint32_t                          audio_irq_priority;   /* PDM audio IRQ priority             */
}PDM_RESOURCES;

#endif /* DRIVER_PDM_PRIVATE_H_ */
