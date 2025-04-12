/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     Driver_OSPI.c
 * @author   Khushboo Singh
 * @email    khushboo.singh@alifsemi.com
 * @version  V1.0.0
 * @date     21-Oct-2022
 * @brief    CMSIS-Driver for OSPI derived from CMSIS SPI driver.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_OSPI_Private.h"

#if !( (RTE_OSPI0) || (RTE_OSPI1) )
#error "OSPI is not enabled in the RTE_Device.h"
#endif

#if !defined(RTE_Drivers_OSPI)
#error "OSPI is not enabled in the RTE_Components.h"
#endif

#define ARM_OSPI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_OSPI_API_VERSION,
    ARM_OSPI_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_OSPI_CAPABILITIES DriverCapabilities = {
    0, /* Simplex Mode (Master and Slave) Deprecated: Reserved (must be zero)*/
    0, /* TI Synchronous Serial Interface */
    0, /* Microwire Interface */
    0, /* Signal Mode Fault event: \ref ARM_OSPI_EVENT_MODE_FAULT */
    0  /* Reserved, must be zero */
};

/**
 * @fn      ARM_DRIVER_VERSION ARM_OSPI_GetVersion(void)
 * @brief   get ospi version
 * @note    none
 * @param   none
 * @retval  driver version
 */
static ARM_DRIVER_VERSION ARM_OSPI_GetVersion(void)
{
    return DriverVersion;
}

/**
 * @fn      ARM_OSPI_CAPABILITIES ARM_OSPI_GetCapabilities(void)
 * @brief   get ospi capabilities
 * @note    none
 * @param   none
 * @retval  driver capabilities
 */
static ARM_OSPI_CAPABILITIES ARM_OSPI_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
 * @fn      void OSPI_IRQHandler(OSPI_RESOURCES *OSPI)
 * @brief   OSPI interrupt handler
 * @note    none
 * @param   OSPI : Pointer to ospi resources structure
 * @retval  none
 */
static void OSPI_IRQHandler(OSPI_RESOURCES *OSPI)
{
    ospi_transfer_t *transfer = &(OSPI->transfer);

    ospi_irq_handler(OSPI->regs, transfer);

    if (transfer->status == SPI_TRANSFER_STATUS_COMPLETE)
    {
        transfer->status        = SPI_TRANSFER_STATUS_NONE;
        OSPI->status.busy       = 0;
        OSPI->cb_event(ARM_OSPI_EVENT_TRANSFER_COMPLETE);
    }

    if (transfer->status == SPI_TRANSFER_STATUS_OVERFLOW)
    {
        transfer->status        = SPI_TRANSFER_STATUS_NONE;
        OSPI->status.busy       = 0;
        OSPI->status.data_lost  = 1;
        OSPI->cb_event(ARM_OSPI_EVENT_DATA_LOST);
    }
}

#if OSPI_DMA_ENABLE
/**
  \fn          static void  OSPI_DMACallback(uint32_t event, int8_t peri_num,
                                             OSPI_RESOURCES *OSPI)
  \brief       Callback function from DMA for OSPI
  \param[in]   event     Event from DMA
  \param[in]   peri_num  Peripheral number
  \param[in]   OSPI      Pointer to OSPI resources
*/
static void OSPI_DMACallback(uint32_t event, int8_t peri_num,
                                  OSPI_RESOURCES *OSPI)
{
    if (!OSPI->cb_event)
        return;

    if (event & ARM_DMA_EVENT_COMPLETE)
    {
        switch (peri_num)
        {
        case OSPI0_DMA_RX_PERIPH_REQ:
        case OSPI1_DMA_RX_PERIPH_REQ:
            OSPI->status.busy = 0;
            OSPI->cb_event(ARM_OSPI_EVENT_TRANSFER_COMPLETE);
            break;
        case OSPI0_DMA_TX_PERIPH_REQ:
        case OSPI1_DMA_TX_PERIPH_REQ:
            if (OSPI->transfer.mode == SPI_TMOD_TX)
            {
                OSPI->status.busy = 0;
                OSPI->cb_event(ARM_OSPI_EVENT_TRANSFER_COMPLETE);
            }
            break;

        default:
            break;
        }
    }

    if (event & ARM_DMA_EVENT_ABORT)
    {
        OSPI->status.busy = 0;
        OSPI->cb_event(ARM_OSPI_EVENT_DATA_LOST);
    }
}

/**
  \fn          int32_t OSPI_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Initialize DMA for OSPI
  \param[in]   dma_periph   Pointer to DMA resources
  \return      \ref         execution_status
*/
__STATIC_INLINE int32_t OSPI_DMA_Initialize(DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    status = dma_drv->Initialize();

    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t OSPI_DMA_PowerControl(ARM_POWER_STATE state, DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       PowerControl DMA for OSPI
  \param[in]   state  Power state
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t OSPI_DMA_PowerControl(ARM_POWER_STATE state, DMA_PERIPHERAL_CONFIG *dma_periph)
{
    int32_t        status;
    ARM_DRIVER_DMA *dma_drv = dma_periph->dma_drv;

    status = dma_drv->PowerControl(state);

    if (status)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t OSPI_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Allocate a channel for OSPI
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t OSPI_DMA_Allocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t OSPI_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       De-allocate channel of OSPI
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t OSPI_DMA_DeAllocate(DMA_PERIPHERAL_CONFIG *dma_periph)
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
  \fn          int32_t OSPI_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                      ARM_DMA_PARAMS *dma_params)
  \brief       Start OSPI DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \param[in]   dma_params     Pointer to DMA parameters
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t OSPI_DMA_Start(DMA_PERIPHERAL_CONFIG *dma_periph,
                                       ARM_DMA_PARAMS *dma_params)
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
  \fn          int32_t OSPI_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
  \brief       Stop OSPI DMA transfer
  \param[in]   dma_periph     Pointer to DMA resources
  \return      \ref execution_status
*/
__STATIC_INLINE int32_t OSPI_DMA_Stop(DMA_PERIPHERAL_CONFIG *dma_periph)
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
#endif

/**
 * @fn      int32_t ARM_OSPI_Initialize(OSPI_RESOURCES *OSPI, ARM_OSPI_SignalEvent_t cb_event).
 * @brief   Initialize the Spi for communication.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @param   cb_event : Pointer to user callback function.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_Initialize(OSPI_RESOURCES *OSPI, ARM_OSPI_SignalEvent_t cb_event)
{
    if (OSPI->state.initialized == 1)
    {
        return ARM_DRIVER_OK;
    }

    if (cb_event == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((OSPI->tx_fifo_threshold > OSPI_TX_FIFO_DEPTH) || (OSPI->tx_fifo_start_level > OSPI_TX_FIFO_DEPTH))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (OSPI->rx_fifo_threshold > OSPI_RX_FIFO_DEPTH)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    OSPI->transfer.addr_len             = 0;
    OSPI->transfer.dummy_cycle          = 0;
    OSPI->transfer.tx_buff              = NULL;
    OSPI->transfer.rx_buff              = NULL;
    OSPI->transfer.tx_default_val       = 0;
    OSPI->transfer.tx_default_enable    = 0;
    OSPI->transfer.tx_total_cnt         = 0;
    OSPI->transfer.rx_total_cnt         = 0;
    OSPI->transfer.tx_current_cnt       = 0;
    OSPI->transfer.rx_current_cnt       = 0;

    OSPI->cb_event                      = cb_event;

#if OSPI_DMA_ENABLE
    if (OSPI->dma_enable)
    {
        OSPI->dma_config->dma_rx.dma_handle = -1;
        OSPI->dma_config->dma_tx.dma_handle = -1;

        /* Initialize DMA for OSPI-Tx */
        if (OSPI_DMA_Initialize(&OSPI->dma_config->dma_tx) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }

        /* Initialize DMA for OSPI-Rx */
        if (OSPI_DMA_Initialize(&OSPI->dma_config->dma_rx) != ARM_DRIVER_OK)
        {
            return ARM_DRIVER_ERROR;
        }
    }
#endif
    OSPI->state.initialized             = 1;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_OSPI_Uninitialize(OSPI_RESOURCES *OSPI).
 * @brief   Un-Initialize the Spi.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_Uninitialize(OSPI_RESOURCES *OSPI)
{
    if (OSPI->state.initialized == 0)
    {
        return ARM_DRIVER_OK;
    }

    if (OSPI->state.powered == 1)
    {
        return ARM_DRIVER_ERROR;
    }

    OSPI->transfer.tx_buff              = NULL;
    OSPI->transfer.rx_buff              = NULL;
    OSPI->transfer.tx_default_val       = 0;
    OSPI->transfer.tx_default_enable    = 0;
    OSPI->transfer.tx_total_cnt         = 0;
    OSPI->transfer.rx_total_cnt         = 0;
    OSPI->transfer.tx_current_cnt       = 0;
    OSPI->transfer.rx_current_cnt       = 0;

    OSPI->cb_event                      = NULL;
    OSPI->state.initialized             = 0;

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_OSPI_PowerControl(OSPI_RESOURCES *OSPI, ARM_POWER_STATE state).
 * @brief   Handles the spi power.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @param   state : power state.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_PowerControl(OSPI_RESOURCES *OSPI, ARM_POWER_STATE state)
{
    if (OSPI->state.initialized == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (state)
    {
        case ARM_POWER_OFF:
        {
            if (OSPI->state.powered == 0)
            {
                return ARM_DRIVER_OK;
            }

            if (ospi_busy(OSPI->regs))
            {
                return ARM_DRIVER_ERROR_BUSY;
            }

            ospi_mask_interrupts(OSPI->regs);
            NVIC_ClearPendingIRQ(OSPI->irq);
            NVIC_DisableIRQ(OSPI->irq);
            ospi_disable(OSPI->regs);
#if OSPI_DMA_ENABLE
            if (OSPI->dma_enable)
            {
                ospi_disable_tx_dma(OSPI->regs);
                ospi_disable_rx_dma(OSPI->regs);
                if (OSPI_DMA_DeAllocate(&OSPI->dma_config->dma_tx) == ARM_DRIVER_ERROR)
                {
                    return ARM_DRIVER_ERROR;
                }

                if (OSPI_DMA_DeAllocate(&OSPI->dma_config->dma_rx) == ARM_DRIVER_ERROR)
                {
                    return ARM_DRIVER_ERROR;
                }
            }
#endif
            OSPI->state.powered = 0;
            break;
        }

        case ARM_POWER_FULL:
        {
            if (OSPI->state.powered == 1)
            {
                return ARM_DRIVER_OK;
            }

            ospi_set_tx_threshold(OSPI->regs, OSPI->tx_fifo_threshold);
            ospi_set_rx_threshold(OSPI->regs, OSPI->rx_fifo_threshold);
            ospi_set_rx_sample_delay(OSPI->regs, OSPI->rx_sample_delay);
            ospi_set_ddr_drive_edge(OSPI->regs, OSPI->ddr_drive_edge);
            aes_set_rxds_delay(OSPI->aes_regs, OSPI->rxds_delay);
            ospi_mask_interrupts(OSPI->regs);
#if OSPI_DMA_ENABLE
            if (OSPI->dma_enable)
            {
                ospi_set_rx_dma_data_level(OSPI->regs, OSPI->rx_dma_data_level);
                ospi_set_tx_dma_data_level(OSPI->regs, OSPI->tx_dma_data_level);
            }
#endif
            NVIC_ClearPendingIRQ(OSPI->irq);
            NVIC_SetPriority(OSPI->irq, OSPI->irq_priority);
            NVIC_EnableIRQ(OSPI->irq);

            OSPI->state.powered = 1;
            break;
        }

        case ARM_POWER_LOW:
        default :
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }


#if OSPI_DMA_ENABLE
    if (OSPI->dma_enable)
    {
        /* Power Control DMA for OSPI-Tx */
        if (OSPI_DMA_PowerControl (state, &OSPI->dma_config->dma_tx) != ARM_DRIVER_OK)
        {
            OSPI->state.powered = 0;
            return ARM_DRIVER_ERROR;
        }

        /* Power Control DMA for OSPI-Rx */
        if (OSPI_DMA_PowerControl (state, &OSPI->dma_config->dma_rx) != ARM_DRIVER_OK)
        {
            OSPI->state.powered = 0;
            return ARM_DRIVER_ERROR;
        }

        if (state == ARM_POWER_FULL)
        {
            /* Try to allocate a DMA channel for Tx */
            if (OSPI_DMA_Allocate(&OSPI->dma_config->dma_tx) == ARM_DRIVER_ERROR)
            {
                OSPI->state.powered = 0;
                return ARM_DRIVER_ERROR;
            }

            /* Try to allocate a DMA channel for Rx */
            if (OSPI_DMA_Allocate(&OSPI->dma_config->dma_rx) == ARM_DRIVER_ERROR)
            {
                OSPI->state.powered = 0;
                return ARM_DRIVER_ERROR;
            }
        }
    }
#endif

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_OSPI_Send(OSPI_RESOURCES *OSPI, const void *data, uint32_t num).
 * @brief   Used to send through spi.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @param   data : Pointer to the data to send.
 * @param   num : Number of data byte to send.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_Send(OSPI_RESOURCES *OSPI, const void *data, uint32_t num)
{
    #if OSPI_DMA_ENABLE
    int32_t status;
    #endif

    if (OSPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if (ospi_busy(OSPI->regs))
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    if ((data == NULL) && (OSPI->transfer.tx_default_enable == false))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (num == 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (OSPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    OSPI->status.busy = 1;
    OSPI->transfer.tx_total_cnt      = num;
    OSPI->transfer.mode              = SPI_TMOD_TX;

#if OSPI_DMA_ENABLE
    ARM_DMA_PARAMS dma_params;

    if (OSPI->dma_enable && (ospi_get_dfs(OSPI->regs) > 8))
    {
        dma_params.peri_reqno   = OSPI->dma_config->dma_tx.dma_periph_req;
        dma_params.dir          = ARM_DMA_MEM_TO_DEV;
        dma_params.cb_event     = OSPI->dma_cb;
        dma_params.src_addr     = (void *) data;
        dma_params.dst_addr     = (void *) ospi_get_dma_addr(OSPI->regs);
        dma_params.num_bytes    = num * 4;
        dma_params.irq_priority = OSPI->dma_irq_priority;
        dma_params.burst_size   = BS_BYTE_4;
        dma_params.burst_len    = 16;

        ospi_dma_send(OSPI->regs, &(OSPI->transfer));

        /* Start DMA transfer */
        status = OSPI_DMA_Start(&OSPI->dma_config->dma_tx, &dma_params);

        if (status)
        {
            OSPI->status.busy = 0;
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        OSPI->transfer.tx_buff           = data;
        OSPI->transfer.tx_current_cnt    = 0;
        OSPI->transfer.status            = SPI_TRANSFER_STATUS_NONE;
        ospi_send(OSPI->regs, &(OSPI->transfer));
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_OSPI_Receive(OSPI_RESOURCES *OSPI, void *data, uint32_t num).
 * @brief   Used to receive data through spi.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @param   data : Pointer to the data received.
 * @param   num : Number of data byte to receive.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_Receive(OSPI_RESOURCES *OSPI, void *data, uint32_t num)
{
    if (OSPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if (ospi_busy(OSPI->regs))
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    if ((data == NULL) || (num == 0))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (OSPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    OSPI->status.busy = 1;

    OSPI->transfer.rx_buff          = (uint8_t *) data;
    OSPI->transfer.rx_current_cnt   = 0;
    OSPI->transfer.rx_total_cnt     = num;
    OSPI->transfer.mode             = SPI_TMOD_RX;
    OSPI->transfer.status           = SPI_TRANSFER_STATUS_NONE;

    ospi_receive(OSPI->regs, &(OSPI->transfer));

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_OSPI_Transfer(OSPI_RESOURCES *OSPI, const void *data_out, void *data_in, uint32_t num).
 * @brief   Used to Transfer and Receive data through spi.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @param   data_out : Pointer to the data send.
 * @param   data_in : Pointer to the data received.
 * @param   num : Number of data byte to receive.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_Transfer(OSPI_RESOURCES *OSPI, const void *data_out, void *data_in, uint32_t num)
{
    #if OSPI_DMA_ENABLE
    int32_t status;
    #endif

    if (OSPI->state.powered == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    if (ospi_busy(OSPI->regs))
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    if ((OSPI->transfer.tx_default_enable == 0) && (data_out == NULL))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((data_out == NULL) || (data_in == NULL) || (num == 0))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (OSPI->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    OSPI->status.busy = 1;

    OSPI->transfer.rx_total_cnt   = num;
    OSPI->transfer.mode           = SPI_TMOD_TX_AND_RX;

    /* Tx total count based on address length */
    if (OSPI->transfer.addr_len == ARM_OSPI_ADDR_LENGTH_0_BITS)
        OSPI->transfer.tx_total_cnt = 1;
    else if (OSPI->transfer.addr_len == ARM_OSPI_ADDR_LENGTH_24_BITS)
        OSPI->transfer.tx_total_cnt = 4;
    else if (OSPI->transfer.addr_len == ARM_OSPI_ADDR_LENGTH_32_BITS)
        OSPI->transfer.tx_total_cnt = 2;

#if OSPI_DMA_ENABLE
    ARM_DMA_PARAMS tx_dma_params, rx_dma_params;
    /*
     * DMA transfers with 8 bit DFS are not supported because of the
     * h/w limitation on 8bit reception
    */
    uint8_t dfs = ospi_get_dfs(OSPI->regs);

    if (OSPI->dma_enable && dfs > 8)
    {
        tx_dma_params.peri_reqno   = OSPI->dma_config->dma_tx.dma_periph_req;
        tx_dma_params.dir          = ARM_DMA_MEM_TO_DEV;
        tx_dma_params.cb_event     = OSPI->dma_cb;
        tx_dma_params.src_addr     = (void *) data_out;
        tx_dma_params.dst_addr     = (void *) ospi_get_dma_addr(OSPI->regs);
        tx_dma_params.num_bytes    = OSPI->transfer.tx_total_cnt * 4;
        tx_dma_params.irq_priority = OSPI->dma_irq_priority;
        tx_dma_params.burst_size   = BS_BYTE_4;
        tx_dma_params.burst_len    = 16;

        rx_dma_params.peri_reqno   = OSPI->dma_config->dma_rx.dma_periph_req;
        rx_dma_params.dir          = ARM_DMA_DEV_TO_MEM;
        rx_dma_params.cb_event     = OSPI->dma_cb;
        rx_dma_params.src_addr     = (void *) ospi_get_dma_addr(OSPI->regs);
        rx_dma_params.dst_addr     = data_in;
        rx_dma_params.irq_priority = OSPI->dma_irq_priority;
        rx_dma_params.burst_len    = 16;

        if (dfs == 16)
        {
            rx_dma_params.num_bytes    = num * 2;
            rx_dma_params.burst_size   = BS_BYTE_2;
        }
        else if (dfs == 32)
        {
            rx_dma_params.num_bytes    = num * 4;
            rx_dma_params.burst_size   = BS_BYTE_4;
        }

        ospi_dma_transfer(OSPI->regs, &(OSPI->transfer));

        /* Start DMA transfers */
        status = OSPI_DMA_Start(&OSPI->dma_config->dma_rx, &rx_dma_params);

        if (status)
        {
            OSPI->status.busy = 0;
            return ARM_DRIVER_ERROR;
        }

        status = OSPI_DMA_Start(&OSPI->dma_config->dma_tx, &tx_dma_params);

        if (status)
        {
            OSPI->status.busy = 0;
            return ARM_DRIVER_ERROR;
        }
    }
    else
#endif
    {
        OSPI->transfer.tx_buff        = data_out;
        OSPI->transfer.rx_buff        = data_in;
        OSPI->transfer.tx_current_cnt = 0;
        OSPI->transfer.rx_current_cnt = 0;
        OSPI->transfer.status         = SPI_TRANSFER_STATUS_NONE;

        ospi_transfer(OSPI->regs, &(OSPI->transfer));
    }

    return ARM_DRIVER_OK;
}

/**
 * @fn      int32_t ARM_OSPI_GetDataCount(OSPI_RESOURCES *OSPI).
 * @brief   Used to get the data count on spi transfer.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @retval  \ref data count
 */
static uint32_t ARM_OSPI_GetDataCount(OSPI_RESOURCES *OSPI)
{
    if (OSPI->transfer.mode == SPI_TMOD_TX)
        return OSPI->transfer.tx_current_cnt;
    else
        return OSPI->transfer.rx_current_cnt;
}

/**
 * @fn      int32_t ARM_OSPI_Control(OSPI_RESOURCES *OSPI, uint32_t control, uint32_t arg).
 * @brief   Used to configure spi.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @param   control : control code.
 * @param   arg : argument.
 * @retval  \ref execution_status
 */
static int32_t ARM_OSPI_Control(OSPI_RESOURCES *OSPI, uint32_t control, uint32_t arg)
{
    int32_t ret = ARM_DRIVER_OK;
    uint32_t clk;

    if (ospi_busy(OSPI->regs))
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    switch (control & ARM_OSPI_CONTROL_MSK)
    {
        case ARM_OSPI_MODE_INACTIVE:
        {
            if (control == 0)
            {
                ospi_disable(OSPI->regs);
                return ARM_DRIVER_OK;
            }
            break;
        }

        case ARM_OSPI_MODE_MASTER:
        {
            ospi_mode_master(OSPI->regs);
            clk = getOSPICoreClock();
            ospi_set_bus_speed(OSPI->regs, arg, clk);
            break;
        }

        case ARM_OSPI_SET_BUS_SPEED:
        {
            clk = getOSPICoreClock();
            ospi_set_bus_speed(OSPI->regs, arg, clk);
            return ARM_DRIVER_OK;
        }

        case ARM_OSPI_GET_BUS_SPEED:
        {
            clk = getOSPICoreClock();
            return (int32_t) ospi_get_bus_speed(OSPI->regs, clk);
        }

        case ARM_OSPI_SET_DEFAULT_TX_VALUE:
        {
            OSPI->transfer.tx_default_val 	 = arg;
            OSPI->transfer.tx_default_enable = true;

            return ARM_DRIVER_OK;
        }

        case ARM_OSPI_CONTROL_SS:
        {
            if (arg == 1)
            {
                ospi_control_ss(OSPI->regs, OSPI->chip_selection_pin, SPI_SS_STATE_ENABLE);
            }
            else if (arg == 0)
            {
                ospi_control_ss(OSPI->regs, OSPI->chip_selection_pin, SPI_SS_STATE_DISABLE);
            }
            else
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            return ARM_DRIVER_OK;
        }

        case ARM_OSPI_ABORT_TRANSFER:
        {
#if OSPI_DMA_ENABLE
            if (OSPI->dma_enable)
            {
                if (OSPI->transfer.mode == SPI_TMOD_TX)
                {
                    if (OSPI_DMA_Stop(&OSPI->dma_config->dma_tx) != ARM_DRIVER_OK)
                    {
                        return ARM_DRIVER_ERROR;
                    }

                }
                else if (OSPI->transfer.mode == SPI_TMOD_TX_AND_RX)
                {
                    if (OSPI_DMA_Stop(&OSPI->dma_config->dma_tx) != ARM_DRIVER_OK)
                    {
                        return ARM_DRIVER_ERROR;
                    }
                    if (OSPI_DMA_Stop(&OSPI->dma_config->dma_rx) != ARM_DRIVER_OK)
                    {
                        return ARM_DRIVER_ERROR;
                    }
                }
            }
#endif
            ospi_mask_interrupts(OSPI->regs);
            ospi_disable(OSPI->regs);
            ospi_enable(OSPI->regs);

            OSPI->transfer.tx_buff              = NULL;
            OSPI->transfer.rx_buff              = NULL;
            OSPI->transfer.tx_default_val       = 0;
            OSPI->transfer.tx_default_enable    = 0;
            OSPI->transfer.tx_total_cnt         = 0;
            OSPI->transfer.rx_total_cnt         = 0;
            OSPI->transfer.tx_current_cnt       = 0;
            OSPI->transfer.rx_current_cnt       = 0;
            OSPI->status.busy          			= 0;

            return ARM_DRIVER_OK;
        }

        case ARM_OSPI_SET_ADDR_LENGTH_WAIT_CYCLE:
        {
            OSPI->transfer.addr_len    = 0xF & arg;
            OSPI->transfer.dummy_cycle = 0xFF & (arg >> ARM_OSPI_WAIT_CYCLE_POS);
            break;
        }

        case ARM_OSPI_SET_FRAME_FORMAT:
        {
            OSPI->transfer.spi_frf = arg;
            break;
        }

        case ARM_OSPI_SET_DDR_MODE:
        {
            OSPI->transfer.ddr = arg;
            break;
        }

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    switch (control & ARM_OSPI_FRAME_FORMAT_MSK)
    {
        case ARM_OSPI_CPOL0_CPHA0:
        {
            ospi_set_mode(OSPI->regs, SPI_MODE_0);
            break;
        }
        case ARM_OSPI_CPOL0_CPHA1:
        {
            ospi_set_mode(OSPI->regs, SPI_MODE_1);
            break;
        }
        case ARM_OSPI_CPOL1_CPHA0:
        {
            ospi_set_mode(OSPI->regs, SPI_MODE_2);
            break;
        }
        case ARM_OSPI_CPOL1_CPHA1:
        {
            ospi_set_mode(OSPI->regs, SPI_MODE_3);
            break;
        }
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    if (control & ARM_OSPI_DATA_BITS_MSK)
    {
        ospi_set_dfs(OSPI->regs, ((control & ARM_OSPI_DATA_BITS_MSK) >> ARM_OSPI_DATA_BITS_POS));
    }

    switch (control & ARM_OSPI_BIT_ORDER_MSK)
    {
        case ARM_OSPI_MSB_LSB:
        {
            break;
        }
        case ARM_OSPI_LSB_MSB:
        {
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }
    }

    switch (control & ARM_OSPI_SS_MASTER_MODE_MSK)
    {
        case ARM_OSPI_SS_MASTER_UNUSED:
        {
            ospi_control_ss(OSPI->regs, 0xF, SPI_SS_STATE_DISABLE);
            break;
        }

        case ARM_OSPI_SS_MASTER_SW:
        {
            if ((control & ARM_OSPI_CONTROL_MSK) == ARM_OSPI_MODE_MASTER)
            {
                return ARM_DRIVER_OK;
            }
            return ARM_DRIVER_ERROR;
        }

        case ARM_OSPI_SS_MASTER_HW_OUTPUT:
        {
            /* This is the default state in the IP, No need to configure */
            return ARM_DRIVER_OK;
        }

        case ARM_OSPI_SS_MASTER_HW_INPUT:
        {
            break;
        }
    }
    return ret;
}

/**
 * @fn      uint32_t ARM_OSPI_GetStatus(OSPI_RESOURCES *OSPI)
 * @brief   Used to get spi status.
 * @note    none.
 * @param   OSPI : Pointer to spi resources structure.
 * @retval  \ref spi driver status.
 */
static ARM_OSPI_STATUS ARM_OSPI_GetStatus(OSPI_RESOURCES *OSPI)
{
    return OSPI->status;
}

/* OSPI0 driver instance */
#if RTE_OSPI0

#if RTE_OSPI0_DMA_ENABLE
static void OSPI0_DMACallback (uint32_t event, int8_t peri_num);

static OSPI_DMA_HW_CONFIG OSPI0_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(OSPI0_DMA),
        .dma_periph_req = OSPI0_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = OSPI0_DMA,
             .group    = OSPI0_DMA_GROUP,
             .channel  = OSPI0_DMA_RX_PERIPH_REQ,
             .enable_handshake = OSPI0_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(OSPI0_DMA),
        .dma_periph_req = OSPI0_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = OSPI0_DMA,
             .group    = OSPI0_DMA_GROUP,
             .channel  = OSPI0_DMA_TX_PERIPH_REQ,
             .enable_handshake = OSPI0_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif
OSPI_RESOURCES OSPI0_RES = {
    .regs                   = (OSPI_Type *) OSPI0_BASE,
    .aes_regs               = (AES_Type *) AES0_BASE,
    .irq                    = OSPI0_IRQ_IRQn,
    .cb_event               = NULL,
    .irq_priority           = RTE_OSPI0_IRQ_PRIORITY,
    .drv_instance           = OSPI_INSTANCE_0,
    .chip_selection_pin     = RTE_OSPI0_CHIP_SELECTION_PIN,
    .transfer.spi_frf       = RTE_OSPI0_SPI_FRAME_FORMAT,
    .tx_fifo_threshold      = RTE_OSPI0_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_OSPI0_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_OSPI0_RX_FIFO_THRESHOLD,
#if RTE_OSPI0_DMA_ENABLE
    .dma_cb                 = OSPI0_DMACallback,
    .dma_config             = &OSPI0_DMA_HW_CONFIG,
    .dma_enable             = RTE_OSPI0_DMA_ENABLE,
    .dma_irq_priority       = RTE_OSPI0_DMA_IRQ_PRIORITY,
    .rx_dma_data_level      = RTE_OSPI0_RX_DMA_DATA_LEVEL,
    .tx_dma_data_level      = RTE_OSPI0_TX_DMA_DATA_LEVEL,
#endif
    .ddr_drive_edge         = RTE_OSPI0_DDR_DRIVE_EDGE,
    .rx_sample_delay        = RTE_OSPI0_RX_SAMPLE_DELAY,
    .rxds_delay             = RTE_OSPI0_RXDS_DELAY
};

void OSPI0_IRQHandler(void)
{
    OSPI_IRQHandler(&OSPI0_RES);
}

#if RTE_OSPI0_DMA_ENABLE
static void OSPI0_DMACallback(uint32_t event, int8_t peri_num)
{
    OSPI_DMACallback(event, peri_num, &OSPI0_RES);
}
#endif

static int32_t ARM_OSPI0_Initialize(ARM_OSPI_SignalEvent_t cb_event)
{
    return ARM_OSPI_Initialize(&OSPI0_RES, cb_event);
}

static int32_t ARM_OSPI0_Uninitialize(void)
{
    return ARM_OSPI_Uninitialize(&OSPI0_RES);
}

static int32_t ARM_OSPI0_PowerControl(ARM_POWER_STATE state)
{
    return ARM_OSPI_PowerControl(&OSPI0_RES, state);
}

static int32_t ARM_OSPI0_Send(const void *data, uint32_t num)
{
    return ARM_OSPI_Send(&OSPI0_RES, data, num);
}

static int32_t ARM_OSPI0_Receive(void *data, uint32_t num)
{
    return ARM_OSPI_Receive(&OSPI0_RES, data, num);
}

static int32_t ARM_OSPI0_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_OSPI_Transfer(&OSPI0_RES, data_out, data_in, num);
}

static uint32_t ARM_OSPI0_GetDataCount(void)
{
    return ARM_OSPI_GetDataCount(&OSPI0_RES);
}

static int32_t ARM_OSPI0_Control(uint32_t control, uint32_t arg)
{
    return ARM_OSPI_Control(&OSPI0_RES, control, arg);
}

static ARM_OSPI_STATUS ARM_OSPI0_GetStatus(void)
{
    return ARM_OSPI_GetStatus(&OSPI0_RES);
}

extern ARM_DRIVER_OSPI Driver_OSPI0;
ARM_DRIVER_OSPI Driver_OSPI0 = {
    ARM_OSPI_GetVersion,
    ARM_OSPI_GetCapabilities,
    ARM_OSPI0_Initialize,
    ARM_OSPI0_Uninitialize,
    ARM_OSPI0_PowerControl,
    ARM_OSPI0_Send,
    ARM_OSPI0_Receive,
    ARM_OSPI0_Transfer,
    ARM_OSPI0_GetDataCount,
    ARM_OSPI0_Control,
    ARM_OSPI0_GetStatus
};
#endif /* RTE_OSPI0 */

/* OSPI1 driver instance */
#if RTE_OSPI1

#if RTE_OSPI1_DMA_ENABLE
static void OSPI1_DMACallback(uint32_t event, int8_t peri_num);

static OSPI_DMA_HW_CONFIG OSPI1_DMA_HW_CONFIG = {
    .dma_rx =
    {
        .dma_drv        = &ARM_Driver_DMA_(OSPI1_DMA),
        .dma_periph_req = OSPI1_DMA_RX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = OSPI1_DMA,
             .group    = OSPI1_DMA_GROUP,
             .channel  = OSPI1_DMA_RX_PERIPH_REQ,
             .enable_handshake = OSPI1_DMA_HANDSHAKE_ENABLE,
        },
    },
    .dma_tx =
    {
        .dma_drv        = &ARM_Driver_DMA_(OSPI1_DMA),
        .dma_periph_req = OSPI1_DMA_TX_PERIPH_REQ,
        .evtrtr_cfg =
        {
             .instance = OSPI1_DMA,
             .group    = OSPI1_DMA_GROUP,
             .channel  = OSPI1_DMA_TX_PERIPH_REQ,
             .enable_handshake = OSPI1_DMA_HANDSHAKE_ENABLE,
        },
    },
};
#endif
OSPI_RESOURCES OSPI1_RES = {
    .regs                   = (OSPI_Type *) OSPI1_BASE,
    .aes_regs               = (AES_Type *) AES1_BASE,
    .irq                    = OSPI1_IRQ_IRQn,
    .cb_event               = NULL,
    .irq_priority           = RTE_OSPI1_IRQ_PRIORITY,
    .drv_instance           = OSPI_INSTANCE_1,
    .chip_selection_pin     = RTE_OSPI1_CHIP_SELECTION_PIN,
    .transfer.spi_frf       = RTE_OSPI1_SPI_FRAME_FORMAT,
    .tx_fifo_threshold      = RTE_OSPI1_TX_FIFO_THRESHOLD,
    .tx_fifo_start_level    = RTE_OSPI1_TX_FIFO_LEVEL_TO_START_TRANSFER,
    .rx_fifo_threshold      = RTE_OSPI1_RX_FIFO_THRESHOLD,
#if RTE_OSPI1_DMA_ENABLE
    .dma_cb                 = OSPI1_DMACallback,
    .dma_config             = &OSPI1_DMA_HW_CONFIG,
    .dma_irq_priority       = RTE_OSPI1_DMA_IRQ_PRIORITY,
    .dma_enable             = RTE_OSPI1_DMA_ENABLE,
    .rx_dma_data_level      = RTE_OSPI1_RX_DMA_DATA_LEVEL,
    .tx_dma_data_level      = RTE_OSPI1_TX_DMA_DATA_LEVEL,
#endif
    .ddr_drive_edge         = RTE_OSPI1_DDR_DRIVE_EDGE,
    .rx_sample_delay        = RTE_OSPI1_RX_SAMPLE_DELAY,
    .rxds_delay             = RTE_OSPI1_RXDS_DELAY
};

void OSPI1_IRQHandler(void)
{
    OSPI_IRQHandler(&OSPI1_RES);
}

#if RTE_OSPI1_DMA_ENABLE
static void OSPI1_DMACallback(uint32_t event, int8_t peri_num)
{
    OSPI_DMACallback(event, peri_num, &OSPI1_RES);
}
#endif
static int32_t ARM_OSPI1_Initialize(ARM_OSPI_SignalEvent_t cb_event)
{
    return ARM_OSPI_Initialize(&OSPI1_RES, cb_event);
}

static int32_t ARM_OSPI1_Uninitialize(void)
{
    return ARM_OSPI_Uninitialize(&OSPI1_RES);
}

static int32_t ARM_OSPI1_PowerControl(ARM_POWER_STATE state)
{
    return ARM_OSPI_PowerControl(&OSPI1_RES, state);
}

static int32_t ARM_OSPI1_Send(const void *data, uint32_t num)
{
    return ARM_OSPI_Send(&OSPI1_RES, data, num);
}

static int32_t ARM_OSPI1_Receive(void *data, uint32_t num)
{
    return ARM_OSPI_Receive(&OSPI1_RES, data, num);
}

static int32_t ARM_OSPI1_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return ARM_OSPI_Transfer(&OSPI1_RES, data_out, data_in, num);
}

static uint32_t ARM_OSPI1_GetDataCount(void)
{
    return ARM_OSPI_GetDataCount(&OSPI1_RES);
}

static int32_t ARM_OSPI1_Control(uint32_t control, uint32_t arg)
{
    return ARM_OSPI_Control(&OSPI1_RES, control, arg);
}

static ARM_OSPI_STATUS ARM_OSPI1_GetStatus(void)
{
    return ARM_OSPI_GetStatus(&OSPI1_RES);
}

extern ARM_DRIVER_OSPI Driver_OSPI1;
ARM_DRIVER_OSPI Driver_OSPI1 = {
    ARM_OSPI_GetVersion,
    ARM_OSPI_GetCapabilities,
    ARM_OSPI1_Initialize,
    ARM_OSPI1_Uninitialize,
    ARM_OSPI1_PowerControl,
    ARM_OSPI1_Send,
    ARM_OSPI1_Receive,
    ARM_OSPI1_Transfer,
    ARM_OSPI1_GetDataCount,
    ARM_OSPI1_Control,
    ARM_OSPI1_GetStatus
};
#endif /* RTE_OSPI1 */
