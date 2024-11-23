/**
 ******************************************************************************
 * @file    ll_aton_osal.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file for defining an abstraction of RTOS differences
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

#ifndef __LL_ATON_OSAL_H
#define __LL_ATON_OSAL_H

/*** Platform choice ***/
#include "ll_aton_config.h"

/*** Platform dependent definitions & includes ***/

/* Bare metal Cortex-M NCSIM simulator OSAL*/
#if (LL_ATON_OSAL == LL_ATON_OSAL_BARE_METAL)

/* Macros for (de-)initialization of OSAL layer */
#define LL_ATON_OSAL_INIT()
#define LL_ATON_OSAL_DEINIT()

/* Wait for / signal event from ATON runtime */
#define LL_ATON_OSAL_WFE() __WFE()
#define LL_ATON_OSAL_SIGNAL_EVENT()

#elif (LL_ATON_OSAL == LL_ATON_OSAL_LINUX_UIO)

#include "ll_aton_osal_linux_uio.h"

/* Macros for (de-)initialization of OSAL layer */
#define LL_ATON_OSAL_INIT()   linux_init()
#define LL_ATON_OSAL_DEINIT() linux_uninit()

/* Wait for / signal event from ATON runtime */
#define LL_ATON_OSAL_WFE()    linux_wfe()
#define LL_ATON_OSAL_SIGNAL_EVENT()

#define LL_ATON_OSAL_INSTALL_IRQ(irq_aton_line_nr, handler) linux_install_irq(irq_aton_line_nr, handler)
#define LL_ATON_OSAL_REMOVE_IRQ(irq_aton_line_nr)           linux_install_irq(irq_aton_line_nr, NULL)
#define LL_ATON_OSAL_ENABLE_IRQ(irq_aton_line_nr)           linux_enable_irq(irq_aton_line_nr, true)
#define LL_ATON_OSAL_DISABLE_IRQ(irq_aton_line_nr)          linux_enable_irq(irq_aton_line_nr, false)

/* Enter/exit a critical section for the ATON runtime thread/task */
#define LL_ATON_OSAL_ENTER_CS()                             /* NVIC_DisableIRQ(CDNN0_IRQn) */
#define LL_ATON_OSAL_EXIT_CS()                              /* NVIC_EnableIRQ(CDNN0_IRQn) */

/* Data synchronization barrier */
#ifdef __ARM_ARCH
#include <cmsis_compiler.h>
#define LL_ATON_OSAL_DSB() __DSB()
#else
#include <immintrin.h>
#define LL_ATON_OSAL_DSB() _mm_lfence()
#endif

#elif (LL_ATON_OSAL == LL_ATON_OSAL_LINUX_BW)
#include "ll_aton_osal_linux_bw.h"

/* Macros for (de-)initialization of OSAL layer */
#define LL_ATON_OSAL_INIT()         bw_init()
#define LL_ATON_OSAL_DEINIT()       bw_uninit()

/* Wait for / signal event from ATON runtime */
#define LL_ATON_OSAL_WFE()          bw_wfe()
#define LL_ATON_OSAL_SIGNAL_EVENT() bw_post_event()

#define LL_ATON_OSAL_INSTALL_IRQ(irq_aton_line_nr, handler) bw_install_irq(irq_aton_line_nr, handler)
#define LL_ATON_OSAL_REMOVE_IRQ(irq_aton_line_nr)           bw_uninstall_irq(irq_aton_line_nr)
#define LL_ATON_OSAL_ENABLE_IRQ(irq_aton_line_nr)
#define LL_ATON_OSAL_DISABLE_IRQ(irq_aton_line_nr)
#define LL_ATON_OSAL_ENTER_CS() bw_enter_cs()
#define LL_ATON_OSAL_EXIT_CS()  bw_exit_cs()

#include <immintrin.h>
#define LL_ATON_OSAL_DSB() _mm_lfence()

#elif (LL_ATON_OSAL == LL_ATON_OSAL_THREADX)
#include "ll_aton_osal_threadx.h"

#define LL_ATON_OSAL_INIT()         aton_osal_threadx_init()
#define LL_ATON_OSAL_DEINIT()       aton_osal_threadx_deinit()

/* Wait for / signal event from ATON runtime */
#define LL_ATON_OSAL_WFE()          aton_osal_threadx_wfe()
#define LL_ATON_OSAL_SIGNAL_EVENT() aton_osal_threadx_signal_event()

/* Locks */
#if APP_HAS_PARALLEL_NETWORKS
#ifdef LL_ATON_OSAL_TX_OLD
#define LL_ATON_OSAL_LOCK_ATON()   aton_osal_threadx_pb_lock()
#define LL_ATON_OSAL_UNLOCK_ATON() aton_osal_threadx_pb_unlock()
#else // LL_ATON_OSAL_TX_OLD
#define LL_ATON_OSAL_LOCK_ATON()   aton_osal_threadx_dao_lock()
#define LL_ATON_OSAL_UNLOCK_ATON() aton_osal_threadx_dao_unlock()
#endif // LL_ATON_OSAL_TX_OLD
#endif // APP_HAS_PARALLEL_NETWORKS

#define LL_ATON_OSAL_LOCK_NPU_CACHE()   aton_osal_threadx_lock()
#define LL_ATON_OSAL_UNLOCK_NPU_CACHE() aton_osal_threadx_unlock()

#elif (LL_ATON_OSAL == LL_ATON_OSAL_FREERTOS)
#include "ll_aton_osal_freertos.h"

#define LL_ATON_OSAL_INIT()         aton_osal_freertos_init()
#define LL_ATON_OSAL_DEINIT()       aton_osal_freertos_deinit()

/* Wait for / signal event from ATON runtime */
#define LL_ATON_OSAL_WFE()          aton_osal_freertos_wfe()
#define LL_ATON_OSAL_SIGNAL_EVENT() aton_osal_freertos_signal_event()

/* Locks */
#if APP_HAS_PARALLEL_NETWORKS
#define LL_ATON_OSAL_LOCK_ATON()   aton_osal_freertos_dao_lock()
#define LL_ATON_OSAL_UNLOCK_ATON() aton_osal_freertos_dao_unlock()
#endif // APP_HAS_PARALLEL_NETWORKS

#define LL_ATON_OSAL_LOCK_NPU_CACHE()   aton_osal_freertos_lock()
#define LL_ATON_OSAL_UNLOCK_NPU_CACHE() aton_osal_freertos_unlock()

#elif (LL_ATON_OSAL == LL_ATON_OSAL_ZEPHYR)
#include "ll_aton_osal_zephyr.h"

/* Init & De-Initialization */
#define LL_ATON_OSAL_INIT()                                 aton_osal_zephyr_init()
#define LL_ATON_OSAL_DEINIT()                               aton_osal_zephyr_deinit()

/* IRQ handling */
#define LL_ATON_OSAL_INSTALL_IRQ(irq_aton_line_nr, handler) aton_osal_zephyr_install_irq(irq_aton_line_nr, handler)
#define LL_ATON_OSAL_REMOVE_IRQ(irq_aton_line_nr)           aton_osal_zephyr_uninstall_irq(irq_aton_line_nr)
#define LL_ATON_OSAL_ENABLE_IRQ(irq_aton_line_nr)           aton_osal_zephyr_enable_irq(irq_aton_line_nr)
#define LL_ATON_OSAL_DISABLE_IRQ(irq_aton_line_nr)          aton_osal_zephyr_disable_irq(irq_aton_line_nr)
#define LL_ATON_OSAL_ENTER_CS()                             aton_osal_zephyr_enter_cs()
#define LL_ATON_OSAL_EXIT_CS()                              aton_osal_zephyr_exit_cs()

/* Wait for / signal event from ATON runtime */
#define LL_ATON_OSAL_WFE()                                  aton_osal_zephyr_wfe()
#define LL_ATON_OSAL_SIGNAL_EVENT()                         aton_osal_zephyr_signal_event()

/* Locks */
#if APP_HAS_PARALLEL_NETWORKS
#define LL_ATON_OSAL_LOCK_ATON()   aton_osal_zephyr_dao_lock()
#define LL_ATON_OSAL_UNLOCK_ATON() aton_osal_zephyr_dao_unlock()
#endif // APP_HAS_PARALLEL_NETWORKS

#define LL_ATON_OSAL_LOCK_NPU_CACHE()   aton_osal_zephyr_lock()
#define LL_ATON_OSAL_UNLOCK_NPU_CACHE() aton_osal_zephyr_unlock()

#elif (LL_ATON_OSAL == LL_ATON_OSAL_USER_IMPL)
#include "ll_aton_osal_user_impl.h" /* file to be provided together with an implemetation of the custom OSAL by the user */

#else
#error No target OSAL is specified. Please define macro `LL_ATON_OSAL`
#endif

/*** Default implementations ***/

/* Handling of ATON IRQ */
#ifndef LL_ATON_OSAL_INSTALL_IRQ
#define LL_ATON_OSAL_INSTALL_IRQ(irq_aton_line_nr, handler)
#endif // LL_ATON_OSAL_INSTALL_IRQ

#ifndef LL_ATON_OSAL_REMOVE_IRQ
#define LL_ATON_OSAL_REMOVE_IRQ(irq_aton_line_nr)
#endif // LL_ATON_OSAL_REMOVE_IRQ

#ifndef LL_ATON_OSAL_ENABLE_IRQ
#define LL_ATON_OSAL_ENABLE_IRQ(irq_aton_line_nr)                                                                      \
  do                                                                                                                   \
  {                                                                                                                    \
    switch (irq_aton_line_nr)                                                                                          \
    {                                                                                                                  \
    case 0:                                                                                                            \
      NVIC_EnableIRQ(CDNN0_IRQn);                                                                                      \
      break;                                                                                                           \
    case 1:                                                                                                            \
      NVIC_EnableIRQ(CDNN1_IRQn);                                                                                      \
      break;                                                                                                           \
    case 2:                                                                                                            \
      NVIC_EnableIRQ(CDNN2_IRQn);                                                                                      \
      break;                                                                                                           \
    case 3:                                                                                                            \
      NVIC_EnableIRQ(CDNN3_IRQn);                                                                                      \
      break;                                                                                                           \
    default:                                                                                                           \
      assert(0);                                                                                                       \
      break;                                                                                                           \
    }                                                                                                                  \
  } while (0)
#endif // LL_ATON_OSAL_ENABLE_IRQ

#ifndef LL_ATON_OSAL_DISABLE_IRQ
#define LL_ATON_OSAL_DISABLE_IRQ(irq_aton_line_nr)                                                                     \
  do                                                                                                                   \
  {                                                                                                                    \
    switch (irq_aton_line_nr)                                                                                          \
    {                                                                                                                  \
    case 0:                                                                                                            \
      NVIC_DisableIRQ(CDNN0_IRQn);                                                                                     \
      break;                                                                                                           \
    case 1:                                                                                                            \
      NVIC_DisableIRQ(CDNN1_IRQn);                                                                                     \
      break;                                                                                                           \
    case 2:                                                                                                            \
      NVIC_DisableIRQ(CDNN2_IRQn);                                                                                     \
      break;                                                                                                           \
    case 3:                                                                                                            \
      NVIC_DisableIRQ(CDNN3_IRQn);                                                                                     \
      break;                                                                                                           \
    default:                                                                                                           \
      assert(0);                                                                                                       \
      break;                                                                                                           \
    }                                                                                                                  \
  } while (0)
#endif // LL_ATON_OSAL_DISABLE_IRQ

#ifndef LL_ATON_OSAL_SET_PRIORITY
#define LL_ATON_OSAL_SET_PRIORITY(irq_aton_line_nr, prio)                                                              \
  do                                                                                                                   \
  {                                                                                                                    \
    switch (irq_aton_line_nr)                                                                                          \
    {                                                                                                                  \
    case 0:                                                                                                            \
      NVIC_SetPriority(CDNN0_IRQn, prio);                                                                              \
      break;                                                                                                           \
    case 1:                                                                                                            \
      NVIC_SetPriority(CDNN1_IRQn, prio);                                                                              \
      break;                                                                                                           \
    case 2:                                                                                                            \
      NVIC_SetPriority(CDNN2_IRQn, prio);                                                                              \
      break;                                                                                                           \
    case 3:                                                                                                            \
      NVIC_SetPriority(CDNN3_IRQn, prio);                                                                              \
      break;                                                                                                           \
    default:                                                                                                           \
      assert(0);                                                                                                       \
      break;                                                                                                           \
    }                                                                                                                  \
  } while (0)
#endif // LL_ATON_OSAL_SET_PRIORITY

/* Enter/exit a critical section for the ATON runtime thread/task */

// NOTE: here we need to block also execution of IRQ handler
#ifndef LL_ATON_OSAL_ENTER_CS
#define LL_ATON_OSAL_ENTER_CS() NVIC_DisableIRQ(ATON_STD_IRQn)
#endif // LL_ATON_OSAL_ENTER_CS

#ifndef LL_ATON_OSAL_EXIT_CS
#define LL_ATON_OSAL_EXIT_CS() NVIC_EnableIRQ(ATON_STD_IRQn)
#endif // LL_ATON_OSAL_EXIT_CS

/* Data synchronization barrier */
#ifndef LL_ATON_OSAL_DSB
#define LL_ATON_OSAL_DSB() __DSB()
#endif // LL_ATON_OSAL_DSB

/** Locking mechanisms for thread-safe ATON runtime **/

/* ATON IP */
#ifndef LL_ATON_OSAL_LOCK_ATON
#define LL_ATON_OSAL_LOCK_ATON()
#endif // !LL_ATON_OSAL_LOCK_ATON

#ifndef LL_ATON_OSAL_UNLOCK_ATON
#define LL_ATON_OSAL_UNLOCK_ATON()
#endif // !LL_ATON_OSAL_UNLOCK_ATON

/* NPU Cache Lock */
// NOTE: Needs to be implemented only if NPU cache maintenance operations cannot be performed in parallel
// NOTE: This lock may be taken while the ATON IP lock (aka `LL_ATON_OSAL_LOCK_ATON`) is held.
//       Therfore, either use two independent locking mechanisms for them or one re-entrant mechanism
#ifndef LL_ATON_OSAL_LOCK_NPU_CACHE
#define LL_ATON_OSAL_LOCK_NPU_CACHE()
#define LL_HAS_NO_ATON_OSAL_LOCK_NPU_CACHE
#endif // !LL_ATON_OSAL_LOCK_NPU_CACHE

#ifndef LL_ATON_OSAL_UNLOCK_NPU_CACHE
#define LL_ATON_OSAL_UNLOCK_NPU_CACHE()
#endif // !LL_ATON_OSAL_UNLOCK_NPU_CACHE

/* MCU Cache Lock */
// NOTE: Needs to be implemented only if MCU cache maintenance operations cannot be performed in parallel
// NOTE: This lock may be taken while the ATON IP lock (aka `LL_ATON_OSAL_LOCK_ATON`) is held.
//       Therfore, either use two independent locking mechanisms for them or one re-entrant mechanism
#ifndef LL_ATON_OSAL_LOCK_MCU_CACHE
#define LL_ATON_OSAL_LOCK_MCU_CACHE()
#define LL_HAS_NO_ATON_OSAL_LOCK_MCU_CACHE
#endif // !LL_ATON_OSAL_LOCK_MCU_CACHE

#ifndef LL_ATON_OSAL_UNLOCK_MCU_CACHE
#define LL_ATON_OSAL_UNLOCK_MCU_CACHE()
#endif // !LL_ATON_OSAL_UNLOCK_MCU_CACHE

#endif // __LL_ATON_OSAL_H
