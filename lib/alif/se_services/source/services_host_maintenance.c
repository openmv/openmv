/**
 * @file services_host_maintenance.c
 *
 * @brief Maintenance Host Services source file
 *
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
 * @ingroup host_services
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
 * @brief   Heart beat service call
 * @return  Errorcode
 * @note    No payload required.
 */
uint32_t SERVICES_heartbeat(uint32_t services_handle)
{
  SERVICES_prepare_packet_buffer(sizeof(service_header_t));
  return SERVICES_send_request(services_handle, 
                               SERVICE_MAINTENANCE_HEARTBEAT_ID,
                               DEFAULT_TIMEOUT);
}
