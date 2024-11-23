/**
  ******************************************************************************
  * @file    lite_pad_generic.h
  * @author  AIS
  * @brief   header file of AI platform lite padding kernel datatypes
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
#ifndef LITE_PAD_GENERIC_H
#define LITE_PAD_GENERIC_H


#include "ai_lite_interface.h"

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles padding with 8 bits input/output in constant mode - Lite I/F
 * Channel 1st Format Input and Output
 * @ingroup lite_padding_dqnn
 */

/* Variant used for padding pattern = (1, 1, 1, 1) */
LITE_API_ENTRY
void forward_lite_pad_8bit_ch1st_3x3_constant_P1111(ai_ptr_const in_data_tensor,
                                              ai_ptr out_data_tensor,
                                              const ai_handle fill_value,
                                              const ai_i32 height_in,
                                              const ai_i32 channel_in,
                                              const ai_ptr_offset ch_stride_in,
                                              const ai_ptr_offset h_stride_in,
                                              const ai_ptr_offset h_stride_pad);

/* Variant used for padding pattern = (0, 0, 2, 2) */
LITE_API_ENTRY
void forward_lite_pad_8bit_ch1st_3x3_constant_P0022(ai_ptr_const in_data_tensor,
                                              ai_ptr out_data_tensor,
                                              const ai_handle fill_value,
                                              const ai_i32 height_in,
                                              const ai_i32 channel_in,
                                              const ai_ptr_offset ch_stride_in,
                                              const ai_ptr_offset h_stride_in,
                                              const ai_ptr_offset h_stride_pad);

/*!
 * @brief Handles padding with 8 bits input/output in constant mode - Lite I/F
 * @ingroup lite_padding_dqnn
 */
LITE_API_ENTRY
void forward_lite_pad_constant(ai_ptr_const in_data,
                               ai_ptr out_data,
                               const ai_handle fill_value,
                               const ai_i16 in_bits,
                               const ai_i32 height_in,
                               const ai_ptr_offset ch_stride_in,
                               const ai_ptr_offset h_stride_in,
                               const ai_ptr_offset h_stride_pad,
                               const ai_ptr_offset h_stride_pad_b,
                               const ai_ptr_offset w_stride_pad,
                               const ai_ptr_offset w_stride_pad_r);

/*!
 * @brief Handles padding with 8 bits input/output in edge mode - Lite I/F
 * @ingroup lite_padding_dqnn
 */
void forward_lite_pad_edge(ai_ptr_const in_data_tensor,
                           ai_ptr out_data,
                           const ai_i32 height_in,
                           const ai_i16 pads_y,
                           const ai_i16 pads_x_r,
                           const ai_ptr_offset h_stride_in,
                           const ai_ptr_offset w_stride_in,
                           const ai_ptr_offset h_stride_out,
                           const ai_ptr_offset h_stride_pad,
                           const ai_ptr_offset w_stride_pad,
                           const ai_ptr_offset h_stride_pad_b);

/*!
 * @brief Handles padding with 8 bits input/output in reflect mode - Lite I/F
 * @ingroup lite_padding_dqnn
 */
void forward_lite_pad_reflect(ai_ptr_const in_data,
                              ai_ptr out_data,
                              const ai_i32 depth,
                              const ai_i32 height_in,
                              const ai_i32 width_in,
                              const ai_i32 height_out,
                              const ai_i32 width_out,
                              const ai_ptr_offset h_stride_in,
                              const ai_ptr_offset w_stride_in,
                              const ai_ptr_offset h_stride_out,
                              const ai_ptr_offset w_stride_out,
                              const ai_i16 pads_x,
                              const ai_i16 pads_y,
                              const ai_i16 pads_y_b,
                              const ai_ptr_offset h_stride_pad,
                              const ai_ptr_offset w_stride_pad,
                              const ai_ptr_offset w_stride_pad_r);

#endif    /* LITE_PAD_GENERIC_H */
