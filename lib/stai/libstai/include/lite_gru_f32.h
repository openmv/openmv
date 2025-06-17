/**
  ******************************************************************************
  * @file    lite_gru_f32.h
  * @author  AIS
  * @brief   header file of AI platform lite gru kernel datatypes
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
#ifndef LITE_GRU_F32_H
#define LITE_GRU_F32_H


#include "ai_lite_interface.h"

/*!
 * @brief Forward function for a stateless GRU (gate recurrent unit) layer with
 * signed float input, signed float output, and float parameters.
 * @ingroup lite_gru_f32
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param gru_kernel The pointer to gru kernel param.
 * @param gru_recurrent The pointer to gru recurrent param.
 * @param gru_bias The pointer to bias.
 * @param gru_scratch The pointer to GRU scratch.
 * @param n_units The number of GRU cells (dimensionality of output space).
 * @param n_timesteps The number of timesteps of the input sequence.
 * @param n_features The number of features of the input sequence.
 * @param activation_nl The activation function used to update memory state.
 * @param recurrent_nl The activation function to use for the recurrent step.
 * @param return_seq If True, returns the full output sequence, else only the last output.
 * @param go_backwards If True, process the input sequence backwards.
 * @param reverse_seq If True, reverse the input sequence
 * @param reset_after Whether to apply reset gate after (True) or before (False) matmul.
 * @param activation_param The parameters for activation_nl (can be NULL)
 * @param recurrent_param The parameters for recurrent_nl (can be NULL)
 * @param initial_hidden Initial state of hidden layer (can be NULL)
 */
LITE_API_ENTRY
void forward_lite_gru_if32of32wf32(
  ai_float* output, const ai_float* input, const ai_float* gru_kernel,
  const ai_float* gru_recurrent, const ai_float* gru_bias, ai_float* gru_scratch,
  const ai_u32 n_units, const ai_size n_timesteps, const ai_size n_features,
  ai_handle activation_nl, ai_handle recurrent_nl, ai_bool return_seq,
  ai_bool go_backwards, ai_bool reverse_seq, ai_bool reset_after,
  const ai_float* activation_param, const ai_float* recurrent_param,
  const ai_float* initial_hidden);


#endif    /* LITE_GRU_F32_H */
