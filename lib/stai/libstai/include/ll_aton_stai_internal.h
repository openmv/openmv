/**
 ******************************************************************************
 * @file    ll_aton_stai_internal.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Implementation of ST.AI public & internal APIs for ATON
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

#ifndef __LL_ATON_STAI_INTERNAL_H
#define __LL_ATON_STAI_INTERNAL_H

// ATON headers
#include "ll_aton_rt_user_api.h"
#include "ll_aton_util.h"

// ST.AI headers
#include "stai_aton_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
#define STAI_INTERNAL_ENTRY /* STAI_INTERNAL_ENTRY */
#define STAI_SIZE(_size)    ((stai_size)(_size))

/*****************************************************************************/
#define STAI_INIT_BUFFER(_flags, _size, _address)                                                                      \
  {                                                                                                                    \
    .size = (_size), .address = (uintptr_t)(_address), .flags = (_flags),                                              \
  }

#define STAI_INIT_TENSOR(_name, _flags, _fmt, _size_bytes, _shape, _scale, _zeropoint)                                 \
  {                                                                                                                    \
    .size_bytes = (_size_bytes), .flags = (_flags), .format = (stai_format)(_fmt), .shape = STAI_PACK(_shape),         \
    .scale = STAI_PACK(_scale), .zeropoint = STAI_PACK(_zeropoint), .name = (_name)                                    \
  }

#define STAI_INIT_ARRAY(_size, _ptr)                                                                                   \
  {                                                                                                                    \
    .size = STAI_SIZE(_size), .data = STAI_PACK(_ptr)                                                                  \
  }

#define STAI_CAST_ARRAY(_type, _size, _ptr)                                                                            \
  {                                                                                                                    \
    .size = STAI_SIZE(_size), .data = (_type)STAI_PACK(_ptr)                                                           \
  }

#define STAI_DECLARE_ARRAY(_type, _size, _array_initializer)                                                           \
  {                                                                                                                    \
    .size = STAI_SIZE(_size), .data = (_type[_size])STAI_PACK(_array_initializer)                                      \
  }

#define STAI_EMPTY_ARRAY()                                                                                             \
  {                                                                                                                    \
    .size = 0, .data = NULL                                                                                            \
  }

#define STAI_INIT_VERSION(_major, _minor, _micro)                                                                      \
  {                                                                                                                    \
    .major = (_major), .minor = (_minor), .micro = (_micro), .reserved = 0x0                                           \
  }

/*****************************************************************************/
/**  Internal ATON ST.AI APIs                                               **/
/*****************************************************************************/

/*****************************************************************************/
/**Macros **/
/* Set context error */
#define __LL_ATON_STAI_SET_1ST_CTX_ERROR_AND_RETURN(ctx, err)                                                          \
  do                                                                                                                   \
  {                                                                                                                    \
    /* check context handle */                                                                                         \
    if (!(ctx))                                                                                                        \
      return STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE;                                                                \
                                                                                                                       \
    if (((ctx)->first_error) < STAI_ERROR_GENERIC)                                                                     \
      (ctx)->first_error = (err);                                                                                      \
    return (err);                                                                                                      \
  } while (0)

/* check context & network interface handle */
#define __LL_ATON_STAI_ACQUIRE_INTERFACE(_net_interface, _network)                                                     \
  const NN_Interface_TypeDef *_net_interface = NULL;                                                                   \
  stai_return_code _ret = __ll_aton_stai_check_context_network_handle(_network, &_net_interface);                      \
  if (_ret >= STAI_ERROR_GENERIC)                                                                                      \
    return _ret;

#define __LL_ATON_START_CONT_EXEC(_nn_instance)                                                                        \
  do                                                                                                                   \
  {                                                                                                                    \
    STAI_ASSERT((_nn_instance) != NULL);                                                                               \
    _stai_aton_context *_stai_nn_ctx =                                                                                 \
        (_stai_aton_context *)((void *)(_nn_instance)) - offsetof(_stai_aton_context, network_instance);               \
                                                                                                                       \
    /* start/continue execution */                                                                                     \
    __ll_aton_stai_set_execution_status(LL_ATON_RT_RunEpochBlock((_nn_instance)), _stai_nn_ctx);                       \
                                                                                                                       \
    /* get current execution status */                                                                                 \
    stai_return_code current_exec_status = __ll_aton_stai_get_execution_status(_stai_nn_ctx);                          \
    return current_exec_status;                                                                                        \
  } while (0)

  /** Functions **/
  STAI_API_DECLARE_BEGIN

  STAI_INTERNAL_ENTRY
  static inline stai_return_code __ll_aton_stai_check_context_network_handle(stai_network *network,
                                                                             const NN_Interface_TypeDef **nn_i_ptr_ptr)
  {
    /* check context handle */
    if (!network)
      return STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE;

    /* check network handle */
    _stai_aton_context *context_ptr = (_stai_aton_context *)network;
    const NN_Interface_TypeDef *nn_i_ptr = context_ptr->network_instance.network;
    if (!nn_i_ptr)
    {
      __LL_ATON_STAI_SET_1ST_CTX_ERROR_AND_RETURN(context_ptr, STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE);
    }

    /* save network pointer */
    *nn_i_ptr_ptr = nn_i_ptr;
    return STAI_SUCCESS;
  }

  STAI_INTERNAL_ENTRY
  void __ll_aton_stai_init_network_instance(_stai_aton_context *nn_context);
  STAI_INTERNAL_ENTRY
  void __ll_aton_stai_deinit_network_instance(_stai_aton_context *nn_context);

  STAI_INTERNAL_ENTRY
  void __ll_aton_stai_set_execution_status(LL_ATON_RT_RetValues_t status, _stai_aton_context *nn_context);
  STAI_INTERNAL_ENTRY
  stai_return_code __ll_aton_stai_get_execution_status(_stai_aton_context *nn_context);

  STAI_INTERNAL_ENTRY
  stai_return_code __ll_aton_stai_run(stai_network *network, const stai_run_mode mode);

  STAI_INTERNAL_ENTRY
  stai_return_code __ll_aton_stai_reset(stai_network *network);

  STAI_INTERNAL_ENTRY
  void _stai_aton_internal_epoch_block_callback(LL_ATON_RT_Callbacktype_t ctype, const NN_Instance_TypeDef *nn_instance,
                                                const EpochBlock_ItemTypeDef *epoch_block);

  STAI_API_DECLARE_END

#ifndef offsetof
#define offsetof(st, m) ((size_t) & (((st *)0)->m))
#endif

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_STAI_INTERNAL_H
