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
 * @file     i2s.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     06-06-2023
 * @brief    Low Level Source File for I2S
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "i2s.h"

/**
  \fn          void i2s_tx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Handle interrupts for the I2S Tx.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  The transfer structure for the I2S instance
  \return      none
*/
void i2s_tx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer)
{
    uint32_t isr           = i2s->I2S_ISR0;
    uint32_t tx_fifo_avail = I2S_FIFO_DEPTH - i2s->I2S_TFCR0 - 1;
    const uint8_t *buff    = transfer->tx_buff;
    uint8_t last_lap       = 0, num_bytes = 0, count = 0;
    I2S_WLEN wlen          = (I2S_WLEN)i2s->I2S_TCR0;
    uint32_t frames        = 0;

    /* Copy the data only for the interrupt mode */
    if((isr & I2S_ISR_TXFE) && !(i2s->I2S_DMACR & I2S_DMACR_DMAEN_TXBLOCK))
    {
        if((wlen > I2S_WLEN_RES_NONE) && (wlen <= I2S_WLEN_RES_16_BIT))
            num_bytes = I2S_16BIT_BUF_TYPE_BYTES;
        else
            num_bytes = I2S_32BIT_BUF_TYPE_BYTES;

        /* Check if it is the last lap */
        if((transfer->tx_current_cnt + (2 * tx_fifo_avail * num_bytes)) > transfer->tx_total_cnt)
        {
            /* Assign the number of iterations required */
            frames = (transfer->tx_total_cnt - transfer->tx_current_cnt) / (2 * num_bytes);
            last_lap = 1;
        }
        else
        {
            frames = tx_fifo_avail;
        }

        for(count = 0; count < frames; count++)
        {
            /* Assuming that application uses 16bit buffer for 16bit data resolution */
            if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
            {
                if(transfer->mono_mode)
                {
                    i2s->I2S_LTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt);
                    i2s->I2S_RTHR0 = 0U;
                    transfer->tx_current_cnt += num_bytes;
                }
                else
                {
                    i2s->I2S_LTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt);
                    i2s->I2S_RTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt + num_bytes);
                    transfer->tx_current_cnt += (2 * num_bytes);
                }
            }
            else /* For > 16bit data resolution consider as 32bit buffer */
            {
                if(transfer->mono_mode)
                {
                    i2s->I2S_LTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt);
                    i2s->I2S_RTHR0 = 0U;
                    transfer->tx_current_cnt += num_bytes;
                }
                else
                {
                    i2s->I2S_LTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt);
                    i2s->I2S_RTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt + num_bytes);
                    transfer->tx_current_cnt += (2 * num_bytes);
                }
            }
        }

        if(last_lap && (transfer->tx_current_cnt < transfer->tx_total_cnt))
        {
            if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
            {
                /* Write the Left sample and fill right with 0 */
                i2s->I2S_LTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt);
                i2s->I2S_RTHR0 = 0U;
                transfer->tx_current_cnt += num_bytes;
            }
            else
            {
                /* Write the Left sample and fill right with 0 */
                i2s->I2S_LTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt);
                i2s->I2S_RTHR0 = 0U;
                transfer->tx_current_cnt += num_bytes;
            }
        }

        /* Send complete event once all the data is copied to FIFO */
        if(transfer->tx_current_cnt >= transfer->tx_total_cnt)
        {
            /* Disable Tx Interrupt */
            i2s_disable_tx_interrupt(i2s);

            transfer->status |= I2S_TRANSFER_STATUS_TX_COMPLETE;
        }

    }

    /* This should not happen */
    if(isr & I2S_ISR_TXFO)
    {
        i2s_clear_tx_overrun(i2s);
    }
}

/**
  \fn          void i2s_rx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Handle interrupts for the I2S Rx.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  The transfer structure for the I2S instance
  \return      none
*/
void i2s_rx_irq_handler(I2S_Type *i2s, i2s_transfer_t *transfer)
{
    uint32_t rx_fifo_avail = i2s->I2S_RFCR0 + 1;
    uint8_t *const buff    = transfer->rx_buff;
    uint8_t last_lap       = 0, num_bytes = 0, count = 0;
    I2S_WLEN wlen          = (I2S_WLEN)i2s->I2S_RCR0;
    uint32_t isr           = i2s->I2S_ISR0;
    uint32_t frames        = 0;

    if(isr & I2S_ISR_RXFO)
    {
        /* Clear overrun interrupt */
        i2s_clear_rx_overrun(i2s);

        /*
         * Disable the Rx Overflow interrupt for now. This will
         * be enabled again when Receive function is called
         */
        i2s_disable_rx_overflow_interrupt(i2s);

        transfer->status |= I2S_TRANSFER_STATUS_RX_OVERFLOW;
    }

    /* Copy the data only for the interrupt mode */
    if((isr & I2S_ISR_RXDA) && !(i2s->I2S_DMACR & I2S_DMACR_DMAEN_RXBLOCK))
    {
        if((wlen > I2S_WLEN_RES_NONE) && (wlen <= I2S_WLEN_RES_16_BIT))
            num_bytes = I2S_16BIT_BUF_TYPE_BYTES;
        else
            num_bytes = I2S_32BIT_BUF_TYPE_BYTES;

        /* Check if it is the last lap */
        if((transfer->rx_current_cnt + (2 * rx_fifo_avail * num_bytes)) > transfer->rx_total_cnt)
        {
            /* Assign the number of iterations required */
            frames = (transfer->rx_total_cnt - transfer->rx_current_cnt) / (2 * num_bytes);
            last_lap = 1;
        }
        else
        {
            frames = rx_fifo_avail;
        }

        for(count = 0; count < frames; count++)
        {
            /* Assuming that application uses 16bit buffer for 16bit data resolution */
            if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
            {
                uint16_t left_data  = (uint16_t)i2s->I2S_LRBR0;
                uint16_t right_data = (uint16_t)i2s->I2S_RRBR0;

                if(transfer->mono_mode)
                {
                    *(uint16_t*)(buff + transfer->rx_current_cnt) = left_data;
                    transfer->rx_current_cnt += num_bytes;
                }
                else
                {
                    *(uint16_t*)(buff + transfer->rx_current_cnt) = left_data;
                    *(uint16_t*)(buff + transfer->rx_current_cnt + num_bytes) = right_data;
                    transfer->rx_current_cnt += (2 * num_bytes);
                }
            }
            else /* For > 16bit data resolution consider as 32bit buffer*/
            {

                uint32_t left_data  = i2s->I2S_LRBR0;
                uint32_t right_data = i2s->I2S_RRBR0;

                if(transfer->mono_mode)
                {
                    *(uint32_t*)(buff + transfer->rx_current_cnt) = left_data;
                    transfer->rx_current_cnt += num_bytes;
                }
                else
                {
                    *(uint32_t*)(buff + transfer->rx_current_cnt) = left_data;
                    *(uint32_t*)(buff + transfer->rx_current_cnt + num_bytes) = right_data;
                    transfer->rx_current_cnt += (2 * num_bytes);
                }
            }
        }

        if(last_lap && (transfer->rx_current_cnt < transfer->rx_total_cnt))
        {
            if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
            {
                uint16_t left_data  = (uint16_t)i2s->I2S_LRBR0;

                /* Read the last sample from left */
                *(uint16_t*)(buff + transfer->rx_current_cnt) = left_data;
                (void)i2s->I2S_RRBR0;
                transfer->rx_current_cnt = transfer->rx_current_cnt + num_bytes;
            }
            else
            {
                uint32_t left_data  = i2s->I2S_LRBR0;

                /* Read the last sample from left */
                *(uint32_t*)(buff + transfer->rx_current_cnt) = left_data;
                (void)i2s->I2S_RRBR0;
                transfer->rx_current_cnt = transfer->rx_current_cnt + num_bytes;
            }
        }

        /* Once the buffer is full, send complete event with interrupt disabled */
        if(transfer->rx_current_cnt >= transfer->rx_total_cnt)
        {
            /* Disable Rx Interrupt */
            i2s_disable_rx_interrupt(i2s);

            transfer->status |= I2S_TRANSFER_STATUS_RX_COMPLETE;
        }
    }

}

/**
  \fn          void i2s_send_blocking(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Execute a blocking I2S send described by the transfer structure.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void i2s_send_blocking(I2S_Type *i2s, i2s_transfer_t *transfer)
{
    const uint8_t *buff    = transfer->tx_buff;
    I2S_WLEN wlen          = (I2S_WLEN)i2s->I2S_TCR0;
    uint8_t num_bytes      = 0;
    uint32_t frames        = 0;
    uint32_t count         = 0;

    /* Enable Tx Channel */
    i2s_txchannel_enable(i2s);

    if((wlen > I2S_WLEN_RES_NONE) && (wlen <= I2S_WLEN_RES_16_BIT))
        num_bytes = I2S_16BIT_BUF_TYPE_BYTES;
    else
        num_bytes = I2S_32BIT_BUF_TYPE_BYTES;

    frames = transfer->tx_total_cnt / (2 * num_bytes);

    for(count = 0; count < frames; count++)
    {
        /* Wait Till FIFO gets empty */
        while(!(i2s->I2S_ISR0 & I2S_ISR_TXFE));

        /* Assuming that application uses 16bit buffer for 16bit data resolution */
        if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
        {
            if(transfer->mono_mode)
            {
                i2s->I2S_LTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt);
                i2s->I2S_RTHR0 = 0U;
                transfer->tx_current_cnt += num_bytes;
            }
            else
            {
                i2s->I2S_LTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt);
                i2s->I2S_RTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt + num_bytes);
                transfer->tx_current_cnt += (2 * num_bytes);
            }
        }
        else /* For > 16bit data resolution consider as 32bit buffer */
        {
            if(transfer->mono_mode)
            {
                i2s->I2S_LTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt);
                i2s->I2S_RTHR0 = 0U;
                transfer->tx_current_cnt += num_bytes;
            }
            else
            {
                i2s->I2S_LTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt);
                i2s->I2S_RTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt + num_bytes);
                transfer->tx_current_cnt += (2 * num_bytes);
            }
        }
    }

    /* Last Lap */
    if(transfer->tx_current_cnt < transfer->tx_total_cnt)
    {
        /* Wait Till FIFO gets empty */
        while(!(i2s->I2S_ISR0 & I2S_ISR_TXFE));

        if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
        {
            /* Write the Left sample and fill right with 0 */
            i2s->I2S_LTHR0 = *(const uint16_t*)(buff + transfer->tx_current_cnt);
            i2s->I2S_RTHR0 = 0U;
            transfer->tx_current_cnt += num_bytes;
        }
        else
        {
            /* Write the Left sample and fill right with 0 */
            i2s->I2S_LTHR0 = *(const uint32_t*)(buff + transfer->tx_current_cnt);
            i2s->I2S_RTHR0 = 0U;
            transfer->tx_current_cnt += num_bytes;
        }
    }

    /* Set complete event */
    transfer->status |= I2S_TRANSFER_STATUS_TX_COMPLETE;
}

/**
  \fn          void i2s_receive_blocking(I2S_Type *i2s, i2s_transfer_t *transfer)
  \brief       Execute a blocking I2S receive described by the transfer structure.
  \param[in]   i2s       Pointer to the I2S register map
  \param[in]   transfer  Pointer to transfer structure
  \return      none
*/
void i2s_receive_blocking(I2S_Type *i2s, i2s_transfer_t *transfer)
{
    uint8_t *const buff    = transfer->rx_buff;
    I2S_WLEN wlen          = (I2S_WLEN)i2s->I2S_RCR0;
    uint8_t num_bytes      = 0;
    uint32_t frames        = 0;
    uint32_t count         = 0;

    /* Enable Rx Channel */
    i2s_rxchannel_enable(i2s);

    if((wlen > I2S_WLEN_RES_NONE) && (wlen <= I2S_WLEN_RES_16_BIT))
        num_bytes = I2S_16BIT_BUF_TYPE_BYTES;
    else
        num_bytes = I2S_32BIT_BUF_TYPE_BYTES;

    /* Assign the number of iterations required */
    frames = transfer->rx_total_cnt / (2 * num_bytes);

    for(count = 0; count < frames; count++)
    {
        /* Wait Till DATA is available  */
        while(!(i2s->I2S_ISR0 & I2S_ISR_RXDA));

        /* Assuming that application uses 16bit buffer for 16bit data resolution */
        if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
        {
            uint16_t left_data  = (uint16_t)i2s->I2S_LRBR0;
            uint16_t right_data = (uint16_t)i2s->I2S_RRBR0;

            if(transfer->mono_mode)
            {
                *(uint16_t*)(buff + transfer->rx_current_cnt) = left_data;
                transfer->rx_current_cnt += num_bytes;
            }
            else
            {
                *(uint16_t*)(buff + transfer->rx_current_cnt) = left_data;
                *(uint16_t*)(buff + transfer->rx_current_cnt + num_bytes) = right_data;
                transfer->rx_current_cnt += (2 * num_bytes);
            }
        }
        else /* For > 16bit data resolution consider as 32bit buffer*/
        {

            uint32_t left_data  = i2s->I2S_LRBR0;
            uint32_t right_data = i2s->I2S_RRBR0;

            if(transfer->mono_mode)
            {
                *(uint32_t*)(buff + transfer->rx_current_cnt) = left_data;
                transfer->rx_current_cnt += num_bytes;
            }
            else
            {
                *(uint32_t*)(buff + transfer->rx_current_cnt) = left_data;
                *(uint32_t*)(buff + transfer->rx_current_cnt + num_bytes) = right_data;
                transfer->rx_current_cnt += (2 * num_bytes);
            }
        }
    }

    if(transfer->rx_current_cnt < transfer->rx_total_cnt)
    {
        /* Wait Till DATA is available  */
        while(!(i2s->I2S_ISR0 & I2S_ISR_RXDA));

        if(num_bytes == I2S_16BIT_BUF_TYPE_BYTES)
        {
            uint16_t left_data  = (uint16_t)i2s->I2S_LRBR0;

            /* Read the last sample from left */
            *(uint16_t*)(buff + transfer->rx_current_cnt) = left_data;
            (void)i2s->I2S_RRBR0;
            transfer->rx_current_cnt = transfer->rx_current_cnt + num_bytes;
        }
        else
        {
            uint32_t left_data  = i2s->I2S_LRBR0;

            /* Read the last sample from left */
            *(uint32_t*)(buff + transfer->rx_current_cnt) = left_data;
            (void)i2s->I2S_RRBR0;
            transfer->rx_current_cnt = transfer->rx_current_cnt + num_bytes;
        }
    }


    if(i2s->I2S_ISR0 & I2S_ISR_RXFO)
    {
        /* Clear overrun interrupt */
        i2s_clear_rx_overrun(i2s);

        transfer->status |= I2S_TRANSFER_STATUS_RX_OVERFLOW;
    }

    /*send complete event */
    transfer->status |= I2S_TRANSFER_STATUS_RX_COMPLETE;
}
