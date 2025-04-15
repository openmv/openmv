/**
 * @file  services_host_error.c
 *
 * @brief Services library Error handling
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
 *  @ingroup host_services
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "services_lib_api.h"
#include "services_lib_protocol.h"

#define MAX_ERROR_STRING_LENGTH     38

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
 * @fn    char *SERVICES_error_to_string(uint32_t error_code)
 * @brief Error code to string conversion
 * @param error_code
 * @return
 */
char *SERVICES_error_to_string(uint32_t error_code)
{
  static char err_string[MAX_ERROR_STRING_LENGTH] = { 0 }; /* return string */
  char *p_str = NULL; /* Error string */

  switch (error_code)
   {
       case SERVICES_REQ_SUCCESS:
         p_str = "SERVICES_REQ_SUCCESS          "; break;
       case SERVICES_REQ_NOT_ACKNOWLEDGE:
         p_str = "SERVICES_REQ_NOT_ACKNOWLEDGE  "; break;
       case SERVICES_REQ_TIMEOUT:
         p_str = "SERVICES_REQ_TIMEOUT          "; break;
       case SERVICES_RESP_UNKNOWN_COMMAND:
         p_str = "SERVICES_RESP_UNKNOWN_COMMAND "; break;
       case SERVICE_INVALID_PARAMETER:
           p_str = "SERVICES_INVALID_PARAMETER   "; break;
       default:
         p_str = ">>  Error UNKNOWN  <<"; break;
  }
  strncpy(err_string, p_str, sizeof(err_string));

  return (char *)&err_string[0];
}

/**
 * @fn    const char *SERVICES_version(void)
 * @brief SERVICES version
 * @return version string
 */
const char *SERVICES_version(void)
{
  return SE_SERVICES_VERSION_STRING;
}

