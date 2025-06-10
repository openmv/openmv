/**
  ******************************************************************************
  * @file    lite_argminmax.h
  * @author  AIS
  * @brief   header file of AI platform lite argmin argmax funcions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LITE_ARGMINMAX_H
#define LITE_ARGMINMAX_H

#include "ai_lite_interface.h"


void forward_lite_argmax_if32( const ai_float* in_data,
                              ai_u32* dst_out,
                              const ai_size width_in,
                              const ai_size height_in,
                              const ai_size n_channel_in,
                              const ai_i16 axis,
                              const ai_i16 select_last_index);
void forward_lite_argmin_if32( const ai_float* in_data,
                              ai_u32* dst_out,
                              const ai_size width_in,
                              const ai_size height_in,
                              const ai_size n_channel_in,
                              const ai_i16 axis,
                              const ai_i16 select_last_index);
void forward_lite_argmax_is8( const ai_i8* in_data,
                              ai_u32* dst_out,
                              const ai_size width_in,
                              const ai_size height_in,
                              const ai_size n_channel_in,
                              const ai_i16 axis,
                              const ai_i16 select_last_index);
void forward_lite_argmax_iu8( const ai_u8* in_data,
                              ai_u32* dst_out,
                              const ai_size width_in,
                              const ai_size height_in,
                              const ai_size n_channel_in,
                              const ai_i16 axis,
                              const ai_i16 select_last_index);
void forward_lite_argmin_is8( const ai_i8* in_data,
                              ai_u32* dst_out,
                              const ai_size width_in,
                              const ai_size height_in,
                              const ai_size n_channel_in,
                              const ai_i16 axis,
                              const ai_i16 select_last_index);


#endif    /*LITE_ARGMINMAX_H*/
