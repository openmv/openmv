/**
 * @file services_host_update.c
 *
 * Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
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
#if defined(A32)
#include "a32_device.h"
#else
#include "system_utils.h"
#endif

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

#define UNUSED(x) (void)(x)

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
 * @fn      uint32_t SERVICES_update_stoc(uint32_t services_handle,
 *                                        uint32_t image_address,
 *                                        uint32_t image_size,
 *                                        uint32_t *error_code)
 *
 * @brief   Update the whole STOC
 * @param services_handle
 * @param image_address
 * @param image_size
 * @param error_code
 * @return
 */
uint32_t SERVICES_update_stoc(uint32_t services_handle,
                              uint32_t image_address,
                              uint32_t image_size,
                              uint32_t *error_code)
{
  update_stoc_svc_t * p_svc = (update_stoc_svc_t *)
      SERVICES_prepare_packet_buffer(sizeof(update_stoc_svc_t));

  p_svc->send_image_address = LocalToGlobal((void *)image_address);
  p_svc->send_image_size = image_size;
  uint32_t ret = SERVICES_send_request(services_handle, 
                                       SERVICE_UPDATE_STOC,
                                       DEFAULT_TIMEOUT);

  *error_code = p_svc->resp_error_code;
  return ret;
}
