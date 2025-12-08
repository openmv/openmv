/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Project Includes */
#include "Driver_USART.h"
#include "Driver_USART_Private.h"
#include "uart.h"
#include "sys_ctrl_uart.h"
#include "system_utils.h"

#if !(RTE_UART0 || RTE_UART1 || RTE_UART2 || RTE_UART3 || RTE_UART4 || RTE_UART5 || RTE_UART6 || RTE_UART7)
#error "UART is not enabled in the RTE_Device.h"
#endif

#if (defined(RTE_Drivers_USART0) && !RTE_UART0)
#error "UART0 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART1) && !RTE_UART1)
#error "UART1 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART2) && !RTE_UART2)
#error "UART2 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART3) && !RTE_UART3)
#error "UART3 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART4) && !RTE_UART4)
#error "UART4 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART5) && !RTE_UART5)
#error "UART5 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART6) && !RTE_UART6)
#error "UART6 not configured in RTE_Device.h!"
#endif

#if (defined(RTE_Drivers_USART7) && !RTE_UART7)
#error "UART7 not configured in RTE_Device.h!"
#endif


#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* driver version */

/* enable transmit/receive interrupt */
#define UART_ENABLE_TRANSMITTER_INT                 (1U)    /* enable transmitter interrupt  */
#define UART_ENABLE_RECEIVER_INT                    (2U)    /* enable receiver interrupt     */

/* disable transmit/receive interrupt */
#define UART_DISABLE_TRANSMITTER_INT                (3U)    /* disable transmitter interrupt */
#define UART_DISABLE_RECEIVER_INT                   (4U)    /* disable receiver interrupt    */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    1, /* RTS Flow Control available */
    1, /* CTS Flow Control available */
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    1, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0, /* Signal RI change event: \ref ARM_USART_EVENT_RI */
    0  /* Reserved (must be zero) */
};

/*  Driver Capabilities Without RTS-CTS.
 *  As per PINMUX,
 *    UART0-UART3 instances has RS232 with RTS/CTS functionality and
 *    UART4-UART7 instances has RS232 without RTS/CTS and RS485 functionality.
 */
static const ARM_USART_CAPABILITIES DriverCapabilities_WO_RTS_CTS = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    1, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    1, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0, /* Signal RI change event: \ref ARM_USART_EVENT_RI */
    0  /* Reserved (must be zero) */
};


/**
 * @fn      ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
 * @brief   get uart version
 * @note    none
 * @param   none
 * @retval  driver version
 */
static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
  return DriverVersion;
}

/**
 * @fn      ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
 * @brief   get uart capabilites
 * @note    none
 * @param   none
 * @retval  driver capabilites
 */
static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
  return DriverCapabilities;
}

/**
 * @fn      ARM_USART_CAPABILITIES ARM_USART_GetCapabilities_WO_RTS_CTS(void)
 * @brief   get uart capabilites without RTS-CTS.
 *          As per PinMux Only UART0-UART3(not UART4-UART7) instances has RTS/CTS functionality.
 *          UART4-UART7 instances has RS485 functionality.
 * @note    none
 * @param   none
 * @retval  driver capabilites without RTS-CTS.
 */
static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities_WO_RTS_CTS(void)
{
  return DriverCapabilities_WO_RTS_CTS;
}

#if UART_DMA_ENABLE
/**
  \fn          int32_t UART_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for UART
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t UART_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->Initialize();
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t UART_DMA_PowerControl(ARM_POWER_STATE state,
                                            DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       PowerControl DMA for UART
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t UART_DMA_PowerControl(ARM_POWER_STATE state,
                                             DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->PowerControl(state);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t UART_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for UART
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t UART_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Allocate handle for peripheral */
    status = dma_drv->Allocate(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Enable the channel in the Event Router */
    if(dma_periph->evtrtr_cfg.instance == 0)
    {
        evtrtr0_enable_dma_channel(dma_periph->evtrtr_cfg.channel,
                                   dma_periph->evtrtr_cfg.group,
                                   DMA_ACK_COMPLETION_PERIPHERAL);
        evtrtr0_enable_dma_handshake(dma_periph->evtrtr_cfg.channel,
                                     dma_periph->evtrtr_cfg.group);
    }
    else
    {
        evtrtrlocal_enable_dma_channel(dma_periph->evtrtr_cfg.channel,
                                       DMA_ACK_COMPLETION_PERIPHERAL);
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t UART_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of UART
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t UART_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* De-Allocate handle  */
    status = dma_drv->DeAllocate(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Disable the channel in the Event Router */
    if(dma_periph->evtrtr_cfg.instance == 0)
    {
        evtrtr0_disable_dma_channel(dma_periph->evtrtr_cfg.channel);
        evtrtr0_disable_dma_handshake(dma_periph->evtrtr_cfg.channel,
                                      dma_periph->evtrtr_cfg.group);
    }
    else
    {
        evtrtrlocal_disable_dma_channel(dma_periph->evtrtr_cfg.channel);
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t UART_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                     ARM_DMA_PARAMS *dma_params)
  \brief       Start UART DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref           execution_status
*/
__STATIC_INLINE int32_t UART_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                      ARM_DMA_PARAMS *dma_params)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Start transfer */
    status = dma_drv->Start(&dma_periph->dma_handle, dma_params);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t UART_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop UART DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t UART_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Stop transfer */
    status = dma_drv->Stop(&dma_periph->dma_handle);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t UART_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph
                                         uint32_t *count)
  \brief       Status of UART DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \param[in]   count        Current transfer count
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t UART_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph,
                                          uint32_t *count)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Stop transfer */
    status = dma_drv->GetStatus(&dma_periph->dma_handle, count);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}
#endif /* UART_DMA_ENABLE */

/**
 * @fn      int32_t ARM_USART_PowerControl (ARM_POWER_STATE    state,
                                            UART_RESOURCES    *uart)
 * @brief   CMSIS-Driver uart power control
 * @note    none
 * @param   state   : Power state
 * @param   uart    : Pointer to uart resources structure
 * @retval  \ref execution_status
 */
static int32_t ARM_USART_PowerControl (ARM_POWER_STATE   state,
                                       UART_RESOURCES   *uart)
{
    switch (state)
    {
        case ARM_POWER_OFF:
            /* Disable uart IRQ */
            NVIC_DisableIRQ (uart->irq_num);

            /* Clear Any Pending IRQ*/
            NVIC_ClearPendingIRQ (uart->irq_num);

            /* wait for data to be transmitted */
            while(READ_BIT(uart->regs->UART_LSR, BIT(6)) != UART_LSR_TRANSMITTER_EMPTY);

            /* uart EXPMST0 configuration,
             * Disable the selected UART instance. */
            if (uart->instance == UART_INSTANCE_LP) /* LPUART */
            {
                disable_lpuart_clock();
            }
            else /* UART0-7 */
            {
                disable_uart_clock(uart->instance);
            }

            /* Reset the power state of uart. */
            uart->state.powered = 0;
            break;

        case ARM_POWER_FULL:
            if (uart->state.initialized == 0U)
            {
                /* error: Driver is not initialized
                 * \ref ARM_USART_Initialize */
                return ARM_DRIVER_ERROR;
            }

            if (uart->state.powered)
            {
                return ARM_DRIVER_OK;
            }

            /* uart EXPMST0 configuration:
             *  Enable clock and clock source for
             *  selected UART instance. */
            if (uart->instance == UART_INSTANCE_LP) /* LPUART */
            {
                /* update peripheral clock frequency. */
                uart->clk = GetSystemCoreClock();

                /* enable LPUART clock. */
                enable_lpuart_clock();
            } /* if UART_INSTANCE_LP */
            else /* else UART0-7 */
            {
                if (uart->clk_source == UART_CLK_SOURCE_HFOSC_CLK)
                {
                    select_uart_clock_hfosc_clk(uart->instance);

                    /* update peripheral clock frequency. */
                    uart->clk = GetSystemHFOSClock();
                }

                if (uart->clk_source == UART_CLK_SOURCE_SYST_PCLK)
                {
                    select_uart_clock_syst_pclk(uart->instance);

                    /* update peripheral clock frequency. */
                    uart->clk = GetSystemAPBClock();
                }

                /* enable UART Clock. */
                enable_uart_clock(uart->instance);

#if UART_DMA_ENABLE
                /* DMA enable? */
                if(uart->dma_enable)
                {
                    /* For UART4-UART7,
                     *  check which DMA channel(from EXPMST0) is selected
                     *  DMA0 or DMA1?
                     */
                    if( (uart->instance >= UART_INSTANCE_4) &&
                        (uart->instance <= UART_INSTANCE_7) )
                    {
                        /* DMA1 is selected? */
                        if(uart->dma_cfg->dma_tx.evtrtr_cfg.instance == 1)
                        {
                            select_uart_dma1(uart->instance);
                        }
                        /* else: default DMA0 is selected. */
                    }
                } /* dma_enable */
#endif /* UART_DMA_ENABLE */

            } /* else UART0-7 */

#if UART_DMA_ENABLE
            /* DMA enable? */
            if(uart->dma_enable)
            {
                /* use UART DMA Mode-1. */
                uart_select_dma_mode1(uart->regs);
            }
#endif /* UART_DMA_ENABLE */

            /* reset the uart. */
            uart_software_reset(uart->regs);

            /* As per datasheet,
             *  Clock Support section:
             *   There is slightly more time(eight clock cycles of the slower clock)
             *   required after initial serial control register programming
             *   before serial data can be transmitted or received.
             */
            sys_busy_loop_us(5);

#if RS485_SUPPORT  /* Check if UART RS485 mode is enabled? */
            if(uart->rs485_cfg.rs485_control == UART_RS485_MODE_ENABLE)
            {
                /* enable UART RS485 mode */
                uart_rs485_enable(uart->regs);

                /* uart RS485 configuration from RTE_device.h */

                /* uart set RS485 transfer mode */
                if(uart->rs485_cfg.rs485_transfer_mode > UART_RS485_HW_CONTROL_HALF_DULPLX_MODE)
                    return ARM_DRIVER_ERROR_PARAMETER;
                uart_rs485_set_transfer_mode(uart->regs, uart->rs485_cfg.rs485_transfer_mode);

                /* uart set RS485 Driver Enable DE Assertion Time (8-bit) */
                uart_rs485_set_de_assertion_time(uart->regs,
                                   uart->rs485_cfg.rs485_de_assertion_time_8bit);

                /* uart set RS485 Driver Enable DE De-Assertion Time (8-bit) */
                uart_rs485_set_de_deassertion_time(uart->regs,
                                   uart->rs485_cfg.rs485_de_deassertion_time_8bit);

                /* uart set RS485 Turn Around Time TAT 16 bit for
                 * Driver Enable DE to Receive Enable RE */
                uart_rs485_set_de_to_re_turn_around_time(uart->regs,
                                   uart->rs485_cfg.rs485_de_to_re_turn_around_time_16bit);

                /* uart set RS485 Turn Around Time TAT 16 bit for
                 * Receive Enable RE to Driver Enable DE */
                uart_rs485_set_re_to_de_turn_around_time(uart->regs,
                                   uart->rs485_cfg.rs485_re_to_de_turn_around_time_16bit);

                /* set the state as RS485 enabled. */
                uart->state.rs485_enabled = 1;
            }
            /* else   RS485 mode is disable. */
#endif /* END of RS485_SUPPORT */

            /* enable uart fifo */
            uart_enable_fifo(uart->regs);

            /* disable transmit interrupt */
            uart_disable_tx_irq(uart->regs);

            /* disable receiver interrupt */
            uart_disable_rx_irq(uart->regs);

            /* Enable uart IRQ*/
            NVIC_ClearPendingIRQ (uart->irq_num);
            NVIC_SetPriority(uart->irq_num, uart->irq_priority);
            NVIC_EnableIRQ (uart->irq_num);

            /* Set the state as powered */
            uart->state.powered = 1;
            break;

        case ARM_POWER_LOW:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

#if UART_DMA_ENABLE
    /* DMA enable? */
    if(uart->dma_enable)
    {
        /* Power Control DMA for UART-Tx */
        if(UART_DMA_PowerControl(state, &uart->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        /* Power Control DMA for UART-Rx */
        if(UART_DMA_PowerControl(state, &uart->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_USART_Initialize (ARM_USART_SignalEvent_t   cb_event,
                                          UART_RESOURCES           *uart)
 * @brief   CMSIS-Driver uart initialize
 *          this function will
 *              -set tx/rx fifo length,
 *              -reset uart,
 *              -disable the interrupt,
 *              -intialize all the variable to 0
 * @note    none
 * @param   cb_event    : Pointer to USART Event \ref ARM_USART_SignalEvent
 * @param   uart        : Pointer to uart resources structure
 * @retval  \ref execution_status
 */
static int32_t ARM_USART_Initialize (ARM_USART_SignalEvent_t   cb_event,
                                     UART_RESOURCES           *uart)
{
    if (uart->state.initialized == 1)
    {
        /* Driver is already initialized */
        return ARM_DRIVER_OK;
    }

    /* set the user callback event. */
    uart->cb_event = cb_event;

    /* initialize the tx_buffer */
    uart->transfer.tx_buf              = NULL;
    uart->transfer.tx_total_num        = 0U;
    uart->transfer.tx_curr_cnt         = 0U;

    /* clear TX busy flag */
    uart->status.tx_busy               = UART_STATUS_FREE;

    /* initialize the rx_buffer */
    uart->transfer.rx_buf              = NULL;
    uart->transfer.rx_total_num        = 0U;
    uart->transfer.rx_curr_cnt         = 0U;

    /* clear Receive busy flag */
    uart->status.rx_busy               = UART_STATUS_FREE;

    /* Clear RX status */
    uart->status.rx_break              = 0U;
    uart->status.rx_framing_error      = 0U;
    uart->status.rx_overflow           = 0U;
    uart->status.rx_parity_error       = 0U;

#if UART_DMA_ENABLE
    /* DMA enable? */
    if(uart->dma_enable)
    {
        uart->dma_cfg->dma_rx.dma_handle = -1;
        uart->dma_cfg->dma_tx.dma_handle = -1;

        /* Initialize DMA for UART-Tx */
        if(UART_DMA_Initialize(&uart->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        /* Initialize DMA for UART-Rx */
        if(UART_DMA_Initialize(&uart->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    /* set the state as initialized. */
    uart->state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_USART_Uninitialize (UART_RESOURCES *uart)
 * @brief   CMSIS-Driver uart uninitialize
 *          this function will
 *              -disable the interrupt,
 *              -abort TX/RX,
 *              -reset tx/rx fifo,
 *              -set baudrate to 0
 *              -initialize all the variable to 0
 * @note    needs to initialize first if wants to use it again.
 * @param   uart    : Pointer to uart resources structure
 * @retval  \ref execution_status
 */
static int32_t ARM_USART_Uninitialize (UART_RESOURCES *uart)
{
    if (uart->state.initialized == 0)
    {
        /* Driver is not initialized */
        return ARM_DRIVER_OK;
    }

    /* set the user callback event to NULL. */
    uart->cb_event = NULL;

    /* disable transmit interrupt */
    uart_disable_tx_irq(uart->regs);

    /* disable receiver interrupt */
    uart_disable_rx_irq(uart->regs);

    /* reset TX fifo */
    uart_reset_txfifo(uart->regs);

    /* Reset RX fifo */
    uart_reset_rxfifo(uart->regs);

    /* set baudrate to 0 */
    uart_set_baudrate(uart->regs, uart->clk, 0);

    /* update baudrate to uart resource structure */
    uart->baudrate = 0;

    /* initialize all variables to 0 */
#if UART_DMA_ENABLE
    /* DMA enable? */
    if(uart->dma_enable)
    {
        uart->dma_cfg->dma_rx.dma_handle = -1;
        uart->dma_cfg->dma_tx.dma_handle = -1;
    }
#endif

    /* initialize the tx_buffer */
    uart->transfer.tx_buf              = NULL;
    uart->transfer.tx_total_num        = 0U;
    uart->transfer.tx_curr_cnt         = 0U;

    /* clear TX busy flag */
    uart->status.tx_busy               = UART_STATUS_FREE;

    /* initialize the rx_buffer */
    uart->transfer.rx_buf              = NULL;
    uart->transfer.rx_total_num        = 0U;
    uart->transfer.rx_curr_cnt         = 0U;

    /* clear Receive active flag */
    uart->status.rx_busy               = UART_STATUS_FREE;

    /* Clear RX status */
    uart->status.rx_break              = 0U;
    uart->status.rx_framing_error      = 0U;
    uart->status.rx_overflow           = 0U;
    uart->status.rx_parity_error       = 0U;

#if RS485_SUPPORT  /* Check if UART RS485 mode is enabled? */
    if(uart->rs485_cfg.rs485_control == UART_RS485_MODE_ENABLE)
    {
        /* disable UART RS485 mode */
        uart_rs485_disable(uart->regs);
    }
#endif /* END of RS485_SUPPORT */

    /* Reset UART state. */
    uart->state.initialized = 0;
    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_USART_Send (const void       *data,
                                    uint32_t          num,
                                    UART_RESOURCES   *uart)
 * @brief   CMSIS-Driver uart send
 *          Start sending data to UART transmitter.
 * @note    tx flag UART_FLAG_TX_ENABLED should be enabled first
 *          \ref ARM_USART_CONTROL_TX
 * @param   data : Pointer to buffer with data to send to USART transmitter
 * @param   num  : Number of data items to send
 * @param   uart : Pointer to uart resources structure
 * @retval  \ref execution_status
 */
static int32_t ARM_USART_Send (const void       *data,
                               uint32_t          num,
                               UART_RESOURCES   *uart)
{
    if ((data == NULL) || (num == 0U))
    {
        /* invalid parameters */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (uart->state.powered == 0)
        return ARM_DRIVER_ERROR;

    if (uart->state.tx_enabled == 0U)
    {
        /* error: UART is not configured (mode not selected)
         * tx flag UART_FLAG_TX_ENABLED should be enabled first
         * \ref ARM_USART_CONTROL_TX
         */
        return ARM_DRIVER_ERROR;
    }

    /* fill the user input details for
     * uart send transfer structure.
     */

    /* check previous TX is completed or not? */
    if (uart->status.tx_busy != UART_STATUS_FREE)
    {
        /* return busy as previous send is not yet completed */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Set TX busy flag to active */
    uart->status.tx_busy        = UART_STATUS_BUSY;

    /* fill the uart transfer structure as per user input */
    uart->transfer.tx_buf       = (uint8_t *)data;
    uart->transfer.tx_total_num = num;
    uart->transfer.tx_curr_cnt  = 0U;

#if UART_DMA_ENABLE
    /* DMA enable? */
    if(uart->dma_enable)
    {
        int32_t        status;
        ARM_DMA_PARAMS dma_params;

        /* Start the DMA engine for sending the data to UART */
        dma_params.peri_reqno    = (int8_t)uart->dma_cfg->dma_tx.dma_periph_req;
        dma_params.dir           = ARM_DMA_MEM_TO_DEV;
        dma_params.cb_event      = uart->dmatx_cb;
        dma_params.src_addr      = (void *)data;
        dma_params.dst_addr      = uart_get_dma_tx_addr(uart->regs);
        dma_params.num_bytes     = uart->transfer.tx_total_num;
        dma_params.irq_priority  = uart->dma_irq_priority;

        /* As per UART protocol, only 1 Byte(8-bit) can be send at a time. */
        dma_params.burst_size = BS_BYTE_1;

        /* decide burst length based on TX Trigger value */
        dma_params.burst_len  = UART_FIFO_DEPTH - uart_get_decoded_tx_trigger(uart->regs);
        if( dma_params.burst_len > 16)
        {
            dma_params.burst_len = 16;
        }

        /* Start DMA transfer */
        status = UART_DMA_Start(&uart->dma_cfg->dma_tx, &dma_params);
        if(status)
            return ARM_DRIVER_ERROR;
    } /* if DMA enable? */
    else
#endif /* UART_DMA_ENABLE */
    {
#if UART_BLOCKING_MODE_ENABLE
        /* Blocking(Polling) mode enable? */
        if(uart->blocking_mode)
        {
            /* Blocking call(Polling Method),
             *  this will block till uart sends all the data. */
            uart_send_blocking(uart->regs, &uart->transfer);

            /* clear TX busy flag */
            uart->status.tx_busy   = UART_STATUS_FREE;
        }
        else
#endif /* UART_BLOCKING_MODE_ENABLE */
        {
            /* enable transmitter interrupt */
            uart_enable_tx_irq(uart->regs);
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_USART_Receive (void             *data,
                                       uint32_t          num,
                                       UART_RESOURCES   *uart)
 * @brief   CMSIS-Driver uart receive
 *          Start receiving data from USART receiver.
 * @note    none
 * @param   data   : Pointer to buffer for data to receive from UART receiver
 * @param   num    : Number of data items to receive
 * @param   uart   : Pointer to uart resources structure
 * @retval  \ref execution_status
 */
static int32_t ARM_USART_Receive (void             *data,
                                  uint32_t          num,
                                  UART_RESOURCES   *uart)
{
    if ((data == NULL) || (num == 0U))
    {
        /* invalid parameters */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (uart->state.powered == 0)
        return ARM_DRIVER_ERROR;

    if (uart->state.rx_enabled == 0U)
    {
        /* error: UART is not configured (mode not selected) */
        return ARM_DRIVER_ERROR;
    }

    /* fill the user input details for
     * uart receive transfer structure.
     */

    /* check previous receive is completed or not? */
    if (uart->status.rx_busy == UART_STATUS_BUSY)
    {
        /* return busy as previous receive is not yet completed */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* set rx busy flag to active */
    uart->status.rx_busy           = UART_STATUS_BUSY;

    /* clear rx status */
    uart->status.rx_break          = 0U;
    uart->status.rx_framing_error  = 0U;
    uart->status.rx_overflow       = 0U;
    uart->status.rx_parity_error   = 0U;

    /* fill the uart transfer structure as per user input */
    uart->transfer.rx_buf          = (uint8_t *)data;
    uart->transfer.rx_total_num    = num;
    uart->transfer.rx_curr_cnt     = 0U;

#if UART_DMA_ENABLE
    /* DMA enable? */
    if(uart->dma_enable)
    {
        ARM_DMA_PARAMS dma_params;
        int32_t        status;

        /* Start the DMA engine for sending the data to UART */
        dma_params.peri_reqno    = (int8_t)uart->dma_cfg->dma_rx.dma_periph_req;
        dma_params.dir           = ARM_DMA_DEV_TO_MEM;
        dma_params.cb_event      = uart->dmarx_cb;
        dma_params.src_addr      = uart_get_dma_rx_addr(uart->regs);
        dma_params.dst_addr      = (void *)data;
        dma_params.num_bytes     = uart->transfer.rx_total_num;
        dma_params.irq_priority  = uart->dma_irq_priority;

        /* As per UART protocol, only 1 Byte(8-bit) can be send at a time. */
        dma_params.burst_size = BS_BYTE_1;

        /* decide burst length based on RX Trigger value */
        dma_params.burst_len  = uart_get_decoded_rx_trigger(uart->regs);
        if( dma_params.burst_len > 16)
        {
            dma_params.burst_len = 16;
        }

        /* Start DMA transfer */
        status = UART_DMA_Start(&uart->dma_cfg->dma_rx, &dma_params);
        if(status)
            return ARM_DRIVER_ERROR;
    } /* if DMA enable? */
    else
#endif /* UART_DMA_ENABLE */
    {
#if UART_BLOCKING_MODE_ENABLE
        /* Blocking(Polling) mode enable? */
        if(uart->blocking_mode)
        {
            UART_TRANSFER *transfer = &(uart->transfer);

            /* Blocking call(Polling Method),
             *  this will block till uart receives all the data. */
            uart_receive_blocking(uart->regs, transfer);

            /* check for transfer error. */
            if(transfer->status & UART_TRANSFER_STATUS_ERROR)
            {
                /* there can be multiple RX line status,
                 * break character implicitly generates a framing error / parity error.
                 */

                /* update uart transfer status. */
                if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_BREAK)
                {
                    uart->status.rx_break = 1;
                }

                if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_FRAMING)
                {
                    uart->status.rx_framing_error = 1;
                }

                if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_PARITY)
                {
                    uart->status.rx_parity_error = 1;
                }

                if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_OVERRUN)
                {
                    uart->status.rx_overflow = 1;
                }

                /* clear transfer status */
                transfer->status = UART_TRANSFER_STATUS_NONE;
            }

            /* clear RX busy flag */
            uart->status.rx_busy   = UART_STATUS_FREE;
        }
        else
#endif /* UART_BLOCKING_MODE_ENABLE */
        {
            /* enable receiver interrupt */
            uart_enable_rx_irq(uart->regs);
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_USART_Transfer (const void       *data_out,
                                        void             *data_in,
                                        uint32_t          num,
                                        UART_RESOURCES   *uart)
 * @brief   CMSIS-Driver uart transfer
 *          Start sending/receiving data to/from UART transmitter/receiver.
 * @note    This function used in synchronous mode, currently our driver is not supporting it.
 * @param   data_out    : Pointer to buffer with data to send to USART transmitter
 * @param   data_in     : Pointer to buffer for data to receive from USART receiver
 * @param   num         : Number of data items to transfer
 * @param   uart        : Pointer to uart resources structure
 * @retval  ARM_DRIVER_ERROR_UNSUPPORTED
 */
static int32_t ARM_USART_Transfer (const void       *data_out,
                                   void             *data_in,
                                   uint32_t          num,
                                   UART_RESOURCES   *uart)
{
    /* Added to fix Warning: unused parameter [-Wunused-parameter] */
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);
    ARG_UNUSED(uart);

    /* Use with Synchronous mode only */
    /* Not supported as our driver is only Asynchronous. */
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @fn      uint32_t ARM_USART_GetTxCount (UART_RESOURCES *uart)
 * @brief   CMSIS-Driver uart get transmitted data count
 * @note    none
 * @param   uart    : Pointer to uart resources structure
 * @retval  transmitted data count
 */
static uint32_t ARM_USART_GetTxCount (UART_RESOURCES *uart)
{

#if UART_DMA_ENABLE
    uint32_t tx_current_cnt = 0;

    /* DMA enable? */
    if(uart->dma_enable)
    {
        /* Get the current transfer count */
        UART_DMA_GetStatus(&uart->dma_cfg->dma_tx, &tx_current_cnt);

        uart->transfer.tx_curr_cnt = tx_current_cnt;
    }
#endif /* UART_DMA_ENABLE */

    return uart->transfer.tx_curr_cnt;
}

/**
 * @fn      uint32_t ARM_USART_GetRxCount (UART_RESOURCES *uart)
 * @brief   CMSIS-Driver uart get received data count
 * @note    none
 * @param   uart    : Pointer to uart resources structure
 * @retval  received data count
 */
static uint32_t ARM_USART_GetRxCount (UART_RESOURCES *uart)
{

#if UART_DMA_ENABLE
    uint32_t rx_current_cnt = 0;

    /* DMA enable? */
    if(uart->dma_enable)
    {
        /* Get the current transfer count */
        UART_DMA_GetStatus(&uart->dma_cfg->dma_rx, &rx_current_cnt);

        uart->transfer.rx_curr_cnt = rx_current_cnt;
    }
#endif /* UART_DMA_ENABLE */

    return uart->transfer.rx_curr_cnt;
}

/**
 * @fn      int32_t UartAsynchronousModeCtrl (UART_RESOURCES *uart,
                                              uint32_t        control,
                                              uint32_t        arg)
 * @brief   set uart asynchronous parameters
 *          baudrate, data bits, parity, stop bits, flow control
 * @note    none
 * @param   uart      : Pointer to uart resources structure
 * @param   control   : Control Operation
 * @param   arg       : Argument of operation
 * @retval  \ref execution_status
 */
static int32_t UartAsynchronousModeCtrl (UART_RESOURCES *uart,
                                         uint32_t        control,
                                         uint32_t        arg)
{
    UART_DATA_BITS    data_bits    = 0;
    UART_PARITY       parity       = 0;
    UART_STOP_BITS    stop_bits    = 0;
    UART_FLOW_CONTROL flow_control = 0;
    uint32_t            baudrate     = arg;

    /* set the uart baudrate */
    uart_set_baudrate(uart->regs, uart->clk, baudrate);

    /* update baudrate to uart resource structure */
    uart->baudrate = baudrate;

    /* UART Data bits */
    switch (control & ARM_USART_DATA_BITS_Msk)
    {
        case ARM_USART_DATA_BITS_5: data_bits = UART_DATA_BITS_5; break;
        case ARM_USART_DATA_BITS_6: data_bits = UART_DATA_BITS_6; break;
        case ARM_USART_DATA_BITS_7: data_bits = UART_DATA_BITS_7; break;
        case ARM_USART_DATA_BITS_8: data_bits = UART_DATA_BITS_8; break;
        default: return ARM_USART_ERROR_DATA_BITS;
    }

    /* UART Parity */
    switch (control & ARM_USART_PARITY_Msk)
    {
        case ARM_USART_PARITY_NONE: parity = UART_PARITY_NONE; break;
        case ARM_USART_PARITY_EVEN: parity = UART_PARITY_EVEN; break;
        case ARM_USART_PARITY_ODD:  parity = UART_PARITY_ODD;  break;
        default: return ARM_USART_ERROR_PARITY;
    }

    /* UART Stop bits */
    switch (control & ARM_USART_STOP_BITS_Msk)
    {
        case ARM_USART_STOP_BITS_1: stop_bits = UART_STOP_BITS_1; break;
        case ARM_USART_STOP_BITS_2: stop_bits = UART_STOP_BITS_2; break;
        default: return ARM_USART_ERROR_STOP_BITS;
    }

    /* set data, parity, stop bits */
    uart_set_data_parity_stop_bits(uart->regs, data_bits, parity, stop_bits);

    /* uart flow control */
    switch (control & ARM_USART_FLOW_CONTROL_Msk)
    {
        case ARM_USART_FLOW_CONTROL_NONE:
            flow_control = UART_FLOW_CONTROL_NONE;
            break;

        case ARM_USART_FLOW_CONTROL_RTS:
            /*  As per PINMUX,
             *    UART0-UART3 instances has RS232 with RTS/CTS functionality and
             *    UART4-UART7 instances has RS232 without RTS/CTS and RS485 functionality.
             */
            if( (uart->instance >= 4) && (uart->instance <= 7) )
                return ARM_USART_ERROR_FLOW_CONTROL;   /* Unsupported. */

            flow_control = UART_FLOW_CONTROL_RTS;
            break;

        case ARM_USART_FLOW_CONTROL_CTS:
            /*  As per PINMUX,
             *    UART0-UART3 instances has RS232 with RTS/CTS functionality and
             *    UART4-UART7 instances has RS232 without RTS/CTS and RS485 functionality.
             */
            if( (uart->instance >= 4) && (uart->instance <= 7) )
                return ARM_USART_ERROR_FLOW_CONTROL;   /* Unsupported. */

            flow_control = UART_FLOW_CONTROL_CTS;
            break;

        case ARM_USART_FLOW_CONTROL_RTS_CTS:
            /*  As per PINMUX,
             *    UART0-UART3 instances has RS232 with RTS/CTS functionality and
             *    UART4-UART7 instances has RS232 without RTS/CTS and RS485 functionality.
             */
            if( (uart->instance >= 4) && (uart->instance <= 7) )
                return ARM_USART_ERROR_FLOW_CONTROL;   /* Unsupported. */

            flow_control = UART_FLOW_CONTROL_RTS_CTS;
            break;

        default: return ARM_USART_ERROR_FLOW_CONTROL;
    }

    uart_set_flow_control(uart->regs, flow_control);

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_USART_Control (uint32_t        control,
                                       uint32_t        arg,
                                       UART_RESOURCES *uart)
 * @brief   CMSIS-Driver uart control.
 *          Control USART Interface.
 * @note    none
 * @param   control   : Operation \ref Driver_USART.h: USART Control Codes
 * @param   arg       : Argument of operation (optional)
 * @param   uart      : Pointer to uart resources structure
 * @retval  common \ref execution_status and
 *          driver specific \ref usart_execution_status
 */
static int32_t ARM_USART_Control (uint32_t         control,
                                  uint32_t         arg,
                                  UART_RESOURCES  *uart)
{
    int32_t ret = ARM_DRIVER_OK;

    /* if not powered? then return error */
    if (uart->state.powered == 0)
        return ARM_DRIVER_ERROR;

    switch (control & ARM_USART_CONTROL_Msk)
    {
        case ARM_USART_MODE_ASYNCHRONOUS:
            /* uart asynchronous mode */

            /* set uart asynchronous mode parameters as per arg
             * set baudrate, data bits, parity, stop bits and flow control.
             */
            ret = UartAsynchronousModeCtrl(uart, control, arg);
            break;

        case ARM_USART_CONTROL_TX:
            /* uart enable/disable transmitter */

            if (arg) /* uart enable transmitter */
            {

#if UART_DMA_ENABLE
                /* DMA enable? */
                if(uart->dma_enable)
                {
                    /* Try to allocate a DMA channel */
                    if(UART_DMA_Allocate(&uart->dma_cfg->dma_tx))
                        return ARM_DRIVER_ERROR;
                }
#endif

#if RS485_SUPPORT  /* Check if UART RS485 mode is enabled? */
                if (uart->state.rs485_enabled == 1)
                {
                    /* uart RS485 mode is enabled,
                     * now enable Driver-Enable DE_EN signal. */
                    uart_rs485_enable_de_en(uart->regs);
                }
#endif
                if(uart->tx_fifo_trg_lvl > UART_TX_FIFO_HALF_FULL)
                    return ARM_DRIVER_ERROR_PARAMETER;

                /* uart set transmitter trigger level as per
                 * RTE configuration */
                uart_set_tx_trigger(uart->regs, uart->tx_fifo_trg_lvl);

                /* set state as tx enabled. */
                uart->state.tx_enabled = 1;
            }
            else /* uart disable transmitter */
            {

#if UART_DMA_ENABLE
                /* DMA enable? */
                if(uart->dma_enable)
                {
                    /* Deallocate DMA channel */
                    if(UART_DMA_DeAllocate(&uart->dma_cfg->dma_tx) == ARM_DRIVER_ERROR)
                        return ARM_DRIVER_ERROR;
                }
#endif

#if RS485_SUPPORT /* Check if UART RS485 mode is enabled? */
                if (uart->state.rs485_enabled == 1)
                {
                    /* uart RS485 mode is enabled,
                     * now disable Driver-Enable DE_EN signal. */
                    uart_rs485_disable_de_en(uart->regs);
                }
#endif
                /* clear state as tx enabled. */
                uart->state.tx_enabled = 0;
            }
            break;

        case ARM_USART_CONTROL_RX:
            /* uart enable/disable receiver */

            if (arg) /* uart enable receiver */
            {

#if UART_DMA_ENABLE
                /* Check if DMA is enabled for this */
                if(uart->dma_enable)
                {
                    /* Try to allocate a DMA channel */
                    if(UART_DMA_Allocate(&uart->dma_cfg->dma_rx))
                        return ARM_DRIVER_ERROR;
                }
#endif

#if RS485_SUPPORT  /* Check if UART RS485 mode is enabled? */
                if (uart->state.rs485_enabled == 1)
                {
                    /* uart RS485 mode is enabled,
                     * now enable Receiver-Enable RE_EN signal. */
                    uart_rs485_enable_re_en(uart->regs);
                }
#endif
                if(uart->rx_fifo_trg_lvl > UART_RX_FIFO_TWO_LESS_FULL)
                    return ARM_DRIVER_ERROR_PARAMETER;

                /* uart set receiver trigger level as per
                 * RTE configuration */
                uart_set_rx_trigger(uart->regs, uart->rx_fifo_trg_lvl);

                /* set state as rx enabled. */
                uart->state.rx_enabled = 1;
            }
            else /* uart disable receiver */
            {

#if UART_DMA_ENABLE
                /* Check if DMA is enabled for this */
                if(uart->dma_enable)
                {
                    /* Deallocate DMA channel */
                    if(UART_DMA_DeAllocate(&uart->dma_cfg->dma_rx))
                        return ARM_DRIVER_ERROR;
                }
#endif

#if RS485_SUPPORT  /* Check if UART RS485 mode is enabled? */
                if (uart->state.rs485_enabled == 1)
                {
                    /* uart RS485 mode is enabled,
                     * now disable Receiver-Enable RE_EN signal. */
                    uart_rs485_disable_re_en(uart->regs);
                }
#endif
                /* clear state as rx enabled. */
                uart->state.rx_enabled = 0;
            }
            break;

        case ARM_USART_ABORT_SEND:
            /* uart abort transmitter */
            if (uart->state.tx_enabled == 0U)
            {
                /* error: UART transmitter is not enabled
                 * \ref ARM_USART_CONTROL_TX
                 */
                return ARM_DRIVER_ERROR;
            }

#if UART_DMA_ENABLE
            /* Check if DMA is enabled for this */
            if(uart->dma_enable)
            {
                /* Stop DMA transfer */
                if(UART_DMA_Stop(&uart->dma_cfg->dma_tx))
                    return ARM_DRIVER_ERROR;
            }
#endif
            /* disable transmit interrupt */
            uart_disable_tx_irq(uart->regs);

            /* reset TX fifo */
            uart_reset_txfifo(uart->regs);

            /* clear TX busy flag */
            uart->status.tx_busy = UART_STATUS_FREE;

            /* Reset the tx_buffer */
            uart->transfer.tx_total_num = 0U;

            break;

        case ARM_USART_ABORT_RECEIVE:
            /* uart abort receiver */
            if (uart->state.rx_enabled == 0U)
            {
                /* error: UART receiver is not enabled
                 * \ref ARM_USART_CONTROL_RX
                 */
                return ARM_DRIVER_ERROR;
            }

#if UART_DMA_ENABLE
            /* Check if DMA is enabled for this */
            if(uart->dma_enable)
            {
                /* Stop DMA transfer */
                if(UART_DMA_Stop(&uart->dma_cfg->dma_rx))
                    return ARM_DRIVER_ERROR;
            }
#endif
            /* disable receiver interrupt */
            uart_disable_rx_irq(uart->regs);

            /* Reset rx fifo */
            uart_reset_rxfifo(uart->regs);

            /* clear Receive busy flag */
            uart->status.rx_busy = UART_STATUS_FREE;

            /* Reset the rx_buffer */
            uart->transfer.rx_total_num = 0U;

            break;

        case ARM_USART_CONTROL_BREAK:
            /* set/clear break */

            if (arg)
            {
                uart_set_break_control(uart->regs);
            }
            else
            {
                uart_clear_break_control(uart->regs);
            }
            break;

        /* Unsupported command */
        default:
            ret =  ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
    }
    return ret;
}

/**
 * @fn      ARM_USART_STATUS ARM_USART_GetStatus (UART_RESOURCES *uart)
 * @brief   CMSIS-Driver uart get status
 * @note    none.
 * @param   uart    : Pointer to uart resources structure
 * @retval  ARM_USART_STATUS
 */
static ARM_USART_STATUS ARM_USART_GetStatus (UART_RESOURCES *uart)
{
    return uart->status;
}

/**
 * @fn      int32_t ARM_USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                               UART_RESOURCES          *uart)
 * @brief   CMSIS-Driver Set UART Modem Control line state.
 * @note    not implemented yet.
 * @param   control : \ref ARM_USART_MODEM_CONTROL
 * @param   uart    : Pointer to uart resources structure
 * @retval  \ref execution_status
 */
static int32_t ARM_USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                          UART_RESOURCES          *uart)
{
    /* Added to fix Warning: unused parameter [-Wunused-parameter] */
    ARG_UNUSED(control);
    ARG_UNUSED(uart);

    /* not implemented yet. */
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
 * @fn      ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus (UART_RESOURCES *uart)
 * @brief   CMSIS-Driver uart Get UART Modem Status lines state.
 * @note    not implemented yet.
 * @param   uart    : Pointer to uart resources structure
 * @retval  modem status \ref ARM_USART_MODEM_STATUS
 */
static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus (UART_RESOURCES *uart)
{
    /* Added to fix Warning: unused parameter [-Wunused-parameter] */
    ARG_UNUSED(uart);

    /* not implemented yet. */
    ARM_USART_MODEM_STATUS status = {0, 0, 0, 0, 0};
    return status;
}

/**
 * @fn      void UART_IRQHandler(UART_RESOURCES *uart)
 * @brief   UART IRQ Handler
 * @param   uart    : Pointer to uart resources structure
 * @retval  none
 */
static void UART_IRQHandler(UART_RESOURCES *uart)
{
    UART_TRANSFER *transfer = &(uart->transfer);
    uint32_t cb_event = 0U;

    uart_irq_handler(uart->regs, transfer);

    /* check for transfer error. */
    if(transfer->status & UART_TRANSFER_STATUS_ERROR)
    {
        /* there can be multiple RX line status,
         * break character implicitly generates framing error / parity error.
         */

        /* update uart transfer status and callback event. */
        if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_BREAK)
        {
            uart->status.rx_break = 1;
            cb_event |= ARM_USART_EVENT_RX_BREAK;
        }

        if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_FRAMING)
        {
            uart->status.rx_framing_error = 1;
            cb_event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
        }

        if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_PARITY)
        {
            uart->status.rx_parity_error = 1;
            cb_event |= ARM_USART_EVENT_RX_PARITY_ERROR;
        }

        if(transfer->status & UART_TRANSFER_STATUS_ERROR_RX_OVERRUN)
        {
            uart->status.rx_overflow = 1;
            cb_event |= ARM_USART_EVENT_RX_OVERFLOW;
        }

        /* clear transfer status */
        transfer->status = UART_TRANSFER_STATUS_NONE;

        /* in error case not clearing busy flag
         * it is up to user to decide whether
         * to wait for remaining bytes or call the abort rx.
        */

        /* mark event as error and call the user callback */
        if(uart->cb_event)
            uart->cb_event(cb_event);
    }

    /* check for transfer send complete. */
    if(transfer->status & UART_TRANSFER_STATUS_SEND_COMPLETE)
    {
        /* clear transfer status */
        transfer->status = UART_TRANSFER_STATUS_NONE;

        /* clear TX busy flag */
        uart->status.tx_busy = UART_STATUS_FREE;

        /* mark event as Send Complete and call the user callback */
        if(uart->cb_event)
            uart->cb_event(ARM_USART_EVENT_SEND_COMPLETE);
    }

    /* check for transfer receive complete OR If the data requested by
     * application is already received then mark the transaction as complete
     * even if there is RX timeout.
     */
    if((uart->status.rx_busy == UART_STATUS_BUSY)                  &&
       ((transfer->status & UART_TRANSFER_STATUS_RECEIVE_COMPLETE) ||
        (transfer->rx_total_num == transfer->rx_curr_cnt)))
    {
        /* clear transfer status */
        transfer->status = UART_TRANSFER_STATUS_NONE;

        /* clear rx busy flag */
        uart->status.rx_busy = UART_STATUS_FREE;

        /* mark event as Receive Complete and call the user callback */
        if(uart->cb_event)
            uart->cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
    }

    /* check for transfer receive timeout. */
    if(transfer->status & UART_TRANSFER_STATUS_RX_TIMEOUT)
    {
        /* clear transfer status */
        transfer->status = UART_TRANSFER_STATUS_NONE;

        /* in RX_Timeout case not clearing rx busy flag
         * it is up to user to decide whether
         * to wait for remaining bytes or call the abort rx.
        */

        /* mark event as RX Timeout and call the user callback */
        if(uart->cb_event)
            uart->cb_event(ARM_USART_EVENT_RX_TIMEOUT);
    }
}

#if UART_DMA_ENABLE
/**
  \fn          static void  UART_DMATxCallback(uint32_t event, int8_t peri_num,
                                            UART_RESOURCES *uart)
  \brief       Callback function from DMA for UART Tx
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \param[in]   uart       Pointer to UART resources
*/
static void UART_DMATxCallback(uint32_t event, int8_t peri_num,
                               UART_RESOURCES *uart)
{
    /* Transfer Completed */
    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        switch(peri_num)
        {
        case UART0_DMA_TX_PERIPH_REQ:
        case UART1_DMA_TX_PERIPH_REQ:
        case UART2_DMA_TX_PERIPH_REQ:
        case UART3_DMA_TX_PERIPH_REQ:
        case UART4_DMA_TX_PERIPH_REQ:
        case UART5_DMA_TX_PERIPH_REQ:
        case UART6_DMA_TX_PERIPH_REQ:
        case UART7_DMA_TX_PERIPH_REQ:
#if defined (M55_HE)
        case LPUART_DMA_TX_PERIPH_REQ:
#endif
            /* clear the Tx flag. */
            uart->status.tx_busy = 0U;

            /* mark event as Send Complete and
             * call the user callback */
            if(uart->cb_event)
                uart->cb_event(ARM_USART_EVENT_SEND_COMPLETE);
            break;

        default:
            break;
        }
    } /* if event & ARM_DMA_EVENT_COMPLETE */

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {
        /* not implemented yet. */
    }
}

/**
  \fn          static void  UART_DMARxCallback(uint32_t event, int8_t peri_num,
                                               UART_RESOURCES *uart)
  \brief       Callback function from DMA for UART Rx
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \param[in]   uart       Pointer to UART resources
*/
static void UART_DMARxCallback(uint32_t event, int8_t peri_num,
                               UART_RESOURCES *uart)
{
    /* Transfer Completed */
    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        switch(peri_num)
        {
        case UART0_DMA_RX_PERIPH_REQ:
        case UART1_DMA_RX_PERIPH_REQ:
        case UART2_DMA_RX_PERIPH_REQ:
        case UART3_DMA_RX_PERIPH_REQ:
        case UART4_DMA_RX_PERIPH_REQ:
        case UART5_DMA_RX_PERIPH_REQ:
        case UART6_DMA_RX_PERIPH_REQ:
        case UART7_DMA_RX_PERIPH_REQ:
        /* case LPUART_DMA_RX_PERIPH_REQ:
         *  muxed with UART0_DMA_RX_PERIPH_REQ */

            /* clear the Rx flag */
            uart->status.rx_busy = 0U;

            /* mark event as Receive Complete and
             * call the user callback */
            if(uart->cb_event)
                uart->cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
            break;

        default:
            break;
        }
    } /* if event & ARM_DMA_EVENT_COMPLETE */

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {
        /* not implemented yet. */
    }
}
#endif /* UART_DMA_ENABLE */

/* End UART Interface */


/* UART0 Driver Instance */
#if (RTE_UART0)

#if RTE_UART0_DMA_ENABLE
static void UART0_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART0_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART0_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART0_DMA),
        .dma_periph_req = UART0_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART0_DMA,
             .group    = UART0_DMA_GROUP,
             .channel  = UART0_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART0_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART0_DMA),
        .dma_periph_req = UART0_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART0_DMA,
             .group    = UART0_DMA_GROUP,
             .channel  = UART0_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART0_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART0_DMA_ENABLE */

/* UART0 Driver Resources */
static UART_RESOURCES UART0 =
{
    .regs             = (UART_Type *)UART0_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART0_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART0_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART0_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART0_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART0_IRQ_PRI,
    .instance         = UART_INSTANCE_0,

#if RTE_UART0_DMA_ENABLE
    .dma_enable       = RTE_UART0_DMA_ENABLE,
    .dma_irq_priority = RTE_UART0_DMA_IRQ_PRI,
    .dmatx_cb         = UART0_DMATxCallback,
    .dmarx_cb         = UART0_DMARxCallback,
    .dma_cfg          = &UART0_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART0_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* UART0 does not support RS485 */
    .rs485_cfg        = {0},
#endif /* END of RS485_SUPPORT */
};

void UART0_IRQHandler (void)
{
    UART_IRQHandler(&UART0);
}

#if RTE_UART0_DMA_ENABLE
static void UART0_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART0);
}

static void UART0_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART0);
}
#endif /* RTE_UART0_DMA_ENABLE */

static int32_t UART0_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART0));
}

static int32_t UART0_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART0));
}

static int32_t UART0_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART0));
}

static int32_t UART0_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART0));
}

static int32_t UART0_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART0));
}

static int32_t UART0_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART0));
}

static uint32_t UART0_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART0));
}

static uint32_t UART0_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART0));
}

static int32_t UART0_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART0));
}

static ARM_USART_STATUS UART0_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART0));
}

static ARM_USART_MODEM_STATUS UART0_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART0));
}

static int32_t UART0_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART0));
}

extern ARM_DRIVER_USART Driver_USART0;
ARM_DRIVER_USART Driver_USART0 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    UART0_Initialize,
    UART0_Uninitialize,
    UART0_PowerControl,
    UART0_Send,
    UART0_Receive,
    UART0_Transfer,
    UART0_GetTxCount,
    UART0_GetRxCount,
    UART0_Control,
    UART0_GetStatus,
    UART0_SetModemControl,
    UART0_GetModemStatus
};
#endif /* RTE_UART0 */


/* UART1 Driver Instance */
#if (RTE_UART1)

#if RTE_UART1_DMA_ENABLE
static void UART1_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART1_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART1_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART1_DMA),
        .dma_periph_req = UART1_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART1_DMA,
             .group    = UART1_DMA_GROUP,
             .channel  = UART1_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART1_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART1_DMA),
        .dma_periph_req = UART1_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART1_DMA,
             .group    = UART1_DMA_GROUP,
             .channel  = UART1_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART1_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART1_DMA_ENABLE */

/* UART1 Driver Resources */
static UART_RESOURCES UART1 =
{
    .regs             = (UART_Type *)UART1_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART1_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART1_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART1_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART1_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART1_IRQ_PRI,
    .instance         = UART_INSTANCE_1,

#if RTE_UART1_DMA_ENABLE
    .dma_enable       = RTE_UART1_DMA_ENABLE,
    .dma_irq_priority = RTE_UART1_DMA_IRQ_PRI,
    .dmatx_cb         = UART1_DMATxCallback,
    .dmarx_cb         = UART1_DMARxCallback,
    .dma_cfg          = &UART1_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART1_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* UART1 does not support RS485 */
    .rs485_cfg        = {0},
#endif /* END of RS485_SUPPORT */
};

void UART1_IRQHandler (void)
{
    UART_IRQHandler(&UART1);
}

#if RTE_UART1_DMA_ENABLE
static void UART1_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART1);
}

static void UART1_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART1);
}
#endif /* RTE_UART1_DMA_ENABLE */

static int32_t UART1_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART1));
}

static int32_t UART1_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART1));
}

static int32_t UART1_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART1));
}

static int32_t UART1_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART1));
}

static int32_t UART1_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART1));
}

static int32_t UART1_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART1));
}

static uint32_t UART1_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART1));
}

static uint32_t UART1_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART1));
}

static int32_t UART1_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART1));
}

static ARM_USART_STATUS UART1_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART1));
}

static ARM_USART_MODEM_STATUS UART1_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART1));
}

static int32_t UART1_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART1));
}

extern ARM_DRIVER_USART Driver_USART1;
ARM_DRIVER_USART Driver_USART1 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    UART1_Initialize,
    UART1_Uninitialize,
    UART1_PowerControl,
    UART1_Send,
    UART1_Receive,
    UART1_Transfer,
    UART1_GetTxCount,
    UART1_GetRxCount,
    UART1_Control,
    UART1_GetStatus,
    UART1_SetModemControl,
    UART1_GetModemStatus
};
#endif /* RTE_UART1 */


/* UART2 Driver Instance */
#if (RTE_UART2)

#if RTE_UART2_DMA_ENABLE
static void UART2_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART2_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART2_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART2_DMA),
        .dma_periph_req = UART2_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART2_DMA,
             .group    = UART2_DMA_GROUP,
             .channel  = UART2_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART2_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART2_DMA),
        .dma_periph_req = UART2_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART2_DMA,
             .group    = UART2_DMA_GROUP,
             .channel  = UART2_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART2_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART2_DMA_ENABLE */


/* UART2 Driver Resources */
static UART_RESOURCES UART2 =
{
    .regs             = (UART_Type *)UART2_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART2_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART2_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART2_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART2_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART2_IRQ_PRI,
    .instance         = UART_INSTANCE_2,

#if RTE_UART2_DMA_ENABLE
    .dma_enable       = RTE_UART2_DMA_ENABLE,
    .dma_irq_priority = RTE_UART2_DMA_IRQ_PRI,
    .dmatx_cb         = UART2_DMATxCallback,
    .dmarx_cb         = UART2_DMARxCallback,
    .dma_cfg          = &UART2_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART2_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* UART2 does not support RS485 */
    .rs485_cfg        = {0},
#endif /* END of RS485_SUPPORT */
};

void UART2_IRQHandler (void)
{
    UART_IRQHandler(&UART2);
}

#if RTE_UART2_DMA_ENABLE
static void UART2_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART2);
}

static void UART2_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART2);
}
#endif /* RTE_UART2_DMA_ENABLE */

static int32_t UART2_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART2));
}

static int32_t UART2_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART2));
}

static int32_t UART2_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART2));
}

static int32_t UART2_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART2));
}

static int32_t UART2_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART2));
}

static int32_t UART2_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART2));
}

static uint32_t UART2_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART2));
}

static uint32_t UART2_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART2));
}

static int32_t UART2_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART2));
}

static ARM_USART_STATUS UART2_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART2));
}

static ARM_USART_MODEM_STATUS UART2_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART2));
}

static int32_t UART2_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART2));
}

extern ARM_DRIVER_USART Driver_USART2;
ARM_DRIVER_USART Driver_USART2 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    UART2_Initialize,
    UART2_Uninitialize,
    UART2_PowerControl,
    UART2_Send,
    UART2_Receive,
    UART2_Transfer,
    UART2_GetTxCount,
    UART2_GetRxCount,
    UART2_Control,
    UART2_GetStatus,
    UART2_SetModemControl,
    UART2_GetModemStatus
};
#endif /* RTE_UART2 */


/* UART3 Driver Instance */
#if (RTE_UART3)

#if RTE_UART3_DMA_ENABLE
static void UART3_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART3_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART3_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART3_DMA),
        .dma_periph_req = UART3_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART3_DMA,
             .group    = UART3_DMA_GROUP,
             .channel  = UART3_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART3_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART3_DMA),
        .dma_periph_req = UART3_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART3_DMA,
             .group    = UART3_DMA_GROUP,
             .channel  = UART3_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART3_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART3_DMA_ENABLE */

/* UART3 Driver Resources */
static UART_RESOURCES UART3 =
{
    .regs             = (UART_Type *)UART3_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART3_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART3_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART3_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART3_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART3_IRQ_PRI,
    .instance         = UART_INSTANCE_3,

#if RTE_UART3_DMA_ENABLE
    .dma_enable       = RTE_UART3_DMA_ENABLE,
    .dma_irq_priority = RTE_UART3_DMA_IRQ_PRI,
    .dmatx_cb         = UART3_DMATxCallback,
    .dmarx_cb         = UART3_DMARxCallback,
    .dma_cfg          = &UART3_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART3_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* UART3 does not support RS485 */
    .rs485_cfg        = {0},
#endif /* END of RS485_SUPPORT */
};

void UART3_IRQHandler (void)
{
    UART_IRQHandler(&UART3);
}

#if RTE_UART3_DMA_ENABLE
static void UART3_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART3);
}

static void UART3_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART3);
}
#endif /* RTE_UART3_DMA_ENABLE */

static int32_t UART3_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART3));
}

static int32_t UART3_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART3));
}

static int32_t UART3_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART3));
}

static int32_t UART3_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART3));
}

static int32_t UART3_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART3));
}

static int32_t UART3_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART3));
}

static uint32_t UART3_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART3));
}

static uint32_t UART3_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART3));
}

static int32_t UART3_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART3));
}

static ARM_USART_STATUS UART3_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART3));
}

static ARM_USART_MODEM_STATUS UART3_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART3));
}

static int32_t UART3_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART3));
}

extern ARM_DRIVER_USART Driver_USART3;
ARM_DRIVER_USART Driver_USART3 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    UART3_Initialize,
    UART3_Uninitialize,
    UART3_PowerControl,
    UART3_Send,
    UART3_Receive,
    UART3_Transfer,
    UART3_GetTxCount,
    UART3_GetRxCount,
    UART3_Control,
    UART3_GetStatus,
    UART3_SetModemControl,
    UART3_GetModemStatus
};
#endif /* RTE_UART3 */


/* UART4 Driver Instance */
#if (RTE_UART4)

#if RTE_UART4_DMA_ENABLE
static void UART4_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART4_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART4_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART4_DMA),
        .dma_periph_req = UART4_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART4_DMA,
             .group    = UART4_DMA_GROUP,
             .channel  = UART4_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART4_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART4_DMA),
        .dma_periph_req = UART4_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART4_DMA,
             .group    = UART4_DMA_GROUP,
             .channel  = UART4_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART4_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART4_DMA_ENABLE */

/* UART4 Driver Resources */
static UART_RESOURCES UART4 =
{
    .regs             = (UART_Type *)UART4_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART4_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART4_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART4_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART4_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART4_IRQ_PRI,
    .instance         = UART_INSTANCE_4,

#if RTE_UART4_DMA_ENABLE
    .dma_enable       = RTE_UART4_DMA_ENABLE,
    .dma_irq_priority = RTE_UART4_DMA_IRQ_PRI,
    .dmatx_cb         = UART4_DMATxCallback,
    .dmarx_cb         = UART4_DMARxCallback,
    .dma_cfg          = &UART4_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART4_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* RS485_SUPPORT */

#if RTE_UART4_RS485_ENABLE
    .rs485_cfg.rs485_control                          = RTE_UART4_RS485_ENABLE,
    .rs485_cfg.rs485_transfer_mode                    = RTE_UART4_RS485_TRANSFER_MODE,
    .rs485_cfg.rs485_de_assertion_time_8bit           = RTE_UART4_RS485_DE_ASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_deassertion_time_8bit         = RTE_UART4_RS485_DE_DEASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_to_re_turn_around_time_16bit  = RTE_UART4_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT,
    .rs485_cfg.rs485_re_to_de_turn_around_time_16bit  = RTE_UART4_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT,
#else
    .rs485_cfg        = {0},
#endif /* END of RTE_UART4_RS485_ENABLE */

#endif /* END of RS485_SUPPORT */
};

void UART4_IRQHandler (void)
{
    UART_IRQHandler(&UART4);
}

#if RTE_UART4_DMA_ENABLE
static void UART4_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART4);
}

static void UART4_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART4);
}
#endif /* RTE_UART4_DMA_ENABLE */

static int32_t UART4_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART4));
}

static int32_t UART4_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART4));
}

static int32_t UART4_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART4));
}

static int32_t UART4_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART4));
}

static int32_t UART4_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART4));
}

static int32_t UART4_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART4));
}

static uint32_t UART4_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART4));
}

static uint32_t UART4_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART4));
}

static int32_t UART4_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART4));
}

static ARM_USART_STATUS UART4_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART4));
}

static ARM_USART_MODEM_STATUS UART4_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART4));
}

static int32_t UART4_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART4));
}

extern ARM_DRIVER_USART Driver_USART4;
ARM_DRIVER_USART Driver_USART4 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities_WO_RTS_CTS,
    UART4_Initialize,
    UART4_Uninitialize,
    UART4_PowerControl,
    UART4_Send,
    UART4_Receive,
    UART4_Transfer,
    UART4_GetTxCount,
    UART4_GetRxCount,
    UART4_Control,
    UART4_GetStatus,
    UART4_SetModemControl,
    UART4_GetModemStatus
};
#endif /* RTE_UART4 */


/* UART5 Driver Instance */
#if (RTE_UART5)

#if RTE_UART5_DMA_ENABLE
static void UART5_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART5_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART5_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART5_DMA),
        .dma_periph_req = UART5_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART5_DMA,
             .group    = UART5_DMA_GROUP,
             .channel  = UART5_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART5_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART5_DMA),
        .dma_periph_req = UART5_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART5_DMA,
             .group    = UART5_DMA_GROUP,
             .channel  = UART5_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART5_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART5_DMA_ENABLE */

/* UART5 Driver Resources */
static UART_RESOURCES UART5 =
{
    .regs             = (UART_Type *)UART5_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART5_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART5_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART5_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART5_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART5_IRQ_PRI,
    .instance         = UART_INSTANCE_5,

#if RTE_UART5_DMA_ENABLE
    .dma_enable       = RTE_UART5_DMA_ENABLE,
    .dma_irq_priority = RTE_UART5_DMA_IRQ_PRI,
    .dmatx_cb         = UART5_DMATxCallback,
    .dmarx_cb         = UART5_DMARxCallback,
    .dma_cfg          = &UART5_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART5_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* RS485_SUPPORT */

#if RTE_UART5_RS485_ENABLE
    .rs485_cfg.rs485_control                          = RTE_UART5_RS485_ENABLE,
    .rs485_cfg.rs485_transfer_mode                    = RTE_UART5_RS485_TRANSFER_MODE,
    .rs485_cfg.rs485_de_assertion_time_8bit           = RTE_UART5_RS485_DE_ASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_deassertion_time_8bit         = RTE_UART5_RS485_DE_DEASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_to_re_turn_around_time_16bit  = RTE_UART5_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT,
    .rs485_cfg.rs485_re_to_de_turn_around_time_16bit  = RTE_UART5_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT,
#else
    .rs485_cfg        = {0},
#endif /* END of RTE_UART5_RS485_ENABLE */

#endif /* END of RS485_SUPPORT */
};

void UART5_IRQHandler (void)
{
    UART_IRQHandler(&UART5);
}

#if RTE_UART5_DMA_ENABLE
static void UART5_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART5);
}

static void UART5_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART5);
}
#endif /* RTE_UART5_DMA_ENABLE */

static int32_t UART5_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART5));
}

static int32_t UART5_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART5));
}

static int32_t UART5_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART5));
}

static int32_t UART5_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART5));
}

static int32_t UART5_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART5));
}

static int32_t UART5_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART5));
}

static uint32_t UART5_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART5));
}

static uint32_t UART5_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART5));
}

static int32_t UART5_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART5));
}

static ARM_USART_STATUS UART5_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART5));
}

static ARM_USART_MODEM_STATUS UART5_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART5));
}

static int32_t UART5_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART5));
}

extern ARM_DRIVER_USART Driver_USART5;
ARM_DRIVER_USART Driver_USART5 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities_WO_RTS_CTS,
    UART5_Initialize,
    UART5_Uninitialize,
    UART5_PowerControl,
    UART5_Send,
    UART5_Receive,
    UART5_Transfer,
    UART5_GetTxCount,
    UART5_GetRxCount,
    UART5_Control,
    UART5_GetStatus,
    UART5_SetModemControl,
    UART5_GetModemStatus
};
#endif /* RTE_UART5 */


/* UART6 Driver Instance */
#if (RTE_UART6)

#if RTE_UART6_DMA_ENABLE
static void UART6_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART6_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART6_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART6_DMA),
        .dma_periph_req = UART6_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART6_DMA,
             .group    = UART6_DMA_GROUP,
             .channel  = UART6_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART6_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART6_DMA),
        .dma_periph_req = UART6_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART6_DMA,
             .group    = UART6_DMA_GROUP,
             .channel  = UART6_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART6_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART6_DMA_ENABLE */

/* UART6 Driver Resources */
static UART_RESOURCES UART6 =
{
    .regs             = (UART_Type *)UART6_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART6_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART6_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART6_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART6_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART6_IRQ_PRI,
    .instance         = UART_INSTANCE_6,

#if RTE_UART6_DMA_ENABLE
    .dma_enable       = RTE_UART6_DMA_ENABLE,
    .dma_irq_priority = RTE_UART6_DMA_IRQ_PRI,
    .dmatx_cb         = UART6_DMATxCallback,
    .dmarx_cb         = UART6_DMARxCallback,
    .dma_cfg          = &UART6_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART6_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* RS485_SUPPORT */

#if RTE_UART6_RS485_ENABLE
    .rs485_cfg.rs485_control                          = RTE_UART6_RS485_ENABLE,
    .rs485_cfg.rs485_transfer_mode                    = RTE_UART6_RS485_TRANSFER_MODE,
    .rs485_cfg.rs485_de_assertion_time_8bit           = RTE_UART6_RS485_DE_ASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_deassertion_time_8bit         = RTE_UART6_RS485_DE_DEASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_to_re_turn_around_time_16bit  = RTE_UART6_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT,
    .rs485_cfg.rs485_re_to_de_turn_around_time_16bit  = RTE_UART6_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT,
#else
    .rs485_cfg        = {0},
#endif /* END of RTE_UART6_RS485_ENABLE */

#endif /* END of RS485_SUPPORT */
};

void UART6_IRQHandler (void)
{
    UART_IRQHandler(&UART6);
}

#if RTE_UART6_DMA_ENABLE
static void UART6_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART6);
}

static void UART6_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART6);
}
#endif /* RTE_UART6_DMA_ENABLE */

static int32_t UART6_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART6));
}

static int32_t UART6_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART6));
}

static int32_t UART6_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART6));
}

static int32_t UART6_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART6));
}

static int32_t UART6_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART6));
}

static int32_t UART6_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART6));
}

static uint32_t UART6_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART6));
}

static uint32_t UART6_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART6));
}

static int32_t UART6_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART6));
}

static ARM_USART_STATUS UART6_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART6));
}

static ARM_USART_MODEM_STATUS UART6_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART6));
}

static int32_t UART6_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART6));
}

extern ARM_DRIVER_USART Driver_USART6;
ARM_DRIVER_USART Driver_USART6 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities_WO_RTS_CTS,
    UART6_Initialize,
    UART6_Uninitialize,
    UART6_PowerControl,
    UART6_Send,
    UART6_Receive,
    UART6_Transfer,
    UART6_GetTxCount,
    UART6_GetRxCount,
    UART6_Control,
    UART6_GetStatus,
    UART6_SetModemControl,
    UART6_GetModemStatus
};
#endif /* RTE_UART6 */


/* UART7 Driver Instance */
#if (RTE_UART7)

#if RTE_UART7_DMA_ENABLE
static void UART7_DMATxCallback(uint32_t event, int8_t peri_num);
static void UART7_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG UART7_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART7_DMA),
        .dma_periph_req = UART7_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART7_DMA,
             .group    = UART7_DMA_GROUP,
             .channel  = UART7_DMA_RX_PERIPH_REQ,
             .enable_handshake = UART7_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(UART7_DMA),
        .dma_periph_req = UART7_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = UART7_DMA,
             .group    = UART7_DMA_GROUP,
             .channel  = UART7_DMA_TX_PERIPH_REQ,
             .enable_handshake = UART7_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_UART7_DMA_ENABLE */

/* UART7 Driver Resources */
static UART_RESOURCES UART7 =
{
    .regs             = (UART_Type *)UART7_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = RTE_UART7_CLK_SOURCE,
    .tx_fifo_trg_lvl  = RTE_UART7_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_UART7_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) UART7_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_UART7_IRQ_PRI,
    .instance         = UART_INSTANCE_7,

#if RTE_UART7_DMA_ENABLE
    .dma_enable       = RTE_UART7_DMA_ENABLE,
    .dma_irq_priority = RTE_UART7_DMA_IRQ_PRI,
    .dmatx_cb         = UART7_DMATxCallback,
    .dmarx_cb         = UART7_DMARxCallback,
    .dma_cfg          = &UART7_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_UART7_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* RS485_SUPPORT */

#if RTE_UART7_RS485_ENABLE
    .rs485_cfg.rs485_control                          = RTE_UART7_RS485_ENABLE,
    .rs485_cfg.rs485_transfer_mode                    = RTE_UART7_RS485_TRANSFER_MODE,
    .rs485_cfg.rs485_de_assertion_time_8bit           = RTE_UART7_RS485_DE_ASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_deassertion_time_8bit         = RTE_UART7_RS485_DE_DEASSERTION_TIME_8BIT,
    .rs485_cfg.rs485_de_to_re_turn_around_time_16bit  = RTE_UART7_RS485_DE_TO_RE_TURN_AROUND_TIME_16BIT,
    .rs485_cfg.rs485_re_to_de_turn_around_time_16bit  = RTE_UART7_RS485_RE_TO_DE_TURN_AROUND_TIME_16BIT,
#else
    .rs485_cfg        = {0},
#endif /* END of RTE_UART7_RS485_ENABLE */

#endif /* END of RS485_SUPPORT */
};

void UART7_IRQHandler (void)
{
    UART_IRQHandler(&UART7);
}

#if RTE_UART7_DMA_ENABLE
static void UART7_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &UART7);
}

static void UART7_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &UART7);
}
#endif /* RTE_UART7_DMA_ENABLE */

static int32_t UART7_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event,  &UART7));
}

static int32_t UART7_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&UART7));
}

static int32_t UART7_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &UART7));
}

static int32_t UART7_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &UART7));
}

static int32_t UART7_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &UART7));
}

static int32_t UART7_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &UART7));
}

static uint32_t UART7_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&UART7));
}

static uint32_t UART7_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&UART7));
}

static int32_t UART7_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &UART7));
}

static ARM_USART_STATUS UART7_GetStatus(void)
{
    return (ARM_USART_GetStatus(&UART7));
}

static ARM_USART_MODEM_STATUS UART7_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&UART7));
}

static int32_t UART7_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &UART7));
}

extern ARM_DRIVER_USART Driver_USART7;
ARM_DRIVER_USART Driver_USART7 =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities_WO_RTS_CTS,
    UART7_Initialize,
    UART7_Uninitialize,
    UART7_PowerControl,
    UART7_Send,
    UART7_Receive,
    UART7_Transfer,
    UART7_GetTxCount,
    UART7_GetRxCount,
    UART7_Control,
    UART7_GetStatus,
    UART7_SetModemControl,
    UART7_GetModemStatus
};
#endif /* RTE_UART7 */


/* LPUART Driver Instance */
#if (RTE_LPUART)

#if RTE_LPUART_DMA_ENABLE
static void LPUART_DMATxCallback(uint32_t event, int8_t peri_num);
static void LPUART_DMARxCallback(uint32_t event, int8_t peri_num);

static UART_DMA_HW_CONFIG LPUART_DMA_HW_CONFIG =
{
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(LPUART_DMA),
        .dma_periph_req = LPUART_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = LPUART_DMA,
             .group    = LPUART_DMA_GROUP,
             .channel  = LPUART_DMA_RX_PERIPH_REQ,
             .enable_handshake = LPUART_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(LPUART_DMA),
        .dma_periph_req = LPUART_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = LPUART_DMA,
             .group    = LPUART_DMA_GROUP,
             .channel  = LPUART_DMA_TX_PERIPH_REQ,
             .enable_handshake = LPUART_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif /* RTE_LPUART_DMA_ENABLE */

/* LPUART Driver Resources */
static UART_RESOURCES LPUART =
{
    .regs             = (UART_Type *)LPUART_BASE,
    .cb_event         = NULL,
    .transfer         = {0},
    .status           = {0},
    .state            = {0},
    .baudrate         = 0,
    .clk              = 0,
    .clk_source       = 0,
    .tx_fifo_trg_lvl  = RTE_LPUART_TX_TRIG_LVL,
    .rx_fifo_trg_lvl  = RTE_LPUART_RX_TRIG_LVL,
    .irq_num          = (IRQn_Type) LPUART_IRQ_IRQn,
    .irq_priority     = (uint32_t)RTE_LPUART_IRQ_PRI,
    .instance         = UART_INSTANCE_LP,

#if RTE_LPUART_DMA_ENABLE
    .dma_enable       = RTE_LPUART_DMA_ENABLE,
    .dma_irq_priority = RTE_LPUART_DMA_IRQ_PRI,
    .dmatx_cb         = LPUART_DMATxCallback,
    .dmarx_cb         = LPUART_DMARxCallback,
    .dma_cfg          = &LPUART_DMA_HW_CONFIG,
#endif

#if UART_BLOCKING_MODE_ENABLE
    .blocking_mode    = RTE_LPUART_BLOCKING_MODE_ENABLE,
#endif

#if RS485_SUPPORT /* LPUART does not support RS485 */
    .rs485_cfg        = {0},
#endif /* END of RS485_SUPPORT */
};

void LPUART_IRQHandler (void)
{
    UART_IRQHandler(&LPUART);
}

#if RTE_LPUART_DMA_ENABLE
static void LPUART_DMATxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMATxCallback(event, peri_num, &LPUART);
}

static void LPUART_DMARxCallback(uint32_t event, int8_t peri_num)
{
    UART_DMARxCallback(event, peri_num, &LPUART);
}
#endif /* RTE_LPUART_DMA_ENABLE */

static int32_t LPUART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    return (ARM_USART_Initialize(cb_event, &LPUART));
}

static int32_t LPUART_Uninitialize(void)
{
    return (ARM_USART_Uninitialize(&LPUART));
}

static int32_t LPUART_PowerControl(ARM_POWER_STATE state)
{
    return (ARM_USART_PowerControl(state, &LPUART));
}

static int32_t LPUART_Send(void const * const p_data, uint32_t num)
{
    return (ARM_USART_Send(p_data, num , &LPUART));
}

static int32_t LPUART_Receive(void * const p_data, uint32_t num)
{
    return (ARM_USART_Receive(p_data, num, &LPUART));
}

static int32_t LPUART_Transfer(void const * const p_data_out, void * const p_data_in, uint32_t num)
{
    return (ARM_USART_Transfer(p_data_out, p_data_in, num, &LPUART));
}

static uint32_t LPUART_GetTxCount(void)
{
    return (ARM_USART_GetTxCount(&LPUART));
}

static uint32_t LPUART_GetRxCount(void)
{
    return (ARM_USART_GetRxCount(&LPUART));
}

static int32_t LPUART_Control(uint32_t control, uint32_t arg)
{
    return (ARM_USART_Control(control, arg, &LPUART));
}

static ARM_USART_STATUS LPUART_GetStatus(void)
{
    return (ARM_USART_GetStatus(&LPUART));
}

static ARM_USART_MODEM_STATUS LPUART_GetModemStatus(void)
{
    return (ARM_USART_GetModemStatus(&LPUART));
}

static int32_t LPUART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    return (ARM_USART_SetModemControl(control, &LPUART));
}

extern ARM_DRIVER_USART Driver_USARTLP;
ARM_DRIVER_USART Driver_USARTLP =
{
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    LPUART_Initialize,
    LPUART_Uninitialize,
    LPUART_PowerControl,
    LPUART_Send,
    LPUART_Receive,
    LPUART_Transfer,
    LPUART_GetTxCount,
    LPUART_GetRxCount,
    LPUART_Control,
    LPUART_GetStatus,
    LPUART_SetModemControl,
    LPUART_GetModemStatus
};
#endif /* RTE_LPUART */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
