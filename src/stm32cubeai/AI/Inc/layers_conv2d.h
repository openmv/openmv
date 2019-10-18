/**
  ******************************************************************************
  * @file    layers_conv2d.h
  * @author  AST Embedded Analytics Research Platform
  * @date    18-Apr-2018
  * @brief   header file of AI platform conv2d layers datatypes
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __LAYERS_CONV2D_H_
#define __LAYERS_CONV2D_H_
#pragma once

#include "layers_nl.h"
#include "layers_pool.h"

#define AI_LAYER_CONV2D_FIELDS_DECLARE \
  AI_LAYER_COMMON_FIELDS_DECLARE \
  ai_u32      groups;            /*!< groups for separable convolution */ \
  AI_CONST ai_array*  nl_params; /*!< array pointer to non linear parameters */ \
  func_nl     nl_func;           /*!< function pointer to non linear transform */ \
  ai_shape_2d filter_stride;     /*!< filter stride, how much the filter moves */ \
  ai_shape_2d dilation;          /*!< dilation value along axis of the filter */ \
  ai_shape    filter_pad;        /*!< filter pad 4d */

/*!
 * @defgroup layers_conv2d Convolutive Layers Definitions
 * @brief definition 
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_dense
 * @ingroup layers_conv2d
 * @brief Dense (fully connected) layer
 */
typedef ai_layer ai_layer_dense;

/*!
 * @struct ai_layer_gemm
 * @ingroup layers_conv2d
 * @brief layer for General Matrix Multiplication
 *
 * Layer for General Matrix Multiplication (GEMM):
 * \f{equation}{ Y = \alpha A \cdot B + \beta C \f}
 * \f$\alpha\f$ and \f$\beta\f$ are paramaters, A and B are matrices,
 * C is a matrix or an array. Size checks for A, B, C, and Y are performed and
 * broadcast is applied on C if necessary.
 * This is a sequential layer (see @ref ai_layer).
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_gemm_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_float alpha;    /*!< alpha coefficient */
  ai_float beta;     /*!< beta coefficient */
  ai_u8 tA;          /*!< transpose A flag */
  ai_u8 tB;          /*!< transpose B flag */
} ai_layer_gemm;

/*!
 * @struct ai_layer_conv2d
 * @ingroup layers_conv2d
 * @brief 2D convolutional layer with strides and pads
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_conv2d_ {
  AI_LAYER_CONV2D_FIELDS_DECLARE
} ai_layer_conv2d;

/*!
 * @struct ai_layer_conv2d_nl_pool
 * @ingroup layers_conv2d
 * @brief 2D convolutional layer + nl + pooling with strides and pads
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_conv2d_nl_pool_ {
  AI_LAYER_CONV2D_FIELDS_DECLARE

  ai_shape_2d pool_size;    /*!< pooling size */
  ai_shape_2d pool_stride;  /*!< pooling stride */
  ai_shape    pool_pad;     /*!< pooling pad */

  func_pool pool_func;      /*!< function pointer to pooling transform */
} ai_layer_conv2d_nl_pool;


AI_INTERNAL_API
void ai_dict8_dot_array_f32(ai_handle out, ai_ptr_const data0, ai_ptr_const lut,
                            const ai_float* data1, const ai_size data_size);

AI_INTERNAL_API
void ai_dict4_dot_array_f32(ai_handle out, ai_ptr_const data0, ai_ptr_const lut,
                            const ai_float* data1, const ai_size data_size);/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Computes the activations of a 2D convolutional layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d(ai_layer* layer);

/*!
 * @brief Computes the activations of a @ref ai_layer_conv2d_nl_pool layer
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max, softmax)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool(ai_layer* layer);

/*!
 * @brief Computes the activations of a GEMM layer.
 * @ingroup layers
 * @param layer the layer including output and input tensors
 */
AI_INTERNAL_API
void forward_gemm(ai_layer* layer);

/*!
 * @brief Computes matmul layer, intended as numpy.matmul(A,B).
 * @ingroup layers
 * @param layer the layer including output and input tensors
 */
AI_INTERNAL_API
void forward_matmul(ai_layer* layer);

/*!
 * @brief Computes the activations of a dense (fully connected) layer.
 * @ingroup layers_conv2d
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense(ai_layer* layer);


/*!
 * @brief Computes the activations of a fixed point 2D convolutional layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a fixed point @ref ai_layer_conv2d_nl_pool 
 * layer.
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer.
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer.
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*__LAYERS_CONV2D_H_*/
