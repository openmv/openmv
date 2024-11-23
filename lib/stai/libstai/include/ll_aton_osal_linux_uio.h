/**
 ******************************************************************************
 * @file    ll_aton_osal_linux_uio.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to Linux+UIO as the underlying OS/platform for ATONN
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

#ifndef __LL_ATON_OSAL_LINUX_UIO_H
#define __LL_ATON_OSAL_LINUX_UIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

  /**
   * @brief Initialize the platform subsystem
   */
  void linux_init();

  /**
   * @brief Deinitialize the platform subsystem
   */
  void linux_uninit();

  /**
   * @brief Install userspace IRQ handler. Use NULL to unregister.
   */
  void linux_install_irq(int irq_aton_line_nr, void (*handler)(void));

  /**
   * @brief Enable/Disable IRQ handling
   */
  void linux_enable_irq(int irq_aton_line_nr, bool enable);

  /**
   * @brief Wait for event routine to be called after programming an epoch.
   */
  void linux_wfe();

#ifdef __cplusplus
}
#endif

#endif /* __LL_ATON_OSAL_LINUX_UIO_H */
