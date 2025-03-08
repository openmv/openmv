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
 * @file  services_lib_interface.c
 * @brief Public interface for Services library
 * @note  Unique for each platform
 * @par
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "services_lib_interface.h"
#include "services_lib_protocol.h"
#include "mhu.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/
#define PRINT_BUFFER_MAXIMUM  256 /* Max size of TTY print buffer */

#define TEST_PRINT_ENABLE           1   /* Enable printing from Test harness  */
#define PRINT_VIA_CONSOLE           0   /* Print via Debugger console         */
#define PRINT_VIA_SE_UART           1   /* Print via SE UART terminal         */

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/*******************************************************************************
 *  G L O B A L   V A R I A B L E S
 ******************************************************************************/
static uint8_t
  s_packet_buffer[SERVICES_MAX_PACKET_BUFFER_SIZE] __attribute__ ((aligned (4)));

debug_print_function_t drv_debug_print_fn;

/*******************************************************************************
 *  C O D E
 ******************************************************************************/

/**
 * @brief Public interface for SERVICES delay function
 * @param wait_time_ms
 * @return
 * @note  User must supply this implementation for their platform. This is a
 *        bare metal use case
 */
int32_t SERVICES_wait_ms(uint32_t wait_time_ms)
{
  /*
   * To be filled in by the user
   */
  for (volatile uint32_t i = 0; i < wait_time_ms; i++)
  {
     /* Do nothing, but please do not optimse me out either */
     __asm__ volatile("nop");
  }

  return 0;
}

/**
 * @brief Public interface for SERVICES stub printing function
 * @param fmt
 * @note  Add you favourite printing method here
 */
int SERVICES_print(const char * fmt, ...)
{
#if SERVICES_PRINT_ENABLE != 0
  va_list args;
  char buffer[PRINT_BUFFER_MAXIMUM];

  va_start(args,fmt);
  vsnprintf(buffer, PRINT_BUFFER_MAXIMUM, fmt, args);
  va_end(args);

  printf("%s", buffer);
#else
  (void)fmt;
#endif // #if SERVICES_PRINT_ENABLE != 0

  return 0;
}

/**
 * @fn    SERVICES_Setup(MHU_send_message_t send_message)
 * @brief Public interface to initialize the SERVICES library
 *
 * @param send_message
 * @param timeout
 */
void SERVICES_Setup(MHU_send_message_t send_message, uint32_t timeout)
{
  services_lib_t  services_init_params =
  {
    .packet_buffer_address = (uint32_t)s_packet_buffer,
    .fn_send_mhu_message   = send_message,
    .fn_wait_ms            = &SERVICES_wait_ms,
    .wait_timeout          = timeout,
    .fn_print_msg          = &SERVICES_print,
  };
  drv_debug_print_fn = &SERVICES_print;

  SERVICES_initialize(&services_init_params);
}

/**
 * @fn    void TEST_print(uint32_t services_handle, char * fmt, ...)
 * @param services_handle
 * @param fmt
 */
void TEST_print(uint32_t services_handle, char *fmt, ...)
{
#if TEST_PRINT_ENABLE != 0
  va_list args;
  static char buffer[PRINT_BUFFER_SIZE] = { 0 };
  size_t buffer_size;

  /*
   * @todo Handle long strings bigger than buffer size
   */
  va_start(args,fmt);
  buffer_size = vsnprintf(buffer, PRINT_BUFFER_SIZE, fmt, args);
  va_end(args);

  /**
   * Choice of Console printing or via the SE-UART
   */
#if PRINT_VIA_CONSOLE != 0
  if (buffer_size >= 0)
  {
    printf("%s", buffer);
  }
#endif
#if PRINT_VIA_SE_UART != 0
  SERVICES_uart_write(services_handle, strlen(buffer)+1, (uint8_t *)buffer);
#endif
  (void)buffer_size;
#endif // #if SERVICES_PRINT_ENABLE != 0
}

/**
 * @fn    void TEST_init(uint32_t services_handle)
 * @param services_handle
 */
void TEST_init(uint32_t services_handle)
{
  /* keep sending heartbeat services requests until one succeeds */
  int retry_count = SERVICES_synchronize_with_se(services_handle);
  TEST_print(services_handle, "SERVICES_synchronize_with_se() returned %d\n",
             retry_count);

  /* Disable tracing output for services */
  uint32_t service_error_code;
  SERVICES_system_set_services_debug(services_handle, false,
                                     &service_error_code);

  /* show services version */
  TEST_print(services_handle, "SERVICES version %s %s %s\n",
             SERVICES_version(), __DATE__, __TIME__);
}
