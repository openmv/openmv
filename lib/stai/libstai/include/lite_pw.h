/**
  ******************************************************************************
  * @file    lite_pw.h
  * @author  AIS
  * @brief   header file of AI platform lite pointwise kernel datatypes
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
#ifndef LITE_PW_H
#define LITE_PW_H


#include "ai_lite_interface.h"



/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles pw convolutions generic case
 * @ingroup lite_pw
 */
LITE_API_ENTRY
void
forward_lite_pw_sssa8_ch(const ai_i8 *pData_in,
                         const ai_u16 width_in,
                         const ai_u16 height_in,
                         const ai_u16 filt_stride_x,
                         const ai_u16 filt_stride_y,
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
                         ai_u16 weights_prefetch_enabled,
                         ai_i32 scratch_size,
                         ai_i16 *pBuffer_a);

void
forward_lite_pw_hsp_sssa8_ch(const ai_i8 *pData_in,
                         const ai_u16 width_in,
                         const ai_u16 height_in,
                         const ai_u16 filt_stride_x,
                         const ai_u16 filt_stride_y,
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
                         ai_i32 scratch_size,
                         ai_i16 *pBuffer_a);

void 
forward_lite_pw_hsp_1step_sssa8_ch(const ai_i8 *pData_in,
                         const ai_u16 width_in,
                         const ai_u16 height_in,
                         const ai_u16 filt_stride_x,
                         const ai_u16 filt_stride_y,
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
                         ai_i32 scratch_size,
                         ai_i16 *pBuffer_a);


void 
forward_lite_pw_hsp_3step_sssa8_ch(const ai_i8 *pData_in,
                         const ai_u16 width_in,
                         const ai_u16 height_in,
                         const ai_u16 filt_stride_x,
                         const ai_u16 filt_stride_y,
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
                         ai_i32 scratch_size,
                         ai_i16 *pBuffer_a);

#endif    /*LITE_PW_H*/
