/**
  ******************************************************************************
  * @file    layers_pool.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform pooling layers datatypes
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
#ifndef LAYERS_POOL_H
#define LAYERS_POOL_H

#include "layers_common.h"
#include "lite_maxpool_dqnn.h"
#include "lite_pool_f32.h"

/*!
 * @defgroup layers_pool Pooling Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_pool
 * @ingroup layers_pool
 * @brief Pooling layer
 *
 * The type of pooling function is handled by the specific forward function
 * @ref forward_pool
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_pool_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape_2d pool_size;            /*!< pooling size */
  ai_shape_2d pool_stride;          /*!< pooling stride */
  ai_shape    pool_pad;             /*!< pooling pad, y,x border sizes */
  ai_u8       count_include_pad;    /*!< include pad flag */
} ai_layer_pool;



/*!
 * @brief Max Pooling on a 8/16 bits fixed point data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to output data
 */
AI_INTERNAL_API
void pool_func_mp_array_fixed(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

/*!
 * @brief Max Pooling on a 8-bits integer quantized data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to output data
 */
AI_INTERNAL_API
void pool_func_mp_array_integer(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

/*!
 * @brief Max Pooling on a signed 8-bits integer quantized data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to output data
 */
AI_INTERNAL_API
void pool_func_mp_array_integer_INT8(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

/*!
 * @brief Max Pooling on a unsigned 8-bits integer quantized data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to output data
 */
AI_INTERNAL_API
void pool_func_mp_array_integer_UINT8(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

/*!
 * @brief Average Pooling on a 8/16 bits fixed point data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to scratch memory
 */
AI_INTERNAL_API
void pool_func_ap_array_fixed(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

 /*!
 * @brief Average Pooling on a 8-bits integer quantized data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to scratch memory
 */
AI_INTERNAL_API
void pool_func_ap_array_integer(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

 /*!
 * @brief Average Pooling on a signed 8-bits integer quantized data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to scratch memory
 */
AI_INTERNAL_API
void pool_func_ap_array_integer_INT8(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

 /*!
 * @brief Average Pooling on a unsigned 8-bits integer quantized data array
 * @ingroup layers_pool
 * @param in  opaque handler to input data to process
 * @param dim_im_in_x  input feature map width
 * @param dim_im_in_y  input feature map height
 * @param ch_im_in  number of input channels
 * @param dim_kernel_x  kernel width
 * @param dim_kernel_y  kernel height
 * @param padding_x  right padding value
 * @param padding_y  top padding value
 * @param stride_x  stride value on x dimension
 * @param stride_y  stride value on y dimension
 * @param dim_im_out_x  output feature map width
 * @param dim_im_out_y  output feature map height
 * @param out opaque handler to scratch memory
 */
AI_INTERNAL_API
void pool_func_ap_array_integer_UINT8(ai_handle in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_handle out);

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Computes the activations of a max pooling layer.
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp(ai_layer* layer);

/*!
 * @brief Computes the activations of a fixed point max pooling layer.
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized max pooling layer.
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized max pooling layer
 *        with int8 I/O
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_integer_INT8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized max pooling layer
 *        with int8 I/O. Optimized for HSP
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_hsp_INT8(ai_layer *pLayer);


/*!
 * @brief Computes the activations of an integer-quantized max pooling layer
 *        with int8 I/O. Optimized for HSP: 2 Step variant for bigger tensors
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_hsp_2step_INT8(ai_layer *pLayer);


/*!
 * @brief Computes the activations of an integer-quantized max pooling layer
 *        with uint8 I/O
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_integer_UINT8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized max pooling layer
 *        with int16 I/O
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_integer_INT16(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized max pooling layer
 *        with uint16 I/O
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_mp_integer_UINT16(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an average pooling layer.
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_ap(ai_layer* layer);


/*!
 * @brief Computes the activations of a fixed point average pooling layer.
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_ap_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized average pooling layer.
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_ap_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an average pooling layer. Optimized for HSP
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_ap_hsp_INT8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an average pooling layer. Optimized for HSP
 * Variant for larger tensors
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer,
 */
AI_INTERNAL_API
void forward_ap_hsp_2step_INT8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized average pooling layer
 *        with int8 I/O
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_ap_integer_INT8(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer-quantized average pooling layer
 *        with uint8 I/O
 * @ingroup layers_pool
 * @param layer the pooling (pool) layer
 */
AI_INTERNAL_API
void forward_ap_integer_UINT8(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*LAYERS_POOL_H*/
