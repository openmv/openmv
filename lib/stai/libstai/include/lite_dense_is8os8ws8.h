/**
  ******************************************************************************
  * @file    lite_dense_is8os8ws8.h
  * @author  AIS
  * @brief   header file of AI platform lite dense kernel datatypes
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
#ifndef LITE_DENSE_IS8OS8WS8_H
#define LITE_DENSE_IS8OS8WS8_H

#include "ai_lite_interface.h"

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Forward function for a dense layer with signed input,
 *        signed output and signed weights all at 8 bits.
 * @ingroup lite_dense_is8os8ws8
 * @param input The pointer to input buffer.
 * @param output The pointer to output buffer.
 * @param weights The pointer to weights.
 * @param bias The pointer to bias (NULL if not available).
 * @param in_zeropoint The value of the zero point of the input.
 * @param out_zeropoint TThe value of the zero point of the output.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e.,
 *                      the number of dense hidden neurons.
 * @param n_pixels Total number of pixels.
 */
LITE_API_ENTRY
void forward_lite_dense_is8os8ws8(ai_i8 * pDataOut,
                                  const ai_i8 *pDataIn,
                                  const ai_i8 *pWeights,
                                  const ai_i32 *pBias,
                                  const ai_i8 in_zeropoint,
                                  const ai_i8 out_zeropoint,
                                  const ai_u16 n_channel_in,
                                  const ai_u16 n_channel_out,
                                  const ai_size n_pixels,
                                  const ai_float in_scale,
                                  const ai_float out_scale,
                                  const ai_float Wt_scale,
                                  ai_i16 *pBuffer_a);

void forward_lite_dense_hsp_is8os8ws8(ai_i8 * pDataOut,
                                  const ai_i8 *pDataIn,
                                  const ai_i8 *pWeights,
                                  const ai_i32 *pBias,
                                  const ai_i8 in_zeropoint,
                                  const ai_i8 out_zeropoint,
                                  const ai_u16 n_channel_in,
                                  const ai_u16 n_channel_out,
                                  const ai_size n_pixels,
                                  const ai_float in_scale,
                                  const ai_float out_scale,
                                  const ai_float Wt_scale);

void forward_lite_dense_hsp_3step_is8os8ws8(ai_i8 * pDataOut,
                                  const ai_i8 *pDataIn,
                                  const ai_i8 *pWeights,
                                  const ai_i32 *pBias,
                                  const ai_i8 in_zeropoint,
                                  const ai_i8 out_zeropoint,
                                  const ai_u16 n_channel_in,
                                  const ai_u16 n_channel_out,
                                  const ai_size n_pixels,
                                  const ai_float in_scale,
                                  const ai_float out_scale,
                                  const ai_float Wt_scale);

void forward_lite_dense_is8os8ws8_ch(ai_i8 * pDataOut,
                                     const ai_i8 *pDataIn,
                                     const ai_i8 *pWeights,
                                     const ai_i32 *pBias,
                                     const ai_i8 in_zeropoint,
                                     const ai_i8 out_zeropoint,
                                     const ai_u16 n_channel_in,
                                     const ai_u16 n_channel_out,
                                     const ai_size n_pixels,
                                     const ai_float in_scale,
                                     const ai_float out_scale,
                                     const ai_float *pWt_scale,
                                     ai_i16 *pBuffer_a);

#endif    /*LITE_DENSE_IS8OS8WS8_H*/
