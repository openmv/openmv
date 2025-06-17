
/**
  ******************************************************************************
  * @file    lite_dense_is16.h
  * @author  Giacomo Turati
  * @brief   header file of AI platform lite dense kernel (with signed int16 input)
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
#ifndef LITE_DENSE_IS16_H
#define LITE_DENSE_IS16_H


#include "stai.h"
#include "ai_lite_interface.h"

/*!
 * @brief Dense layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support signed integer 16 input and signed integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @ingroup lite_dense_ws16
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 */
LITE_API_ENTRY
void forward_lite_dense_is16os16ws16_fxp(
                          int16_t*            output,
                          const int16_t*      input,
                          const int16_t*      weights,
                          const int16_t*      bias,
                          const uint32_t      n_channel_in,
                          const uint32_t      n_channel_out,
                          const uint8_t*      shifts,
                          const uint8_t*      bias_shifts
);

/*!
 * @brief Dense layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support signed integer 16 input and unsigned integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @ingroup lite_dense_ws16
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 */

LITE_API_ENTRY
void forward_lite_dense_is16ou16ws16_fxp(
                          uint16_t*           output,
                          const int16_t*      input,
                          const int16_t*      weights,
                          const int16_t*      bias,
                          const uint32_t      n_channel_in,
                          const uint32_t      n_channel_out,
                          const uint8_t*      shifts,
                          const uint8_t*      bias_shifts
);

/*!
 * @brief Dense layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support unsigned integer 16 input and signed integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @ingroup lite_dense_ws16
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 */
LITE_API_ENTRY
void forward_lite_dense_iu16os16ws16_fxp(
                          int16_t*            output,
                          const uint16_t*     input,
                          const int16_t*      weights,
                          const int16_t*      bias,
                          const uint32_t      n_channel_in,
                          const uint32_t      n_channel_out,
                          const uint8_t*      shifts,
                          const uint8_t*      bias_shifts
);

/*!
 * @brief Dense layer with fixed-point int16_t weights (e.g., Qkeras "auto_po2").
 * Support unsigned integer 16 input and unsigned integer 16 output activations.
 * Both weights and bias (if any) must be quantized with 16 bits.
 * Manage different fixed-point scales between weights and bias (if any).
 * @ingroup lite_dense_ws16
 * @param output The pointer to output buffer.
 * @param input The pointer to input buffer.
 * @param weights The pointer to weights.
 * @param n_channel_in The number of channels of the input.
 * @param n_channel_out The number of channels of the output, i.e.,
 *        the number of dense hidden neurons.
 * @param shifts Array of fixed-point binary scales for the weights
 * @param bias_shifts Array of fixed-point binary scales for the bias
 */

/*!
 * @brief Signed input / signed output API wrapper for _conv2d_ws16_fxp_backend.
 * @ingroup lite_conv2d_ws16
 */
LITE_API_ENTRY
void forward_lite_dense_iu16ou16ws16_fxp(
                          uint16_t*           output,
                          const uint16_t*     input,
                          const int16_t*      weights,
                          const int16_t*      bias,
                          const uint32_t      n_channel_in,
                          const uint32_t      n_channel_out,
                          const uint8_t*      shifts,
                          const uint8_t*      bias_shifts
);

#endif    /* LITE_DENSE_IS16_H */
