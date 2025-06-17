/**
  ******************************************************************************
  * @file    layers_conv2d_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform DQNN conv datatypes
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
#ifndef LAYERS_CONV2D_DQNN_H
#define LAYERS_CONV2D_DQNN_H

#include "layers_common.h"
#include "layers_conv2d.h"

/*!
 * @defgroup layers_conv2d_dqnn Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN


#define AI_DQNN_PAD_1_KEY     (1)
#define AI_DQNN_PAD_M1_KEY    (-1)
#define AI_DQNN_PAD_0_KEY     (0)
#define AI_DQNN_PAD_1_VALUE   (0x0)
#define AI_DQNN_PAD_M1_VALUE  (0xFFFFFFFF)
#define AI_DQNN_PAD_0_VALUE   (0x2)


/*!
 * @struct ai_layer_conv2d_dqnn
 * @ingroup layers_conv2d_dqnn
 * @brief conv2d_dqnn layer
 *
 * @ref forward_conv2d_is1os1ws1
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_conv2d_dqnn_ {
  AI_LAYER_CONV2D_FIELDS_DECLARE
  ai_i32  pad_value;
} ai_layer_conv2d_dqnn;


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles point wise convolution with binary input, binary output and
 *        binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_pw_is1os1ws1_bn(ai_layer *pLayer);

/*!
 * @brief Handles point wise convolution with binary input, binary output and
 *        binary weights - Optimized thanks to Optim2 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_pw_is1os1ws1_bn_optim2(ai_layer *pLayer);

/*!
 * @brief Handles point wise convolution with binary input, 8-bits output and
 *        binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_pw_is1os8ws1_bn(ai_layer *pLayer);

/*!
 * @brief Handles point wise convolution with binary input, 8-bits output and
 *        binary weights - Optimized thanks to Optim1 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_pw_is1os8ws1_bn_optim1(ai_layer *pLayer);

/*!
 * @brief Handles point-wise convolution with binary input, float32 output
 *        and binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_pw_is1of32ws1_bn(ai_layer *pLayer);

/*!
 * @brief Handles point-wise convolution with binary input, float32 output
 *        and binary weights - Optimized thanks to Optim1 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_pw_is1of32ws1_bn_optim1(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os1ws1_bn(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - Optimized thanks to Optim2 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os1ws1_bn_optim2(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os8ws1_bn(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - Optimized thanks to Optim1 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os8ws1_bn_optim1(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like)
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os1ws1_bn_pad0(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like) - Optimized thanks to
 *        Optim0 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os1ws1_bn_pad0_optim0(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - with 0 padding (QKeras like)
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os8ws1_bn_pad0(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like)
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os1ws1_bn_pad1(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like) - Optimized thanks
 *        to Optim2 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os1ws1_bn_pad1_optim2(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - with +1/-1 padding (Larq like)
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os8ws1_bn_pad1(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Optimized thanks
 *        to Optim1 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os8ws1_bn_pad1_optim1(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and weights and
 *        binary output
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is8os1ws8(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and weights and
 *        binary output - Optimized thanks to Optim2 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is8os1ws8_optim2(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and weights and
 *        binary output - quantized with DoReFa SotA quantizer
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_dorefa_is8os1ws8(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with 16-bits quantized input, binary weights
          and binary output
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is16os1ws1_bn_fxp(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with 16-bits quantized input, binary weights
          and 16-bits quantized output
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is16os16ws1_fxp(ai_layer *pLayer);

/*!
 * @brief Handles depth-wise convolution with binary input, binary output and
 *        binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_dw_is1os1ws1_bn(ai_layer *pLayer);

/*!
 * @brief Handles depth-wise convolution with binary input, binary output and
 *        binary weights - Optimized thanks to Optim3 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_dw_is1os1ws1_bn_optim3(ai_layer *pLayer);

/*!
 * @brief Handles depth-wise convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like)
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_dw_is1os1ws1_bn_pad0(ai_layer *pLayer);

/*!
 * @brief Handles depth-wise convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like) - Optimized thanks to
 *        Optim3 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_dw_is1os1ws1_bn_pad0_optim3(ai_layer *pLayer);

/*!
 * @brief Handles depth-wise convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like)
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_dw_is1os1ws1_bn_pad1(ai_layer *pLayer);

/*!
 * @brief Handles depth-wise convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like) - Optimized thanks to
 *        Optim3 assumptions
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_dw_is1os1ws1_bn_pad1_optim3(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and output and
 *        binary weights
 * @ingroup layers_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
AI_INTERNAL_API
void forward_conv2d_is8os8ws1(ai_layer *pLayer);

/**
 * @brief Handles 2D convolution with binary input, fixed point 16-bits output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os16ws1_bn_pad0_fxp(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os16ws1_bn_pad1_fxp(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *        - Optimized thanks to Optim1 assumptions
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is1os16ws1_bn_pad1_optim1_fxp(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits unsigned output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is1ou16ws1_bn_pad0_fxp(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits unsigned output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is1ou16ws1_bn_pad1_fxp(ai_layer *pLayer);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits unsiged output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *        - Optimized thanks to Optim1 assumptions
 * @ingroup lite_conv2d_dqnn
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is1ou16ws1_bn_pad1_optim1_fxp(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer quantized 2D convolutional layer
 *        for SSSA per channel quantized RGB scheme using n_channel_in = 3
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_conv2d_is8os8ws8_sssa_ch_rgb(const ai_i8 *pData_in,
                                          ai_i8 *pData_out,
                                          const ai_i8 *pWeights,
                                          const ai_i32 *pBias,
                                          ai_u16 *pBuffer_a,
                                          const ai_size width_in,
                                          const ai_size height_in,
                                          const ai_size width_out,
                                          const ai_size height_out,
                                          const ai_u16 n_channel_in,
                                          const ai_u16 n_channel_out,
                                          const ai_size filt_width,
                                          const ai_size filt_height,
                                          const ai_u16 filt_pad_x,
                                          const ai_u16 filt_pad_y,
                                          const ai_u16 filt_stride_x,
                                          const ai_u16 filt_stride_y,
                                          const ai_float in_scale,
                                          const ai_float out_scale,
                                          const ai_float *pWt_scale,
                                          const ai_i8 in_zeropoint,
                                          const ai_i8 out_zeropoint,
                                          const ai_bool out_ch_format,
                                          ai_i16 *p_out_r_shift,
                                          ai_i32 *p_out_factor);

/*!
 * @brief Computes the activations of a point-wise integer quantized convolution
          for SSSA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_pw_is8os8ws8_sssa_ch(const ai_i8 *pData_in,
                                  ai_i8 *pData_out,
                                  const ai_i8 *pWeights,
                                  const ai_i32 *pBias,
                                  ai_u16 *pBuffer_a,
                                  const ai_size width_in,
                                  const ai_size height_in,
                                  const ai_size width_out,
                                  const ai_size height_out,
                                  const ai_u16 n_channel_in,
                                  const ai_u16 n_channel_out,
                                  const ai_size filt_width,
                                  const ai_size filt_height,
                                  const ai_u16 filt_pad_x,
                                  const ai_u16 filt_pad_y,
                                  const ai_u16 filt_stride_x,
                                  const ai_u16 filt_stride_y,
                                  const ai_u16 dilation_x,
                                  const ai_u16 dilation_y,
                                  const ai_float in_scale,
                                  const ai_float out_scale,
                                  const ai_float *pWt_scale,
                                  const ai_i8 in_zeropoint,
                                  const ai_i8 out_zeropoint,
                                  ai_i16 *p_out_r_shift,
                                  ai_i32 *p_out_factor,
                                  ai_i32 AI_PWOverlay,
                                  ai_i16 *bufferA,
                                  ai_i32 scratch_size);
                                  // st_nn_context_t context);

/*!
 * @brief Computes the activations of a depth-wise integer quantized convolution
          for SSSA per channel quantized scheme
 * @ingroup layers_conv2d
 * @param layer the convolutional (conv) layer
 */
AI_INTERNAL_API
void forward_dw_is8os8ws8_sssa_ch(const ai_i8 *pData_in,
                                  ai_i8 *pData_out,
                                  const ai_i8 *pWeights,
                                  const ai_i32 *pBias,
                                  ai_u16 *pBuffer_a,
                                  const ai_size width_in,
                                  const ai_size height_in,
                                  const ai_size width_out,
                                  const ai_size height_out,
                                  const ai_u16 n_channel_in,
                                  const ai_u16 n_channel_out,
                                  const ai_size filt_width,
                                  const ai_size filt_height,
                                  const ai_u16 filt_pad_x,
                                  const ai_u16 filt_pad_y,
                                  const ai_u16 filt_stride_x,
                                  const ai_u16 filt_stride_y,
                                  const ai_u16 dilation_x,
                                  const ai_u16 dilation_y,
                                  const ai_float in_scale,
                                  const ai_float out_scale,
                                  const ai_float *pWt_scale,
                                  const ai_i8 in_zeropoint,
                                  const ai_i8 out_zeropoint,
                                  ai_i16 *p_out_r_shift,
                                  ai_i32 *p_out_factor);


AI_API_DECLARE_END

#endif    /*LAYERS_CONV2D_DQNN_H*/
