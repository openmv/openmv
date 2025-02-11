/**
 ******************************************************************************
 * @file    ll_aton_osal_linux_bw.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to Linux+Bittware as the underlying OS/platform for ATONN
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

#ifndef __LL_ATON_OSAL_LINUX_BW_H
#define __LL_ATON_OSAL_LINUX_BW_H

#ifdef __cplusplus
extern "C"
{
#endif

  int bw_init(void);
  int bw_uninit(void);
  int bw_install_irq(int irq_aton_line_nr, void (*handler)(void));
  int bw_uninstall_irq(int irq_aton_line_nr);
  int bw_wfe(void);
  int bw_enter_cs(void);
  int bw_exit_cs(void);
  int bw_post_event(void);

#ifdef __cplusplus
}
#endif

#endif //__LL_ATON_OSAL_LINUX_BW_H
