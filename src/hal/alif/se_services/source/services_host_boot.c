/**
 * @file services_host_boot.c
 *
 * @brief Boot services service source file
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
 * @brief Services Boot process TOC entry
 * @param services_handle
 * @param image_id
 * @param error_code
 * @return
 */
uint32_t SERVICES_boot_process_toc_entry(uint32_t services_handle, 
                                         const uint8_t * image_id,
                                         uint32_t *error_code)
{
  process_toc_entry_svc_t * p_svc = (process_toc_entry_svc_t *)
    SERVICES_prepare_packet_buffer(sizeof(process_toc_entry_svc_t));

  strncpy((char *)p_svc->send_entry_id, (const char *)image_id,
          IMAGE_NAME_LENGTH);

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_BOOT_PROCESS_TOC_ENTRY,
                                       DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @brief Services Boot a CPU core
 * @param services_handle
 * @param cpu_id
 * @param address
 * @param error_code
 * @return
 */
uint32_t SERVICES_boot_cpu(uint32_t services_handle, 
                           uint32_t cpu_id,
                           uint32_t address,
                           uint32_t *error_code)
{
  boot_cpu_svc_t * p_svc = (boot_cpu_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(boot_cpu_svc_t)); /* Srvc bucket */

  p_svc->send_cpu_id = cpu_id;
  p_svc->send_address = address;

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_BOOT_CPU,
                                       DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;

  return ret;
}

/**
 * @brief Set the VTOR of a core (applies only to M55)
 * @param services_handle
 * @param cpu_id
 * @param address   VTOR value
 * @param error_code
 * @return
 */
uint32_t SERVICES_boot_set_vtor(uint32_t services_handle,
                                uint32_t cpu_id,
                                uint32_t address,
                                uint32_t *error_code)
{
  boot_cpu_svc_t * p_svc = (boot_cpu_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(boot_cpu_svc_t));

  p_svc->send_cpu_id = cpu_id;
  p_svc->send_address = address;

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_BOOT_SET_VTOR, DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @brief Services Reset a CPU core
 * @param services_handle
 * @param cpu_id
 * @param error_code
 * @return
 */
uint32_t SERVICES_boot_reset_cpu(uint32_t services_handle,
                                 uint32_t cpu_id,
                                 uint32_t *error_code)
{
  control_cpu_svc_t * p_svc = (control_cpu_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(control_cpu_svc_t));

  p_svc->send_cpu_id = cpu_id;

  uint32_t ret = SERVICES_send_request(services_handle,
                               SERVICE_BOOT_RESET_CPU, DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @brief Services Release a CPU core
 * @param services_handle
 * @param cpu_id
 * @param error_code
 * @return
 */
uint32_t SERVICES_boot_release_cpu(uint32_t services_handle,
                                   uint32_t cpu_id,
                                   uint32_t *error_code)
{
  control_cpu_svc_t * p_svc = (control_cpu_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(control_cpu_svc_t));

  p_svc->send_cpu_id = cpu_id;

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_BOOT_RELEASE_CPU, DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;
  return ret;
}

/**
 * @brief Services Reset the SoC
 * @param services_handle
 * @return
 */
uint32_t SERVICES_boot_reset_soc(uint32_t services_handle)
{
  SERVICES_prepare_packet_buffer(sizeof(service_header_t));
  return SERVICES_send_request(services_handle, 
                               SERVICE_BOOT_RESET_SOC, DEFAULT_TIMEOUT);
}
