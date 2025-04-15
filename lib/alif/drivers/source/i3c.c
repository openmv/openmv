/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "i3c.h"
#include "string.h"


/**
  \fn           void i3c_wr_tx_fifo(I3C_Type        *i3c,
                                    const  uint8_t  *bytes,
                                           uint32_t  nbytes)
  \brief        Write data to i3c TX FIFO
  \param[in]    i3c     : Pointer to i3c resources structure
  \param[in]    bytes   : Pointer to buffer with data which
                           needs to be write to i3c transmitter
  \param[in]    nbytes  : Number of bytes needs to be write
  \return       none
*/
static void i3c_wr_tx_fifo(I3C_Type        *i3c,
                           const  uint8_t  *bytes,
                                  uint32_t  nbytes)
{
  uint32_t len_in_words = nbytes / 4;
  uint32_t i, tmp;

  /* FIXME Add check for MAX TX FIFO Length */
  for (i = 0; i < len_in_words; i++)
  {
    /* FIXME Fix Memory Alignment issue */
    tmp = *((uint32_t *) bytes);
    i3c->I3C_TX_DATA_PORT = tmp;
    bytes += 4;
  }

  /* write the remaining bytes in the last incomplete word */
  if (nbytes & 3)
  {
    tmp = 0;
    memcpy(&tmp, bytes, nbytes & 3);
    i3c->I3C_TX_DATA_PORT = tmp;
  }
}

/**
  \fn           void i3c_read_rx_fifo(I3C_Type *i3c,
                                      uint8_t  *bytes,
                                      uint32_t  nbytes)
  \brief        Read data from i3c RX FIFO
  \param[in]    i3c      : Pointer to i3c resources structure
  \param[in]    bytes    : Pointer to buffer for data
                            to receive from i3c RX FIFO
  \param[in]    nbytes   : Number of bytes needs to be receive
  \return       none
*/
static void i3c_read_rx_fifo(I3C_Type *i3c,
                             uint8_t  *bytes,
                             uint32_t  nbytes)
{
  uint32_t len_in_words = nbytes / 4;
  uint32_t i, tmp;

  /* FIXME Add check for MAX RX FIFO Length */
  for (i = 0; i < len_in_words; i++)
  {
    /* FIXME Fix Memory Alignment issue */
    *((uint32_t *) bytes) = i3c->I3C_RX_DATA_PORT;
    bytes += 4;
  }

  /* read the last word and copy the actual remaining data */
  if (nbytes & 3)
  {
    tmp = i3c->I3C_RX_DATA_PORT;
    memcpy(bytes, &tmp, nbytes & 3);
  }
}

/**
  \fn           void i3c_enqueue_xfer(I3C_Type *i3c,
                                      I3C_XFER *xfer)
  \brief        Add commands to i3c Command Queue
  \param[in]    i3c  : Pointer to i3c resources structure
  \param[in]    xfer : Pointer to i3c transfer structure
  \return       none
*/
static void i3c_enqueue_xfer(I3C_Type *i3c,
                             I3C_XFER *xfer)
{
  uint32_t thld_ctrl;

  /* write data to tx port (if any) */
  if (xfer->tx_buf)
  {
    i3c_wr_tx_fifo(i3c, xfer->tx_buf, xfer->tx_len);
  }

  thld_ctrl = i3c->I3C_QUEUE_THLD_CTRL;

  thld_ctrl &= ~QUEUE_THLD_CTRL_RESP_BUF_MASK;
  /* set up for an interrupt after one response */
  thld_ctrl |= QUEUE_THLD_CTRL_RESP_BUF(1);

  i3c->I3C_QUEUE_THLD_CTRL = thld_ctrl;

  if(xfer->cmd_hi)
  {
    i3c->I3C_COMMAND_QUEUE_PORT = xfer->cmd_hi;
  }

  if(xfer->cmd_lo)
  {
    i3c->I3C_COMMAND_QUEUE_PORT = xfer->cmd_lo;
  }
}

/**
  \fn           void i3c_master_tx(I3C_Type *i3c,
                                   I3C_XFER *xfer,
                                   uint32_t  index,
                                   uint16_t  len)
  \brief        send master transmit command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    len     : Transmit length
  \return       none
*/
void i3c_master_tx(I3C_Type *i3c,
                   I3C_XFER *xfer,
                   uint32_t  index,
                   uint16_t  len)
{
  xfer->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(len) |
                 COMMAND_PORT_TRANSFER_ARG;

  xfer->cmd_lo = COMMAND_PORT_SPEED(0)            |
                 COMMAND_PORT_TID(I3C_MST_TX_TID) |
                 COMMAND_PORT_DEV_INDEX(index)    |
                 COMMAND_PORT_ROC                 |
                 COMMAND_PORT_TOC;

  /* Add commands to i3c Command Queue */
  i3c_enqueue_xfer(i3c, xfer);
}

/**
  \fn           void i3c_master_rx(I3C_Type *i3c,
                                   I3C_XFER *xfer,
                                   uint32_t  index,
                                   uint16_t  len)
  \brief        send master receive command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    len     : Receive length
  \return       none
*/
void i3c_master_rx(I3C_Type *i3c,
                   I3C_XFER *xfer,
                   uint32_t  index,
                   uint16_t  len)
{
  xfer->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(len) |
                 COMMAND_PORT_TRANSFER_ARG;

  xfer->cmd_lo = COMMAND_PORT_READ_TRANSFER       |
                 COMMAND_PORT_SPEED(0)            |
                 COMMAND_PORT_TID(I3C_MST_RX_TID) |
                 COMMAND_PORT_DEV_INDEX(index)    |
                 COMMAND_PORT_ROC                 |
                 COMMAND_PORT_TOC;

  /* Add commands to i3c Command Queue */
  i3c_enqueue_xfer(i3c, xfer);
}

/**
  \fn           void i3c_slave_tx(I3C_Type *i3c,
                                  I3C_XFER *xfer,
                                  uint16_t  len)
  \brief        send slave transmit command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    len     : Transmit length
  \return       none
*/
void i3c_slave_tx(I3C_Type *i3c,
                  I3C_XFER *xfer,
                  uint16_t  len)
{
  /* As per mipi_i3c_databook Section 2.7.13
   * no command required for transmit,
   * only data length is required
   */
  xfer->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(len)      |
                 COMMAND_SLV_PORT_TID(I3C_SLV_TX_TID);

  xfer->cmd_lo = NULL;

  /* Add commands to i3c Command Queue */
  i3c_enqueue_xfer(i3c, xfer);
}

/**
  \fn           void i3c_slave_rx(I3C_Type *i3c,
                                  I3C_XFER *xfer)
  \brief        send slave receive command to i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \return       none
*/
void i3c_slave_rx(I3C_Type *i3c,
                  I3C_XFER *xfer)
{
  /* As per mipi_i3c_user Section 7
   * no command is required for slave receive
   */
  xfer->cmd_hi = NULL;
  xfer->cmd_lo = NULL;

  /* Add commands to i3c Command Queue */
  i3c_enqueue_xfer(i3c, xfer);
}

/**
  \fn           void i3c_ccc_set(I3C_Type *i3c,
                                 I3C_XFER *xfer,
                                 uint32_t  index,
                                 uint8_t   cmd_id,
                                 uint16_t  cmd_len)
  \brief        send CCC (Common Command Codes) SET command to
                 i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    cmd_id  : Command-ID \ref Driver_I3C.h
  \param[in]    cmd_len : Command length
  \return       none
*/
void i3c_ccc_set(I3C_Type *i3c,
                 I3C_XFER *xfer,
                 uint32_t  index,
                 uint8_t   cmd_id,
                 uint16_t  cmd_len)
{
  xfer->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(cmd_len) |
                 COMMAND_PORT_TRANSFER_ARG;

  xfer->cmd_lo = COMMAND_PORT_CP                   |
                 COMMAND_PORT_TID(I3C_CCC_SET_TID) |
                 COMMAND_PORT_DEV_INDEX(index)     |
                 COMMAND_PORT_CMD(cmd_id)          |
                 COMMAND_PORT_TOC                  |
                 COMMAND_PORT_ROC;

  i3c_enqueue_xfer(i3c, xfer);
}

/**
  \fn           void i3c_ccc_get(I3C_Type *i3c,
                                 I3C_XFER *xfer,
                                 uint32_t  index,
                                 uint8_t   cmd_id,
                                 uint16_t  cmd_len)
  \brief        send CCC (Common Command Codes) GET command to
                 i3c bus.
  \param[in]    i3c     : Pointer to i3c register set structure
  \param[in]    xfer    : Pointer to i3c transfer structure
  \param[in]    index   : DAT Slave index
  \param[in]    cmd_id  : Command-ID \ref Driver_I3C.h
  \param[in]    cmd_len : Command length
  \return       none
*/
void i3c_ccc_get(I3C_Type *i3c,
                 I3C_XFER *xfer,
                 uint32_t  index,
                 uint8_t   cmd_id,
                 uint16_t  cmd_len)
{
  xfer->cmd_hi = COMMAND_PORT_ARG_DATA_LEN(cmd_len) |
                 COMMAND_PORT_TRANSFER_ARG;

  xfer->cmd_lo = COMMAND_PORT_READ_TRANSFER        |
                 COMMAND_PORT_CP                   |
                 COMMAND_PORT_TID(I3C_CCC_GET_TID) |
                 COMMAND_PORT_DEV_INDEX(index)     |
                 COMMAND_PORT_CMD(cmd_id)          |
                 COMMAND_PORT_TOC                  |
                 COMMAND_PORT_ROC;

  i3c_enqueue_xfer(i3c, xfer);
}

/**
  \fn           void i3c_clk_cfg(I3C_Type *i3c,
                                 uint32_t  core_clk)
  \brief        i3c clock configuration for i3c slave device
  \param[in]    i3c       : Pointer to i3c register set structure
  \param[in]    core_clk  : core clock
  \return       none
*/
void i3c_clk_cfg(I3C_Type *i3c,
                 uint32_t  core_clk)
{
  unsigned long core_rate, core_period;
  uint32_t scl_timing;
  uint8_t  hcnt, lcnt;

  core_rate = core_clk;

  /* Calculate core clk period */
  core_period = DIV_ROUND_UP(REF_CLK_RATE, core_rate);

  /* Calculate SCL push-pull High and Low count for
   *  I3C transfers targeted to I3C devices.*/
  hcnt = DIV_ROUND_UP(I3C_BUS_THIGH_MAX_NS, core_period) - 1;
  if (hcnt < SCL_I3C_TIMING_CNT_MIN)
    hcnt = SCL_I3C_TIMING_CNT_MIN;

  lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_TYP_I3C_SCL_RATE) - hcnt;
  if (lcnt < SCL_I3C_TIMING_CNT_MIN)
    lcnt = SCL_I3C_TIMING_CNT_MIN;

  scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | SCL_I3C_TIMING_LCNT(lcnt);
  i3c->I3C_SCL_I3C_PP_TIMING = scl_timing;

  /* set the Bus free time for initiating the transfer in master mode.*/
  if (!(i3c->I3C_DEVICE_CTRL & DEV_CTRL_I2C_SLAVE_PRESENT))
    i3c->I3C_BUS_FREE_AVAIL_TIMING = BUS_I3C_MST_FREE(lcnt);

  /* SCL open-drain High and Low count (I3C) for
   *  I3C transfers targeted to I3C devices*/
  lcnt = DIV_ROUND_UP(I3C_BUS_TLOW_OD_MIN_NS, core_period);
  scl_timing = SCL_I3C_TIMING_HCNT(hcnt) | SCL_I3C_TIMING_LCNT(lcnt);
  i3c->I3C_SCL_I3C_OD_TIMING = scl_timing;

  /* set SCL Extended Low Count Timing Register. */

  /* Calculate the minimum low count for SDR1 */
  lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR1_SCL_RATE) - hcnt;
  scl_timing = SCL_EXT_LCNT_1(lcnt);

  /* Calculate the minimum low count for SDR2 */
  lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR2_SCL_RATE) - hcnt;
  scl_timing |= SCL_EXT_LCNT_2(lcnt);

  /* Calculate the minimum low count for SDR3 */
  lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR3_SCL_RATE) - hcnt;
  scl_timing |= SCL_EXT_LCNT_3(lcnt);

  /* Calculate the minimum low count for SDR4 */
  lcnt = DIV_ROUND_UP(core_rate, I3C_BUS_SDR4_SCL_RATE) - hcnt;
  scl_timing |= SCL_EXT_LCNT_4(lcnt);

  i3c->I3C_SCL_EXT_LCNT_TIMING = scl_timing;
}

/**
  \fn           void i2c_clk_cfg(I3C_Type           *i3c,
                                 uint32_t            core_clk,
                                 I3C_I2C_SPEED_MODE  i2c_speed_mode)
  \brief        i3c clock configuration for legacy i2c slave device
  \param[in]    i3c             : Pointer to i3c register set structure
  \param[in]    core_clk        : core clock
  \param[in]    i2c_speed_mode  : i2c Speed mode
                 I3C_I2C_SPEED_MODE_FMP_1_MBPS  : Fast Mode Plus 1   MBPS
                 I3C_I2C_SPEED_MODE_FM_400_KBPS : Fast Mode      400 KBPS
                 I3C_I2C_SPEED_MODE_SS_100_KBPS : Standard Mode  100 KBPS
  \return        none
*/
void i2c_clk_cfg(I3C_Type           *i3c,
                 uint32_t            core_clk,
                 I3C_I2C_SPEED_MODE  i2c_speed_mode)
{
  unsigned long core_rate   = 0U;
  unsigned long core_period = 0U;
  uint16_t hcnt             = 0U;
  uint16_t lcnt             = 0U;
  uint32_t scl_timing       = 0U;

  core_rate = core_clk;

  /* Calculate core clk period */
  core_period = DIV_ROUND_UP(REF_CLK_RATE, core_rate);

  /* Speed Mode: Fast Mode Plus >1 MBPS (approximately 3.124 MBPS) */
  if(i2c_speed_mode == I3C_I2C_SPEED_MODE_FMP_1_MBPS)
  {
    /* Calculate the SCL clock high period and low period count for
     *  I2C Fast Mode Plus transfers. */
    lcnt = DIV_ROUND_UP(I3C_BUS_I2C_FMP_TLOW_MIN_NS, core_period);
    hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_PLUS_SCL_RATE) - lcnt;

    scl_timing = SCL_I2C_FMP_TIMING_HCNT(hcnt) |
                 SCL_I2C_FMP_TIMING_LCNT(lcnt);

    i3c->I3C_SCL_I2C_FMP_TIMING = scl_timing;
  }

  /* Speed Mode: Fast Mode 400 KBPS */
  if(i2c_speed_mode == I3C_I2C_SPEED_MODE_FM_400_KBPS)
  {
    /* Calculate the SCL clock high period and low period count for
     *  I2C Fast Mode transfers. */
    lcnt = DIV_ROUND_UP(I3C_BUS_I2C_FM_TLOW_MIN_NS, core_period);
    hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_FM_SCL_RATE) - lcnt;

    scl_timing = SCL_I2C_FM_TIMING_HCNT(hcnt) |
                 SCL_I2C_FM_TIMING_LCNT(lcnt);

    /* Set the high and low period count for FM mode */
    i3c->I3C_SCL_I2C_FM_TIMING = scl_timing;
  }

  /* Speed Mode: Standard Mode 100 KBPS */
  if(i2c_speed_mode == I3C_I2C_SPEED_MODE_SS_100_KBPS)
  {
    /* Calculate the SCL clock high period and low period count for
     *  I2C Fast Mode transfers. */
    lcnt = DIV_ROUND_UP(I3C_BUS_I2C_SS_TLOW_MIN_NS, core_period);
    hcnt = DIV_ROUND_UP(core_rate, I3C_BUS_I2C_SS_SCL_RATE) - lcnt;

    scl_timing = SCL_I2C_FM_TIMING_HCNT(hcnt) |
                 SCL_I2C_FM_TIMING_LCNT(lcnt);

    i3c->I3C_SCL_I2C_FM_TIMING = scl_timing;
  }

  /* set the Bus free time for initiating the transfer in master mode.*/
  i3c->I3C_BUS_FREE_AVAIL_TIMING = BUS_I3C_MST_FREE(lcnt);

  /* Set as legacy i2c device is present. */
  i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL |
                         DEV_CTRL_I2C_SLAVE_PRESENT;
}

/**
  \fn           void i3c_master_init(I3C_Type *i3c)
  \brief        Initialize i3c master.
                 This function will :
                  - Free all the position from
                     DAT(Device Address Table)
                  - Clear Command Queue and Data buffer Queue
                  - Enable interrupt for
                      Response Queue Ready and
                      Transfer error status
                  - Enable i3c controller
  \param[in]    i3c  : Pointer to i3c register
                        set structure
  \return       none
*/
void i3c_master_init(I3C_Type *i3c)
{
  uint32_t val;

  val = i3c->I3C_QUEUE_THLD_CTRL;

  val &= ~QUEUE_THLD_CTRL_RESP_BUF_MASK;
  i3c->I3C_QUEUE_THLD_CTRL = val;

  val = i3c->I3C_DATA_BUFFER_THLD_CTRL;
  val &= ~DATA_BUFFER_THLD_CTRL_RX_BUF;
  i3c->I3C_DATA_BUFFER_THLD_CTRL = val;

  i3c->I3C_INTR_STATUS    = INTR_ALL;
  i3c->I3C_INTR_STATUS_EN = INTR_MASTER_MASK;
  i3c->I3C_INTR_SIGNAL_EN = INTR_MASTER_MASK;

  /* set first non reserved address as the master's DA */
  i3c->I3C_DEVICE_ADDR = DEV_ADDR_DYNAMIC_ADDR_VALID |
                         DEV_ADDR_DYNAMIC(0x08);

  /* reject all ibis */
  i3c->I3C_IBI_SIR_REQ_REJECT = IBI_REQ_REJECT_ALL;
  i3c->I3C_IBI_MR_REQ_REJECT  = IBI_REQ_REJECT_ALL;

  i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL |
                         DEV_CTRL_HOT_JOIN_NACK;

  /* Enable i3c controller. */
  i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL | DEV_CTRL_ENABLE;
}

/**
  \fn           void i3c_slave_init(I3C_Type *i3c,
                                    uint8_t   slv_addr)
  \brief        Initialize i3c slave.
                 This function will :
                  - set slave address
                  - set control to adaptive i2c and i3c
                  - Enable interrupt for
                     Response Queue Ready and
                     Transfer error status
                     dynamic address assignment
                  - set secondary master as slave mode
                  - Enable i3c controller
  \param[in]    i3c       : Pointer to i3c register
                             set structure
  \param[in]    slv_addr  : Slave own Address
  \return       none
*/
void i3c_slave_init(I3C_Type *i3c, uint8_t slv_addr)
{
    int32_t val;

    /* As per mipi_i3c_user Section 5 */

    /* Slave adaptive i2c and i3c control DEVICE_CTRL */
    i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL | ADAPTIVE_I2C_I3C;

    /* DEVICE_ADDR to set static addr valid bit */
    i3c->I3C_DEVICE_ADDR = i3c->I3C_DEVICE_ADDR | STATIC_ADDR_VAILD;

    /*DEVICE_ADDR used to set the slave address*/
    i3c->I3C_DEVICE_ADDR = i3c->I3C_DEVICE_ADDR | slv_addr;

    /* Response buffer threshold */
    val = i3c->I3C_QUEUE_THLD_CTRL;
    val &= ~QUEUE_THLD_CTRL_RESP_BUF_MASK;
    i3c->I3C_QUEUE_THLD_CTRL = val;

    val = i3c->I3C_DATA_BUFFER_THLD_CTRL;
    val &= ~DATA_BUFFER_THLD_CTRL_RX_BUF;
    i3c->I3C_DATA_BUFFER_THLD_CTRL = val;

    /* Setting up interrupt bit */
    i3c->I3C_INTR_STATUS    = INTR_ALL;
    i3c->I3C_INTR_STATUS_EN = INTR_SLAVE_MASK;
    i3c->I3C_INTR_SIGNAL_EN = INTR_SLAVE_MASK;

    /* Slave bus control DEVICE_CTRL_EXTENDED */
    i3c->I3C_DEVICE_CTRL_EXTENDED = i3c->I3C_DEVICE_CTRL_EXTENDED |
                                    DEV_OPERATION_MODE_AS_SLV;

    /* controller to operate either as a master or a slave without
     * participating in the dynamic mode switching
     * As per mipi_i3c_user Section 5.1.4
     */
    i3c->I3C_DEVICE_CTRL_EXTENDED = i3c->I3C_DEVICE_CTRL_EXTENDED |
                                    REQMST_ACK_CTRL_AS_NACK;

    /* reject all MR request */
    i3c->I3C_IBI_MR_REQ_REJECT = i3c->I3C_IBI_MR_REQ_REJECT |
                                 MR_REQ_REJECT;

    /* reject all ibis */
    i3c->I3C_IBI_SIR_REQ_REJECT = IBI_REQ_REJECT_ALL;
    i3c->I3C_IBI_MR_REQ_REJECT  = IBI_REQ_REJECT_ALL;

    /* Enable i3c controller. */
    i3c->I3C_DEVICE_CTRL = i3c->I3C_DEVICE_CTRL | DEV_CTRL_ENABLE;
}

/**
  \fn           void i3c_irq_handler(I3C_Type *i3c,
                                     I3C_XFER *xfer)
  \brief        i3c interrupt service routine
  \param[in]    i3c  : Pointer to i3c register
                        set structure
  \param[in]    xfer : Pointer to i3c transfer
                        structure
  \return       none
*/
void i3c_irq_handler(I3C_Type *i3c,
                     I3C_XFER *xfer)
{
  uint32_t status, nresp, resp, tid;

  status = i3c->I3C_INTR_STATUS;

  /* Checking for dynamic address valid */
  if(status & DYN_ADDR_ASSGN_STS)
  {
      i3c->I3C_INTR_STATUS = i3c->I3C_INTR_STATUS |
                             DYN_ADDR_ASSGN_STS;
      xfer->status = I3C_XFER_STATUS_SLV_DYN_ADDR_ASSGN;
  }

  if (!(status & i3c->I3C_INTR_STATUS_EN))
  {
    /* there are no interrupts that we are interested in */
    i3c->I3C_INTR_STATUS = INTR_ALL;
    return;
  }

  /* we are only interested in a response interrupt,
   *  make sure we have a response */
  nresp = i3c->I3C_QUEUE_STATUS_LEVEL;
  nresp = QUEUE_STATUS_LEVEL_RESP(nresp);

  if (!nresp)
    return;

  resp = i3c->I3C_RESPONSE_QUEUE_PORT;

  xfer->rx_len = RESPONSE_PORT_DATA_LEN(resp);
  xfer->error  = RESPONSE_PORT_ERR_STATUS(resp);

  tid = RESPONSE_PORT_TID(resp);

  switch (tid)
  {
    case I3C_MST_TX_TID:
    case I3C_SLV_TX_TID:
    case I3C_CCC_SET_TID:
      if (xfer->error)
      {
        xfer->status = I3C_XFER_STATUS_ERROR   | \
                       I3C_XFER_STATUS_ERROR_TX;
      }
      else
      {
        if (tid == I3C_MST_TX_TID)
        {
          xfer->status = I3C_XFER_STATUS_MST_TX_DONE;
        }
        if (tid == I3C_SLV_TX_TID)
        {
          xfer->status = I3C_XFER_STATUS_SLV_TX_DONE;
        }
        if (tid == I3C_CCC_SET_TID)
        {
          xfer->status = I3C_XFER_STATUS_CCC_SET_DONE;
        }

        /* mark all success event also as Transfer DONE */
        xfer->status |= I3C_XFER_STATUS_DONE;
      }
      break;

    case I3C_MST_RX_TID:
    case I3C_SLV_RX_TID:
    case I3C_CCC_GET_TID:
      if (xfer->rx_len && !xfer->error)
      {
        if (xfer->rx_buf)
        {
          i3c_read_rx_fifo(i3c, xfer->rx_buf, xfer->rx_len);
        }

        if (tid == I3C_MST_RX_TID)
        {
          xfer->status = I3C_XFER_STATUS_MST_RX_DONE;
        }
        if (tid == I3C_SLV_RX_TID)
        {
          xfer->status = I3C_XFER_STATUS_SLV_RX_DONE;
        }
        if (tid == I3C_CCC_GET_TID)
        {
          xfer->status = I3C_XFER_STATUS_CCC_GET_DONE;
        }

        /* mark all success event also as Transfer DONE */
        xfer->status |= I3C_XFER_STATUS_DONE;
      }
      else if (xfer->error)
      {
        xfer->status = I3C_XFER_STATUS_ERROR    | \
                       I3C_XFER_STATUS_ERROR_RX;
      }
      break;

    case I3C_ADDR_ASSIGN_TID:
      if (xfer->error)
      {
        xfer->status = I3C_XFER_STATUS_ERROR            | \
                       I3C_XFER_STATUS_ERROR_ADDR_ASSIGN;
      }
      else
      {
        /* mark all success event also as Transfer DONE */
        xfer->status = I3C_XFER_STATUS_ADDR_ASSIGN_DONE |
                       I3C_XFER_STATUS_DONE;
      }
      break;
  }
}

/************************ (C) COPYRIGHT ALIF SEMICONDUCTOR *****END OF FILE****/
