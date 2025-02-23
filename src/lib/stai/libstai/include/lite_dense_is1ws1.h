/**
  ******************************************************************************
  * @file    lite_dense_is1ws1.h
  * @author  AIS
  * @brief   header file of AI platform lite dense kernel datatypes
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
#ifndef LITE_DENSE_IS1WS1_H
#define LITE_DENSE_IS1WS1_H

#include "stai.h"
#include "ai_lite_interface.h"

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed binary output, and signed binary weights.
 * @ingroup lite_dense_is1ws1
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param bias The pointer to bias (NULL if not available).
 * @param scratch The pointer to the scratch buffer.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_ouy The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 */
LITE_API_ENTRY
void forward_lite_dense_is1os1ws1(
  ai_pbits *output, const ai_pbits *input, const ai_pbits *weights,
  const ai_pbits *bias, ai_i32 *scratch,
  const ai_u32 n_channel_in, const ai_u32 n_channel_out
);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed binary output, and signed binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup lite_dense_is1ws1
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param scale The pointer to scale.
 * @param offset The pointer to offset.
 * @param scratch The pointer to the scratch buffer.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_ouy The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 */
LITE_API_ENTRY
void forward_lite_dense_is1os1ws1_bn(
  ai_pbits *output, const ai_pbits *input, const ai_pbits *weights,
  const ai_float *scale, const ai_float *offset, ai_i32 *scratch,
  const ai_u32 n_channel_in, const ai_u32 n_channel_out
);


/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed binary output, and signed 16bit weights.
 * @ingroup lite_dense_is1ws1
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param bias The pointer to bias (NULL if not available).
 * @param scratch The pointer to the scratch buffer (signed 32bit).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_ouy The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 */
LITE_API_ENTRY
void forward_lite_dense_is1os16ws1(
  ai_i16 *output, const ai_pbits *input, const ai_pbits *weights,
  const ai_pbits *bias, ai_i32 *scratch,
  const ai_u32 n_channel_in, const ai_u32 n_channel_out);


/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed binary output, and signed 16bit weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup lite_dense_is1ws1
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param bias The pointer to bias (NULL if not available).
 * @param scratch The pointer to the scratch buffer (signed 32bit).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_ouy The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 */
LITE_API_ENTRY
void forward_lite_dense_is1os16ws1_bn(
  ai_i16 *output, const ai_pbits *input, const ai_pbits *weights,
  const ai_float *scale, const ai_float *offset, ai_i32 *scratch,
  const ai_u32 n_channel_in, const ai_u32 n_channel_out);


/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed float output, and signed binary weights.
 * @ingroup lite_dense_is1ws1
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param bias The pointer to bias (NULL if not available).
 * @param scratch The pointer to the scratch buffer (unused).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_ouy The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 */
LITE_API_ENTRY
void forward_lite_dense_is1of32ws1(
  ai_float *output, const ai_pbits *input, const ai_pbits *weights,
  const ai_pbits *bias, ai_i32 *scratch,
  const ai_u32 n_channel_in, const ai_u32 n_channel_out
);


/*!
 * @brief C struct for a dense layer with signed binary input,
 * signed float output, and signed binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup lite_dense_is1ws1
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param scale The pointer to scale.
 * @param offset The pointer to offset.
 * @param scratch The pointer to the scratch buffer (unused).
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 */
typedef struct {
  float*              output;
  const stai_pbits*   input;
  const stai_pbits*   weights;
  const float*        scale;
  const float*        offset;
  int32_t*            scratch;
  const uint32_t      n_channel_in;
  const uint32_t      n_channel_out;
} forward_lite_dense_is1of32ws1_bn_args;


LITE_API_ENTRY
void forward_lite_dense_is1of32ws1_bn(
  forward_lite_dense_is1of32ws1_bn_args* args);


#endif    /*LITE_DENSE_IS1WS1_H*/
