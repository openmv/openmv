/**
 ******************************************************************************
 * @file    ll_aton_osal_zephyr.h
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

#ifndef __LL_ATON_OSAL_ZEPHYR_H
#define __LL_ATON_OSAL_ZEPHYR_H

/*** Common macro for all RTOS OSAL ports ***/
/* Please make sure that beyond macro value fits your application needs or define the correct value on the compiler
   command line!
   Note, this value should be equal to `1` only if there are parallel neural network (aka NN) execution
   threads in your application. */
#ifndef APP_HAS_PARALLEL_NETWORKS
#define APP_HAS_PARALLEL_NETWORKS 1 // there are parallel networks in the application
#warning Using default value `1` for macro `APP_HAS_PARALLEL_NETWORKS`.
#endif // APP_HAS_PARALLEL_NETWORKS

/*** Zephyr includes ***/
#include <zephyr/irq.h>
#include <zephyr/kernel.h>

/*** `LL_ATON` includes */
#include "ll_aton_attributes.h"
#include "ll_aton_osal.h"
#include "ll_aton_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* API functions*/
  void aton_osal_zephyr_init(void);
  void aton_osal_zephyr_deinit(void);

  void aton_osal_zephyr_wfe(void);
  void aton_osal_zephyr_signal_event(void);

  void aton_osal_zephyr_dao_lock(void);
  void aton_osal_zephyr_dao_unlock(void);

  void aton_osal_zephyr_lock(void);
  void aton_osal_zephyr_unlock(void);

  void aton_osal_zephyr_install_irq(int irq_aton_line_nr, void (*handler)(void));
  void aton_osal_zephyr_uninstall_irq(int irq_aton_line_nr);

  void aton_osal_zephyr_enable_irq(int irq_aton_line_nr);
  void aton_osal_zephyr_disable_irq(int irq_aton_line_nr);

  void aton_osal_zephyr_enter_cs(void);
  void aton_osal_zephyr_exit_cs(void);

/*** Zephyr dependent type macros ***/
#define _DaoMutexNoWaitersType_ struct k_mutex
#define _DaoWaitQueueType_      struct k_sem
#define _DaoWaitQueueValueType_ unsigned int
#define _WfeSemaphoreType_      struct k_sem
#define _CacheMutexType_        struct k_mutex
#define _TaskHandleType_        k_tid_t
#define _PriorityType_          int
#define _ReturnType_            int

/*** Zephyr dependent values ***/
#define _OsTrue_     0
#define _NullHandle_ NULL

  /*** Helper inline functions ***/
  static inline int _my_zephyr_mutex_create(struct k_mutex *dao_addr)
  {
    k_mutex_init(dao_addr); // mutex afterwards is available
    return 0;
  }

  static inline int _my_zephyr_binary_semaphore_create(struct k_sem *dao_addr)
  {
    k_sem_init(dao_addr, 0, 1); // initial count is 0 (i.e. unavailable), max count is 1
    return 0;
  }

  static inline int _my_zephyr_counting_semaphore_create(struct k_sem *dao_addr)
  {
    k_sem_init(dao_addr, 0, UINT_MAX); // initial count is 0 (i.e. unavailable), max count is `UINT_MAX`
    return 0;
  }

  static inline int _my_sem_give(struct k_sem *sem)
  {
    k_sem_give(sem);
    return 0;
  }

/*** Zephyr function macros ***/
#define _CreateDaoMutexNoWaiters_(_dao_obj, _dao_static_buffer) _my_zephyr_mutex_create(&(_dao_obj))
#define _CreateDaoWaitQueue_(_dao_obj, _dao_static_buffer)      _my_zephyr_binary_semaphore_create(&(_dao_obj))
#define _CreateWfeSemaphore_(_dao_obj, _dao_static_buffer)      _my_zephyr_counting_semaphore_create(&(_dao_obj))
#define _CreateCacheMutex_(_dao_obj, _dao_static_buffer)        _my_zephyr_mutex_create(&(_dao_obj))

#define _FinalizeIRQHandling_() /* using default IRQ priority */

#define _DisablePreemption_() k_sched_lock()
#define _EnablePreemption_()  k_sched_unlock()

#define _InitNonDao_()   /* nothing to do */
#define _DeInitNonDao_() /* nothing to do */

#define _GetCurrentTaskHandle_()        k_current_get()
#define _GetTaskPriority_(_task)        k_thread_priority_get(_task)
#define _SetTaskPriority_(_task, _prio) k_thread_priority_set(_task, _prio)
#define _YieldCurrentTask_()            k_yield()

#define _MakeDaoMutexNoWaitersAvailable_(_mutex)
#define _GetDaoMutexNoWaiters_(_mutex)     k_mutex_lock(&(_mutex), K_NO_WAIT) // no waiters
#define _ReleaseDaoMutexNoWaiters_(_mutex) k_mutex_unlock(&(_mutex))

#define _MakeDaoWaitQueueUnavailable_(_wq)
#define _GetDaoWaitQueueValue_(_wq) k_sem_count_get(&(_wq))
#define _GetDaoWaitQueue_(_wq)      k_sem_take(&(_wq), K_FOREVER)
#define _ReleaseDaoWaitQueue_(_wq)  _my_sem_give(&(_wq))

#define _MakeWfeSemaphoreUnavailable_(_sem)
#define _GetWfeSemaphore_(_sem)             k_sem_take(&(_sem), K_FOREVER)
#define _ReleaseWfeSemaphore_(_sem)         _my_sem_give(&(_sem))
#define _ReleaseWfeSemaphoreISR_(_sem, ...) _my_sem_give(&(_sem))

#define _MakeCacheMutexAvailable_(_mutex)
#define _GetCacheMutex_(_mutex)     k_mutex_lock(&(_mutex), K_FOREVER)
#define _ReleaseCacheMutex_(_mutex) k_mutex_unlock(&(_mutex))

#define _HeadIsrCode_(...)
#define _TailIsrCode_(...)

#define _FirstPrioHigherThanScnd_(_first, _second)                                                                     \
  ((_first) < (_second)) // in Zephyr, lower numerical values represent higher priorities.

#ifdef __cplusplus
}
#endif

#endif //__LL_ATON_OSAL_ZEPHYR_H
