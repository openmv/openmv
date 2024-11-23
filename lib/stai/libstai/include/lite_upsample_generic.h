/**
  ******************************************************************************
  * @file    lite_upsample.h
  * @author  AIS
  * @brief   header file of AI platform lite upsample kernel datatypes
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
#ifndef LITE_UPSAMPLE_GENERIC_H
#define LITE_UPSAMPLE_GENERIC_H


#include "ai_lite_interface.h"


void forward_lite_upsample_generic_nearest(const ai_u8* in_data,
                                           ai_u8* out_data,
                                           const ai_size width_in,
                                           const ai_size width_out,
                                           const ai_float width_scale,
                                           const ai_size height_out,
                                           const ai_float height_scale,
                                           const ai_u32 output_tensor_w_stride,
                                           const ai_float offset_round_coeff);

void forward_lite_upsample_nearest(ai_ptr in_data,
                                   ai_ptr out_data,
                                   const ai_size width_in,
                                   const ai_size height_in,
                                   const ai_float width_scale,
                                   const ai_float height_scale,
                                   const ai_size width_out,
                                   const ai_size height_out,
                                   const ai_ptr_offset stride_w,
                                   const ai_float offset_round_coeff);

void forward_lite_upsample_zeros( ai_ptr in_data,
                                  ai_ptr out_data,
                                  const ai_size width_in,
                                  const ai_size height_in,
                                  const ai_float width_scale,
                                  const ai_float height_scale,
                                  const ai_size width_out,
                                  const ai_size height_out,
                                  const ai_ptr_offset stride_ch,
                                  const ai_ptr_offset stride_w,
                                  const ai_handle p_zero_value);

#endif    /*LITE_UPSAMPLE_GENERIC_H*/
