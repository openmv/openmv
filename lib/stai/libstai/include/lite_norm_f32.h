/**
  ******************************************************************************
  * @file    lite_norm_f32.h
  * @author  AIS
  * @brief   header file of AI platform norm in lite mode
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
#ifndef LITE_NORM_F32_H
#define LITE_NORM_F32_H
#pragma once

#include "ai_lite_interface.h"

enum ai_lite_norm_type_ {
  AI_LITE_NORM_NONE = 0,
  AI_LITE_NORM_L1 = 1,
  AI_LITE_NORM_L2 = 2,
  AI_LITE_NORM_MAX = 3,
};

/*!
 * @brief Forward function for a batch normalization (BN) layer with
 * signed float input, signed float output, and float parameters.
 * @ingroup lite_norm_f32
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param scale The pointer to BN scale param.
 * @param bias The pointer to bias.
 * @param n_elements The number of elements in the input tensor.
 * @param n_channel_in The number of channel in the input tensor.
 */
LITE_API_ENTRY
void forward_lite_norm_if32of32( ai_float* output,
                                 const ai_float* input,
                                 const ai_u32 ai_lite_norm_type,
                                 const ai_float exponent,
                                 const ai_size n_axis,
                                 const ai_size n_axis_stride,
                                 const ai_size n_el,
                                 ai_bool scale);

#endif    /* LITE_NORM_F32_H */
