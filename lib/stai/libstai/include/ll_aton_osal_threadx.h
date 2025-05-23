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

/*** Common macro for all RTOS OSAL ports ***/
/* Please make sure that beyond macro value fits your application needs or define the correct value on the compiler
   command line!
   Note, this value should be equal to `1` only if there are parallel neural network (aka NN) execution
   threads in your application. */
#ifndef APP_HAS_PARALLEL_NETWORKS
#define APP_HAS_PARALLEL_NETWORKS 1 // there are parallel networks in the application
#warning Using default value `1` for macro `APP_HAS_PARALLEL_NETWORKS`.
#endif // APP_HAS_PARALLEL_NETWORKS

/*** ThreadX includes ***/
#include "tx_api.h"
#include "tx_thread.h"

/*** `LL_ATON` includes */
#include "ll_aton_attributes.h"
#include "ll_aton_osal.h"
#include "ll_aton_platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* API functions*/
  void aton_osal_threadx_init(void);
  void aton_osal_threadx_deinit(void);

  void aton_osal_threadx_wfe(void);
  void aton_osal_threadx_signal_event(void);

  void aton_osal_threadx_dao_lock(void);
  void aton_osal_threadx_dao_unlock(void);

  void aton_osal_threadx_lock(void);
  void aton_osal_threadx_unlock(void);

#ifdef NDEBUG
#define SUCCESS_ASSERT(ret) LL_ATON_LIB_UNUSED(ret)
#else
#define SUCCESS_ASSERT(ret) assert((ret) == TX_SUCCESS)
#endif

/*** ThreadX dependent type macros ***/
#define _DaoMutexNoWaitersType_ TX_MUTEX
#define _DaoWaitQueueType_      TX_SEMAPHORE
#define _DaoWaitQueueValueType_ ULONG
#define _WfeSemaphoreType_      TX_SEMAPHORE
#define _CacheMutexType_        TX_MUTEX
#define _TaskHandleType_        TX_THREAD *
#define _PriorityType_          UINT
#define _ReturnType_            UINT

/*** ThreadX dependent values ***/
#define _OsTrue_     TX_SUCCESS
#define _NullHandle_ TX_NULL

  /*** Helper inline functions ***/
  /**
   * @brief Disable preemption
   * @note Unfortunately, ThreadX does not provide a direct API to enable/disable preemption
   *       and therefore we're forced to dive straight into the bowels of the kernel
   */
  static inline void _my_disable_preemption(void)
  {
    TX_INTERRUPT_SAVE_AREA

    /* Disable interrupts */
    TX_DISABLE
    /* increment global ThreadX preemption counter */
    _tx_thread_preempt_disable++;
    /* Restore interrupts */
    TX_RESTORE
  }

  /**
   * @brief Enable preemption
   * @note Unfortunately, ThreadX does not provide a direct API to enable/disable preemption
   *       and therefore we're forced to dive straight into the bowels of the kernel
   */
  static inline void _my_enable_preemption(void)
  {
    TX_INTERRUPT_SAVE_AREA
    assert(_tx_thread_preempt_disable > 0);

    /* Disable interrupts */
    TX_DISABLE
    /* decrement global ThreadX preemption counter */
    _tx_thread_preempt_disable--;
    /* Restore interrupts */
    TX_RESTORE

    /* Check for preemption */
    _tx_thread_system_preempt_check();
  }

  static inline void _my_threadx_deinit(TX_MUTEX *_dao_mutex, TX_SEMAPHORE *_dao_wait_queue, TX_SEMAPHORE *_wfe_sem,
                                        TX_MUTEX *_cache_mutex) // de-initialize ThreadX OSAL implementation
  {
    UINT ret;

    /* delete DAO mutex with no waiters */
    ret = tx_mutex_delete(_dao_mutex);
    SUCCESS_ASSERT(ret);

    /* delete DAO wait queue */
    ret = tx_semaphore_delete(_dao_wait_queue);
    SUCCESS_ASSERT(ret);

    /* delete WFE semaphore */
    ret = tx_semaphore_delete(_wfe_sem);
    SUCCESS_ASSERT(ret);

    /* delete cache mutex */
    ret = tx_mutex_delete(_cache_mutex);
    SUCCESS_ASSERT(ret);
  }

  static inline UINT _my_tx_thread_priority_get(TX_THREAD *thread_ptr)
  {
    UINT priority;
    UINT ret = tx_thread_info_get(thread_ptr, TX_NULL, TX_NULL, TX_NULL, &priority, TX_NULL, TX_NULL, TX_NULL, TX_NULL);
    SUCCESS_ASSERT(ret);
    return priority;
  }

  static inline void _my_tx_thread_priority_change(TX_THREAD *thread_ptr, UINT new_priority)
  {
    UINT ret;
    UINT old_priority;

    ret = tx_thread_priority_change(thread_ptr, new_priority,
                                    /* may not be `NULL` */ &old_priority); // changes also preemption threshold
    SUCCESS_ASSERT(ret);
  }

  static inline ULONG _my_tx_get_count(TX_SEMAPHORE *sem)
  {
    ULONG current_value;
    UINT ret = tx_semaphore_info_get(sem, TX_NULL, &current_value, TX_NULL, TX_NULL, TX_NULL);
    SUCCESS_ASSERT(ret);
    return current_value;
  }

  static inline UINT _my_tx_mutex_put_prioritized(TX_MUTEX *mutex)
  {
    UINT ret = tx_mutex_prioritize(mutex); // need to wake up highest priority thread
    SUCCESS_ASSERT(ret);

    ret = tx_mutex_put(mutex);

    return ret;
  }

  static inline UINT _my_tx_semaphore_put_prioritized(TX_SEMAPHORE *sem)
  {
    UINT ret = tx_semaphore_prioritize(sem); // need to wake up highest priority thread
    SUCCESS_ASSERT(ret);

    ret = tx_semaphore_put(sem);

    return ret;
  }

  static inline UINT _my_tx_semaphore_put_isr_non_prioritized(TX_SEMAPHORE *sem)
  {
    UINT ret;

    // Save the interrupt context
    TX_INTERRUPT_SAVE_AREA

    // Disable interrupts
    TX_DISABLE

    // Give the semaphore
    ret = tx_semaphore_put(sem);

    // Restore interrupts
    TX_RESTORE

    return ret;
  }

/*** ThreadX function macros ***/
#define _CreateDaoMutexNoWaiters_(_dao_obj, _dao_static_buffer) tx_mutex_create(&(_dao_obj), TX_NULL, TX_NO_INHERIT)
#define _CreateDaoWaitQueue_(_dao_obj, _dao_static_buffer)      tx_semaphore_create(&(_dao_obj), TX_NULL, 0)
#define _CreateWfeSemaphore_(_dao_obj, _dao_static_buffer)      tx_semaphore_create(&(_dao_obj), TX_NULL, 0)
#define _CreateCacheMutex_(_dao_obj, _dao_static_buffer)        tx_mutex_create(&(_dao_obj), TX_NULL, TX_INHERIT)

#define _FinalizeIRQHandling_() /* using default IRQ priority */

#define _DisablePreemption_() _my_disable_preemption()
#define _EnablePreemption_()  _my_enable_preemption()

#define _InitNonDao_()   /* nothing to do */
#define _DeInitNonDao_() _my_threadx_deinit(&_dao_mutex, &_dao_wait_queue, &_wfe_sem, &_cache_mutex)

#define _GetCurrentTaskHandle_()        tx_thread_identify()
#define _GetTaskPriority_(_task)        _my_tx_thread_priority_get(_task)
#define _SetTaskPriority_(_task, _prio) _my_tx_thread_priority_change(_task, _prio)
#define _YieldCurrentTask_()            tx_thread_relinquish()

#define _MakeDaoMutexNoWaitersAvailable_(_mutex)
#define _GetDaoMutexNoWaiters_(_mutex) tx_mutex_get(&(_mutex), TX_NO_WAIT) // no waiters
#define _ReleaseDaoMutexNoWaiters_(_mutex)                                                                             \
  tx_mutex_put(&(_mutex)) // no prioritization necessary as there are no waiters

#define _MakeDaoWaitQueueUnavailable_(_wq)
#define _GetDaoWaitQueueValue_(_wq) _my_tx_get_count(&(_wq))
#define _GetDaoWaitQueue_(_wq)      tx_semaphore_get(&(_wq), TX_WAIT_FOREVER)
#define _ReleaseDaoWaitQueue_(_wq)  _my_tx_semaphore_put_prioritized(&(_wq))

#define _MakeWfeSemaphoreUnavailable_(_sem)
#define _GetWfeSemaphore_(_sem) tx_semaphore_get(&(_sem), TX_WAIT_FOREVER)
#define _ReleaseWfeSemaphore_(_sem)                                                                                    \
  tx_semaphore_put(&(_sem)) // only the ATON IP owner may wait on this semaphore (i.e. no prioritization necessary)
#define _ReleaseWfeSemaphoreISR_(_sem, ...)                                                                            \
  _my_tx_semaphore_put_isr_non_prioritized(                                                                            \
      &(_sem)) // only the ATON IP owner may wait on this semaphore (i.e. no prioritization necessary)

#define _MakeCacheMutexAvailable_(_mutex)
#define _GetCacheMutex_(_mutex)     tx_mutex_get(&(_mutex), TX_WAIT_FOREVER)
#define _ReleaseCacheMutex_(_mutex) _my_tx_mutex_put_prioritized(&(_mutex))

#define _HeadIsrCode_(...)
#define _TailIsrCode_(...)

#define _FirstPrioHigherThanScnd_(_first, _second) ((_first) < (_second)) // smaller numbers mean higher priority

#ifdef __cplusplus
}
#endif

#endif //__LL_ATON_OSAL_THREADX_H
