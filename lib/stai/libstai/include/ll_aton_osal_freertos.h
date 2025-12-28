/**
 ******************************************************************************
 * @file    ll_aton_osal_freertos.h
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

#ifndef __LL_ATON_OSAL_FREERTOS_H
#define __LL_ATON_OSAL_FREERTOS_H

/*** Common macro for all RTOS OSAL ports ***/
/* Please make sure that beyond macro value fits your application needs or define the correct value on the compiler
   command line!
   Note, this value should be equal to `1` only if there are parallel neural network (aka NN) execution
   threads in your application. */
#ifndef APP_HAS_PARALLEL_NETWORKS
#define APP_HAS_PARALLEL_NETWORKS 1 // there are parallel networks in the application
#warning Using default value `1` for macro `APP_HAS_PARALLEL_NETWORKS`.
#endif // APP_HAS_PARALLEL_NETWORKS

/*** FreeRTOS includes ***/
#include "FreeRTOS.h"

#include "semphr.h"
#include "task.h"

/*** `LL_ATON` includes */
#include "ll_aton_attributes.h"
#include "ll_aton_osal.h"
#include "ll_aton_platform.h"
#include "ll_aton_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* API functions*/
  void aton_osal_freertos_init(void);
  void aton_osal_freertos_deinit(void);

  void aton_osal_freertos_wfe(void);
  void aton_osal_freertos_signal_event(void);

  void aton_osal_freertos_dao_lock(void);
  void aton_osal_freertos_dao_unlock(void);

  void aton_osal_freertos_lock(void);
  void aton_osal_freertos_unlock(void);

/*** FreeRTOS dependent type macros ***/
#define _DaoMutexNoWaitersType_ SemaphoreHandle_t
#define _DaoWaitQueueType_      SemaphoreHandle_t
#define _DaoWaitQueueValueType_ UBaseType_t
#define _WfeSemaphoreType_      SemaphoreHandle_t
#define _CacheMutexType_        SemaphoreHandle_t
#define _TaskHandleType_        TaskHandle_t
#define _PriorityType_          UBaseType_t
#define _ReturnType_            BaseType_t

/*** FreeRTOS dependent values ***/
#define _OsTrue_     pdTRUE
#define _NullHandle_ NULL

  /*** Helper inline functions ***/
  static inline _ReturnType_ _my_xSemaphoreCreateBinaryStatic(SemaphoreHandle_t *dao_addr,
                                                              StaticSemaphore_t *static_buffer)
  {
    SemaphoreHandle_t ret = xSemaphoreCreateBinaryStatic(static_buffer);
    if (ret != _NullHandle_)
    {
      *dao_addr = ret;
      return _OsTrue_;
    }
    else
    {
      return pdFALSE;
    }
  }

  static inline _ReturnType_ _my_xSemaphoreCreateCountingStatic(SemaphoreHandle_t *dao_addr,
                                                                StaticSemaphore_t *static_buffer)
  {
    SemaphoreHandle_t ret = xSemaphoreCreateCountingStatic(0xFFFFFFFF, 0, static_buffer);
    if (ret != _NullHandle_)
    {
      *dao_addr = ret;
      return _OsTrue_;
    }
    else
    {
      return pdFALSE;
    }
  }

  static inline _ReturnType_ _my_xSemaphoreCreateMutexStatic(SemaphoreHandle_t *dao_addr,
                                                             StaticSemaphore_t *static_buffer)
  {
    SemaphoreHandle_t ret = xSemaphoreCreateMutexStatic(static_buffer);
    if (ret != _NullHandle_)
    {
      *dao_addr = ret;
      return _OsTrue_;
    }
    else
    {
      return pdFALSE;
    }
  }

/*** FreeRTOS function macros ***/
#define _CreateDaoMutexNoWaiters_(_dao_obj, _dao_static_buffer)                                                        \
  _my_xSemaphoreCreateBinaryStatic(&(_dao_obj), &(_dao_static_buffer))
#define _CreateDaoWaitQueue_(_dao_obj, _dao_static_buffer)                                                             \
  _my_xSemaphoreCreateBinaryStatic(&(_dao_obj), &(_dao_static_buffer))
#define _CreateWfeSemaphore_(_dao_obj, _dao_static_buffer)                                                             \
  _my_xSemaphoreCreateCountingStatic(&(_dao_obj), &(_dao_static_buffer))
#define _CreateCacheMutex_(_dao_obj, _dao_static_buffer)                                                               \
  _my_xSemaphoreCreateMutexStatic(&(_dao_obj), &(_dao_static_buffer))

#define _FinalizeIRQHandling_()                                                                                        \
  LL_ATON_OSAL_SET_PRIORITY(                                                                                           \
      ATON_STD_IRQ_LINE,                                                                                               \
      configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY); // must use `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` as we
                                                     // use `NVIC_SetPriority()`

#define _DisablePreemption_() vTaskSuspendAll()
#define _EnablePreemption_()  xTaskResumeAll()

#define _InitNonDao_()   /* nothing to do */
#define _DeInitNonDao_() /* nothing to do */

#define _GetCurrentTaskHandle_()        xTaskGetCurrentTaskHandle()
#define _GetTaskPriority_(_task)        uxTaskPriorityGet(_task)
#define _SetTaskPriority_(_task, _prio) vTaskPrioritySet(_task, _prio)
#define _YieldCurrentTask_()            taskYIELD()

#define _MakeDaoMutexNoWaitersAvailable_(_mutex) xSemaphoreGive(_mutex)
#define _GetDaoMutexNoWaiters_(_mutex)           xSemaphoreTake((_mutex), 0)
#define _ReleaseDaoMutexNoWaiters_(_mutex)       xSemaphoreGive(_mutex)

#define _MakeDaoWaitQueueUnavailable_(_wq)
#define _GetDaoWaitQueueValue_(_wq) uxSemaphoreGetCount(_wq)
#define _GetDaoWaitQueue_(_wq)      xSemaphoreTake(_wq, portMAX_DELAY)
#define _ReleaseDaoWaitQueue_(_wq)  xSemaphoreGive(_wq)

#define _MakeWfeSemaphoreUnavailable_(_sem)
#define _GetWfeSemaphore_(_sem)             xSemaphoreTake(_sem, portMAX_DELAY)
#define _ReleaseWfeSemaphore_(_sem)         xSemaphoreGive(_sem)
#define _ReleaseWfeSemaphoreISR_(_sem, ...) xSemaphoreGiveFromISR(_sem, __VA_ARGS__)

#define _MakeCacheMutexAvailable_(_mutex)
#define _GetCacheMutex_(_mutex)     xSemaphoreTake(_mutex, portMAX_DELAY)
#define _ReleaseCacheMutex_(_mutex) xSemaphoreGive(_mutex)

#define _HeadIsrCode_(...)
#define _TailIsrCode_(...) portYIELD_FROM_ISR(__VA_ARGS__)

#define _FirstPrioHigherThanScnd_(_first, _second) ((_first) > (_second)) // smaller numbers mean lower priority

#ifdef __cplusplus
}
#endif

#endif //__LL_ATON_OSAL_FREERTOS_H
