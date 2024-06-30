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
 * @file mhu.h
 * @brief Header file for MHU module
 * @par
 */
#ifndef __MHU_H__
#define __MHU_H__

#include <stdint.h>
#include <stdbool.h>

#define MHU_NUMBER_OF_CHANNELS_MAX        124
#define MHU_DEBUG_PRINT_ENABLE            0

#if MHU_DEBUG_PRINT_ENABLE == 1
#define debug_print drv_debug_print_fn
#else
#define debug_print(...)
#endif

typedef int (*debug_print_function_t)(const char * fmt, ...);
extern debug_print_function_t drv_debug_print_fn;

typedef void (*MHU_send_msg_acked_callback_t)(uint32_t mhu_id, 
                                              uint32_t channel_number);
typedef void (*MHU_rx_msg_callback_t)(uint32_t mhu_id, 
                                      uint32_t channel_number,
                                      uint32_t message_data);

/*
 * Data and functions passed to the MHU driver by the client, to be called
 * by the MHU driver
 */
typedef struct 
{
  uint32_t * sender_base_address_list;                   // provided by the client
  uint32_t * receiver_base_address_list;                 // provided by the client
  uint32_t mhu_count;                                    // provided by the client
  MHU_send_msg_acked_callback_t send_msg_acked_callback; // SERVICES_send_msg_acked_callback
  MHU_rx_msg_callback_t         rx_msg_callback;         // SERVICES_rx_msg_callback
  debug_print_function_t        debug_print;             // provided by the client
} mhu_driver_in_t;

typedef enum
{
  MHU_SEND_OK,
  MHU_SEND_RECEIVER_BUSY,
  MHU_SEND_COMPLETION_FAILED,
  MHU_SEND_COMPLETED_OK,
  MHU_SEND_FAILED,
} mhu_send_status_t;

typedef mhu_send_status_t (*MHU_send_message_t)(uint32_t mhu_id,
                                                uint32_t channel_number,
                                                uint32_t message_data);

typedef void (*MHU_irq_handler_t)(uint32_t mhu_id);

/**
 * @struct mhu_driver_out_t
 * Functions implemented by the MHU driver and called by the client and
 * the Services library
 */
typedef struct 
{
  MHU_send_message_t send_message;          // Called by Services
  MHU_irq_handler_t sender_irq_handler;     // Called by client IRQ handler
  MHU_irq_handler_t receiver_irq_handler;   // Called by client IRQ handler
} mhu_driver_out_t;

void MHU_driver_initialize(mhu_driver_in_t *data_in, mhu_driver_out_t *data_out);
void MHU_initialize(void);

#endif /* __MHU_H__ */
