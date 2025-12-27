/**
 ******************************************************************************
 * @file    ll_aton_osal_rtos_template.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Generic template implementation of DAO mutex mechanism
 *          (underlying OS/platform agnostic)
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

/*************************
The beyond "Deferred ATON Owner" (aka "DAO") mutex mechanism implements a mutex which on unlock does
not immediately assign a new owner to the mutex but forces waiters to try to re-acquire the mutex again.
This allows a higher priority thread to re-acquire the mutex again before a lower priority thread, which
is waiting.
It comes together with a "hand-made" priority inheritance mechanism, which requires that the
priority of a network/inference task is not changed during the execution of an epoch(-block)!
*************************/

/* Static variables on the basis of this locking mechanism */
static _DaoMutexNoWaitersType_ _dao_mutex; // handle for main FIFO mutex of the "deferred ATON owner" mechanism
static _DaoWaitQueueType_ _dao_wait_queue; // semaphore for threads waiting for `_dao_mutex`

#if APP_HAS_PARALLEL_NETWORKS

// current owner of ATON
static _TaskHandleType_ _current_aton_owner = _NullHandle_;
// original priority of current ATON owner (assuming that `_PriorityType_` is a regular integer type)
static _PriorityType_ _current_aton_owner_orig_priority = 0;
// number of threads waiting on `_dao_wait_queue`
volatile static uint32_t _nr_dao_waiters = 0;

#endif // APP_HAS_PARALLEL_NETWORKS

static _WfeSemaphoreType_ _wfe_sem;   // for WFE blocking and IRQ signalling
static _CacheMutexType_ _cache_mutex; // for non deferred locking/unlocking (e.g. NPU/MCU cache operation protection)

/*** API functions ***/

/**
 * @brief Initialize RTOS OSAL implementation
 */
void LL_ATON_OSAL_INIT()
{
  _InitNonDao_(); // initialize non DAO part

  /* create main "deferred ATON owner" mechanism mutex */
  _ReturnType_ ret = _CreateDaoMutexNoWaiters_(
      _dao_mutex,
      _dao_mutex_buffer); // no thread will ever wait on this "conceptional" mutex
                          // (so priority inheritance - as would come with a "real" mutex - is not needed)
  LL_ATON_ASSERT(ret == _OsTrue_);
  _MakeDaoMutexNoWaitersAvailable_(_dao_mutex); // make it available

  /* create "deferred ATON owner" mechanism semaphore */
  ret = _CreateDaoWaitQueue_(_dao_wait_queue, _dao_wait_queue_buffer);
  LL_ATON_ASSERT(ret == _OsTrue_);
  _MakeDaoWaitQueueUnavailable_(_dao_wait_queue); // make it un-available

  /* create WFE semaphore */
  ret = _CreateWfeSemaphore_(_wfe_sem, _wfe_sem_buffer);
  LL_ATON_ASSERT(ret == _OsTrue_);
  _MakeWfeSemaphoreUnavailable_(_wfe_sem); // make it un-available

  /* create cache mutex */
  ret = _CreateCacheMutex_(_cache_mutex, _cache_mutex_buffer);
  LL_ATON_ASSERT(ret == _OsTrue_);
  _MakeCacheMutexAvailable_(_cache_mutex); // make it available

  /* Finalize IRQ handling (e.g. priority) */
  _FinalizeIRQHandling_();
}

/**
 * @brief De-initialize RTOS OSAL implementation
 */
void LL_ATON_OSAL_DEINIT()
{
  _DeInitNonDao_(); // de-initialize non DAO part
}

#if APP_HAS_PARALLEL_NETWORKS
/**
 * @brief Lock priority base mutex
 */
void LL_ATON_OSAL_LOCK_ATON()
{
  _ReturnType_ ret;

  // disable preemption
  _DisablePreemption_();

  // Get current thread
  _TaskHandleType_ current_thread = _GetCurrentTaskHandle_();
  LL_ATON_ASSERT(current_thread != _NullHandle_); // there should be a current thread

  do
  {
    ret = _GetDaoMutexNoWaiters_(_dao_mutex); // never block

    _PriorityType_ current_thread_priority = _GetTaskPriority_(
        current_thread); // get current thread's priority (might have change while waiting on `_dao_wait_queue`)

    if (ret == _OsTrue_)
    { // we have gotten the mutex
      // perform current owner housekeeping
      LL_ATON_ASSERT(_current_aton_owner == _NullHandle_); // there should be no current owner
      _current_aton_owner = current_thread;
      _current_aton_owner_orig_priority = current_thread_priority;

      // enable preemption
      _EnablePreemption_();

      return; // we are done
    }
    else
    { // didn't get mutex
      // increase current owner's priority if necessary
      LL_ATON_ASSERT(_current_aton_owner != _NullHandle_); // there should be a current owner

      // check for necessity for priority inheritance
      if (_FirstPrioHigherThanScnd_(current_thread_priority, _GetTaskPriority_(_current_aton_owner)))
      {
        // increase owner's priority
        LL_ATON_ASSERT(_current_aton_owner != current_thread);
        _SetTaskPriority_(_current_aton_owner, current_thread_priority);
      }

      // increment waiters counter
      _nr_dao_waiters++;

      // enable preemption
      _EnablePreemption_();

      /* wait on wait queue semaphore */
      ret = _GetDaoWaitQueue_(_dao_wait_queue); // block on wait queue
      LL_ATON_ASSERT(ret == _OsTrue_);
      LL_ATON_LIB_UNUSED(ret);

      // disable preemption
      _DisablePreemption_();

      // decrement waiters counter
      LL_ATON_ASSERT(_nr_dao_waiters > 0);
      _nr_dao_waiters--;

      // re-try to get the mutex
    }
  } while (1);
}

/**
 * @brief Unlock "deferred ATON owner" mutex
 */
void LL_ATON_OSAL_UNLOCK_ATON()
{
  _ReturnType_ ret;
  bool do_yield = false;

  // disable preemption
  _DisablePreemption_();

  /* release `_dao_mutex` */
  ret = _ReleaseDaoMutexNoWaiters_(_dao_mutex);
  LL_ATON_ASSERT(ret == _OsTrue_);
  LL_ATON_LIB_UNUSED(ret);

  // check if priority has changed while having been the ATON owner
  LL_ATON_ASSERT(_current_aton_owner == _GetCurrentTaskHandle_());
  if (_current_aton_owner_orig_priority != _GetTaskPriority_(_current_aton_owner))
  {
    _SetTaskPriority_(_current_aton_owner, _current_aton_owner_orig_priority); // restore original priority
  }
  _current_aton_owner = _NullHandle_;
  _current_aton_owner_orig_priority = 0;

  // LL_ATON_ASSERT that `_dao_wait_queue`'s value is 0 or 1
  _DaoWaitQueueValueType_ dao_wait_queue_val = _GetDaoWaitQueueValue_(_dao_wait_queue);
  LL_ATON_ASSERT(dao_wait_queue_val <= 1); // assuming that `_DaoWaitQueueValueType_` is a regular integer type

  // in case of waiters release `_dao_wait_queue` semaphore (if not already available)
  if (_nr_dao_waiters > 0)
  {
    // force a yield after enabling preemption
    do_yield = true;

    // increment semaphore
    if (dao_wait_queue_val == 0)
    {
      ret = _ReleaseDaoWaitQueue_(_dao_wait_queue);
      LL_ATON_ASSERT(ret == _OsTrue_);
      LL_ATON_LIB_UNUSED(ret);
    }
  }

  // enable preemption
  _EnablePreemption_();

  if (do_yield)
  {
    /* give other tasks of same priority a better chance to run
       and do not wait for the next timer tick or time-slice end */
    _YieldCurrentTask_();
  }
}
#endif // APP_HAS_PARALLEL_NETWORKS

#ifndef LL_HAS_NO_ATON_OSAL_LOCK_NPU_CACHE
/**
 * @brief Lock cache mutex
 */
void LL_ATON_OSAL_LOCK_NPU_CACHE()
{
  _ReturnType_ ret;

  ret = _GetCacheMutex_(_cache_mutex);
  LL_ATON_ASSERT(ret == _OsTrue_);
  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Unlock cache mutex
 */
void LL_ATON_OSAL_UNLOCK_NPU_CACHE()
{
  _ReturnType_ ret;

  ret = _ReleaseCacheMutex_(_cache_mutex);
  LL_ATON_ASSERT(ret == _OsTrue_);
  LL_ATON_LIB_UNUSED(ret);
}
#endif // LL_HAS_NO_ATON_OSAL_LOCK_NPU_CACHE

#ifndef LL_HAS_NO_ATON_OSAL_LOCK_MCU_CACHE
/**
 * @brief Lock cache mutex
 */
void LL_ATON_OSAL_LOCK_MCU_CACHE()
{
  _ReturnType_ ret;

  ret = _GetCacheMutex_(_cache_mutex);
  LL_ATON_ASSERT(ret == _OsTrue_);
  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Unlock cache mutex
 */
void LL_ATON_OSAL_UNLOCK_MCU_CACHE()
{
  _ReturnType_ ret;

  ret = _ReleaseCacheMutex_(_cache_mutex);
  LL_ATON_ASSERT(ret == _OsTrue_);
  LL_ATON_LIB_UNUSED(ret);
}
#endif // LL_HAS_NO_ATON_OSAL_LOCK_MCU_CACHE

/**
 * @brief Wait for event
 */
void LL_ATON_OSAL_WFE()
{
  _ReturnType_ ret;

  ret = _GetWfeSemaphore_(_wfe_sem);
  LL_ATON_ASSERT(ret == _OsTrue_);
  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Signal event
 * @note  Define macro `RTOS_HAS_NO_ISR_SIGNAL` if signalling is not performed from an interrupt handler.
 *        Calling this function within the same application from both ISR and normal contexts is currently not
 *        supported!
 */
void LL_ATON_OSAL_SIGNAL_EVENT()
{
  _ReturnType_ ret;

#ifdef RTOS_HAS_NO_ISR_SIGNAL
  ret = _ReleaseWfeSemaphore_(_wfe_sem); // assuming that this function gets NOT called from within an interrupt handler
  LL_ATON_ASSERT(ret == _OsTrue_);
#else  // !RTOS_HAS_NO_ISR_SIGNAL
  _ReturnType_ task_woken;

  _HeadIsrCode_();
  ret = _ReleaseWfeSemaphoreISR_(
      _wfe_sem, &task_woken); // assuming that this function gets called from within an interrupt handler
  LL_ATON_ASSERT(ret == _OsTrue_);
  _TailIsrCode_(task_woken);

  LL_ATON_LIB_UNUSED(task_woken);
#endif // !RTOS_HAS_NO_ISR_SIGNAL

  LL_ATON_LIB_UNUSED(ret);
}
