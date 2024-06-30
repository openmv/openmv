/**
 * @file services_host_extsys0.c
 *
 * @brief Extsys0 services service source file
 * @ingroup host_services
 * @par
 *
 * Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
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
#if defined(A32_LINUX)
#include "a32_linux.h"
#else
#include "system_utils.h"
#endif

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
 * @brief  EXTSys0 Boot
 * @param services_handle
 * @param boot_args
 * @param error_code
 * @return
 */
uint32_t SERVICES_Boot_Net_Proc(uint32_t services_handle,
                                net_proc_boot_args_t* boot_args,
                                uint32_t *error_code)
{
  net_proc_boot_svc_t *p_svc = (net_proc_boot_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(net_proc_boot_svc_t)); /* packet */

  /*
   * Assemble SERVICE packet
   */
  p_svc->send_nvds_src_addr = LocalToGlobal((void*)boot_args->nvds_src_addr);
  p_svc->send_nvds_dst_addr = boot_args->nvds_dst_addr;
  p_svc->send_nvds_copy_len = boot_args->nvds_copy_len;
  p_svc->send_trng_dst_addr = boot_args->trng_dst_addr;
  p_svc->send_trng_len      = boot_args->trng_len;
  p_svc->send_internal_clock_select = boot_args->es0_clock_select;

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_EXTSYS0_BOOT_SET_ARGS,
                                       DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;

  return ret;
}

/**
 * @brief Shutdown ExtSys0
 *
 * @param services_handle
 * @param error_code
 * @return
 */
uint32_t SERVICES_Shutdown_Net_Proc(uint32_t services_handle,
                                    uint32_t *error_code)
{
  net_proc_shutdown_svc_t *p_svc = (net_proc_shutdown_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(net_proc_shutdown_svc_t));

  uint32_t ret = SERVICES_send_request(services_handle,
                                       SERVICE_EXTSYS0_SHUTDOWN,
                                       DEFAULT_TIMEOUT);
  *error_code = p_svc->resp_error_code;

  return ret;
}
