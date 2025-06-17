/**
  ******************************************************************************
  * @file    layers_dense_dqnn.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of deeply quantized dense layers.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_DENSE_DQNN_H
#define LAYERS_DENSE_DQNN_H

#include "layers_common.h"

/*!
 * @defgroup layers_dense_dqnn Quantized Dense Layers definition.
 * @brief Implements the kernels and the forward functions to implement
 * dense layers with quantized inputs, weights, or outputs.
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_dense_dqnn
 * @ingroup layers_dense_dqnn
 * @brief Specific instance of deeply quantized dense layers.
 */
typedef ai_layer_base ai_layer_dense_dqnn;

/*****************************************************************************/
/*  Forward Functions Section                                                */
/*****************************************************************************/

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed binary output, and signed binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os1ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * signed binary output, and signed binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os1ws1_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 8-bit signed output, and signed binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os8ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 8-bit signed output, and signed binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os16ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 32-bit floating point output, and signed binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1of32ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 32-bit floating point output, and signed binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1of32ws1_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 32-bit floating point output, and 32-bit floating point weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1of32wf32(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 32-bit floating point output, and 32-bit floating point weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1of32wf32_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 32-bit floating point output, and 8-bit signed weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1of32ws8(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 32-bit floating point output, and 8-bit signed weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1of32ws8_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * binary output, and 8-bit signed weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os1ws8(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * binary output, and 8-bit signed weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os1ws8_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 8-bit signed output, and 8-bit signed weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os8ws8(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed binary input,
 * 16-bit signed output, and 8-bit signed weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is1os16ws8(ai_layer* layer);


/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * float output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8of32ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * float output, and binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8of32ws1_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * 1-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8os1ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * 1-bit signed output, and binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8os1ws1_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * binary weights and binary output.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8os1ws1_bn_fxp(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * 8-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8os8ws1(ai_layer* layer);


/*!
 * @brief Forward function for a dense layer with signed 8-bit input,
 * 16-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is8os16ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 16-bit input,
 * 1-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is16os1ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 16-bit input,
 * 1-bit signed output, and binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is16os1ws1_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 16-bit input,
 * 8-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is16os8ws1(ai_layer* layer);


/*!
 * @brief Forward function for a dense layer with signed 16-bit input,
 * 16-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is16os16ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed 16-bit input,
 * f32 output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is16of32ws1(ai_layer* layer);


/*!
 * @brief Forward function for a dense layer with signed 16-bit input,
 * f32 output, and binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_is16of32ws1_bn(ai_layer* layer);


/*!
 * @brief Forward function for a dense layer with signed f32 input,
 * 1-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_if32os1ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed f32 input,
 * 1-bit signed output, and binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_if32os1ws1_bn(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed f32 input,
 * 8-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_if32os8ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed f32 input,
 * 16-bit signed output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_if32os16ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed f32 input,
 * f32 output, and binary weights.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_if32of32ws1(ai_layer* layer);

/*!
 * @brief Forward function for a dense layer with signed f32 input,
 * f32 output, and binary weights.
 * The BN is fused, i.e., the layer requires weights, scale, and offset, where
 * weights are those of the dense layer, scale is that of the BN, and the offset
 * corresponds to dense bias * bn scale + bn offset. If the parameters do not
 * agree with such convention, the behavior is undefined.
 * @ingroup layers_dense_dqnn
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_dense_if32of32ws1_bn(ai_layer* layer);

AI_API_DECLARE_END

#endif    /*LAYERS_DENSE_DQNN_H*/
