/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/* Includes */
#include <stddef.h>
#include <stdint.h>
#include "stdio.h"

/* system includes */
#include "Driver_I2C_Private.h"

/* Driver version */
#define ARM_I2C_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 3)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_I2C_API_VERSION,
    ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities = {
    1,  /* supports 10-bit addressing */
    0   /* reserved */
};

/**
 * @brief   get i2c version
 * @note    none
 * @param   none
 * @retval  driver version
 */
static ARM_DRIVER_VERSION ARM_I2C_GetVersion(void)
{
    return DriverVersion;
}

/**
 * @brief   get i2c capabilites
 * @note    none
 * @param   none
 * @retval  driver capabilites
 */
static ARM_I2C_CAPABILITIES ARM_I2C_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
 * @brief   Set i2c Target address
 * @param   I2C  : Pointer to I2C resources structure
 * @param   addr : Target slave address
 * @retval  none
 */
static void I2C_SetTargetAddress(I2C_RESOURCES *I2C, const uint32_t addr)
{
    /* addr is different from current target address */
    if ((addr & (~ARM_I2C_ADDRESS_10BIT)) != I2C->tar_addr)
    {
        if(addr & ARM_I2C_ADDRESS_10BIT)
        {
            I2C->addr_mode = I2C_10BIT_ADDRESS;
        }
        else
        {
            I2C->addr_mode = I2C_7BIT_ADDRESS;
        }
        /* set target address */
        i2c_set_target_addr(I2C->regs, addr, I2C->addr_mode, I2C->transfer.curr_stat);

        I2C->tar_addr = (addr & (~ARM_I2C_ADDRESS_10BIT));
    }
}
/**
 * @brief   get i2c bus speed
 * @note    implemented only ARM_I2C_BUS_SPEED_STANDARD
 * @param   I2C  : Pointer to I2C resources structure
 * @param   i2c_bus_speed    : i2c bus speed
 *          ARM_I2C_BUS_SPEED_STANDARD /
 *          ARM_I2C_BUS_SPEED_FAST     /
 *          ARM_I2C_BUS_SPEED_FAST_PLUS
 * @retval  none
 */
static int32_t I2C_GetBusSpeed(I2C_RESOURCES *I2C, uint32_t i2c_bus_speed)
{
    int32_t speed;

    switch (i2c_bus_speed)
    {
        case ARM_I2C_BUS_SPEED_STANDARD:
            /* Standard Speed (100kHz) */
            speed = I2C_IC_CON_SPEED_STANDARD;
            I2C->speed_mode = I2C_SPEED_STANDARD;
            break;

        case ARM_I2C_BUS_SPEED_FAST:
            /* Fast Speed (400kHz) */
            speed = I2C_IC_CON_SPEED_FAST;
            I2C->speed_mode = I2C_SPEED_FAST;
            break;

        case ARM_I2C_BUS_SPEED_FAST_PLUS:
            /* Fast+ Speed (1MHz) */
            speed = I2C_IC_CON_SPEED_FAST;
            I2C->speed_mode = I2C_SPEED_FASTPLUS;
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return speed;
}

#if I2C_DMA_ENABLE

/**
 * @brief   Initialises I2C DMA
 * @param   dma_periph : Pointer to DMA resources
 * @retval  execution_status
 */
__STATIC_INLINE int32_t I2C_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
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
 * @brief   PowerControls I2C DMA
 * @param   state      : Power state
 * @param   dma_periph : Pointer to DMA resources
 * @retval  execution_status
 */
__STATIC_INLINE int32_t I2C_DMA_PowerControl(ARM_POWER_STATE state,
                                             DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Controls power of DMA interface */
    status = dma_drv->PowerControl(state);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
 * @brief   Allocate a channel for I2C
 * @param   dma_periph : Pointer to DMA resources
 * @retval  execution_status
 */
__STATIC_INLINE int32_t I2C_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
 * @brief   De-allocate a channel for I2C
 * @param   dma_periph : Pointer to DMA resources
 * @retval  execution_status
 */
__STATIC_INLINE int32_t I2C_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
 * @brief   Start I2C DMA transfer
 * @param   dma_periph : Pointer to DMA resources
 * @param   dma_params : Pointer to DMA parameters
 * @retval  execution_status
 */
__STATIC_INLINE int32_t I2C_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
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
 * @brief   Stop I2C DMA transfer
 * @param   dma_periph : Pointer to DMA resources
 * @retval  execution_status
 */
__STATIC_INLINE int32_t I2C_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
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
#endif /* I2C_DMA_ENABLE */

/**
 * @brief   CMSIS-Driver i2c initialize
 * @note    it will use interrupt method for data send and receive.
 * @param   cb_event    : Pointer to \ref ARM_I2C_SignalEvent
 * @param   i2c         : Pointer to i2c resources structure
 * @retval  ARM_DRIVER_OK : successfully initialized
 */
static int32_t ARM_I2C_Initialize(ARM_I2C_SignalEvent_t cb_event,
                                   I2C_RESOURCES       *I2C)
{
    /* Driver is already initialized */
    if (I2C->state.initialized == 1)
        return ARM_DRIVER_OK;

    /* default setting */
    I2C->addr_mode = I2C_7BIT_ADDRESS;
    I2C->tar_addr  = I2C_0_TARADDR;

    I2C->tar_addr &= I2C_7BIT_ADDRESS_MASK;
    I2C->slv_addr &= I2C_7BIT_ADDRESS_MASK;

    /* Disable device before init it */
    i2c_disable(I2C->regs);

    I2C->transfer.err_state = I2C_ERR_NONE;
    I2C->transfer.next_cond = I2C_MODE_STOP;

    I2C->transfer.tx_over = 0U;
    I2C->transfer.rx_over = 0;

    /* set the user callback event. */
    I2C->cb_event = cb_event;

    /* Get the SYST_PCLK */
    I2C->clk = GetSystemAPBClock();

    /* set the flag as initialized. */
    I2C->state.initialized = 1;

#if I2C_DMA_ENABLE
    if(I2C->dma_enable)
    {
        I2C->dma_cfg->dma_rx.dma_handle = -1;
        I2C->dma_cfg->dma_tx.dma_handle = -1;

        /* Initialize DMA for I2C-Tx */
        if(I2C_DMA_Initialize(&I2C->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        /* Initialize DMA for I2C-Rx */
        if(I2C_DMA_Initialize(&I2C->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}
/**
 * @brief   CMSIS-Driver i2c uninitialize
 * @note    none
 * @param   i2c    : Pointer to i2c resources structure
 * @retval  ARM_DRIVER_OK : successfully uninitialized
 */
static int32_t ARM_I2C_Uninitialize(I2C_RESOURCES *I2C)
{
    int ret = ARM_DRIVER_OK;

    /* check i2c driver is initialized or not */
    if (I2C->state.initialized == 0)
        return ARM_DRIVER_OK;

    /* check i2c driver is powered or not */
    if (I2C->state.powered == 1)
        return ARM_DRIVER_ERROR;

#if I2C_DMA_ENABLE
    if(I2C->dma_enable)
    {
        I2C->dma_cfg->dma_rx.dma_handle = -1;
        I2C->dma_cfg->dma_tx.dma_handle = -1;
    }
#endif

    i2c_disable(I2C->regs);

    /* initialize all variables to 0 */

    /* initialize the tx_buffer */
    I2C->transfer.tx_buf               = NULL;
    I2C->transfer.tx_total_num         = 0U;
    I2C->transfer.tx_curr_cnt          = 0U;
    I2C->transfer.curr_cnt             = 0U;
    I2C->transfer.tx_over              = 0U;

    /* initialize the rx_buffer */
    I2C->transfer.rx_buf               = NULL;
    I2C->transfer.rx_total_num         = 0U;
    I2C->transfer.rx_curr_cnt          = 0U;
    I2C->transfer.rx_curr_tx_index     = 0U;
    I2C->transfer.rx_over              = 0U;

    I2C->transfer.next_cond            = I2C_MODE_STOP;
    I2C->transfer.err_state            = I2C_ERR_NONE;
    I2C->transfer.curr_stat            = I2C_TRANSFER_NONE;

    /* Clear driver status \ref ARM_I2C_STATUS */
    I2C->status.busy                   = 0U;
    I2C->status.mode                   = 0U;
    I2C->status.direction              = 0U;
    I2C->status.arbitration_lost       = 0U;
    I2C->status.bus_error              = 0U;
    I2C->addr_mode                     = I2C_7BIT_ADDRESS;
    I2C->tar_addr                      = 0U;

    /* Reset the flags. */
    I2C->state.initialized = 0U;

    return ret;
}

/**
 * @func    : CMSIS Driver I2C Power Control
 * @brief   : Power the driver and enable the NVIC
 * @param   : state : Power state
 * @param   : I2C   : Pointer to i2c resources structure
 * @return  : ARM_DRIVER_OK
 */
static int32_t ARM_I2C_PowerControl(ARM_POWER_STATE state, I2C_RESOURCES *I2C)
{
    switch (state)
    {
    case ARM_POWER_FULL:
        /* check for Driver initialization */
        if (I2C->state.initialized == 0)
        {
            return ARM_DRIVER_ERROR;
        }

        /* check for the power is done before initialization or not */
        if (I2C->state.powered == 1)
        {
            return ARM_DRIVER_OK;
        }

        /* Disable all interrupts */
        i2c_master_disable_tx_interrupt(I2C->regs);
        i2c_master_disable_rx_interrupt(I2C->regs);
        i2c_slave_disable_tx_interrupt(I2C->regs);
        i2c_slave_disable_rx_interrupt(I2C->regs);

        /* Clear Any Pending Irq */
        NVIC_ClearPendingIRQ(I2C->irq_num);

        /* Set Priority */
        NVIC_SetPriority(I2C->irq_num, I2C->irq_priority);

        /* Enable IRQ */
        NVIC_EnableIRQ(I2C->irq_num);

        i2c_set_tx_threshold(I2C->regs, I2C->tx_fifo_threshold);
        i2c_set_rx_threshold(I2C->regs, I2C->rx_fifo_threshold);

#if I2C_DMA_ENABLE
        if(I2C->dma_enable)
        {
            /* Power On DMA for I2C-Tx */
            if(I2C_DMA_PowerControl(state, &I2C->dma_cfg->dma_tx) != ARM_DRIVER_OK)
                return ARM_DRIVER_ERROR;

            /* Power On DMA for I2C-Rx */
            if(I2C_DMA_PowerControl(state, &I2C->dma_cfg->dma_rx) != ARM_DRIVER_OK)
                return ARM_DRIVER_ERROR;

            /* Allocate DMA Tx channel */
            if(I2C_DMA_Allocate(&I2C->dma_cfg->dma_tx) == ARM_DRIVER_ERROR)
                return ARM_DRIVER_ERROR;

            /* Allocate DMA Rx channel */
            if(I2C_DMA_Allocate(&I2C->dma_cfg->dma_rx) == ARM_DRIVER_ERROR)
                return ARM_DRIVER_ERROR;
        }
#endif
        I2C->state.powered = 1;
        break;
    case ARM_POWER_OFF:
        if (I2C->state.powered == 0)
        {
            return ARM_DRIVER_OK;
        }
#if I2C_DMA_ENABLE
        if(I2C->dma_enable)
        {
            i2c_disable_tx_dma(I2C->regs);
            i2c_disable_rx_dma(I2C->regs);

            /* Deallocate DMA Tx channel */
            if(I2C_DMA_DeAllocate(&I2C->dma_cfg->dma_tx) == ARM_DRIVER_ERROR)
                return ARM_DRIVER_ERROR;

            /* Deallocate DMA Rx channel */
            if(I2C_DMA_DeAllocate(&I2C->dma_cfg->dma_rx) == ARM_DRIVER_ERROR)
                return ARM_DRIVER_ERROR;

            /* Power Off DMA for I2C-Tx */
            if(I2C_DMA_PowerControl(state, &I2C->dma_cfg->dma_tx) != ARM_DRIVER_OK)
                return ARM_DRIVER_ERROR;

            /* Power Off DMA for I2C-Rx */
            if(I2C_DMA_PowerControl(state, &I2C->dma_cfg->dma_rx) != ARM_DRIVER_OK)
                return ARM_DRIVER_ERROR;
        }
#endif
        /* Disabling interrupts */
        if (I2C->mode == I2C_MASTER_MODE)
        {
            i2c_master_disable_tx_interrupt(I2C->regs);
            i2c_master_disable_rx_interrupt(I2C->regs);
        }
        else
        {
            i2c_slave_disable_tx_interrupt(I2C->regs);
            i2c_slave_disable_rx_interrupt(I2C->regs);
        }

        /* Disable the IRQ */
        NVIC_DisableIRQ(I2C->irq_num);

        /* Clearing pending */
        NVIC_ClearPendingIRQ(I2C->irq_num);

        I2C->state.powered = 0;
        break;
    case ARM_POWER_LOW:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}


/**
 * @brief   CMSIS-Driver i2c master transmit
 *          Start sending data to i2c transmitter.
 * @note    I2C_FLAG_MASTER_SETUP flag should be enabled first /ref ARM_I2C_BUS_SPEED
 * @param   data         : Pointer to buffer with data to send to i2c transmitter
 * @param   num          : Number of data items to send
 * @param   I2C          : Pointer to i2c resources structure
 * @param   xfer_pending : Transfer operation is pending - Stop condition will not be
 *                         generated
 * @retval  ARM_DRIVER_ERROR_PARAMETER  : error in parameter
 * @retval  ARM_DRIVER_ERROR            : error in driver
 * @retval  ARM_DRIVER_OK               : success in interrupt case
 * @retval  ARM_DRIVER_ERROR_BUSY       : driver busy in interrupt case
 */
static int32_t ARM_I2C_MasterTransmit(I2C_RESOURCES *I2C,
                                      uint32_t        addr,
                                      const uint8_t   *data,
                                      uint32_t        num,
                                      bool            xfer_pending)
{
#if I2C_DMA_ENABLE
    ARM_DMA_PARAMS dma_params;
#endif

    /* check i2c driver is initialized or not */
    if (I2C->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    /* check i2c driver is powered or not */
    if (I2C->state.powered == 0)
        return ARM_DRIVER_ERROR;

    /* Error when RESTART is requested */
    if (xfer_pending)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    /* addr 7bit addr: 0x7F , 10bit addr: 0x3FF */
    if ((data == NULL) || (num == 0U) ||
        ((addr & (~ARM_I2C_ADDRESS_10BIT)) > 0x3FF))
    {
        /* Invalid parameters */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (I2C->state.master_setup == 0U)
    {
        /* error master mode is not configured (mode not selected)
         * master_setup  should be enabled first \ref ARM_I2C_BUS_SPEED
         */
        return ARM_DRIVER_ERROR;
    }

    if (I2C->status.busy)
    {
        /* Transfer operation in progress */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Update driver status \ref ARM_I2C_STATUS */
    I2C->status.busy             = 1;
    I2C->status.mode             = I2C_MASTER_MODE;
    I2C->status.direction        = I2C_DIR_TRANSMITTER;
    I2C->status.arbitration_lost = 0;
    I2C->status.bus_error        = 0;

    /* fill the I2C transfer structure as per user detail */
    I2C->transfer.tx_curr_cnt    = 0U;
    I2C->transfer.curr_cnt       = 0U;
    I2C->transfer.tx_over        = 0U;
    I2C->transfer.curr_stat      = I2C_TRANSFER_MST_TX;

    I2C_SetTargetAddress(I2C, addr);

#if I2C_DMA_ENABLE
    if(I2C->dma_enable)
    {
        /* Nullify Tx buf pointer, total num bytes and pending
         * fields of I2C transfer structure */
        I2C->transfer.tx_buf        = (uint8_t *)NULL;
        I2C->transfer.tx_total_num  = 0U;

        /* Start the DMA engine for sending the data to I2C */
        dma_params.peri_reqno    = (int8_t)I2C->dma_cfg->dma_tx.dma_periph_req;
        dma_params.dir           = ARM_DMA_MEM_TO_DEV;
        dma_params.cb_event      = I2C->dma_cb;
        dma_params.src_addr      = (const void*)data;
        dma_params.dst_addr      = i2c_get_data_addr(I2C->regs);
        dma_params.num_bytes     = (num * 2U);
        dma_params.irq_priority  = I2C->dma_irq_priority;

        /* i2c TX/RX FIFO is 2-byte aligned */
        dma_params.burst_size = BS_BYTE_2;

        /* Max DMA burst length can be 16*/
        if((32U - I2C->tx_fifo_threshold) > 16U)
        {
            dma_params.burst_len = 16U;
        }
        else
        {
            dma_params.burst_len = (32U - I2C->tx_fifo_threshold);
        }

        /* Prepare the I2C controller for DMA transmission */
        i2c_enable_tx_dma(I2C->regs);

        i2c_set_dma_tx_level(I2C->regs, dma_params.burst_len);

        /* Start DMA transfer */
        if(I2C_DMA_Start(&I2C->dma_cfg->dma_tx, &dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        i2c_enable_dma_master_tx(I2C->regs);

    }
    else
#endif
    {
        /* Update Tx buf pointer, total num bytes and pending
         * fields of I2C transfer structure as per user detail */
        I2C->transfer.tx_buf        = (const uint8_t *)data;
        I2C->transfer.tx_total_num  = num;

        /* Enabling the tx interrupt */
        i2c_master_enable_tx_interrupt(I2C->regs);
    }

    return ARM_DRIVER_OK;
}

/**
 * @brief   CMSIS-Driver i2c master receive
 *          Start receiving data from i2c receiver.
 * @note    none
 * @param   data         : Pointer to buffer for data to receive from i2c receiver
 * @param   addr         : Target slave address
 * @param   num          : Number of data items to receive
 * @param   I2C          : Pointer to i2c resources structure
 * @param   xfer_pending : Transfer operation is pending - Stop condition will not be
 *                         generated
 * @retval  ARM_DRIVER_ERROR_PARAMETER  : error in parameter
 * @retval  ARM_DRIVER_ERROR            : error in driver
 * @retval  ARM_DRIVER_OK               : success in interrupt case
 * @retval  ARM_DRIVER_ERROR_BUSY       : driver busy in interrupt case
 */
static int32_t ARM_I2C_MasterReceive(I2C_RESOURCES *I2C,
                                     uint32_t        addr,
                                     uint8_t         *data,
                                     uint32_t        num,
                                     bool            xfer_pending)
{
#if I2C_DMA_ENABLE
    ARM_DMA_PARAMS dma_params;
#endif

    /* check i2c driver is initialized or not */
    if (I2C->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    /* check i2c driver is powered or not */
    if (I2C->state.powered == 0)
        return ARM_DRIVER_ERROR;

    /* Error when RESTART is requested */
    if (xfer_pending)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    /* addr 7bit addr: 0x7F , 10bit addr: 0x3FF */
    if ((data == NULL) || (num == 0U) || ((addr & (~ARM_I2C_ADDRESS_10BIT)) > 0x3FF))
    {
        /* Invalid parameters */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (I2C->state.master_setup == 0U)
    {
        /* error master mode is not configured (mode not selected)
         * master_setup  should be enabled first \ref ARM_I2C_BUS_SPEED
         */
        return ARM_DRIVER_ERROR;
    }

    if (I2C->status.busy)
    {
        /* Transfer operation in progress */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Update driver status \ref ARM_I2C_STATUS */
    I2C->status.busy              = 1;
    I2C->status.mode              = I2C_MASTER_MODE;
    I2C->status.direction         = I2C_DIR_RECEIVER;
    I2C->status.arbitration_lost  = 0;
    I2C->status.bus_error         = 0;

    /* fill the i2c transfer structure as per user detail */
    I2C->transfer.rx_buf           = (uint8_t *)data;
    I2C->transfer.rx_total_num     = num;
    I2C->transfer.rx_curr_cnt      = 0U;
    I2C->transfer.curr_cnt         = 0U;
    I2C->transfer.rx_curr_tx_index = 0U;
    I2C->transfer.rx_over          = 0U;
    I2C->transfer.curr_stat        = I2C_TRANSFER_MST_RX;

    if(I2C->wr_mode_info & I2C_WRITE_READ_MODE_EN)
    {
        /* fill the i2c transfer structure required for Write-Read xfer */
        I2C->transfer.tx_buf           = (uint8_t *)data;
        I2C->transfer.tx_total_num     = I2C_WRITE_READ_TAR_REG_ADDR_SIZE
                                         (I2C->wr_mode_info);
        I2C->transfer.tx_curr_cnt      = 0U;
        I2C->transfer.wr_mode          = true;
        I2C_SetTargetAddress(I2C, addr);
        i2c_master_enable_rx_interrupt(I2C->regs);
    }
    else
    {
        I2C_SetTargetAddress(I2C, addr);

#if I2C_DMA_ENABLE
        /* Check if DMA is enabled for this */
        if (I2C->dma_enable)
        {
            /* Start the DMA engine for sending the data to I2C */
            dma_params.peri_reqno    = (int8_t)I2C->dma_cfg->dma_rx.dma_periph_req;
            dma_params.dir           = ARM_DMA_DEV_TO_MEM;
            dma_params.cb_event      = I2C->dma_cb;
            dma_params.src_addr      = i2c_get_data_addr(I2C->regs);
            dma_params.dst_addr      = (void*)data;
            dma_params.num_bytes     = (num * 2U);
            dma_params.irq_priority  = I2C->dma_irq_priority;

            /* i2c TX/RX FIFO is 2-byte aligned */
            dma_params.burst_size = BS_BYTE_2;

            /* Maximum DMA Rx burst length can be 1*/
            dma_params.burst_len = 1U;

            i2c_enable_rx_dma(I2C->regs);
            i2c_set_dma_rx_level(I2C->regs, (dma_params.burst_len - 1U));

            /* Start DMA transfer */
            if(I2C_DMA_Start(&I2C->dma_cfg->dma_rx, &dma_params) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            i2c_enable_dma_master_rx(I2C->regs);
        }
        else
#endif
        {
            /* Enabling the rx interrupt */
            i2c_master_enable_rx_interrupt(I2C->regs);
        }
    }
    return ARM_DRIVER_OK;
}

/**
 * @brief   CMSIS-Driver i2c slave transmit
 *          Start sending data to i2c master.
 * @note    master_setup bit should be enabled first /ref ARM_I2C_BUS_SPEED
 * @param   addr : Target slave address
 * @param   data : Pointer to buffer with data to send to i2c master
 * @param   num  : Number of data items to send
 * @param   I2C  : Pointer to i2c resources structure
 * @retval  ARM_DRIVER_ERROR_PARAMETER  : error in parameter
 * @retval  ARM_DRIVER_ERROR            : error in driver
 * @retval  ARM_DRIVER_OK               : success in interrupt case
 * @retval  ARM_DRIVER_ERROR_BUSY       : driver busy in interrupt case
 * @retval  transmit count              : For data transmit count /ref ARM_I2C_GetDataCount
 */
static int32_t ARM_I2C_SlaveTransmit(I2C_RESOURCES *I2C,
                                     const uint8_t *data,
                                           uint32_t num)
{
#if I2C_DMA_ENABLE
    ARM_DMA_PARAMS dma_params;
#endif

    /* check i2c driver is initialized or not */
    if (I2C->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    /* check i2c driver is powered or not */
    if (I2C->state.powered == 0)
        return ARM_DRIVER_ERROR;

    if((data == NULL) || (num == 0U))
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Check Slave mode is enabled */
    if (I2C->state.slave_setup == 0U)
    {
        /* error master mode is not configured (mode not selected)
         * master_setup  should be enabled first \ref ARM_I2C_BUS_SPEED
         */
        return ARM_DRIVER_ERROR;
    }

    if (I2C->status.busy)
    {
        /* Transfer operation in progress */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Update driver status \ref ARM_I2C_STATUS */
    I2C->status.busy             = 1;
    I2C->status.mode             = I2C_SLAVE_MODE;
    I2C->status.direction        = I2C_DIR_TRANSMITTER;
    I2C->status.arbitration_lost = 0;
    I2C->status.bus_error        = 0;

    /* fill the i2c transfer structure as per user detail */
    I2C->transfer.tx_curr_cnt    = 0U;
    I2C->transfer.curr_cnt       = 0U;
    I2C->transfer.tx_over        = 0U;
    I2C->transfer.curr_stat      = I2C_TRANSFER_SLV_TX;

#if I2C_DMA_ENABLE
    if(I2C->dma_enable)
    {
        /* Nullify Tx buf pointer and total num bytes
         * of I2C transfer structure */
        I2C->transfer.tx_buf        = (uint8_t *)NULL;
        I2C->transfer.tx_total_num  = 0U;

        /* Start the DMA engine for sending the data to I2C */
        dma_params.peri_reqno    = (int8_t)I2C->dma_cfg->dma_tx.dma_periph_req;
        dma_params.dir           = ARM_DMA_MEM_TO_DEV;
        dma_params.cb_event      = I2C->dma_cb;
        dma_params.src_addr      = (const void*)data;
        dma_params.dst_addr      = i2c_get_data_addr(I2C->regs);
        dma_params.num_bytes     = (num * 2U);
        dma_params.irq_priority  = I2C->dma_irq_priority;

        /* i2c TX/RX FIFO is 2-byte aligned */
        dma_params.burst_size = BS_BYTE_2;

        /* Max DMA burst length can be 16*/
        if((32U - I2C->tx_fifo_threshold) > 16U)
        {
            dma_params.burst_len = 16U;
        }
        else
        {
            dma_params.burst_len = (32U - I2C->tx_fifo_threshold);
        }

        /* Prepare the I2C controller for DMA transmission */
        i2c_enable_tx_dma(I2C->regs);

        i2c_set_dma_tx_level(I2C->regs, dma_params.burst_len);

        /* Start DMA transfer */
        if(I2C_DMA_Start(&I2C->dma_cfg->dma_tx, &dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        i2c_enable_dma_slave_tx(I2C->regs);
    }
    else
#endif
    {
        /* Update Tx buf pointer and total num bytes
         * of I2C transfer structure as per user detail */
        I2C->transfer.tx_buf        = (const uint8_t *)data;
        I2C->transfer.tx_total_num  = num;

        /* Enabling the tx interrupt */
        i2c_slave_enable_tx_interrupt(I2C->regs);
    }

    return ARM_DRIVER_OK;
}

/**
 * @brief   CMSIS-Driver i2c slave receive
 *          Start receiving data from i2c master.
 * @note    none
 * @param   data : Pointer to buffer for data to receive from i2c master
 * @param   num  : Number of data items to receive
 * @param   I2C  : Pointer to i2c resources structure
 * @retval  ARM_DRIVER_ERROR_PARAMETER  : error in parameter
 * @retval  ARM_DRIVER_ERROR            : error in driver
 * @retval  ARM_DRIVER_OK               : success in interrupt case
 * @retval  ARM_DRIVER_ERROR_BUSY       : driver busy in interrupt case
 * @retval  received count              : For data receive count /ref ARM_I2C_GetDataCount
 */
static int32_t ARM_I2C_SlaveReceive(I2C_RESOURCES *I2C,
                                    uint8_t         *data,
                                    uint32_t         num)
{
#if I2C_DMA_ENABLE
    ARM_DMA_PARAMS dma_params;
#endif

    /* check i2c driver is initialized or not */
    if (I2C->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    /* check i2c driver is powered or not */
    if (I2C->state.powered == 0)
        return ARM_DRIVER_ERROR;

    if((data == NULL) || (num == 0U))
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Check Slave mode is enabled */
    if (I2C->state.slave_setup == 0U)
    {
        /* error master mode is not configured (mode not selected)
         * master_setup  should be enabled first \ref ARM_I2C_BUS_SPEED
         */
        return ARM_DRIVER_ERROR;
    }

    if (I2C->status.busy)
    {
        /* Transfer operation in progress */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Update driver status \ref ARM_I2C_STATUS */
    I2C->status.busy             = 1;
    I2C->status.mode             = I2C_SLAVE_MODE;
    I2C->status.direction        = I2C_DIR_RECEIVER;
    I2C->status.bus_error        = 0;

    /* fill the i2c transfer structure as per user detail */
    I2C->transfer.rx_curr_cnt    = 0U;
    I2C->transfer.curr_cnt       = 0U;
    I2C->transfer.curr_stat      = I2C_TRANSFER_SLV_RX;
    I2C->transfer.rx_over = 0U;

#if I2C_DMA_ENABLE
    if(I2C->dma_enable)
    {
        /* Nullify Rx buf pointer and total num bytes
         * of I2C transfer structure */
        I2C->transfer.rx_buf        = (uint8_t *)NULL;
        I2C->transfer.rx_total_num  = 0U;

        /* Start the DMA engine for sending the data to I2C */
        dma_params.peri_reqno    = (int8_t)I2C->dma_cfg->dma_rx.dma_periph_req;
        dma_params.dir           = ARM_DMA_DEV_TO_MEM;
        dma_params.cb_event      = I2C->dma_cb;
        dma_params.src_addr      = i2c_get_data_addr(I2C->regs);
        dma_params.dst_addr      = (void*)data;
        dma_params.num_bytes     = (num * 2);
        dma_params.irq_priority  = I2C->dma_irq_priority;

        /* i2c TX/RX FIFO is 2-byte aligned */
        dma_params.burst_size = BS_BYTE_2;

        /* Maximum DMA Rx burst length can be 1*/
        dma_params.burst_len = 1U;

        i2c_enable_rx_dma(I2C->regs);
        i2c_set_dma_rx_level(I2C->regs, (dma_params.burst_len - 1U));

        /* Start DMA transfer */
        if(I2C_DMA_Start(&I2C->dma_cfg->dma_rx, &dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        i2c_enable_dma_slave_rx(I2C->regs);
    }
    else
#endif
    {
        /* Update Rx buf pointer and total num bytes
         * of I2C transfer structure as per user detail */
        I2C->transfer.rx_buf        = (uint8_t *)data;
        I2C->transfer.rx_total_num  = num;

        /* Enabling the rx interrupt */
        i2c_slave_enable_rx_interrupt(I2C->regs);
    }
    return ARM_DRIVER_OK;
}

/**
 * @brief   CMSIS-Driver i2c get transfer data count
 * @note    it can be either transmit or receive data count which perform last
 *          (useful only in interrupt mode)
 * @param   I2C   : Pointer to i2c resources structure
 * @retval  transfer data count
 */
static int32_t ARM_I2C_GetDataCount(const I2C_RESOURCES *I2C)
{
    /* return common count for both tx/rx */
    return ((int32_t)I2C->transfer.curr_cnt);
}

/**
 * @brief   CMSIS-Driver i2c control
 *          Control i2c Interface.
 * @note    none
 * @param   control : Operation
 * @param   arg     : Argument of operation (optional)
 * @param   I2C     : Pointer to i2c resources structure
 * @retval  common \ref execution_status and driver specific \ref i2c_execution_status
 */
static int32_t ARM_I2C_Control(I2C_RESOURCES  *I2C,
                               uint32_t         control,
                               uint32_t         arg)
{
    int32_t speed;

    /* check i2c driver is initialized or not */
    if (I2C->state.initialized == 0)
        return ARM_DRIVER_ERROR;

    /* check i2c driver is powered or not */
    if (I2C->state.powered == 0)
        return ARM_DRIVER_ERROR;

    switch (control)
    {
    case ARM_I2C_OWN_ADDRESS:

         speed = I2C_GetBusSpeed(I2C, ARM_I2C_BUS_SPEED_STANDARD);

         if(arg & ARM_I2C_ADDRESS_10BIT)
         {
             /* Sets 10 bit addr mode */
             I2C->addr_mode = I2C_10BIT_ADDRESS;
         }
         else
         {
             /* Sets 7 bit addr mode */
             I2C->addr_mode = I2C_7BIT_ADDRESS;
         }

         i2c_slave_init(I2C->regs, arg, I2C->addr_mode);

         /* Sets bus speed*/
         i2c_set_bus_speed(I2C->regs, speed);

         I2C->mode = I2C_SLAVE_MODE;
         /* setup slave flag */
         I2C->state.slave_setup = 1;

        break;

    case ARM_I2C_BUS_SPEED:

         speed = I2C_GetBusSpeed(I2C, arg);
         /* arg is i2c bus speed */
         i2c_master_init(I2C->regs, I2C->tar_addr);

         /* Sets master clock settings*/
         i2c_master_set_clock(I2C->regs, (I2C->clk / 1000), I2C->speed_mode);

         /* Sets bus speed*/
         i2c_set_bus_speed(I2C->regs, speed);

         I2C->mode = I2C_MASTER_MODE;
         /* setup master flag */
         I2C->state.master_setup = 1;

        break;

    case ARM_I2C_BUS_CLEAR:
        /* disable device, clear all the interrupt, enable device. */
        i2c_disable(I2C->regs);
        i2c_clear_all_interrupt(I2C->regs);

        I2C->transfer.next_cond  = I2C_MODE_STOP;
        I2C->transfer.curr_stat  = I2C_TRANSFER_NONE;
        I2C->transfer.err_state  = I2C_ERR_NONE;
        I2C->transfer.tx_over = 0U;
        I2C->transfer.rx_over = 0U;

        i2c_enable(I2C->regs);

        break;

    case ARM_I2C_ABORT_TRANSFER:

#if I2C_DMA_ENABLE
        if (I2C->dma_enable)
        {
            /* Transmit mode */
            if (I2C->status.direction == I2C_DIR_TRANSMITTER)
            {
                if (I2C_DMA_Stop(&I2C->dma_cfg->dma_tx) != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }
            }
            /* Reception mode */
            else if(I2C->status.direction == I2C_DIR_RECEIVER)
            {
                if (I2C_DMA_Stop(&I2C->dma_cfg->dma_rx) != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }
            }

            i2c_disable_tx_dma(I2C->regs);
            i2c_disable_rx_dma(I2C->regs);
        }
#endif
        /* only useful in interrupt method,
         * no effect on polling method.
         */

        /* i2c is half-duplex, at a time it can be either tx or rx
         * not sure which one is running, so clearing both. */

        /* if tx interrupt flag is enable then only disable transmit interrupt */
        if (I2C->mode == I2C_MASTER_MODE)
        {
            i2c_master_disable_tx_interrupt(I2C->regs);
        }
        else
        {
            i2c_slave_disable_tx_interrupt(I2C->regs);
        }

        if (I2C->mode == I2C_MASTER_MODE)
        {
            i2c_master_disable_rx_interrupt(I2C->regs);
        }
        else
        {
            i2c_slave_disable_rx_interrupt(I2C->regs);
        }

        /* Reset the tx_buffer */
        I2C->transfer.tx_total_num = 0U;
        I2C->transfer.rx_total_num = 0U;
        I2C->transfer.curr_stat = I2C_TRANSFER_NONE;

        /* clear busy status bit. */
        I2C->status.busy = 0U;

        break;

    case ARM_I2C_MODE_WRITE_READ:
        /* Write-Read combined mode selection */
        if(arg & ARM_I2C_WRITE_READ_MODE_EN)
        {
            I2C->wr_mode_info  = I2C_WRITE_READ_MODE_EN;
            I2C->wr_mode_info |= I2C_WRITE_READ_TAR_REG_ADDR_SIZE
                                 (ARM_I2C_TAR_REG_ADDR_SIZE(arg));
        }
        else
        {
            I2C->wr_mode_info &= ~I2C_WRITE_READ_MODE_EN;
        }
        break;
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
 * @brief   CMSIS-Driver i2c get status
 * @note    none
 * @param   I2C : Pointer to i2c resources structure
 * @retval  ARM_i2c_STATUS
 */
static ARM_I2C_STATUS ARM_I2C_GetStatus(const I2C_RESOURCES  *I2C)
{
    return I2C->status;
}

#if I2C_DMA_ENABLE

/**
 * @brief   Callback function from DMA for I2C
 * @param   event    : Event from DMA
 * @param   peri_num : Peripheral number
 * @param   I2C      : Pointer to I2C resources
 * @retval  execution_status
 */
static void I2C_DMACallback(uint32_t event, int8_t peri_num,
                            I2C_RESOURCES *I2C)
{
    (void)peri_num;

    if(!I2C->cb_event)
        return;

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {
        I2C->cb_event(ARM_I2C_EVENT_TRANSFER_INCOMPLETE);
    }
}
#endif

/* I2C0 Driver Instance */
#if (RTE_I2C0)

#if RTE_I2C0_DMA_ENABLE
static void I2C0_DMACallback(uint32_t event, int8_t peri_num);

static I2C_DMA_HW_CONFIG I2C0_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C0_DMA),
        .dma_periph_req = I2C0_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C0_DMA,
             .group    = I2C0_DMA_GROUP,
             .channel  = I2C0_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2C0_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C0_DMA),
        .dma_periph_req = I2C0_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C0_DMA,
             .group    = I2C0_DMA_GROUP,
             .channel  = I2C0_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2C0_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

/* I2C0 Driver Resources */
static I2C_RESOURCES I2C0_RES =
{
    .regs                  = (I2C_Type *)I2C0_BASE,
    .irq_num               = (IRQn_Type)I2C0_IRQ_IRQn,
    .irq_priority          = (uint32_t)RTE_I2C0_IRQ_PRIORITY,
#if RTE_I2C0_DMA_ENABLE
    .dma_enable          = RTE_I2C0_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2C0_DMA_IRQ_PRI,
    .dma_cb              = I2C0_DMACallback,
    .dma_cfg             = &I2C0_DMA_HW_CONFIG,
#endif
    .tx_fifo_threshold     = RTE_I2C0_TX_FIFO_THRESHOLD,
    .rx_fifo_threshold     = RTE_I2C0_RX_FIFO_THRESHOLD
};

static int32_t I2C0_Initialize(ARM_I2C_SignalEvent_t cb_event)
{
    return ARM_I2C_Initialize(cb_event, &I2C0_RES);
}

static int32_t I2C0_Uninitialize(void)
{
    return ARM_I2C_Uninitialize(&I2C0_RES);
}

static int32_t I2C0_PowerControl(ARM_POWER_STATE state)
{
    return ARM_I2C_PowerControl(state, &I2C0_RES);
}

static int32_t I2C0_MasterTransmit(uint32_t addr, const uint8_t *data,
                                   uint32_t num,  bool xfer_pending)
{
    return ARM_I2C_MasterTransmit(&I2C0_RES, addr, data, num, xfer_pending);
}

static int32_t I2C0_MasterReceive(uint32_t addr, uint8_t *data,
                                  uint32_t num, bool xfer_pending)
{
    return (ARM_I2C_MasterReceive(&I2C0_RES, addr, data, num, xfer_pending));
}

static int32_t I2C0_SlaveTransmit(const uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveTransmit(&I2C0_RES, data, num));
}

static int32_t I2C0_SlaveReceive(uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveReceive(&I2C0_RES, data, num));
}

static int32_t I2C0_GetDataCount(void)
{
    return (ARM_I2C_GetDataCount(&I2C0_RES));
}

static int32_t I2C0_Control(uint32_t control, uint32_t arg)
{
    return (ARM_I2C_Control(&I2C0_RES, control, arg));
}

static ARM_I2C_STATUS I2C0_GetStatus(void)
{
    return (ARM_I2C_GetStatus(&I2C0_RES));
}

void I2C0_IRQHandler(void)
{
    i2c_transfer_info_t *transfer = &(I2C0_RES.transfer);
    ARM_I2C_STATUS *i2c_stat = &(I2C0_RES.status);

    /* Check for master mode */
    if (I2C0_RES.mode == I2C_MASTER_MODE)
    {
        if (transfer->curr_stat == I2C_TRANSFER_MST_TX)
        {
            /* Master transmit*/
            i2c_master_tx_isr(I2C0_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_MST_RX)
        {
            /* Master receive */
            i2c_master_rx_isr(I2C0_RES.regs, transfer);
        }
    }
    else /* Slave mode */
    {
        if(transfer->curr_stat == I2C_TRANSFER_SLV_TX)
        {
            /* slave transmit*/
            i2c_slave_tx_isr(I2C0_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_SLV_RX)
        {
            /* slave receive */
            i2c_slave_rx_isr(I2C0_RES.regs, transfer);
        }
    }/* Slave mode */

    /* Check the ISR response */
    if (transfer->status & I2C_TRANSFER_STATUS_DONE)
    {
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;

        I2C0_RES.cb_event(ARM_I2C_EVENT_TRANSFER_DONE);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_GENERAL_CALL)
    {
        I2C0_RES.cb_event(ARM_I2C_EVENT_GENERAL_CALL);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_BUS_CLEAR)
    {
        I2C0_RES.cb_event(ARM_I2C_EVENT_BUS_CLEAR);
    }
    else
    {
#if RTE_I2C0_DMA_ENABLE
        if(I2C0_RES.status.direction == I2C_DIR_TRANSMITTER)
        {
          I2C_DMA_Stop(&I2C0_RES.dma_cfg->dma_tx);
        }
        else
        {
          I2C_DMA_Stop(&I2C0_RES.dma_cfg->dma_rx);
        }
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;
#endif

        if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_TRANSMIT)
        {
            I2C0_RES.cb_event(ARM_I2C_EVENT_SLAVE_TRANSMIT);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_RECEIVE)
        {
            I2C0_RES.cb_event(ARM_I2C_EVENT_SLAVE_RECEIVE);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ADDRESS_NACK)
        {
            I2C0_RES.cb_event(ARM_I2C_EVENT_ADDRESS_NACK);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ARBITRATION_LOST)
        {
            i2c_stat->arbitration_lost = 1U;

            I2C0_RES.cb_event(ARM_I2C_EVENT_ARBITRATION_LOST);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_BUS_ERROR)
        {
            i2c_stat->bus_error = 1U;

            I2C0_RES.cb_event(ARM_I2C_EVENT_BUS_ERROR);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_INCOMPLETE)
        {
            I2C0_RES.cb_event(ARM_I2C_EVENT_TRANSFER_INCOMPLETE);
            i2c_stat->busy = 0U;
        }
    }
    transfer->status = I2C_TRANSFER_STATUS_NONE;
}

#if RTE_I2C0_DMA_ENABLE
static void I2C0_DMACallback(uint32_t event, int8_t peri_num)
{
    I2C_DMACallback(event, peri_num, &I2C0_RES);
}
#endif

/* I2C0 Driver Control Block */
extern ARM_DRIVER_I2C Driver_I2C0;
ARM_DRIVER_I2C Driver_I2C0 = {
    ARM_I2C_GetVersion,
    ARM_I2C_GetCapabilities,
    I2C0_Initialize,
    I2C0_Uninitialize,
    I2C0_PowerControl,
    I2C0_MasterTransmit,
    I2C0_MasterReceive,
    I2C0_SlaveTransmit,
    I2C0_SlaveReceive,
    I2C0_GetDataCount,
    I2C0_Control,
    I2C0_GetStatus
};
#endif /* RTE_I2C0 */

/* I2C1 Driver Instance */
#if (RTE_I2C1)

#if RTE_I2C1_DMA_ENABLE
static void I2C1_DMACallback(uint32_t event, int8_t peri_num);

static I2C_DMA_HW_CONFIG I2C1_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C1_DMA),
        .dma_periph_req = I2C1_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C1_DMA,
             .group    = I2C1_DMA_GROUP,
             .channel  = I2C1_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2C1_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C1_DMA),
        .dma_periph_req = I2C1_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C1_DMA,
             .group    = I2C1_DMA_GROUP,
             .channel  = I2C1_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2C1_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

/* I2C1 Driver Resources */
static I2C_RESOURCES I2C1_RES =
{
    .regs                  = (I2C_Type *)I2C1_BASE,
    .irq_num               = (IRQn_Type)I2C1_IRQ_IRQn,
    .irq_priority          = (uint32_t)RTE_I2C1_IRQ_PRIORITY,
#if RTE_I2C1_DMA_ENABLE
    .dma_enable          = RTE_I2C1_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2C1_DMA_IRQ_PRI,
    .dma_cb              = I2C1_DMACallback,
    .dma_cfg             = &I2C1_DMA_HW_CONFIG,
#endif
    .tx_fifo_threshold     = RTE_I2C1_TX_FIFO_THRESHOLD,
    .rx_fifo_threshold     = RTE_I2C1_RX_FIFO_THRESHOLD
};

static int32_t I2C1_Initialize(ARM_I2C_SignalEvent_t cb_event)
{
    return ARM_I2C_Initialize(cb_event, &I2C1_RES);
}

static int32_t I2C1_Uninitialize(void)
{
    return ARM_I2C_Uninitialize(&I2C1_RES);
}

static int32_t I2C1_PowerControl(ARM_POWER_STATE state)
{
    return ARM_I2C_PowerControl(state, &I2C1_RES);
}

static int32_t I2C1_MasterTransmit(uint32_t addr, const uint8_t *data,
                                   uint32_t num, bool xfer_pending)
{
    return ARM_I2C_MasterTransmit(&I2C1_RES, addr, data, num, xfer_pending);
}

static int32_t I2C1_MasterReceive(uint32_t addr, uint8_t *data,
                                  uint32_t num, bool xfer_pending)
{
    return (ARM_I2C_MasterReceive(&I2C1_RES, addr, data, num, xfer_pending));
}

static int32_t I2C1_SlaveTransmit(const uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveTransmit(&I2C1_RES, data, num));
}

static int32_t I2C1_SlaveReceive(uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveReceive(&I2C1_RES, data, num));
}

static int32_t I2C1_GetDataCount(void)
{
    return (ARM_I2C_GetDataCount(&I2C1_RES));
}

static int32_t I2C1_Control(uint32_t control, uint32_t arg)
{
    return (ARM_I2C_Control(&I2C1_RES, control, arg));
}

static ARM_I2C_STATUS I2C1_GetStatus(void)
{
    return (ARM_I2C_GetStatus(&I2C1_RES));
}

void I2C1_IRQHandler(void)
{
    i2c_transfer_info_t *transfer = &(I2C1_RES.transfer);
    ARM_I2C_STATUS *i2c_stat = &(I2C1_RES.status);

    /* Check for master mode */
    if (I2C1_RES.mode == I2C_MASTER_MODE)
    {
        if (transfer->curr_stat == I2C_TRANSFER_MST_TX)
        {
            /* Master transmit*/
            i2c_master_tx_isr(I2C1_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_MST_RX)
        {
            /* Master receive */
            i2c_master_rx_isr(I2C1_RES.regs, transfer);
        }
    }
    else /* Slave mode */
    {
        if(transfer->curr_stat == I2C_TRANSFER_SLV_TX)
        {
           /* slave transmit*/
           i2c_slave_tx_isr(I2C1_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_SLV_RX)
        {
           /* slave receive */
            i2c_slave_rx_isr(I2C1_RES.regs, transfer);
        }
    }/* Slave mode */

    if (transfer->status & I2C_TRANSFER_STATUS_DONE)
    {
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;

        I2C1_RES.cb_event(ARM_I2C_EVENT_TRANSFER_DONE);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_GENERAL_CALL)
    {
        I2C1_RES.cb_event(ARM_I2C_EVENT_GENERAL_CALL);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_BUS_CLEAR)
    {
        I2C1_RES.cb_event(ARM_I2C_EVENT_BUS_CLEAR);
    }
    else
    {
#if RTE_I2C1_DMA_ENABLE
        if(I2C1_RES.status.direction == I2C_DIR_TRANSMITTER)
        {
            I2C_DMA_Stop(&I2C1_RES.dma_cfg->dma_tx);
        }
        else
        {
            I2C_DMA_Stop(&I2C1_RES.dma_cfg->dma_rx);
        }
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;
#endif

        if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_TRANSMIT)
        {
             I2C1_RES.cb_event(ARM_I2C_EVENT_SLAVE_TRANSMIT);
             i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_RECEIVE)
        {
             I2C1_RES.cb_event(ARM_I2C_EVENT_SLAVE_RECEIVE);
             i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ADDRESS_NACK)
        {
             I2C1_RES.cb_event(ARM_I2C_EVENT_ADDRESS_NACK);
             i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ARBITRATION_LOST)
        {
             i2c_stat->arbitration_lost = 1U;

             I2C1_RES.cb_event(ARM_I2C_EVENT_ARBITRATION_LOST);
             i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_BUS_ERROR)
        {
             i2c_stat->bus_error = 1U;

             I2C1_RES.cb_event(ARM_I2C_EVENT_BUS_ERROR);
             i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_INCOMPLETE)
        {
             I2C1_RES.cb_event(ARM_I2C_EVENT_TRANSFER_INCOMPLETE);
             i2c_stat->busy = 0U;
        }
    }
    transfer->status = I2C_TRANSFER_STATUS_NONE;
}

#if RTE_I2C1_DMA_ENABLE
static void I2C1_DMACallback(uint32_t event, int8_t peri_num)
{
    I2C_DMACallback(event, peri_num, &I2C1_RES);
}
#endif

/* I2C1 Driver Control Block */
extern ARM_DRIVER_I2C Driver_I2C1;
ARM_DRIVER_I2C Driver_I2C1 = {
    ARM_I2C_GetVersion,
    ARM_I2C_GetCapabilities,
    I2C1_Initialize,
    I2C1_Uninitialize,
    I2C1_PowerControl,
    I2C1_MasterTransmit,
    I2C1_MasterReceive,
    I2C1_SlaveTransmit,
    I2C1_SlaveReceive,
    I2C1_GetDataCount,
    I2C1_Control,
    I2C1_GetStatus
};
#endif /* RTE_I2C1 */

/* I2C2 Driver Instance */
#if (RTE_I2C2)

#if RTE_I2C2_DMA_ENABLE
static void I2C2_DMACallback(uint32_t event, int8_t peri_num);

static I2C_DMA_HW_CONFIG I2C2_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C2_DMA),
        .dma_periph_req = I2C2_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C2_DMA,
             .group    = I2C2_DMA_GROUP,
             .channel  = I2C2_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2C2_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C2_DMA),
        .dma_periph_req = I2C2_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C2_DMA,
             .group    = I2C2_DMA_GROUP,
             .channel  = I2C2_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2C2_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

/* I2C2 Driver Resources */
static I2C_RESOURCES I2C2_RES =
{
    .regs                  = (I2C_Type *)I2C2_BASE,
    .irq_num               = (IRQn_Type)I2C2_IRQ_IRQn,
    .irq_priority          = (uint32_t)RTE_I2C2_IRQ_PRIORITY,
#if RTE_I2C2_DMA_ENABLE
    .dma_enable          = RTE_I2C2_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2C2_DMA_IRQ_PRI,
    .dma_cb              = I2C2_DMACallback,
    .dma_cfg             = &I2C2_DMA_HW_CONFIG,
#endif
    .tx_fifo_threshold     = RTE_I2C2_TX_FIFO_THRESHOLD,
    .rx_fifo_threshold     = RTE_I2C2_RX_FIFO_THRESHOLD
};

static int32_t I2C2_Initialize(ARM_I2C_SignalEvent_t cb_event)
{
    return ARM_I2C_Initialize(cb_event, &I2C2_RES);
}

static int32_t I2C2_Uninitialize(void)
{
    return ARM_I2C_Uninitialize(&I2C2_RES);
}

static int32_t I2C2_PowerControl(ARM_POWER_STATE state)
{
    return ARM_I2C_PowerControl(state, &I2C2_RES);
}

static int32_t I2C2_MasterTransmit(uint32_t addr, const uint8_t *data,
                                   uint32_t num, bool xfer_pending)
{
    return ARM_I2C_MasterTransmit(&I2C2_RES, addr, data, num, xfer_pending);
}

static int32_t I2C2_MasterReceive(uint32_t addr, uint8_t *data,
                                  uint32_t num, bool xfer_pending)
{
    return (ARM_I2C_MasterReceive(&I2C2_RES, addr, data, num, xfer_pending));
}

static int32_t I2C2_SlaveTransmit(const uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveTransmit(&I2C2_RES, data, num));
}

static int32_t I2C2_SlaveReceive(uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveReceive(&I2C2_RES, data, num));
}

static int32_t I2C2_GetDataCount(void)
{
    return (ARM_I2C_GetDataCount(&I2C2_RES));
}

static int32_t I2C2_Control(uint32_t control, uint32_t arg)
{
    return (ARM_I2C_Control(&I2C2_RES, control, arg));
}

static ARM_I2C_STATUS I2C2_GetStatus(void)
{
    return (ARM_I2C_GetStatus(&I2C2_RES));
}

void I2C2_IRQHandler(void)
{
    i2c_transfer_info_t *transfer = &(I2C2_RES.transfer);
    ARM_I2C_STATUS *i2c_stat = &(I2C2_RES.status);

    /* Check for master mode */
    if (I2C2_RES.mode == I2C_MASTER_MODE)
    {
        if (transfer->curr_stat == I2C_TRANSFER_MST_TX)
        {
            /* Master transmit*/
            i2c_master_tx_isr(I2C2_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_MST_RX)
        {
            /* Master receive */
            i2c_master_rx_isr(I2C2_RES.regs, transfer);
        }
    }
    else /* Slave mode */
    {
        if(transfer->curr_stat == I2C_TRANSFER_SLV_TX)
        {
           /* slave transmit*/
           i2c_slave_tx_isr(I2C2_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_SLV_RX)
        {
           /* slave receive */
            i2c_slave_rx_isr(I2C2_RES.regs, transfer);
        }
    }/* Slave mode */

    if (transfer->status & I2C_TRANSFER_STATUS_DONE)
    {
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;

        I2C2_RES.cb_event(ARM_I2C_EVENT_TRANSFER_DONE);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_GENERAL_CALL)
    {
        I2C2_RES.cb_event(ARM_I2C_EVENT_GENERAL_CALL);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_BUS_CLEAR)
    {
        I2C2_RES.cb_event(ARM_I2C_EVENT_BUS_CLEAR);
    }
    else
    {
#if RTE_I2C2_DMA_ENABLE
        if(I2C2_RES.status.direction == I2C_DIR_TRANSMITTER)
        {
          I2C_DMA_Stop(&I2C2_RES.dma_cfg->dma_tx);
        }
        else
        {
          I2C_DMA_Stop(&I2C2_RES.dma_cfg->dma_rx);
        }
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;
#endif

        if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_TRANSMIT)
        {
            I2C2_RES.cb_event(ARM_I2C_EVENT_SLAVE_TRANSMIT);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_RECEIVE)
        {
            I2C2_RES.cb_event(ARM_I2C_EVENT_SLAVE_RECEIVE);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ADDRESS_NACK)
        {
            I2C2_RES.cb_event(ARM_I2C_EVENT_ADDRESS_NACK);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ARBITRATION_LOST)
        {
            i2c_stat->arbitration_lost = 1U;

            I2C2_RES.cb_event(ARM_I2C_EVENT_ARBITRATION_LOST);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_BUS_ERROR)
        {
            i2c_stat->bus_error = 1U;

            I2C2_RES.cb_event(ARM_I2C_EVENT_BUS_ERROR);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_INCOMPLETE)
        {
            I2C2_RES.cb_event(ARM_I2C_EVENT_TRANSFER_INCOMPLETE);
            i2c_stat->busy = 0U;
        }
    }
    transfer->status = I2C_TRANSFER_STATUS_NONE;
}

#if RTE_I2C2_DMA_ENABLE
static void I2C2_DMACallback(uint32_t event, int8_t peri_num)
{
    I2C_DMACallback(event, peri_num, &I2C2_RES);
}
#endif

/* I2C2 Driver Control Block */
extern ARM_DRIVER_I2C Driver_I2C2;
ARM_DRIVER_I2C Driver_I2C2 = {
    ARM_I2C_GetVersion,
    ARM_I2C_GetCapabilities,
    I2C2_Initialize,
    I2C2_Uninitialize,
    I2C2_PowerControl,
    I2C2_MasterTransmit,
    I2C2_MasterReceive,
    I2C2_SlaveTransmit,
    I2C2_SlaveReceive,
    I2C2_GetDataCount,
    I2C2_Control,
    I2C2_GetStatus
};
#endif /* RTE_I2C2 */

/* I2C3 Driver Instance */
#if (RTE_I2C3)

#if RTE_I2C3_DMA_ENABLE
static void I2C3_DMACallback(uint32_t event, int8_t peri_num);

static I2C_DMA_HW_CONFIG I2C3_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C3_DMA),
        .dma_periph_req = I2C3_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C3_DMA,
             .group    = I2C3_DMA_GROUP,
             .channel  = I2C3_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2C3_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2C3_DMA),
        .dma_periph_req = I2C3_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2C3_DMA,
             .group    = I2C3_DMA_GROUP,
             .channel  = I2C3_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2C3_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

/* I2C3 Driver Resources */
static I2C_RESOURCES I2C3_RES =
{
    .regs                  = (I2C_Type *)I2C3_BASE,
    .irq_num               = (IRQn_Type)I2C3_IRQ_IRQn,
    .irq_priority          = (uint32_t)RTE_I2C3_IRQ_PRIORITY,
#if RTE_I2C3_DMA_ENABLE
    .dma_enable          = RTE_I2C3_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2C3_DMA_IRQ_PRI,
    .dma_cb              = I2C3_DMACallback,
    .dma_cfg             = &I2C3_DMA_HW_CONFIG,
#endif
    .tx_fifo_threshold     = RTE_I2C3_TX_FIFO_THRESHOLD,
    .rx_fifo_threshold     = RTE_I2C3_RX_FIFO_THRESHOLD
};

static int32_t I2C3_Initialize(ARM_I2C_SignalEvent_t cb_event)
{
    return ARM_I2C_Initialize(cb_event, &I2C3_RES);
}

static int32_t I2C3_Uninitialize(void)
{
    return ARM_I2C_Uninitialize(&I2C3_RES);
}

static int32_t I2C3_PowerControl(ARM_POWER_STATE state)
{
    return ARM_I2C_PowerControl(state, &I2C3_RES);
}

static int32_t I2C3_MasterTransmit(uint32_t addr, const uint8_t *data,
                                   uint32_t num, bool xfer_pending)
{
    return ARM_I2C_MasterTransmit(&I2C3_RES, addr, data, num, xfer_pending);
}

static int32_t I2C3_MasterReceive(uint32_t addr, uint8_t *data,
                                  uint32_t num, bool xfer_pending)
{
    return (ARM_I2C_MasterReceive(&I2C3_RES, addr, data, num, xfer_pending));
}

static int32_t I2C3_SlaveTransmit(const uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveTransmit(&I2C3_RES, data, num));
}

static int32_t I2C3_SlaveReceive(uint8_t *data, uint32_t num)
{
    return (ARM_I2C_SlaveReceive(&I2C3_RES, data, num));
}

static int32_t I2C3_GetDataCount(void)
{
    return (ARM_I2C_GetDataCount(&I2C3_RES));
}

static int32_t I2C3_Control(uint32_t control, uint32_t arg)
{
    return (ARM_I2C_Control(&I2C3_RES, control, arg));
}

static ARM_I2C_STATUS I2C3_GetStatus(void)
{
    return (ARM_I2C_GetStatus(&I2C3_RES));
}

void I2C3_IRQHandler(void)
{
    i2c_transfer_info_t *transfer = &(I2C3_RES.transfer);
    ARM_I2C_STATUS *i2c_stat = &(I2C3_RES.status);

    /* Check for master mode */
    if (I2C3_RES.mode == I2C_MASTER_MODE)
    {
        if (transfer->curr_stat == I2C_TRANSFER_MST_TX)
        {
            /* Master transmit*/
            i2c_master_tx_isr(I2C3_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_MST_RX)
        {
            /* Master receive */
            i2c_master_rx_isr(I2C3_RES.regs, transfer);
        }
    }
    else /* Slave mode */
    {
        if(transfer->curr_stat == I2C_TRANSFER_SLV_TX)
        {
            /* slave transmit*/
            i2c_slave_tx_isr(I2C3_RES.regs, transfer);
        }
        if(transfer->curr_stat == I2C_TRANSFER_SLV_RX)
        {
            /* slave receive */
            i2c_slave_rx_isr(I2C3_RES.regs, transfer);
        }
    }/* Slave mode */

    if (transfer->status & I2C_TRANSFER_STATUS_DONE)
    {
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;

        I2C3_RES.cb_event(ARM_I2C_EVENT_TRANSFER_DONE);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_GENERAL_CALL)
    {
        I2C3_RES.cb_event(ARM_I2C_EVENT_GENERAL_CALL);
    }
    else if (transfer->status & I2C_TRANSFER_STATUS_BUS_CLEAR)
    {
        I2C3_RES.cb_event(ARM_I2C_EVENT_BUS_CLEAR);
    }
    else
    {
#if RTE_I2C3_DMA_ENABLE
        if(I2C3_RES.status.direction == I2C_DIR_TRANSMITTER)
        {
          I2C_DMA_Stop(&I2C3_RES.dma_cfg->dma_tx);
        }
        else
        {
          I2C_DMA_Stop(&I2C3_RES.dma_cfg->dma_rx);
        }
        /* set busy flag to 0U */
        i2c_stat->busy = 0U;
#endif

        if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_TRANSMIT)
        {
            I2C3_RES.cb_event(ARM_I2C_EVENT_SLAVE_TRANSMIT);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_SLAVE_RECEIVE)
        {
            I2C3_RES.cb_event(ARM_I2C_EVENT_SLAVE_RECEIVE);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ADDRESS_NACK)
        {
            I2C3_RES.cb_event(ARM_I2C_EVENT_ADDRESS_NACK);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_ARBITRATION_LOST)
        {
            i2c_stat->arbitration_lost = 1U;

            I2C3_RES.cb_event(ARM_I2C_EVENT_ARBITRATION_LOST);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_BUS_ERROR)
        {
            i2c_stat->bus_error = 1U;

            I2C3_RES.cb_event(ARM_I2C_EVENT_BUS_ERROR);
            i2c_stat->busy = 0U;
        }

        else if (transfer->status & I2C_TRANSFER_STATUS_INCOMPLETE)
        {
           I2C3_RES.cb_event(ARM_I2C_EVENT_TRANSFER_INCOMPLETE);
           i2c_stat->busy = 0U;
        }
    }
    transfer->status = I2C_TRANSFER_STATUS_NONE;
}

#if RTE_I2C3_DMA_ENABLE
static void I2C3_DMACallback(uint32_t event, int8_t peri_num)
{
    I2C_DMACallback(event, peri_num, &I2C3_RES);
}
#endif

/* I2C3 Driver Control Block */
extern ARM_DRIVER_I2C Driver_I2C3;
ARM_DRIVER_I2C Driver_I2C3 = {
    ARM_I2C_GetVersion,
    ARM_I2C_GetCapabilities,
    I2C3_Initialize,
    I2C3_Uninitialize,
    I2C3_PowerControl,
    I2C3_MasterTransmit,
    I2C3_MasterReceive,
    I2C3_SlaveTransmit,
    I2C3_SlaveReceive,
    I2C3_GetDataCount,
    I2C3_Control,
    I2C3_GetStatus
};
#endif /* RTE_I2C3 */

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
