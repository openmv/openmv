/**
 ******************************************************************************
 * @file    stai_aton_common.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Implementation of ST.AI platform public APIs for ATON
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

#ifndef __STAI_ATON_COMMON_H
#define __STAI_ATON_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stai.h"
#include "stai_debug.h"

#include "ll_aton_NN_interface.h"

  /*****************************************************************************/
  /** Private context: DO NOT EDIT NOR access directly                        **/
  /*****************************************************************************/
  typedef struct
  {
    NN_Instance_TypeDef network_instance;
    stai_event_cb callback;
    void *callback_cookie;
    stai_return_code exec_status; // (asynchronous) execution status
    stai_return_code first_error; // 1st generated error
  } _stai_aton_context;

/*****************************************************************************/
/** Cache line size alignment helper macro                                  **/
/*****************************************************************************/
#ifndef STAI_CACHE_IS_LINESIZE_ALIGNED
// NOTE: `linesize` must be a power of two
#define STAI_CACHE_IS_LINESIZE_ALIGNED(value, linesize) (((uintptr_t)value & ((uintptr_t)linesize - 1)) == 0)
#endif // STAI_CACHE_IS_LINESIZE_ALIGNED

#ifdef __cplusplus
}
#endif

#endif // __STAI_ATON_COMMON_H
