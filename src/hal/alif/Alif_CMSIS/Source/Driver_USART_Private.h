/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef DRIVER_USART_PRIVATE_H
#define DRIVER_USART_PRIVATE_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include "RTE_Device.h"
#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_USART.h"
#include "uart.h"

/*  As per PINMUX,
 *    UART0-UART3 instances has RS232 with RTS/CTS functionality and
 *    UART4-UART7 instances has RS232 without RTS/CTS and RS485 functionality.
 */

/* Check if RS485 functionality is enable? */
#if ( (RTE_UART4_RS485_ENABLE) || (RTE_UART5_RS485_ENABLE) || (RTE_UART6_RS485_ENABLE) || (RTE_UART7_RS485_ENABLE) )
#ifndef RS485_SUPPORT
#define RS485_SUPPORT  (1) /* Enable(1): RS485 Support */
#endif
#else
#ifndef RS485_SUPPORT
#define RS485_SUPPORT  (0) /* Disable(0): RS485 Support */
#endif
#endif

/* Check if DMA Support is enable? */
#if (RTE_UART0_DMA_ENABLE || RTE_UART1_DMA_ENABLE || RTE_UART2_DMA_ENABLE  || \
     RTE_UART3_DMA_ENABLE || RTE_UART4_DMA_ENABLE || RTE_UART5_DMA_ENABLE  || \
     RTE_UART6_DMA_ENABLE || RTE_UART7_DMA_ENABLE || RTE_LPUART_DMA_ENABLE)
#define UART_DMA_ENABLE  1
#else
#define UART_DMA_ENABLE  0
#endif

#if UART_DMA_ENABLE
#include <DMA_Common.h>
#endif

/* Check if Blocking(Polling) mode support is enable? */
#if (RTE_UART0_BLOCKING_MODE_ENABLE || RTE_UART1_BLOCKING_MODE_ENABLE || RTE_UART2_BLOCKING_MODE_ENABLE || \
     RTE_UART3_BLOCKING_MODE_ENABLE || RTE_UART4_BLOCKING_MODE_ENABLE || RTE_UART5_BLOCKING_MODE_ENABLE || \
     RTE_UART6_BLOCKING_MODE_ENABLE || RTE_UART7_BLOCKING_MODE_ENABLE || RTE_LPUART_BLOCKING_MODE_ENABLE)
#define UART_BLOCKING_MODE_ENABLE  1
#else
#define UART_BLOCKING_MODE_ENABLE  0
#endif

/**
 * enum UART_INSTANCE
 * UART instances
 */
typedef enum
{
    UART_INSTANCE_0,                /**< UART instance - 0   */
    UART_INSTANCE_1,                /**< UART instance - 1   */
    UART_INSTANCE_2,                /**< UART instance - 2   */
    UART_INSTANCE_3,                /**< UART instance - 3   */
    UART_INSTANCE_4,                /**< UART instance - 4   */
    UART_INSTANCE_5,                /**< UART instance - 5   */
    UART_INSTANCE_6,                /**< UART instance - 6   */
    UART_INSTANCE_7,                /**< UART instance - 7   */
    UART_INSTANCE_LP                /**< UART instance - LP  */
} UART_INSTANCE;

/* UART Status */
typedef enum
{
    UART_STATUS_FREE  = 0,      /* UART Status Free. */
    UART_STATUS_BUSY  = 1       /* UART Status Busy. */
}UART_STATUS;

/* UART Clock Source */
typedef enum _UART_CLK_SOURCE
{
    UART_CLK_SOURCE_HFOSC_CLK = 0,   /* UART Clock Source HFOSC_CLK */
    UART_CLK_SOURCE_SYST_PCLK = 1,   /* UART Clock Source SYST_PCLK */
} UART_CLK_SOURCE;

/** \brief UART Driver states. */
typedef volatile struct _UART_DRIVER_STATE
{
    uint32_t initialized   : 1; /* Driver initialized   */
    uint32_t powered       : 1; /* Driver powered       */
    uint32_t tx_enabled    : 1; /* Driver tx enabled    */
    uint32_t rx_enabled    : 1; /* Driver rx enabled    */
    uint32_t rs485_enabled : 1; /* Driver rs485 enabled */
    uint32_t reserved      : 27;/* Reserved             */
} UART_DRIVER_STATE;

#if RS485_SUPPORT
/* UART Configuration definitions for RS485. */
/* @brief Structure to save contexts for a UART RS485 configurations */
typedef struct _UART_RS485_CONFIG
{
    uint8_t                    rs485_control;                          /* uart RS485 Control, 0-Disable, 1-Enable                                               */
    UART_RS485_TRANSFER_MODE   rs485_transfer_mode;                    /* uart RS485 transfer mode.                                                             */
    uint8_t                    rs485_de_assertion_time_8bit;           /* uart RS485 Driver Enable DE Assertion Time (8-bit).                                   */
    uint8_t                    rs485_de_deassertion_time_8bit;         /* uart RS485 Driver Enable DE De-Assertion Time (8-bit).                                */
    uint16_t                   rs485_de_to_re_turn_around_time_16bit;  /* uart RS485 Turn Around Time TAT for Driver Enable DE to Receive Enable RE (16-bit).   */
    uint16_t                   rs485_re_to_de_turn_around_time_16bit;  /* uart RS485 Turn Around Time TAT for Receive Enable RE to Driver Enable DE (16-bit).   */
} UART_RS485_CONFIG;
#endif /* END RS485_SUPPORT */

#if UART_DMA_ENABLE
typedef struct _UART_DMA_HW_CONFIG
{
    DMA_PERIPHERAL_CONFIG dma_tx;  /* DMA Tx interface */
    DMA_PERIPHERAL_CONFIG dma_rx;  /* DMA Rx interface */
} UART_DMA_HW_CONFIG;
#endif


/* UART Resources definitions */
/* @brief Structure to save contexts for a UART */
typedef struct _UART_RESOURCES
{
    UART_Type                 *regs;               /* Pointer to UART regs            */

    ARM_USART_SignalEvent_t    cb_event;           /* Pointer to call back function   */

#if UART_DMA_ENABLE
    ARM_DMA_SignalEvent_t      dmatx_cb;           /* Pointer to DMA Tx Callback      */
    ARM_DMA_SignalEvent_t      dmarx_cb;           /* Pointer to DMA Rx Callback      */
#endif

    UART_TRANSFER              transfer;           /* UART Transfer information       */
    ARM_USART_STATUS           status;             /* UART driver status              */
    UART_DRIVER_STATE          state;              /* UART driver state               */
    uint32_t                   baudrate;           /* UART Baudrate                   */
    uint32_t                   clk;                /* UART system clock               */
    UART_CLK_SOURCE            clk_source;         /* UART clock source               */
    UART_TX_TRIGGER            tx_fifo_trg_lvl;    /* UART Tx FIFO Trigger Level      */
    UART_RX_TRIGGER            rx_fifo_trg_lvl;    /* UART Rx FIFO Trigger Level      */
    IRQn_Type                  irq_num;            /* UART interrupt vector number    */
    uint32_t                   irq_priority;       /* UART interrupt priority         */
    uint8_t                    instance;           /* UART instance                   */

#if UART_DMA_ENABLE
    const bool                 dma_enable;         /* UART dma enable                  */
    const uint32_t             dma_irq_priority;   /* DMA IRQ priority number          */
    UART_DMA_HW_CONFIG        *dma_cfg;            /* DMA Controller configuration     */
#endif

#if UART_BLOCKING_MODE_ENABLE
    const bool                 blocking_mode;      /* UART instance blocking mode transfer enable  */
#endif

#if RS485_SUPPORT
    UART_RS485_CONFIG          rs485_cfg;          /* UART rs485 configuration.       */
#endif

} UART_RESOURCES;


#ifdef  __cplusplus
}
#endif

#endif /* DRIVER_USART_PRIVATE_H */
