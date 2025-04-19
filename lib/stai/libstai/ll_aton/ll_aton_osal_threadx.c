/**
 ******************************************************************************
 * @file    ll_aton_osal_threadx.c
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

#include "ll_aton_config.h"

#if (LL_ATON_OSAL == LL_ATON_OSAL_THREADX)

#include <assert.h>
#include <stdbool.h>

#include "ll_aton_attributes.h"
#include "ll_aton_osal_threadx.h"

#include "tx_api.h"

// Set beyond macro to `1` to enabled "hand-made priority inheritance"
// This is necessary in case there are non NN-execution threads in the
// application with a priority between the lowest priority and the highest priority
// (both included) of all NN-execution threads.
// Furthermore, in order to guarantee that this "hand-made priority inheritance"
// works correctly, the user/application developer may not change the priority of an
// NN-execution thread while it is performing an inference
// (or rather executing an epoch)!
//
// NOTE: the implementation of this "hand-made priority inheritance" uses internal
//       data structures of ThreadX and therefore may not be forward compatible!
#ifndef TX_HAND_MADE_PRIORITY_INHERITANCE
#define TX_HAND_MADE_PRIORITY_INHERITANCE 1
#endif // TX_HAND_MADE_PRIORITY_INHERITANCE

#ifdef NDEBUG
#define SUCCESS_ASSERT(ret) LL_ATON_LIB_UNUSED(ret)
#else
#define SUCCESS_ASSERT(ret) assert((ret) == TX_SUCCESS)
#endif

/*** Helper functions ***/
/**
 * @brief Disable preemption
 * @return Preemption threshold of current thread
 */
static inline UINT _disable_preemption(void)
{
  TX_THREAD *current = tx_thread_identify();
  assert(current != TX_NULL);

  UINT ret;
  UINT old_threshold;

  ret = tx_thread_preemption_change(current, 0, &old_threshold);
  SUCCESS_ASSERT(ret);

  return old_threshold;
}

/**
 * @brief Enable preemption
 * @param preemption_threshold Preemption threshold to assign to current thread
 */
static inline void _enable_preemption(UINT preemption_threshold)
{
  TX_THREAD *current = tx_thread_identify();
  assert(current != TX_NULL);

  UINT ret;
  UINT old_threshold;

  ret = tx_thread_preemption_change(current, preemption_threshold, /* may not be `NULL` */ &old_threshold);
  SUCCESS_ASSERT(ret);
}

/*** Static variables on the basis of this locking mechanism ***/
static TX_MUTEX _pb_mutex;          // main FIFO mutex of the priority based mechanism
static TX_SEMAPHORE _pb_wait_queue; // semaphore for threads waiting for `_pb_mutex`

/* Manual inheritance is sufficient as long as the priorities of the involved threads do not get changed by someone else
   while manual inheritance is "active".
   This can only be changed by really integrating the desired mutex behavior (i.e. not assigning a new mutex owner on
   unlock, but forcing an active new acquisition attempt). */
#if TX_HAND_MADE_PRIORITY_INHERITANCE
static TX_THREAD *_pb_owner = TX_NULL; // current owner of `_pb_mutex` (for "hand-made priority inheritance")
static UINT
    _pb_owner_orig_user_priority; // original "user" priority of `_pb_owner` (for "hand-made priority inheritance")
static UINT _pb_owner_orig_actual_priority; // original "actual" priority of `_pb_owner` (for "hand-made
                                            // priority inheritance")
static UINT _pb_owner_inherited_priority; // "inherited" (or "actual") priority of `_pb_owner` (for "hand-made priority
                                          // inheritance")
#endif                                    // TX_HAND_MADE_PRIORITY_INHERITANCE

static TX_SEMAPHORE _wfe_sem; // for WFE blocking and IRQ signalling
static TX_MUTEX _cache_mutex; // for non prioritized locking/unlocking (e.g. NPU cache lock, see functions
                              // `aton_osal_threadx_lock()` & `aton_osal_threadx_unlock()`)

/*** API functions ***/

/**
 * @brief Initialize ThreadX OSAL implementation
 */
void aton_osal_threadx_init()
{
  UINT ret;

  /* create main priority based mechanism mutex */
  ret = tx_mutex_create(&_pb_mutex, NULL,
                        TX_NO_INHERIT); // no thread will ever wait on this mutex so priority inheritance will never
                                        // take place (this is most likely the most important downside of this OSAL
                                        // implementation, see "hand-made priority inheritance")
  SUCCESS_ASSERT(ret);

  /* create priority based mechanism semaphore */
  ret = tx_semaphore_create(&_pb_wait_queue, NULL, 0);
  SUCCESS_ASSERT(ret);

  /* create WFE semaphore */
  ret = tx_semaphore_create(&_wfe_sem, NULL, 0);
  SUCCESS_ASSERT(ret);

  /* create cache mutex */
  ret = tx_mutex_create(&_cache_mutex, NULL, TX_INHERIT);
  SUCCESS_ASSERT(ret);
}

/**
 * @brief De-initialize ThreadX OSAL implementation
 */
void aton_osal_threadx_deinit()
{
  UINT ret;

  /* delete main priority based mechanism mutex */
  ret = tx_mutex_delete(&_pb_mutex);
  SUCCESS_ASSERT(ret);

  /* delete priority based mechanism semaphore */
  ret = tx_semaphore_delete(&_pb_wait_queue);
  SUCCESS_ASSERT(ret);

  /* delete WFE semaphore */
  ret = tx_semaphore_delete(&_wfe_sem);
  SUCCESS_ASSERT(ret);

  /* delete cache mutex */
  ret = tx_mutex_delete(&_cache_mutex);
  SUCCESS_ASSERT(ret);
}

/**
 * @brief Lock priority base mutex
 */
void aton_osal_threadx_pb_lock()
{
  UINT ret;
  UINT old_threshold;

  old_threshold = _disable_preemption();

#if TX_HAND_MADE_PRIORITY_INHERITANCE
  TX_THREAD *current = tx_thread_identify();
#endif // TX_HAND_MADE_PRIORITY_INHERITANCE

  do
  {
    ret = tx_mutex_get(&_pb_mutex, TX_NO_WAIT);

    if (ret == TX_SUCCESS)
    { // we have gotten the mutex
#if TX_HAND_MADE_PRIORITY_INHERITANCE
      /* save information about the owner */
      assert(_pb_owner == TX_NULL);
      _pb_owner = current;
      _pb_owner_orig_actual_priority = _pb_owner_inherited_priority = current->tx_thread_priority;
      _pb_owner_orig_user_priority = current->tx_thread_user_priority;
#endif // TX_HAND_MADE_PRIORITY_INHERITANCE

      /* enable preemption again */
      _enable_preemption(old_threshold);
      return; // we are done
    }
    else
    { // didn't get mutex
#if TX_HAND_MADE_PRIORITY_INHERITANCE
      assert(_pb_owner != TX_NULL);

      /* perform "hand-made priority inheritance" */
      if (current->tx_thread_priority <
          _pb_owner->tx_thread_user_priority) // current thread's "actual" priority is higher than the owner's "user"
                                              // priority (smaller numbers mean higher priority)
      {
        ret = tx_thread_priority_change(_pb_owner, current->tx_thread_priority,
                                        NULL); // boost owner's "user" priority with current's "actual" priority
        SUCCESS_ASSERT(ret);

        _pb_owner_inherited_priority = current->tx_thread_priority;
      }
#endif // TX_HAND_MADE_PRIORITY_INHERITANCE

      ret = tx_semaphore_get(&_pb_wait_queue, TX_WAIT_FOREVER); // block on semaphore
      SUCCESS_ASSERT(ret);
      // assumption: preemption is again disabled at this point
      // re-try to get the mutex
    }
  } while (1);
}

/**
 * @brief Unlock priority based mutex
 */
void aton_osal_threadx_pb_unlock()
{
  UINT ret;
  UINT old_threshold;
  bool relinquish = false;

#if TX_HAND_MADE_PRIORITY_INHERITANCE
  assert(tx_thread_identify() == _pb_owner); // only owner may call this function
#endif                                       // !TX_HAND_MADE_PRIORITY_INHERITANCE

  old_threshold = _disable_preemption();

  ULONG suspended_count;
  ret = tx_semaphore_info_get(&_pb_wait_queue, TX_NULL, TX_NULL, TX_NULL, &suspended_count, TX_NULL);
  SUCCESS_ASSERT(ret);

  if (suspended_count > 0)
  {
    ret = tx_semaphore_prioritize(&_pb_wait_queue); // need to wake up highest priority thread
    SUCCESS_ASSERT(ret);

    ret = tx_semaphore_put(&_pb_wait_queue);
    SUCCESS_ASSERT(ret);

    relinquish = true; // give other threads - above all the waked-up suspended thread - if of same priority a chance
                       // to run (and do not wait for my refreshed time-slice to expire)
  }

#if TX_HAND_MADE_PRIORITY_INHERITANCE
  /* reset priority to original one */
  if ((_pb_owner_inherited_priority != _pb_owner_orig_actual_priority) // we have changed the owner's "user" priority
      &&
      (_pb_owner_inherited_priority ==
       _pb_owner->tx_thread_user_priority)) // only we have modified the priority
                                            // (best possible but not sufficient check; would require integration in TX)
  {
    ret = tx_thread_priority_change(_pb_owner, _pb_owner_orig_user_priority, NULL);
    SUCCESS_ASSERT(ret);
  }

  /* reset owner info */
  _pb_owner = TX_NULL;
#endif // TX_HAND_MADE_PRIORITY_INHERITANCE

  /* release `_pb_mutex` */
  ret = tx_mutex_put(&_pb_mutex);
  SUCCESS_ASSERT(ret);

  _enable_preemption(old_threshold);

  if (relinquish)
  {
    tx_thread_relinquish();
  }
}

/**
 * @brief Wait for event
 */
void aton_osal_threadx_wfe()
{
  UINT ret;

  ret = tx_semaphore_get(&_wfe_sem, TX_WAIT_FOREVER);
  SUCCESS_ASSERT(ret);
}

/**
 * @brief Signal event
 */
void aton_osal_threadx_signal_event()
{
  UINT ret;

  ret = tx_semaphore_put(&_wfe_sem); // only one thread may be blocked on `_wfe_sem` at a time
  SUCCESS_ASSERT(ret);
}

/**
 * @brief Lock cache mutex
 */
void aton_osal_threadx_lock()
{
  UINT ret;

  ret = tx_mutex_get(&_cache_mutex, TX_WAIT_FOREVER);
  SUCCESS_ASSERT(ret);
}

/**
 * @brief Unlock cache mutex
 */
void aton_osal_threadx_unlock()
{
  UINT ret;

  ret = tx_mutex_prioritize(&_cache_mutex); // need to wake up highest priority thread
  SUCCESS_ASSERT(ret);

  ret = tx_mutex_put(&_cache_mutex);
  SUCCESS_ASSERT(ret);
}

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_THREADX)
