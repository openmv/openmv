/**
  ******************************************************************************
  * @file    lite_dw.h
  * @author  AIS
  * @brief   header file of AI platform lite depthwise kernel datatypes
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
#ifndef LITE_DW_H
#define LITE_DW_H


#include "ai_lite_interface.h"



/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles dw convolutions generic case (supports depth multiplier >= 1)
 * @ingroup lite_dw
 */
LITE_API_ENTRY
void
forward_lite_dw_dm_sssa8_ch(const ai_i8 *Im_in,
                            const ai_u16 dim_im_in_x,
                            const ai_u16 dim_im_in_y,
                            const ai_u16 ch_im_in,
                            const ai_i8 *wt,
                            const ai_u16 ch_im_out,
                            const ai_u16 dim_kernel_x,
                            const ai_u16 dim_kernel_y,
                            const ai_u16 padding_x,
                            const ai_u16 padding_y,
                            const ai_u16 stride_x,
                            const ai_u16 stride_y,
                            const ai_i32 *bias,
                            const ai_i8 In_ZeroPoint,
                            const ai_i8 Out_ZeroPoint,
                            const ai_float in_scale,
                            const ai_float out_scale,
                            const ai_float *pWt_scale,
                            ai_i8 *Im_out,
                            const ai_u16 dim_im_out_x,
                            const ai_u16 dim_im_out_y,
                            const ai_i32 nl_pool_fused,
                            const ai_u32 scratch_size,
                            ai_i16 *bufferA);

/*!
 * @brief Handles dw convolutions with depth multiplier = 1 only
 * @ingroup lite_dw
 */
LITE_API_ENTRY
void
forward_lite_dw_sssa8_ch(const ai_i8 *Im_in,
                         const ai_u16 dim_im_in_x,
                         const ai_u16 dim_im_in_y,
                         const ai_u16 ch_im_in,
                         const ai_i8 *wt,
                         const ai_u16 dim_kernel_x,
                         const ai_u16 dim_kernel_y,
                         const ai_u16 padding_x,
                         const ai_u16 padding_y,
                         const ai_u16 stride_x,
                         const ai_u16 stride_y,
                         const ai_i32 *bias,
                         const ai_i8 In_ZeroPoint,
                         const ai_i8 Out_ZeroPoint,
                         const ai_float in_scale,
                         const ai_float out_scale,
                         const ai_float *pWt_scale,
                         ai_i8 *Im_out,
                         const ai_u16 dim_im_out_x,
                         const ai_u16 dim_im_out_y,
                         const ai_i32 nl_pool_fused,
                         const ai_u32 scratch_size,
                         ai_i16 *bufferA);

/* Variant optimized for HSP */
void
forward_lite_dw_hsp_1step_sssa8_ch(const ai_i8 *Im_in,
                             const ai_u16 dim_im_in_x,
                             const ai_u16 dim_im_in_y,
                             const ai_u16 ch_im_in,
                             const ai_i8 *wt,
                             const ai_u16 dim_kernel_x,
                             const ai_u16 dim_kernel_y,
                             const ai_u16 padding_x,
                             const ai_u16 padding_x_r,
                             const ai_u16 padding_y,
                             const ai_u16 padding_y_b,
                             const ai_u16 stride_x,
                             const ai_u16 stride_y,
                             const ai_i32 *bias,
                             const ai_i8 In_ZeroPoint,
                             const ai_i8 Out_ZeroPoint,
                             const ai_float in_scale,
                             const ai_float out_scale,
                             const ai_float *pWt_scale,
                             ai_i8 *Im_out,
                             const ai_u16 dim_im_out_x,
                             const ai_u16 dim_im_out_y);

void
forward_lite_dw_hsp_2step_sssa8_ch(const ai_i8 *Im_in,
                             const ai_u16 dim_im_in_x,
                             const ai_u16 dim_im_in_y,
                             const ai_u16 ch_im_in,
                             const ai_i8 *wt,
                             const ai_u16 dim_kernel_x,
                             const ai_u16 dim_kernel_y,
                             const ai_u16 padding_x,
                             const ai_u16 padding_x_r,
                             const ai_u16 padding_y,
                             const ai_u16 padding_y_b,
                             const ai_u16 stride_x,
                             const ai_u16 stride_y,
                             const ai_i32 *bias,
                             const ai_i8 In_ZeroPoint,
                             const ai_i8 Out_ZeroPoint,
                             const ai_float in_scale,
                             const ai_float out_scale,
                             const ai_float *pWt_scale,
                             ai_i8 *Im_out,
                             const ai_u16 dim_im_out_x,
                             const ai_u16 dim_im_out_y);


/* Variant optimized for HSP: Large tensors */
void
forward_lite_dw_hsp_3step_sssa8_ch(const ai_i8 *Im_in,
                                   const ai_u16 dim_im_in_x,
                                   const ai_u16 dim_im_in_y,
                                   const ai_u16 ch_im_in,
                                   const ai_i8 *wt,
                                   const ai_u16 dim_kernel_x,
                                   const ai_u16 dim_kernel_y,
                                   const ai_u16 padding_x,
                                   const ai_u16 padding_x_r,
                                   const ai_u16 padding_y,
                                   const ai_u16 padding_y_b,
                                   const ai_u16 stride_x,
                                   const ai_u16 stride_y,
                                   const ai_i32 *bias,
                                   const ai_i8 In_ZeroPoint,
                                   const ai_i8 Out_ZeroPoint,
                                   const ai_float in_scale,
                                   const ai_float out_scale,
                                   const ai_float *pWt_scale,
                                   ai_i8 *Im_out,
                                   const ai_u16 dim_im_out_x,
                                   const ai_u16 dim_im_out_y);

/*!
 * @brief Handles dw convolutions with depth multiplier = 1, valid padding
 *        and 3*3 kernel size
 * @ingroup lite_dw
 */
LITE_API_ENTRY
void
forward_lite_dw_3x3_sssa8_ch(const ai_i8 *Im_in,
                             const ai_u16 dim_im_in_x,
                             const ai_u16 dim_im_in_y,
                             const ai_u16 ch_im_in,
                             const ai_i8 *wt,
                             const ai_u16 stride_x,
                             const ai_u16 stride_y,
                             const ai_i32 *bias,
                             const ai_i8 In_ZeroPoint,
                             const ai_i8 Out_ZeroPoint,
                             const ai_float in_scale,
                             const ai_float out_scale,
                             const ai_float *pWt_scale,
                             ai_i8 *Im_out,
                             const ai_u16 dim_im_out_x,
                             const ai_u16 dim_im_out_y,
                             const ai_i32 nl_pool_fused,
                             const ai_u32 scratch_size,
                             ai_i16 *bufferA);

/*!
 * @brief Handles dw convolutions with depth multiplier = 1, valid padding,
 *        3*3 kernel size, stride_x = 1 and weights/input are channel first
 * @ingroup lite_dw
 */
LITE_API_ENTRY
void
forward_lite_dw_3x3_ch1st_sssa8_ch(const ai_i8 *Im_in,
                                   const ai_u16 dim_im_in_x,
                                   const ai_u16 dim_im_in_y,
                                   const ai_u16 ch_im_in,
                                   const ai_i8 *wt,
                                   const ai_u16 stride_x,
                                   const ai_u16 stride_y,
                                   const ai_i32 *bias,
                                   const ai_i8 In_ZeroPoint,
                                   const ai_i8 Out_ZeroPoint,
                                   const ai_float in_scale,
                                   const ai_float out_scale,
                                   const ai_float *pWt_scale,
                                   ai_i8 *Im_out,
                                   const ai_u16 dim_im_out_x,
                                   const ai_u16 dim_im_out_y,
                                   const ai_i32 nl_pool_fused,
                                   const ai_u32 scratch_size,
                                   ai_i16 *bufferA);


/*!
 * @brief Handles dw convolutions with depth multiplier = 1, valid padding,
 *        1*N kernel size, stride_x = 1
 * @ingroup lite_dw
 */
LITE_API_ENTRY
void
forward_lite_dw_1xN_sssa8_ch(const ai_i8 *Im_in,
                             const ai_u16 dim_im_in_y,
                             const ai_u16 ch_im_in,
                             const ai_i8 *wt,
                             const ai_u16 dim_kernel_y,
                             const ai_i32 *bias,
                             const ai_i8 In_ZeroPoint,
                             const ai_i8 Out_ZeroPoint,
                             const ai_float in_scale,
                             const ai_float out_scale,
                             const ai_float *pWt_scale,
                             ai_i8 *Im_out,
                             const ai_u16 dim_im_out_y,
                             const ai_i32 nl_pool_fused,
                             const ai_u32 scratch_size,
                             ai_i16 *bufferA);



#endif    /*LITE_DW_H*/
