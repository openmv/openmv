/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef LPI2C_H_
#define LPI2C_H_

#include <stdint.h>
#include <stdbool.h>

/* lpi2c register set */
typedef struct {
  volatile uint32_t LPI2C_DATA;             /*!< (@ 0x00000000) Data read/write register               */
  volatile uint32_t reserved1[3];
  volatile uint32_t LPI2C_INBOND_DATA;      /*!< (@ 0x00000010) inbond fifo(incoming data from master) */
  volatile uint32_t reserved2[3];
  volatile uint32_t LPI2C_OUTBOND_DATA;     /*!< (@ 0x00000020) outbond fifo(outgoing data to master)  */
}LPI2C_TYPE;

/* Macro */
#define LPI2C_FIFO_EMPTY              (0x20)    /* LPI2C Fifo empty     */
#define LPI2C_FIFO_FULL               (0x10)    /* LPI2C Fifo full      */
#define LPI2C_AVL_DATA                (0X0F)    /* LPI2C number of data */

/**
 * enum LPI2C_XFER_STATUS.
 * Status of an ongoing LPI2C transfer.
 */
typedef enum _LPI2C_XFER_STATUS {
  LPI2C_XFER_STAT_NONE        = (0U),         /* LPI2C transfer status none     */
  LPI2C_XFER_STAT_COMPLETE    = (1U << 0),    /* LPI2C transfer status complete */
  LPI2C_XFER_STAT_ERROR       = (1U << 1),    /* LPI2C transfer status complete */
} LPI2C_XFER_STATUS;

/* lpi2c transfer Information (Run-Time) */
typedef struct _LPI2C_XFER_INFO_T
{
  const uint8_t                   *tx_buf;           /* Pointer to out data buffer                                     */
  uint32_t                         tx_total_num;     /* Total number of data to be send                                */
  volatile uint32_t                tx_curr_cnt;      /* current Number of data sent from total num                     */
  uint8_t                         *rx_buf;           /* Pointer to in data buffer                                      */
  uint32_t                         rx_total_num;     /* Total number of data to be received                            */
  volatile uint32_t                rx_curr_cnt;      /* Number of data received                                        */
  volatile LPI2C_XFER_STATUS       status;           /* transfer status                                                */
} LPI2C_XFER_INFO_T;

/**
 * @func   : void lpi2c_wr_tx_fifo(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer)
 * @brief  : writing to the register for transmit data
 * @param  : lpi2c    : Pointer to lpi2c register map
 * @param  : transfer : Pointer to LPI2C_XFER_INFO_T
 * @retval : callback event
 */
void lpi2c_send(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer);

/**
 * @func   : void lpi2c_irq_handler(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer)
 * @brief  : lpi2c irq handler
 * @param  : lpi2c    : Pointer to lpi2c register map
 * @param  : transfer : Pointer to LPI2C_XFER_INFO_T
 * @retval : callback event
 */
void lpi2c_irq_handler(LPI2C_TYPE *lpi2c, LPI2C_XFER_INFO_T *transfer);

#endif /* LPI2C_H_ */
