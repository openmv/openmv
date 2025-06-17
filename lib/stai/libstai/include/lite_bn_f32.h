/**
  ******************************************************************************
  * @file    lite_bnf32.h
  * @author  AIS
  * @brief   header file of AI platform lite batch normalization functions
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
#ifndef LITE_BN_F32_H
#define LITE_BN_F32_H


#include "ai_lite_interface.h"

/*!
 * @brief Forward function for a batch normalization (BN) layer with
 * signed float input, signed float output, and float parameters.
 * @ingroup lite_bn_f32
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param scale The pointer to BN scale param.
 * @param bias The pointer to bias.
 * @param n_elements The number of elements in the input tensor.
 * @param n_channel_in The number of channel in the input tensor.
 */
LITE_API_ENTRY
void forward_lite_bn_if32of32wf32(
  ai_float* output, const ai_float* input,
  const ai_float* scale, const ai_float* bias,
  const ai_u32 n_elements, const ai_u32 n_channel_in);


#endif    /* LITE_BN_F32_H */
