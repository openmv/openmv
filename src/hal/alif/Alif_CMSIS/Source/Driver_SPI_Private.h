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
 * @file     Driver_SPI_Private.h
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     20-04-2023
 * @brief    Header file for SPI.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef DRIVER_SPI_PRIVATE_H
#define DRIVER_SPI_PRIVATE_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_SPI.h"
#include "sys_ctrl_spi.h"
#include "spi.h"

#if (RTE_SPI0_DMA_ENABLE || RTE_SPI1_DMA_ENABLE || RTE_SPI2_DMA_ENABLE || \
     RTE_SPI3_DMA_ENABLE || RTE_LPSPI_DMA_ENABLE)
#define SPI_DMA_ENABLE  1
#else
#define SPI_DMA_ENABLE  0
#endif

#if (RTE_SPI0_BLOCKING_MODE_ENABLE || RTE_SPI1_BLOCKING_MODE_ENABLE || RTE_SPI2_BLOCKING_MODE_ENABLE || \
     RTE_SPI3_BLOCKING_MODE_ENABLE || RTE_LPSPI_BLOCKING_MODE_ENABLE)
#define SPI_BLOCKING_MODE_ENABLE  1
#else
#define SPI_BLOCKING_MODE_ENABLE  0
#endif

#if (RTE_SPI0_USE_MASTER_SS_SW || RTE_SPI1_USE_MASTER_SS_SW || RTE_SPI2_USE_MASTER_SS_SW || \
     RTE_SPI3_USE_MASTER_SS_SW || RTE_LPSPI_USE_MASTER_SS_SW)
#define SPI_USE_MASTER_SS_SW  1
#else
#define SPI_USE_MASTER_SS_SW  0
#endif

#if (RTE_SPI0_MICROWIRE_FRF_ENABLE || RTE_SPI1_MICROWIRE_FRF_ENABLE || RTE_SPI2_MICROWIRE_FRF_ENABLE || \
     RTE_SPI3_MICROWIRE_FRF_ENABLE || RTE_LPSPI_MICROWIRE_FRF_ENABLE)
#define SPI_MICROWIRE_FRF_ENABLE  1
#else
#define SPI_MICROWIRE_FRF_ENABLE  0
#endif


#if SPI_DMA_ENABLE
#include <DMA_Common.h>
#endif

#if SPI_USE_MASTER_SS_SW
#include "Driver_GPIO.h"
#endif

/** \brief SPI Driver states. */
typedef volatile struct _SPI_DRIVER_STATE {
    uint32_t initialized : 1; /* Driver Initialized*/
    uint32_t powered     : 1; /* Driver powered */
    uint32_t reserved    : 30;/* Reserved */
} SPI_DRIVER_STATE;

/** \brief SPI SS control types. */
typedef enum _SPI_SS_CONTROL {
    SPI_SS_SW_CONTROL,
    SPI_SS_HW_CONTROL
} SPI_SS_CONTROL;

#if SPI_DMA_ENABLE
typedef struct _SPI_DMA_HW_CONFIG {
    DMA_PERIPHERAL_CONFIG dma_tx;             /*< DMA tx interface >*/
    DMA_PERIPHERAL_CONFIG dma_rx;             /*< DMA rx interface >*/
} SPI_DMA_HW_CONFIG;
#endif

#if SPI_USE_MASTER_SS_SW
typedef struct _SPI_SS_MASTER_SW_CONFIG {
    ARM_DRIVER_GPIO             *drvGPIO;           /**< Pointer to gpio instance */
    uint8_t                     ss_port;            /**< SPI port number for software controlled SS line  */
    uint8_t                     ss_pin;             /**< SPI pin number for software controlled SS line   */
    bool                        active_polarity;    /**< SPI active state for software controlled SS line */
} SPI_SS_MASTER_SW_CONFIG;
#endif

#if SPI_MICROWIRE_FRF_ENABLE
#define SPI_MW_CONTROL_FRAME_SIZE_MIN             1U   /* Min MW control frame size */
#define SPI_MW_CONTROL_FRAME_SIZE_MAX             16U  /* Max MW control frame size */

/** \brief SPI_MW_TRANSFER_MODE. */
typedef enum _SPI_MW_TRANSFER_MODE {
    SPI_MW_TRANSFER_MODE_SEQUANTIAL,
    SPI_MW_TRANSFER_MODE_NON_SEQUANTIAL
} SPI_MW_TRANSFER_MODE;

typedef struct _SPI_MICROWIRE_CONFIG {
    SPI_MW_TRANSFER_MODE        transfer_mode;      /*< microwire transfer mode */
    bool                        handshake_enable ;  /*< ENABLE/DISABLE MicroWire handshaking interface */
    uint8_t                     cfs;                /*< MicroWire control size format */
} SPI_MICROWIRE_CONFIG;
#endif

/** \brief Resources for a SPI instance. */
typedef struct _SPI_RESOURCES
{
    SPI_Type                    *regs;              /**< Pointer to regs                                  */
    ARM_SPI_SignalEvent_t       cb_event;           /**< Pointer to call back function                    */
    ARM_SPI_STATUS              status;             /**< SPI driver status                                */
    SPI_DRIVER_STATE            state;              /**< SPI driver state                                 */
    spi_transfer_t              transfer;           /**< Transfer structure for the SPI instance          */
    SPI_SS_CONTROL              master_ss_control;  /**< operate Slave select control pin                 */
    uint8_t                     irq_priority;       /**< Interrupt priority                               */
    uint8_t                     slave_select;       /**< chip selection pin from 0-3                      */
    uint8_t                     rx_fifo_threshold;  /**< Rx FIFO threshold                                */
    uint8_t                     tx_fifo_threshold;  /**< Tx FIFO threshold                                */
    SPI_INSTANCE                drv_instance;       /**< Driver instance                                  */
    uint8_t                     rx_sample_delay;    /**< Receive data sample delay                        */
    bool                        sste_enable;        /**< Enable Slave select toggle after each data frame */
#if SPI_DMA_ENABLE
    bool                        dma_enable;         /**< SPI instance DMA enable                          */
    uint8_t                     dma_irq_priority;   /**< SPI instance DMA IRQ priority                    */
    ARM_DMA_SignalEvent_t       dma_cb;             /**< SPI  DMA callback function                       */
    SPI_DMA_HW_CONFIG           *dma_cfg;           /**< DMA Controller configuration                     */
#endif
#if SPI_BLOCKING_MODE_ENABLE
    bool                        blocking_mode;      /**< SPI instance blocking mode transfer enable       */
#endif
    uint16_t                    tx_fifo_start_level;/**< TX FIFO start level                              */
    IRQn_Type                   irq;                /**< Instance IRQ number                              */
#if SPI_USE_MASTER_SS_SW
    SPI_SS_MASTER_SW_CONFIG     sw_config;          /**< SPI soft controlled slave select configuration   */
#endif
#if SPI_MICROWIRE_FRF_ENABLE
    bool                        mw_enable;          /*< Enable MicroWIre Frame Format */
    SPI_MICROWIRE_CONFIG        mw_config;          /**< Microwire frame format configurations            */
#endif
} SPI_RESOURCES;

/**
  \fn          static inline uint32_t getSpiCoreClock(SPI_INSTANCE instance)
  \brief       Get the SPI input clock rate
  \param[in]   instance  spi instance
  \return      SPI input clock rate
*/
static inline uint32_t getSpiCoreClock(SPI_INSTANCE instance)
{
    /* LPSPI will be enabled only for M55 HE */
    if (instance == LPSPI_INSTANCE)
    {
        return SystemCoreClock;
    }
    return GetSystemAHBClock();
}

#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_SPI_PRIVATE_H */
