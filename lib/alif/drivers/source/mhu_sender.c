/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**
 * @file mhu_sender.c
 * @brief MHU sender driver
 * @ingroup host_services
 * @par
 */
#include "stddef.h"
#include "drivers_common.h"
#include "mhu_driver.h"

#define MHU_RECEIVER_TIMEOUT_MAX              1000000

static MHU_sender_callback s_nr2r_callback;
static MHU_sender_callback s_r2nr_callback;
static MHU_send_msg_acked_callback_t s_chcomb_callback;

static uint32_t s_sender_frame_base_address_list[MHU_NUMBER_MAX];
static uint32_t s_sender_frame_count = 0;

/**
 * @fn    static bool sender_id_valid(uint32_t sender_id)
 * @param sender_id
 * @return
 */
static bool sender_id_valid(uint32_t sender_id)
{
  return sender_id < s_sender_frame_count;
}

/**
 * @brief     This functions returns the sender frame base address
 * @param[in] sender_id    Sender frame id
 * @return    Sender frame base address
 */
static MHU_sender_frame_register_t * get_sender_frame(uint32_t sender_id)
{
  uint32_t sender_address = s_sender_frame_base_address_list[sender_id];
  return (MHU_sender_frame_register_t *)sender_address;
}

/**
 * @fn    static void sender_request_access(uint32_t sender_id)
 * @param sender_id
 */
static void sender_request_access(uint32_t sender_id)
{
  MHU_sender_frame_register_t * sender_reg_base = get_sender_frame(sender_id);
  uint32_t address = (uint32_t)&sender_reg_base->ACCESS_REQUEST;
  SET_REGISTER_BITS_U32(address, MHU_ACC_REQ);
}

/**
 * @fn static void sender_release_access(uint32_t sender_id)
 * @param sender_id Sender frame id
 */
static void sender_release_access(uint32_t sender_id)
{
  MHU_sender_frame_register_t * sender_reg_base = get_sender_frame(sender_id);
  uint32_t address = (uint32_t)&sender_reg_base->ACCESS_REQUEST;
  CLEAR_REGISTER_BITS_U32(address, MHU_ACC_REQ);
}

/**
 * @brief     Function sets MHU sender irq enable
 * @param     sender_reg_base
 * @param     mask
 * @param     enable
 * @return    None
 */
static void set_irq_enable(MHU_sender_frame_register_t * sender_reg_base,
                           uint32_t mask, bool enable)
{
  if (enable)
  {
    SET_REGISTER_BITS_U32(&sender_reg_base->INT_EN, mask);
  }
  else
  {
    CLEAR_REGISTER_BITS_U32(&sender_reg_base->INT_EN, mask);
  }
}

/**
 * @brief     Function clear MHU sender irq status
 * @param sender_reg_base
 * @param mask
 * @return    None
 */
static void clear_irq(MHU_sender_frame_register_t * sender_reg_base,
                                 uint32_t mask)
{
  WRITE_REGISTER_U32(&sender_reg_base->INT_CLR, mask);
}

/**
 * @brief     Function returns access ready status
 * @param     sender_id Sender frame ID
 * @return    ACCESS_READY status
 */
static bool sender_is_access_ready(uint32_t sender_id)
{
  MHU_sender_frame_register_t * sender_reg_base = get_sender_frame(sender_id);
  return (sender_reg_base->ACCESS_READY & MHU_ACC_RDY) == MHU_ACC_RDY;
}

/**
 * @brief     Interrupt handler for send message
 * @param     sender_id    MHU frame ID
 */
void MHU_send_message_irq_handler(uint32_t sender_id)
{
  if (!sender_id_valid(sender_id))
  {
    return;
  }

  MHU_sender_frame_register_t * sender_reg_base = get_sender_frame(sender_id);
  uint32_t status = sender_reg_base->INT_ST;
  uint32_t chcomb_status;

  if (status & MHU_NR2R)
  {
    // NOT READY to READY (ACC_RDY = 1)
    debug_print("IRQ: Sender id 0x%x : MHU_NR2R\n", sender_id);
    clear_irq(sender_reg_base, MHU_NR2R);
    if (NULL != s_nr2r_callback)
    {
      s_nr2r_callback();
    }
  }

  if (status & MHU_R2NR)
  {
    // READY to NOT READY (ACC_RDY = 0)
    debug_print("IRQ: Sender id 0x%x : MHU_R2NR\n", sender_id);
    clear_irq(sender_reg_base, MHU_R2NR);

    if (NULL != s_r2nr_callback)
    {
      s_r2nr_callback();
    }
  }

  if (status & MHU_CHCOMB)
  {
    // Send OS message for Channel combined interrupt
    const volatile uint32_t * status_reg_base = &sender_reg_base->CH_INT_ST0;
    chcomb_status = status_reg_base[MHU_CHANNEL_COMBINED_CH0_31];
    for(uint32_t channel = 0; channel < 2; channel++)
    {
      if (chcomb_status & (1 << channel))
      {
        // clear the channel status interrupt
        WRITE_REGISTER_U32(&sender_reg_base->CHANNEL[channel].CH_INT_CLR, MHU_CH_CLR);

        // To know when the transfer has been processed by the Receiver, the sender must wait for all bits in the
        // CH_ST register to read as 0b0.
        if (sender_reg_base->CHANNEL[channel].CH_ST == 0 && (sender_reg_base->CHANNEL[channel].CH_INT_EN & MHU_CH_CLR)) {
          // Disable channel interrupt
          CLEAR_REGISTER_BITS_U32(&sender_reg_base->CHANNEL[channel].CH_INT_EN, MHU_CH_CLR);

          sender_release_access(sender_id);

          // Invoke the user callback
          s_chcomb_callback(sender_id, channel);
        }
      }
    }
  }
}

/**
 * @brief
 * @note    This function is called in the services library. The compiler
 *          tries to optimize it out as it is not called within SERAM
 * @return
 */
mhu_send_status_t MHU_send_message(uint32_t sender_id,
                                   uint32_t channel_number,
                                   uint32_t message_data) __attribute__((used));

/**
 * @fn mhu_send_status_t MHU_send_message(uint32_t, uint32_t, uint32_t)
 * @brief     This functions sends a message out on a channel
 * @param sender_id
 * @param channel_number
 * @param message_data
 * @return
 */
mhu_send_status_t MHU_send_message(uint32_t sender_id,
                                   uint32_t channel_number,
                                   uint32_t message_data)
{
  if (!sender_id_valid(sender_id))
  {
    return MHU_SEND_FAILED;
  }

  uint32_t timeout;

  if (channel_number >= MHU_CHANNELS)
  {
    debug_print("[MHU] Incorrect channel number\n");
    return MHU_SEND_FAILED;
  }

  /*
   * @todo All the wait states below should be replaced by timer OS messages
   * sent from corresponding ISRs
   * 1. ACCESS_READY
   * 2. ACCESS not READY
   * 3. MESSAGE ACKed
   * 4. MESSAGE NACKed
   */

  uint32_t ch_num = channel_number;

  // Request access
  sender_request_access(sender_id);

  // Wait for access
  for (timeout = MHU_RECEIVER_TIMEOUT_MAX; timeout > 0; timeout--)
  {
    if (sender_is_access_ready(sender_id))
    {
      break;
    }
  }

  if (timeout == 0)
  {
    debug_print("[MHU] Receiver is busy\n");

    // Request access timeout
    sender_release_access(sender_id);
    return MHU_SEND_RECEIVER_BUSY;
  }
  else
  {
    debug_print("[MHU] Request granted\n");
  }

  // Receiver is ready for data transfer from sender
  MHU_sender_frame_register_t * sender_reg_base = get_sender_frame(sender_id);

  // Enable channel interrupt
  WRITE_REGISTER_U32(&sender_reg_base->CHANNEL[ch_num].CH_INT_CLR, MHU_CH_CLR);
  WRITE_REGISTER_U32(&sender_reg_base->CHANNEL[ch_num].CH_INT_EN, MHU_CH_CLR);

  // Write message to send channel
  sender_reg_base->CHANNEL[ch_num].CH_SET = message_data;

  return MHU_SEND_OK;
}

/**
 *
 * @param sender_frame_base_address_list
 * @param sender_frame_count
 * @param msg_acked_callback
 */
void MHU_sender_initialize(uint32_t sender_frame_base_address_list[],
                           uint32_t sender_frame_count,
                           MHU_send_msg_acked_callback_t msg_acked_callback)
{
  for (uint32_t i = 0; i < sender_frame_count; i++)
  {
    s_sender_frame_base_address_list[i] = sender_frame_base_address_list[i];
  }
  s_sender_frame_count = sender_frame_count;

  s_nr2r_callback = NULL;
  s_r2nr_callback = NULL;
  s_chcomb_callback = msg_acked_callback;

  uint32_t mask_disable = (MHU_CHCOMB | MHU_R2NR | MHU_NR2R);
  uint32_t mask_enable = (MHU_CHCOMB | MHU_R2NR | MHU_NR2R);

  for (uint32_t sender_id = 0; sender_id < sender_frame_count; sender_id++)
  {
    MHU_sender_frame_register_t * sender_reg_base = get_sender_frame(sender_id);

    clear_irq(sender_reg_base, mask_disable);
    set_irq_enable(sender_reg_base, mask_disable, false);
    set_irq_enable(sender_reg_base, mask_enable, true);
  }
}
