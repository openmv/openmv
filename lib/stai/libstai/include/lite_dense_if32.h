/**
  ******************************************************************************
  * @file    lite_dense_if32.h
  * @author  STMicroelectronics
  * @brief   Definitions of runtime-lite dense core kernels (with float f32 input)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LITE_DENSE_IF32_H
#define LITE_DENSE_IF32_H

#include "ai_lite_interface.h"

/*!
 * @brief decompress the weights into a scratch buffer

 * @ingroup lite_dense_if32
 * @param out pointer to the scratch buffer data
 * @param lut pointer to the compression dictionary
 * @param lut_bits bits used for compression (only 4 or 8 supported)
 * @param n_in if last dimension is not even, specify its size for padding
 * @param n_out: number of elements to be decompressed
 */
LITE_API_ENTRY
const uint8_t* lite_decompress_ilutof32(
  float* out, const uint8_t* data0,
  const float* lut, const uint16_t lut_bits,
  const ai_size n_in, const ai_size n_out);


/*!
 * @brief C struct for a dense layer with signed float input, signed float output, and float weights.
 * @ingroup lite_dense_if32
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param bias The pointer to bias (NULL if not available).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e., the number of dense hidden neurons.
 */
typedef struct {
  ai_float*           output;
  const ai_float*     input;
  const ai_float*     weights;
  const ai_float*     bias;
  const ai_size       n_channel_in;
  const ai_size       n_channel_out;
  const ai_size       n_elements;
} forward_lite_dense_if32of32wf32_args;


/*!
 * @brief Forward function for a dense layer with signed float input,
 * signed float output, and float weights.
 * @ingroup lite_dense_if32
 * @param args pointer to @ref forward_lite_dense_if32of32wf32_args structure
 */
LITE_API_ENTRY
void forward_lite_dense_if32of32wf32(
  forward_lite_dense_if32of32wf32_args* args);


/*!
 * @brief Forward function for a dense layer with signed float input,
 * signed float output, and 4bit LUT compressed weights.
 * @ingroup lite_dense_if32
 * @param output The pointer to output buffer.
 * @param weights_lut The pointer to compressed weights LUT table (16 entries).
 * @param input The pointer to input buffer.
 * @param weights_indeces The pointer to compressed weights indeces table (packed 4bits buffer).
 * @param scratch_lut The pointer to cache buffer where to prefetch weights_lut values. (optional)
 * @param bias The pointer to bias (NULL if not available).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e., the number of dense hidden neurons.
 * @param n_elements The number of elements to process
 */
LITE_API_ENTRY
void forward_lite_dense_if32of32wf32_lut4(
  ai_float* output, const ai_float* input,
  const ai_u8* weights_indeces, const ai_float* weights_lut, ai_float* scratch_lut,
  const ai_float* bias,
  const ai_size n_channel_in, const ai_size n_channel_out,
  const ai_size n_elements);


/*!
 * @brief Forward function for a dense layer with signed float input,
 * signed float output, and 8bit LUT compressed weights.
 * @ingroup lite_dense_if32
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights_indeces The pointer to compressed weights indeces table (8bits buffer).
 * @param weights_lut The pointer to compressed weights LUT table (256 entries).
 * @param scratch_lut The pointer to cache buffer where to prefetch weights_lut values. (optional)
 * @param bias The pointer to bias (NULL if not available).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e., the number of dense hidden neurons.
 * @param n_elements The number of elements to process
 */
LITE_API_ENTRY
void forward_lite_dense_if32of32wf32_lut8(
  ai_float* output, const ai_float* input,
  const ai_u8* weights_indeces, const ai_float* weights_lut, ai_float* scratch_lut,
  const ai_float* bias,
  const ai_size n_channel_in, const ai_size n_channel_out,
  const ai_size n_elements);


#endif    /* LITE_DENSE_IF32_H */
