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
 * @file     Driver_SPI.c
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     20-04-2023
 * @brief    CMSIS-Driver for SPI.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_SPI.h"
#include "Driver_SPI_Private.h"
#include "sys_ctrl_spi.h"
#include "spi.h"

#if !((RTE_SPI0) || (RTE_SPI1) || (RTE_SPI2) || (RTE_SPI3) || (RTE_LPSPI))
#error "SPI is not enabled in the RTE_Device.h"
#endif

#if !defined(RTE_Drivers_SPI)
#error "SPI is not enabled in the RTE_Components.h"
#endif

#define ARM_SPI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_SPI_API_VERSION,
    ARM_SPI_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_SPI_CAPABILITIES DriverCapabilities = {
    0, /* Reserved (must be zero) */
    0, /* TI Synchronous Serial Interface */
    1, /* Microwire Interface */
    0, /* Signal Mode Fault event: \ref ARM_SPI_EVENT_MODE_FAULT */
    0  /* Reserved (must be zero) */
};

/**
 * @fn      ARM_DRIVER_VERSION ARM_SPI_GetVersion(void)
 * @brief   get spi version
 * @note    none
 * @param   none
 * @retval  driver version
 */
__STATIC_INLINE ARM_DRIVER_VERSION ARM_SPI_GetVersion(void)
{
    return DriverVersion;
}

/**
 * @fn      ARM_SPI_CAPABILITIES ARM_SPI_GetCapabilities(void)
 * @brief   get spi capabilities
 * @note    none
 * @param   none
 * @retval  driver capabilities
 */
__STATIC_INLINE ARM_SPI_CAPABILITIES ARM_SPI_GetCapabilities(void)
{
    return DriverCapabilities;
}

#if SPI_DMA_ENABLE
/**
  \fn          int32_t SPI_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for SPI
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
static inline int32_t SPI_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->Initialize();
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPI_DMA_PowerControl(DMA_PERIPHERAL_CONFIG *dma_periph, ARM_POWER_STATE state)
  \brief       PowerControl DMA for SPI
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
static inline int32_t SPI_DMA_PowerControl(DMA_PERIPHERAL_CONFIG *dma_periph, ARM_POWER_STATE state)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Initializes DMA interface */
    status = dma_drv->PowerControl(state);
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPI_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for SPI
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
static inline int32_t SPI_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Allocate handle for peripheral */
    status = dma_drv->Allocate(&dma_periph->dma_handle);
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Enable the channel in the Event Router */
    if (dma_periph->evtrtr_cfg.instance == 0)
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
  \fn          int32_t SPI_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of SPI
  \param[in]   dma_periph  Pointer to DMA resources
  \return      \ref        execution_status
*/
static inline int32_t SPI_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* De-Allocate handle  */
    status = dma_drv->DeAllocate(&dma_periph->dma_handle);
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Disable the channel in the Event Router */
    if (dma_periph->evtrtr_cfg.instance == 0)
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
  \fn          int32_t SPI_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                     ARM_DMA_PARAMS *dma_params)
  \brief       Start SPI DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref           execution_status
*/
static inline int32_t SPI_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph, ARM_DMA_PARAMS *dma_params)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Start transfer */
    status = dma_drv->Start(&dma_periph->dma_handle, dma_params);
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPI_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop SPI DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
static inline int32_t SPI_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Stop transfer */
    status = dma_drv->Stop(&dma_periph->dma_handle);
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPI_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph, uint32_t *count)
  \brief       Status of SPI DMA transfer
  \param[in]   dma_periph   Pointer to DMA resources
  \param[in]   count        Current transfer count
  \return      \ref         execution_status
*/
static inline int32_t SPI_DMA_GetStatus(DMA_PERIPHERAL_CONFIG *dma_periph, uint32_t *count)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    /* Stop transfer */
    status = dma_drv->GetStatus(&dma_periph->dma_handle, count);
    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}
#endif /* SPI_DMA_ENABLE */

#if SPI_MICROWIRE_FRF_ENABLE
/**
 * @fn      int32_t ARM_SPI_MicroWire_Config(SPI_RESOURCES *SPI).
 * @brief   Config the Microwire for communication.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_MicroWire_Config(SPI_RESOURCES *SPI)
{
    if (!((SPI->mw_config.cfs >= SPI_MW_CONTROL_FRAME_SIZE_MIN) && (SPI->mw_config.cfs <= SPI_MW_CONTROL_FRAME_SIZE_MAX)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (SPI->mw_config.transfer_mode == SPI_MW_TRANSFER_MODE_SEQUANTIAL)
    {
        spi_mw_set_sequential_mode(SPI->regs);
    }
    else
    {
        spi_mw_set_non_sequential_mode(SPI->regs);
    }

    if (SPI->transfer.is_master)
    {
        if (SPI->mw_config.handshake_enable)
        {
            spi_mw_enable_handshake(SPI->regs);
        }
        else
        {
            spi_mw_disable_handshake(SPI->regs);
        }
    }

    if (SPI->drv_instance == LPSPI_INSTANCE)
    {
        lpspi_mw_set_cfs(SPI->regs, SPI->mw_config.cfs);
    }
    else
    {
        spi_mw_set_cfs(SPI->regs, SPI->mw_config.cfs);
    }

    return ARM_DRIVER_OK;
}
#endif

/**
 * @fn      int32_t ARM_SPI_Initialize(SPI_RESOURCES *SPI, ARM_SPI_SignalEvent_t cb_event).
 * @brief   Initialize the Spi for communication.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   cb_event : Pointer to user callback function.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_Initialize(SPI_RESOURCES *SPI, ARM_SPI_SignalEvent_t cb_event)
{
    if (SPI->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    bool blocking_mode = false;
#if SPI_BLOCKING_MODE_ENABLE
    if (SPI->blocking_mode)
         blocking_mode = true;
#endif

    if (blocking_mode == false && cb_event == NULL)
    {
         return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((SPI->tx_fifo_threshold > SPI_TX_FIFO_DEPTH) || (SPI->tx_fifo_start_level > SPI_TX_FIFO_DEPTH))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (SPI->rx_fifo_threshold > SPI_RX_FIFO_DEPTH)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Reset the transfer structure for this instance */
    SPI->transfer.tx_buff            = NULL;
    SPI->transfer.rx_buff            = NULL;

    SPI->transfer.tx_default_val     = 0;
    SPI->transfer.tx_default_enable  = false;

    SPI->transfer.tx_total_cnt       = 0;
    SPI->transfer.rx_total_cnt       = 0;
    SPI->transfer.tx_current_cnt     = 0;
    SPI->transfer.rx_current_cnt     = 0;
    SPI->transfer.status             = SPI_TRANSFER_STATUS_NONE;

    SPI->cb_event = cb_event;

#if SPI_DMA_ENABLE
    if (SPI->dma_enable)
    {
        SPI->dma_cfg->dma_rx.dma_handle = -1;
        SPI->dma_cfg->dma_tx.dma_handle = -1;

        /* Initialize DMA for SPI-Tx */
        if (SPI_DMA_Initialize(&SPI->dma_cfg->dma_tx) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Initialize DMA for SPI-Rx */
        if (SPI_DMA_Initialize(&SPI->dma_cfg->dma_rx) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif

    SPI->state.initialized = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_SPI_Uninitialize(SPI_RESOURCES *SPI).
 * @brief   Un-Initialize the Spi.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_Uninitialize(SPI_RESOURCES *SPI)
{
    if (SPI->state.initialized == 0)
    {
        return ARM_DRIVER_OK;
    }

    if (SPI->state.powered == 1)
    {
        return ARM_DRIVER_ERROR;
    }

#if SPI_DMA_ENABLE
    if (SPI->dma_enable)
    {
        SPI->dma_cfg->dma_rx.dma_handle = -1;
        SPI->dma_cfg->dma_tx.dma_handle = -1;
    }
#endif

    SPI->cb_event                   = NULL;

    SPI->transfer.tx_buff           = NULL;
    SPI->transfer.rx_buff           = NULL;
    SPI->transfer.tx_default_val    = 0;
    SPI->transfer.tx_default_enable = false;
    SPI->transfer.tx_total_cnt      = 0;
    SPI->transfer.rx_total_cnt      = 0;
    SPI->transfer.tx_current_cnt    = 0;
    SPI->transfer.rx_current_cnt    = 0;
    SPI->transfer.status            = SPI_TRANSFER_STATUS_NONE;

    SPI->state.initialized = 0;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_SPI_PowerControl(SPI_RESOURCES *SPI, ARM_POWER_STATE state).
 * @brief   Handles the spi power.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   state : power state.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_PowerControl(SPI_RESOURCES *SPI, ARM_POWER_STATE state)
{
#if SPI_USE_MASTER_SS_SW
    int32_t ret = ARM_DRIVER_OK;
#endif

    if (SPI->state.initialized == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            if (SPI->state.powered == 0)
            {
                return ARM_DRIVER_OK;
            }

            if (SPI->status.busy)
            {
                return ARM_DRIVER_ERROR_BUSY;
            }

#if SPI_DMA_ENABLE
    if (SPI->dma_enable)
    {
        /* Disable the TX & RX DMA interface of SPI */
        spi_disable_tx_dma(SPI->regs);
        spi_disable_rx_dma(SPI->regs);

        /* DeAllocate DMA for SPI-Tx */
        if (SPI_DMA_DeAllocate(&SPI->dma_cfg->dma_tx) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
        /* DeAllocate DMA for SPI-Rx */
        if (SPI_DMA_DeAllocate(&SPI->dma_cfg->dma_rx) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Power Control DMA for SPI-Tx */
        if (SPI_DMA_PowerControl(&SPI->dma_cfg->dma_tx, state) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
        /* Power Control DMA for SPI-Rx */
        if (SPI_DMA_PowerControl(&SPI->dma_cfg->dma_rx, state) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif

#if SPI_USE_MASTER_SS_SW
            if ((SPI->transfer.is_master) && (SPI->master_ss_control == SPI_SS_SW_CONTROL))
            {
                ret = SPI->sw_config.drvGPIO->PowerControl(SPI->sw_config.ss_pin, ARM_POWER_OFF);
                if(ret != ARM_DRIVER_OK)            {    return ret;    }
                ret = SPI->sw_config.drvGPIO->Uninitialize(SPI->sw_config.ss_pin);
                if(ret != ARM_DRIVER_OK)            {    return ret;    }
            }
#endif

            NVIC_ClearPendingIRQ(SPI->irq);
            NVIC_DisableIRQ(SPI->irq);

            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                disable_lpspi_clk();
            }
            else
            {
                /* reset to default value */
                ctrl_ss_in(SPI->drv_instance, SS_IN_IO_PIN);
            }

            SPI->state.powered = 0;

            break;
        }

        case ARM_POWER_FULL:
        {
            if (SPI->state.powered == 1)
            {
                return ARM_DRIVER_OK;
            }

            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                enable_lpspi_clk();
            }

            spi_mask_interrupts(SPI->regs);

            NVIC_ClearPendingIRQ(SPI->irq);
            NVIC_SetPriority(SPI->irq, SPI->irq_priority);
            NVIC_EnableIRQ(SPI->irq);

            spi_set_tx_threshold(SPI->regs, SPI->tx_fifo_threshold);

            if (SPI->drv_instance != LPSPI_INSTANCE)
            {
                spi_set_tx_fifo_start_level(SPI->regs, SPI->tx_fifo_start_level);
                spi_set_rx_sample_delay(SPI->regs, SPI->rx_sample_delay);
            }

#if SPI_DMA_ENABLE
    if (SPI->dma_enable)
    {
        /* Power Control DMA for SPI-Tx */
        if (SPI_DMA_PowerControl(&SPI->dma_cfg->dma_tx, state) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
        /* Power Control DMA for SPI-Rx */
        if (SPI_DMA_PowerControl(&SPI->dma_cfg->dma_rx, state) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        spi_set_dma_tx_level(SPI->regs, SPI->tx_fifo_threshold);
        spi_set_dma_rx_level(SPI->regs, SPI->rx_fifo_threshold);

        /* Try to allocate a DMA channel */
        if (SPI_DMA_Allocate(&SPI->dma_cfg->dma_rx))
        {
            return ARM_DRIVER_ERROR;
        }
        if (SPI_DMA_Allocate(&SPI->dma_cfg->dma_tx))
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif

            SPI->state.powered = 1;

            break;
        }

        case ARM_POWER_LOW:
        default:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_SPI_Send(SPI_RESOURCES *SPI, const void *data, uint32_t num).
 * @brief   Used to send through spi.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   data : Pointer to the data to send.
 * @param   num : Number of data frames to send.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_Send(SPI_RESOURCES *SPI, const void *data, uint32_t num)
{
    if (SPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

#if SPI_MICROWIRE_FRF_ENABLE
    if (SPI->mw_enable)
    {
        if (!SPI->transfer.is_master)
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif

    if ((data == NULL) && (SPI->transfer.tx_default_enable == false))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (num == 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (SPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    SPI->status.busy = 1;

    SPI->transfer.tx_buff        = (const uint8_t *) data;
    SPI->transfer.tx_total_cnt   = num;
    SPI->transfer.tx_current_cnt = 0;
    SPI->transfer.status         = SPI_TRANSFER_STATUS_NONE;
    SPI->transfer.mode           = SPI_TMOD_TX;

#if SPI_MICROWIRE_FRF_ENABLE
    if (SPI->mw_enable)
    {
        if (SPI->mw_config.transfer_mode == SPI_MW_TRANSFER_MODE_SEQUANTIAL)
        {
            /* In master sequential mode, continuous data transfer is not available;
             * therefore, the value of 'tx_total_cnt' should be set to 2. */
            SPI->transfer.tx_total_cnt = 2U;
        }
        else
        {
            /* In master non-sequential mode, the value of "tx_total_cnt" should be
             * twice the amount of data that needs to be sent. */
            SPI->transfer.tx_total_cnt = (num << 1);
        }
    }
#endif

    /* If the Frame size is more than 16, check if it is aligned to 4 bytes */
    if ((SPI->transfer.frame_size > 16) && ((uint32_t)data & 0x3U) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    /* If the Frame size is more than 8 and less than 16, check if it is aligned to 2 bytes */
    if ((SPI->transfer.frame_size > 8) && ((uint32_t)data & 0x1U) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

#if SPI_DMA_ENABLE
    /* Check if DMA is enabled */
    if (SPI->dma_enable)
    {
        ARM_DMA_PARAMS dma_params;

        if (SPI->drv_instance == LPSPI_INSTANCE)
        {
            lpspi_dma_send(SPI->regs);
        }
        else
        {
            spi_dma_send(SPI->regs);
        }

        /* Start the DMA engine for sending the data to SPI */
        dma_params.peri_reqno    = (int8_t)SPI->dma_cfg->dma_tx.dma_periph_req;
        dma_params.dir           = ARM_DMA_MEM_TO_DEV;
        dma_params.cb_event      = SPI->dma_cb;
        dma_params.src_addr      = data;
        dma_params.dst_addr      = (void*)spi_get_data_addr(SPI->regs);
        dma_params.irq_priority  = SPI->dma_irq_priority;
        dma_params.burst_len     = SPI_TX_FIFO_DEPTH - SPI->tx_fifo_threshold;

        if (SPI->transfer.frame_size > 16)
        {
            dma_params.num_bytes = SPI->transfer.tx_total_cnt * sizeof(uint32_t);
            dma_params.burst_size = BS_BYTE_4;
        }
        else if (SPI->transfer.frame_size > 8)
        {
            dma_params.num_bytes = SPI->transfer.tx_total_cnt * sizeof(uint16_t);
            dma_params.burst_size = BS_BYTE_2;
        }
        else
        {
            dma_params.num_bytes = SPI->transfer.tx_total_cnt * sizeof(uint8_t);
            dma_params.burst_size = BS_BYTE_1;
        }

        if (SPI_DMA_Start(&SPI->dma_cfg->dma_tx, &dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
#if SPI_MICROWIRE_FRF_ENABLE
        if (SPI->mw_enable)
        {
            spi_mw_transmit(SPI->regs, !(SPI->transfer.is_master));
        }
        else
#endif
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
#if SPI_BLOCKING_MODE_ENABLE
                if (SPI->blocking_mode)
                {
                    lpspi_send_blocking(SPI->regs, &SPI->transfer);
                    SPI->transfer.status = SPI_TRANSFER_STATUS_COMPLETE;
                    SPI->status.busy = 0;
                }
                else
#endif
                {
                    lpspi_send(SPI->regs);
                }
            }
            else
            {
#if SPI_BLOCKING_MODE_ENABLE
                if (SPI->blocking_mode)
                {
                    spi_send_blocking(SPI->regs, &SPI->transfer);
                    SPI->transfer.status = SPI_TRANSFER_STATUS_COMPLETE;
                    SPI->status.busy = 0;
                }
                else
#endif
                {
                    spi_send(SPI->regs);
                }
            }
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_SPI_Receive(SPI_RESOURCES *SPI, void *data, uint32_t num).
 * @brief   Used to receive data through spi.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   data : Pointer to the data received.
 * @param   num : Number of data frames to receive.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_Receive(SPI_RESOURCES *SPI, void *data, uint32_t num)
{
    if (SPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if ((data == NULL) || (num == 0))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

#if SPI_MICROWIRE_FRF_ENABLE
    if (SPI->mw_enable)
    {
        if (SPI->transfer.is_master)
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif

    if (SPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    SPI->status.busy = 1;

    SPI->transfer.rx_buff         = data;
    SPI->transfer.rx_total_cnt    = num;
    SPI->transfer.rx_current_cnt  = 0;
    SPI->transfer.status          = SPI_TRANSFER_STATUS_NONE;
    SPI->transfer.mode            = SPI_TMOD_RX;

    spi_set_rx_threshold(SPI->regs, SPI->rx_fifo_threshold);

#if SPI_MICROWIRE_FRF_ENABLE
    if (SPI->mw_enable)
    {
        if (SPI->mw_config.transfer_mode == SPI_MW_TRANSFER_MODE_SEQUANTIAL)
        {
            /* In slave sequential mode, the value of "rx_total_cnt" should be
             * one greater than the number of data needs to be received. */
            SPI->transfer.rx_total_cnt = (num + 1U);
        }
        else
        {
            /* In slave non-sequential mode, the value of "rx_total_cnt" should be
             * twice the amount of data that needs to be received. */
            SPI->transfer.rx_total_cnt = (num << 1);
        }
    }
#endif

    /* If the Frame size is more than 16, check if it is aligned to 4 bytes */
    if ((SPI->transfer.frame_size > 16) && ((uint32_t)data & 0x3U) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    /* If the Frame size is more than 8 and less than 16, check if it is aligned to 2 bytes */
    if ((SPI->transfer.frame_size > 8) && ((uint32_t)data & 0x1U) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

#if SPI_DMA_ENABLE
    ARM_DMA_PARAMS rx_dma_params;

    /* Check if DMA is enabled for this */
    if (SPI->dma_enable)
    {
        if (SPI->drv_instance == LPSPI_INSTANCE)
        {
            lpspi_dma_receive(SPI->regs, SPI->transfer.rx_total_cnt);
        }
        else
        {
            spi_dma_receive(SPI->regs, &SPI->transfer);
        }

        if (SPI->rx_fifo_threshold > 0)
        {
            SPI->rx_fifo_threshold = spi_dma_calc_rx_level(SPI->transfer.rx_total_cnt, SPI->rx_fifo_threshold);
            spi_set_dma_rx_level(SPI->regs, SPI->rx_fifo_threshold);
        }

        /* Start the DMA engine for receive the data to SPI */
        rx_dma_params.peri_reqno    = (int8_t)SPI->dma_cfg->dma_rx.dma_periph_req;
        rx_dma_params.dir           = ARM_DMA_DEV_TO_MEM;
        rx_dma_params.cb_event      = SPI->dma_cb;
        rx_dma_params.src_addr      = (void*)spi_get_data_addr(SPI->regs);
        rx_dma_params.dst_addr      = data;
        rx_dma_params.irq_priority  = SPI->dma_irq_priority;
        rx_dma_params.burst_len     = SPI->rx_fifo_threshold + 1;

        if (SPI->transfer.frame_size > 16)
        {
            rx_dma_params.num_bytes = SPI->transfer.rx_total_cnt * sizeof(uint32_t);
            rx_dma_params.burst_size = BS_BYTE_4;
        }
        else if (SPI->transfer.frame_size > 8)
        {
            rx_dma_params.num_bytes = SPI->transfer.rx_total_cnt * sizeof(uint16_t);
            rx_dma_params.burst_size = BS_BYTE_2;
        }
        else
        {
            rx_dma_params.num_bytes = SPI->transfer.rx_total_cnt;
            rx_dma_params.burst_size = BS_BYTE_1;
        }

        if (SPI_DMA_Start(&SPI->dma_cfg->dma_rx, &rx_dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
#if SPI_MICROWIRE_FRF_ENABLE
        if (SPI->mw_enable)
        {
            spi_mw_receive(SPI->regs, &SPI->transfer);
        }
        else
#endif
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
#if SPI_BLOCKING_MODE_ENABLE
                if (SPI->blocking_mode)
                {
                    lpspi_receive_blocking(SPI->regs, &SPI->transfer);
                    SPI->transfer.status = SPI_TRANSFER_STATUS_COMPLETE;
                    SPI->status.busy = 0;
                }
                else
#endif
                {
                    lpspi_receive(SPI->regs, SPI->transfer.rx_total_cnt);
                }
            }
            else
            {
#if SPI_BLOCKING_MODE_ENABLE
                if (SPI->blocking_mode)
                {
                    spi_receive_blocking(SPI->regs, &SPI->transfer);
                    SPI->transfer.status = SPI_TRANSFER_STATUS_COMPLETE;
                    SPI->status.busy = 0;
                }
                else
#endif
                {
                    spi_receive(SPI->regs, &SPI->transfer);
                }
            }
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_SPI_Transfer(SPI_RESOURCES *SPI, const void *data_out, void *data_in, uint32_t num).
 * @brief   Used to Transfer and Receive data through spi.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   data_out : Pointer to the data send.
 * @param   data_in : Pointer to the data received.
 * @param   num : Number of data frames to transfer.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_Transfer(SPI_RESOURCES *SPI, const void *data_out, void *data_in, uint32_t num)
{
    if (SPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if ((data_out == NULL) && (SPI->transfer.tx_default_enable == false))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((data_out == NULL) || (data_in == NULL) || (num == 0))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (SPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    SPI->status.busy = 1;

    SPI->transfer.tx_buff        = (const uint8_t *) data_out;
    SPI->transfer.rx_buff        = data_in;
    SPI->transfer.tx_total_cnt   = num;
    SPI->transfer.rx_total_cnt   = num;
    SPI->transfer.tx_current_cnt = 0;
    SPI->transfer.rx_current_cnt = 0;
    SPI->transfer.status         = SPI_TRANSFER_STATUS_NONE;
    SPI->transfer.mode           = SPI_TMOD_TX_AND_RX;

    spi_set_rx_threshold(SPI->regs, SPI->rx_fifo_threshold);

#if SPI_MICROWIRE_FRF_ENABLE
    if (SPI->mw_enable)
    {
        if (SPI->transfer.is_master && (SPI->mw_config.transfer_mode == SPI_MW_TRANSFER_MODE_SEQUANTIAL))
        {
            /* In master sequential mode receive, tx_total_cnt should be 1 as only one control word is sent */
            SPI->transfer.tx_total_cnt = 1U;
        }

        if ((!SPI->transfer.is_master) && (SPI->mw_config.transfer_mode == SPI_MW_TRANSFER_MODE_SEQUANTIAL))
        {
            /* In slave sequential mode transmit, rx_total_cnt should be 1 as only one control word is received */
            SPI->transfer.rx_total_cnt = 1U;
        }
    }
#endif

    /* If the Frame size is more than 16, check if it is aligned to 4 bytes */
    if ((SPI->transfer.frame_size > 16) && ((((uint32_t)data_in & 0x3U) != 0U) && (((uint32_t)data_out & 0x3U) != 0U)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    /* If the Frame size is more than 8 and less than 16, check if it is aligned to 2 bytes */
    if ((SPI->transfer.frame_size > 8) && ((((uint32_t)data_in & 0x1U) != 0U) && (((uint32_t)data_out & 0x1U) != 0U)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

#if SPI_DMA_ENABLE
    ARM_DMA_PARAMS tx_dma_params, rx_dma_params;

    /* Check if DMA is enabled for this */
    if (SPI->dma_enable)
    {
        if (SPI->drv_instance == LPSPI_INSTANCE)
        {
            lpspi_dma_transfer(SPI->regs);
        }
        else
        {
            spi_dma_transfer(SPI->regs);
        }

        if (SPI->rx_fifo_threshold > 0)
        {
            SPI->rx_fifo_threshold = spi_dma_calc_rx_level(SPI->transfer.rx_total_cnt, SPI->rx_fifo_threshold);
            spi_set_dma_rx_level(SPI->regs, SPI->rx_fifo_threshold);
        }

        /* Start the DMA engine for sending the data to SPI */
        tx_dma_params.peri_reqno    = (int8_t)SPI->dma_cfg->dma_tx.dma_periph_req;
        tx_dma_params.dir           = ARM_DMA_MEM_TO_DEV;
        tx_dma_params.cb_event      = SPI->dma_cb;
        tx_dma_params.src_addr      = data_out;
        tx_dma_params.dst_addr      = (void*)spi_get_data_addr(SPI->regs);
        tx_dma_params.irq_priority  = SPI->dma_irq_priority;
        tx_dma_params.burst_len     = SPI_TX_FIFO_DEPTH - SPI->tx_fifo_threshold;

        /* Start the DMA engine for receive the data to SPI */
        rx_dma_params.peri_reqno    = (int8_t)SPI->dma_cfg->dma_rx.dma_periph_req;
        rx_dma_params.dir           = ARM_DMA_DEV_TO_MEM;
        rx_dma_params.cb_event      = SPI->dma_cb;
        rx_dma_params.src_addr      = (void*)spi_get_data_addr(SPI->regs);
        rx_dma_params.dst_addr      = data_in;
        rx_dma_params.irq_priority  = SPI->dma_irq_priority;
        rx_dma_params.burst_len     = SPI->rx_fifo_threshold + 1;

        if (SPI->transfer.frame_size > 16)
        {
            tx_dma_params.num_bytes  = SPI->transfer.tx_total_cnt * sizeof(uint32_t);
            rx_dma_params.num_bytes  = SPI->transfer.rx_total_cnt * sizeof(uint32_t);
            tx_dma_params.burst_size = BS_BYTE_4;
            rx_dma_params.burst_size = BS_BYTE_4;
        }
        else if (SPI->transfer.frame_size > 8)
        {
            tx_dma_params.num_bytes  = SPI->transfer.tx_total_cnt * sizeof(uint16_t);
            rx_dma_params.num_bytes  = SPI->transfer.rx_total_cnt * sizeof(uint16_t);
            tx_dma_params.burst_size = BS_BYTE_2;
            rx_dma_params.burst_size = BS_BYTE_2;
        }
        else
        {
            tx_dma_params.src_addr   = (const uint8_t *)data_out;
            tx_dma_params.num_bytes  = SPI->transfer.tx_total_cnt * sizeof(uint8_t);
            rx_dma_params.num_bytes  = SPI->transfer.rx_total_cnt * sizeof(uint8_t);
            tx_dma_params.burst_size = BS_BYTE_1;
            rx_dma_params.burst_size = BS_BYTE_1;
        }

        if (SPI_DMA_Start(&SPI->dma_cfg->dma_rx, &rx_dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
        if (SPI_DMA_Start(&SPI->dma_cfg->dma_tx, &tx_dma_params) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
#if SPI_MICROWIRE_FRF_ENABLE
        if (SPI->mw_enable)
        {
            if (SPI->transfer.is_master)
            {
                spi_mw_receive(SPI->regs, &SPI->transfer);
            }
            else
            {
                spi_mw_transmit(SPI->regs, !(SPI->transfer.is_master));
            }
        }
        else
#endif
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
#if SPI_BLOCKING_MODE_ENABLE
                if (SPI->blocking_mode)
                {
                    lpspi_transfer_blocking(SPI->regs, &SPI->transfer);
                    SPI->transfer.status = SPI_TRANSFER_STATUS_COMPLETE;
                    SPI->status.busy = 0;
                }
                else
#endif
                {
                    lpspi_transfer(SPI->regs);
                }
            }
            else
            {
#if SPI_BLOCKING_MODE_ENABLE
                if (SPI->blocking_mode)
                {
                    spi_transfer_blocking(SPI->regs, &SPI->transfer);
                    SPI->transfer.status = SPI_TRANSFER_STATUS_COMPLETE;
                    SPI->status.busy = 0;
                }
                else
#endif
                {
                    spi_transfer(SPI->regs);
                }
            }
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_SPI_GetDataCount(SPI_RESOURCES *SPI).
 * @brief   Used to get the data count on spi data transferring modes.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @retval  \ref data count
 */
static uint32_t ARM_SPI_GetDataCount(SPI_RESOURCES *SPI)
{
    uint32_t count = 0;
    SPI_TMOD tmod;

    if (SPI->drv_instance == LPSPI_INSTANCE)
    {
        tmod = lpspi_get_tmod(SPI->regs);
    }
    else
    {
        tmod = spi_get_tmod(SPI->regs);
    }

    switch (tmod)
    {
        case SPI_TMOD_TX:
            count = SPI->transfer.tx_current_cnt;
            break;
        case SPI_TMOD_RX:
        case SPI_TMOD_TX_AND_RX:
            count = SPI->transfer.rx_current_cnt;
            break;
        case SPI_TMOD_EEPROM_READ:
        default:
            break;
    }

    return count;
}

/**
 * @fn      int32_t ARM_SPI_Control(SPI_RESOURCES *SPI, uint32_t control, uint32_t arg).
 * @brief   Used to configure spi.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   control : control code.
 * @param   arg : argument.
 * @retval  \ref execution_status
 */
static int32_t ARM_SPI_Control(SPI_RESOURCES *SPI, uint32_t control, uint32_t arg)
{
    int32_t ret = ARM_DRIVER_OK;
    uint32_t clk;

    if (SPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if (SPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    switch (control & ARM_SPI_CONTROL_Msk)
    {
        case ARM_SPI_MODE_INACTIVE:
        {
            if (control == 0)
            {
                spi_disable(SPI->regs);
            }
            break;
        }

        /* SPI Master (Output on MOSI, Input on MISO); arg = Bus Speed in bps */
        case ARM_SPI_MODE_MASTER:
        {
            if (SPI->drv_instance != LPSPI_INSTANCE)
            {
                ctrl_ss_in(SPI->drv_instance, SS_IN_SS_IN_VAL);

                spi_mode_master(SPI->regs);
            }

            clk = getSpiCoreClock(SPI->drv_instance);
            spi_set_bus_speed(SPI->regs, arg, clk);

            SPI->transfer.is_master = 1;

            break;
        }

        /* SPI Slave  (Output on MISO, Input on MOSI) arg = Bus speed in bps */
        case ARM_SPI_MODE_SLAVE:
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }
            else
            {
                spi_mode_slave(SPI->regs);

                SPI->transfer.is_master = 0;
            }
            break;
        }

        /* SPI Slave  (Output/Input on MISO) */
        case ARM_SPI_SET_BUS_SPEED:
        {
            clk = getSpiCoreClock(SPI->drv_instance);
            spi_set_bus_speed(SPI->regs, arg, clk);
            break;
        }

        /* Get Bus Speed in bps */
        case ARM_SPI_GET_BUS_SPEED:
        {
            clk = getSpiCoreClock(SPI->drv_instance);
            return (int32_t) spi_get_bus_speed(SPI->regs, clk);
        }

        /* Set the default transmission value */
        case ARM_SPI_SET_DEFAULT_TX_VALUE:
        {
            SPI->transfer.tx_default_val = arg;
            SPI->transfer.tx_default_enable = true;
            break;
        }

        /* Control the Slave Select signal */
        case ARM_SPI_CONTROL_SS:
        {
            if (SPI->master_ss_control == SPI_SS_HW_CONTROL)
            {
                if (arg == 1)
                {
                    spi_control_ss(SPI->regs, SPI->slave_select, SPI_SS_STATE_ENABLE);
                }
                else if (arg == 0)
                {
                    spi_control_ss(SPI->regs, SPI->slave_select, SPI_SS_STATE_DISABLE);
                }
                else
                {
                    return ARM_DRIVER_ERROR_PARAMETER;
                }
            }
            else if (SPI->master_ss_control == SPI_SS_SW_CONTROL)
            {
#if SPI_USE_MASTER_SS_SW
                if (arg == 1)
                {
                    ret = SPI->sw_config.drvGPIO->SetValue(SPI->sw_config.ss_pin, SPI->sw_config.active_polarity);
                    if(ret != ARM_DRIVER_OK)            {    return ret;    }
                }
                else if (arg == 0)
                {
                    ret = SPI->sw_config.drvGPIO->SetValue(SPI->sw_config.ss_pin, !(SPI->sw_config.active_polarity));
                    if(ret != ARM_DRIVER_OK)            {    return ret;    }
                }
                else
                {
                    return ARM_DRIVER_ERROR_PARAMETER;
                }
#endif
            }
            return ARM_DRIVER_OK;
        }

        /* Abort the current data transfer */
        case ARM_SPI_ABORT_TRANSFER:
        {
#if SPI_DMA_ENABLE
    if (SPI->dma_enable)
    {
        /* SEND ONLY mode */
        if (SPI->transfer.mode == SPI_TMOD_TX)
        {
            if (SPI_DMA_Stop(&SPI->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
        }

        /* RECEIVE ONLY mode */
        if (SPI->transfer.mode == SPI_TMOD_RX)
        {
            if (SPI_DMA_Stop(&SPI->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }

            if (SPI->transfer.is_master)
            {
                if (SPI_DMA_Stop(&SPI->dma_cfg->dma_tx) != ARM_DRIVER_OK)
                {
                    return ARM_DRIVER_ERROR;
                }
            }

        }

        /* TRANSFER mode */
        if (SPI->transfer.mode == SPI_TMOD_TX_AND_RX)
        {
            if (SPI_DMA_Stop(&SPI->dma_cfg->dma_tx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            if (SPI_DMA_Stop(&SPI->dma_cfg->dma_rx) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
        }
    }
#endif

            spi_mask_interrupts(SPI->regs);

            SPI->transfer.tx_buff            = NULL;
            SPI->transfer.rx_buff            = NULL;
            SPI->transfer.tx_default_val     = 0;
            SPI->transfer.tx_default_enable  = false;
            SPI->transfer.tx_total_cnt       = 0;
            SPI->transfer.rx_total_cnt       = 0;
            SPI->transfer.tx_current_cnt     = 0;
            SPI->transfer.rx_current_cnt     = 0;
            SPI->status.busy                 = 0;

            spi_disable(SPI->regs);
            spi_enable(SPI->regs);
            break;
        }

        default:
        {
            ret = ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
        }
    }

    switch (control & ARM_SPI_FRAME_FORMAT_Msk)
    {
        /* SPI Mode configuration */
        case ARM_SPI_CPOL0_CPHA0:
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                lpspi_set_mode(SPI->regs, SPI_MODE_0);
                lpspi_set_sste(SPI->regs, SPI->sste_enable);
            }
            else
            {
                spi_set_mode(SPI->regs, SPI_MODE_0);
                spi_set_sste(SPI->regs, SPI->sste_enable);
            }
            break;
        }
        case ARM_SPI_CPOL0_CPHA1:
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                lpspi_set_mode(SPI->regs, SPI_MODE_1);
            }
            else
            {
                spi_set_mode(SPI->regs, SPI_MODE_1);
            }
            break;
        }
        case ARM_SPI_CPOL1_CPHA0:
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                lpspi_set_mode(SPI->regs, SPI_MODE_2);
                lpspi_set_sste(SPI->regs, SPI->sste_enable);
            }
            else
            {
                spi_set_mode(SPI->regs, SPI_MODE_2);
                spi_set_sste(SPI->regs, SPI->sste_enable);
            }
            break;
        }
        case ARM_SPI_CPOL1_CPHA1:
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                lpspi_set_mode(SPI->regs, SPI_MODE_3);
            }
            else
            {
                spi_set_mode(SPI->regs, SPI_MODE_3);
            }
            break;
        }

        /* Texas Instruments Frame Format */
        case ARM_SPI_TI_SSI:
        {
            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                lpspi_set_protocol(SPI->regs, SPI_PROTO_SSP);
            }
            else
            {
                spi_set_protocol(SPI->regs, SPI_PROTO_SSP);
            }
            break;
        }

        /* National Microwire Frame Format */
        case ARM_SPI_MICROWIRE:
        {
#if SPI_MICROWIRE_FRF_ENABLE
            if (!SPI->mw_enable)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            if (SPI->drv_instance == LPSPI_INSTANCE)
            {
                lpspi_set_protocol(SPI->regs, SPI_PROTO_MICROWIRE);
            }
            else
            {
                spi_set_protocol(SPI->regs, SPI_PROTO_MICROWIRE);
            }

            ret = ARM_SPI_MicroWire_Config(SPI);
            if (ret != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
#endif
            break;
        }

        default:
        {
            ret = ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
        }
    }

    /* Configure frame size */
    if (control & ARM_SPI_DATA_BITS_Msk)
    {
        SPI->transfer.frame_size = ((control & ARM_SPI_DATA_BITS_Msk) >> ARM_SPI_DATA_BITS_Pos);

        if (SPI->drv_instance == LPSPI_INSTANCE)
        {
            lpspi_set_dfs(SPI->regs, SPI->transfer.frame_size);
        }
        else
        {
            spi_set_dfs(SPI->regs, SPI->transfer.frame_size);
        }
    }

    switch (control & ARM_SPI_BIT_ORDER_Msk)
    {
        /* SPI Bit order from MSB to LSB (default) */
        case ARM_SPI_MSB_LSB:
        {
            break;
        }
        /* SPI Bit order from LSB to MSB */
        case ARM_SPI_LSB_MSB:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    if (SPI->transfer.is_master)
    {
        switch (control & ARM_SPI_SS_MASTER_MODE_Msk)
        {
            /* SPI Slave Select when Master: Not used (default) */
            case ARM_SPI_SS_MASTER_UNUSED:
            {
                /* add dummy value to SER reg to start the data transfer. Below, setting first bit */
                spi_control_ss(SPI->regs, 0x0, SPI_SS_STATE_ENABLE);

                break;
            }

            /* SPI Slave Select when Master: Software controlled */
            case ARM_SPI_SS_MASTER_SW:
            {
#if SPI_USE_MASTER_SS_SW
                /* add dummy value to SER reg to start the data transfer. Below, setting first bit */
                spi_control_ss(SPI->regs, 0x0, SPI_SS_STATE_ENABLE);

                /* GPIO setup */
                ret = SPI->sw_config.drvGPIO->Initialize(SPI->sw_config.ss_pin, NULL);
                if(ret != ARM_DRIVER_OK)            {    return ret;    }
                ret = SPI->sw_config.drvGPIO->PowerControl(SPI->sw_config.ss_pin, ARM_POWER_FULL);
                if(ret != ARM_DRIVER_OK)            {    return ret;    }
                ret = SPI->sw_config.drvGPIO->SetDirection(SPI->sw_config.ss_pin, GPIO_PIN_DIRECTION_OUTPUT);
                if(ret != ARM_DRIVER_OK)            {    return ret;    }
                ret = SPI->sw_config.drvGPIO->SetValue(SPI->sw_config.ss_pin, !(SPI->sw_config.active_polarity));
                if(ret != ARM_DRIVER_OK)            {    return ret;    }

                SPI->master_ss_control = SPI_SS_SW_CONTROL;
#endif
                break;
            }

            /* SPI Slave Select when Master: Hardware controlled Output */
            case ARM_SPI_SS_MASTER_HW_OUTPUT:
            {
                SPI->master_ss_control = SPI_SS_HW_CONTROL;
                break;
            }

            /* SPI Slave Select when Master: Hardware monitored Input */
            case ARM_SPI_SS_MASTER_HW_INPUT:
            {
                //TODO: Need to Implement
                break;
            }
        }
    }

    if (!(SPI->transfer.is_master))
    {
        switch (control & ARM_SPI_SS_SLAVE_MODE_Msk)
        {
            /* SPI Slave Select when Slave: Hardware monitored (default) */
            case ARM_SPI_SS_SLAVE_HW:
            {
                /* No need to configure anything */
                break;
            }

            /* SPI Slave Select when Slave: Software controlled */
            case ARM_SPI_SS_SLAVE_SW:
            {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }
        }
    }
    return ret;
}

/**
 * @fn      void SPI_IRQ_Handler(SPI_RESOURCES *SPI, spi_transfer_t *transfer).
 * @brief   SPI IRQ handler.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   transfer : transfer structure pointer for the SPI instance
 * @retval  none
 */
static void SPI_IRQ_Handler(SPI_RESOURCES *SPI)
{
#if SPI_MICROWIRE_FRF_ENABLE
    if (SPI->mw_enable)
    {
        spi_mw_irq_handler(SPI->regs, &(SPI->transfer));
    }
    else
#endif
    {
        spi_irq_handler(SPI->regs, &(SPI->transfer));
    }

    if (SPI->transfer.status == SPI_TRANSFER_STATUS_COMPLETE)
    {
        SPI->transfer.status = SPI_TRANSFER_STATUS_NONE;
        SPI->status.busy = 0;
        SPI->cb_event(ARM_SPI_EVENT_TRANSFER_COMPLETE);
    }

    if (SPI->transfer.status == SPI_TRANSFER_STATUS_OVERFLOW)
    {
        SPI->transfer.status = SPI_TRANSFER_STATUS_NONE;
        SPI->status.data_lost = 1;
        SPI->status.busy = 0;
        SPI->cb_event(ARM_SPI_EVENT_DATA_LOST);
    }
}

#if SPI_DMA_ENABLE
/**
 * @fn      void SPI_DMACallback(SPI_RESOURCES *SPI, uint32_t event, int8_t peri_num)
 * @brief   DMA Callback function for SPI.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @param   event : Event from DMA
 * @param   peri_num : peripheral request number
 * @retval  none
 */
static void SPI_DMACallback(SPI_RESOURCES *SPI, uint32_t event, int8_t peri_num)
{
    if (!SPI->cb_event)
    {
        return;
    }

    /* Transfer Completed */
    if (event & ARM_DMA_EVENT_COMPLETE)
    {
        switch(peri_num)
        {
            case SPI0_DMA_TX_PERIPH_REQ:
            case SPI1_DMA_TX_PERIPH_REQ:
            case SPI2_DMA_TX_PERIPH_REQ:
            case SPI3_DMA_TX_PERIPH_REQ:
#if defined (M55_HE)
            case LPSPI_DMA_TX_PERIPH_REQ:
#endif
                if (SPI->transfer.mode == SPI_TMOD_TX)
                {
                    SPI->status.busy = 0;
                    SPI->cb_event(ARM_SPI_EVENT_TRANSFER_COMPLETE);
                }
                break;
            case SPI0_DMA_RX_PERIPH_REQ:
            case SPI1_DMA_RX_PERIPH_REQ:
            case SPI2_DMA_RX_PERIPH_REQ:
            case SPI3_DMA_RX_PERIPH_REQ:
#if defined (M55_HE)
            case LPSPI_DMA_RX_PERIPH_REQ:
#endif
                SPI->status.busy = 0;
                SPI->cb_event(ARM_SPI_EVENT_TRANSFER_COMPLETE);
                break;
            default:
                break;
        }
    }

    /* Abort Occurred */
    if (event & ARM_DMA_EVENT_ABORT)
    {
        SPI->status.busy = 0;
        SPI->cb_event(ARM_SPI_EVENT_DATA_LOST);
    }
}
#endif

/**
 * @fn      ARM_SPI_STATUS ARM_SPI_GetStatus(SPI_RESOURCES *SPI)
 * @brief   Used to get spi status.
 * @note    none.
 * @param   SPI : Pointer to spi resources structure.
 * @retval  \ref spi driver status.
 */
__STATIC_INLINE ARM_SPI_STATUS ARM_SPI_GetStatus(SPI_RESOURCES *SPI)
{
    return SPI->status;
}


/* SPI0 driver instance */
#if RTE_SPI0

#if RTE_SPI0_DMA_ENABLE
static void SPI0_DMACallback(uint32_t event, int8_t peri_num);
static SPI_DMA_HW_CONFIG SPI0_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI0_DMA),
        .dma_periph_req = SPI0_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI0_DMA,
             .group    = SPI0_DMA_GROUP,
             .channel  = SPI0_DMA_RX_PERIPH_REQ,
             .enable_handshake = SPI0_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI0_DMA),
        .dma_periph_req = SPI0_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI0_DMA,
             .group    = SPI0_DMA_GROUP,
             .channel  = SPI0_DMA_TX_PERIPH_REQ,
             .enable_handshake = SPI0_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

#if RTE_SPI0_USE_MASTER_SS_SW
/* GPIO driver Instance for SPI0 SW controlled slave select */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_SPI0_SW_SPI_PORT);
#endif

static SPI_RESOURCES SPI0_RES = {
    .regs                   = (SPI_Type*) SPI0_BASE,
    .cb_event               = NULL,
    .irq_priority           = RTE_SPI0_IRQ_PRIORITY,
    .drv_instance           = SPI_INSTANCE_0,
    .slave_select           = RTE_SPI0_CHIP_SELECTION_PIN,
    .tx_fifo_threshold      = RTE_SPI0_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_SPI0_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_SPI0_RX_FIFO_THRESHOLD,
    .rx_sample_delay        = RTE_SPI0_RX_SAMPLE_DELAY,
    .sste_enable            = RTE_SPI0_SSTE_ENABLE,
    .irq                    = SPI0_IRQ_IRQn,
#if RTE_SPI0_DMA_ENABLE
    .dma_enable             = RTE_SPI0_DMA_ENABLE,
    .dma_irq_priority       = RTE_SPI0_DMA_IRQ_PRI,
    .dma_cb                 = SPI0_DMACallback,
    .dma_cfg                = &SPI0_DMA_HW_CONFIG,
#endif
#if SPI_BLOCKING_MODE_ENABLE
    .blocking_mode          = RTE_SPI0_BLOCKING_MODE_ENABLE,
#endif
#if RTE_SPI0_USE_MASTER_SS_SW
    .sw_config =
    {
        .ss_port            = RTE_SPI0_SW_SPI_PORT,
        .ss_pin             = RTE_SPI0_SW_SPI_PIN,
        .active_polarity    = RTE_SPI0_SW_SPI_SS_POLARITY,
        .drvGPIO            = (ARM_DRIVER_GPIO*) &ARM_Driver_GPIO_(RTE_SPI0_SW_SPI_PORT),
    }
#endif
#if RTE_SPI0_MICROWIRE_FRF_ENABLE
    .mw_enable              = RTE_SPI0_MICROWIRE_FRF_ENABLE,
    .mw_config =
    {
#if RTE_SPI0_MW_TRANSFER_MODE
        .transfer_mode      = SPI_MW_TRANSFER_MODE_SEQUANTIAL,
#else
        .transfer_mode      = SPI_MW_TRANSFER_MODE_NON_SEQUANTIAL,
#endif
        .handshake_enable   = RTE_SPI0_MW_HANDSAHKE_ENABLE,
        .cfs                = RTE_SPI0_MW_CFS
    }
#endif
};

extern void SPI0_IRQHandler(void);
void SPI0_IRQHandler(void)
{
    SPI_IRQ_Handler(&SPI0_RES);
}

#if RTE_SPI0_DMA_ENABLE
void SPI0_DMACallback(uint32_t event, int8_t peri_num)
{
    SPI_DMACallback(&SPI0_RES, event, peri_num);
}
#endif

static int32_t ARM_SPI0_Initialize(ARM_SPI_SignalEvent_t cb_event)
{
    return ARM_SPI_Initialize(&SPI0_RES, cb_event);
}

static int32_t ARM_SPI0_Uninitialize(void)
{
    return ARM_SPI_Uninitialize(&SPI0_RES);
}

static int32_t ARM_SPI0_PowerControl(ARM_POWER_STATE state)
{
    return ARM_SPI_PowerControl(&SPI0_RES, state);
}

static int32_t ARM_SPI0_Send(const void *data, uint32_t num)
{
    return ARM_SPI_Send(&SPI0_RES, data, num);
}

static int32_t ARM_SPI0_Receive(void *data, uint32_t num)
{
    return ARM_SPI_Receive(&SPI0_RES, data, num);
}

static int32_t ARM_SPI0_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_SPI_Transfer(&SPI0_RES, data_out, data_in, num);
}

static uint32_t ARM_SPI0_GetDataCount(void)
{
    return ARM_SPI_GetDataCount(&SPI0_RES);
}

static int32_t ARM_SPI0_Control(uint32_t control, uint32_t arg)
{
    return ARM_SPI_Control(&SPI0_RES, control, arg);
}

static ARM_SPI_STATUS ARM_SPI0_GetStatus(void)
{
    return ARM_SPI_GetStatus(&SPI0_RES);
}

extern ARM_DRIVER_SPI Driver_SPI0;
ARM_DRIVER_SPI Driver_SPI0 = {
    ARM_SPI_GetVersion,
    ARM_SPI_GetCapabilities,
    ARM_SPI0_Initialize,
    ARM_SPI0_Uninitialize,
    ARM_SPI0_PowerControl,
    ARM_SPI0_Send,
    ARM_SPI0_Receive,
    ARM_SPI0_Transfer,
    ARM_SPI0_GetDataCount,
    ARM_SPI0_Control,
    ARM_SPI0_GetStatus
};
#endif /* RTE_SPI0 */

/* SPI1 driver instance */
#if RTE_SPI1

#if RTE_SPI1_DMA_ENABLE
static void SPI1_DMACallback(uint32_t event, int8_t peri_num);
static SPI_DMA_HW_CONFIG SPI1_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI1_DMA),
        .dma_periph_req = SPI1_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI1_DMA,
             .group    = SPI1_DMA_GROUP,
             .channel  = SPI1_DMA_RX_PERIPH_REQ,
             .enable_handshake = SPI1_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI1_DMA),
        .dma_periph_req = SPI1_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI1_DMA,
             .group    = SPI1_DMA_GROUP,
             .channel  = SPI1_DMA_TX_PERIPH_REQ,
             .enable_handshake = SPI1_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

#if RTE_SPI1_USE_MASTER_SS_SW
/* GPIO driver Instance for SPI1 SW controlled slave select */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_SPI1_SW_SPI_PORT);
#endif

static SPI_RESOURCES SPI1_RES = {
    .regs                   = (SPI_Type*) SPI1_BASE,
    .cb_event               = NULL,
    .irq_priority           = RTE_SPI1_IRQ_PRIORITY,
    .drv_instance           = SPI_INSTANCE_1,
    .slave_select           = RTE_SPI1_CHIP_SELECTION_PIN,
    .tx_fifo_threshold      = RTE_SPI1_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_SPI1_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_SPI1_RX_FIFO_THRESHOLD,
    .rx_sample_delay        = RTE_SPI1_RX_SAMPLE_DELAY,
    .sste_enable            = RTE_SPI1_SSTE_ENABLE,
    .irq                    = SPI1_IRQ_IRQn,
#if RTE_SPI1_DMA_ENABLE
    .dma_enable             = RTE_SPI1_DMA_ENABLE,
    .dma_irq_priority       = RTE_SPI1_DMA_IRQ_PRI,
    .dma_cb                 = SPI1_DMACallback,
    .dma_cfg                = &SPI1_DMA_HW_CONFIG,
#endif
#if SPI_BLOCKING_MODE_ENABLE
    .blocking_mode          = RTE_SPI1_BLOCKING_MODE_ENABLE,
#endif
#if RTE_SPI1_USE_MASTER_SS_SW
    .sw_config =
    {
        .ss_port            = RTE_SPI1_SW_SPI_PORT,
        .ss_pin             = RTE_SPI1_SW_SPI_PIN,
        .active_polarity    = RTE_SPI1_SW_SPI_SS_POLARITY,
        .drvGPIO            = (ARM_DRIVER_GPIO*) &ARM_Driver_GPIO_(RTE_SPI1_SW_SPI_PORT),
    }
#endif
#if RTE_SPI1_MICROWIRE_FRF_ENABLE
    .mw_enable              = RTE_SPI1_MICROWIRE_FRF_ENABLE,
    .mw_config =
    {
#if RTE_SPI1_MW_TRANSFER_MODE
        .transfer_mode      = SPI_MW_TRANSFER_MODE_SEQUANTIAL,
#else
        .transfer_mode      = SPI_MW_TRANSFER_MODE_NON_SEQUANTIAL,
#endif
        .handshake_enable   = RTE_SPI1_MW_HANDSAHKE_ENABLE,
        .cfs                = RTE_SPI1_MW_CFS
    }
#endif
};

extern void SPI1_IRQHandler(void);
void SPI1_IRQHandler(void)
{
    SPI_IRQ_Handler(&SPI1_RES);
}

#if RTE_SPI1_DMA_ENABLE
void SPI1_DMACallback(uint32_t event, int8_t peri_num)
{
    SPI_DMACallback(&SPI1_RES, event, peri_num);
}
#endif

static int32_t ARM_SPI1_Initialize(ARM_SPI_SignalEvent_t cb_event)
{
    return ARM_SPI_Initialize(&SPI1_RES, cb_event);
}

static int32_t ARM_SPI1_Uninitialize(void)
{
    return ARM_SPI_Uninitialize(&SPI1_RES);
}

static int32_t ARM_SPI1_PowerControl(ARM_POWER_STATE state)
{
    return ARM_SPI_PowerControl(&SPI1_RES, state);
}

static int32_t ARM_SPI1_Send(const void *data, uint32_t num)
{
    return ARM_SPI_Send(&SPI1_RES, data, num);
}

static int32_t ARM_SPI1_Receive(void *data, uint32_t num)
{
    return ARM_SPI_Receive(&SPI1_RES, data, num);
}

static int32_t ARM_SPI1_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_SPI_Transfer(&SPI1_RES, data_out, data_in, num);
}

static uint32_t ARM_SPI1_GetDataCount(void)
{
    return ARM_SPI_GetDataCount(&SPI1_RES);
}

static int32_t ARM_SPI1_Control(uint32_t control, uint32_t arg)
{
    return ARM_SPI_Control(&SPI1_RES, control, arg);
}

static ARM_SPI_STATUS ARM_SPI1_GetStatus(void)
{
    return ARM_SPI_GetStatus(&SPI1_RES);
}

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI Driver_SPI1 = {
    ARM_SPI_GetVersion,
    ARM_SPI_GetCapabilities,
    ARM_SPI1_Initialize,
    ARM_SPI1_Uninitialize,
    ARM_SPI1_PowerControl,
    ARM_SPI1_Send,
    ARM_SPI1_Receive,
    ARM_SPI1_Transfer,
    ARM_SPI1_GetDataCount,
    ARM_SPI1_Control,
    ARM_SPI1_GetStatus
};
#endif /* RTE_SPI1 */

/* SPI2 driver instance */
#if RTE_SPI2

#if RTE_SPI2_DMA_ENABLE
static void SPI2_DMACallback(uint32_t event, int8_t peri_num);
static SPI_DMA_HW_CONFIG SPI2_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI2_DMA),
        .dma_periph_req = SPI2_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI2_DMA,
             .group    = SPI2_DMA_GROUP,
             .channel  = SPI2_DMA_RX_PERIPH_REQ,
             .enable_handshake = SPI2_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI2_DMA),
        .dma_periph_req = SPI2_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI2_DMA,
             .group    = SPI2_DMA_GROUP,
             .channel  = SPI2_DMA_TX_PERIPH_REQ,
             .enable_handshake = SPI2_DMA_HANDSHAKE_ENABLE,
        },

    },
};
#endif

#if RTE_SPI2_USE_MASTER_SS_SW
/* GPIO driver Instance for SPI2 SW controlled slave select */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_SPI2_SW_SPI_PORT);
#endif

static SPI_RESOURCES SPI2_RES = {
    .regs                   = (SPI_Type*) SPI2_BASE,
    .cb_event               = NULL,
    .irq_priority           = RTE_SPI2_IRQ_PRIORITY,
    .drv_instance           = SPI_INSTANCE_2,
    .slave_select           = RTE_SPI2_CHIP_SELECTION_PIN,
    .tx_fifo_threshold      = RTE_SPI2_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_SPI2_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_SPI2_RX_FIFO_THRESHOLD,
    .rx_sample_delay        = RTE_SPI2_RX_SAMPLE_DELAY,
    .sste_enable            = RTE_SPI2_SSTE_ENABLE,
    .irq                    = SPI2_IRQ_IRQn,
#if RTE_SPI2_DMA_ENABLE
    .dma_enable             = RTE_SPI2_DMA_ENABLE,
    .dma_irq_priority       = RTE_SPI2_DMA_IRQ_PRI,
    .dma_cb                 = SPI2_DMACallback,
    .dma_cfg                = &SPI2_DMA_HW_CONFIG
#endif
#if SPI_BLOCKING_MODE_ENABLE
    .blocking_mode          = RTE_SPI2_BLOCKING_MODE_ENABLE,
#endif
#if RTE_SPI2_USE_MASTER_SS_SW
    .sw_config =
    {
        .ss_port            = RTE_SPI2_SW_SPI_PORT,
        .ss_pin             = RTE_SPI2_SW_SPI_PIN,
        .active_polarity    = RTE_SPI2_SW_SPI_SS_POLARITY,
        .drvGPIO            = (ARM_DRIVER_GPIO*) &ARM_Driver_GPIO_(RTE_SPI2_SW_SPI_PORT),
    }
#endif
#if RTE_SPI2_MICROWIRE_FRF_ENABLE
    .mw_enable              = RTE_SPI2_MICROWIRE_FRF_ENABLE,
    .mw_config =
    {
#if RTE_SPI2_MW_TRANSFER_MODE
        .transfer_mode      = SPI_MW_TRANSFER_MODE_SEQUANTIAL,
#else
        .transfer_mode      = SPI_MW_TRANSFER_MODE_NON_SEQUANTIAL,
#endif
        .handshake_enable   = RTE_SPI2_MW_HANDSAHKE_ENABLE,
        .cfs                = RTE_SPI2_MW_CFS
    }
#endif
};

extern void SPI2_IRQHandler(void);
void SPI2_IRQHandler(void)
{
    SPI_IRQ_Handler(&SPI2_RES);
}

#if RTE_SPI2_DMA_ENABLE
void SPI2_DMACallback(uint32_t event, int8_t peri_num)
{
    SPI_DMACallback(&SPI2_RES, event, peri_num);
}
#endif

static int32_t ARM_SPI2_Initialize(ARM_SPI_SignalEvent_t cb_event)
{
    return ARM_SPI_Initialize(&SPI2_RES, cb_event);
}

static int32_t ARM_SPI2_Uninitialize(void)
{
    return ARM_SPI_Uninitialize(&SPI2_RES);
}

static int32_t ARM_SPI2_PowerControl(ARM_POWER_STATE state)
{
    return ARM_SPI_PowerControl(&SPI2_RES, state);
}

static int32_t ARM_SPI2_Send(const void *data, uint32_t num)
{
    return ARM_SPI_Send(&SPI2_RES, data, num);
}

static int32_t ARM_SPI2_Receive(void *data, uint32_t num)
{
    return ARM_SPI_Receive(&SPI2_RES, data, num);
}

static int32_t ARM_SPI2_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_SPI_Transfer(&SPI2_RES, data_out, data_in, num);
}

static uint32_t ARM_SPI2_GetDataCount(void)
{
    return ARM_SPI_GetDataCount(&SPI2_RES);
}

static int32_t ARM_SPI2_Control(uint32_t control, uint32_t arg)
{
    return ARM_SPI_Control(&SPI2_RES, control, arg);
}

static ARM_SPI_STATUS ARM_SPI2_GetStatus(void)
{
    return ARM_SPI_GetStatus(&SPI2_RES);
}

extern ARM_DRIVER_SPI Driver_SPI2;
ARM_DRIVER_SPI Driver_SPI2 = {
    ARM_SPI_GetVersion,
    ARM_SPI_GetCapabilities,
    ARM_SPI2_Initialize,
    ARM_SPI2_Uninitialize,
    ARM_SPI2_PowerControl,
    ARM_SPI2_Send,
    ARM_SPI2_Receive,
    ARM_SPI2_Transfer,
    ARM_SPI2_GetDataCount,
    ARM_SPI2_Control,
    ARM_SPI2_GetStatus
};
#endif /* RTE_SPI2 */

/* SPI3 driver instance */
#if RTE_SPI3

#if RTE_SPI3_DMA_ENABLE
static void SPI3_DMACallback(uint32_t event, int8_t peri_num);
static SPI_DMA_HW_CONFIG SPI3_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI3_DMA),
        .dma_periph_req = SPI3_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI3_DMA,
             .group    = SPI3_DMA_GROUP,
             .channel  = SPI3_DMA_RX_PERIPH_REQ,
             .enable_handshake = SPI3_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(SPI3_DMA),
        .dma_periph_req = SPI3_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = SPI3_DMA,
             .group    = SPI3_DMA_GROUP,
             .channel  = SPI3_DMA_TX_PERIPH_REQ,
             .enable_handshake = SPI3_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

#if RTE_SPI3_USE_MASTER_SS_SW
/* GPIO driver Instance for SPI3 SW controlled slave select */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_SPI3_SW_SPI_PORT);
#endif

static SPI_RESOURCES SPI3_RES = {
    .regs                   = (SPI_Type*) SPI3_BASE,
    .cb_event               = NULL,
    .irq_priority           = RTE_SPI3_IRQ_PRIORITY,
    .drv_instance           = SPI_INSTANCE_3,
    .slave_select           = RTE_SPI3_CHIP_SELECTION_PIN,
    .tx_fifo_threshold      = RTE_SPI3_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_SPI3_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_SPI3_RX_FIFO_THRESHOLD,
    .rx_sample_delay        = RTE_SPI3_RX_SAMPLE_DELAY,
    .sste_enable            = RTE_SPI3_SSTE_ENABLE,
    .irq                    = SPI3_IRQ_IRQn,
#if RTE_SPI3_DMA_ENABLE
    .dma_enable             = RTE_SPI3_DMA_ENABLE,
    .dma_irq_priority       = RTE_SPI3_DMA_IRQ_PRI,
    .dma_cb                 = SPI3_DMACallback,
    .dma_cfg                = &SPI3_DMA_HW_CONFIG,
#endif
#if SPI_BLOCKING_MODE_ENABLE
    .blocking_mode          = RTE_SPI3_BLOCKING_MODE_ENABLE,
#endif
#if RTE_SPI3_USE_MASTER_SS_SW
    .sw_config =
    {
        .ss_port            = RTE_SPI3_SW_SPI_PORT,
        .ss_pin             = RTE_SPI3_SW_SPI_PIN,
        .active_polarity    = RTE_SPI3_SW_SPI_SS_POLARITY,
        .drvGPIO            = (ARM_DRIVER_GPIO*) &ARM_Driver_GPIO_(RTE_SPI3_SW_SPI_PORT),
    }
#endif
#if RTE_SPI3_MICROWIRE_FRF_ENABLE
    .mw_enable              = RTE_SPI3_MICROWIRE_FRF_ENABLE,
    .mw_config =
    {
#if RTE_SPI3_MW_TRANSFER_MODE
        .transfer_mode      = SPI_MW_TRANSFER_MODE_SEQUANTIAL,
#else
        .transfer_mode      = SPI_MW_TRANSFER_MODE_NON_SEQUANTIAL,
#endif
        .handshake_enable   = RTE_SPI3_MW_HANDSAHKE_ENABLE,
        .cfs                = RTE_SPI3_MW_CFS
    }
#endif
};

extern void SPI3_IRQHandler(void);
void SPI3_IRQHandler(void)
{
    SPI_IRQ_Handler(&SPI3_RES);
}

#if RTE_SPI3_DMA_ENABLE
void SPI3_DMACallback(uint32_t event, int8_t peri_num)
{
    SPI_DMACallback(&SPI3_RES, event, peri_num);
}
#endif

static int32_t ARM_SPI3_Initialize(ARM_SPI_SignalEvent_t cb_event)
{
    return ARM_SPI_Initialize(&SPI3_RES, cb_event);
}

static int32_t ARM_SPI3_Uninitialize(void)
{
    return ARM_SPI_Uninitialize(&SPI3_RES);
}

static int32_t ARM_SPI3_PowerControl(ARM_POWER_STATE state)
{
    return ARM_SPI_PowerControl(&SPI3_RES, state);
}

static int32_t ARM_SPI3_Send(const void *data, uint32_t num)
{
    return ARM_SPI_Send(&SPI3_RES, data, num);
}

static int32_t ARM_SPI3_Receive(void *data, uint32_t num)
{
    return ARM_SPI_Receive(&SPI3_RES, data, num);
}

static int32_t ARM_SPI3_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_SPI_Transfer(&SPI3_RES, data_out, data_in, num);
}

static uint32_t ARM_SPI3_GetDataCount(void)
{
    return ARM_SPI_GetDataCount(&SPI3_RES);
}

static int32_t ARM_SPI3_Control(uint32_t control, uint32_t arg)
{
    return ARM_SPI_Control(&SPI3_RES, control, arg);
}

static ARM_SPI_STATUS ARM_SPI3_GetStatus(void)
{
    return ARM_SPI_GetStatus(&SPI3_RES);
}

extern ARM_DRIVER_SPI Driver_SPI3;
ARM_DRIVER_SPI Driver_SPI3 = {
    ARM_SPI_GetVersion,
    ARM_SPI_GetCapabilities,
    ARM_SPI3_Initialize,
    ARM_SPI3_Uninitialize,
    ARM_SPI3_PowerControl,
    ARM_SPI3_Send,
    ARM_SPI3_Receive,
    ARM_SPI3_Transfer,
    ARM_SPI3_GetDataCount,
    ARM_SPI3_Control,
    ARM_SPI3_GetStatus
};
#endif /* RTE_SPI3 */

/* LPSPI driver instance */
#if RTE_LPSPI

#if RTE_LPSPI_DMA_ENABLE
static void LPSPI_DMACallback(uint32_t event, int8_t peri_num);
static SPI_DMA_HW_CONFIG LPSPI_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(LPSPI_DMA),
        .dma_periph_req = LPSPI_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = LPSPI_DMA,
             .group    = LPSPI_DMA_GROUP,
             .channel  = LPSPI_DMA_RX_PERIPH_REQ,
             .enable_handshake = LPSPI_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(LPSPI_DMA),
        .dma_periph_req = LPSPI_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = LPSPI_DMA,
             .group    = LPSPI_DMA_GROUP,
             .channel  = LPSPI_DMA_TX_PERIPH_REQ,
             .enable_handshake = LPSPI_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif

#if RTE_LPSPI_USE_MASTER_SS_SW
/* GPIO driver Instance for LPSPI SW controlled slave select */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(RTE_LPSPI_SW_SPI_PORT);
#endif

static SPI_RESOURCES LPSPI_RES = {
    .regs                   = (SPI_Type*) LPSPI_BASE,
    .cb_event               = NULL,
    .irq_priority           = RTE_LPSPI_IRQ_PRIORITY,
    .drv_instance           = LPSPI_INSTANCE,
    .slave_select           = RTE_LPSPI_CHIP_SELECTION_PIN,
    .tx_fifo_threshold      = RTE_LPSPI_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_LPSPI_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_LPSPI_RX_FIFO_THRESHOLD,
    .sste_enable            = RTE_LPSPI_SSTE_ENABLE,
    .irq                    = LPSPI_IRQ_IRQn,
#if RTE_LPSPI_DMA_ENABLE
    .dma_enable             = RTE_LPSPI_DMA_ENABLE,
    .dma_irq_priority       = RTE_LPSPI_DMA_IRQ_PRI,
    .dma_cb                 = LPSPI_DMACallback,
    .dma_cfg                = &LPSPI_DMA_HW_CONFIG
#endif
#if SPI_BLOCKING_MODE_ENABLE
    .blocking_mode          = RTE_LPSPI_BLOCKING_MODE_ENABLE,
#endif
#if RTE_LPSPI_USE_MASTER_SS_SW
    .sw_config =
    {
        .ss_port            = RTE_LPSPI_SW_SPI_PORT,
        .ss_pin             = RTE_LPSPI_SW_SPI_PIN,
        .active_polarity    = RTE_LPSPI_SW_SPI_SS_POLARITY,
        .drvGPIO            = (ARM_DRIVER_GPIO*) &ARM_Driver_GPIO_(RTE_LPSPI_SW_SPI_PORT),
    }
#endif
#if RTE_LPSPI_MICROWIRE_FRF_ENABLE
    .mw_enable              = RTE_LPSPI_MICROWIRE_FRF_ENABLE,
    .mw_config =
    {
#if RTE_LPSPI_MW_TRANSFER_MODE
        .transfer_mode      = SPI_MW_TRANSFER_MODE_SEQUANTIAL,
#else
        .transfer_mode      = SPI_MW_TRANSFER_MODE_NON_SEQUANTIAL,
#endif
        .handshake_enable   = RTE_LPSPI_MW_HANDSAHKE_ENABLE,
        .cfs                = RTE_LPSPI_MW_CFS
    }
#endif
};

extern void LPSPI_IRQHandler(void);
void LPSPI_IRQHandler(void)
{
    SPI_IRQ_Handler(&LPSPI_RES);
}

#if RTE_LPSPI_DMA_ENABLE
void LPSPI_DMACallback(uint32_t event, int8_t peri_num)
{
    SPI_DMACallback(&LPSPI_RES, event, peri_num);
}
#endif

static int32_t ARM_LPSPI_Initialize(ARM_SPI_SignalEvent_t cb_event)
{
    return ARM_SPI_Initialize(&LPSPI_RES, cb_event);
}

static int32_t ARM_LPSPI_Uninitialize(void)
{
    return ARM_SPI_Uninitialize(&LPSPI_RES);
}

static int32_t ARM_LPSPI_PowerControl(ARM_POWER_STATE state)
{
    return ARM_SPI_PowerControl(&LPSPI_RES, state);
}

static int32_t ARM_LPSPI_Send(const void *data, uint32_t num)
{
    return ARM_SPI_Send(&LPSPI_RES, data, num);
}

static int32_t ARM_LPSPI_Receive(void *data, uint32_t num)
{
    return ARM_SPI_Receive(&LPSPI_RES, data, num);
}

static int32_t ARM_LPSPI_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_SPI_Transfer(&LPSPI_RES, data_out, data_in, num);
}

static uint32_t ARM_LPSPI_GetDataCount(void)
{
    return ARM_SPI_GetDataCount(&LPSPI_RES);
}

static int32_t ARM_LPSPI_Control(uint32_t control, uint32_t arg)
{
    return ARM_SPI_Control(&LPSPI_RES, control, arg);
}

static ARM_SPI_STATUS ARM_LPSPI_GetStatus(void)
{
    return ARM_SPI_GetStatus(&LPSPI_RES);
}

extern ARM_DRIVER_SPI Driver_SPILP;
ARM_DRIVER_SPI Driver_SPILP = {
    ARM_SPI_GetVersion,
    ARM_SPI_GetCapabilities,
    ARM_LPSPI_Initialize,
    ARM_LPSPI_Uninitialize,
    ARM_LPSPI_PowerControl,
    ARM_LPSPI_Send,
    ARM_LPSPI_Receive,
    ARM_LPSPI_Transfer,
    ARM_LPSPI_GetDataCount,
    ARM_LPSPI_Control,
    ARM_LPSPI_GetStatus
};
#endif /* RTE_LPSPI */
