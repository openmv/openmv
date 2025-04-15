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
 * @file     ospi.c
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     19-Jun-2023
 * @brief    Low level OSPI driver.
 ******************************************************************************/

#include "ospi.h"

/**
  \fn          void ospi_set_mode(OSPI_Type *ospi, SPI_MODE mode)
  \brief       Set the OSPI mode for the OSPI instance.
  \param[in]   ospi     Pointer to the OSPI register map
  \param[in]   mode    The mode to be set.
  \return      none
*/
void ospi_set_mode(OSPI_Type *ospi, SPI_MODE mode)
{
    uint32_t val;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_SCPOL_HIGH | SPI_CTRLR0_SCPH_HIGH);

    switch (mode)
    {
        /* Clock Polarity 0, Clock Phase 0 */
        case SPI_MODE_0:
            break;

        /* Clock Polarity 0, Clock Phase 1 */
        case SPI_MODE_1:
            val |= (SPI_CTRLR0_SCPOL_LOW | SPI_CTRLR0_SCPH_HIGH);
            break;

        /* Clock Polarity 1, Clock Phase 0 */
        case SPI_MODE_2:
            val |= (SPI_CTRLR0_SCPOL_HIGH | SPI_CTRLR0_SCPH_LOW);
            break;

        /* Clock Polarity 1, Clock Phase 1 */
        case SPI_MODE_3:
            val |= (SPI_CTRLR0_SCPOL_HIGH | SPI_CTRLR0_SCPH_HIGH);
            break;
    }

    ospi->OSPI_CTRLR0 = val;
    ospi_enable(ospi);
}

/**
  \fn          void ospi_set_dfs(OSPI_Type *ospi, uint8_t dfs)
  \brief       Set the data frame size for the OSPI instance.
  \param[in]   ospi     Pointer to the OSPI register map
  \param[in]   dfs     The data frame size
  \return      none
*/
void ospi_set_dfs(OSPI_Type *ospi, uint8_t dfs)
{
    uint32_t val = 0;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~SPI_CTRLR0_DFS_MASK;
    val |= (dfs - 1);
    ospi->OSPI_CTRLR0 = val;

    ospi_enable(ospi);
}

/**
  \fn          void ospi_set_tmod(OSPI_Type *spi, SPI_TMOD tmod)
  \brief       Set the transfer mode for the OSPI instance.
  \param[in]   ospi    Pointer to the OSPI register map
  \param[in]   tmod    Transfer mode
  \return      none
*/
void ospi_set_tmod(OSPI_Type *ospi, SPI_TMOD tmod)
{
    uint32_t val = 0;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_TMOD_MASK);

    switch(tmod)
    {
    case SPI_TMOD_TX_AND_RX:
        val |= SPI_CTRLR0_TMOD_TRANSFER;
        break;
    case SPI_TMOD_TX:
        val |= SPI_CTRLR0_TMOD_SEND_ONLY;
        break;
    case SPI_TMOD_RX:
        val |= SPI_CTRLR0_TMOD_RECEIVE_ONLY;
        break;
    case SPI_TMOD_EEPROM_READ:
        val |= SPI_CTRLR0_TMOD_EEPROM_READ_ONLY;
        break;
    default:
        break;
    }
    ospi->OSPI_CTRLR0 = val;

    ospi_enable(ospi);
}

/**
  \fn          void ospi_set_tx_threshold(OSPI_Type *ospi, uint8_t threshold)
  \brief       Set Transmit FIFO interrupt threshold for the OSPI instance
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   threshold  Transmit FIFO threshold
  \return      none
*/
void ospi_set_tx_threshold(OSPI_Type *ospi, uint8_t threshold)
{
    uint32_t val = ospi->OSPI_TXFTLR;
    val &= ~(SPI_TXFTLR_TFT_MASK);
    val |= threshold << SPI_TXFTLR_TFT_SHIFT;
    ospi->OSPI_TXFTLR = val;
}

/**
  \fn          void ospi_set_rx_sample_delay(OSPI_Type *ospi, uint8_t rx_sample_delay)
  \brief       Set Receive sample delay for the OSPI instance
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   threshold  Receive FIFO threshold
  \return      none
*/
void ospi_set_rx_sample_delay(OSPI_Type *ospi, uint8_t rx_sample_delay)
{
    ospi_disable(ospi);
    ospi->OSPI_RX_SAMPLE_DELAY = rx_sample_delay;
    ospi_enable(ospi);
}

/**
  \fn          void ospi_set_ddr_drive_edge(OSPI_Type *ospi, uint8_t ddr_drive_edge)
  \brief       Set DDR drive edge for the OSPI instance
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   threshold  Receive FIFO threshold
  \return      none
*/
void ospi_set_ddr_drive_edge(OSPI_Type *ospi, uint8_t ddr_drive_edge)
{
    ospi_disable(ospi);
    ospi->OSPI_DDR_DRIVE_EDGE = ddr_drive_edge;
    ospi_enable(ospi);
}

/**
  \fn          void ospi_control_ss(OSPI_Type *ospi, uint8_t slave, SPI_SS_STATE state)
  \brief       Control the slave select line
  \param[in]   spi    Pointer to the OSPI register map
  \param[in]   slave  The slave to be selected
  \param[in]   state  The state of the slave select line
  \return      none
*/
void ospi_control_ss(OSPI_Type *ospi, uint8_t slave, SPI_SS_STATE state)
{
    ospi_disable(ospi);

    if (state == SPI_SS_STATE_ENABLE)
    {
        ospi->OSPI_SER |= 1 << slave;
    }
    else
    {
        ospi->OSPI_SER &= ~(1 << slave);
    }
    ospi_enable(ospi);
}

/**
  \fn          void ospi_control_xip_ss(OSPI_Type *ospi, uint8_t slave, SPI_SS_STATE state)
  \brief       Control the XIP slave select line
  \param[in]   ospi   Pointer to the OSPI register map
  \param[in]   slave  The slave to be selected
  \param[in]   state  The state of the slave select line
  \return      none
*/
void ospi_control_xip_ss(OSPI_Type *ospi, uint8_t slave, SPI_SS_STATE state)
{
    ospi_disable(ospi);

    if (state == SPI_SS_STATE_ENABLE)
    {
        ospi->OSPI_XIP_SER |= 1 << slave;
    }
    else
    {
        ospi->OSPI_XIP_SER &= ~(1 << slave);
    }
    ospi_enable(ospi);
}


/**
  \fn          void ospi_send(OSPI_Type *spi, ospi_transfer_t *transfer)
  \brief       Prepare the OSPI instance for transmission
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   transfer   Transfer parameters
  \return      none
*/
void ospi_send(OSPI_Type *ospi, ospi_transfer_t *transfer)
{
    uint32_t val;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_SPI_FRF_MASK | (SPI_CTRLR0_TMOD_MASK | SPI_CTRLR0_SSTE_MASK));
    val |= ((transfer->spi_frf << SPI_CTRLR0_SPI_FRF) | SPI_CTRLR0_TMOD_SEND_ONLY);
    ospi->OSPI_CTRLR0 = val;

    ospi->OSPI_CTRLR1 = 0;

    val = SPI_TRANS_TYPE_FRF_DEFINED
              | SPI_CTRLR0_SPI_RXDS_ENABLE << SPI_CTRLR0_SPI_RXDS_EN_OFFSET
              | (transfer->ddr << SPI_CTRLR0_SPI_DDR_EN_OFFSET)
              | (SPI_CTRLR0_INST_L_8bit << SPI_CTRLR0_INST_L_OFFSET)
              | (transfer->addr_len << SPI_CTRLR0_ADDR_L_OFFSET)
              | (transfer->dummy_cycle << SPI_CTRLR0_WAIT_CYCLES_OFFSET);

    ospi->OSPI_SPI_CTRLR0 = val;

    ospi->OSPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK
                   | SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);

    ospi_enable(ospi);
}

/**
  \fn          void ospi_receive(OSPI_Type *ospi, ospi_transfer_t *transfer)
  \brief       Prepare the OSPI instance for reception
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   transfer   Transfer parameters
  \return      none
*/
void ospi_receive(OSPI_Type *ospi, ospi_transfer_t *transfer)
{
    uint32_t val;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_SPI_FRF_MASK | (SPI_CTRLR0_TMOD_MASK | SPI_CTRLR0_SSTE_MASK));
    val |= ((transfer->spi_frf << SPI_CTRLR0_SPI_FRF) | SPI_CTRLR0_TMOD_RECEIVE_ONLY);
    ospi->OSPI_CTRLR0 = val;

    ospi->OSPI_CTRLR1 = transfer->rx_total_cnt - 1;

    val = SPI_TRANS_TYPE_FRF_DEFINED
              | (SPI_CTRLR0_SPI_RXDS_ENABLE << SPI_CTRLR0_SPI_RXDS_EN_OFFSET)
              | (transfer->ddr << SPI_CTRLR0_SPI_DDR_EN_OFFSET)
              | (SPI_CTRLR0_INST_L_0bit << SPI_CTRLR0_INST_L_OFFSET)
              | (transfer->addr_len << SPI_CTRLR0_ADDR_L_OFFSET)
              | (transfer->dummy_cycle << SPI_CTRLR0_WAIT_CYCLES_OFFSET);

    ospi->OSPI_SPI_CTRLR0 = val;

    ospi->OSPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK
                   | SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK
                   | SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);

    ospi_enable(ospi);
}

/**
  \fn          void ospi_transfer(OSPI_Type *spi, ospi_transfer_t *transfer)
  \brief       Prepare the OSPI instance for transfer
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   transfer   Transfer parameters
  \return      none
*/
void ospi_transfer(OSPI_Type *ospi, ospi_transfer_t *transfer)
{
    uint32_t val;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_SPI_FRF_MASK | (SPI_CTRLR0_TMOD_MASK | SPI_CTRLR0_SSTE_MASK ));
    val |= ((transfer->spi_frf << SPI_CTRLR0_SPI_FRF) | SPI_CTRLR0_TMOD_RECEIVE_ONLY);
    ospi->OSPI_CTRLR0 = val;

    ospi->OSPI_CTRLR1 = transfer->rx_total_cnt - 1;

    val = SPI_TRANS_TYPE_FRF_DEFINED
              | (SPI_CTRLR0_SPI_RXDS_ENABLE << SPI_CTRLR0_SPI_RXDS_EN_OFFSET)
              | (transfer->ddr << SPI_CTRLR0_SPI_DDR_EN_OFFSET)
              | (SPI_CTRLR0_INST_L_8bit << SPI_CTRLR0_INST_L_OFFSET)
              | (transfer->addr_len << SPI_CTRLR0_ADDR_L_OFFSET)
              | (transfer->dummy_cycle << SPI_CTRLR0_WAIT_CYCLES_OFFSET);

    ospi->OSPI_SPI_CTRLR0 = val;

    ospi->OSPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK
                   | SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK
                   | SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK
                   | SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);

    ospi_enable(ospi);
}

/**
  \fn          void ospi_dma_send(OSPI_Type *spi, ospi_transfer_t *transfer)
  \brief       Prepare the OSPI instance for transmission with DMA support
  \param[in]   ospi       Pointer to the OSPI register map
  \param[in]   transfer   Transfer parameters
  \return      none
*/
void ospi_dma_send(OSPI_Type *ospi, ospi_transfer_t *transfer)
{
    uint32_t val;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_SPI_FRF_MASK | (SPI_CTRLR0_TMOD_MASK | SPI_CTRLR0_SSTE_MASK));
    val |= ((transfer->spi_frf << SPI_CTRLR0_SPI_FRF) | SPI_CTRLR0_TMOD_SEND_ONLY);
    ospi->OSPI_CTRLR0 = val;

    val = SPI_TRANS_TYPE_FRF_DEFINED
              | SPI_CTRLR0_SPI_RXDS_ENABLE << SPI_CTRLR0_SPI_RXDS_EN_OFFSET
              | (transfer->ddr << SPI_CTRLR0_SPI_DDR_EN_OFFSET)
              | (SPI_CTRLR0_INST_L_8bit << SPI_CTRLR0_INST_L_OFFSET)
              | (transfer->addr_len << SPI_CTRLR0_ADDR_L_OFFSET)
              | (transfer->dummy_cycle << SPI_CTRLR0_WAIT_CYCLES_OFFSET);

    ospi->OSPI_SPI_CTRLR0 = val;

    ospi->OSPI_IMR = SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK;

    ospi->OSPI_TXFTLR &= ~(0xFFU << SPI_TXFTLR_TXFTHR_SHIFT);

    ospi->OSPI_TXFTLR |= ((transfer->tx_total_cnt - 1U) << SPI_TXFTLR_TXFTHR_SHIFT);

    ospi_enable_tx_dma(ospi);

    ospi_enable(ospi);

}

void ospi_dma_transfer(OSPI_Type *ospi, ospi_transfer_t *transfer)
{
    uint32_t val;

    ospi_disable(ospi);

    val = ospi->OSPI_CTRLR0;
    val &= ~(SPI_CTRLR0_SPI_FRF_MASK | (SPI_CTRLR0_TMOD_MASK | SPI_CTRLR0_SSTE_MASK ));
    val |= ((transfer->spi_frf << SPI_CTRLR0_SPI_FRF) | SPI_CTRLR0_TMOD_RECEIVE_ONLY);
    ospi->OSPI_CTRLR0 = val;

    ospi->OSPI_CTRLR1 = transfer->rx_total_cnt - 1;

    val = SPI_TRANS_TYPE_FRF_DEFINED
              | (SPI_CTRLR0_SPI_RXDS_ENABLE << SPI_CTRLR0_SPI_RXDS_EN_OFFSET)
              | (transfer->ddr << SPI_CTRLR0_SPI_DDR_EN_OFFSET)
              | (SPI_CTRLR0_INST_L_8bit << SPI_CTRLR0_INST_L_OFFSET)
              | (transfer->addr_len << SPI_CTRLR0_ADDR_L_OFFSET)
              | (transfer->dummy_cycle << SPI_CTRLR0_WAIT_CYCLES_OFFSET);

    ospi->OSPI_SPI_CTRLR0 = val;

    ospi->OSPI_TXFTLR &= ~(0xFFU << SPI_TXFTLR_TXFTHR_SHIFT);

    ospi->OSPI_TXFTLR |= ((transfer->tx_total_cnt - 1U) << SPI_TXFTLR_TXFTHR_SHIFT);

    ospi->OSPI_IMR = SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK
                     | SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK
                     | SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK;

    ospi_enable_tx_dma(ospi);

    ospi_enable_rx_dma(ospi);

    ospi_enable(ospi);
}

/**
  \fn          void ospi_hyperbus_xip_init(OSPI_Type *ospi, uint8_t wait_cycles)
  \brief       Initialize hyperbus XIP configuration for the OSPI instance
  \param[in]   ospi        Pointer to the OSPI register map
  \param[in]   wait_cycles Wait cycles needed by the hyperbus device
  \return      none
*/
void ospi_hyperbus_xip_init(OSPI_Type *ospi, uint8_t wait_cycles)
{
    ospi_disable(ospi);

    ospi->OSPI_SPI_CTRLR0 = 1 << SPI_CTRLR0_SPI_DM_EN_OFFSET;

    ospi->OSPI_XIP_CTRL = (1 << XIP_CTRL_XIP_HYPERBUS_EN_OFFSET)
                        | (1 << XIP_CTRL_RXDS_SIG_EN_OFFSET)
                        | (wait_cycles << XIP_CTRL_WAIT_CYCLES_OFFSET);

    ospi->OSPI_XIP_WRITE_CTRL = (1 << XIP_WRITE_CTRL_XIPWR_HYPERBUS_EN_OFFSET)
                              | (1 << XIP_WRITE_CTRL_XIPWR_RXDS_SIG_EN_OFFSET)
                              | (wait_cycles << XIP_WRITE_CTRL_XIPWR_WAIT_CYCLES);

    ospi_enable(ospi);
}

/**
  \fn          void ospi_irq_handler(OSPI_Type *ospi, ospi_transfer_t *transfer)
  \brief       Handle interrupts for the OSPI instance.
  \param[in]   ospi      Pointer to the OSPI register map
  \param[in]   transfer  The transfer structure for the SPI instance
  \return      none
*/
void ospi_irq_handler(OSPI_Type *ospi, ospi_transfer_t *transfer)
{
    uint32_t event, tx_data, index, rx_count, tx_count;
    uint16_t frame_size;

    event = ospi->OSPI_ISR;

    if (event & SPI_TX_FIFO_EMPTY_EVENT)
    {
        frame_size = (SPI_CTRLR0_DFS_MASK & ospi->OSPI_CTRLR0);

        /* Calculate data count to transfer */
        if (transfer->tx_total_cnt >= (transfer->tx_current_cnt + OSPI_TX_FIFO_DEPTH))
        {
            tx_count = (uint16_t) OSPI_TX_FIFO_DEPTH;
        }
        else
        {
            tx_count = (transfer->tx_total_cnt - transfer->tx_current_cnt);
        }

        ospi->OSPI_TXFTLR &= ~(0xFFU << SPI_TXFTLR_TXFTHR_SHIFT);
        ospi->OSPI_TXFTLR |= ((tx_count - 1U) << SPI_TXFTLR_TXFTHR_SHIFT);

        for (index = 0; index < tx_count; index++)
        {
            tx_data = 0;

            if (transfer->tx_buff == NULL)
            {
                /* Check if the default buffer transmit is enabled */
                if (transfer->tx_default_enable == true)
                {
                    tx_data = transfer->tx_default_val;
                }
            }
            else
            {
                tx_data = transfer->tx_buff[0];
                transfer->tx_buff = (transfer->tx_buff + 1);
            }
            ospi->OSPI_DR0 = tx_data;
            transfer->tx_current_cnt++;
        }
    }

    if (event & SPI_RX_FIFO_FULL_EVENT)
    {
        frame_size = (SPI_CTRLR0_DFS_MASK & ospi->OSPI_CTRLR0);

        rx_count = ospi->OSPI_RXFLR;

        if (frame_size > SPI_CTRLR0_DFS_16bit)
        {
            for (index = 0; index < rx_count; index++)
            {
                *((uint32_t *) transfer->rx_buff) = ospi->OSPI_DR0;

                transfer->rx_buff = (uint8_t *) transfer->rx_buff + sizeof(uint32_t);
                transfer->rx_current_cnt++;
            }
        }
        else if (frame_size > SPI_CTRLR0_DFS_8bit)
        {
            for (index = 0; index < rx_count; index++)
            {
                *((uint16_t *) transfer->rx_buff) = (uint16_t) (ospi->OSPI_DR0);

                transfer->rx_buff = (uint8_t *) transfer->rx_buff + sizeof(uint16_t);
                transfer->rx_current_cnt++;
            }
        }
        else
        {
            for (index = 0; index < rx_count; index++)
            {
                /*
                 * It is observed that with DFS set to 8, the controller reads in 16bit
                 * frames. Workaround this by making two valid 8bit frames out of the
                 * DR content.
                 */
                uint32_t val = ospi->OSPI_DR0;

                *((uint8_t *) transfer->rx_buff) = (uint8_t) ((val >> 8) & 0xff);
                transfer->rx_buff = (uint8_t *) transfer->rx_buff + sizeof(uint8_t);
                transfer->rx_current_cnt++;

                if (transfer->rx_current_cnt == transfer->rx_total_cnt)
                    break;

                *((uint8_t *) transfer->rx_buff) = (uint8_t) (val & 0xff);
                transfer->rx_buff = (uint8_t *) transfer->rx_buff + sizeof(uint8_t);
                transfer->rx_current_cnt++;
            }
        }
    }

    if (event & (SPI_RX_FIFO_OVER_FLOW_EVENT | SPI_TX_FIFO_OVER_FLOW_EVENT))
    {
        /* Disabling and Enabling the OSPI will Reset the FIFO */
        ospi_disable(ospi);
        ospi_enable(ospi);

        transfer->status = SPI_TRANSFER_STATUS_OVERFLOW;
    }

    /* Rx FIFO was accessed when it was empty */
    if (event & SPI_RX_FIFO_UNDER_FLOW_EVENT)
    {
        transfer->status = SPI_TRANSFER_STATUS_RX_UNDERFLOW;
    	ospi->OSPI_IMR &= ~(SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK);
    }

    /* SEND ONLY mode : check if the transfer is finished */
    if ((transfer->mode == SPI_TMOD_TX) &&
           (transfer->tx_total_cnt == transfer->tx_current_cnt))
    {
        /* Wait for the transfer to complete */
        if((ospi->OSPI_SR & (SPI_SR_BUSY | SPI_SR_TX_FIFO_EMPTY)) == SPI_SR_TX_FIFO_EMPTY)
        {
            /* Mask the TX interrupts */
            ospi->OSPI_IMR &= ~(SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                                SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                                SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);

            transfer->tx_current_cnt = 0;
            transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
        }
    }
    /* RECEIVE ONLY mode : check if the transfer is finished */
    if ((transfer->mode == SPI_TMOD_RX) &&
             (transfer->rx_total_cnt == transfer->rx_current_cnt))
    {
        /* Mask the RX interrupts */
        ospi->OSPI_IMR &= ~(SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                                SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                                SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK |
                                SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);

        transfer->rx_current_cnt = 0;
        transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
    }

    if((transfer->mode == SPI_TMOD_TX_AND_RX) &&
              (transfer->rx_total_cnt == (transfer->rx_current_cnt)))
    {
        /* Mask all the interrupts */
        ospi->OSPI_IMR = 0;

        ospi_disable(ospi);

        transfer->rx_current_cnt = 0;
        transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
    }

    if((transfer->mode == SPI_TMOD_TX_AND_RX) && (transfer->tx_total_cnt == transfer->tx_current_cnt))
    {
        if((ospi->OSPI_SR & SPI_SR_TX_FIFO_EMPTY) == SPI_SR_TX_FIFO_EMPTY)
        {
            /* Reset the Tx FIFO start level */
            ospi->OSPI_TXFTLR &= ~(0xFFU << SPI_TXFTLR_TXFTHR_SHIFT);

            /* Mask the TX interrupts */
            ospi->OSPI_IMR &= ~(SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK
                             | SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK
                             | SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
        }
    }

    /* Read interrupt clear registers */
    (void) ospi->OSPI_TXEICR;
    (void) ospi->OSPI_RXOICR;
    (void) ospi->OSPI_RXUICR;
    (void) ospi->OSPI_ICR;
}
