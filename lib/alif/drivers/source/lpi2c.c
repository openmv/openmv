/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include "lpi2c.h"

/**
 * @func   : void lpi2c_send(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer)
 * @brief  : writing to the register for transmit data
 * @param  : lpi2c    : Pointer to lpi2c register map
 * @param  : transfer : Pointer to LPI2C_XFER_INFO_T
 * @retval : callback event
 */
void lpi2c_send(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer)
{
    /* check current count is less than total count */
    while (transfer->tx_curr_cnt < transfer->tx_total_num)
    {
        /* Check for Fifo full */
        if (!(lpi2c->LPI2C_OUTBOND_DATA & LPI2C_FIFO_FULL))
        {
            /*writing data to fifo*/
           lpi2c->LPI2C_DATA = transfer->tx_buf[transfer->tx_curr_cnt++];
        }
    }

    /* transfer complete */
    transfer->status = LPI2C_XFER_STAT_COMPLETE;
}

/**
 * @func   : void lpi2c_irq_handler(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer)
 * @brief  : lpi2c irq handler
 * @param  : lpi2c    : Pointer to lpi2c register map
 * @param  : transfer : Pointer to LPI2C_XFER_INFO_T
 * @retval : callback event
 */
void lpi2c_irq_handler(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer)
{
  /* Storing receive value to the buffer */
  transfer->rx_buf[transfer->rx_curr_cnt++] = lpi2c->LPI2C_DATA;

  if (transfer->rx_curr_cnt == (transfer->rx_total_num))
  {
     transfer->status = LPI2C_XFER_STAT_COMPLETE;
  }
}
