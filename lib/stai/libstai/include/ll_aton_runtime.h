/**
 ******************************************************************************
 * @file    ll_aton_runtime.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON LL runtime.
 * @note    ATON LL runtime currently assumes a single network run
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

#ifndef __LL_ATON_RUNTIME_H
#define __LL_ATON_RUNTIME_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ll_aton_config.h"

#include "ll_aton_NN_interface.h"
#include "ll_aton_osal.h"
#include "ll_aton_platform.h"
#include "ll_aton_util.h"

  /*** Types ***/

  typedef EpochBlock_ItemTypeDef LL_ATON_RT_EpochBlockItem_t;

  /*** Helper Functions ***/

  static inline void __ll_set_aton_owner(NN_Instance_TypeDef *new_owner)
  {
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(new_owner != __ll_current_aton_ip_owner);

    LL_ATON_OSAL_LOCK_ATON();

    LL_ATON_ASSERT(__ll_current_aton_ip_owner == NULL);

#ifndef NDEBUG
    extern uint32_t volatile __ll_current_wait_mask;
    LL_ATON_ASSERT(__ll_current_wait_mask == 0);
#endif // NDEBUG

    __ll_current_aton_ip_owner = new_owner;
  }

  static inline void __ll_clear_aton_owner(NN_Instance_TypeDef *current_owner)
  {
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(current_owner == __ll_current_aton_ip_owner);

#ifndef NDEBUG
    extern uint32_t volatile __ll_current_wait_mask;
    LL_ATON_ASSERT(__ll_current_wait_mask == 0);
#endif // NDEBUG

    __ll_current_aton_ip_owner = NULL;
    LL_ATON_OSAL_UNLOCK_ATON();
  }

  /**
   *  Note: the following function may only be called at the beginning of
   *       `LL_ATON_Start_EpochBlock<n>()` functions, assuming also that at that point
   *       no streaming engine interrupts might trigger (anymore)!
   **/
  static inline void __LL_ATON_RT_Start_AtoNN_Epoch(NN_Instance_TypeDef *nn_instance)
  {
    LL_ATON_ASSERT(nn_instance != NULL);
#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
    LL_ATON_ASSERT(nn_instance->exec_state.triggered_events ==
                   0x0); // with the removal of parallel SW/HW epochs execution all triggered events must have been
                         // cleared at this point in time!
#endif
  }

  static inline uint32_t __LL_ATON_RT_GetCurrEpochBlockIndex(NN_Instance_TypeDef *nn_instance)
  {
    const LL_ATON_RT_EpochBlockItem_t *_current_epoch_block = nn_instance->exec_state.current_epoch_block;
    const LL_ATON_RT_EpochBlockItem_t *_first_epoch_block = nn_instance->exec_state.first_epoch_block;
    LL_ATON_ASSERT(_first_epoch_block <= _current_epoch_block); // should never happen

    return (_current_epoch_block - _first_epoch_block);
  }

  static inline void __LL_ATON_RT_SetCurrentEpochBlock(int32_t index, NN_Instance_TypeDef *nn_instance)
  {
#ifndef NDEBUG
    /* should never happen (assumes that a `nn_instance->exec_state.current_epoch_block++` will be
       performed by the runtime immediately afterwards) */
    LL_ATON_ASSERT(index < (int32_t)(nn_instance->exec_state.nr_of_epoch_blocks - 1));
#endif

    nn_instance->exec_state.current_epoch_block = &nn_instance->exec_state.first_epoch_block[index];
  }

  /* set wait mask(s) in interrupt controller */
  static inline void __LL_ATON_RT_SetWaitMask(uint32_t wait_mask)
  {
#ifndef NDEBUG
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);

    extern uint32_t volatile __ll_current_wait_mask;
    __ll_current_wait_mask = wait_mask;
#endif // NDEBUG

#if (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
    wait_mask <<= ATON_STRENG_INT(0);
#ifndef LL_ATON_RT_USE_IRQ_OR_MASK
    /* configure interrupt controller AND mask for epoch block */
    ATON_INTCTRL_STD_INTANDMSK_SET(~wait_mask);
#else  // LL_ATON_RT_USE_IRQ_OR_MASK
    /* configure interrupt controller OR mask for epoch block */
    uint32_t val = ATON_STRENG_INT_MASK(ATON_STRENG_NUM, 0, 0); // disable all streaming engine events in OR mask
                                                                // (all other events & errors are enabled)
    val &= ~wait_mask;
    ATON_INTCTRL_STD_INTORMSK_SET(val);
#endif // LL_ATON_RT_USE_IRQ_OR_MASK
#endif // (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
  }

  /* return from inserted epoch block */
  static inline void __LL_ATON_RT_RetFromLibEpochBlockArray(bool unlock, NN_Instance_TypeDef *nn_instance)
  {
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;

    if (!unlock)
    {
      LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
      LL_ATON_ASSERT(nn_instance == NULL);

      nn_instance = __ll_current_aton_ip_owner;
    }

    LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
    LL_ATON_ASSERT(unlock ? EpochBlock_IsLastEpochBlock(nn_instance->exec_state.current_epoch_block)
                          : EpochBlock_IsEpochInternal(nn_instance->exec_state.current_epoch_block));
    LL_ATON_ASSERT(EpochBlock_IsEpochHybrid(nn_instance->exec_state.saved_current_epoch_block));

    /* Clear owner */
    if (unlock)
    {
      __ll_clear_aton_owner(__ll_current_aton_ip_owner);
    }

    /* set old context */
    LL_ATON_ASSERT(nn_instance->exec_state.next_epoch_block == NULL);
    nn_instance->exec_state.current_epoch_block = nn_instance->exec_state.saved_current_epoch_block;
    nn_instance->exec_state.first_epoch_block = nn_instance->exec_state.saved_first_epoch_block;

#ifndef NDEBUG
    nn_instance->exec_state.nr_of_epoch_blocks = nn_instance->exec_state.saved_nr_of_epoch_blocks;
#endif

    /* reset saved context */
    nn_instance->exec_state.saved_current_epoch_block = NULL;
    nn_instance->exec_state.saved_first_epoch_block = NULL;
#ifndef NDEBUG
    nn_instance->exec_state.saved_nr_of_epoch_blocks = 0;
#endif
  }

  /*** AtoNN API Functions ***/

  static inline void LL_ATON_RT_Insert_LibEpochBlockArray(const LL_ATON_RT_EpochBlockItem_t *new_epoch_block_array)
  {
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);

    // only one saved context at a time allowed!
    LL_ATON_ASSERT(__ll_current_aton_ip_owner->exec_state.next_epoch_block == NULL);
    LL_ATON_ASSERT(__ll_current_aton_ip_owner->exec_state.saved_current_epoch_block == NULL);

    __ll_current_aton_ip_owner->exec_state.next_epoch_block = new_epoch_block_array;
  }

  /**
   * Note: the following two API functions may only be called at the end of `LL_ATON_End_EpochBlock<n>()` functions
   *       (otherwise the runtime will fail)!!!
   **/
  static inline void LL_ATON_RT_IncCurrEpochBlock(uint32_t inc)
  {
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);

    uint32_t current_index = __LL_ATON_RT_GetCurrEpochBlockIndex(__ll_current_aton_ip_owner);
    current_index += inc;
    __LL_ATON_RT_SetCurrentEpochBlock(current_index, __ll_current_aton_ip_owner);
  }

  static inline void LL_ATON_RT_DecCurrEpochBlock(uint32_t dec)
  {
    extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);

    uint32_t current_index = __LL_ATON_RT_GetCurrEpochBlockIndex(__ll_current_aton_ip_owner);
    LL_ATON_ASSERT((current_index + 1) >= dec); // should never happen
    int32_t new_index = current_index - dec;
    __LL_ATON_RT_SetCurrentEpochBlock(new_index, __ll_current_aton_ip_owner);
  }

  /*** User API Functions ***/

  /**
   * @brief Register callback for ATON runtime related events (e.g. initialization/deinitialization, see
   * `LL_ATON_RT_Callbacktype_t`)
   * @param rt_callback Function pointer to callback function (set to `NULL` to disable epoch tracing)
   *
   * @note  This function must only be called when no network is currently executing!
   */
  void LL_ATON_RT_SetRuntimeCallback(TraceRuntime_FuncPtr_t rt_callback);

  /**
   * @brief Register callback for tracing epoch/network related events (see `LL_ATON_RT_Callbacktype_t`)
   * @param epoch_block_callback Function pointer to callback function (set to `NULL` to disable epoch tracing)
   * @param nn_instance          Pointer to network instance for which to set the callback (may not be `NULL`)
   *
   * @note  This function must only be called while the passed network instance is not executing
   *        and should be called before `LL_ATON_RT_Init_Network()`!
   */
  void LL_ATON_RT_SetEpochCallback(TraceEpochBlock_FuncPtr_t epoch_block_callback, NN_Instance_TypeDef *nn_instance);

  /**
   * @brief Initialise a network instance
   * @param nn_instance Pointer to network instance to initialize
   */
  void LL_ATON_RT_Init_Network(NN_Instance_TypeDef *nn_instance);

  /**
   * @brief De-initialise a network instance
   * @param nn_instance Pointer to network instance to de-initialize
   */
  void LL_ATON_RT_DeInit_Network(NN_Instance_TypeDef *nn_instance);

  /**
   * @brief Reset network instance for getting ready for a new inference
   * @param nn_instance Pointer to network instance to initialize
   *
   * @note May only be called in case `LL_ATON_RT_RunEpochBlock()` returned either `LL_ATON_RT_DONE` or
   *       `LL_ATON_RT_NO_WFE`
   */
  void LL_ATON_RT_Reset_Network(NN_Instance_TypeDef *nn_instance);

  /**
   * @brief Initialize the ATON runtime
   */
  void LL_ATON_RT_RuntimeInit(void);

  /**
   * @brief De-initialize the ATON runtime
   */
  void LL_ATON_RT_RuntimeDeInit(void);

  /**
   * @brief  Checks status of previously started epoch block and
   *         starts execution of next epoch block of the NN provided the previous epoch block has terminated
   * @param nn_instance Pointer to network instance to run/continue (may not be `NULL`)
   * @retval LL_ATON_RT_NO_WFE  Next epoch block may be started, NN execution not finished, do not call
   *                            `LL_ATON_OSAL_WFE()`
   * @retval LL_ATON_RT_WFE     Epoch block is still running, NN execution not finished, you may call
   *                            `LL_ATON_OSAL_WFE()`.
   *                            NOTE, that in this case no other network instance may be run/continued from within the
   *                            same thread!
   *                            It is entirely the user's responsibility to comply with this restriction!
   * @retval LL_ATON_RT_DONE    NN execution finished
   */
  LL_ATON_RT_RetValues_t LL_ATON_RT_RunEpochBlock(NN_Instance_TypeDef *nn_instance);

  /**
   * @brief Template for synchronously executing a single network instance (e.g. regression tests)
   * @param network_instance pointer to the network instance representing the network and execution instance to execute.
   *                         The instance object MUST have already set a valid link to a network interface.
   *                         The user may declare/instantiate such an object by using either macro
   *                          `LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE()` to create both the execution instance
   *                         and the network interface, or macros
   *                         `LL_ATON_DECLARE_NAMED_NN_INTERFACE()` & `LL_ATON_DECLARE_NAMED_NN_INSTANCE()` to
   *                         create/instantiate the objects separately.
   */
  void LL_ATON_RT_Main(NN_Instance_TypeDef *network_instance);

  /** @brief Dumps status of all DMAs. Used for debugging purposes
   */
  void dump_dma_state(void);

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_RUNTIME_H
