/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*******************************************************************************
 * @file     se_services_port.c
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     01-Aug-2023
 * @brief    Implemented the porting layer for the SE Service Library functions
 ******************************************************************************/

#include <RTE_Components.h>
#include CMSIS_device_header

#include "se_services_port.h"

#define SE_SERVICES_DEBUG             0           /* Enable debug logs            */

#define SE_SERVICES_MHU_COUNT         1           /* We are using only Secure MHU */
#define SE_SERVICES_S_MHU             0           /* Secure MHU index             */
#define SE_SERVICES_S_MHU_CHANNEL     0           /* Secure MHU channel number    */
#define SE_SERVICES_MAX_TIMEOUT       0x01000000  /* Max timeout waiting for resp */

/* Set the IRQ Priority for MHU TX and RX IRQs */
#define MHU_SESS_S_TX_IRQ_PRIORITY        2
#define MHU_SESS_S_RX_IRQ_PRIORITY        2

/* Secure SE Service handle which will be used across the application */
uint32_t        se_services_s_handle = 0xFFFFFFFF;

/* SE MHU driver structures */
static mhu_driver_in_t  se_services_mhu_driver_in;
static mhu_driver_out_t se_services_mhu_driver_out;

/* Buffer allocation for the SE data transfer */
static uint8_t
  se_services_packet_buffer[SERVICES_MAX_PACKET_BUFFER_SIZE] __attribute__ ((aligned (4)));

/* Array holding the MHU Secure TX and RX address */
static uint32_t se_services_sender_base_address_list[SE_SERVICES_MHU_COUNT] =
{
    MHU_SESS_S_TX_BASE,
};

static uint32_t se_services_receiver_base_address_list[SE_SERVICES_MHU_COUNT] =
{
    MHU_SESS_S_RX_BASE,
};

/**
  @fn           int32_t se_services_wait_ms(uint32_t wait_time_ms)
  @brief        SE service delay implementation
  @param[in]    wait_time_ms wait time in milliseconds
  @return       Always return 0
 */
static int32_t se_services_wait_ms(uint32_t wait_time_ms)
{
    for(uint32_t count = 0; count < wait_time_ms; count++)
        sys_busy_loop_us(1000);

    return 0;
}

/**
  @fn           int32_t se_services_print(const char * fmt, ...)
  @brief        Print SE service debug information
  @param[in]    fmt formatted string
  @return       0
 */
static int se_services_print(const char * fmt, ...)
{
#if SE_SERVICES_DEBUG
  va_list args;
  static char se_services_log_buffer[256];

  /*
   * @todo Handle long strings bigger than buffer size
   */
  va_start(args,fmt);
  vsprintf(se_services_log_buffer, fmt, args);
  va_end(args);

  printf("%s", se_services_log_buffer);
#else
  (void)fmt;
#endif // SE_SERVICES_DEBUG

  return 0;
}

/**
  @fn           void se_services_initialize(MHU_send_message_t send_message)
  @brief        SE service library initialization
  @param[in]    send_message MHU Send function
  @return       none
 */
static void se_services_initialize(MHU_send_message_t send_message)
{
    services_lib_t  services_init_params =
    {
         .packet_buffer_address = (uint32_t)se_services_packet_buffer,
         .fn_send_mhu_message   = send_message,
         .fn_wait_ms            = &se_services_wait_ms,
         .wait_timeout          = SE_SERVICES_MAX_TIMEOUT,
         .fn_print_msg          = &se_services_print,
    };

    SERVICES_initialize(&services_init_params);
}

/**
  @fn           void se_services_mhu_s_initialize(void)
  @brief        Initialize the MHU for SE Service
  @return       none
 */
static void se_services_mhu_s_initialize(void)
{
    se_services_mhu_driver_in.sender_base_address_list   = se_services_sender_base_address_list;
    se_services_mhu_driver_in.receiver_base_address_list = se_services_receiver_base_address_list;
    se_services_mhu_driver_in.mhu_count                  = SE_SERVICES_MHU_COUNT;
    se_services_mhu_driver_in.send_msg_acked_callback    = SERVICES_send_msg_acked_callback;
    se_services_mhu_driver_in.rx_msg_callback            = SERVICES_rx_msg_callback;

    MHU_driver_initialize(&se_services_mhu_driver_in, &se_services_mhu_driver_out);

    NVIC_DisableIRQ(MHU_SESS_S_RX_IRQ_IRQn);
    NVIC_ClearPendingIRQ(MHU_SESS_S_RX_IRQ_IRQn);
    NVIC_SetPriority(MHU_SESS_S_RX_IRQ_IRQn, MHU_SESS_S_RX_IRQ_PRIORITY);
    NVIC_EnableIRQ(MHU_SESS_S_RX_IRQ_IRQn);

    NVIC_DisableIRQ(MHU_SESS_S_TX_IRQ_IRQn);
    NVIC_ClearPendingIRQ(MHU_SESS_S_TX_IRQ_IRQn);
    NVIC_SetPriority(MHU_SESS_S_TX_IRQ_IRQn, MHU_SESS_S_TX_IRQ_PRIORITY);
    NVIC_EnableIRQ(MHU_SESS_S_TX_IRQ_IRQn);
}

/**
 * @brief SE Service MHU Secure TX IRQ handler
 * @fn    void MHU_SESS_S_TX_IRQHandler(void)
 */
void MHU_SESS_S_TX_IRQHandler(void)
{
    se_services_mhu_driver_out.sender_irq_handler(SE_SERVICES_S_MHU);
}

/**
 * @brief SE Service MHU Secure RX IRQ handler
 * @fn    void MHU_SESS_S_RX_IRQHandler(void)
 */
void MHU_SESS_S_RX_IRQHandler(void)
{
    se_services_mhu_driver_out.receiver_irq_handler(SE_SERVICES_S_MHU);
}

/**
  @fn           void se_services_port_init(void)
  @brief        Initialize the porting layer for SE Servive library
  @return       none
 */
void se_services_port_init(void)
{
    se_services_s_handle = SERVICES_register_channel(SE_SERVICES_S_MHU, SE_SERVICES_S_MHU_CHANNEL);

    /**
     * Initialize the MHU first
     */
    se_services_mhu_s_initialize();

    /**
     * Initialize the SE services library
     */
    se_services_initialize(se_services_mhu_driver_out.send_message);

    /* keep sending heartbeat services requests until one succeeds */
    SERVICES_synchronize_with_se(se_services_s_handle);
}
