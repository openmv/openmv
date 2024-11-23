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
#ifndef LITE_UPSAMPLE_H
#define LITE_UPSAMPLE_H
#pragma once

#include "ai_lite_interface.h"


/**
 * @brief Function to upsample in bilinear mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are float.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 * @param[in] center centered coordinates
 */
void forward_lite_upsample_bilinear_if32of32(const ai_float* in_data,
                                    ai_float* out_data,
                                    const ai_size width_in,
                                    const ai_size height_in,
                                    const ai_float width_scale,
                                    const ai_float height_scale,
                                    const ai_size width_out,
                                    const ai_size height_out,
                                    const ai_size n_channel,
                                    const ai_bool center);

/**
 * @brief Function to upsample in bilinear mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are signed int8.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 * @param[in] center centered coordinates
 */
void forward_lite_upsample_bilinear_is8os8(const ai_i8* in_data,
                                    ai_i8* out_data,
                                    const ai_size width_in,
                                    const ai_size height_in,
                                    const ai_float width_scale,
                                    const ai_float height_scale,
                                    const ai_size width_out,
                                    const ai_size height_out,
                                    const ai_size n_channel,
                                    const ai_bool center);

/**
 * @brief Function to upsample in bilinear mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are unsinged int8.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 * @param[in] center centered coordinates
 */
void forward_lite_upsample_bilinear_iu8ou8(const ai_u8* in_data,
                                    ai_u8* out_data,
                                    const ai_size width_in,
                                    const ai_size height_in,
                                    const ai_float width_scale,
                                    const ai_float height_scale,
                                    const ai_size width_out,
                                    const ai_size height_out,
                                    const ai_size n_channel,
                                    const ai_bool center);

/**
 * @brief Function to upsample in bilinear mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are signed int16.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] center centered coordinates
 * @param[in] channel_in input/output channels.
 */
void forward_lite_upsample_bilinear_is16os16(const ai_i16* in_data,
                                    ai_i16* out_data,
                                    const ai_size width_in,
                                    const ai_size height_in,
                                    const ai_float width_scale,
                                    const ai_float height_scale,
                                    const ai_size width_out,
                                    const ai_size height_out,
                                    const ai_size n_channel,
                                    const ai_bool center);

/**
 * @brief Function to upsample in bilinear mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are unsigned int16.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 * @param[in] center centered coordinates
 */
void forward_lite_upsample_bilinear_iu16ou16(const ai_u16* in_data,
                                    ai_u16* out_data,
                                    const ai_size width_in,
                                    const ai_size height_in,
                                    const ai_float width_scale,
                                    const ai_float height_scale,
                                    const ai_size width_out,
                                    const ai_size height_out,
                                    const ai_size n_channel,
                                    const ai_bool center);

/**
 * @brief Function to upsample in zero mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are singed int8.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 * @param[in] zero_s8 out zeropoint value
 */
void forward_lite_upsample_zeros_is8os8( const ai_i8 *in_data,
                                         ai_i8 *out_data,
                                         const ai_size width_in,
                                         const ai_size height_in,
                                         const ai_float width_scale,
                                         const ai_float height_scale,
                                         const ai_size width_out,
                                         const ai_size height_out,
                                         const ai_size channel_in,
                                         const ai_i8 zero_s8);

/**
 * @brief Function to upsample in zero mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are signed int16.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 * @param[in] zero_s16 out zeropoint value
 */
void forward_lite_upsample_zeros_is16os16( const ai_i16 *in_data,
                                         ai_i16 *out_data,
                                         const ai_size width_in,
                                         const ai_size height_in,
                                         const ai_float width_scale,
                                         const ai_float height_scale,
                                         const ai_size channel_in,
                                         const ai_size width_out,
                                         const ai_size height_out,
                                         const ai_i16 zero_s16);

/**
 * @brief Function to upsample in zero mode.
 *        The number of output channels is the same as the number of input channels.
 *        Input and output types are float.
 *
 * @param[in] in_data input data, to be upsampled
 * @param[out] out_data upsampled output data
 * @param[in] width_in input data width
 * @param[in] height_in input data height
 * @param[in] width_scale width_out/width_in scale ratio
 * @param[in] height_scale height_out/height_in scale ratio
 * @param[in] width_out output data width
 * @param[in] height_out output data height
 * @param[in] channel_in input/output channels.
 */
void forward_lite_upsample_zeros_if32of32( const ai_float *in_data,
                                         ai_float *out_data,
                                         const ai_size width_in,
                                         const ai_size height_in,
                                         const ai_float width_scale,
                                         const ai_float height_scale,
                                         const ai_size channel_in,
                                         const ai_size width_out,
                                         const ai_size height_out);

#endif    /*LITE_UPSAMPLE__H*/
