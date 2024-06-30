/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * @file  m55_services_main.c
 * @brief Source file for SERVICES test harness
 * @par
 */

/******************************************************************************
 *  I N C L U D E   F I L E S
 *****************************************************************************/
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include <time.h>
#include "mhu.h"
#include "mhu_driver.h"
#include "services_lib_interface.h"
#include "RTE_Components.h"

/*******************************************************************************
 *  M A C R O   D E F I N E S
 ******************************************************************************/

#if   defined(M55_HE)
#define CPU_STRING "M55_HE"
#include "M55_HE.h"
#elif defined(M55_HP)
#define CPU_STRING "M55_HP"
#include "M55_HP.h"
#else
#define CPU_STRING "<unknown>"
#endif

#if defined(RTE_Compiler_IO_STDOUT)
#include "retarget_stdout.h"
#endif  /* RTE_Compiler_IO_STDOUT */

/**
 * Test parameters
 * CONTINUOUS_RUN Will run forever if set to 1
 * LIMITED_RUN    Default is 1 so just once around
 */
#define CONTINUOUS_RUN         0
#define LIMITED_RUN            1

#define SE_MHU0_RECV_BASE      0x40040000
#define SE_MHU0_SEND_BASE      0x40050000

#define SE_MHU1_RECV_BASE      0x40060000
#define SE_MHU1_SEND_BASE      0x40070000

#define NUM_MHU                2

#define MHU_M55_SE_MHU0        0
#define MHU_M55_SE_MHU1        1

#define MAXIMUM_TIMEOUT        0x01000000

/* CMSIS >= 0.9.0 names change */
#if ALIF_CMSIS_BUILD_VERSION == 9
#define MHU_SESS_S_RX_IRQ  MHU_SESS_S_RX_IRQ_IRQn
#define MHU_SESS_S_TX_IRQ  MHU_SESS_S_TX_IRQ_IRQn
#define MHU_SESS_NS_RX_IRQ MHU_SESS_NS_RX_IRQ_IRQn
#define MHU_SESS_NS_TX_IRQ MHU_SESS_NS_TX_IRQ_IRQn
#endif

/*******************************************************************************
 *  T Y P E D E F S
 ******************************************************************************/

/*******************************************************************************
 *  G L O B A L   V A R I A B L E S
 ******************************************************************************/

uint32_t sender_base_address_list[NUM_MHU] =
{
  SE_MHU0_SEND_BASE,
  SE_MHU1_SEND_BASE,
};

uint32_t receiver_base_address_list[NUM_MHU] =
{
  SE_MHU0_RECV_BASE,
  SE_MHU1_RECV_BASE,
};

static mhu_driver_in_t s_mhu_driver_in;
static mhu_driver_out_t s_mhu_driver_out;

extern void SERVICES_test(uint32_t services_handle);

/*******************************************************************************
 *  C O D E
 ******************************************************************************/

/**
 * @brief MHU0 TX IRQ handler
 * @fn    void MHU_SESS_S_TX_IRQHandler(void)
 */
void MHU_SESS_S_TX_IRQHandler(void)
{
  s_mhu_driver_out.sender_irq_handler(MHU_M55_SE_MHU0);
}

/**
 * @brief MHU0 RX IRQ handler
 * @fn    void MHU_SESS_S_RX_IRQHandler(void)
 */
void MHU_SESS_S_RX_IRQHandler(void)
{
  s_mhu_driver_out.receiver_irq_handler(MHU_M55_SE_MHU0);
}

/**
 * @brief MHU1 TX IRQ handler
 * @fn    void MHU_SESS_NS_TX_IRQHandler(void)
 */
void MHU_SESS_NS_TX_IRQHandler(void)
{
  s_mhu_driver_out.sender_irq_handler(MHU_M55_SE_MHU1);
}

/**
 * @fn    void MHU_SESS_NS_RX_IRQHandler(void)
 * @brief MHU1 RX IRQ handler
 */
void MHU_SESS_NS_RX_IRQHandler(void)
{
  s_mhu_driver_out.receiver_irq_handler(MHU_M55_SE_MHU1);
}

/**
 * @brief Function to set up IRQs
 * @fn    static void setup_irq(int irq_num, uint32_t irq_handler)
 * @param irq_num
 * @param irq_handler
 */
static void setup_irq(int irq_num, uint32_t irq_handler)
{
  NVIC_DisableIRQ(irq_num);
  NVIC_ClearPendingIRQ(irq_num);
  // Place the interrupt handler directly in the vector table (for XIP case)
  //NVIC_SetVector(irq_num, irq_handler);
  (void)(irq_handler);
  NVIC_SetPriority(irq_num, 2);
  NVIC_EnableIRQ(irq_num);
}

/**
 * @brief MHU initialize function
 * @fn    static void mhu_initialize(void)
 */
static void mhu_initialize(void)
{
  s_mhu_driver_in.sender_base_address_list = sender_base_address_list;
  s_mhu_driver_in.receiver_base_address_list = receiver_base_address_list;
  s_mhu_driver_in.mhu_count = NUM_MHU;
  s_mhu_driver_in.send_msg_acked_callback = SERVICES_send_msg_acked_callback;
  s_mhu_driver_in.rx_msg_callback = SERVICES_rx_msg_callback;
  s_mhu_driver_in.debug_print = SERVICES_print;
  
  MHU_driver_initialize(&s_mhu_driver_in, &s_mhu_driver_out);

  setup_irq(MHU_SESS_S_RX_IRQ_IRQn, (uint32_t)&MHU_SESS_S_RX_IRQHandler);
  setup_irq(MHU_SESS_S_TX_IRQ_IRQn, (uint32_t)&MHU_SESS_S_TX_IRQHandler);
  setup_irq(MHU_SESS_NS_RX_IRQ_IRQn, (uint32_t)&MHU_SESS_NS_RX_IRQHandler);
  setup_irq(MHU_SESS_NS_TX_IRQ_IRQn, (uint32_t)&MHU_SESS_NS_TX_IRQHandler);
}

/**
 * @brief Main function
 * @return
 */
int main(void)
{
  #if defined(RTE_Compiler_IO_STDOUT_User)
   int32_t ret;
   ret = stdout_init();
   if(ret != 0)
   {
       while(1)
       {
       }
   }
   #endif
  SERVICES_print("[SE SERVICES] %s Test harness - STARTS\n", CPU_STRING);
  
  /**
   * Initialize the MHU and SERVICES Library
   */
  mhu_initialize();
  SERVICES_Setup(s_mhu_driver_out.send_message, MAXIMUM_TIMEOUT);
  
  //SERVICES_wait_ms(0x1000000);

  uint32_t services_handle = SERVICES_register_channel(MHU_M55_SE_MHU0, 0);

  /**
   * Call the SERVICES test harness
   */
#if CONTINUOUS_RUN == 1
  for (;;)
#else
  for (int test_run=0; test_run < LIMITED_RUN; test_run++)
#endif // #if CONTINUOUS_RUN == 1
  {
    SERVICES_test(services_handle);
  }

  SERVICES_print("[SE SERVICES] Test harness ENDS\n");

  while(1);

  return 0;
}

