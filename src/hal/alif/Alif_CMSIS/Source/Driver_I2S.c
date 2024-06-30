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
 * @file     Driver_I2S.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V3.0.0
 * @date     06-Jun-2023
 * @brief    CMSIS-Driver for I2S
 * @bug      None
 * @Note     None
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver_I2S_Private.h"
#include "Driver_SAI_EX.h"

#define ARM_SAI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(3, 0) /*!< I2S Driver Version */

static const ARM_DRIVER_VERSION DriverVersion = {
        ARM_SAI_API_VERSION,
        ARM_SAI_DRV_VERSION
};


#if !(RTE_I2S0 || RTE_I2S1 || RTE_I2S2 || RTE_I2S3)
#error "I2S is not enabled in the RTE_Device.h"
#endif

#if !defined(RTE_Drivers_SAI)
#error "I2S is not enabled in RTE_Components.h!"
#endif

/* Driver Capabilities */
static const ARM_SAI_CAPABILITIES DriverCapabilities = {
    1, /* supports asynchronous Transmit/Receive */
    1, /* supports synchronous Transmit/Receive */
    0, /* supports user defined Protocol */
    1, /* supports I2S Protocol */
    0, /* supports MSB/LSB justified Protocol */
    0, /* supports PCM short/long frame Protocol */
    0, /* supports AC'97 Protocol */
    1, /* supports Mono mode */
    0, /* supports Companding */
    1, /* supports MCLK (Master Clock) pin */
    0, /* supports Frame error event: \ref ARM_SAI_EVENT_FRAME_ERROR */
    0  /* reserved (must be zero) */
};

//
//  Functions
//

/**
  \fn          ARM_DRIVER_VERSION I2S_GetVersion(void)
  \brief       Get I2S driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION I2S_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn          ARM_SAI_CAPABILITIES I2S_GetCapabilities(void)
  \brief       Get I2S driver capabilities
  \return      \ref ARM_SAI_CAPABILITIES
*/
static ARM_SAI_CAPABILITIES I2S_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn          int32_t I2S_SetSamplingRate(I2S_RESOURCES *I2S)
  \brief       Set the audio sample rate
  \param[in]   I2S   Pointer to I2S resources
  \return      \ref  execution_status
*/
static int32_t I2S_SetSamplingRate(I2S_RESOURCES *I2S)
{
    uint32_t sclk_freq = 0;
    int32_t ret = 0;

    if(!I2S->sample_rate)
        return ARM_DRIVER_ERROR;

    sclk_freq = i2s_get_sclk_frequency(I2S->sample_rate, I2S->cfg->wss_len);

    ret = set_i2s_sampling_rate(I2S->instance, sclk_freq, I2S->cfg->clk_source);
    if(ret)
        return ARM_DRIVER_ERROR;

    return ARM_DRIVER_OK;
}

/**
  \fn          I2S_WSS I2S_GetWordSelectSize(uint8_t value)
  \brief       Get the Word Select Size from RTE
  \param[in]   value   Input value from RTE configuration
  \return      I2S_WSS  \ref I2S_WSS
*/
static I2S_WSS I2S_GetWordSelectSize(uint8_t value)
{
    I2S_WSS wss;

    switch(value)
    {
    case 0:
        wss = I2S_WSS_SCLK_CYCLES_16;
        break;
    case 1:
        wss = I2S_WSS_SCLK_CYCLES_24;
        break;
    case 2:
        wss = I2S_WSS_SCLK_CYCLES_32;
        break;
    default:
        wss = I2S_WSS_SCLK_CYCLES_MAX;
        break;
    }

    return wss;
}

/**
  \fn          I2S_SCLKG I2S_GetClockGatingCycles(uint8_t value)
  \brief       Get the Serial Clock gating cycles
  \param[in]   value   Input value from RTE configuration
  \return      I2S_SCLKG  \ref I2S_SCLKG
*/
static I2S_SCLKG I2S_GetClockGatingCycles(uint8_t value)
{
    I2S_SCLKG sclkg;

    switch(value)
    {
    case 0:
        sclkg = I2S_SCLKG_NONE;
        break;
    case 1:
        sclkg = I2S_SCLKG_CLOCK_CYCLES_12;
        break;
    case 2:
        sclkg = I2S_SCLKG_CLOCK_CYCLES_16;
        break;
    case 3:
        sclkg = I2S_SCLKG_CLOCK_CYCLES_20;
        break;
    case 4:
        sclkg = I2S_SCLKG_CLOCK_CYCLES_24;
        break;
    default:
        sclkg = I2S_SCLKG_CLOCK_CYCLES_MAX;
        break;
    }

    return sclkg;
}

#if I2S_DMA_ENABLE
/**
  \fn          int32_t I2S_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for I2S
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t I2S_DMA_PowerControl(ARM_POWER_STATE state,
                                            DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       PowerControl DMA for I2S
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_PowerControl(ARM_POWER_STATE state,
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
  \fn          int32_t I2S_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for I2S
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t I2S_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of I2S
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t I2S_DMA_EnableMono(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Enable I2S DMA Mono transfer
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_EnableMono(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Enable I2S mono feature */
    status = dma_drv->Control(&dma_periph->dma_handle, ARM_DMA_I2S_MONO_MODE, 0);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2S_DMA_Usermcode(DMA_PERIPHERAL_CONFIG *dma_periph,
                                         uint32_t dma_mcode)
  \brief       Use Custom Microcode for I2S
  \param[in]   dma_periph  Pointer to DMA resources
  \param[in]   dma_mcode  Pointer to DMA microcode
  \return      \ref        execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_Usermcode(DMA_PERIPHERAL_CONFIG *dma_periph,
                                          uint32_t dma_mcode)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Use User provided custom microcode */
    status = dma_drv->Control(&dma_periph->dma_handle,
                              ARM_DMA_USER_PROVIDED_MCODE,
                              dma_mcode);
    if(status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2S_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                     ARM_DMA_PARAMS *dma_params)
  \brief       Start I2S DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref           execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
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
  \fn          int32_t I2S_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop I2S DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t I2S_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph
                                         uint32_t *count)
  \brief       Status of I2S DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \param[in]   count        Current transfer count
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t I2S_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph,
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
#endif /* I2S_DMA_ENABLE */

/**
  \fn          int32_t I2S_PowerControl(ARM_POWER_STATE state, I2S_RESOURCES *I2S)
  \brief       Control I2S Interface Power.
  \param[in]   state  Power state
  \param[in]   I2S   Pointer to I2S resources
  \return      \ref  execution_status
*/
static int32_t I2S_PowerControl(ARM_POWER_STATE state, I2S_RESOURCES *I2S)
{
    if(I2S->state.initialized == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (state) {
    case ARM_POWER_OFF:

        if(I2S->state.powered == 0)
        {
            return ARM_DRIVER_OK;
        }

        /** Disable I2S IRQ */
        NVIC_DisableIRQ(I2S->irq);

        /* Disable the DMA */
        i2s_rx_dma_disable(I2S->regs);
        i2s_tx_dma_disable(I2S->regs);

        /* Disable the I2S Global Enable */
        i2s_rxblock_disable(I2S->regs);
        i2s_txblock_disable(I2S->regs);

        /* Clear Any Pending IRQ*/
        NVIC_ClearPendingIRQ(I2S->irq);

        /* Mask all the interrupts */
        i2s_disable_rx_interrupt(I2S->regs);
        i2s_disable_tx_interrupt(I2S->regs);

        I2S->drv_status.status = 0U;

        i2s_clock_disable(I2S->regs);
        i2s_disable(I2S->regs);

        /* Disable the I2S module clock */
        disable_i2s_clock(I2S->instance);
        disable_i2s_sclk_aon(I2S->instance);

        I2S->state.powered = 0;
        break;

    case ARM_POWER_FULL:

        if(I2S->state.powered == 1)
        {
            return ARM_DRIVER_OK;
        }

        I2S->drv_status.status = 0U;

        /* Initialize with the internal clock source */
        select_i2s_clock_source(I2S->instance, I2S_INTERNAL_CLOCK_SOURCE);

        /* Enable the I2S module clock */
        enable_i2s_sclk_aon(I2S->instance);
        enable_i2s_clock(I2S->instance);

        /* Enable I2S */
        i2s_enable(I2S->regs);

        /* Mask all the interrupts */
        i2s_disable_rx_interrupt(I2S->regs);
        i2s_disable_tx_interrupt(I2S->regs);

        /* Enable I2S and IRQ */
        NVIC_ClearPendingIRQ(I2S->irq);
        NVIC_SetPriority(I2S->irq, I2S->cfg->irq_priority);
        NVIC_EnableIRQ(I2S->irq);

        /* Set the power flag enabled */
        I2S->state.powered = 1;
        break;

    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    }

#if I2S_DMA_ENABLE
    if(I2S->cfg->dma_enable)
    {
        /* Power Control DMA for I2S-Tx */
        if(I2S_DMA_PowerControl(state, &I2S->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        /* Power Control DMA for I2S-Rx */
        if(I2S_DMA_PowerControl(state, &I2S->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2S_Initialize(ARM_SAI_SignalEvent_t cb_event, I2S_RESOURCES *I2S)
  \brief       Initialize I2S Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SAI_SignalEvent
  \param[in]   I2S       Pointer to I2S resources
  \return      \ref      execution_status
*/
static int32_t I2S_Initialize(ARM_SAI_SignalEvent_t cb_event, I2S_RESOURCES *I2S)
{
    int32_t ret = ARM_DRIVER_OK;
    bool blocking_mode = false;

    if(I2S->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

#if I2S_BLOCKING_MODE_ENABLE
    if(I2S->cfg->blocking_mode)
        blocking_mode = true;
#endif

    if(!blocking_mode && !cb_event)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(I2S->cfg->wss_len >= I2S_WSS_SCLK_CYCLES_MAX)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(I2S->cfg->sclkg >= I2S_SCLKG_CLOCK_CYCLES_MAX)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(I2S->cfg->rx_fifo_trg_lvl > I2S_FIFO_TRIGGER_LEVEL_MAX)
        return ARM_DRIVER_ERROR_PARAMETER;

    if(I2S->cfg->tx_fifo_trg_lvl > I2S_FIFO_TRIGGER_LEVEL_MAX)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Initialize the driver elements*/
    I2S->cb_event          = cb_event;
    I2S->drv_status.status = 0U;

    /* Initialize the transfer structure */
    I2S->transfer.tx_buff        =  NULL;
    I2S->transfer.tx_current_cnt =  0;
    I2S->transfer.tx_total_cnt   =  0;
    I2S->transfer.rx_total_cnt   =  0;
    I2S->transfer.rx_buff        =  NULL;
    I2S->transfer.rx_current_cnt =  0;
    I2S->transfer.mono_mode      = false;
    I2S->transfer.status         = I2S_TRANSFER_STATUS_NONE;

#if I2S_DMA_ENABLE
    if(I2S->cfg->dma_enable)
    {
        I2S->dma_cfg->dma_rx.dma_handle = -1;
        I2S->dma_cfg->dma_tx.dma_handle = -1;

        /* Initialize DMA for I2S-Tx */
        if(I2S_DMA_Initialize(&I2S->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;

        /* Initialize DMA for I2S-Rx */
        if(I2S_DMA_Initialize(&I2S->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            return ARM_DRIVER_ERROR;
    }
#endif

    I2S->state.initialized = 1;

    return ret;
}

/**
  \fn          int32_t I2S_Uninitialize(I2S_RESOURCES *I2S)
  \brief       De-initialize I2S Interface.
  \param[in]   I2S   Pointer to I2S resources
  \return      \ref  execution_status
*/
static int32_t I2S_Uninitialize(I2S_RESOURCES *I2S)
{
    I2S->cb_event = NULL;

#if I2S_DMA_ENABLE
    if(I2S->cfg->dma_enable)
    {
        I2S->dma_cfg->dma_rx.dma_handle = -1;
        I2S->dma_cfg->dma_tx.dma_handle = -1;
    }
#endif

    I2S->flags             = 0U;
    I2S->drv_status.status = 0U;
    I2S->state.initialized = 0U;

    /* Initialize the transfer structure */
    I2S->transfer.tx_buff        =  NULL;
    I2S->transfer.tx_current_cnt =  0;
    I2S->transfer.tx_total_cnt   =  0;
    I2S->transfer.rx_total_cnt   =  0;
    I2S->transfer.rx_buff        =  NULL;
    I2S->transfer.rx_current_cnt =  0;
    I2S->transfer.mono_mode      = false;
    I2S->transfer.status         = I2S_TRANSFER_STATUS_NONE;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2S_Send(const void *data, uint32_t num, I2S_RESOURCES *I2S)
  \brief       Start sending data to I2S transmitter.
  \param[in]   data  Location of the data buffer to be transmitted
  \param[in]   num   Number of data items to send
  \param[in]   I2S   Pointer to I2S resources
  \return      \ref  execution_status
*/
static int32_t I2S_Send(const void *data, uint32_t num, I2S_RESOURCES *I2S)
{
    /* Verify the input parameters */
    if(!data || !num)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Verify whether the driver is configured and powered */
    if(I2S->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Check if any Transfer is in progress */
    if(I2S->drv_status.status_b.tx_busy)
        return ARM_DRIVER_ERROR_BUSY;

    /* If the WSS len is 16, check if it is aligned to 2 bytes */
    if((I2S->cfg->wss_len == I2S_WSS_SCLK_CYCLES_16) && ((uint32_t)data & 0x1U) != 0U)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* If the WSS len is greater than 16, check if it is aligned to 4 bytes */
    if((I2S->cfg->wss_len > I2S_WSS_SCLK_CYCLES_16) && ((uint32_t)data & 0x3U) != 0U)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Set the Tx flags */
    I2S->drv_status.status_b.tx_busy = 1U;
    I2S->drv_status.status_b.tx_underflow = 0U;

    /* Fill the transfer information */
    I2S->transfer.tx_buff        = data;
    I2S->transfer.tx_current_cnt = 0U;

    if((I2S->cfg->wlen > I2S_WLEN_RES_NONE)
        && (I2S->cfg->wlen <= I2S_WLEN_RES_16_BIT))
    {
        I2S->transfer.tx_total_cnt  = num * sizeof(uint16_t);
    }
    else
    {
        I2S->transfer.tx_total_cnt  = num * sizeof(uint32_t);
    }

    if(I2S->flags & I2S_FLAG_DRV_MONO_MODE)
    {
        I2S->transfer.mono_mode  = true;
    }
    else
    {
        I2S->transfer.mono_mode  = false;
    }

#if I2S_DMA_ENABLE
    if(I2S->cfg->dma_enable)
    {
        int32_t        status;
        ARM_DMA_PARAMS dma_params;

        /* Prepare the I2S controller for DMA transmission */
        i2s_dma_send(I2S->regs);

        /* Start the DMA engine for sending the data to I2S */
        dma_params.peri_reqno    = (int8_t)I2S->dma_cfg->dma_tx.dma_periph_req;
        dma_params.dir           = ARM_DMA_MEM_TO_DEV;
        dma_params.cb_event      = I2S->dma_cb;
        dma_params.src_addr      = data;
        dma_params.dst_addr      = i2s_get_dma_tx_addr(I2S->regs);
        dma_params.num_bytes     = I2S->transfer.tx_total_cnt;
        dma_params.irq_priority  = I2S->cfg->dma_irq_priority;

        if((I2S->cfg->wlen > I2S_WLEN_RES_NONE)
            && (I2S->cfg->wlen <= I2S_WLEN_RES_16_BIT))
        {
            dma_params.burst_size = BS_BYTE_2;
        }
        else
        {
            dma_params.burst_size = BS_BYTE_4;
        }

        /* See if this operation is using only one channel */
        if(I2S->flags & I2S_FLAG_DRV_MONO_MODE)
        {
            dma_params.burst_len  = 1;
        }
        else
        {
            dma_params.burst_len  = I2S_FIFO_DEPTH - I2S->cfg->tx_fifo_trg_lvl;
        }

        /* Start DMA transfer */
        status = I2S_DMA_Start(&I2S->dma_cfg->dma_tx, &dma_params);
        if(status)
            return ARM_DRIVER_ERROR;
    }
    else
#endif
    {
#if I2S_BLOCKING_MODE_ENABLE
        if(I2S->cfg->blocking_mode)
        {
            i2s_send_blocking(I2S->regs, &I2S->transfer);

            if(I2S->transfer.status & I2S_TRANSFER_STATUS_TX_COMPLETE)
            {
                I2S->drv_status.status_b.tx_busy = 0U;
                I2S->transfer.status &= ~I2S_TRANSFER_STATUS_TX_COMPLETE;
            }
        }
        else
#endif
        {
            i2s_send(I2S->regs);
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2S_Receive(void *data, uint32_t num, I2S_RESOURCES *I2S)
  \brief       Start receiving data from I2S receiver.
  \param[out]  data  Data pointer to store the received data from I2S
  \param[in]   num   Number of data items to receive
  \param[in]   I2S   Pointer to I2S resources
  \return      \ref  execution_status
*/
static int32_t I2S_Receive(void *data, uint32_t num, I2S_RESOURCES *I2S)
{
    /* Verify the input parameters */
    if(!data || !num)
        return ARM_DRIVER_ERROR_PARAMETER;

    /* Verify whether the driver is configured and powered*/
    if(I2S->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Check if any Transfer is in progress*/
    if(I2S->drv_status.status_b.rx_busy)
        return ARM_DRIVER_ERROR_BUSY;

    /* If the WSS len is 16, check if it is aligned to 2 bytes */
    if((I2S->cfg->wss_len == I2S_WSS_SCLK_CYCLES_16)
        && ((uint32_t)data & 0x1U) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* If the WSS len is greater than 16, check if it is aligned to 4 bytes */
    if((I2S->cfg->wss_len > I2S_WSS_SCLK_CYCLES_16)
        && ((uint32_t)data & 0x3U) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Set the Rx flags*/
    I2S->drv_status.status_b.rx_busy = 1U;
    I2S->drv_status.status_b.rx_overflow = 0U;

#if I2S_DMA_ENABLE
    /* Check if DMA & Mono is enabled for this */
    if(I2S->cfg->dma_enable && (I2S->flags & I2S_FLAG_DRV_MONO_MODE))
    {
        /*
         * This is a hack for the mono + dma mode.
         * Mono mode feature is not supported by I2S HW.
         *
         * To achieve this in software, we need to read
         * Left and Right channel and ignore the right channel
         * data.
         *
         * But in DMA, we need to read the data from peripheral
         * and write to the buffer. We can't simply ignore the write here.
         * So we use the next buffer location to read the right channel
         * and re-adjust the destination address to store the next
         * left channel data in the same location.
         *
         * Due to this, there will be memory overwrite to num+1 buffer.
         * To avoid this, instruct the DMA to copy the number of samples
         * to (num - 1)
         */
        num = num - 1;
    }
#endif

    /* Fill in the transfer buffer information */
    I2S->transfer.rx_buff        = data;
    I2S->transfer.rx_current_cnt = 0U;

    if ((I2S->cfg->wlen > I2S_WLEN_RES_NONE) && (I2S->cfg->wlen <= I2S_WLEN_RES_16_BIT))
        I2S->transfer.rx_total_cnt  = num * sizeof(uint16_t);
    else
        I2S->transfer.rx_total_cnt  = num * sizeof(uint32_t);

    /* See if this operation is using only one channel */
    if(I2S->flags & I2S_FLAG_DRV_MONO_MODE)
        I2S->transfer.mono_mode  = true;
    else
        I2S->transfer.mono_mode  = false;

#if I2S_DMA_ENABLE
    if(I2S->cfg->dma_enable)
    {
        ARM_DMA_PARAMS dma_params;
        int32_t        status;

        /* Start the DMA engine for sending the data to I2S */
        dma_params.peri_reqno    = (int8_t)I2S->dma_cfg->dma_rx.dma_periph_req;
        dma_params.dir           = ARM_DMA_DEV_TO_MEM;
        dma_params.cb_event      = I2S->dma_cb;
        dma_params.src_addr      = i2s_get_dma_rx_addr(I2S->regs);
        dma_params.dst_addr      = data;
        dma_params.num_bytes     = I2S->transfer.rx_total_cnt;
        dma_params.irq_priority  = I2S->cfg->dma_irq_priority;

        if ((I2S->cfg->wlen > I2S_WLEN_RES_NONE)
             && (I2S->cfg->wlen <= I2S_WLEN_RES_16_BIT))
        {
            dma_params.burst_size = BS_BYTE_2;
        }
        else
        {
            dma_params.burst_size = BS_BYTE_4;
        }

        if(I2S->flags & I2S_FLAG_DRV_MONO_MODE)
        {
            dma_params.burst_len  = 1;
        }
        else
        {
            dma_params.burst_len  = I2S->cfg->rx_fifo_trg_lvl + 1;
        }

        /* Start DMA transfer */
        status = I2S_DMA_Start(&I2S->dma_cfg->dma_rx, &dma_params);
        if(status)
            return ARM_DRIVER_ERROR;

        /* Prepare the I2S controller for DMA reception */
        i2s_dma_receive(I2S->regs);
    }
    else
#endif
    {
#if I2S_BLOCKING_MODE_ENABLE
        if(I2S->cfg->blocking_mode)
        {
            i2s_receive_blocking(I2S->regs, &I2S->transfer);

            if(I2S->transfer.status & I2S_TRANSFER_STATUS_RX_COMPLETE)
            {
                I2S->drv_status.status_b.rx_busy = 0U;
                I2S->transfer.status &= ~I2S_TRANSFER_STATUS_RX_COMPLETE;
            }

            if(I2S->transfer.status & I2S_TRANSFER_STATUS_RX_OVERFLOW)
            {
                I2S->transfer.status &= ~I2S_TRANSFER_STATUS_RX_OVERFLOW;
                I2S->drv_status.status_b.rx_overflow = 1U;
            }
        }
        else
#endif
        {
            i2s_receive(I2S->regs);
        }
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          uint32_t I2S_GetTxCount(I2S_RESOURCES *I2S)
  \brief       Get the total transmitted items
  \param[in]   I2S  Pointer to I2S resources
  \return      \ret number of data items transmitted
*/
static uint32_t I2S_GetTxCount(I2S_RESOURCES *I2S)
{
#if I2S_DMA_ENABLE
    uint32_t tx_current_cnt = 0;

    if(I2S->cfg->dma_enable)
    {
        /* Get the current transfer count */
        I2S_DMA_GetStatus(&I2S->dma_cfg->dma_tx, &tx_current_cnt);

        I2S->transfer.tx_current_cnt = tx_current_cnt;
    }
#endif
    return I2S->transfer.tx_current_cnt;
}

/**
  \fn          uint32_t I2S_GetRxCount(I2S_RESOURCES *I2S)
  \brief       Get total items received
  \param[in]   I2S   Pointer to I2S resources
  \return      \ret  number of data items received
*/
static uint32_t I2S_GetRxCount(I2S_RESOURCES *I2S)
{
#if I2S_DMA_ENABLE
    uint32_t rx_current_cnt = 0;

    if(I2S->cfg->dma_enable)
    {
        /* Get the current transfer count */
        I2S_DMA_GetStatus(&I2S->dma_cfg->dma_rx, &rx_current_cnt);

        I2S->transfer.rx_current_cnt = rx_current_cnt;
    }
#endif
    return I2S->transfer.rx_current_cnt;
}

/**
  \fn          int32_t I2S_Control(uint32_t control, uint32_t arg1,
                                   uint32_t arg2, I2S_RESOURCES *I2S)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg1     Argument 1 of operation (optional)
  \param[in]   arg2     Argument 2 of operation (optional)
  \param[in]   I2S      Pointer to I2S resources
  \return      \ref     execution_status and driver specific \ref sai_execution_status
*/

static int32_t I2S_Control(uint32_t control, uint32_t arg1,
                           uint32_t arg2, I2S_RESOURCES *I2S)
{
    uint16_t frame_length = 0;
    uint8_t  datasize = 0;
    int32_t  ret = 0;
    uint16_t mclk_prescaler = 0;

    /* Verify whether the driver is initialized and powered*/
    if(I2S->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Handle Control Codes */
    switch(control & ARM_SAI_CONTROL_Msk)
    {
    case ARM_SAI_CONFIGURE_TX:
        /* Set FIFO Trigger Level */
        i2s_set_tx_triggerLevel(I2S->regs, I2S->cfg->tx_fifo_trg_lvl);

        break;
    case ARM_SAI_CONFIGURE_RX:
        /* Set FIFO Trigger Level */
        i2s_set_rx_triggerLevel(I2S->regs, I2S->cfg->rx_fifo_trg_lvl);

        break;
    case ARM_SAI_CONTROL_TX:
        /* Enable TX */
        if(arg1 == true)
        {
            /* Reset the Tx FIFO */
            i2s_reset_tx_fifo(I2S->regs);

            /* Set WLEN */
            i2s_set_tx_wlen(I2S->regs, I2S->cfg->wlen);

            /* Enable Master Clock */
            i2s_clock_enable(I2S->regs, I2S->cfg->sclkg, I2S->cfg->wss_len);

            /* Enable Tx Block */
            i2s_txblock_enable(I2S->regs);

#if I2S_DMA_ENABLE
            /* Check if DMA is enabled for this */
            if(I2S->cfg->dma_enable)
            {
                /* Try to allocate a DMA channel */
                if(I2S_DMA_Allocate(&I2S->dma_cfg->dma_tx))
                    return ARM_DRIVER_ERROR;

                if(I2S->flags & I2S_FLAG_DRV_MONO_MODE)
                {
                    if(I2S_DMA_EnableMono(&I2S->dma_cfg->dma_tx))
                        return ARM_DRIVER_ERROR;
                }

                /* Enable the DMA interface of I2S */
                i2s_tx_dma_enable(I2S->regs);
            }
#endif

        }
        else if(arg1 == false)
        {
#if I2S_DMA_ENABLE
            if(I2S->cfg->dma_enable)
            {
                /* Disable the DMA interface of I2S */
                i2s_tx_dma_disable(I2S->regs);

                /* Deallocate DMA channel */
                if(I2S_DMA_DeAllocate(&I2S->dma_cfg->dma_tx) == ARM_DRIVER_ERROR)
                    return ARM_DRIVER_ERROR;
            }
#endif
            /* Disable Tx Channel */
            i2s_txchannel_disable(I2S->regs);

            /* Disable Tx Block */
            i2s_txblock_disable(I2S->regs);

            /* Disable Tx Interrupt */
            i2s_disable_tx_interrupt(I2S->regs);

            /* Disable Master Clock */
            i2s_clock_disable(I2S->regs);

            /* Set the Tx flags*/
            I2S->drv_status.status_b.tx_busy = 0U;

        }
        else
            return ARM_DRIVER_ERROR;

        return ARM_DRIVER_OK;
    case ARM_SAI_CONTROL_RX:
        /* Enable RX */
        if(arg1 == true)
        {
            /* Reset the Rx FIFO */
            i2s_reset_rx_fifo(I2S->regs);

            /* Set WLEN */
            i2s_set_rx_wlen(I2S->regs, I2S->cfg->wlen);

            /* Enable serial Clock */
            i2s_clock_enable(I2S->regs, I2S->cfg->sclkg, I2S->cfg->wss_len);

            /* Enable Rx Block */
            i2s_rxblock_enable(I2S->regs);

#if I2S_DMA_ENABLE
            /* Check if DMA is enabled for this */
            if(I2S->cfg->dma_enable)
            {
                /* Try to allocate a DMA channel */
                if(I2S_DMA_Allocate(&I2S->dma_cfg->dma_rx))
                    return ARM_DRIVER_ERROR;

                if(I2S->flags & I2S_FLAG_DRV_MONO_MODE)
                {
                    if(I2S_DMA_EnableMono(&I2S->dma_cfg->dma_rx))
                        return ARM_DRIVER_ERROR;
                }

                /* Enable the DMA interface of I2S */
                i2s_rx_dma_enable(I2S->regs);
            }
#endif
        }
        else if(arg1 == false)
        {
#if I2S_DMA_ENABLE
            /* Check if DMA is enabled for this */
            if(I2S->cfg->dma_enable)
            {
                /* Disable the DMA interface of I2S */
                i2s_rx_dma_disable(I2S->regs);

                /* Deallocate DMA channel */
                if(I2S_DMA_DeAllocate(&I2S->dma_cfg->dma_rx))
                    return ARM_DRIVER_ERROR;
            }
#endif
            /* Disable Rx Channel */
            i2s_rxchannel_disable(I2S->regs);

            /* Disable Rx Block */
            i2s_rxblock_disable(I2S->regs);

            /* Disable Rx Interrupt */
            i2s_disable_rx_interrupt(I2S->regs);

            /* Disable Master Clock */
            i2s_clock_disable(I2S->regs);

            /* Set the rx flags*/
            I2S->drv_status.status_b.rx_busy = 0U;

        }
        else
            return ARM_DRIVER_ERROR;

        return ARM_DRIVER_OK;
    case ARM_SAI_ABORT_SEND:
#if I2S_DMA_ENABLE
        /* Check if DMA is enabled for this */
        if(I2S->cfg->dma_enable)
        {
            /* Stop DMA transfer */
            if(I2S_DMA_Stop(&I2S->dma_cfg->dma_tx))
                return ARM_DRIVER_ERROR;
        }
#endif
        /* Disable Tx Channel */
        i2s_txchannel_disable(I2S->regs);

        /* Disable Tx Interrupt */
        i2s_disable_tx_interrupt(I2S->regs);

        /* Reset the Tx FIFO */
        i2s_reset_tx_fifo(I2S->regs);

        /* Set the Tx flags*/
        I2S->drv_status.status_b.tx_busy = 0U;

        return ARM_DRIVER_OK;
    case ARM_SAI_ABORT_RECEIVE:
#if I2S_DMA_ENABLE
        /* Check if DMA is enabled for this */
        if(I2S->cfg->dma_enable)
        {
            /* Disable the overflow interrupt */
            i2s_disable_rx_overflow_interrupt(I2S->regs);

            /* Stop DMA transfer */
            if(I2S_DMA_Stop(&I2S->dma_cfg->dma_rx))
                return ARM_DRIVER_ERROR;
        }
#endif

        /* Disable Rx Channel */
        i2s_rxchannel_disable(I2S->regs);

        /* Disable Rx Interrupt */
        i2s_disable_rx_interrupt(I2S->regs);

        /* Reset the Rx FIFO */
        i2s_reset_rx_fifo(I2S->regs);

        /* Set the rx flags*/
        I2S->drv_status.status_b.rx_busy = 0U;

        return ARM_DRIVER_OK;
#if I2S_DMA_ENABLE
    case ARM_SAI_USE_CUSTOM_DMA_MCODE_TX:
        if(!arg1)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Use User Defined microcode for DMA */
        if(I2S_DMA_Usermcode(&I2S->dma_cfg->dma_tx, arg1))
            return ARM_DRIVER_ERROR;
        else
            return ARM_DRIVER_OK;
    case ARM_SAI_USE_CUSTOM_DMA_MCODE_RX:
        if(!arg1)
            return ARM_DRIVER_ERROR_PARAMETER;

        /* Use User Defined microcode for DMA */
        if(I2S_DMA_Usermcode(&I2S->dma_cfg->dma_rx, arg1))
            return ARM_DRIVER_ERROR;
        else
            return ARM_DRIVER_OK;
#endif
    case ARM_SAI_MASK_SLOTS_TX:
    case ARM_SAI_MASK_SLOTS_RX:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    /* Handle I2S Modes */
    if((control & ARM_SAI_MODE_Msk) != ARM_SAI_MODE_MASTER)
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    /* Handle Synchronization */
    switch(control & ARM_SAI_SYNCHRONIZATION_Msk)
    {
    case ARM_SAI_ASYNCHRONOUS:
        break;
    case ARM_SAI_SYNCHRONOUS:
        if((control & ARM_SAI_MODE_Msk) == ARM_SAI_MODE_MASTER)
            return ARM_SAI_ERROR_SYNCHRONIZATION;
        break;
    default:
        return ARM_SAI_ERROR_SYNCHRONIZATION;
    }

    /* Handle Protocol  */
    switch(control & ARM_SAI_PROTOCOL_Msk)
    {
    case ARM_SAI_PROTOCOL_I2S:
        break;
    default:
        return ARM_SAI_ERROR_PROTOCOL;
    }

    /* Handle DataSize */
    datasize = ((control & ARM_SAI_DATA_SIZE_Msk) >> ARM_SAI_DATA_SIZE_Pos) + 1;
    switch(datasize)
    {
    case 12:
        I2S->cfg->wlen    = I2S_WLEN_RES_12_BIT;
        break;
    case 16:
        I2S->cfg->wlen    = I2S_WLEN_RES_16_BIT;
        break;
    case 20:
        I2S->cfg->wlen    = I2S_WLEN_RES_20_BIT;
        break;
    case 24:
        I2S->cfg->wlen    = I2S_WLEN_RES_24_BIT;
        break;
    case 32:
        I2S->cfg->wlen    = I2S_WLEN_RES_32_BIT;
        break;

    default:
        return ARM_SAI_ERROR_DATA_SIZE;
    }

    /* Handle Bit Order */
    switch(control & ARM_SAI_BIT_ORDER_Msk)
    {
        case ARM_SAI_MSB_FIRST:
        break;

        default:
            return ARM_SAI_ERROR_BIT_ORDER;
    }

    /* Handle Mono Mode */
    if(control & ARM_SAI_MONO_MODE)
        I2S->flags |= I2S_FLAG_DRV_MONO_MODE;

    /* Handle Frame Length */
    frame_length =  ((arg1 & ARM_SAI_FRAME_LENGTH_Msk) >> ARM_SAI_FRAME_LENGTH_Pos);
    if(frame_length != (datasize * 2))
        return ARM_SAI_ERROR_FRAME_LENGTH;

    /* Handle Sample Rate */
    if(arg2 & ARM_SAI_AUDIO_FREQ_Msk)
        I2S->sample_rate = arg2 & ARM_SAI_AUDIO_FREQ_Msk;
    else
        return ARM_SAI_ERROR_AUDIO_FREQ;

    switch(control & ARM_SAI_MCLK_PIN_Msk)
    {
    case ARM_SAI_MCLK_PIN_INACTIVE:
        /* Enable internal clock source */
        select_i2s_clock_source(I2S->instance, I2S_INTERNAL_CLOCK_SOURCE);

        /* Configure the I2S serial clock */
        ret = I2S_SetSamplingRate(I2S);
        if(ret)
            return ARM_DRIVER_ERROR;

        break;

    case ARM_SAI_MCLK_PIN_OUTPUT:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
        break;

    case ARM_SAI_MCLK_PIN_INPUT:
        /* Enable external clock source */
        select_i2s_clock_source(I2S->instance, I2S_EXTERNAL_CLOCK_SOURCE);

        /* Set the MCLK clock divider */
        mclk_prescaler = ((arg2 & ARM_SAI_MCLK_PRESCALER_Msk)
                           >> ARM_SAI_MCLK_PRESCALER_Pos);

        if(mclk_prescaler > I2S_CLK_DIVISOR_MAX)
            return ARM_SAI_ERROR_MCLK_PRESCALER;

        if(mclk_prescaler < I2S_CLK_DIVISOR_MIN)
            bypass_i2s_clock_divider(I2S->instance);
        else
            set_i2s_clock_divisor(I2S->instance, mclk_prescaler);

        break;

    default:
        return ARM_SAI_ERROR_MCLK_PIN;
    }

    /* Unsupported Codes */
    if((control & ARM_SAI_COMPANDING_Msk)
        || (control & ARM_SAI_CLOCK_POLARITY_Msk))
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    return ARM_DRIVER_OK;
}

/**
  \fn          ARM_SAI_STATUS I2S_GetStatus(I2S_RESOURCES *I2S)
  \brief       Get I2S status.
  \param[in]   I2S  Pointer to I2S resources
  \return      \ref ARM_SAI_STATUS
*/
static ARM_SAI_STATUS I2S_GetStatus(I2S_RESOURCES *I2S)
{
    return I2S->drv_status.status_b;
}

/**
  \fn          void I2S_IRQHandler(I2S_RESOURCES *I2S)
  \brief       Run the IRQ Handler
  \param[in]   I2S  Pointer to I2S resources
*/
static void I2S_IRQHandler(I2S_RESOURCES *I2S)
{
    i2s_transfer_t *transfer = &I2S->transfer;

    if(I2S->drv_status.status_b.tx_busy)
    {
        i2s_tx_irq_handler(I2S->regs, transfer);

        if(transfer->status & I2S_TRANSFER_STATUS_TX_COMPLETE)
        {
            I2S->drv_status.status_b.tx_busy = 0U;
            transfer->status &= ~I2S_TRANSFER_STATUS_TX_COMPLETE;
            I2S->cb_event(ARM_SAI_EVENT_SEND_COMPLETE);
        }
    }

    if(I2S->drv_status.status_b.rx_busy)
    {
        i2s_rx_irq_handler(I2S->regs, transfer);

        if(transfer->status & I2S_TRANSFER_STATUS_RX_COMPLETE)
        {
            I2S->drv_status.status_b.rx_busy = 0U;
            transfer->status &= ~I2S_TRANSFER_STATUS_RX_COMPLETE;
            I2S->cb_event(ARM_SAI_EVENT_RECEIVE_COMPLETE);
        }

        if(transfer->status & I2S_TRANSFER_STATUS_RX_OVERFLOW)
        {
            /* Send event to application to handle it */
            transfer->status &= ~I2S_TRANSFER_STATUS_RX_OVERFLOW;
            I2S->drv_status.status_b.rx_overflow = 1U;
            I2S->cb_event(ARM_SAI_EVENT_RX_OVERFLOW);
        }
    }
}

#if I2S_DMA_ENABLE
/**
  \fn          static void  I2S_DMACallback(uint32_t event, int8_t peri_num,
                                            I2S_RESOURCES *I2S)
  \brief       Callback function from DMA for I2S
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \param[in]   I2S       Pointer to I2S resources
*/
static void I2S_DMACallback(uint32_t event, int8_t peri_num,
                            I2S_RESOURCES *I2S)
{
    if(!I2S->cb_event)
        return;

    /* Transfer Completed */
    if(event & ARM_DMA_EVENT_COMPLETE)
    {
        switch(peri_num)
        {
        case I2S0_DMA_TX_PERIPH_REQ:
        case I2S1_DMA_TX_PERIPH_REQ:
        case I2S2_DMA_TX_PERIPH_REQ:
        case I2S3_DMA_TX_PERIPH_REQ:
#if defined (M55_HE)
        case LPI2S_DMA_TX_PERIPH_REQ:
#endif
            /* Set the Tx flags*/
            I2S->drv_status.status_b.tx_busy = 0U;
            I2S->cb_event(ARM_SAI_EVENT_SEND_COMPLETE);
            break;

        case I2S0_DMA_RX_PERIPH_REQ:
        case I2S1_DMA_RX_PERIPH_REQ:
        case I2S2_DMA_RX_PERIPH_REQ:
        case I2S3_DMA_RX_PERIPH_REQ:
#if defined (M55_HE)
        case LPI2S_DMA_RX_PERIPH_REQ:
#endif
            /* Set the Rx flags*/
            I2S->drv_status.status_b.rx_busy = 0U;

            /* Disable the Overflow interrupt */
            i2s_disable_rx_overflow_interrupt(I2S->regs);

            I2S->cb_event(ARM_SAI_EVENT_RECEIVE_COMPLETE);
            break;

        default:
            break;
        }
    }

    /* Abort Occurred */
    if(event & ARM_DMA_EVENT_ABORT)
    {
        /*
        * There is no event for indicating error in SAI driver.
        * Let the application get timeout and restart the I2S.
        *
        */
    }
}
#endif

#if (RTE_I2S0)

static I2S_CONFIG_INFO I2S0_CONFIG = {
    .rx_fifo_trg_lvl     = RTE_I2S0_RX_TRIG_LVL,
    .tx_fifo_trg_lvl     = RTE_I2S0_TX_TRIG_LVL,
    .irq_priority        = RTE_I2S0_IRQ_PRI,
#if RTE_I2S0_BLOCKING_MODE_ENABLE
    .blocking_mode       = RTE_I2S0_BLOCKING_MODE_ENABLE,
#endif
#if RTE_I2S0_DMA_ENABLE
    .dma_enable          = RTE_I2S0_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2S0_DMA_IRQ_PRI,
#endif
    .clk_source          = I2S_CLK_SOURCE_76P8M_IN_HZ,
};

#if RTE_I2S0_DMA_ENABLE
static void I2S0_DMACallback(uint32_t event, int8_t peri_num);
static I2S_DMA_HW_CONFIG I2S0_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S0_DMA),
        .dma_periph_req = I2S0_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S0_DMA,
             .group    = I2S0_DMA_GROUP,
             .channel  = I2S0_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2S0_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S0_DMA),
        .dma_periph_req = I2S0_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S0_DMA,
             .group    = I2S0_DMA_GROUP,
             .channel  = I2S0_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2S0_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

static I2S_RESOURCES I2S0 = {
    .cb_event  = NULL,
    .cfg       = &I2S0_CONFIG,
#if RTE_I2S0_DMA_ENABLE
    .dma_cb    = I2S0_DMACallback,
    .dma_cfg   = &I2S0_DMA_HW_CONFIG,
#endif
    .regs      = (I2S_Type *) I2S0_BASE,
    .instance  =  I2S_INSTANCE_0,
    .irq       = (IRQn_Type) I2S0_IRQ_IRQn,
    .flags     = 0,
};

/**
  \fn          int32_t I2S0_Initialize(ARM_SAI_SignalEvent_t cb_event)
  \brief       Initialize I2S Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SAI_SignalEvent
  \return      \ref      execution_status
*/
static int32_t I2S0_Initialize(ARM_SAI_SignalEvent_t cb_event)
{
    I2S_RESOURCES *I2S = &I2S0;

    I2S->cfg->wss_len = I2S_GetWordSelectSize(RTE_I2S0_WSS_CLOCK_CYCLES);
    I2S->cfg->sclkg   = I2S_GetClockGatingCycles(RTE_I2S0_SCLKG_CLOCK_CYCLES);

    return I2S_Initialize(cb_event, &I2S0);
}

/**
  \fn          int32_t I2S0_Uninitialize(void)
  \brief       De-initialize I2S Interface.
  \return      \ref execution_status
*/
static int32_t I2S0_Uninitialize(void)
{
    return I2S_Uninitialize(&I2S0);
}

/**
  \fn          int32_t I2S0_PowerControl(ARM_POWER_STATE state)
  \brief       Control I2S Interface Power.
  \param[in]   state  Power state
  \return      \ref   execution_status
*/
static int32_t I2S0_PowerControl(ARM_POWER_STATE state)
{
    return I2S_PowerControl(state, &I2S0);
}

/**
  \fn          int32_t I2S0_Send(const void *data, uint32_t num)
  \brief       Start sending data to I2S transmitter.
  \param[in]   data  Pointer to buffer with data to send to I2S transmitter
  \param[in]   num   Number of data items to send
  \return      \ref  execution_status
*/
static int32_t I2S0_Send(const void *data, uint32_t num)
{
    return I2S_Send(data, num, &I2S0);
}

/**
  \fn          int32_t I2S0_Receive(void *data, uint32_t num)
  \brief       Start receiving data from I2S receiver.
  \param[out]  data  Pointer to buffer for data to receive from I2S receiver
  \param[in]   num   Number of data items to receive
  \return      \ref  execution_status
*/
static int32_t I2S0_Receive(void *data, uint32_t num)
{
    return I2S_Receive (data, num, &I2S0);
}

/**
  \fn          uint32_t I2S0_GetTxCount(void)
  \brief       Get transmitted data count.
  \return      number of data items transmitted
*/
static uint32_t I2S0_GetTxCount(void)
{
    return I2S_GetTxCount(&I2S0);
}

/**
  \fn          uint32_t I2S0_GetRxCount(void)
  \brief       Get received data count.
  \return      number of data items received
*/
static uint32_t I2S0_GetRxCount(void)
{
    return I2S_GetRxCount(&I2S0);
}

/**
  \fn          int32_t I2S0_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg1     Argument 1 of operation (optional)
  \param[in]   arg2     Argument 2 of operation (optional)
  \return      common \ref execution_status and driver specific \ref sai_execution_status
*/
static int32_t I2S0_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return I2S_Control(control, arg1, arg2, &I2S0);
}

/**
  \fn          ARM_SAI_STATUS I2S0_GetStatus(void)
  \brief       Get I2S status.
  \return      SAI status \ref ARM_SAI_STATUS
*/
static ARM_SAI_STATUS I2S0_GetStatus(void)
{
    return I2S_GetStatus(&I2S0);
}

/**
  \fn          void  I2S0_IRQHandler (void)
  \brief       Run the IRQ Handler for I2S0
*/
void I2S0_IRQHandler(void)
{
    I2S_IRQHandler(&I2S0);
}

#if RTE_I2S0_DMA_ENABLE
/**
  \fn          static void  I2S0_DMACallback (uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for I2S0
*/
static void I2S0_DMACallback(uint32_t event, int8_t peri_num)
{
    I2S_DMACallback(event, peri_num, &I2S0);
}
#endif

/**
\brief Access structure of the I2S0 Driver.
*/
extern \
ARM_DRIVER_SAI Driver_SAI0;
ARM_DRIVER_SAI Driver_SAI0 = {
    I2S_GetVersion,
    I2S_GetCapabilities,
    I2S0_Initialize,
    I2S0_Uninitialize,
    I2S0_PowerControl,
    I2S0_Send,
    I2S0_Receive,
    I2S0_GetTxCount,
    I2S0_GetRxCount,
    I2S0_Control,
    I2S0_GetStatus
};
#endif //RTE_I2S0

#if (RTE_I2S1)

static I2S_CONFIG_INFO I2S1_CONFIG = {
    .rx_fifo_trg_lvl     = RTE_I2S1_RX_TRIG_LVL,
    .tx_fifo_trg_lvl     = RTE_I2S1_TX_TRIG_LVL,
    .irq_priority        = RTE_I2S1_IRQ_PRI,
#if RTE_I2S1_BLOCKING_MODE_ENABLE
    .blocking_mode       = RTE_I2S1_BLOCKING_MODE_ENABLE,
#endif
#if RTE_I2S1_DMA_ENABLE
    .dma_enable          = RTE_I2S1_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2S1_DMA_IRQ_PRI,
#endif
    .clk_source          = I2S_CLK_SOURCE_76P8M_IN_HZ,
};

#if RTE_I2S1_DMA_ENABLE
static void I2S1_DMACallback (uint32_t event, int8_t peri_num);
static I2S_DMA_HW_CONFIG I2S1_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S1_DMA),
        .dma_periph_req = I2S1_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S1_DMA,
             .group    = I2S1_DMA_GROUP,
             .channel  = I2S1_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2S1_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S1_DMA),
        .dma_periph_req = I2S1_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S1_DMA,
             .group    = I2S1_DMA_GROUP,
             .channel  = I2S1_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2S1_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

static I2S_RESOURCES I2S1 = {
    .cb_event  = NULL,
    .cfg       = &I2S1_CONFIG,
#if RTE_I2S1_DMA_ENABLE
    .dma_cb    = I2S1_DMACallback,
    .dma_cfg   = &I2S1_DMA_HW_CONFIG,
#endif
    .regs      = (I2S_Type *) I2S1_BASE,
    .instance  =  I2S_INSTANCE_1,
    .irq       = (IRQn_Type) I2S1_IRQ_IRQn,
    .flags     = 0,
};

/**
  \fn          int32_t I2S1_Initialize(ARM_SAI_SignalEvent_t cb_event)
  \brief       Initialize I2S Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SAI_SignalEvent
  \return      \ref      execution_status
*/
static int32_t I2S1_Initialize(ARM_SAI_SignalEvent_t cb_event)
{
    I2S_RESOURCES *I2S = &I2S1;

    I2S->cfg->wss_len = I2S_GetWordSelectSize(RTE_I2S1_WSS_CLOCK_CYCLES);
    I2S->cfg->sclkg   = I2S_GetClockGatingCycles(RTE_I2S1_SCLKG_CLOCK_CYCLES);

    return I2S_Initialize(cb_event, &I2S1);
}

/**
  \fn          int32_t I2S1_Uninitialize(void)
  \brief       De-initialize I2S Interface.
  \return      \ref execution_status
*/
static int32_t I2S1_Uninitialize(void)
{
    return I2S_Uninitialize (&I2S1);
}

/**
  \fn          int32_t I2S1_PowerControl(ARM_POWER_STATE state)
  \brief       Control I2S Interface Power.
  \param[in]   state  Power state
  \return      \ref   execution_status
*/
static int32_t I2S1_PowerControl(ARM_POWER_STATE state)
{
    return I2S_PowerControl(state, &I2S1);
}

/**
  \fn          int32_t I2S1_Send(const void *data, uint32_t num)
  \brief       Start sending data to I2S transmitter.
  \param[in]   data  Pointer to buffer with data to send to I2S transmitter
  \param[in]   num   Number of data items to send
  \return      \ref  execution_status
*/
static int32_t I2S1_Send(const void *data, uint32_t num)
{
    return I2S_Send(data, num, &I2S1);
}

/**
  \fn          int32_t I2S1_Receive(void *data, uint32_t num)
  \brief       Start receiving data from I2S receiver.
  \param[out]  data  Pointer to buffer for data to receive from I2S receiver
  \param[in]   num   Number of data items to receive
  \return      \ref  execution_status
*/
static int32_t I2S1_Receive(void *data, uint32_t num)
{
    return I2S_Receive(data, num, &I2S1);
}

/**
  \fn          uint32_t I2S1_GetTxCount(void)
  \brief       Get transmitted data count.
  \return      number of data items transmitted
*/
static uint32_t I2S1_GetTxCount(void)
{
    return I2S_GetTxCount(&I2S1);
}

/**
  \fn          uint32_t I2S1_GetRxCount(void)
  \brief       Get received data count.
  \return      number of data items received
*/
static uint32_t I2S1_GetRxCount(void)
{
    return I2S_GetRxCount(&I2S1);
}

/**
  \fn          int32_t I2S1_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg1     Argument 1 of operation (optional)
  \param[in]   arg2     Argument 2 of operation (optional)
  \return      common \ref execution_status and driver specific \ref sai_execution_status
*/
static int32_t I2S1_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return I2S_Control(control, arg1, arg2, &I2S1);
}

/**
  \fn          ARM_SAI_STATUS I2S1_GetStatus(void)
  \brief       Get I2S status.
  \return      SAI status \ref ARM_SAI_STATUS
*/
static ARM_SAI_STATUS I2S1_GetStatus(void)
{
    return I2S_GetStatus (&I2S1);
}

/**
  \fn          void  I2S1_IRQHandler(void)
  \brief       Run the IRQ Handler for I2S1
*/
void I2S1_IRQHandler(void)
{
    I2S_IRQHandler(&I2S1);
}

#if RTE_I2S1_DMA_ENABLE
/**
  \fn          void  I2S1_DMACallback(uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for I2S1
*/
void I2S1_DMACallback(uint32_t event, int8_t peri_num)
{
    I2S_DMACallback(event, peri_num, &I2S1);
}
#endif

/**
\brief Access structure of the I2S1 Driver.
*/
extern \
ARM_DRIVER_SAI Driver_SAI1;
ARM_DRIVER_SAI Driver_SAI1 = {
    I2S_GetVersion,
    I2S_GetCapabilities,
    I2S1_Initialize,
    I2S1_Uninitialize,
    I2S1_PowerControl,
    I2S1_Send,
    I2S1_Receive,
    I2S1_GetTxCount,
    I2S1_GetRxCount,
    I2S1_Control,
    I2S1_GetStatus
};
#endif //RTE_I2S1

#if (RTE_I2S2)

static I2S_CONFIG_INFO I2S2_CONFIG = {
    .rx_fifo_trg_lvl     = RTE_I2S2_RX_TRIG_LVL,
    .tx_fifo_trg_lvl     = RTE_I2S2_TX_TRIG_LVL,
    .irq_priority        = RTE_I2S2_IRQ_PRI,
#if RTE_I2S2_BLOCKING_MODE_ENABLE
    .blocking_mode       = RTE_I2S2_BLOCKING_MODE_ENABLE,
#endif
#if RTE_I2S2_DMA_ENABLE
    .dma_enable          = RTE_I2S2_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2S2_DMA_IRQ_PRI,
#endif
    .clk_source          = I2S_CLK_SOURCE_76P8M_IN_HZ,
};

#if RTE_I2S2_DMA_ENABLE
static void I2S2_DMACallback (uint32_t event, int8_t peri_num);
static I2S_DMA_HW_CONFIG I2S2_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S2_DMA),
        .dma_periph_req = I2S2_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S2_DMA,
             .group    = I2S2_DMA_GROUP,
             .channel  = I2S2_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2S2_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S2_DMA),
        .dma_periph_req = I2S2_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S2_DMA,
             .group    = I2S2_DMA_GROUP,
             .channel  = I2S2_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2S2_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

static I2S_RESOURCES I2S2 = {
    .cb_event  = NULL,
    .cfg       = &I2S2_CONFIG,
#if RTE_I2S2_DMA_ENABLE
    .dma_cb    = I2S2_DMACallback,
    .dma_cfg   = &I2S2_DMA_HW_CONFIG,
#endif
    .regs      = (I2S_Type *) I2S2_BASE,
    .instance  =  I2S_INSTANCE_2,
    .irq       = (IRQn_Type) I2S2_IRQ_IRQn,
    .flags     = 0,
};


/**
  \fn          int32_t I2S2_Initialize(ARM_SAI_SignalEvent_t cb_event)
  \brief       Initialize I2S Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SAI_SignalEvent
  \return      \ref      execution_status
*/
static int32_t I2S2_Initialize(ARM_SAI_SignalEvent_t cb_event)
{
    I2S_RESOURCES *I2S = &I2S2;

    I2S->cfg->wss_len = I2S_GetWordSelectSize(RTE_I2S2_WSS_CLOCK_CYCLES);
    I2S->cfg->sclkg   = I2S_GetClockGatingCycles(RTE_I2S2_SCLKG_CLOCK_CYCLES);

    return I2S_Initialize(cb_event, &I2S2);
}

/**
  \fn          int32_t I2S2_Uninitialize(void)
  \brief       De-initialize I2S Interface.
  \return      \ref execution_status
*/
static int32_t I2S2_Uninitialize(void)
{
    return I2S_Uninitialize(&I2S2);
}

/**
  \fn          int32_t I2S2_PowerControl(ARM_POWER_STATE state)
  \brief       Control I2S Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t I2S2_PowerControl(ARM_POWER_STATE state)
{
    return I2S_PowerControl(state, &I2S2);
}

/**
  \fn          int32_t I2S2_Send(const void *data, uint32_t num)
  \brief       Start sending data to I2S transmitter.
  \param[in]   data  Pointer to buffer with data to send to I2S transmitter
  \param[in]   num   Number of data items to send
  \return      \ref  execution_status
*/
static int32_t I2S2_Send(const void *data, uint32_t num)
{
    return I2S_Send(data, num, &I2S2);
}

/**
  \fn          int32_t I2S2_Receive(void *data, uint32_t num)
  \brief       Start receiving data from I2S receiver.
  \param[out]  data  Pointer to buffer for data to receive from I2S receiver
  \param[in]   num   Number of data items to receive
  \return      \ref execution_status
*/
static int32_t I2S2_Receive(void *data, uint32_t num)
{
    return I2S_Receive(data, num, &I2S2);
}

/**
  \fn          uint32_t I2S2_GetTxCount(void)
  \brief       Get transmitted data count.
  \return      number of data items transmitted
*/
static uint32_t I2S2_GetTxCount(void)
{
    return I2S_GetTxCount(&I2S2);
}

/**
  \fn          uint32_t I2S2_GetRxCount(void)
  \brief       Get received data count.
  \return      number of data items received
*/
static uint32_t I2S2_GetRxCount(void)
{
    return I2S_GetRxCount(&I2S2);
}

/**
  \fn          int32_t I2S2_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg1     Argument 1 of operation (optional)
  \param[in]   arg2     Argument 2 of operation (optional)
  \return      common \ref execution_status and driver specific \ref sai_execution_status
*/
static int32_t I2S2_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return I2S_Control(control, arg1, arg2, &I2S2);
}

/**
  \fn          ARM_SAI_STATUS I2S2_GetStatus(void)
  \brief       Get I2S status.
  \return      SAI status \ref ARM_SAI_STATUS
*/
static ARM_SAI_STATUS I2S2_GetStatus(void)
{
    return I2S_GetStatus(&I2S2);
}

/**
  \fn          void  I2S2_IRQHandler (void)
  \brief       Run the IRQ Handler for I2S2
*/
void I2S2_IRQHandler(void)
{
    I2S_IRQHandler(&I2S2);
}

#if RTE_I2S2_DMA_ENABLE
/**
  \fn          void  I2S2_DMACallback(uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for I2S2
*/
void I2S2_DMACallback(uint32_t event, int8_t peri_num)
{
    I2S_DMACallback(event, peri_num, &I2S2);
}
#endif

/**
\brief Access structure of the I2S2 Driver.
*/
extern \
ARM_DRIVER_SAI Driver_SAI2;
ARM_DRIVER_SAI Driver_SAI2 = {
    I2S_GetVersion,
    I2S_GetCapabilities,
    I2S2_Initialize,
    I2S2_Uninitialize,
    I2S2_PowerControl,
    I2S2_Send,
    I2S2_Receive,
    I2S2_GetTxCount,
    I2S2_GetRxCount,
    I2S2_Control,
    I2S2_GetStatus
};
#endif //RTE_I2S2

#if (RTE_I2S3)

static I2S_CONFIG_INFO I2S3_CONFIG = {
    .rx_fifo_trg_lvl     = RTE_I2S3_RX_TRIG_LVL,
    .tx_fifo_trg_lvl     = RTE_I2S3_TX_TRIG_LVL,
    .irq_priority        = RTE_I2S3_IRQ_PRI,
#if RTE_I2S3_BLOCKING_MODE_ENABLE
    .blocking_mode       = RTE_I2S3_BLOCKING_MODE_ENABLE,
#endif
#if RTE_I2S3_DMA_ENABLE
    .dma_enable          = RTE_I2S3_DMA_ENABLE,
    .dma_irq_priority    = RTE_I2S3_DMA_IRQ_PRI,
#endif
    .clk_source          = I2S_CLK_SOURCE_76P8M_IN_HZ,
};

#if RTE_I2S3_DMA_ENABLE
static void I2S3_DMACallback(uint32_t event, int8_t peri_num);
static I2S_DMA_HW_CONFIG I2S3_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S3_DMA),
        .dma_periph_req = I2S3_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S3_DMA,
             .group    = I2S3_DMA_GROUP,
             .channel  = I2S3_DMA_RX_PERIPH_REQ,
             .enable_handshake = I2S3_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(I2S3_DMA),
        .dma_periph_req = I2S3_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = I2S3_DMA,
             .group    = I2S3_DMA_GROUP,
             .channel  = I2S3_DMA_TX_PERIPH_REQ,
             .enable_handshake = I2S3_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

static I2S_RESOURCES I2S3 = {
    .cb_event  = NULL,
    .cfg       = &I2S3_CONFIG,
#if RTE_I2S3_DMA_ENABLE
    .dma_cb    = I2S3_DMACallback,
    .dma_cfg   = &I2S3_DMA_HW_CONFIG,
#endif
    .regs      = (I2S_Type *) I2S3_BASE,
    .instance  =  I2S_INSTANCE_3,
    .irq       = (IRQn_Type) I2S3_IRQ_IRQn,
    .flags     = 0,
};


/**
  \fn          int32_t I2S3_Initialize(ARM_SAI_SignalEvent_t cb_event)
  \brief       Initialize I2S Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SAI_SignalEvent
  \return      \ref      execution_status
*/
static int32_t I2S3_Initialize(ARM_SAI_SignalEvent_t cb_event)
{
    I2S_RESOURCES *I2S = &I2S3;

    I2S->cfg->wss_len = I2S_GetWordSelectSize(RTE_I2S3_WSS_CLOCK_CYCLES);
    I2S->cfg->sclkg   = I2S_GetClockGatingCycles(RTE_I2S3_SCLKG_CLOCK_CYCLES);

    return I2S_Initialize(cb_event, &I2S3);
}

/**
  \fn          int32_t I2S3_Uninitialize(void)
  \brief       De-initialize I2S Interface.
  \return      \ref execution_status
*/
static int32_t I2S3_Uninitialize(void)
{
    return I2S_Uninitialize(&I2S3);
}

/**
  \fn          int32_t I2S3_PowerControl(ARM_POWER_STATE state)
  \brief       Control I2S Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t I2S3_PowerControl(ARM_POWER_STATE state)
{
    return I2S_PowerControl(state, &I2S3);
}

/**
  \fn          int32_t I2S3_Send(const void *data, uint32_t num)
  \brief       Start sending data to I2S transmitter.
  \param[in]   data  Pointer to buffer with data to send to I2S transmitter
  \param[in]   num   Number of data items to send
  \return      \ref  execution_status
*/
static int32_t I2S3_Send(const void *data, uint32_t num)
{
    return I2S_Send(data, num, &I2S3);
}

/**
  \fn          int32_t I2S3_Receive(void *data, uint32_t num)
  \brief       Start receiving data from I2S receiver.
  \param[out]  data  Pointer to buffer for data to receive from I2S receiver
  \param[in]   num   Number of data items to receive
  \return      \ref  execution_status
*/
static int32_t I2S3_Receive(void *data, uint32_t num)
{
    return I2S_Receive(data, num, &I2S3);
}

/**
  \fn          uint32_t I2S3_GetTxCount(void)
  \brief       Get transmitted data count.
  \return      number of data items transmitted
*/
static uint32_t I2S3_GetTxCount(void)
{
    return I2S_GetTxCount(&I2S3);
}

/**
  \fn          uint32_t I2S3_GetRxCount(void)
  \brief       Get received data count.
  \return      number of data items received
*/
static uint32_t I2S3_GetRxCount(void)
{
    return I2S_GetRxCount(&I2S3);
}

/**
  \fn          int32_t I2S3_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg1     Argument 1 of operation (optional)
  \param[in]   arg2     Argument 2 of operation (optional)
  \return      common \ref execution_status and driver specific \ref sai_execution_status
*/
static int32_t I2S3_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return I2S_Control(control, arg1, arg2, &I2S3);
}

/**
  \fn          ARM_SAI_STATUS I2S3_GetStatus(void)
  \brief       Get I2S status.
  \return      SAI status \ref ARM_SAI_STATUS
*/
static ARM_SAI_STATUS I2S3_GetStatus(void)
{
    return I2S_GetStatus(&I2S3);
}

/**
  \fn          void  I2S3_IRQHandler(void)
  \brief       Run the IRQ Handler for I2S3
*/
void I2S3_IRQHandler(void)
{
    I2S_IRQHandler(&I2S3);
}

#if RTE_I2S3_DMA_ENABLE
/**
  \fn          void  I2S3_DMACallback(uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for I2S3
*/
void I2S3_DMACallback(uint32_t event, int8_t peri_num)
{
    I2S_DMACallback(event, peri_num, &I2S3);
}
#endif

/**
\brief Access structure of the I2S3 Driver.
*/
extern \
ARM_DRIVER_SAI Driver_SAI3;
ARM_DRIVER_SAI Driver_SAI3 = {
    I2S_GetVersion,
    I2S_GetCapabilities,
    I2S3_Initialize,
    I2S3_Uninitialize,
    I2S3_PowerControl,
    I2S3_Send,
    I2S3_Receive,
    I2S3_GetTxCount,
    I2S3_GetRxCount,
    I2S3_Control,
    I2S3_GetStatus
};
#endif //RTE_I2S3

#if (RTE_LPI2S)

static I2S_CONFIG_INFO LPI2S_CONFIG = {
    .rx_fifo_trg_lvl     = RTE_LPI2S_RX_TRIG_LVL,
    .tx_fifo_trg_lvl     = RTE_LPI2S_TX_TRIG_LVL,
    .irq_priority        = RTE_LPI2S_IRQ_PRI,
#if RTE_LPI2S_BLOCKING_MODE_ENABLE
    .blocking_mode       = RTE_LPI2S_BLOCKING_MODE_ENABLE,
#endif
#if RTE_LPI2S_DMA_ENABLE
    .dma_enable          = RTE_LPI2S_DMA_ENABLE,
    .dma_irq_priority    = RTE_LPI2S_DMA_IRQ_PRI,
#endif
    .clk_source          = I2S_CLK_SOURCE_76P8M_IN_HZ,
};

#if RTE_LPI2S_DMA_ENABLE
static void LPI2S_DMACallback(uint32_t event, int8_t peri_num);
static I2S_DMA_HW_CONFIG LPI2S_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(LPI2S_DMA),
        .dma_periph_req = LPI2S_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = LPI2S_DMA,
             .group    = LPI2S_DMA_GROUP,
             .channel  = LPI2S_DMA_RX_PERIPH_REQ,
             .enable_handshake = LPI2S_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(LPI2S_DMA),
        .dma_periph_req = LPI2S_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = LPI2S_DMA,
             .group    = LPI2S_DMA_GROUP,
             .channel  = LPI2S_DMA_TX_PERIPH_REQ,
             .enable_handshake = LPI2S_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

static I2S_RESOURCES LPI2S = {
    .cb_event  = NULL,
    .cfg       = &LPI2S_CONFIG,
#if RTE_LPI2S_DMA_ENABLE
    .dma_cb    = LPI2S_DMACallback,
    .dma_cfg   = &LPI2S_DMA_HW_CONFIG,
#endif
    .regs      = (I2S_Type *) LPI2S_BASE,
    .instance  = I2S_INSTANCE_LP,
    .irq       = (IRQn_Type) LPI2S_IRQ_IRQn,
    .flags     = 0,
};


/**
  \fn          int32_t LPI2S_Initialize(ARM_SAI_SignalEvent_t cb_event)
  \brief       Initialize I2S Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SAI_SignalEvent
  \return      \ref      execution_status
*/
static int32_t LPI2S_Initialize(ARM_SAI_SignalEvent_t cb_event)
{
    I2S_RESOURCES *I2S = &LPI2S;

    I2S->cfg->wss_len = I2S_GetWordSelectSize(RTE_LPI2S_WSS_CLOCK_CYCLES);
    I2S->cfg->sclkg   = I2S_GetClockGatingCycles(RTE_LPI2S_SCLKG_CLOCK_CYCLES);

    return I2S_Initialize(cb_event, &LPI2S);
}

/**
  \fn          int32_t LPI2S_Uninitialize(void)
  \brief       De-initialize I2S Interface.
  \return      \ref execution_status
*/
static int32_t LPI2S_Uninitialize(void)
{
    return I2S_Uninitialize(&LPI2S);
}

/**
  \fn          int32_t LPI2S_PowerControl(ARM_POWER_STATE state)
  \brief       Control I2S Interface Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t LPI2S_PowerControl(ARM_POWER_STATE state)
{
    return I2S_PowerControl(state, &LPI2S);
}

/**
  \fn          int32_t LPI2S_Send(const void *data, uint32_t num)
  \brief       Start sending data to I2S transmitter.
  \param[in]   data  Pointer to buffer with data to send to I2S transmitter
  \param[in]   num   Number of data items to send
  \return      \ref  execution_status
*/
static int32_t LPI2S_Send(const void *data, uint32_t num)
{
    return I2S_Send(data, num, &LPI2S);
}

/**
  \fn          int32_t LPI2S_Receive(void *data, uint32_t num)
  \brief       Start receiving data from I2S receiver.
  \param[out]  data  Pointer to buffer for data to receive from I2S receiver
  \param[in]   num   Number of data items to receive
  \return      \ref  execution_status
*/
static int32_t LPI2S_Receive(void *data, uint32_t num)
{
    return I2S_Receive(data, num, &LPI2S);
}

/**
  \fn          uint32_t LPI2S_GetTxCount(void)
  \brief       Get transmitted data count.
  \return      number of data items transmitted
*/
static uint32_t LPI2S_GetTxCount(void)
{
    return I2S_GetTxCount(&LPI2S);
}

/**
  \fn          uint32_t LPI2S_GetRxCount(void)
  \brief       Get received data count.
  \return      number of data items received
*/
static uint32_t LPI2S_GetRxCount(void)
{
    return I2S_GetRxCount(&LPI2S);
}

/**
  \fn          int32_t LPI2S_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg1     Argument 1 of operation (optional)
  \param[in]   arg2     Argument 2 of operation (optional)
  \return      common \ref execution_status and driver specific \ref sai_execution_status
*/
static int32_t LPI2S_Control(uint32_t control, uint32_t arg1, uint32_t arg2)
{
    return I2S_Control(control, arg1, arg2, &LPI2S);
}

/**
  \fn          ARM_SAI_STATUS LPI2S_GetStatus(void)
  \brief       Get I2S status.
  \return      SAI status \ref ARM_SAI_STATUS
*/
static ARM_SAI_STATUS LPI2S_GetStatus(void)
{
    return I2S_GetStatus(&LPI2S);
}

/**
  \fn          void  LPI2S_IRQHandler(void)
  \brief       Run the IRQ Handler for LPI2S
*/
void LPI2S_IRQHandler(void)
{
    I2S_IRQHandler(&LPI2S);
}

#if RTE_LPI2S_DMA_ENABLE
/**
  \fn          void  LPI2S_DMACallback(uint32_t event, int8_t peri_num)
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \brief       Callback function from DMA for LPI2S
*/
void LPI2S_DMACallback(uint32_t event, int8_t peri_num)
{
    I2S_DMACallback(event, peri_num, &LPI2S);
}
#endif

/**
\brief Access structure of the Low Power I2S Driver.
*/
extern \
ARM_DRIVER_SAI Driver_SAILP;
ARM_DRIVER_SAI Driver_SAILP = {
    I2S_GetVersion,
    I2S_GetCapabilities,
    LPI2S_Initialize,
    LPI2S_Uninitialize,
    LPI2S_PowerControl,
    LPI2S_Send,
    LPI2S_Receive,
    LPI2S_GetTxCount,
    LPI2S_GetRxCount,
    LPI2S_Control,
    LPI2S_GetStatus
};
#endif //RTE_LPI2S
