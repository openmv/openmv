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
 * @file     spi.c
 * @author   Girish BN, Manoj A Murudi
 * @email    girish.bn@alifsemi.com, manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     20-04-2023
 * @brief    Low Level Source File for SPI.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "spi.h"

/**
  \fn          void spi_set_mode(SPI_Type *spi, SPI_MODE mode)
  \brief       Set the SPI mode for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   mode    The mode to be set.
  \return      none
*/
void spi_set_mode(SPI_Type *spi, SPI_MODE mode)
{
    uint32_t val;

    spi_disable(spi);

    val = spi->SPI_CTRLR0;
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

    spi->SPI_CTRLR0 = val;
    spi_enable(spi);
}

/**
  \fn          void spi_set_protocol(SPI_Type *spi, SPI_PROTO format)
  \brief       Set the protocol format for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   format  The protocol to be set
  \return      none
*/
void spi_set_protocol(SPI_Type *spi, SPI_PROTO format)
{
    uint32_t val;

    spi_disable(spi);

    val = spi->SPI_CTRLR0;
    val &= ~(SPI_CTRLR0_FRF_MASK);

    switch(format)
    {
        case SPI_PROTO_SPI:
            break;
        case SPI_PROTO_SSP:
            val |= SPI_CTRLR0_FRF_TI;
            break;
        case SPI_PROTO_MICROWIRE:
            val |= SPI_CTRLR0_FRF_MICROWIRE;
            break;
    }

    spi->SPI_CTRLR0 = val;
    spi_enable(spi);
}

/**
  \fn          void spi_set_dfs(SPI_Type *spi, uint8_t dfs)
  \brief       Set the data frame size for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   dfs     The data frame size
  \return      none
*/
void spi_set_dfs(SPI_Type *spi, uint8_t dfs)
{
    uint32_t val = 0;

    spi_disable(spi);

    val = spi->SPI_CTRLR0;
    val &= ~SPI_CTRLR0_DFS_MASK;
    val |= (dfs - 1);
    spi->SPI_CTRLR0 = val;

    spi_enable(spi);
}

/**
  \fn          void spi_set_tmod(SPI_Type *spi, SPI_TMOD tmod)
  \brief       Set the transfer mode for the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \param[in]   tmod    Transfer mode
  \return      none
*/
void spi_set_tmod(SPI_Type *spi, SPI_TMOD tmod)
{
    uint32_t val = 0;

    spi_disable(spi);

    val = spi->SPI_CTRLR0;
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
    spi->SPI_CTRLR0 = val;

    spi_enable(spi);
}

/**
  \fn          SPI_TMOD spi_get_tmod(SPI_Type *spi)
  \brief       Get the transfer mode of the SPI instance.
  \param[in]   spi     Pointer to the SPI register map
  \return      The current transfer mode
*/
SPI_TMOD spi_get_tmod(SPI_Type *spi)
{
    uint32_t val = spi->SPI_CTRLR0;

    if ((val & SPI_CTRLR0_TMOD_MASK) == SPI_CTRLR0_TMOD_SEND_ONLY)
    {
        return SPI_TMOD_TX;
    }
    else if ((val & SPI_CTRLR0_TMOD_MASK) == SPI_CTRLR0_TMOD_RECEIVE_ONLY)
    {
        return SPI_TMOD_RX;
    }
    else if ((val & SPI_CTRLR0_TMOD_MASK) == SPI_CTRLR0_TMOD_TRANSFER)
    {
        return SPI_TMOD_TX_AND_RX;
    }
    else
    {
        return SPI_TMOD_EEPROM_READ;
    }
}

/**
  \fn          void spi_set_tx_threshold(SPI_Type *spi, uint8_t threshold)
  \brief       Set Transmit FIFO interrupt threshold for the SPI instance
  \param[in]   spi        Pointer to the SPI register map
  \param[in]   threshold  Transmit FIFO threshold
  \return      none
*/
void spi_set_tx_threshold(SPI_Type *spi, uint8_t threshold)
{
    uint32_t val = spi->SPI_TXFTLR;
    val &= ~(SPI_TXFTLR_TFT_MASK);
    val |= threshold << SPI_TXFTLR_TFT_SHIFT;
    spi->SPI_TXFTLR = val;
}

/**
  \fn          void spi_set_rx_threshold(SPI_Type *spi, uint8_t threshold)
  \brief       Set Receive FIFO interrupt threshold for the SPI instance
  \param[in]   spi        Pointer to the SPI register map
  \param[in]   threshold  Receive FIFO threshold
  \return      none
*/
void spi_set_rx_threshold(SPI_Type *spi, uint8_t threshold)
{
    spi->SPI_RXFTLR = threshold;
}

/**
  \fn          void spi_set_tx_fifo_start_level(SPI_Type *spi, uint16_t level)
  \brief       Set Transmit FIFO start level
  \param[in]   spi    Pointer to the SPI register map
  \param[in]   level  Transmit FIFO start level
  \return      none
*/
void spi_set_tx_fifo_start_level(SPI_Type *spi, uint16_t level)
{
    uint32_t val = spi->SPI_TXFTLR;
    val &= ~(SPI_TXFTLR_TXFTHR_MASK);
    val |= level << SPI_TXFTLR_TXFTHR_SHIFT;
    spi->SPI_TXFTLR = val;
}

/**
  \fn          void spi_control_ss(SPI_Type *spi, uint8_t slave, SPI_SS_STATE state)
  \brief       Control the slave select line
  \param[in]   spi    Pointer to the SPI register map
  \param[in]   slave  The slave to be selected
  \param[in]   state  The state of the slave select line
  \return      none
*/
void spi_control_ss(SPI_Type *spi, uint8_t slave, SPI_SS_STATE state)
{
    spi_disable(spi);

    if (state == SPI_SS_STATE_ENABLE)
    {
        spi->SPI_SER |= 1 << slave;
    }
    else
    {
        spi->SPI_SER &= ~(1 << slave);
    }
    spi_enable(spi);
}

/**
  \fn          void spi_set_sste(SPI_Type *spi, bool enable)
  \brief       Enable/Disable Slave Select Toggle for the SPI instance
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   enable    Enable/Disable control
  \return      none
*/
void spi_set_sste(SPI_Type *spi, bool enable)
{
    uint32_t val = spi->SPI_CTRLR0;

    spi_disable(spi);

    if (enable)
    {
        val |= SPI_CTRLR0_SSTE_ENABLE;
    }
    else
    {
        val &= ~SPI_CTRLR0_SSTE_ENABLE;
    }

    spi->SPI_CTRLR0 = val;
    spi_enable(spi);
}

/**
  \fn          void spi_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for transmission
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_send(SPI_Type *spi)
{
    spi_set_tmod(spi, SPI_TMOD_TX);
    spi->SPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                    SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void spi_receive(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Prepare the SPI instance for reception
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_receive(SPI_Type *spi, spi_transfer_t *transfer)
{
    spi_set_tmod(spi, SPI_TMOD_RX);
    spi_disable(spi);
    spi->SPI_CTRLR1 = transfer->rx_total_cnt - 1;
    spi->SPI_IMR = (SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK);
    spi_enable(spi);

    if (transfer->is_master)
    {
        /* Initiate the receive operation by writing a dummy byte to the FIFO */
        spi->SPI_DR[0] = 0x0;
    }
}

/**
  \fn          void spi_transfer(SPI_Type *spi)
  \brief       Prepare the SPI instance for transfer
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_transfer(SPI_Type *spi)
{
    spi_set_tmod(spi, SPI_TMOD_TX_AND_RX);
    spi->SPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                    SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void spi_send_blocking(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI send described by the transfer structure.
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_send_blocking(SPI_Type *spi, spi_transfer_t *transfer)
{
    uint32_t curr_fifo_level, tx_count, tx_data;
    uint32_t index;

    spi_set_tmod(spi, SPI_TMOD_TX);

    while (transfer->tx_current_cnt < transfer->tx_total_cnt)
    {
        /* wait for a free FIFO location */
        while ((spi->SPI_SR & SPI_SR_TFNF) == 0)
        {
        }

        curr_fifo_level = spi->SPI_TXFLR;

        if (transfer->tx_total_cnt >= (transfer->tx_current_cnt + SPI_TX_FIFO_DEPTH - curr_fifo_level))
        {
            tx_count = SPI_TX_FIFO_DEPTH - curr_fifo_level;
        }
        else
        {
            tx_count = (transfer->tx_total_cnt - transfer->tx_current_cnt);
        }

        for (index = 0; index < tx_count; index++)
        {
            if (transfer->tx_buff == NULL)
            {
                if (transfer->tx_default_enable)
                {
                    tx_data = transfer->tx_default_val;
                }
            }
            else
            {
                if (transfer->frame_size > 16)
                {
                    tx_data = (uint32_t) (transfer->tx_buff[0] | (transfer->tx_buff[1] << 8) |
                                         (transfer->tx_buff[2] << 16) | (transfer->tx_buff[3] << 24));
                    transfer->tx_buff = transfer->tx_buff + 4U;
                }
                else if (transfer->frame_size > 8)
                {
                    tx_data = (uint32_t)(transfer->tx_buff[0] | (transfer->tx_buff[1] << 8));
                    transfer->tx_buff = transfer->tx_buff + 2;
                }
                else
                {
                    tx_data = transfer->tx_buff[0];
                    transfer->tx_buff = transfer->tx_buff + 1;
                }
            }

            spi->SPI_DR[0] = tx_data;
            transfer->tx_current_cnt++;
        }
    }

    while (spi_busy(spi))
    {

    }
}

/**
  \fn          void spi_receive_blocking(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI receive described by the transfer structure.
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_receive_blocking(SPI_Type *spi, spi_transfer_t *transfer)
{
    uint32_t rx_count;
    uint32_t index;

    spi_set_tmod(spi, SPI_TMOD_RX);

    if (transfer->is_master)
    {
        spi_disable(spi);
        spi->SPI_CTRLR1 = transfer->rx_total_cnt - 1;
        spi_enable(spi);

        /* Initiate the receive operation by writing a dummy byte to the FIFO */
        spi->SPI_DR[0] = 0x0;
    }

    while (transfer->rx_current_cnt < transfer->rx_total_cnt)
    {
        /* Wait for data in the Rx FIFO */
        while ((spi->SPI_SR & SPI_SR_RFNE) == 0)
        {
        }

        rx_count = spi->SPI_RXFLR;

        for (index = 0; index < rx_count; index++)
        {
            if (transfer->frame_size > 16)
            {
                *((uint32_t *) transfer->rx_buff) = spi->SPI_DR[0];
                transfer->rx_buff = ((uint32_t *)transfer->rx_buff) + 1U;
            }
            else if (transfer->frame_size > 8)
            {
                *((uint16_t *) transfer->rx_buff) = (uint16_t) (spi->SPI_DR[0]);
                transfer->rx_buff = ((uint16_t *)transfer->rx_buff) + 1U;
            }
            else
            {
                *((uint8_t *) transfer->rx_buff) = (uint8_t) (spi->SPI_DR[0]);
                transfer->rx_buff = ((uint8_t *)transfer->rx_buff) + 1U;
            }
            transfer->rx_current_cnt++;
        }
    }
}

/**
  \fn          void spi_transfer_blocking(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI transfer described by the transfer structure
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_transfer_blocking(SPI_Type *spi, spi_transfer_t *transfer)
{
    uint32_t tx_data;

    spi_set_tmod(spi, SPI_TMOD_TX_AND_RX);

    while (transfer->tx_current_cnt < transfer->tx_total_cnt)
    {
        /* Wait for space in the FIFO */
        while ((spi->SPI_SR & SPI_SR_TFNF) == 0)
        {
        }

        if (transfer->tx_buff == NULL)
        {
            if (transfer->tx_default_enable)
            {
                tx_data = transfer->tx_default_val;
            }
        }
        else
        {
            if (transfer->frame_size > 16)
            {
                tx_data = (uint32_t) (transfer->tx_buff[0] | (transfer->tx_buff[1] << 8) |
                                     (transfer->tx_buff[2] << 16) | (transfer->tx_buff[3] << 24));
                transfer->tx_buff = transfer->tx_buff + 4U;
            }
            else if (transfer->frame_size > 8)
            {
                tx_data = (uint32_t)(transfer->tx_buff[0] | (transfer->tx_buff[1] << 8));
                transfer->tx_buff = transfer->tx_buff + 2;
            }
            else
            {
                tx_data = transfer->tx_buff[0];
                transfer->tx_buff = transfer->tx_buff + 1;
            }
        }

        spi->SPI_DR[0] = tx_data;
        transfer->tx_current_cnt++;

        /* wait for data in the Rx FIFO */
        while ((spi->SPI_SR & SPI_SR_RFNE) == 0)
        {
        }

        if (transfer->frame_size > 16)
        {
            *((uint32_t *) transfer->rx_buff) = spi->SPI_DR[0];
            transfer->rx_buff = ((uint32_t *)transfer->rx_buff) + 1U;
        }
        else if (transfer->frame_size > 8)
        {
            *((uint16_t *) transfer->rx_buff) = (uint16_t) (spi->SPI_DR[0]);
            transfer->rx_buff = ((uint16_t *)transfer->rx_buff) + 1U;
        }
        else
        {
            *((uint8_t *) transfer->rx_buff) = (uint8_t) (spi->SPI_DR[0]);
            transfer->rx_buff = ((uint8_t *)transfer->rx_buff) + 1U;
        }
    }
}

/**
  \fn          void lpspi_send_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI send described by the transfer structure.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void lpspi_send_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
{
    uint32_t curr_fifo_level, tx_count, tx_data;
    uint32_t index;

    lpspi_set_tmod(lpspi, SPI_TMOD_TX);

    while (transfer->tx_current_cnt < transfer->tx_total_cnt)
    {
        /* wait for a free FIFO location */
        while ((lpspi->SPI_SR & SPI_SR_TFNF) == 0)
        {
        }

        curr_fifo_level = lpspi->SPI_TXFLR;

        if (transfer->tx_total_cnt >= (transfer->tx_current_cnt + SPI_TX_FIFO_DEPTH - curr_fifo_level))
        {
            tx_count = SPI_TX_FIFO_DEPTH - curr_fifo_level;
        }
        else
        {
            tx_count = (transfer->tx_total_cnt - transfer->tx_current_cnt);
        }

        for (index = 0; index < tx_count; index++)
        {
            if (transfer->tx_buff == NULL)
            {
                if (transfer->tx_default_enable)
                {
                    tx_data = transfer->tx_default_val;
                }
            }
            else
            {
                if (transfer->frame_size > 16)
                {
                    tx_data = (uint32_t) (transfer->tx_buff[0] | (transfer->tx_buff[1] << 8) |
                                         (transfer->tx_buff[2] << 16) | (transfer->tx_buff[3] << 24));
                    transfer->tx_buff = transfer->tx_buff + 4U;
                }
                else if (transfer->frame_size > 8)
                {
                    tx_data = (uint32_t)(transfer->tx_buff[0] | (transfer->tx_buff[1] << 8));
                    transfer->tx_buff = transfer->tx_buff + 2;
                }
                else
                {
                    tx_data = transfer->tx_buff[0];
                    transfer->tx_buff = transfer->tx_buff + 1;
                }
            }

            lpspi->SPI_DR[0] = tx_data;
            transfer->tx_current_cnt++;
        }
    }

    while (spi_busy(lpspi))
    {

    }
}

/**
  \fn          void lpspi_receive_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI receive described by the transfer structure.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void lpspi_receive_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
{
    uint32_t rx_count;
    uint32_t index;

    lpspi_set_tmod(lpspi, SPI_TMOD_RX);

    if (transfer->is_master)
    {
        spi_disable(lpspi);
        lpspi->SPI_CTRLR1 = transfer->rx_total_cnt - 1;
        spi_enable(lpspi);

        /* Initiate the receive operation by writing a dummy byte to the FIFO */
        lpspi->SPI_DR[0] = 0x0;
    }

    while (transfer->rx_current_cnt < transfer->rx_total_cnt)
    {
        /* Wait for data in the Rx FIFO */
        while ((lpspi->SPI_SR & SPI_SR_RFNE) == 0)
        {
        }

        rx_count = lpspi->SPI_RXFLR;

        for (index = 0; index < rx_count; index++)
        {
            if (transfer->frame_size > 16)
            {
                *((uint32_t *) transfer->rx_buff) = lpspi->SPI_DR[0];
                transfer->rx_buff = ((uint32_t *)transfer->rx_buff) + 1U;
            }
            else if (transfer->frame_size > 8)
            {
                *((uint16_t *) transfer->rx_buff) = (uint16_t) (lpspi->SPI_DR[0]);
                transfer->rx_buff = ((uint16_t *)transfer->rx_buff) + 1U;
            }
            else
            {
                *((uint8_t *) transfer->rx_buff) = (uint8_t) (lpspi->SPI_DR[0]);
                transfer->rx_buff = ((uint8_t *)transfer->rx_buff) + 1U;
            }
            transfer->rx_current_cnt++;
        }
    }
}

/**
  \fn          void lpspi_transfer_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
  \brief       Execute a blocking SPI transfer described by the transfer structure
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void lpspi_transfer_blocking(SPI_Type *lpspi, spi_transfer_t *transfer)
{
    uint32_t tx_data;

    spi_set_tmod(lpspi, SPI_TMOD_TX_AND_RX);

    while (transfer->tx_current_cnt < transfer->tx_total_cnt)
    {
        /* Wait for space in the FIFO */
        while ((lpspi->SPI_SR & SPI_SR_TFNF) == 0)
        {
        }

        if (transfer->tx_buff == NULL)
        {
            if (transfer->tx_default_enable)
            {
                tx_data = transfer->tx_default_val;
            }
        }
        else
        {
            if (transfer->frame_size > 16)
            {
                tx_data = (uint32_t) (transfer->tx_buff[0] | (transfer->tx_buff[1] << 8) |
                                     (transfer->tx_buff[2] << 16) | (transfer->tx_buff[3] << 24));
                transfer->tx_buff = transfer->tx_buff + 4U;
            }
            else if (transfer->frame_size > 8)
            {
                tx_data = (uint32_t)(transfer->tx_buff[0] | (transfer->tx_buff[1] << 8));
                transfer->tx_buff = transfer->tx_buff + 2;
            }
            else
            {
                tx_data = transfer->tx_buff[0];
                transfer->tx_buff = transfer->tx_buff + 1;
            }
        }

        lpspi->SPI_DR[0] = tx_data;
        transfer->tx_current_cnt++;

        /* wait for data in the Rx FIFO */
        while ((lpspi->SPI_SR & SPI_SR_RFNE) == 0)
        {
        }

        if (transfer->frame_size > 16)
        {
            *((uint32_t *) transfer->rx_buff) = lpspi->SPI_DR[0];
            transfer->rx_buff = ((uint32_t *)transfer->rx_buff) + 1U;
        }
        else if (transfer->frame_size > 8)
        {
            *((uint16_t *) transfer->rx_buff) = (uint16_t) (lpspi->SPI_DR[0]);
            transfer->rx_buff = ((uint16_t *)transfer->rx_buff) + 1U;
        }
        else
        {
            *((uint8_t *) transfer->rx_buff) = (uint8_t) (lpspi->SPI_DR[0]);
            transfer->rx_buff = ((uint8_t *)transfer->rx_buff) + 1U;
        }
    }
}

/**
  \fn          void spi_mw_transmit(SPI_Type *spi, bool is_slave)
  \brief       config microwire in transmit mode
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   is_slave  whether config as master/slave
  \return      none
*/
void spi_mw_transmit(SPI_Type *spi, bool is_slave)
{
    spi_disable(spi);
    spi->SPI_MWCR |= SPI_MWCR_MDD_TRANSMIT;
    spi_enable(spi);

    spi->SPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                    SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);

    if (is_slave)
    {
        spi->SPI_IMR |= (SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                         SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                         SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK);
    }
}

/**
  \fn          void spi_mw_receive(SPI_Type *spi,  spi_transfer_t *transfer)
  \brief       config microwire in receive mode
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  pointer to transfer structure
  \return      none
*/
void spi_mw_receive(SPI_Type *spi, spi_transfer_t *transfer)
{
    spi_disable(spi);
    spi->SPI_MWCR |= SPI_MWCR_MDD_RECEIVE;
    spi_enable(spi);

    spi->SPI_IMR = (SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK);

    if (transfer->is_master)
    {
        spi_disable(spi);
        spi->SPI_CTRLR1 = transfer->rx_total_cnt - 1;
        spi_enable(spi);

        spi->SPI_IMR |= (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                         SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                         SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
    }
}

/**
  \fn          void lpspi_set_mode(SPI_Type *spi, SPI_MODE mode)
  \brief       Set the mode for the LPSPI instance.
  \param[in]   spi     Pointer to the LPSPI register map
  \param[in]   mode    The mode to be set.
  \return      none
*/
void lpspi_set_mode(SPI_Type *lpspi, SPI_MODE mode)
{
    uint32_t val;

    spi_disable(lpspi);

    val = lpspi->SPI_CTRLR0;
    val &= ~(LPSPI_CTRLR0_SCPOL_HIGH | LPSPI_CTRLR0_SCPH_HIGH);

    switch (mode)
    {
        /* Clock Polarity 0, Clock Phase 0 */
        case SPI_MODE_0:
            break;

        /* Clock Polarity 0, Clock Phase 1 */
        case SPI_MODE_1:
            val |= (LPSPI_CTRLR0_SCPOL_LOW | LPSPI_CTRLR0_SCPH_HIGH);
            break;

        /* Clock Polarity 1, Clock Phase 0 */
        case SPI_MODE_2:
            val |= (LPSPI_CTRLR0_SCPOL_HIGH | LPSPI_CTRLR0_SCPH_LOW);
            break;

        /* Clock Polarity 1, Clock Phase 1 */
        case SPI_MODE_3:
            val |= (LPSPI_CTRLR0_SCPOL_HIGH | LPSPI_CTRLR0_SCPH_HIGH);
            break;
    }

    lpspi->SPI_CTRLR0 = val;
    spi_enable(lpspi);
}

/**
  \fn          void lpspi_set_protocol(SPI_Type *lpspi, SPI_PROTO format)
  \brief       Set the protocol format for the LPSPI instance.
  \param[in]   spi     Pointer to the LPSPI register map
  \param[in]   format  The protocol to be set
  \return      none
*/
void lpspi_set_protocol(SPI_Type *lpspi, SPI_PROTO format)
{
    uint32_t val;

    spi_disable(lpspi);

    val = lpspi->SPI_CTRLR0;
    val &= ~(LPSPI_CTRLR0_FRF_MASK);

    switch(format)
    {
        case SPI_PROTO_SPI:
            break;
        case SPI_PROTO_SSP:
            val |= LPSPI_CTRLR0_FRF_TI;
            break;
        case SPI_PROTO_MICROWIRE:
            val |= LPSPI_CTRLR0_FRF_MICROWIRE;
            break;
    }

    lpspi->SPI_CTRLR0 = val;
    spi_enable(lpspi);
}

/**
  \fn          void lpspi_set_dfs(SPI_Type *lpspi, uint8_t dfs)
  \brief       Set the data frame size for the LPSPI instance.
  \param[in]   spi     Pointer to the LPSPI register map
  \param[in]   dfs     The data frame size
  \return      none
*/
void lpspi_set_dfs(SPI_Type *lpspi, uint8_t dfs)
{
    uint32_t val = 0;

    spi_disable(lpspi);

    val = lpspi->SPI_CTRLR0;
    val &= ~LPSPI_CTRLR0_DFS32_MASK;
    val |= (dfs - 1)  << LPSPI_CTRLR0_DFS_32;
    lpspi->SPI_CTRLR0 = val;

    spi_enable(lpspi);
}

/**
  \fn          void lpspi_set_tmod(SPI_Type *lpspi, SPI_TMOD tmod)
  \brief       Set the transfer mode for the LPSPI instance.
  \param[in]   lpspi   Pointer to the LPSPI register map
  \param[in]   tmod    Transfer mode
  \return      none
*/
void lpspi_set_tmod(SPI_Type *lpspi, SPI_TMOD tmod)
{
    uint32_t val = 0;

    spi_disable(lpspi);

    val = lpspi->SPI_CTRLR0;
    val &= ~(LPSPI_CTRLR0_TMOD_MASK);

    switch(tmod)
    {
        case SPI_TMOD_TX_AND_RX:
            val |= LPSPI_CTRLR0_TMOD_TRANSFER;
            break;
        case SPI_TMOD_TX:
            val |= LPSPI_CTRLR0_TMOD_SEND_ONLY;
            break;
        case SPI_TMOD_RX:
            val |= LPSPI_CTRLR0_TMOD_RECEIVE_ONLY;
            break;
        case SPI_TMOD_EEPROM_READ:
            val |= LPSPI_CTRLR0_TMOD_EEPROM_READ_ONLY;
            break;
        default:
            break;
    }
    lpspi->SPI_CTRLR0 = val;

    spi_enable(lpspi);
}

/**
  \fn          SPI_TMOD lpspi_get_tmod(SPI_Type *lpspi)
  \brief       Get the transfer mode of the LPSPI instance.
  \param[in]   lpspi     Pointer to the LPSPI register map
  \return      The current transfer mode
*/
SPI_TMOD lpspi_get_tmod(SPI_Type *lpspi)
{
    uint32_t val = lpspi->SPI_CTRLR0;

    if ((val & LPSPI_CTRLR0_TMOD_MASK) == LPSPI_CTRLR0_TMOD_SEND_ONLY)
    {
        return SPI_TMOD_TX;
    }
    else if ((val & LPSPI_CTRLR0_TMOD_MASK) == LPSPI_CTRLR0_TMOD_RECEIVE_ONLY)
    {
        return SPI_TMOD_RX;
    }
    else if ((val & LPSPI_CTRLR0_TMOD_MASK) == LPSPI_CTRLR0_TMOD_TRANSFER)
    {
        return SPI_TMOD_TX_AND_RX;
    }
    else
    {
        return SPI_TMOD_EEPROM_READ;
    }
}

/**
  \fn          void lpspi_send(SPI_Type *lpspi)
  \brief       Prepare the SPI instance for transmission
  \param[in]   lpspi       Pointer to the LPSPI register map
  \return      none
*/
void lpspi_send(SPI_Type *lpspi)
{
    lpspi_set_tmod(lpspi, SPI_TMOD_TX);
    lpspi->SPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                    SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void lpspi_receive(SPI_Type *lpspi, uint32_t total_cnt)
  \brief       Prepare the LPSPI instance for reception
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   total_cnt total number of data count
  \return      none
*/
void lpspi_receive(SPI_Type *lpspi, uint32_t total_cnt)
{
    lpspi_set_tmod(lpspi, SPI_TMOD_RX);
    spi_disable(lpspi);
    lpspi->SPI_CTRLR1 = total_cnt - 1;
    spi_enable(lpspi);

    lpspi->SPI_IMR = (SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK);

    /* Initiate the receive operation by writing a dummy byte to the FIFO */
    lpspi->SPI_DR[0] = 0x0;
}

/**
  \fn          void lpspi_transfer(SPI_Type *lpspi)
  \brief       Prepare the LPSPI instance for transfer
  \param[in]   lpspi      Pointer to the LPSPI register map
  \return      none
*/
void lpspi_transfer(SPI_Type *lpspi)
{
    lpspi_set_tmod(lpspi, SPI_TMOD_TX_AND_RX);

    lpspi->SPI_IMR = (SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                    SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_FULL_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void lpspi_set_sste(SPI_Type *lpspi, bool enable)
  \brief       Enable/Disable Slave Select Toggle for the LPSPI instance
  \param[in]   lpspi     Pointer to the SPI register map
  \param[in]   enable    Enable/Disable control
  \return      none
*/
void lpspi_set_sste(SPI_Type *lpspi, bool enable)
{
    uint32_t val = lpspi->SPI_CTRLR0;

    spi_disable(lpspi);

    if (enable)
    {
        val |= LPSPI_CTRLR0_SSTE_ENABLE;
    }
    else
    {
        val &= ~LPSPI_CTRLR0_SSTE_ENABLE;
    }

    lpspi->SPI_CTRLR0 = val;
    spi_enable(lpspi);
}

/**
  \fn          uint32_t spi_dma_calc_rx_level(uint32_t total_cnt, uint8_t fifo_threshold)
  \brief       Calculate SPI DMA receive data level
  \param[in]   fifo_threshold  receive fifo threshold value
  \param[in]   total_cnt  total number of data count
  \return      final value after calculation
*/
uint32_t spi_dma_calc_rx_level(uint32_t total_cnt, uint8_t fifo_threshold)
{
    uint32_t temp;

    while (fifo_threshold > 0)
    {
        temp = total_cnt % fifo_threshold;
        total_cnt = fifo_threshold;
        fifo_threshold = temp;
    }

    return (total_cnt - 1) ;
}

/**
  \fn          void spi_dma_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for DMA send
  \param[in]   spi        Pointer to the SPI register map
  \return      none
*/
void spi_dma_send(SPI_Type *spi)
{
    spi_set_tmod(spi, SPI_TMOD_TX);

    /* Enable the TX DMA interface of SPI */
    spi_enable_tx_dma(spi);

    spi->SPI_IMR = (SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void spi_dma_receive(SPI_Type *spi, spi_transfer_t *transfer)
  \brief       Prepare the SPI instance for DMA reception
  \param[in]   spi       Pointer to the SPI register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void spi_dma_receive(SPI_Type *spi, spi_transfer_t *transfer)
{
    spi_set_tmod(spi, SPI_TMOD_RX);
    spi_disable(spi);
    spi->SPI_CTRLR1 = transfer->rx_total_cnt - 1;
    spi_enable(spi);

    /* Enable the RX DMA interface of SPI */
    spi_enable_rx_dma(spi);

    spi->SPI_IMR |= (SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK);

    if (transfer->is_master)
    {
        /* Initiate the receive operation by writing a dummy byte to the FIFO */
        spi->SPI_DR[0] = 0x0;
    }
}

/**
  \fn          void spi_dma_transfer(SPI_Type *spi)
  \brief       Prepare the SPI instance for DMA transfer
  \param[in]   spi       Pointer to the SPI register map
  \return      none
*/
void spi_dma_transfer(SPI_Type *spi)
{
    spi_set_tmod(spi, SPI_TMOD_TX_AND_RX);

    /* Enable the TX & RX DMA interface of SPI */
    spi_enable_tx_dma(spi);
    spi_enable_rx_dma(spi);

    spi->SPI_IMR = (SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                    SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void lpspi_dma_send(SPI_Type *spi)
  \brief       Prepare the SPI instance for DMA transmission
  \param[in]   lpspi       Pointer to the LPSPI register map
  \return      none
*/
void lpspi_dma_send(SPI_Type *lpspi)
{
    lpspi_set_tmod(lpspi, SPI_TMOD_TX);

    /* Enable the TX DMA interface of LPSPI */
    spi_enable_tx_dma(lpspi);

    lpspi->SPI_IMR = (SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void lpspi_dma_receive(SPI_Type *lpspi, uint32_t total_cnt)
  \brief       Prepare the LPSPI instance for DMA reception
  \param[in]   lpspi     Pointer to the LPSPI register map
  \param[in]   total_cnt total number of data count
  \return      none
*/
void lpspi_dma_receive(SPI_Type *lpspi, uint32_t total_cnt)
{
    lpspi_set_tmod(lpspi, SPI_TMOD_RX);
    spi_disable(lpspi);
    lpspi->SPI_CTRLR1 = total_cnt - 1;
    spi_enable(lpspi);

    /* Enable the RX DMA interface of LPSPI */
    spi_enable_rx_dma(lpspi);

    lpspi->SPI_IMR |= (SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK);

    /* Initiate the receive operation by writing a dummy byte to the FIFO */
    lpspi->SPI_DR[0] = 0x0;
}

/**
  \fn          void lpspi_dma_transfer(SPI_Type *lpspi)
  \brief       Prepare the LPSPI instance for DMA transfer
  \param[in]   lpspi      Pointer to the LPSPI register map
  \return      none
*/
void lpspi_dma_transfer(SPI_Type *lpspi)
{
    lpspi_set_tmod(lpspi, SPI_TMOD_TX_AND_RX);

    /* Enable the TX & RX DMA interface of LPSPI */
    spi_enable_tx_dma(lpspi);
    spi_enable_rx_dma(lpspi);

    lpspi->SPI_IMR = (SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_RX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                      SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
}

/**
  \fn          void spi_irq_handler(SPI_Type *spi, spi_master_transfer_t *transfer)
  \brief       Handle interrupts for the SPI instance.
  \param[in]   spi         Pointer to the SPI register map
  \param[in]   transfer    The transfer structure for the SPI instance
  \return      none
*/
void spi_irq_handler(SPI_Type *spi, spi_transfer_t *transfer)
{
    uint32_t event, tx_data, index, curr_fifo_level;
    uint32_t tx_count, rx_count;

    event = spi->SPI_ISR;

    if (event & SPI_TX_FIFO_EMPTY_EVENT)
    {
        curr_fifo_level = spi->SPI_TXFLR;

        if (transfer->tx_total_cnt >= (transfer->tx_current_cnt + SPI_TX_FIFO_DEPTH - curr_fifo_level))
        {
            tx_count = SPI_TX_FIFO_DEPTH - curr_fifo_level;
        }
        else
        {
            tx_count = (transfer->tx_total_cnt - transfer->tx_current_cnt);
        }

        for (index = 0; index < tx_count; index++)
        {
            tx_data = 0;

            if (transfer->tx_buff == NULL)
            {
                if (transfer->tx_default_enable)
                {
                    tx_data = transfer->tx_default_val;
                }
            }
            else
            {
                if (transfer->frame_size > 16)
                {
                    tx_data = (uint32_t) (transfer->tx_buff[0] | (transfer->tx_buff[1] << 8) | (transfer->tx_buff[2] << 16) | (transfer->tx_buff[3] << 24));
                    transfer->tx_buff = transfer->tx_buff + 4U;
                }
                else if (transfer->frame_size > 8)
                {
                    tx_data = (uint32_t)(transfer->tx_buff[0] | (transfer->tx_buff[1] << 8));
                    transfer->tx_buff = transfer->tx_buff + 2;
                }
                else
                {
                    tx_data = transfer->tx_buff[0];
                    transfer->tx_buff = transfer->tx_buff + 1;
                }
            }

            spi->SPI_DR[0] = tx_data;
            transfer->tx_current_cnt++;
        }
    }

    if (event & SPI_RX_FIFO_FULL_EVENT)
    {
        rx_count = spi->SPI_RXFLR;

        if (transfer->frame_size > 16)
        {
            for (index = 0; index < rx_count; index++)
            {
                *((uint32_t *) transfer->rx_buff) = spi->SPI_DR[0];

                transfer->rx_buff = ((uint32_t *)transfer->rx_buff) + 1U;
                transfer->rx_current_cnt++;
            }
        }
        else if (transfer->frame_size > 8)
        {
            for (index = 0; index < rx_count; index++)
            {
                *((uint16_t *) transfer->rx_buff) = (uint16_t) (spi->SPI_DR[0]);

                transfer->rx_buff = ((uint16_t *)transfer->rx_buff) + 1U;
                transfer->rx_current_cnt++;
            }
        }
        else
        {
            for (index = 0; index < rx_count; index++)
            {
                *((uint8_t *) transfer->rx_buff) = (uint8_t) (spi->SPI_DR[0]);

                transfer->rx_buff = ((uint8_t *)transfer->rx_buff) + 1U;
                transfer->rx_current_cnt++;
            }
        }

        /* Rx threshold is configured as greater than zero */
        if (spi->SPI_RXFTLR)
        {
            if ((transfer->rx_total_cnt - transfer->rx_current_cnt) <= spi->SPI_RXFTLR)
            {
                spi->SPI_RXFTLR = ((transfer->rx_total_cnt - transfer->rx_current_cnt) - 1U);
            }
        }
    }

    if (event & (SPI_RX_FIFO_OVER_FLOW_EVENT | SPI_TX_FIFO_OVER_FLOW_EVENT))
    {
        /* clear interrupt events */
        (void) spi->SPI_TXOICR;
        (void) spi->SPI_RXOICR;

        spi_disable(spi);
        spi_enable(spi);

        transfer->status = SPI_TRANSFER_STATUS_OVERFLOW;
    }

    if (event & SPI_MULTI_MASTER_CONTENTION_EVENT)
    {
        /* clear interrupt event */
        (void) spi->SPI_MSTICR;

        transfer->status = SPI_TRANSFER_STATUS_MASTER_CONTENTION;
    }

    if (event & SPI_RX_FIFO_UNDER_FLOW_EVENT)
    {
        /* clear interrupt event */
        (void) spi->SPI_RXUICR;

        transfer->status = SPI_TRANSFER_STATUS_RX_UNDERFLOW;
    }

    /* SEND ONLY mode : check if the transfer is finished */
    if ((transfer->mode == SPI_TMOD_TX) &&
           (transfer->tx_total_cnt == transfer->tx_current_cnt))
    {
        /* Wait for the transfer to complete */
        if(!spi_busy(spi))
        {
            /* Mask the TX interrupts */
            spi->SPI_IMR &= ~(SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
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
        spi->SPI_IMR &= ~(SPI_IMR_RX_FIFO_UNDER_FLOW_INTERRUPT_MASK |
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
        spi->SPI_IMR = 0;

        spi_disable(spi);

        transfer->rx_current_cnt = 0;
        transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
    }

    if((transfer->mode == SPI_TMOD_TX_AND_RX) && (transfer->tx_total_cnt == transfer->tx_current_cnt))
    {
        if((spi->SPI_SR & SPI_SR_TX_FIFO_EMPTY) == SPI_SR_TX_FIFO_EMPTY)
        {
            /* Reset the Tx FIFO start level */
            spi->SPI_TXFTLR &= ~(0xFFU << SPI_TXFTLR_TXFTHR_SHIFT);

            /* Mask the TX interrupts */
            spi->SPI_IMR &= ~(SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK |
                              SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK |
                              SPI_IMR_MULTI_MASTER_CONTENTION_INTERRUPT_MASK);
        }
    }
}

/**
  \fn          void spi_irq_handler(SPI_Type *spi, spi_master_transfer_t *transfer)
  \brief       Handle interrupts for the MW frame format.
  \param[in]   spi         Pointer to the SPI register map
  \param[in]   transfer    The transfer structure for the SPI instance
  \return      none
*/
void spi_mw_irq_handler(SPI_Type *spi, spi_transfer_t *transfer)
{
    uint32_t event, tx_data, index, curr_fifo_level;
    uint32_t tx_count, rx_count;

    event = spi->SPI_ISR;

    if (event & SPI_TX_FIFO_EMPTY_EVENT)
    {
        curr_fifo_level = spi->SPI_TXFLR;

        if (transfer->tx_total_cnt >= (transfer->tx_current_cnt + SPI_TX_FIFO_DEPTH - curr_fifo_level))
        {
            tx_count = SPI_TX_FIFO_DEPTH - curr_fifo_level;
        }
        else
        {
            tx_count = (transfer->tx_total_cnt - transfer->tx_current_cnt);
        }

        for (index = 0; index < tx_count; index++)
        {
            tx_data = 0;

            if (transfer->tx_buff == NULL)
            {
                if (transfer->tx_default_enable)
                {
                    tx_data = transfer->tx_default_val;
                }
            }
            else
            {
                /* local buffer size will be width of 4 bytes and hardware will take care of transferring
                 * proper data and control codes based on their size configured */
                tx_data = (uint32_t) (transfer->tx_buff[0] | (transfer->tx_buff[1] << 8) | (transfer->tx_buff[2] << 16) | (transfer->tx_buff[3] << 24));
                transfer->tx_buff = transfer->tx_buff + 4U;
            }

            spi->SPI_DR[0] = tx_data;
            transfer->tx_current_cnt++;
        }
    }

    if (event & SPI_RX_FIFO_FULL_EVENT)
    {
        rx_count = spi->SPI_RXFLR;

        for (index = 0; index < rx_count; index++)
        {
            /* local buffer size will be width of 4 bytes and hardware will take care of receiving
             * proper data and control codes based on their size configured */
            *((uint32_t *) transfer->rx_buff) = spi->SPI_DR[0];

            transfer->rx_buff = ((uint32_t *)transfer->rx_buff) + 1U;
            transfer->rx_current_cnt++;
        }

        /* Rx threshold is configured as greater than zero */
        if (spi->SPI_RXFTLR)
        {
            if ((transfer->rx_total_cnt - transfer->rx_current_cnt) <= spi->SPI_RXFTLR)
            {
                spi->SPI_RXFTLR = ((transfer->rx_total_cnt - transfer->rx_current_cnt) - 1U);
            }
        }
    }

    if (event & (SPI_RX_FIFO_OVER_FLOW_EVENT | SPI_TX_FIFO_OVER_FLOW_EVENT))
    {
        /* clear interrupt events */
        (void) spi->SPI_TXOICR;
        (void) spi->SPI_RXOICR;

        spi_disable(spi);
        spi_enable(spi);

        transfer->status = SPI_TRANSFER_STATUS_OVERFLOW;
    }

    if (event & SPI_MULTI_MASTER_CONTENTION_EVENT)
    {
        /* clear interrupt event */
        (void) spi->SPI_MSTICR;

        transfer->status = SPI_TRANSFER_STATUS_MASTER_CONTENTION;
    }

    if (event & SPI_RX_FIFO_UNDER_FLOW_EVENT)
    {
        /* clear interrupt event */
        (void) spi->SPI_RXUICR;

        transfer->status = SPI_TRANSFER_STATUS_RX_UNDERFLOW;
    }

    if (spi->SPI_MWCR & SPI_MWCR_MDD_TRANSMIT)
    {
        if (transfer->tx_total_cnt == transfer->tx_current_cnt)
        {
            if (transfer->is_master)
            {
                if(!spi_busy(spi))
                {
                    spi->SPI_IMR &= ~(SPI_IMR_TX_FIFO_EMPTY_INTERRUPT_MASK | SPI_IMR_TX_FIFO_OVER_FLOW_INTERRUPT_MASK);
                    transfer->tx_current_cnt = 0;
                    transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
                }
            }
            else
            {
                if (transfer->rx_total_cnt == transfer->rx_current_cnt)
                {
                    spi->SPI_IMR = 0;
                    transfer->rx_current_cnt = 0;
                    transfer->tx_current_cnt = 0;
                    transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
                }
            }
        }
    }
    else
    {
        if (transfer->rx_total_cnt == transfer->rx_current_cnt)
        {
            spi->SPI_IMR = 0;
            transfer->rx_current_cnt = 0;
            transfer->status = SPI_TRANSFER_STATUS_COMPLETE;
        }
    }
}
