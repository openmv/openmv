/**
  ******************************************************************************
  * @file    lite_conv2d_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite dqnn conv kernel datatypes
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
#ifndef LITE_CONV2D_DQNN_H
#define LITE_CONV2D_DQNN_H

#include "ai_lite_interface.h"

# define AI_16_OVERFLOW_CHECK(val_) (val_ <= 32767)

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

AI_API_DECLARE_BEGIN

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os1ws1_bn_pad0(const ai_u32 *pDataIn_init,
                                          ai_u32 *pDataOut_init,
                                          const ai_u32 *pWeights_init,
                                          ai_float *pScratch_32,
                                          const ai_u32 n_channel_in,
                                          const ai_u32 n_channel_out,
                                          const ai_i32 width_in,
                                          const ai_i32 height_in,
                                          const ai_i32 width_out,
                                          const ai_i32 height_out,
                                          const ai_i32 filt_width,
                                          const ai_i32 filt_height,
                                          const ai_i32 filt_pad_x,
                                          const ai_i32 filt_pad_y,
                                          const ai_i32 filt_stride_x,
                                          const ai_i32 filt_stride_y,
                                          const ai_i32 *pThreshold);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 *        - Optimized thanks to Optim0 assumptions
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os1ws1_bn_pad0_optim0(const ai_u32 *pDataIn_init,
                                                  ai_u32 *pDataOut_init,
                                                  const ai_u32 *pWeights_init,
                                                  ai_float *pScratch_32,
                                                  const ai_u32 n_channel_in,
                                                  const ai_u32 n_channel_out,
                                                  const ai_i32 width_in,
                                                  const ai_i32 height_in,
                                                  const ai_i32 width_out,
                                                  const ai_i32 height_out,
                                                  const ai_i32 filt_width,
                                                  const ai_i32 filt_height,
                                                  const ai_i32 filt_pad_x,
                                                  const ai_i32 filt_pad_y,
                                                  const ai_i32 filt_stride_x,
                                                  const ai_i32 filt_stride_y,
                                                  const ai_i32 *pThreshold);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os8ws1_bn_pad0(const ai_u32 *pDataIn_init,
                                          ai_i8 *pDataOut_init,
                                          const ai_u32 *pWeights_init,
                                          ai_float *pScratch_32,
                                          const ai_u32 n_channel_in,
                                          const ai_u32 n_channel_out,
                                          const ai_i32 width_in,
                                          const ai_i32 height_in,
                                          const ai_i32 width_out,
                                          const ai_i32 height_out,
                                          const ai_i32 filt_width,
                                          const ai_i32 filt_height,
                                          const ai_i32 filt_pad_x,
                                          const ai_i32 filt_pad_y,
                                          const ai_i32 filt_stride_x,
                                          const ai_i32 filt_stride_y,
                                          const ai_float *pScale,
                                          const ai_float *pOffset);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os1ws1_bn_pad1(const ai_u32 *pDataIn_init,
                                        ai_u32 *pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        ai_float *pScratch_32,
                                        const ai_u32 n_channel_in,
                                        const ai_u32 n_channel_out,
                                        const ai_i32 width_in,
                                        const ai_i32 height_in,
                                        const ai_i32 width_out,
                                        const ai_i32 height_out,
                                        const ai_i32 filt_width,
                                        const ai_i32 filt_height,
                                        const ai_i32 filt_pad_x,
                                        const ai_i32 filt_pad_y,
                                        const ai_i32 filt_stride_x,
                                        const ai_i32 filt_stride_y,
                                        const ai_i32 *pThreshold,
                                        const ai_i32 pad_value);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *        - Optimized thanks to Optim2 assumptions
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os1ws1_bn_pad1_optim2(const ai_u32 *pDataIn_init,
                                                  ai_u32 *pDataOut_init,
                                                  const ai_u32 *pWeights_init,
                                                  ai_float *pScratch_32,
                                                  const ai_u32 n_channel_in,
                                                  const ai_u32 n_channel_out,
                                                  const ai_i32 width_in,
                                                  const ai_i32 height_in,
                                                  const ai_i32 width_out,
                                                  const ai_i32 height_out,
                                                  const ai_i32 filt_width,
                                                  const ai_i32 filt_height,
                                                  const ai_i32 filt_pad_x,
                                                  const ai_i32 filt_pad_y,
                                                  const ai_i32 filt_stride_x,
                                                  const ai_i32 filt_stride_y,
                                                  const ai_i32 *pThreshold,
                                                  const ai_i32 pad_value);

/*!
 * @brief  Handles 2D convolution with binary input, binary output and
 *        binary weights
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os1ws1_bn(const ai_u32 *pDataIn_init,
                                        ai_u32 * pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        ai_float *pScratch_32,
                                        const ai_u16 n_channel_in,
                                        const ai_u16 n_channel_out,
                                        const ai_u16 width_in,
                                        const ai_u16 height_in,
                                        const ai_u16 width_out,
                                        const ai_u16 height_out,
                                        const ai_u16 filt_width,
                                        const ai_u16 filt_height,
                                        const ai_u16 filt_pad_x,
                                        const ai_u16 filt_pad_y,
                                        const ai_u16 filt_stride_x,
                                        const ai_u16 filt_stride_y,
                                        const ai_i32 *pThreshold,
                                        const ai_u8 flatten_output);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os8ws1_bn_pad1(const ai_u32 *pDataIn_init,
                                        ai_i8 *pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        ai_float *pScratch_32,
                                        const ai_u32 n_channel_in,
                                        const ai_u32 n_channel_out,
                                        const ai_i32 width_in,
                                        const ai_i32 height_in,
                                        const ai_i32 width_out,
                                        const ai_i32 height_out,
                                        const ai_i32 filt_width,
                                        const ai_i32 filt_height,
                                        const ai_i32 filt_pad_x,
                                        const ai_i32 filt_pad_y,
                                        const ai_i32 filt_stride_x,
                                        const ai_i32 filt_stride_y,
                                        const ai_float *pScale,
                                        const ai_float *pOffset,
                                        const ai_i32 pad_value);

/*!
 * @brief Handles 2D convolution with binary input, 8-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *        - Optimized thanks to Optim1 assumptions
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os8ws1_bn_pad1_optim1(const ai_u32 *pDataIn_init,
                                                  ai_i8 *pDataOut_init,
                                                  const ai_u32 *pWeights_init,
                                                  ai_float *pScratch_32,
                                                  const ai_u32 n_channel_in,
                                                  const ai_u32 n_channel_out,
                                                  const ai_i32 width_in,
                                                  const ai_i32 height_in,
                                                  const ai_i32 width_out,
                                                  const ai_i32 height_out,
                                                  const ai_i32 filt_width,
                                                  const ai_i32 filt_height,
                                                  const ai_i32 filt_pad_x,
                                                  const ai_i32 filt_pad_y,
                                                  const ai_i32 filt_stride_x,
                                                  const ai_i32 filt_stride_y,
                                                  const ai_float *pScale,
                                                  const ai_float *pOffset,
                                                  const ai_i32 pad_value);

/**
 * @brief Handles 2D convolution with binary input, fixed point 16-bits output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F

 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os16ws1_bn_pad0_fxp(const ai_u32 *pDataIn_init,
                                                ai_i16 *pDataOut_init,
                                                const ai_u32 *pWeights_init,
                                                ai_float *pScratch_32,
                                                const ai_u32 n_channel_in,
                                                const ai_u32 n_channel_out,
                                                const ai_i32 width_in,
                                                const ai_i32 height_in,
                                                const ai_i32 width_out,
                                                const ai_i32 height_out,
                                                const ai_i32 filt_width,
                                                const ai_i32 filt_height,
                                                const ai_i32 filt_pad_x,
                                                const ai_i32 filt_pad_y,
                                                const ai_i32 filt_stride_x,
                                                const ai_i32 filt_stride_y,
                                                const ai_float *pScale_init,
                                                const ai_float *pOffset_init);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os16ws1_bn_pad1_fxp(const ai_u32 *pDataIn_init,
                                                ai_i16 *pDataOut_init,
                                                const ai_u32 *pWeights_init,
                                                ai_float *pScratch_32,
                                                const ai_u32 n_channel_in,
                                                const ai_u32 n_channel_out,
                                                const ai_i32 width_in,
                                                const ai_i32 height_in,
                                                const ai_i32 width_out,
                                                const ai_i32 height_out,
                                                const ai_i32 filt_width,
                                                const ai_i32 filt_height,
                                                const ai_i32 filt_pad_x,
                                                const ai_i32 filt_pad_y,
                                                const ai_i32 filt_stride_x,
                                                const ai_i32 filt_stride_y,
                                                const ai_float *pScale_init,
                                                const ai_float *pOffset_init,
                                                const ai_i32 pad_value);


/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *        - Optimized thanks to Optim1 assumptions
 *
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1os16ws1_bn_pad1_optim1_fxp(const ai_u32 *pDataIn_init,
                                                       ai_i16 *pDataOut_init,
                                                       const ai_u32 *pWeights_init,
                                                       ai_float *pScratch_32,
                                                       const ai_u32 n_channel_in,
                                                       const ai_u32 n_channel_out,
                                                       const ai_i32 width_in,
                                                       const ai_i32 height_in,
                                                       const ai_i32 width_out,
                                                       const ai_i32 height_out,
                                                       const ai_i32 filt_width,
                                                       const ai_i32 filt_height,
                                                       const ai_i32 filt_pad_x,
                                                       const ai_i32 filt_pad_y,
                                                       const ai_i32 filt_stride_x,
                                                       const ai_i32 filt_stride_y,
                                                       const ai_float *pScale_init,
                                                       const ai_float *pOffset_init,
                                                       const ai_i32 pad_value);


/**
 * @brief Handles 2D convolution with binary input, fixed point 16-bits unsigned output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 *
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1ou16ws1_bn_pad1_fxp(const ai_u32 *pDataIn_init,
                                                       ai_u16 *pDataOut_init,
                                                       const ai_u32 *pWeights_init,
                                                       ai_float *pScratch_32,
                                                       const ai_u32 n_channel_in,
                                                       const ai_u32 n_channel_out,
                                                       const ai_i32 width_in,
                                                       const ai_i32 height_in,
                                                       const ai_i32 width_out,
                                                       const ai_i32 height_out,
                                                       const ai_i32 filt_width,
                                                       const ai_i32 filt_height,
                                                       const ai_i32 filt_pad_x,
                                                       const ai_i32 filt_pad_y,
                                                       const ai_i32 filt_stride_x,
                                                       const ai_i32 filt_stride_y,
                                                       const ai_float *pScale_init,
                                                       const ai_float *pOffset_init,
                                                       const ai_i32 pad_value);


/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits unsigned output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1ou16ws1_bn_pad0_fxp(const ai_u32 *pDataIn_init,
                                                ai_u16 *pDataOut_init,
                                                const ai_u32 *pWeights_init,
                                                ai_float *pScratch_32,
                                                const ai_u32 n_channel_in,
                                                const ai_u32 n_channel_out,
                                                const ai_i32 width_in,
                                                const ai_i32 height_in,
                                                const ai_i32 width_out,
                                                const ai_i32 height_out,
                                                const ai_i32 filt_width,
                                                const ai_i32 filt_height,
                                                const ai_i32 filt_pad_x,
                                                const ai_i32 filt_pad_y,
                                                const ai_i32 filt_stride_x,
                                                const ai_i32 filt_stride_y,
                                                const ai_float *pScale_init,
                                                const ai_float *pOffset_init);

/*!
 * @brief Handles 2D convolution with binary input, fixed point 16-bits unsigned output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F.
 *        - Optimized thanks to Optim1 assumptions
 *
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is1ou16ws1_bn_pad1_optim1_fxp(const ai_u32 *pDataIn_init,
                                                       ai_u16 *pDataOut_init,
                                                       const ai_u32 *pWeights_init,
                                                       ai_float *pScratch_32,
                                                       const ai_u32 n_channel_in,
                                                       const ai_u32 n_channel_out,
                                                       const ai_i32 width_in,
                                                       const ai_i32 height_in,
                                                       const ai_i32 width_out,
                                                       const ai_i32 height_out,
                                                       const ai_i32 filt_width,
                                                       const ai_i32 filt_height,
                                                       const ai_i32 filt_pad_x,
                                                       const ai_i32 filt_pad_y,
                                                       const ai_i32 filt_stride_x,
                                                       const ai_i32 filt_stride_y,
                                                       const ai_float *pScale_init,
                                                       const ai_float *pOffset_init,
                                                       const ai_i32 pad_value);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and weights and
 *        binary output - Lite I/F
 * @ingroup lite_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
LITE_API_ENTRY
void forward_lite_conv2d_is8os1ws8(const ai_i8 *pDataIn_init,
                                   ai_u32 *pDataOut_init,
                                   const ai_i8 *pWeights_init,
                                   ai_float *pScratch_32,
                                   const ai_u32 n_channel_in,
                                   const ai_u32 n_channel_out,
                                   const ai_i32 width_in,
                                   const ai_i32 height_in,
                                   const ai_i32 width_out,
                                   const ai_i32 height_out,
                                   const ai_i32 filt_width,
                                   const ai_i32 filt_height,
                                   const ai_i32 filt_pad_x,
                                   const ai_i32 filt_pad_y,
                                   const ai_i32 filt_stride_x,
                                   const ai_i32 filt_stride_y,
                                   const ai_i32 *pThreshold,
                                   const ai_i8 in_zeropoint);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and weights and
 *        binary output - Lite I/F - Optimized thanks to Optim2 assumptions
 * @ingroup lite_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
LITE_API_ENTRY
void forward_lite_conv2d_is8os1ws8_optim2(const ai_i8 *pDataIn_init,
                                         ai_u32 *pDataOut_init,
                                         const ai_i8 *pWeights_init,
                                         ai_float *pScratch_32,
                                         const ai_u32 n_channel_in,
                                         const ai_u32 n_channel_out,
                                         const ai_i32 width_in,
                                         const ai_i32 height_in,
                                         const ai_i32 width_out,
                                         const ai_i32 height_out,
                                         const ai_i32 filt_width,
                                         const ai_i32 filt_height,
                                         const ai_i32 filt_pad_x,
                                         const ai_i32 filt_pad_y,
                                         const ai_i32 filt_stride_x,
                                         const ai_i32 filt_stride_y,
                                         const ai_i32 *pThreshold,
                                         const ai_i8 in_zeropoint);

/*!
 * @brief Handles 2D convolution with 8-bits quantized Input and weights and
 *        binary output - quantized with DoReFa SotA quantizer, lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_dorefa_is8os1ws8(const ai_i8 *pDataIn_init,
                                          ai_u32 *pDataOut_init,
                                          const ai_u8 *pWeights_init,
                                          ai_float *pScratch_32,
                                          const ai_u32 n_channel_in,
                                          const ai_u32 n_channel_out,
                                          const ai_i32 width_in,
                                          const ai_i32 height_in,
                                          const ai_i32 width_out,
                                          const ai_i32 height_out,
                                          const ai_i32 filt_width,
                                          const ai_i32 filt_height,
                                          const ai_i32 filt_pad_x,
                                          const ai_i32 filt_pad_y,
                                          const ai_i32 filt_stride_x,
                                          const ai_i32 filt_stride_y,
                                          const ai_i32 *pThreshold,
                                          const ai_i8 in_zeropoint);


/*!
 * @brief Handles 2D convolution with 8-bits quantized input, output and weights
 *        - quantized with with different quantization for channel
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is8os8ws8_sssa_ch(const ai_i8 *pData_in,
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
                                           const ai_i32 scratch_size);

/*!
 * @brief Handles 2D convolution with 16-bits quantized inputs, binary outputs and binary weights - Lite I/F.
 * Vanilla version.
 * @ingroup lite_conv2d_dqnn
 * @param layer conv2d_dqnn layer
 */
LITE_API_ENTRY
void forward_lite_conv2d_is16os1ws1_bn_fxp(const ai_i16 *pIn,
                                           ai_u32 *pOut_32,
                                           const ai_u32 *pWeights,
                                           const ai_i32 *pThreshold,
                                           ai_i8 *pBufferA,
                                           const ai_i32 dim_kernel,
                                           const ai_i16 dim_im_in_x,
                                           const ai_i16 dim_im_in_y,
                                           const ai_i16 dim_im_out_x,
                                           const ai_i16 dim_im_out_y,
                                           const ai_i16 ch_im_in,
                                           const ai_i16 ch_im_out,
                                           const ai_i16 dim_kernel_x,
                                           const ai_i16 dim_kernel_y,
                                           const ai_i16 padding_x,
                                           const ai_i16 padding_y,
                                           const ai_i16 stride_x,
                                           const ai_i16 stride_y,
                                           const ai_i16 dilation_x,
                                           const ai_i16 dilation_y,
                                           const ai_i16 in_zeropoint);


/**
 * @brief Handles 2D convolution with 16-bits quantized inputs, 16-bits quantized outputs and binary weights - Lite I/F
 *
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_conv2d_is16os16ws1_fxp(const ai_i16 *pIn,
                                         ai_i16 *pOut,
                                         const ai_u32 *pWeights,
                                         ai_i8 *pBufferA,
                                         const ai_i16 dim_im_in_x,
                                         const ai_i16 dim_im_in_y,
                                         const ai_i16 dim_im_out_x,
                                         const ai_i16 dim_im_out_y,
                                         const ai_i16 ch_im_in,
                                         const ai_i16 ch_im_out,
                                         const ai_u32 dim_kernel,
                                         const ai_i16 dim_kernel_x,
                                         const ai_i16 dim_kernel_y,
                                         const ai_i16 padding_x,
                                         const ai_i16 padding_y,
                                         const ai_i16 stride_x,
                                         const ai_i16 stride_y,
                                         const ai_i16 dilation_x,
                                         const ai_i16 dilation_y,
                                         const ai_i16 in_zeropoint);

AI_API_DECLARE_END

#endif    /*LITE_CONV2D_DQNN_H*/
