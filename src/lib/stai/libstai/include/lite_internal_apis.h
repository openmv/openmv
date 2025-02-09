/**
  ******************************************************************************
  * @file    lite_internal_apis.h
  * @author  STMicroelectronics
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LITE_INTERNAL_APIS
#define LITE_INTERNAL_APIS

#include "ai_platform.h"
#include "ai_lite_interface.h"

/* lite_nl_generic_float */
#define LITE_NL_ENTRY(nl_id_, nl_name_, nl_op_, nl_op_args_) \
/** \
 * @brief lite function for a templated non-linearity nl_op_. \
 * @ingroup lite_nl_generic_float \
 * @param out_ptr The pointer to output buffer. \
 * @param in_ptr The pointer to input buffer. \
 * @param in_size. The size of the input. \
 * @param params opaque handler to optional NL params (not used). \
 */ \
LITE_API_ENTRY \
void forward_lite_nl_ ## nl_name_ ## _if32of32( \
  ai_handle out_ptr, const ai_handle in_ptr, const ai_i32 in_size, const ai_handle params);

#include "lite_nl_list.h"

/**
 * @brief lite function for a float softmax non-linearity where the softmax is applied per channel.
 * @ingroup lite_nl_generic_float
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param in_size The size of the input.
 * @param inner_loop_cnt The size of the inner loop (elements after the selected axis)
 * @param axis_elem The elements number along the selected axis.
 */
LITE_API_ENTRY
void forward_lite_nl_softmax_if32of32(
  ai_handle out_ptr, const ai_handle in_ptr,
  const ai_size in_size, const ai_size inner_loop_cnt, const ai_size axis_elem);


/**
 * @brief lite function for a float softmax zero channel non-linearity where the softmax is applied per channel.
 * @ingroup lite_nl_generic_float
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param in_size. The size of the input.
 * @param channel_size The nsize of each channel.
 * @param in_channel_step
 * @param out_channel_step
 */
LITE_API_ENTRY
void forward_lite_nl_softmax_zero_channel_if32of32(
  ai_handle out_ptr, const ai_handle in_ptr, const ai_i32 in_size, const ai_size ch_size,
  const ai_i32 in_ch_step, const ai_i32 out_ch_step);

/*!
 * @typedef (*func_nl_lite)
 * @ingroup layers_nl
 * @brief Fuction pointer for generic non linear transform
 * this function pointer abstracts a generic non linear layer.
 * see @ref nl_func_tanh_array_f32 and similar as examples.
 */
  typedef void (*func_nl_lite)(ai_handle out_ptr, const ai_handle in_ptr,
                               const ai_i32 in_size, const ai_handle params);

/**
 * @brief lite function for a float gelu non-linearity.
 * @ingroup lite_nl_generic_float \
 * @param out_ptr The pointer to output buffer. \
 * @param in_ptr The pointer to input buffer. \
 * @param in_size. The size of the input. \
 * @param params opaque handler to optional NL params. \
 */
LITE_API_ENTRY
void forward_lite_nl_gelu_if32of32(
  ai_handle out_ptr, const ai_handle in_ptr, const ai_i32 in_size, const ai_handle params);

#endif /* LITE_INTERNAL_APIS */
