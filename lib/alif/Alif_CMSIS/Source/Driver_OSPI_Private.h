/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     Driver_OSPI_Private.h
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     19-06-2023
 * @brief    Private header file for OSPI.
 ******************************************************************************/

#ifndef DRIVER_OSPI_PRIVATE_H
#define DRIVER_OSPI_PRIVATE_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_OSPI.h"
#include "ospi.h"
#include "clk.h"
#include "sys_ctrl_aes.h"

#if (RTE_OSPI0_DMA_ENABLE || RTE_OSPI1_DMA_ENABLE)
#define OSPI_DMA_ENABLE  1
#else
#define OSPI_DMA_ENABLE  0
#endif

#if OSPI_DMA_ENABLE
#include <DMA_Common.h>
#endif

typedef enum _OSPI_INSTANCE
{
    OSPI_INSTANCE_0,                         /* SPI instance - 0 */
    OSPI_INSTANCE_1,                         /* SPI instance - 1 */
} OSPI_INSTANCE;

typedef volatile struct _OSPI_DRIVER_STATE {
    uint32_t initialized: 1;                /* SPI driver initialized status */
    uint32_t powered    : 1;                /* SPI driver powered status */
    uint32_t reserved   : 30;               /* Reserved */
} OSPI_DRIVER_STATE;

#if OSPI_DMA_ENABLE
typedef struct _OSPI_DMA_HW_CONFIG {
    DMA_PERIPHERAL_CONFIG dma_tx;           /* DMA Tx config */
    DMA_PERIPHERAL_CONFIG dma_rx;           /* DMA Rx config */
} OSPI_DMA_HW_CONFIG;
#endif

typedef struct _OSPI_RESOURCES
{
    OSPI_Type                   *regs;                 /* Pointer to OSPI memory map */
    AES_Type                    *aes_regs;             /* Pointer to AES memory map */
    IRQn_Type                   irq;                   /* IRQ number */
    OSPI_INSTANCE               drv_instance;          /* Driver instance */
    ARM_OSPI_SignalEvent_t      cb_event;              /* Pointer to call back function */
    ARM_OSPI_STATUS             status;                /* OSPI driver status */
    OSPI_DRIVER_STATE           state;                 /* OSPI driver state */
    uint32_t                    irq_priority;          /* Interrupt priority */
    uint32_t                    rx_total_cnt;          /* Total count to receive */
    ospi_transfer_t             transfer;              /* Transfer structure for the instance */
    SPI_TMOD                    mode;                  /* Transfer mode */
#if OSPI_DMA_ENABLE
    ARM_DMA_SignalEvent_t       dma_cb;                /* Pointer to DMA callback function */
    OSPI_DMA_HW_CONFIG          *dma_config;           /* OSPI DMA configuration */
    const bool                  dma_enable;            /* Enable DMA for the instance */
    const uint32_t              dma_irq_priority;      /* DMA irq priority */
    uint16_t                    rx_dma_data_level;     /* Rx DMA data level */
    uint16_t                    tx_dma_data_level;     /* Tx DMA data level */
#endif
    uint16_t                    rx_fifo_threshold;     /* Rx fifo threshold */
    uint16_t                    tx_fifo_threshold;     /* Tx fifo threshold */
    uint16_t                    tx_fifo_start_level;   /* Tx fifo level to start communication */
    uint8_t                     ddr_drive_edge;        /* Drive edge for transmit data */
    uint8_t                     rx_sample_delay;       /* Receive data sample delay */
    uint8_t                     rxds_delay;            /* RXDS delay */
    uint8_t                     chip_selection_pin;    /* chip selection pin from 0-3 */
} OSPI_RESOURCES;

static inline uint32_t getOSPICoreClock(void)
{
    return GetSystemAXIClock();
}

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_OSPI_PRIVATE_H */
