/**
  ******************************************************************************
  * @file    lite_conv2d.h
  * @author  AIS
  * @brief   header file of AI platform lite conv2d kernel datatypes
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
#ifndef LITE_CONV2D_H
#define LITE_CONV2D_H

#include "ai_lite_interface.h"
#include "lite_internal_apis.h"

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles 2D convolution with float input, float output and
 *        float weights
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void forward_lite_conv2d_if32of32wf32(const ai_float *pDataIn_init,
                                ai_float *pDataOut_init,
                                const ai_ptr_const pWeights_init,
                                const ai_ptr_const pBias_init,
                                ai_float *pWeights_prefetch,
                                const ai_size n_channel_in,
                                const ai_size n_channel_out,
                                const ai_size width_in,
                                const ai_size height_in,
                                const ai_size width_out,
                                const ai_size height_out,
                                const ai_size filt_width,
                                const ai_size filt_height,
                                const ai_u16 filt_pad_x,
                                const ai_u16 filt_pad_y,
                                const ai_u16 filt_stride_x,
                                const ai_u16 filt_stride_y,
                                const ai_size filt_height_dilated,
                                const ai_size filt_width_dilated,
                                const ai_u16 dilation_x,
                                const ai_u16 dilation_y,
                                const ai_size n_groups);

/*!
 * @brief Handles 2D depthwise convolution with float input, float output and
 *        float weights
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void forward_lite_dw_if32of32wf32(const ai_float *pDataIn_init,
                                ai_float *pDataOut_init,
                                const ai_ptr_const pWeights_init,
                                const ai_ptr_const pBias_init,
                                const ai_size n_channel_in,
                                const ai_size n_channel_out,
                                const ai_size width_in,
                                const ai_size height_in,
                                const ai_size width_out,
                                const ai_size height_out,
                                const ai_size filt_width,
                                const ai_size filt_height,
                                const ai_u16 filt_pad_x,
                                const ai_u16 filt_pad_y,
                                const ai_u16 filt_stride_x,
                                const ai_u16 filt_stride_y,
                                const ai_size filt_height_dilated,
                                const ai_size filt_width_dilated,
                                const ai_u16 dilation_x,
                                const ai_u16 dilation_y,
                                const ai_size n_groups);

/*!
 * @brief Handles 2D grouped convolution with float input, float output and
 *        float weights
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void forward_lite_conv2d_if32of32wf32_group(const ai_float *pDataIn_init,
                                ai_float *pDataOut_init,
                                const ai_ptr_const pWeights_init,
                                const ai_ptr_const pBias_init,
                                const ai_size n_channel_in,
                                const ai_size n_channel_out,
                                const ai_size width_in,
                                const ai_size height_in,
                                const ai_size width_out,
                                const ai_size height_out,
                                const ai_size filt_width,
                                const ai_size filt_height,
                                const ai_u16 filt_pad_x,
                                const ai_u16 filt_pad_y,
                                const ai_u16 filt_stride_x,
                                const ai_u16 filt_stride_y,
                                const ai_size filt_height_dilated,
                                const ai_size filt_width_dilated,
                                const ai_u16 dilation_x,
                                const ai_u16 dilation_y,
                                const ai_size n_groups);

/*!
 * @brief Handles dilated conv2d convolutions (valid padding)
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void
forward_lite_conv2d_dilated_sssa8_ch(const ai_i8 *pData_in,
                                     const ai_u16 dim_im_in_x,
                                     const ai_u16 dim_im_in_y,
                                     const ai_u16 n_channel_in,
                                     const ai_i8 *pWeights,
                                     const ai_u16 n_channel_out,
                                     const ai_u16 dim_kernel_x,
                                     const ai_u16 dim_kernel_y,
                                     const ai_u16 stride_x,
                                     const ai_u16 stride_y,
                                     const ai_u16 dilation_x,
                                     const ai_u16 dilation_y,
                                     const ai_i32 *pBias,
                                     const ai_i8 in_zeropoint,
                                     const ai_i8 out_zeropoint,
                                     const ai_float in_scale,
                                     const ai_float out_scale,
                                     const ai_float *pWt_scale,
                                     const ai_layer_format_type out_ch_format,
                                     ai_i8 *pData_out,
                                     const ai_u16 dim_im_out_x,
                                     const ai_u16 dim_im_out_y,
                                     ai_u32 height_loop_cnt,
                                     const ai_u16 weights_prefetch_enabled,
                                     ai_i32 scratch_size,
                                     ai_i16 *pBuffer_a);

/*!
 * @brief Handles conv2d convolutions (valid padding) with number of channels >= 8
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void
forward_lite_conv2d_deep_sssa8_ch(const ai_i8 *pData_in,
                                  const ai_u16 dim_im_in_x,
                                  const ai_u16 dim_im_in_y,
                                  const ai_u16 n_channel_in,
                                  const ai_i8 *pWeights,
                                  const ai_u16 n_channel_out,
                                  const ai_u16 dim_kernel_x,
                                  const ai_u16 dim_kernel_y,
                                  const ai_u16 stride_x,
                                  const ai_u16 stride_y,
                                  const ai_i32 *pBias,
                                  const ai_i8 in_zeropoint,
                                  const ai_i8 out_zeropoint,
                                  const ai_float in_scale,
                                  const ai_float out_scale,
                                  const ai_float *pWt_scale,
                                  const ai_layer_format_type out_ch_format,
                                  ai_i8 *pData_out,
                                  const ai_u16 dim_im_out_x,
                                  const ai_u16 dim_im_out_y,
                                  ai_u32 height_loop_cnt,
                                  const ai_u16 weights_prefetch_enabled,
                                  ai_i32 scratch_size,
                                  ai_i16 *pBuffer_a);
/*!
 * @brief Handles conv2d convolutions (valid padding) with number of channels >= 8
 * Special forward function for 3x3 kernels and Stride = 1
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void
forward_lite_conv2d_deep_3x3_sssa8_ch(const ai_i8 *pData_in,
                                      const ai_u16 dim_im_in_x,
                                      const ai_u16 dim_im_in_y,
                                      const ai_u16 n_channel_in,
                                      const ai_i8 *pWeights,
                                      const ai_u16 n_channel_out,
                                      const ai_i32 *pBias,
                                      const ai_i8 in_zeropoint,
                                      const ai_i8 out_zeropoint,
                                      const ai_float in_scale,
                                      const ai_float out_scale,
                                      const ai_float *pWt_scale,
                                      const ai_layer_format_type out_ch_format,
                                      ai_i8 *pData_out,
                                      const ai_u16 dim_im_out_x,
                                      const ai_u16 dim_im_out_y,
                                      ai_u32 height_loop_cnt,
                                      ai_i32 scratch_size,
                                      ai_i16 *pBuffer_a);

/*!
 * @brief Handles conv2d convolutions optimized by HSP HW
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void
forward_lite_conv2d_hsp_sssa8_ch(const ai_i8 *pData_in,
                                 const ai_u16 dim_im_in_x,
                                 const ai_u16 dim_im_in_y,
                                 const ai_u16 n_channel_in,
                                 const ai_i8 *pWeights,
                                 const ai_u16 n_channel_out,
                                 const ai_u16 dim_kernel_x,
                                 const ai_u16 dim_kernel_y,
                                 const ai_u16 stride_x,
                                 const ai_u16 stride_y,
                                 const ai_u16 padding_x,
                                 const ai_u16 padding_x_r,
                                 const ai_u16 padding_y,
                                 const ai_u16 padding_y_b,
                                 const ai_i32 *pBias,
                                 const ai_i8 in_zeropoint,
                                 const ai_i8 out_zeropoint,
                                 const ai_float in_scale,
                                 const ai_float out_scale,
                                 const ai_float *pWt_scale,
                                 const ai_layer_format_type out_ch_format,
                                 ai_i8 *pData_out,
                                 const ai_u16 dim_im_out_x,
                                 const ai_u16 dim_im_out_y,
                                 ai_i32 scratch_size,
                                 ai_i16 *pBuffer_a);


/*!
 * @brief Handles conv2d convolutions with same padding or with number of channels < 8
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void
forward_lite_conv2d_sssa8_ch(const ai_i8 *pData_in,
                             const ai_u16 dim_im_in_x,
                             const ai_u16 dim_im_in_y,
                             const ai_u16 n_channel_in,
                             const ai_i8 *pWeights,
                             const ai_u16 n_channel_out,
                             const ai_u16 dim_kernel_x,
                             const ai_u16 dim_kernel_y,
                             const ai_u16 stride_x,
                             const ai_u16 stride_y,
                             const ai_u16 padding_x,
                             const ai_u16 padding_y,
                             const ai_i32 *pBias,
                             const ai_i8 in_zeropoint,
                             const ai_i8 out_zeropoint,
                             const ai_float in_scale,
                             const ai_float out_scale,
                             const ai_float *pWt_scale,
                             const ai_layer_format_type out_ch_format,
                             ai_i8 *pData_out,
                             const ai_u16 dim_im_out_x,
                             const ai_u16 dim_im_out_y,
                             const ai_u16 weights_prefetch_enabled,
                             ai_i32 scratch_size,
                             ai_i16 *pBuffer_a);

/*!
 * @brief Handles rgb conv2d convolutions
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void
forward_lite_conv2d_rgb_sssa8_ch(const ai_i8 *pData_in,
                                 const ai_u16 dim_im_in,
                                 const ai_i8 *pWeights,
                                 const ai_u16 n_channel_out,
                                 const ai_u16 dim_kernel,
                                 const ai_u16 padding,
                                 const ai_u16 stride,
                                 const ai_i32 *pBias,
                                 const ai_i8 in_zeropoint,
                                 const ai_i8 out_zeropoint,
                                 const ai_float in_scale,
                                 const ai_float out_scale,
                                 const ai_float *pWt_scale,
                                 const ai_layer_format_type out_ch_format,
                                 ai_i8 *pData_out,
                                 const ai_u16 dim_im_out,
                                 ai_i32 scratch_size,
                                 ai_i16 *pBuffer_a);

/*!
 * @brief Handles 2D convolution with float input, float output and
 *        float weights with pool fused
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void forward_lite_conv2d_if32of32wf32_pool(const ai_float *pDataIn_init,
                                ai_float *pDataOut_init,
                                const ai_float *pWeights_init,
                                const ai_float *pBias_init,
                                ai_float *pScratch_init,
                                ai_float *pWeights_prefetch,
                                const ai_short_size n_channel_in,
                                const ai_short_size n_channel_out,
                                const ai_short_size width_in,
                                const ai_short_size height_in,
                                const ai_short_size width_out,
                                const ai_short_size height_out,
                                const ai_short_size filt_width,
                                const ai_short_size filt_height,
                                const ai_u16 filt_pad_x,
                                const ai_u16 filt_pad_y,
                                const ai_u16 filt_stride_x,
                                const ai_u16 filt_stride_y,
                                const ai_short_size filt_height_dilated,
                                const ai_short_size filt_width_dilated,
                                const ai_u16 dilation_x,
                                const ai_u16 dilation_y,
                                const ai_short_size n_groups,
                                const ai_short_size width_conv_out,
                                const ai_short_size height_conv_out,
                                func_nl_lite nl_func_lite,
                                ai_ptr_const nl_params,
                                const ai_ptr_offset nl_params_step,
                                const ai_ptr_offset nl_params_size,
                                ai_handle pool_func,
                                const ai_short_size pool_width,
                                const ai_short_size pool_height,
                                const ai_short_size pool_stride_x,
                                const ai_short_size pool_stride_y,
                                const ai_short_size pool_pad_x,
                                const ai_short_size pool_pad_y);

/*!
 * @brief Handles 2D depthwise convolution with float input, float output and
 *        float weights with pool fused
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void forward_lite_dw_if32of32wf32_pool(const ai_float *pDataIn_init,
                                ai_float *pDataOut_init,
                                const ai_float *pWeights_init,
                                const ai_float *pBias_init,
                                ai_float *pScratch_init,
                                const ai_short_size n_channel_in,
                                const ai_short_size n_channel_out,
                                const ai_short_size width_in,
                                const ai_short_size height_in,
                                const ai_short_size width_out,
                                const ai_short_size height_out,
                                const ai_short_size filt_width,
                                const ai_short_size filt_height,
                                const ai_u16 filt_pad_x,
                                const ai_u16 filt_pad_y,
                                const ai_u16 filt_stride_x,
                                const ai_u16 filt_stride_y,
                                const ai_short_size filt_height_dilated,
                                const ai_short_size filt_width_dilated,
                                const ai_u16 dilation_x,
                                const ai_u16 dilation_y,
                                const ai_short_size n_groups,
                                const ai_short_size width_conv_out,
                                const ai_short_size height_conv_out,
                                func_nl_lite nl_func_lite,
                                ai_ptr_const nl_params,
                                const ai_ptr_offset nl_params_step,
                                const ai_ptr_offset nl_params_size,
                                ai_handle pool_func,
                                const ai_short_size pool_width,
                                const ai_short_size pool_height,
                                const ai_short_size pool_stride_x,
                                const ai_short_size pool_stride_y,
                                const ai_short_size pool_pad_x,
                                const ai_short_size pool_pad_y);
/*!
 * @brief Handles 2D grouped convolution with float input, float output and
 *        float weights with pool fused
 * @ingroup lite_conv2d
 */
LITE_API_ENTRY
void forward_lite_conv2d_if32of32wf32_group_pool(const ai_float *pDataIn_init,
                                ai_float *pDataOut_init,
                                const ai_float *pWeights_init,
                                const ai_float *pBias_init,
                                ai_float *pScratch_init,
                                const ai_short_size n_channel_in,
                                const ai_short_size n_channel_out,
                                const ai_short_size width_in,
                                const ai_short_size height_in,
                                const ai_short_size width_out,
                                const ai_short_size height_out,
                                const ai_short_size filt_width,
                                const ai_short_size filt_height,
                                const ai_u16 filt_pad_x,
                                const ai_u16 filt_pad_y,
                                const ai_u16 filt_stride_x,
                                const ai_u16 filt_stride_y,
                                const ai_short_size filt_height_dilated,
                                const ai_short_size filt_width_dilated,
                                const ai_u16 dilation_x,
                                const ai_u16 dilation_y,
                                const ai_short_size n_groups,
                                const ai_short_size width_conv_out,
                                const ai_short_size height_conv_out,
                                func_nl_lite nl_func_lite,
                                ai_ptr_const nl_params,
                                const ai_ptr_offset nl_params_step,
                                const ai_ptr_offset nl_params_size,
                                ai_handle pool_func,
                                const ai_short_size pool_width,
                                const ai_short_size pool_height,
                                const ai_short_size pool_stride_x,
                                const ai_short_size pool_stride_y,
                                const ai_short_size pool_pad_x,
                                const ai_short_size pool_pad_y);

#endif    /*LITE_CONV2D_H*/
