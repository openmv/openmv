/**
 ******************************************************************************
 * @file    ll_aton_osal_freertos.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Interface to FreeRTOS as the underlying OS/platform for ATON
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

#if (LL_ATON_OSAL == LL_ATON_OSAL_FREERTOS)

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

#include "ll_aton_attributes.h"
#include "ll_aton_osal.h"
#include "ll_aton_osal_freertos.h"
#include "ll_aton_platform.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// NOTE: in FreeRTOS it is not possible to access internal data structures from outside
//       as these are defined with in `*.c` files (rather than include-able `*.h` files).
//       Furthermore, there is no way to obtain the base (original) priority of a task thru
//       its API functions.
//       Therefore it is not possible to implement a kind of "hand-made priority inheritance"
//       as done for e.g. ThreadX, with the drawback that this priority based mechanism lacks the
//       priority inheritance feature.

/*** Static variables on the basis of this locking mechanism ***/
static SemaphoreHandle_t _pb_mutex;        // handle for main FIFO mutex of the priority based mechanism
static StaticSemaphore_t _pb_mutex_buffer; // buffer for `_pb_mutex` which holds its state

static SemaphoreHandle_t _pb_wait_queue;        // semaphore for threads waiting for `_pb_mutex`
static StaticSemaphore_t _pb_wait_queue_buffer; // buffer for `_pb_wait_queue` which holds its state

static SemaphoreHandle_t _wfe_sem;        // for WFE blocking and IRQ signalling
static StaticSemaphore_t _wfe_sem_buffer; // buffer for `_wfe_sem` which holds its state

static SemaphoreHandle_t _cache_mutex; // for non prioritized locking/unlocking (e.g. NPU cache lock, see functions
                                       // `aton_osal_freertos_lock()` & `aton_osal_freertos_unlock()`)
static StaticSemaphore_t _cache_mutex_buffer; // buffer for `_cache_mutex` which holds its state

/*** API functions ***/

/**
 * @brief Initialize FreeRTOS OSAL implementation
 */
void aton_osal_freertos_init()
{
  /* create main priority based mechanism mutex */
  _pb_mutex =
      xSemaphoreCreateBinaryStatic(&_pb_mutex_buffer); // no thread will ever wait on this mutex so priority inheritance
                                                       // will never take place (this is most likely the most important
                                                       // downside of this OSAL implementation)
  xSemaphoreGive(_pb_mutex);                           // make it available

  /* create priority based mechanism semaphore */
  _pb_wait_queue = xSemaphoreCreateBinaryStatic(&_pb_wait_queue_buffer);

  /* create WFE semaphore */
  _wfe_sem = xSemaphoreCreateBinaryStatic(&_wfe_sem_buffer);

  /* create cache mutex */
  _cache_mutex = xSemaphoreCreateMutexStatic(&_cache_mutex_buffer);

  /* Set ATON interrupt priority */
  LL_ATON_OSAL_SET_PRIORITY(
      ATON_STD_IRQ_LINE,
      configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY); // must use `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` as we
                                                     // use `NVIC_SetPriority()`
}

/**
 * @brief De-initialize FreeRTOS OSAL implementation
 */
void aton_osal_freertos_deinit()
{
  /* nothing to do */
}

/**
 * @brief Lock priority base mutex
 */
void aton_osal_freertos_pb_lock()
{
  BaseType_t ret;

  do
  {
    ret = xSemaphoreTake(_pb_mutex, 0);

    if (ret == pdTRUE)
    { // we have gotten the mutex
      LL_ATON_LIB_UNUSED(ret);
      return; // we are done
    }
    else
    { // didn't get mutex
      /* wait on wait queue semaphore */
      ret = xSemaphoreTake(_pb_wait_queue, portMAX_DELAY); // block on semaphore
      assert(ret == pdTRUE);

      // re-try to get the mutex
    }
  } while (1);
}

/**
 * @brief Unlock priority based mutex
 */
// #define FREERTOS_NO_SUSPEND_SCHEDULER
void aton_osal_freertos_pb_unlock()
{
  BaseType_t ret;

  /* release `_pb_mutex` */
  ret = xSemaphoreGive(_pb_mutex);
  assert(ret == pdTRUE);

#ifndef FREERTOS_NO_SUSPEND_SCHEDULER
  // disable preemption
  vTaskSuspendAll();

  // check if `_pb_wait_queue` counter is zero and in case increment it by one
  UBaseType_t uxCount = uxSemaphoreGetCount(_pb_wait_queue);
  assert(uxCount <= 1);
  if (uxCount == 0)
#endif // 0/1
  {    // semaphore is currently blocked
    ret = xSemaphoreGive(_pb_wait_queue);
    assert(ret == pdTRUE);
  }

#ifndef FREERTOS_NO_SUSPEND_SCHEDULER
  // enable preemption
  xTaskResumeAll();

  if (uxCount == 0)
#endif // 0/1
  {
    /* give other tasks of same priority a chance to run
       and do not wait for the next tick interrupt */
    taskYIELD();
  }

  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Wait for event
 */
void aton_osal_freertos_wfe()
{
  BaseType_t ret;

  ret = xSemaphoreTake(_wfe_sem, portMAX_DELAY);
  assert(ret == pdTRUE);

  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Signal event
 * @note  Define macro `LL_ATON_OSAL_FREERTOS_NO_ISR_SIGNAL` if signalling is not performed from an interrupt handler.
 *        Calling this function within the same application from both ISR and normal contexts is currently not
 *        supported!
 */
void aton_osal_freertos_signal_event()
{
  BaseType_t ret;

#ifdef LL_ATON_OSAL_FREERTOS_NO_ISR_SIGNAL
  ret = xSemaphoreGive(_wfe_sem); // assuming that this function gets called from within an interrupt handler
  assert(ret == pdTRUE);
#else  // !LL_ATON_OSAL_FREERTOS_NO_ISR_SIGNAL
  xSemaphoreGiveFromISR(_wfe_sem, &ret); // assuming that this function gets called from within an interrupt handler
  portYIELD_FROM_ISR(ret);
#endif // !LL_ATON_OSAL_FREERTOS_NO_ISR_SIGNAL

  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Lock cache mutex
 */
void aton_osal_freertos_lock()
{
  BaseType_t ret;

  ret = xSemaphoreTake(_cache_mutex, portMAX_DELAY);
  assert(ret == pdTRUE);

  LL_ATON_LIB_UNUSED(ret);
}

/**
 * @brief Unlock cache mutex
 */
void aton_osal_freertos_unlock()
{
  BaseType_t ret;

  ret = xSemaphoreGive(_cache_mutex);
  assert(ret == pdTRUE);

  LL_ATON_LIB_UNUSED(ret);
}

#endif // (LL_ATON_OSAL == LL_ATON_OSAL_FREERTOS)
