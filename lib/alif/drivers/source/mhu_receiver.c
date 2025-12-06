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
 * @file mhu_receiver.c
 * @brief MHU receiver driver
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <string.h>
#include "drivers_common.h"
#include "mhu_driver.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/*******************************************************************************
 *  G L O B A L   V A R I A B L E S
 ******************************************************************************/

/*******************************************************************************
 *  C O D E
 ******************************************************************************/

static uint32_t s_receiver_frame_base_address_list[MHU_NUMBER_MAX];
static uint32_t s_receiver_frame_count = 0;
static MHU_rx_msg_callback_t s_callback;

static bool receiver_id_valid(uint32_t receiver_id)
{
  return receiver_id < s_receiver_frame_count;
}

/**
 * @brief     This functions returns the receiver frame base address
 * @param[in] receiver_id    Receiver frame id
 * @return    Receiver frame base address
 */
static __inline MHU_receiver_frame_register_t
    *get_receiver_frame_base_address(uint32_t receiver_id)
{
  uint32_t receiver_address = s_receiver_frame_base_address_list[receiver_id];
  return (MHU_receiver_frame_register_t *)receiver_address;
}

/**
 * @brief     Function clears the channel mask
 * @param[in] channel_number    Channel number
 * @return    Channel group number
 */
static mhu_channel_combined_group_t sender_receiver_combined_ch_group(
    mhu_channel_number_t channel_number)
{
  mhu_channel_combined_group_t channel_group;

  if (channel_number < MHU_CHANNEL_32)
  {
    channel_group = MHU_CHANNEL_COMBINED_CH0_31;
  }
  else if (channel_number < MHU_CHANNEL_64)
  {
    channel_group = MHU_CHANNEL_COMBINED_CH32_63;
  }
  else if (channel_number < MHU_CHANNEL_96)
  {
    channel_group = MHU_CHANNEL_COMBINED_CH64_95;
  }
  else
  {
    channel_group = MHU_CHANNEL_COMBINED_CH96_123;
  }

  return channel_group;
}

/**
 * @fn        void MHU_receive_message_irq_handler(uint32_t receiver_id)
 * @brief     This functions handles received messages
 * @param     receiver_id  Receiver frame id
 */
void MHU_receive_message_irq_handler(uint32_t receiver_id)
{
  if (!receiver_id_valid(receiver_id))
  {
    return;
  }

  MHU_receiver_frame_register_t * receiver_reg_base =
                      get_receiver_frame_base_address(receiver_id);

  for (mhu_channel_number_t channel_number = 0;
       channel_number < MHU_CHANNELS; channel_number++)
  {
    mhu_channel_combined_group_t channel_group =
        sender_receiver_combined_ch_group(channel_number);

    const volatile uint32_t * status_reg_base = &receiver_reg_base->CH_INT_ST0;
    uint32_t channel_irq_status = status_reg_base[channel_group];

    // Message has been received on this channel
    if (channel_irq_status & (0x1 << channel_number))
    {
      // Get message data
      uint32_t message_data = receiver_reg_base->CHANNEL[channel_number].CH_ST;

      // Invoke the user callback
      s_callback(receiver_id, channel_number, message_data);
      debug_print("[MHU] rx isr: rx_id=0x%x ch_irq_status=0x%x ch=0x%x data=0x%x\n",
                  receiver_id, channel_irq_status, channel_number, message_data);

      // Clear channel data
      WRITE_REGISTER_U32(&receiver_reg_base->CHANNEL[channel_number].CH_CLR, 0xFFFFFFFF);
    }
  }
}

/**
 * @brief     Function sets the irq enable
 * @param[in] mhu_frame_id    MHU frame ID
 *            mask            Mask
 *            enable          1 = enable; 0 = disable
 * @return    None
 */
static void MHU_receiver_set_irq_enable(
             MHU_receiver_frame_register_t * receiver_reg_base,
             uint32_t mask,
             bool enable)
{
  if (enable)
  {
    SET_REGISTER_BITS_U32(&receiver_reg_base->INT_EN, mask);
  }
  else
  {
    CLEAR_REGISTER_BITS_U32(&receiver_reg_base->INT_EN, mask);
  }
}

#if 0
/**
 * @brief     Function clears the irq status
 * @param[in] mhu_frame_id    MHU frame ID
 *            mask            Mask
 * @return    None
 */
static void MHU_receiver_clear_irq(
    MHU_receiver_frame_register_t * receiver_reg_base, uint32_t mask)
{
  WRITE_REGISTER_U32(&receiver_reg_base->INT_CLR, mask);
}
#endif

/**
 * @brief     Initialize function for MHU receiver interrupts
 * @param[in] None
 * @return    None
 */
static void MHU_receiver_interrupt_initialize(uint32_t receiver_frame_count)
{
  for (uint32_t receiver_id = 0;
       receiver_id < receiver_frame_count; receiver_id++)
  {
    MHU_receiver_frame_register_t * receiver_reg_base =
        get_receiver_frame_base_address(receiver_id);

    MHU_receiver_set_irq_enable(receiver_reg_base, MHU_CHCOMB, true);
  }
}

/**
 *
 * @param receiver_frame_base_address_list
 * @param receiver_frame_count
 * @param callback
 */
void MHU_receiver_initialize(
    uint32_t receiver_frame_base_address_list[],
    uint32_t receiver_frame_count,
    MHU_rx_msg_callback_t callback)
{
  for (uint32_t i = 0; i < receiver_frame_count; i++)
  {
    s_receiver_frame_base_address_list[i] = receiver_frame_base_address_list[i];
  }
  s_receiver_frame_count = receiver_frame_count;
  s_callback = callback;

  MHU_receiver_interrupt_initialize(receiver_frame_count);
}
