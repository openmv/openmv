/**
 * @file services_host_handler.c
 * @brief Services handler file
 *
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * @ingroup host_services
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "services_lib_bare_metal.h"
#include "services_lib_protocol.h"
#if defined(A32)
#include "a32_device.h"
#else
#include "system_utils.h"
#endif

#define SERVICES_REQ_TIMEOUT_MS  0x20
#define SEND_MSG_ACK_TIMEOUT     1000000ul
#define UNUSED(x) (void)(x)

static services_lib_t s_services_host = {0};

static uint32_t s_pkt_buffer_address_global = 0x0;
static volatile bool s_service_req_ack_received = false;
static volatile bool s_new_msg_received = false;

/**
 * @brief Function to initialize the services library
 * @param init_params Initialization parameters
 */
void SERVICES_initialize(services_lib_t * init_params)
{
  s_services_host.packet_buffer_address = init_params->packet_buffer_address;
  s_pkt_buffer_address_global =
      LocalToGlobal((void *)s_services_host.packet_buffer_address);
  s_services_host.fn_send_mhu_message = init_params->fn_send_mhu_message;
  s_services_host.fn_wait_ms = init_params->fn_wait_ms;
  s_services_host.wait_timeout = init_params->wait_timeout,
  s_services_host.fn_print_msg = init_params->fn_print_msg;
}

/**
 * @brief  Function to synchronize with SE
 * @param  services_handle Services library handle
 * @return total number of retries
 *          if nonnegative, success
 *          if negative, failure
 */
int SERVICES_synchronize_with_se(uint32_t services_handle)
{
  const int MAX_RETRY = 100;
  uint32_t error_code = SERVICES_REQ_SUCCESS;
  int retry_count = 0;

  while (1)
  {
    retry_count ++;
    error_code = SERVICES_heartbeat(services_handle);
    if (error_code == SERVICES_REQ_SUCCESS)
    {
      break;
    }
    if (retry_count > MAX_RETRY)
    {
      return -retry_count;
    }
  }

  return retry_count;
}

/**
 * @brief prepare the packet buffer ()
 * @return
 */
uintptr_t SERVICES_prepare_packet_buffer(uint32_t size)
{
  memset((void *)s_services_host.packet_buffer_address, 0x0, size);
  return s_services_host.packet_buffer_address;
}

/**
 * @fn      uint32_t SERVICES_register_channel(uint32_t mhu_id,
 *                                             uint32_t channel_number)
 * @brief   Register a MHU communication channel with the Services library
 * @param   mhu_id
 * @param   channel_number
 * @return  Handle to be used in subsequent service calls
 */
uint32_t SERVICES_register_channel(uint32_t mhu_id, 
                                   uint32_t channel_number)
{
  return mhu_id * MHU_NUMBER_OF_CHANNELS_MAX + channel_number;
}

/**
 *
 * @param services_handle
 * @return
 */
static uint32_t services_get_mhu_id(uint32_t services_handle)
{
  return services_handle / MHU_NUMBER_OF_CHANNELS_MAX;
}

/**
 *
 * @param services_handle
 * @return
 */
static uint32_t services_get_channel_number(uint32_t services_handle)
{
  return services_handle % MHU_NUMBER_OF_CHANNELS_MAX;
}

/**
 * @brief Callback function for sent msg ACK
 * @fn    void SERVICES_send_msg_acked_callback(uint32_t sender_id,
 *                                              uint32_t channel_number)
 */
void SERVICES_send_msg_acked_callback(uint32_t sender_id,
                                      uint32_t channel_number)
{
  s_service_req_ack_received = true;
  UNUSED(sender_id);
  UNUSED(channel_number);
}

/**
 * @fn    void SERVICES_rx_msg_callback(uint32_t receiver_id,
 *                                      uint32_t channel_number,
 *                                      uint32_t service_data)
 * @brief Callback function for response message reception
 * @param receiver_id
 * @param channel_number
 * @param service_data
 */
void SERVICES_rx_msg_callback(uint32_t receiver_id, 
                              uint32_t channel_number, 
                              uint32_t service_data)
{
  UNUSED(receiver_id);
  UNUSED(channel_number);

  // Validate response by checking the message
  if (s_pkt_buffer_address_global == service_data)
  {
    s_services_host.fn_print_msg("[SERVICESLIB] rx_msg=0x%x \n", service_data);
    s_new_msg_received = true;
  }
  else
  {
    // @todo: handle invalid message
    s_services_host.fn_print_msg("[SERVICESLIB] Invalid msg=0x%x\n",
                                  service_data);
  }
}

/**
 * @fn    uint32_t SERVICES_send_msg(uint32_t services_handle, uint32_t service_data)
 * @brief Send the MHU message pointed by 'service_data'
 */
uint32_t SERVICES_send_msg(uint32_t services_handle, uint32_t services_data)
{
    s_service_req_ack_received = false;

    // Send a MHU message
    uint32_t global_address = services_data;
    s_services_host.fn_send_mhu_message(services_get_mhu_id(services_handle),
                                        services_get_channel_number(services_handle),
                                        global_address);

    // Wait for a MHU 'send' ACK
    uint32_t timeout = SEND_MSG_ACK_TIMEOUT;
    while (!s_service_req_ack_received)
    {
      timeout--;
      if (0 == timeout) // ACK not received
      {
        s_services_host.fn_print_msg("[ERROR][SERVICESLIB] SERVICES_REQ_NOT_ACKNOWLEDGE \n");
        return SERVICES_REQ_NOT_ACKNOWLEDGE;
      }
    }
    return SERVICES_REQ_SUCCESS;
}
/**
 * @brief Send services request to MHU
 * @param services_handle
 * @param service_id
 * @param service_timeout
 * @return
 */
uint32_t SERVICES_send_request(uint32_t services_handle,
                               uint16_t service_id,
                               uint32_t service_timeout)
{
  s_services_host.fn_print_msg("[SERVICESLIB] Send service request 0x%x\n",
                               service_id);

  s_new_msg_received = false;

  /**
   * Initialize the service request common header
   */
  service_header_t * p_header = (service_header_t *)s_services_host.packet_buffer_address;
  p_header->hdr_service_id = service_id;
  p_header->hdr_flags = 0;
  
  /**
   * Send a message to the SE
   */

  RTSS_CleanDCache_by_Addr((uint32_t *)s_services_host.packet_buffer_address,
                          SERVICES_MAX_PACKET_BUFFER_SIZE);

  uint32_t ret = SERVICES_send_msg(services_handle, s_pkt_buffer_address_global);
  if (ret != SERVICES_REQ_SUCCESS)
  {
      return ret;
  }

  // Wait for response from SE
  uint32_t timeout = service_timeout != DEFAULT_TIMEOUT
                     ? service_timeout :
                     s_services_host.wait_timeout;
  while (!s_new_msg_received)
  {
    timeout--;  
    if (0 == timeout) // No response from SE
    {
      return SERVICES_REQ_TIMEOUT;
    }
	
    //if (0 != s_services_host.fn_wait_ms(SERVICES_REQ_TIMEOUT_MS))
    //{
    //  break;
    //}
  }

  RTSS_InvalidateDCache_by_Addr((uint32_t*)s_services_host.packet_buffer_address,
                                SERVICES_MAX_PACKET_BUFFER_SIZE);

  return p_header->hdr_error_code;
}
