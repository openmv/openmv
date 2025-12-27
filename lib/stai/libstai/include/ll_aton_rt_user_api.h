/**
 ******************************************************************************
 * @file    ll_aton_rt_user_api.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON LL runtime.
 * @note    User API to Aton Runtime.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __LL_ATON_RT_USER_API
#define __LL_ATON_RT_USER_API

#ifdef __cplusplus
extern "C"
{
#endif

#include "ll_aton_NN_interface.h"
#include "ll_aton_caches_interface.h"
#include "ll_aton_osal.h"
#include "ll_aton_platform.h"
#include "ll_aton_util.h"

  /** @defgroup User API Types, Events & Return Values
   * @{
   */

  /**
   * @brief  Return vales of the central inference run function `LL_ATON_RT_RunEpochBlock()`
   * @note   See explanations for the different values in the function description
   */
  typedef enum LL_ATON_RT_RetValues
  {
    LL_ATON_RT_NO_WFE = 0,
    LL_ATON_RT_WFE,
    LL_ATON_RT_DONE,
  } LL_ATON_RT_RetValues_t;

  /*
   * NOTE:
   * =====
   * regarding the type definitions of `LL_ATON_User_IO_Result_t` (i.e. User I/O Return Values),
   * `LL_ATON_RT_Callbacktype_t` (i.e. Callback Events), `TraceRuntime_FuncPtr_t` (i.e. Runtime Callback Functions),
   * `TraceEpochBlock_FuncPtr_t` (i.e. Epoch Callback Functions), `NN_Interface_TypeDef` (i.e. Network Interface),
   * and `NN_Instance_TypeDef` (i.e. Network Instance),
   * please refer to include file `ll_aton_NN_interface.h`
   */

  /**
   * @}
   */

  /** @defgroup User API Macros for NN Interface & Instance definition
   * @{
   */

/**
 * @brief Declare and fill a constant named NN interface object
 * @param nn_if_name name of the network as provided by option `--network-name`
 */
#define LL_ATON_DECLARE_NAMED_NN_INTERFACE(nn_if_name)                                                                 \
  LL_ATON_DECLARE_NAMED_NN_PROTOS(nn_if_name);                                                                         \
                                                                                                                       \
  static const NN_Interface_TypeDef NN_Interface_##nn_if_name = {                                                      \
      .network_name = #nn_if_name,                                                                                     \
      .ec_network_init = &LL_ATON_EC_Network_Init_##nn_if_name,                                                        \
      .ec_inference_init = &LL_ATON_EC_Inference_Init_##nn_if_name,                                                    \
      .input_setter = &LL_ATON_Set_User_Input_Buffer_##nn_if_name,                                                     \
      .input_getter = &LL_ATON_Get_User_Input_Buffer_##nn_if_name,                                                     \
      .output_setter = &LL_ATON_Set_User_Output_Buffer_##nn_if_name,                                                   \
      .output_getter = &LL_ATON_Get_User_Output_Buffer_##nn_if_name,                                                   \
      .epoch_block_items = &LL_ATON_EpochBlockItems_##nn_if_name,                                                      \
      .output_buffers_info = &LL_ATON_Output_Buffers_Info_##nn_if_name,                                                \
      .input_buffers_info = &LL_ATON_Input_Buffers_Info_##nn_if_name,                                                  \
      .weight_encryption_info = &LL_ATON_WeightEncryption_Info_##nn_if_name,                                           \
      .blob_encryption_info = &LL_ATON_BlobEncryption_Info_##nn_if_name,                                               \
      .internal_buffers_info = &LL_ATON_Internal_Buffers_Info_##nn_if_name}

/**
 * @brief Declare and fill a non-constant named NN execution instance
 * @param nn_exec_name typically name of the network as provided by option `--network-name`
 * @param nn_if_ptr pointer to network interface
 */
#define LL_ATON_DECLARE_NAMED_NN_INSTANCE(nn_exec_name, nn_if_ptr)                                                     \
  static NN_Instance_TypeDef NN_Instance_##nn_exec_name = {.network = nn_if_ptr, .exec_state = {0}}

/**
 * @brief Declare and fill a non-constant named NN execution instance and constant network interface,
 *        which get linked together (by this macro).
 * @param nn_name name of the network as provided by option `--network-name`
 */
#define LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(nn_name)                                                       \
  LL_ATON_DECLARE_NAMED_NN_INTERFACE(nn_name);                                                                         \
  LL_ATON_DECLARE_NAMED_NN_INSTANCE(nn_name, &NN_Interface_##nn_name);

  /**
   * @}
   */

  /** @defgroup User API Function Declarations
   * (apart from cache maintenance functions, which can be found in file `ll_aton_caches_interface.h`)
   * @{
   */

  /**
   * @brief Initialize the ATON runtime
   * @note  If not explicitly stated otherwise, this function must be called before any other `LL_*` Aton runtime or
   *        library function
   */
  void LL_ATON_RT_RuntimeInit(void);

  /**
   * @brief Register callback for ATON runtime related events (e.g. initialization/deinitialization, see
   * `LL_ATON_RT_Callbacktype_t`)
   * @param rt_callback Function pointer to callback function (set to `NULL` to disable epoch tracing)
   *
   * @note  This function must only be called when no network is currently executing
   *        and should be called BEFORE `LL_ATON_RT_RuntimeInit()` otherwise runtime init events will be lost!
   */
  void LL_ATON_RT_SetRuntimeCallback(TraceRuntime_FuncPtr_t rt_callback);

  /**
   * @brief Register callback for tracing epoch/network related events (see `LL_ATON_RT_Callbacktype_t`)
   * @param epoch_block_callback Function pointer to callback function (set to `NULL` to disable epoch tracing)
   * @param nn_instance          Pointer to network instance for which to set the callback (may not be `NULL`)
   *
   * @deprecated This function is deprecated and will be removed in a future release.
   *             Use `LL_ATON_RT_SetNetworkCallback()` instead!
   */
  void LL_ATON_RT_SetEpochCallback(TraceEpochBlock_FuncPtr_t epoch_block_callback, NN_Instance_TypeDef *nn_instance);

  /**
   * @brief Register callback for tracing epoch/network related events (see `LL_ATON_RT_Callbacktype_t`)
   * @param nn_instance          Pointer to network instance for which to set the callback (may not be `NULL`)
   * @param epoch_block_callback Function pointer to callback function (set to `NULL` to disable epoch tracing)
   *
   * @note  This function must only be called while the passed network instance is not executing
   *        and should be called before `LL_ATON_RT_Init_Network()`!
   */
  void LL_ATON_RT_SetNetworkCallback(NN_Instance_TypeDef *nn_instance, TraceEpochBlock_FuncPtr_t epoch_block_callback);

  /**
   * @brief Initialise a network instance
   * @param nn_instance Pointer to network instance to initialize
   */
  void LL_ATON_RT_Init_Network(NN_Instance_TypeDef *nn_instance);

  /**
   * @brief Reset network instance for getting ready for a new inference
   * @param nn_instance Pointer to network instance to initialize
   *
   * @note May only be called in case `LL_ATON_RT_RunEpochBlock()` returned either `LL_ATON_RT_DONE` or
   *       `LL_ATON_RT_NO_WFE`
   */
  void LL_ATON_RT_Reset_Network(NN_Instance_TypeDef *nn_instance);

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
   * @brief  Sets user allocated inputs (one at a time)
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @param  num zero base index of the input buffer to set
   * @param  buffer pointer to the area used to store this input
   * @param  size size of the memory reserved for this input
   */
  static inline LL_ATON_User_IO_Result_t LL_ATON_Set_User_Input_Buffer(const NN_Instance_TypeDef *nn_instance,
                                                                       uint32_t num, void *buffer, uint32_t size);

  /**
   * @brief  Gets user allocated inputs (one at a time)
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @param  num zero base index of the input buffer to get
   * @retval returns a pointer to the specified user allocated input
   */
  static inline void *LL_ATON_Get_User_Input_Buffer(const NN_Instance_TypeDef *nn_instance, uint32_t num);

  /**
   * @brief  Sets user allocated outputs (one at a time)
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @param  num zero base index of the output buffer to set
   * @param  buffer pointer to the area used to store this output
   * @param  size size of the memory reserved for this output
   */
  static inline LL_ATON_User_IO_Result_t LL_ATON_Set_User_Output_Buffer(const NN_Instance_TypeDef *nn_instance,
                                                                        uint32_t num, void *buffer, uint32_t size);

  /**
   * @brief  Gets user allocated inputs (one at a time)
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @param  num zero base index of the output buffer to get
   * @retval returns a pointer to the specified user allocated output
   */
  static inline void *LL_ATON_Get_User_Output_Buffer(const NN_Instance_TypeDef *nn_instance, uint32_t num);

  /**
   * @brief  Returns an array of structures describing input buffers
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @retval returns a pointer to the array of LL_Buffer_InfoTypeDef, name is NULL for the last one
   */
  static inline const LL_Buffer_InfoTypeDef *LL_ATON_Output_Buffers_Info(const NN_Instance_TypeDef *nn_instance);

  /**
   * @brief  Returns an array of structures describing output buffers
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @retval Returns a pointer to the array of LL_Buffer_InfoTypeDef, name is NULL for the last one
   */
  static inline const LL_Buffer_InfoTypeDef *LL_ATON_Input_Buffers_Info(const NN_Instance_TypeDef *nn_instance);

  /**
   * @brief  Returns an array of structures describing epoch output transient buffers
   * @param  nn_instance pointer to the network instance
   *         (MUST be an already initialized instance)
   * @retval Returns a pointer to the array of LL_Buffer_InfoTypeDef, name is NULL for the last one
   */
  static inline const LL_Buffer_InfoTypeDef *LL_ATON_Internal_Buffers_Info(const NN_Instance_TypeDef *nn_instance);

  /**
   * @brief De-initialise a network instance
   * @param nn_instance Pointer to network instance to de-initialize
   */
  void LL_ATON_RT_DeInit_Network(NN_Instance_TypeDef *nn_instance);

  /**
   * @brief De-initialize the ATON runtime
   */
  void LL_ATON_RT_RuntimeDeInit(void);

  /**
   * @}
   */

#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
#include "ll_aton_reloc_network.h"
#endif

  /** @defgroup User API Function Inline Implementations (of some User API functions)
   * @{
   */

  static inline LL_ATON_User_IO_Result_t LL_ATON_Set_User_Input_Buffer(const NN_Instance_TypeDef *nn_instance,
                                                                       uint32_t num, void *buffer, uint32_t size)
  {
    LL_ATON_ASSERT(nn_instance != NULL);
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_set_input(nn_instance->exec_state.inst_reloc, num, buffer, size);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->input_setter != NULL);
      return nn_instance->network->input_setter(num, buffer, size);
    }
#else /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->input_setter != NULL));
  return nn_instance->network->input_setter(num, buffer, size);
#endif
  }

  static inline void *LL_ATON_Get_User_Input_Buffer(const NN_Instance_TypeDef *nn_instance, uint32_t num)
  {
    LL_ATON_ASSERT(nn_instance != NULL);
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_get_input(nn_instance->exec_state.inst_reloc, num);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->input_getter != NULL);
      return nn_instance->network->input_getter(num);
    }
#else  /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->input_getter != NULL));
  return nn_instance->network->input_getter(num);
#endif /* !LL_ATON_RT_RELOC */
  }

  static inline LL_ATON_User_IO_Result_t LL_ATON_Set_User_Output_Buffer(const NN_Instance_TypeDef *nn_instance,
                                                                        uint32_t num, void *buffer, uint32_t size)
  {
    LL_ATON_ASSERT(nn_instance != NULL);
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_set_output(nn_instance->exec_state.inst_reloc, num, buffer, size);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->output_setter != NULL);
      return nn_instance->network->output_setter(num, buffer, size);
    }
#else /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->output_setter != NULL));
  return nn_instance->network->output_setter(num, buffer, size);
#endif
  }

  static inline void *LL_ATON_Get_User_Output_Buffer(const NN_Instance_TypeDef *nn_instance, uint32_t num)
  {
    LL_ATON_ASSERT(nn_instance != NULL);
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_get_output(nn_instance->exec_state.inst_reloc, num);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->output_getter != NULL);
      return nn_instance->network->output_getter(num);
    }
#else /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->output_getter != NULL));
  return nn_instance->network->output_getter(num);
#endif
  }

  static inline const LL_Buffer_InfoTypeDef *LL_ATON_Output_Buffers_Info(const NN_Instance_TypeDef *nn_instance)
  {
    LL_ATON_ASSERT(nn_instance != NULL);
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_get_output_buffers_info(nn_instance->exec_state.inst_reloc);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->output_buffers_info != NULL);
      return nn_instance->network->output_buffers_info();
    }
#else /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->output_buffers_info != NULL));
  return nn_instance->network->output_buffers_info();
#endif
  }

  static inline const LL_Buffer_InfoTypeDef *LL_ATON_Input_Buffers_Info(const NN_Instance_TypeDef *nn_instance)
  {
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    LL_ATON_ASSERT(nn_instance != NULL);
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_get_input_buffers_info(nn_instance->exec_state.inst_reloc);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->input_buffers_info != NULL);
      return nn_instance->network->input_buffers_info();
    }
#else /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->input_buffers_info != NULL));
  return nn_instance->network->input_buffers_info();
#endif
  }

  static inline const LL_Buffer_InfoTypeDef *LL_ATON_Internal_Buffers_Info(const NN_Instance_TypeDef *nn_instance)
  {
#if defined(LL_ATON_RT_RELOC) && !defined(BUILD_AI_NETWORK_RELOC)
    LL_ATON_ASSERT(nn_instance != NULL);
    if (nn_instance->exec_state.inst_reloc != 0)
    {
      return ai_rel_network_get_internal_buffers_info(nn_instance->exec_state.inst_reloc);
    }
    else
    {
      LL_ATON_ASSERT(nn_instance->network != NULL);
      LL_ATON_ASSERT(nn_instance->network->internal_buffers_info != NULL);
      return nn_instance->network->internal_buffers_info();
    }
#else /* !LL_ATON_RT_RELOC */
  LL_ATON_ASSERT((nn_instance != NULL) && (nn_instance->network != NULL) &&
                 (nn_instance->network->internal_buffers_info != NULL));
  return nn_instance->network->internal_buffers_info();
#endif
  }

  /**
   * @}
   */

#ifdef __cplusplus
}
#endif

#endif /* __LL_ATON_RT_USER_API */
