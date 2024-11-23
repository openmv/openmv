/**
  ******************************************************************************
  * @file    lite_convert_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite convert kernel datatypes
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
#ifndef LITE_CONVERT_DQNN_H
#define LITE_CONVERT_DQNN_H


#include "ai_lite_interface.h"

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

LITE_API_ENTRY
void forward_lite_node_convert_is1os8(
  const ai_pbits *p_in,
  ai_i8 *p_out,
  const ai_i32 n_channels,
  const ai_i32 n_pixels,
  const ai_i8 *n_values);


LITE_API_ENTRY
void forward_lite_node_convert_is1os16(
  const ai_pbits *p_in,
  ai_i16 *p_out,
  const ai_i32 n_channels,
  const ai_i32 n_pixels,
  const ai_i16 *n_values);


LITE_API_ENTRY
void forward_lite_node_convert_is1of32(
  const ai_pbits *p_in,
  ai_float *p_out,
  const ai_i32 n_channels,
  const ai_i32 n_pixels,
  const ai_float *n_values);


/*!
 * @brief Handles data conversion from 8-bits signed input to signed binary
 *        outputs - Lite API version
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_node_convert_is8os1(
  const ai_i8 *p_in,
  ai_pbits *p_out,
  const ai_i32 n_channels,
  const ai_i32 n_pixels,
  const ai_i8 zp,
  const ai_i8 pad);


LITE_API_ENTRY
void forward_lite_node_convert_is16os1(
  const ai_i16 *p_in,
  ai_pbits *p_out,
  const ai_i32 n_channels,
  const ai_i32 n_pixels,
  const ai_i8 zp,
  const ai_i8 pad);


LITE_API_ENTRY
void forward_lite_node_convert_if32os1(
  const ai_float *p_in,
  ai_pbits *p_out,
  const ai_i32 n_channels,
  const ai_i32 n_pixels,
  const ai_i8 zp,
  const ai_i8 pad);


LITE_API_ENTRY
void forward_lite_node_convert_integer_if32os8(
  const ai_float *p_in,
  ai_i8 *p_out,
  const ai_u32 size,
  const ai_float out_scale,
  const ai_i8 out_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_integer_if32ou8(
  const ai_float *p_in,
  ai_u8 *p_out,
  const ai_u32 size,
  const ai_float out_scale,
  const ai_u8 out_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_integer_is8of32(
  const ai_i8 *p_in,
  ai_float *p_out,
  const ai_u32 size,
  const ai_float in_scale,
  const ai_i8 in_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_integer_iu8of32(
  const ai_u8 *p_in,
  ai_float *p_out,
  const ai_u32 size,
  const ai_float in_scale,
  const ai_u8 in_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_if32os16(
  const ai_float *p_in,
  ai_i16 *p_out,
  const ai_u32 size,
  const ai_float out_scale,
  const ai_i16 out_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_if32ou16(
  const ai_float *p_in,
  ai_u16 *p_out,
  const ai_u32 size,
  const ai_float out_scale,
  const ai_u16 out_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_is16of32(
  const ai_i16 *p_in,
  ai_float *p_out,
  const ai_u32 size,
  const ai_float in_scale,
  const ai_i16 in_zeropoint);


LITE_API_ENTRY
void forward_lite_node_convert_iu16of32(
  const ai_u16 *p_in,
  ai_float *p_out,
  const ai_u32 size,
  const ai_float in_scale,
  const ai_u16 in_zeropoint);

LITE_API_ENTRY
void forward_lite_node_convert_integer_is8os8(
  const ai_i8 *p_in,
  ai_i8 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_i16 in_zp,
  const ai_i16 out_zp);


LITE_API_ENTRY
void forward_lite_node_convert_integer_iu8ou8(
  const ai_u8 *p_in,
  ai_u8 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_u8 in_zp,
  const ai_u8 out_zp);


LITE_API_ENTRY
void forward_lite_node_convert_integer_iu8os8(
  const ai_u8 *p_in,
  ai_i8 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_u8 in_zp,
  const ai_i8 out_zp);


LITE_API_ENTRY
void forward_lite_node_convert_integer_iu8os8_fast(
  const ai_u8 *p_in,
  ai_i8 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_u8 in_zp,
  const ai_i8 out_zp);


LITE_API_ENTRY
void forward_lite_node_convert_integer_is8ou8(
  const ai_i8 *p_in,
  ai_u8 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_i8 in_zp,
  const ai_u8 out_zp);


LITE_API_ENTRY
void forward_lite_node_convert_integer_is8ou8_fast(
  const ai_i8 *p_in,
  ai_u8 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_i8 in_zp,
  const ai_u8 out_zp);


LITE_API_ENTRY
void forward_lite_node_convert_is16ou16(
  const ai_i16 *p_in,
  ai_u16 *p_out,
  const ai_i32 n_elems,
  const ai_float scale_ratio,
  const ai_i16 in_zp,
  const ai_u16 out_zp);

#endif    /*LITE_CONVERT_DQNN_H*/
