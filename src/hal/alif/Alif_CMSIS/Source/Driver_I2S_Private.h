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
 * @file     Driver_I2S_Private.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V3.0.0
 * @date     06-Jun-2023
 * @brief    I2S Private header file.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef DRIVER_I2S_PRIVATE_H
#define DRIVER_I2S_PRIVATE_H
/* Includes ------------------------------------------------------------------*/
#include <Driver_SAI.h>
#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include <stdbool.h>
#include "sys_ctrl_i2s.h"
#include "i2s.h"

#if (RTE_I2S0_DMA_ENABLE || RTE_I2S1_DMA_ENABLE || RTE_I2S2_DMA_ENABLE || \
     RTE_I2S3_DMA_ENABLE || RTE_LPI2S_DMA_ENABLE)
#define I2S_DMA_ENABLE  1
#else
#define I2S_DMA_ENABLE  0
#endif

#if I2S_DMA_ENABLE
#include <DMA_Common.h>
#endif

#if ( RTE_I2S0_BLOCKING_MODE_ENABLE || RTE_I2S1_BLOCKING_MODE_ENABLE || \
      RTE_I2S2_BLOCKING_MODE_ENABLE || RTE_I2S3_BLOCKING_MODE_ENABLE || \
      RTE_LPI2S_BLOCKING_MODE_ENABLE )
#define I2S_BLOCKING_MODE_ENABLE  1
#else
#define I2S_BLOCKING_MODE_ENABLE  0
#endif

#ifdef  __cplusplus
extern "C"
{
#endif

/** \brief I2S Driver states. */
typedef struct _I2S_DRIVER_STATE {
    uint32_t initialized : 1; /* Driver Initialized*/
    uint32_t powered     : 1; /* Device powered */
    uint32_t reserved    : 30;/* Reserved */
} I2S_DRIVER_STATE;

typedef enum _I2S_FLAG {
    I2S_FLAG_DRV_MONO_MODE    = (1U << 0), /*!< I2S Driver in Mono Mode */
} I2S_FLAG;

typedef union _I2S_DRV_STATUS {
    uint32_t status;
    volatile ARM_SAI_STATUS status_b;
} I2S_DRV_STATUS;

typedef struct _I2S_CONFIG_INFO {

    /*!< I2S Word Select Size */
    I2S_WSS wss_len;

    /*!< I2S SCLK Gating */
    I2S_SCLKG sclkg;

    /*!< I2S word length */
    I2S_WLEN wlen;

    /*!< I2S Rx FIFO Trigger Level */
    const uint8_t rx_fifo_trg_lvl;

    /*!< I2S Tx FIFO Trigger Level */
    const uint8_t tx_fifo_trg_lvl;

    /*!< I2S clock source in Hz */
    const uint32_t clk_source;

#if I2S_BLOCKING_MODE_ENABLE
    /*!< I2S instance blocking mode transfer enable */
    const bool blocking_mode;
#endif


#if I2S_DMA_ENABLE
    /*!< I2S dma enable */
    const bool dma_enable;

    /*!< DMA IRQ priority number */
    const uint32_t dma_irq_priority;
#endif

    /*!< I2S irq priority number */
    const uint32_t irq_priority;

} I2S_CONFIG_INFO;

#if I2S_DMA_ENABLE
typedef struct _I2S_DMA_HW_CONFIG {
    /*!< Tx interface */
    DMA_PERIPHERAL_CONFIG dma_tx;

    /*!< Rx interface */
    DMA_PERIPHERAL_CONFIG dma_rx;
} I2S_DMA_HW_CONFIG;
#endif

/** \brief Resources for a I2S instance */
typedef struct _I2S_RESOURCES {

    /*!< I2S Application Event Callback */
    ARM_SAI_SignalEvent_t cb_event;

#if I2S_DMA_ENABLE
    /*!< I2S DMA Callback */
    ARM_DMA_SignalEvent_t dma_cb;

    /*!< DMA Controller configuration */
    I2S_DMA_HW_CONFIG *dma_cfg;
#endif

    /*!< I2S ARM I2S Status */
    I2S_DRV_STATUS drv_status;

    /*!< I2S Driver state */
    I2S_DRIVER_STATE state;

    /*!< I2S Audio Sample Rate */
    uint32_t sample_rate;

    /*!< I2S Controller configuration */
    I2S_CONFIG_INFO *cfg;

    /*!< I2S physical address */
    I2S_Type *regs;

    /*!< I2S Instance number */
    const I2S_INSTANCE instance;

    /*!< I2S irq number */
    const IRQn_Type irq;

    /* I2S transfer buffer information */
    i2s_transfer_t transfer;

    /*!< I2S Driver Flags */
    uint8_t flags;

} I2S_RESOURCES;

#ifdef  __cplusplus
}
#endif

#endif // DRIVER_I2S_PRIVATE_H
