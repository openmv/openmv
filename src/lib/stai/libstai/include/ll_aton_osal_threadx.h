/**
 ******************************************************************************
 * @file    ll_aton_osal_threadx.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to ThreadX as the underlying OS/platform for ATON
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

#ifndef __LL_ATON_OSAL_THREADX_H
#define __LL_ATON_OSAL_THREADX_H

#ifdef __cplusplus
extern "C"
{
#endif

  void aton_osal_threadx_init(void);
  void aton_osal_threadx_deinit(void);

  void aton_osal_threadx_wfe(void);
  void aton_osal_threadx_signal_event(void);

  void aton_osal_threadx_pb_lock(void);
  void aton_osal_threadx_pb_unlock(void);

  void aton_osal_threadx_lock(void);
  void aton_osal_threadx_unlock(void);

/* Please make sure that beyond macro value fits your application needs or define the correct value on the compiler
   command line!
   Note, this value should be equal to `1` only if there are parallel neural network (aka NN) execution
   threads in your application. */
#ifndef TX_HAS_PARALLEL_NETWORKS
#define TX_HAS_PARALLEL_NETWORKS 1 // there are parallel networks in the application
#warning Using default value `1` for macro `TX_HAS_PARALLEL_NETWORKS`.
#endif // TX_HAS_PARALLEL_NETWORKS

#ifdef __cplusplus
}
#endif

#endif //__LL_ATON_OSAL_THREADX_H
