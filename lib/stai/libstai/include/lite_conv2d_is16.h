/**
  ******************************************************************************
  * @file    lite_dense_is16.h
  * @author  Giacomo Turati
  * @brief   header file of AI platform lite conv2d kernel (with signed int16 input)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LITE_CONV2D_IS16_H
#define LITE_CONV2D_IS16_H


#include "stai.h"
#include "ai_lite_interface.h"

/*!
 * @brief Conv2d layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support signed integer 16 input and signed integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @param output Pointer to the output buffer
 * @param input Pointer to the input buffer
 * @param weights Pointer to the weights array
 * @param n_channel_in Number of input channels
 * @param n_channel_out Number of output channels, i.e.,the number of conv2d hidden filters
 * @param width_in Input width
 * @param height_in Input height
 * @param width_out Output width
 * @param height_out Output height
 * @param filt_width Filters width
 * @param filt_height Filters height
 * @param filt_pad_x Filters pad width
 * @param filt_pad_y Filters pad height
 * @param stride_x Stride width
 * @param stride_y Stride height
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 * @param signed_input Signed input flag
 * @param signed_output Signed output flag
*/
LITE_API_ENTRY
void forward_lite_conv2d_is16os16ws16_fxp(
                          int16_t*           output,
                          const int16_t*     input,
                          const int16_t*     weights,
                          const int16_t*     bias,
                          const ai_size      n_channel_in,
                          const ai_size      n_channel_out,
                          const ai_size      width_in,
                          const ai_size      height_in,
                          const ai_size      width_out,
                          const ai_size      height_out,
                          const ai_size      filt_width,
                          const ai_size      filt_height,
                          const ai_size      filt_pad_x,
                          const ai_size      filt_pad_y,
                          const uint16_t     stride_x,
                          const uint16_t     stride_y,
                          const uint8_t*     shifts,
                          const uint8_t*     bias_shifts
               );

/*!
 * @brief Conv2d layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support signed integer 16 input and unsigned integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @param output Pointer to the output buffer
 * @param input Pointer to the input buffer
 * @param weights Pointer to the weights array
 * @param n_channel_in Number of input channels
 * @param n_channel_out Number of output channels, i.e.,the number of conv2d hidden filters
 * @param width_in Input width
 * @param height_in Input height
 * @param width_out Output width
 * @param height_out Output height
 * @param filt_width Filters width
 * @param filt_height Filters height
 * @param filt_pad_x Filters pad width
 * @param filt_pad_y Filters pad height
 * @param stride_x Stride width
 * @param stride_y Stride height
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 * @param signed_input Signed input flag
 * @param signed_output Signed output flag
*/
LITE_API_ENTRY
void forward_lite_conv2d_is16ou16ws16_fxp(
                          uint16_t*           output,
                          const int16_t*     input,
                          const int16_t*     weights,
                          const int16_t*     bias,
                          const ai_size      n_channel_in,
                          const ai_size      n_channel_out,
                          const ai_size      width_in,
                          const ai_size      height_in,
                          const ai_size      width_out,
                          const ai_size      height_out,
                          const ai_size      filt_width,
                          const ai_size      filt_height,
                          const ai_size      filt_pad_x,
                          const ai_size      filt_pad_y,
                          const uint16_t     stride_x,
                          const uint16_t     stride_y,
                          const uint8_t*     shifts,
                          const uint8_t*     bias_shifts
               );

/*!
 * @brief Conv2d layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support unsigned integer 16 input and signed integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @param output Pointer to the output buffer
 * @param input Pointer to the input buffer
 * @param weights Pointer to the weights array
 * @param n_channel_in Number of input channels
 * @param n_channel_out Number of output channels, i.e.,the number of conv2d hidden filters
 * @param width_in Input width
 * @param height_in Input height
 * @param width_out Output width
 * @param height_out Output height
 * @param filt_width Filters width
 * @param filt_height Filters height
 * @param filt_pad_x Filters pad width
 * @param filt_pad_y Filters pad height
 * @param stride_x Stride width
 * @param stride_y Stride height
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 * @param signed_input Signed input flag
 * @param signed_output Signed output flag
*/
LITE_API_ENTRY
void forward_lite_conv2d_iu16os16ws16_fxp(
                          int16_t*           output,
                          const uint16_t*     input,
                          const int16_t*     weights,
                          const int16_t*     bias,
                          const ai_size      n_channel_in,
                          const ai_size      n_channel_out,
                          const ai_size      width_in,
                          const ai_size      height_in,
                          const ai_size      width_out,
                          const ai_size      height_out,
                          const ai_size      filt_width,
                          const ai_size      filt_height,
                          const ai_size      filt_pad_x,
                          const ai_size      filt_pad_y,
                          const uint16_t     stride_x,
                          const uint16_t     stride_y,
                          const uint8_t*     shifts,
                          const uint8_t*     bias_shifts
               );

/*!
 * @brief Conv2d layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support unsigned integer 16 input and unsigned integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @param output Pointer to the output buffer
 * @param input Pointer to the input buffer
 * @param weights Pointer to the weights array
 * @param n_channel_in Number of input channels
 * @param n_channel_out Number of output channels, i.e.,the number of conv2d hidden filters
 * @param width_in Input width
 * @param height_in Input height
 * @param width_out Output width
 * @param height_out Output height
 * @param filt_width Filters width
 * @param filt_height Filters height
 * @param filt_pad_x Filters pad width
 * @param filt_pad_y Filters pad height
 * @param stride_x Stride width
 * @param stride_y Stride height
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 * @param signed_input Signed input flag
 * @param signed_output Signed output flag
*/
LITE_API_ENTRY
void forward_lite_conv2d_iu16ou16ws16_fxp(
                          uint16_t*           output,
                          const uint16_t*     input,
                          const int16_t*     weights,
                          const int16_t*     bias,
                          const ai_size      n_channel_in,
                          const ai_size      n_channel_out,
                          const ai_size      width_in,
                          const ai_size      height_in,
                          const ai_size      width_out,
                          const ai_size      height_out,
                          const ai_size      filt_width,
                          const ai_size      filt_height,
                          const ai_size      filt_pad_x,
                          const ai_size      filt_pad_y,
                          const uint16_t     stride_x,
                          const uint16_t     stride_y,
                          const uint8_t*     shifts,
                          const uint8_t*     bias_shifts
               );

#endif    /* LITE_CONV2D_IS16_H */
