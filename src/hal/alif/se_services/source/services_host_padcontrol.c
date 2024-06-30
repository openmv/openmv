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

#define SERVICES_PAD_CONTROL_PAYLOAD_LENGTH    3

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
 * @brief Pad control service API
 * @param services_handle
 * @param port_number
 * @param pin_number
 * @param config_data
 * @param error_code
 * @return
 */
uint32_t SERVICES_padcontrol(uint32_t services_handle,
                             uint8_t port_number,
                             uint8_t pin_number,
                             uint8_t config_data,
                             uint32_t * error_code)

{
  pad_control_svc_t * p_svc = (pad_control_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(pad_control_svc_t));

  p_svc->send_port_num = port_number;
  p_svc->send_pin_num = pin_number;
  p_svc->send_config_data = config_data;

  uint32_t ret =  SERVICES_send_request(services_handle,
                                        SERVICE_APPLICATION_PAD_CONTROL_ID,
                                        DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;
  return ret;
}
