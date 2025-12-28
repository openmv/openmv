/**
  ******************************************************************************
  * @file    layers_conv2d.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform conv2d layers datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_CONV2D_H
#define LAYERS_CONV2D_H

#include "layers_nl.h"
#include "layers_pool.h"




#define AI_LAYER_CONV2D_FIELDS_DECLARE \
  AI_LAYER_COMMON_FIELDS_DECLARE \
  ai_u32      groups;                   /*!< groups for separable convolution */ \
  AI_CONST ai_array*  nl_params;        /*!< array pointer to non linear parameters */ \
  ai_handle   nl_func;                  /*!< function pointer to non linear transform */ \
  ai_shape_2d filter_stride;            /*!< filter stride, how much the filter moves */ \
  ai_shape_2d dilation;                 /*!< dilation value along axis of the filter */ \
  ai_shape    filter_pad;               /*!< filter pad 4d */ \
  ai_layer_format_type in_ch_format;    /*!< Input format  (Channel 1st vs Channel last */ \
  ai_layer_format_type out_ch_format;   /*!< Output format (Channel 1st vs Channel last */



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
typedef ai_layer_base ai_layer_dense;

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
 * @struct ai_layer_matmul
 * @ingroup layers_conv2d
 * @brief layer for General Matrix Multiplication
 *
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_matmul_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_float alpha;    /*!< alpha coefficient */
  ai_float beta;     /*!< beta coefficient */
  ai_u8 tA;          /*!< transpose A flag */
  ai_u8 tB;          /*!< transpose B flag */
} ai_layer_matmul;

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

  ai_handle pool_func;      /*!< function pointer to pooling transform */
} ai_layer_conv2d_nl_pool;


/*
AI_INTERNAL_API
void ai_dict8_dot_array_f32(ai_handle out, ai_ptr_const data0, ai_ptr_const lut,
                            const ai_float* data1, const ai_size data_size);

AI_INTERNAL_API
void ai_dict4_dot_array_f32(ai_handle out, ai_ptr_const data0, ai_ptr_const lut,
                            const ai_float* data1, const ai_size data_size);
*/


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Computes the activations of a floating point 32 2D convolutional layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_if32of32wf32(ai_layer* layer);

/*!
 * @brief Computes the activations of a floating point 32 2D dw layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_if32of32wf32(ai_layer* layer);

/*!
 * @brief Computes the activations of a floating point 32 2D convolutional group layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_if32of32wf32_group(ai_layer* layer);

/*!
 * @brief Computes the activations of a 2D floating point 32 pool fused convolutional layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_if32of32wf32_nl_pool(ai_layer* layer);

/*!
 * @brief Computes the activations of a 2D floating point 32 pool fused dw layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_if32of32wf32_nl_pool(ai_layer* layer);

/*!
 * @brief Computes the activations of a 2D floating point 32 pool fused convolutional group layer.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_if32of32wf32_group_nl_pool(ai_layer* layer);

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
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for SSSA per layer quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_integer_SSSA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for SSSA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is8os8ws8_sssa_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme Optimized for HSP
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_hsp_1step_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme Optimized for HSP
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_hsp_2step_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme Optimized for HSP
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_hsp_3step_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme, with 3x3 kernels
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_3x3_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme, with 1xN kernels
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_1xN_sssa8_ch(ai_layer *pLayer);



/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme, with 3x3 kernels and input are
 *        channel first
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_3x3_ch1st_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme with depth multiplier > 1
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_dm_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of int8 quantized DW layers.
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_all_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized PW layer
 *        for SSSA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_pw_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized PW layer
 *        for SSSA per channel quantized scheme. Optimized for HSP
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_pw_hsp_2step_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized PW layer
 *        for SSSA per channel quantized scheme. Optimized for HSP
 *        1Step version (nb input channel <= 4)
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_pw_hsp_1step_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized PW layer
 *        for SSSA per channel quantized scheme. Optimized for HSP
 *        3 Step variant
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_pw_hsp_3step_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid padding)
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_dilated_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid padding)
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_deep_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid padding)
 *        number of output channel is greater than 8
 *        Kernels shall be 3x3 and stride is (1,1)
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_deep_3x3_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid or same padding)
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid or same padding)
 *        Used for configuration supported by HSP and if HSP is available
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_hsp_1step_sssa8_ch(ai_layer *pLayer);

AI_INTERNAL_API
void forward_conv2d_hsp_2step_sssa8_ch(ai_layer *pLayer);

AI_INTERNAL_API
void forward_conv2d_hsp_3step_sssa8_ch(ai_layer *pLayer);


/*!
 * @brief Computes the activations of a int8 quantized Conv2d layer
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_all_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized RGB Conv2d layer
 *        for SSSA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_rgb_sssa8_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme with pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme, with 3x3 kernels,
 *        with pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_3x3_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme, with 3x3 kernels,
 *        with pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_3x3_ch1st_sssa8_ch_nl_pool(ai_layer *pLayer);


/*!
 * @brief Computes the activations of a int8 quantized DW layer
 *        for SSSA per channel quantized scheme with depth multiplier > 1
 *        with pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_dm_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of int8 quantized DW layers, with pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_all_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized PW layer,
 *        with pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_pw_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid padding) and  pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_dilated_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid padding) and  pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_deep_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid padding) and  pooling fused
 *        number of output channel is greater than 8
 *        Kernels shall be 3x3 and stride is (1,1)
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_deep_3x3_sssa8_ch_nl_pool(ai_layer *pLayer);


/*!
 * @brief Computes the activations of a int8 quantized non dilated Conv2d layer
 *        for SSSA per channel quantized scheme (valid or same padding) and  pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a int8 quantized Conv2d layer and  pooling fused
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_all_sssa8_ch_nl_pool(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for SSUA per layer quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_integer_SSUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for SSUA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_integer_SSUA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for UAUA per layer quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_integer_UAUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for UAUA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_integer_UAUA_ch(ai_layer *pLayer);

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
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer
 *        for SSSA per layer quantized scheme
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer_SSSA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer
 *        for SSSA per channel quantized scheme
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer_SSSA_ch(ai_layer *pLayer);


/*!
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer
 *        for SSUA per layer quantized scheme
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer_SSUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer
 *        for SSUA per channel quantized scheme
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer_SSUA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer
 *        for UAUA per layer quantized scheme
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer_UAUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer @ref ai_layer_conv2d_nl_pool layer
 *        for UAUA per channel quantized scheme
 * The @ref ai_layer_conv2d_nl_pool is a fused conv2D + optional nonlinear
 * layer + optional pooling / nonlinearity (average, max)
 * @ingroup layers_conv2d
 * @param layer see @ai_layer_conv2d_nl_pool
 */
AI_INTERNAL_API
void forward_conv2d_nl_pool_integer_UAUA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer.
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer(ai_layer *pLayer);



/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSSA per layer quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSSA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSSA per layer quantized scheme Optimized for HSP
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_hsp_sssa8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSSA per layer quantized scheme Optimized for HSP, 3Step loop (out_ch)
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_hsp_1step_sssa8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSSA per channel quantized scheme: HSP variant
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSSA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSUA per layer quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSUA per channel quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSUA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for UAUA per layer quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_UAUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for UAUA per channel quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_UAUA_ch(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*LAYERS_CONV2D_H*/
