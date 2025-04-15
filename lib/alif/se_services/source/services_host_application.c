/**
 * @file services_host_padcontrol.c
 *
 * @brief Pad control service source file
 * @ingroup host_services
 * @par
 *
 * Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "services_lib_api.h"
#include "services_lib_protocol.h"
#include "services_lib_ids.h"

/**
 * @ingroup services
 */

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

/**
 * @fn    uint32_t SERVICES_uart_write(uint32_t services_handle,
 *                                     const uint8_t *uart_data)
 * @brief write to UART
 * @param services_handle
 * @param uart_data
 * @return
 */
uint32_t SERVICES_uart_write(uint32_t services_handle,
                             size_t buffer_size,
                             const uint8_t *uart_data)
{
  uart_write_svc_t * p_svc = (uart_write_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(uart_write_svc_t));

  /**
   * check there is something to send
   */
  if (uart_data == NULL)
  {
    return SERVICES_REQ_SUCCESS;
  }

  /**
   * check we are not sending more than we can handle
   */
  if ( (buffer_size > sizeof(p_svc->send_string_contents)) ||
       (buffer_size == 0) )
  {
    return SERVICES_REQ_SUCCESS;
  }

  memcpy((void *)p_svc->send_string_contents, uart_data, buffer_size);
  p_svc->send_string_length = buffer_size;

  return SERVICES_send_request(services_handle,
                               SERVICE_APPLICATION_UART_WRITE_ID,
                               DEFAULT_TIMEOUT);
}

/**
 * @brief Get the Seccure Enclave Firmware version
 * @param services_handle
 * @param revision_data
 * @param error_code
 * @return
 */
uint32_t SERVICES_get_se_revision(uint32_t services_handle,
                                  uint8_t *revision_data,
                                  uint32_t *error_code)
{
  get_se_revision_t * p_svc = (get_se_revision_t *)
      SERVICES_prepare_packet_buffer(sizeof(get_se_revision_t));

  uint32_t ret = SERVICES_send_request(services_handle,
                                  SERVICE_APPLICATION_FIRMWARE_VERSION_ID,
                                  DEFAULT_TIMEOUT);

  memcpy((void *)revision_data, (const void *)p_svc->resp_se_revision,
      p_svc->resp_se_revision_length);
  *error_code = p_svc->resp_error_code;

  return ret;
}

/**
 * @brief Write an OSPI decryption key
 * @param services_handle
 * @param command
 * @param key
 * @param error_code
 * @return
 */
uint32_t SERVICES_application_ospi_write_key(uint32_t services_handle,
                                             uint32_t command,
                                             uint8_t *key,
                                             uint32_t *error_code)
{
  ospi_write_key_svc_t * p_svc = (ospi_write_key_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(ospi_write_key_svc_t));

  p_svc->send_command = command;
  memcpy((void *)p_svc->send_key, key, OSPI_KEY_LENGTH_BYTES);

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_APPLICATION_OSPI_WRITE_KEY_ID,
                                       DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;

  return ret;
}

