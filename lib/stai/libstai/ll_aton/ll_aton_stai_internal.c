/**
 ******************************************************************************
 * @file    ll_aton_stai_internal.c
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

#include <stdint.h>

#include "stai_debug.h"

#include "ll_aton_caches_interface.h"
#include "ll_aton_rt_user_api.h"
#include "ll_aton_runtime.h"
#include "ll_aton_stai_internal.h"
#include "ll_aton_version.h"

/* -----------------------------------------------------------------------------
 * MACRO - ARM tool chain definition
 * -----------------------------------------------------------------------------
 */

#undef _IS_AC5_COMPILER
#undef _IS_AC6_COMPILER
#undef _IS_GHS_COMPILER
#undef _IS_HTC_COMPILER
#undef _IS_GCC_COMPILER
#undef _IS_IAR_COMPILER

/* ARM Compiler 5 tool-chain */
#if defined(__CC_ARM)
// #if ((__ARMCC_VERSION >= 5000000) && (__ARMCC_VERSION < 6000000))
#define _IS_AC5_COMPILER 1

/* ARM Compiler 6 tool-chain */
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define _IS_AC6_COMPILER 1

/* GHS tool-chain */
#elif defined(__ghs__)
#define _IS_GHS_COMPILER 1

/* HIGHTEC tool-chain */
#elif defined(__clang__)
#define _IS_HTC_COMPILER 1

/* GCC tool-chain */
#elif defined(__GNUC__)
#define _IS_GCC_COMPILER 1

/* IAR tool-chain */
#elif defined(__ICCARM__)
#define _IS_IAR_COMPILER 1

#else
#define _IS_UNDEFINED_COMPILER 1
#endif

#undef _IS_ACx_COMPILER
#if defined(_IS_AC5_COMPILER) && _IS_AC5_COMPILER || defined(_IS_AC6_COMPILER) && _IS_AC6_COMPILER
#define _IS_ACx_COMPILER 1
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x)  STRINGIFY_(x)

#if defined(_IS_AC6_COMPILER) && _IS_AC6_COMPILER
#define _STAI_COMPILER_DESC "MDK-ARM Keil (Arm Compiler 6) " STRINGIFY(__ARMCC_VERSION)
#define _STAI_COMPILER_ID   STAI_COMPILER_ID_KEIL
#elif defined(_IS_GHS_COMPILER) && _IS_GHS_COMPILER
#define _STAI_COMPILER_DESC "GHS-ARM Compiler " STRINGIFY(__GHS_VERSION_NUMBER)
#define _STAI_COMPILER_ID   STAI_COMPILER_ID_GHS
#elif defined(_IS_HTC_COMPILER) && _IS_HTC_COMPILER
#define _STAI_COMPILER_DESC                                                                                            \
  "HIGHTEC Clang " STRINGIFY(__clang_major__) "." STRINGIFY(__clang_minor__) "." STRINGIFY(__clang_patchlevel__)
#define _STAI_COMPILER_ID STAI_COMPILER_ID_HIGHTECH
#elif defined(_IS_GCC_COMPILER) && _IS_GCC_COMPILER
#define _STAI_COMPILER_DESC "GCC " STRINGIFY(__GNUC__) "." STRINGIFY(__GNUC_MINOR__) "." STRINGIFY(__GNUC_PATCHLEVEL__)
#define _STAI_COMPILER_ID   STAI_COMPILER_ID_GCC
#elif defined(_IS_IAR_COMPILER) && _IS_IAR_COMPILER
#define _STAI_COMPILER_DESC "IAR " STRINGIFY(__IAR_SYSTEMS_ICC__) " (build " STRINGIFY(__BUILD_NUMBER__) ")"
#define _STAI_COMPILER_ID   STAI_COMPILER_ID_IAR
#elif defined(_IS_AC5_COMPILER) && _IS_AC5_COMPILER
#define _STAI_COMPILER_DESC "MDK-ARM Keil " STRINGIFY(__ARMCC_VERSION)
#define _STAI_COMPILER_ID   STAI_COMPILER_ID_KEIL
#else
#define _STAI_COMPILER_DESC "Undefined"
#define _STAI_COMPILER_ID   STAI_COMPILER_ID_NONE
#endif

/* macro defining name of (platform specific) function called before ATON & RT initialization */
#ifndef LL_ATON_STAI_PRE_ATON_INIT_NAME
static inline int __ll_empty_stai_pre_aton_init(void)
{
  return 0;
}
#define __LL_ATON_STAI_PRE_ATON_INIT() __ll_empty_stai_pre_aton_init()
#else // LL_ATON_STAI_PRE_ATON_INIT_NAME
extern int LL_ATON_STAI_PRE_ATON_INIT_NAME(void);
#define __LL_ATON_STAI_PRE_ATON_INIT() LL_ATON_STAI_PRE_ATON_INIT_NAME()
#endif // LL_ATON_STAI_PRE_ATON_INIT_NAME

/* macro defining name of (platform specific) function called after ATON & RT de-initialization */
#ifndef LL_ATON_STAI_POST_ATON_DEINIT_NAME
static inline int __ll_empty_stai_post_aton_deinit(void)
{
  return 0;
}
#define __LL_ATON_STAI_POST_ATON_DEINIT() __ll_empty_stai_post_aton_deinit()
#else // LL_ATON_STAI_POST_ATON_DEINIT_NAME
extern int LL_ATON_STAI_POST_ATON_DEINIT_NAME(void);
#define __LL_ATON_STAI_POST_ATON_DEINIT() LL_ATON_STAI_POST_ATON_DEINIT_NAME()
#endif // LL_ATON_STAI_POST_ATON_DEINIT_NAME

/*****************************************************************************/
/** Runtime callback variables                                              **/
/*****************************************************************************/
static stai_event_cb _g_stai_aton_rt_callback = NULL;
static void *_g_stai_aton_rt_cb_cookie = NULL;

/*****************************************************************************/
/**  Helper function (for callbacks)                                        **/
/*****************************************************************************/
static void _stai_aton_internal_rt_callback(LL_ATON_RT_Callbacktype_t ctype)
{
  if (_g_stai_aton_rt_callback != NULL)
  {
    _g_stai_aton_rt_callback(_g_stai_aton_rt_cb_cookie, ctype, NULL);
  }
}

STAI_INTERNAL_ENTRY
void _stai_aton_internal_epoch_block_callback(LL_ATON_RT_Callbacktype_t ctype, const NN_Instance_TypeDef *nn_instance,
                                              const EpochBlock_ItemTypeDef *epoch_block)
{
  STAI_ASSERT(nn_instance != NULL);
  _stai_aton_context *stai_nn_ctx =
      (_stai_aton_context *)((void *)nn_instance) - offsetof(_stai_aton_context, network_instance);

  if (stai_nn_ctx->callback != NULL)
  {
    stai_nn_ctx->callback(stai_nn_ctx->callback_cookie, ctype, (const void *)epoch_block);
  }
}

/*****************************************************************************/
/**  Public ST.AI APIs                                                       **/
/*****************************************************************************/
STAI_API_ENTRY
stai_return_code stai_runtime_set_callback(const stai_event_cb cb, void *cb_cookie)
{
  _g_stai_aton_rt_callback = cb;
  _g_stai_aton_rt_cb_cookie = cb_cookie;

  LL_ATON_RT_SetRuntimeCallback((_g_stai_aton_rt_callback != NULL) ? _stai_aton_internal_rt_callback : NULL);

  return STAI_SUCCESS;
}

STAI_API_ENTRY
stai_return_code stai_runtime_init(void)
{
  /* Execute (eventual) pre init code */
  if (__LL_ATON_STAI_PRE_ATON_INIT() != 0)
  {
    return STAI_ERROR_STAI_INIT_FAILED;
  }

  /* Initialize ATON/N6 runtime */
  LL_ATON_RT_RuntimeInit();
  return STAI_SUCCESS;
}

STAI_API_ENTRY
stai_return_code stai_runtime_deinit(void)
{
  /* De-init ATON/N6 runtime */
  LL_ATON_RT_RuntimeDeInit();

  /* Execute (eventual) post de-init code */
  int err = __LL_ATON_STAI_POST_ATON_DEINIT();

  /* Reset callbacks */
  _g_stai_aton_rt_callback = NULL;
  _g_stai_aton_rt_cb_cookie = NULL;
  LL_ATON_RT_SetRuntimeCallback(NULL);

#ifdef STAI_ATON_ZYNQ
  // assuming zynq-based platform test harness & `stai_app_*.cpp` based application
  exit(err); // force exit status to test results check return value
#endif       // STAI_ATON_ZYNQ

  if (err != 0)
  {
    return STAI_ERROR_STAI_DEINIT_FAILED;
  }
  else
  {
    return STAI_SUCCESS;
  }
}

/* runtime build info bit shifts */
#define STAI_RT_PLAT_BSHIFT  0
#define STAI_RT_OSAL_BSHIFT  (STAI_RT_PLAT_BSHIFT + LL_ATON_CONFIG_PLAT_BSIZE)
#define STAI_RT_MODE_BSHIFT  (STAI_RT_OSAL_BSHIFT + LL_ATON_CONFIG_OSAL_BSIZE)
#define STAI_RT_BINFO_BSHIFT (STAI_RT_MODE_BSHIFT + LL_ATON_CONFIG_RT_MODE_BSIZE)
#define STAI_RT_CLKG_BSHIFT  (STAI_RT_BINFO_BSHIFT + LL_ATON_CONFIG_BINFO_BSIZE)
#define STAI_RT_SWF_BSHIFT   (STAI_RT_CLKG_BSHIFT + LL_ATON_CONFIG_CLKG_BSIZE)
#define STAI_RT_DD_BSHIFT    (STAI_RT_SWF_BSHIFT + LL_ATON_CONFIG_SWF_BSIZE)
#define STAI_RT_EBDBG_BSHIFT (STAI_RT_DD_BSHIFT + LL_ATON_CONFIG_DD_BSIZE)

/* runtime build info size masks */
#define STAI_RT_PLAT_MASK  ((1 << LL_ATON_CONFIG_PLAT_BSIZE) - 1)
#define STAI_RT_OSAL_MASK  ((1 << LL_ATON_CONFIG_OSAL_BSIZE) - 1)
#define STAI_RT_MODE_MASK  ((1 << LL_ATON_CONFIG_RT_MODE_BSIZE) - 1)
#define STAI_RT_BINFO_MASK ((1 << LL_ATON_CONFIG_BINFO_BSIZE) - 1)
#define STAI_RT_CLKG_MASK  ((1 << LL_ATON_CONFIG_CLKG_BSIZE) - 1)
#define STAI_RT_SWF_MASK   ((1 << LL_ATON_CONFIG_SWF_BSIZE) - 1)
#define STAI_RT_DD_MASK    ((1 << LL_ATON_CONFIG_DD_BSIZE) - 1)
#define STAI_RT_EBDBG_MASK ((1 << LL_ATON_CONFIG_EBDBG_BSIZE) - 1)

STAI_API_ENTRY
stai_return_code stai_runtime_get_info(stai_runtime_info *info)
{
  if (info)
  {
    static const stai_runtime_info _info = {
      .runtime_version = STAI_INIT_VERSION(LL_ATON_VERSION_MAJOR, LL_ATON_VERSION_MINOR, LL_ATON_VERSION_MICRO),
      .tools_version = STAI_INIT_VERSION(STAI_TOOLS_VERSION_MAJOR, STAI_TOOLS_VERSION_MINOR, STAI_TOOLS_VERSION_MICRO),
      .api_version = STAI_INIT_VERSION(STAI_API_VERSION_MAJOR, STAI_API_VERSION_MINOR, STAI_API_VERSION_MICRO),

      .runtime_build = (((LL_ATON_PLATFORM & STAI_RT_PLAT_MASK) << STAI_RT_PLAT_BSHIFT) |
                        ((LL_ATON_OSAL & STAI_RT_OSAL_MASK) << STAI_RT_OSAL_BSHIFT) |
                        ((LL_ATON_RT_MODE & STAI_RT_MODE_MASK) << STAI_RT_MODE_BSHIFT) |
                        ((LL_ATON_DBG_BUFFER_INFO_EXCLUDED & STAI_RT_BINFO_MASK) << STAI_RT_BINFO_BSHIFT) |
                        ((LL_ATON_ENABLE_CLOCK_GATING & STAI_RT_CLKG_MASK) << STAI_RT_CLKG_BSHIFT)
#if LL_ATON_SW_FALLBACK == 1
                        | (1 << STAI_RT_SWF_BSHIFT)
#endif // LL_ATON_SW_FALLBACK == 1
#ifdef LL_ATON_DUMP_DEBUG_API
                        | (1 << STAI_RT_DD_BSHIFT)
#endif // LL_ATON_DUMP_DEBUG_API
#ifdef LL_ATON_EB_DBG_INFO
                        | (1 << STAI_RT_EBDBG_BSHIFT)
#endif // LL_ATON_EB_DBG_INFO
                            ),

      .compiler_id = _STAI_COMPILER_ID,
      .compiler_desc = _STAI_COMPILER_DESC,
    };

    *info = _info;
    return STAI_SUCCESS;
  }

  return STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS;
}

STAI_API_ENTRY
stai_return_code stai_cache_mcu_clean_range(uintptr_t virtual_addr, stai_size size)
{
  LL_ATON_Cache_MCU_Clean_Range(virtual_addr, size);
  return STAI_SUCCESS;
}

STAI_API_ENTRY
stai_return_code stai_cache_mcu_invalidate_range(uintptr_t virtual_addr, stai_size size)
{
  LL_ATON_Cache_MCU_Invalidate_Range(virtual_addr, size);
  return STAI_SUCCESS;
}

STAI_API_ENTRY
stai_return_code stai_ext_cache_npu_clean_range(uintptr_t virtual_addr, stai_size size)
{
  LL_ATON_Cache_NPU_Clean_Range(virtual_addr, size);
  return STAI_SUCCESS;
}

STAI_API_ENTRY
stai_return_code stai_ext_cache_npu_clean_invalidate_range(uintptr_t virtual_addr, stai_size size)
{
  /* NOTE: The ATON NPU cache does not provide a pure invalidate-range function, but only a clean-invalidate range
     function! One has to take this into account when using `stai_ext_cache_npu_clean_invalidate_range`. */
  LL_ATON_Cache_NPU_Clean_Invalidate_Range(virtual_addr, size);
  return STAI_SUCCESS;
}

STAI_API_ENTRY
stai_return_code stai_ext_wfe(void)
{
  LL_ATON_OSAL_WFE();
  return STAI_SUCCESS;
}

/*****************************************************************************/
/**  Internal stai APIs                                                     **/
/*****************************************************************************/
STAI_INTERNAL_ENTRY
void __ll_aton_stai_init_network_instance(_stai_aton_context *nn_context)
{
  STAI_ASSERT((nn_context != NULL) && (nn_context->network_instance.network != NULL)); // may never happen

  NN_Instance_TypeDef *nn_instance_ptr = &nn_context->network_instance;
  LL_ATON_RT_Init_Network(nn_instance_ptr);
}

STAI_INTERNAL_ENTRY
void __ll_aton_stai_deinit_network_instance(_stai_aton_context *nn_context)
{
  STAI_ASSERT((nn_context != NULL) && (nn_context->network_instance.network != NULL)); // may never happen

  NN_Instance_TypeDef *nn_instance_ptr = &nn_context->network_instance;
  LL_ATON_RT_DeInit_Network(nn_instance_ptr);
}

STAI_INTERNAL_ENTRY
void __ll_aton_stai_set_execution_status(LL_ATON_RT_RetValues_t status, _stai_aton_context *nn_context)
{
  STAI_ASSERT(nn_context != NULL);

  switch (status)
  {
  case LL_ATON_RT_NO_WFE:
    nn_context->exec_status = STAI_RUNNING_NO_WFE;
    break;
  case LL_ATON_RT_WFE:
    nn_context->exec_status = STAI_RUNNING_WFE;
    break;
  case LL_ATON_RT_DONE:
    nn_context->exec_status = STAI_DONE;
    break;
  default:
    STAI_ASSERT(false);
    break;
  }
}

STAI_INTERNAL_ENTRY
stai_return_code __ll_aton_stai_get_execution_status(_stai_aton_context *nn_context)
{
  STAI_ASSERT(nn_context != NULL);
  return nn_context->exec_status;
}

STAI_INTERNAL_ENTRY
static void __ll_aton_stai_run_synchonously(NN_Instance_TypeDef *nn_instance)
{
  STAI_ASSERT(nn_instance != NULL);
  STAI_ASSERT(nn_instance->exec_state.current_epoch_block != NULL);
  _stai_aton_context *stai_nn_ctx =
      (_stai_aton_context *)((void *)nn_instance) - offsetof(_stai_aton_context, network_instance);

  LL_ATON_RT_RetValues_t ll_aton_rt_ret;

  do
  {
    /* Execute first/next step of ATON runtime */
    ll_aton_rt_ret = LL_ATON_RT_RunEpochBlock(nn_instance);
    __ll_aton_stai_set_execution_status(ll_aton_rt_ret, stai_nn_ctx);

    /* Wait for next event */
    if (ll_aton_rt_ret == LL_ATON_RT_WFE)
    {
      LL_ATON_OSAL_WFE();
    }
  } while (ll_aton_rt_ret != LL_ATON_RT_DONE);
}

STAI_INTERNAL_ENTRY
stai_return_code __ll_aton_stai_reset(stai_network *network)
{
  /* de-init network */
  __LL_ATON_STAI_ACQUIRE_INTERFACE(nn_i_ptr, network);
  LL_ATON_LIB_UNUSED(nn_i_ptr);
  _stai_aton_context *ctx = (_stai_aton_context *)network;

  if ((ctx->exec_status != STAI_DONE) && (ctx->exec_status != STAI_RUNNING_NO_WFE))
  {
    __LL_ATON_STAI_SET_1ST_CTX_ERROR_AND_RETURN(ctx, STAI_ERROR_NETWORK_STILL_RUNNING);
  }

  /* reset network */
  ctx->exec_status = STAI_SUCCESS;
  LL_ATON_RT_Reset_Network(&ctx->network_instance);

  return STAI_SUCCESS;
}

STAI_INTERNAL_ENTRY
stai_return_code __ll_aton_stai_run(stai_network *network, const stai_run_mode mode)
{
  /* check current network */
  STAI_ASSERT(network != NULL);
  _stai_aton_context *nn_context = (_stai_aton_context *)network;

  if (nn_context->network_instance.exec_state.current_epoch_block == NULL)
  {
    __LL_ATON_STAI_SET_1ST_CTX_ERROR_AND_RETURN(nn_context, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS);
  }

  /* reset first error */
  nn_context->first_error = STAI_SUCCESS;

  /* enable/disable tracing */
  if (nn_context->callback != NULL)
  {
    LL_ATON_RT_SetNetworkCallback(&nn_context->network_instance, _stai_aton_internal_epoch_block_callback);
  }
  else
  {
    LL_ATON_RT_SetNetworkCallback(&nn_context->network_instance, NULL);
  }

  /* run/start network execution */
  switch (mode)
  {
  case STAI_MODE_SYNC:
    __ll_aton_stai_run_synchonously(&nn_context->network_instance); // run current network

    /* reset network for new inference */
    {
      LL_ATON_ASSERT(nn_context->exec_status == STAI_DONE);
      stai_return_code ret = __ll_aton_stai_reset(network);
      LL_ATON_ASSERT(ret == STAI_SUCCESS);
      return ret;
    }
    break;
  case STAI_MODE_ASYNC:
    __LL_ATON_START_CONT_EXEC(&nn_context->network_instance); // kick-off execution
    break;
  default:
    __LL_ATON_STAI_SET_1ST_CTX_ERROR_AND_RETURN(nn_context, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS);
  }
}
