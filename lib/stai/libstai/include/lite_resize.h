/**
  ******************************************************************************
  * @file    lite_resize.h
  * @author  AIS
  * @brief   header file of AI platform lite resize kernel datatypes
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
#ifndef LITE_RESIZE_H
#define LITE_RESIZE_H
#pragma once

#include "ai_lite_interface.h"

void forward_lite_resize_nearest(ai_ptr in_data,
                                 ai_ptr out_data,
                                 const ai_size width_in,
                                 const ai_size height_in,
                                 const ai_size n_channel_in,
                                 const ai_ptr_offset stride_ch,
                                 const ai_float width_scale,
                                 const ai_float height_scale,
                                 const ai_size width_out,
                                 const ai_size height_out,
                                 const ai_nearest_mode mode,
                                 const ai_coord_transf_mode coord_transf_mode,
                                 const ai_handle extrapol_val,
                                 const ai_float* roi);

void forward_lite_resize_bilinear_if32of32( const ai_float* in_data,
                                            ai_float* out_data,
                                            const ai_size width_in,
                                            const ai_size height_in,
                                            const ai_size n_channel_in,
                                            const ai_float width_scale,
                                            const ai_float height_scale,
                                            const ai_size width_out,
                                            const ai_size height_out,
                                            const ai_coord_transf_mode coord_transf_mode,
                                            const ai_handle extrapol_val,
                                            const ai_float* roi);

void forward_lite_resize_bilinear_is8os8( const ai_i8* in_data,
                                            ai_i8* out_data,
                                            const ai_size width_in,
                                            const ai_size height_in,
                                            const ai_size n_channel_in,
                                            const ai_float width_scale,
                                            const ai_float height_scale,
                                            const ai_size width_out,
                                            const ai_size height_out,
                                            const ai_coord_transf_mode coord_transf_mode,
                                            const ai_handle extrapol_val,
                                            const ai_float* roi);

void forward_lite_resize_bilinear_is16os16( const ai_i16* in_data,
                                            ai_i16* out_data,
                                            const ai_size width_in,
                                            const ai_size height_in,
                                            const ai_size n_channel_in,
                                            const ai_float width_scale,
                                            const ai_float height_scale,
                                            const ai_size width_out,
                                            const ai_size height_out,
                                            const ai_coord_transf_mode coord_transf_mode,
                                            const ai_handle extrapol_val,
                                            const ai_float* roi);

#endif    /*LITE_RESIZE__H*/
