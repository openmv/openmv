/**
 ******************************************************************************
 * @file    ll_aton_osal_zephyr.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to Zephyr as the underlying OS/platform for ATON
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "ll_aton_config.h"

#if (LL_ATON_OSAL == LL_ATON_OSAL_ZEPHYR)

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

/* Zephyr include */
#include "ll_aton_osal_zephyr.h"

/* IRQ OSAL handling */
static void (*aton_irq_handler)(void) = NULL;
static int aton_irq_number = -1;

static inline int _zephyr_get_irq_nr_from_aton_line_number(int aton_line_number)
{
  switch (aton_line_number)
  {
  case 0:
    return CDNN0_IRQn;
  case 1:
    return CDNN1_IRQn;
  case 2:
    return CDNN2_IRQn;
  case 3:
    return CDNN3_IRQn;
  default:
    assert(0);
    return -1;
  }
}

/**
 * @brief
 * @param arg
 */
static void aton_osal_zephyr_isr(const void *arg)
{
  LL_ATON_LIB_UNUSED(arg);
  assert(aton_irq_handler != NULL);
  assert(aton_irq_number >= 0);

  aton_irq_handler();
}

/**
 * @brief install IRQ handler
 * @param aton_line_number interrupt for which to install `handler`
 * @param handler interrupt handler
 */
void aton_osal_zephyr_install_irq(int aton_line_number, void (*handler)(void))
{
  assert(aton_irq_handler == NULL);
  assert(aton_irq_number < 0);

  aton_irq_handler = handler;
  aton_irq_number = _zephyr_get_irq_nr_from_aton_line_number(aton_line_number);

  irq_connect_dynamic(aton_irq_number, 0, aton_osal_zephyr_isr, NULL, 0);
}

/**
 * @brief
 * @param aton_line_number
 */
void aton_osal_zephyr_uninstall_irq(int aton_line_number)
{
  assert(aton_irq_handler != NULL);
  assert(aton_irq_number >= 0);

  irq_disable(_zephyr_get_irq_nr_from_aton_line_number(aton_line_number));
  // irq_disconnect_dynamic(_zephyr_get_irq_nr_from_aton_line_number(aton_line_number), 0, aton_osal_zephyr_isr, NULL,
  // 0); // betzw: seems not be available for STM32N6

  aton_irq_handler = NULL;
  aton_irq_number = -1;
}

/**
 * @brief
 * @param aton_line_number
 */
void aton_osal_zephyr_enable_irq(int aton_line_number)
{
  irq_enable(_zephyr_get_irq_nr_from_aton_line_number(aton_line_number));
}

/**
 * @brief
 * @param aton_line_number
 */
void aton_osal_zephyr_disable_irq(int aton_line_number)
{
  irq_disable(_zephyr_get_irq_nr_from_aton_line_number(aton_line_number));
}

/**
 * @brief
 * @param
 */
void aton_osal_zephyr_enter_cs(void)
{
  assert(aton_irq_handler != NULL);
  assert(aton_irq_number >= 0);

  irq_disable(aton_irq_number);
}

/**
 * @brief
 * @param
 */
void aton_osal_zephyr_exit_cs(void)
{
  assert(aton_irq_handler != NULL);
  assert(aton_irq_number >= 0);

  irq_enable(aton_irq_number);
}

/* Include common RTOS `.c` template */
#include "ll_aton_osal_rtos_template.c"

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_ZEPHYR)
