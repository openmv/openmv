/**
  ******************************************************************************
  * @file    lite_bn_integer.h
  * @author  AIS
  * @brief   header file of AI platform lite integer batch normalization
  *          normalization functions
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
#ifndef LITE_BN_INTEGER_H
#define LITE_BN_INTEGER_H


#include "ai_lite_interface.h"

/**
 * @brief Batch Normalization with 16-bit input, 16-bit threshold and binary output.
 * It is implemented using a threshold, and this is possible because the output is binary.
 *
 * @param[in] pIn Input data pointer
 * @param[out] pOut_32 Output data pointer
 * @param[in] pThreshold Thresholds pointer (one per channel)
 * @param[in] dim_x X dimension
 * @param[in] dim_y Y dimension
 * @param[in] channels_num Channels number
 */
LITE_API_ENTRY
void forward_lite_bn_is16os1ws16(const ai_i16 *pIn,
                                 ai_u32 *pOut_32,
                                 const ai_i16 *pThreshold,
                                 const ai_i16 dim_x,
                                 const ai_i16 dim_y,
                                 const ai_i16 channels_num);





/**
 * @brief Batch Normalization with signed 8-bit input, output.
 *
 * @param[in]  p_in Input data pointer
 * @param[out] p_out Output data pointer
 * @param[in]  n_channel_inout nb channels
 * @param[in]  n_elements nb elements
 * @param[in]  in_scale     input scale
 * @param[in]  in_zeropoint input zero point
 * @param[in]  out_scale output scale
 * @param[in]  out_zeropoint output zero point
 * @param[in]  pSc_scale  pointer on scale scales
 * @param[in]  pSc_zeropoint  pointer on scale zero_point
 * @param[in]  pScale_data   pointer on scale input
 * @param[in]  pBias_scale  pointer on bias scales
 * @param[in]  pBias_zeropoint  pointer on bias zero_point
 * @param[in]  pBias_data   pointer on bias input
 * @param[in]  bnl_param_sign  sign of BNL parameters (0=unsigned, 1=signed)
 * @param[in]  pBuffer_a scratch buffer for:
 *              out factor: nb channels * sizeof(ai_i32) +
 *              out offset: nb channels * sizeof(ai_i32) +
 *              out shift: nb channels * sizeof(ai_i16)
 */
void forward_lite_bn_is8os8( const ai_i8 *p_in,
                        ai_i8 *p_out,
                        ai_size n_channel_inout,
                        ai_size n_elements,
                        ai_float in_scale,
                        const ai_i32 in_zeropoint,
                        ai_float out_scale,
                        const ai_i32 out_zeropoint,
                        const ai_float *pSc_scale,
                        const ai_i8 *pSc_zeropoint,
                        const ai_i8 *pData_scale,
                        const ai_float *pBias_scale,
                        const ai_i8 *pBias_zeropoint,
                        const ai_i8 *pData_bias,
                        ai_i16 bnl_param_sign,
                        ai_i32 scratch_size,
                        ai_i16 *pBuffer_a);

/**
 * @brief Batch Normalization with signed 8-bit input, output
 *              per channel quantization
 *
 *
 * @param[in]  p_in Input data pointer
 * @param[out] p_out Output data pointer
 * @param[in]  n_channel_inout nb channels
 * @param[in]  n_elements nb elements
 * @param[in]  in_scale     input scale
 * @param[in]  in_zeropoint input zero point
 * @param[in]  out_scale output scale
 * @param[in]  out_zeropoint output zero point
 * @param[in]  pSc_scale  pointer on scale scales
 * @param[in]  pSc_zeropoint  pointer on scale zero_point
 * @param[in]  pScale_data   pointer on scale input
 * @param[in]  pBias_scale  pointer on bias scales
 * @param[in]  pBias_zeropoint  pointer on bias zero_point
 * @param[in]  pBias_data   pointer on bias input
 * @param[in]  bnl_param_sign  sign of BNL parameters (0=unsigned, 1=signed)
 * @param[in]  pBuffer_a scratch buffer for:
 *              out factor: nb channels * sizeof(ai_i32) +
 *              out offset: nb channels * sizeof(ai_i32) +
 *              out shift: nb channels * sizeof(ai_i16)
 */
void forward_lite_bn_is8os8_ch( const ai_i8 *p_in,
                        ai_i8 *p_out,
                        ai_size n_channel_inout,
                        ai_size n_elements,
                        ai_float in_scale,
                        const ai_i32 in_zeropoint,
                        ai_float out_scale,
                        const ai_i32 out_zeropoint,
                        const ai_float *pSc_scale,
                        const ai_i8 *pSc_zeropoint,
                        const ai_i8 *pData_scale,
                        const ai_float *pBias_scale,
                        const ai_i8 *pBias_zeropoint,
                        const ai_i8 *pData_bias,
                        ai_i16 bnl_param_sign,
                        ai_i32 scratch_size,
                        ai_i16 *pBuffer_a);

/**
 * @brief Batch Normalization with unsigned 8-bit input, output
 *
 *
 * @param[in]  p_in Input data pointer
 * @param[out] p_out Output data pointer
 * @param[in]  n_channel_inout nb channels
 * @param[in]  n_elements nb elements
 * @param[in]  in_scale     input scale
 * @param[in]  in_zeropoint input zero point
 * @param[in]  out_scale output scale
 * @param[in]  out_zeropoint output zero point
 * @param[in]  pSc_scale  pointer on scale scales
 * @param[in]  pSc_zeropoint  pointer on scale zero_point
 * @param[in]  pScale_data   pointer on scale input
 * @param[in]  pBias_scale  pointer on bias scales
 * @param[in]  pBias_zeropoint  pointer on bias zero_point
 * @param[in]  pBias_data   pointer on bias input
 * @param[in]  bnl_param_sign  sign of BNL parameters (0=unsigned, 1=signed)
 * @param[in]  pBuffer_a scratch buffer for:
 *              out factor: nb channels * sizeof(ai_i32) +
 *              out offset: nb channels * sizeof(ai_i32) +
 *              out shift: nb channels * sizeof(ai_i16)
 */
void forward_lite_bn_iu8ou8( const ai_u8 *p_in,
                        ai_u8 *p_out,
                        ai_size n_channel_inout,
                        ai_size n_elements,
                        ai_float in_scale,
                        const ai_i32 in_zeropoint_32,
                        ai_float out_scale,
                        const ai_i32 out_zeropoint_32,
                        const ai_float *pSc_scale,
                        const ai_i8 *pSc_zeropoint,
                        const ai_i8 *pData_scale,
                        const ai_float *pBias_scale,
                        const ai_i8 *pBias_zeropoint,
                        const ai_i8 *pData_bias,
                        ai_i16 bnl_param_sign,
                        ai_i32 scratch_size,
                        ai_i16 *pBuffer_a);

/**
 * @brief Batch Normalization with unsigned 8-bit input, output
 *              per channel quantization
 *
 *
 * @param[in]  p_in Input data pointer
 * @param[out] p_out Output data pointer
 * @param[in]  n_channel_inout nb channels
 * @param[in]  n_elements nb elements
 * @param[in]  in_scale     input scale
 * @param[in]  in_zeropoint input zero point
 * @param[in]  out_scale output scale
 * @param[in]  out_zeropoint output zero point
 * @param[in]  pSc_scale  pointer on scale scales
 * @param[in]  pSc_zeropoint  pointer on scale zero_point
 * @param[in]  pScale_data   pointer on scale input
 * @param[in]  pBias_scale  pointer on bias scales
 * @param[in]  pBias_zeropoint  pointer on bias zero_point
 * @param[in]  pBias_data   pointer on bias input
 * @param[in]  bnl_param_sign  sign of BNL parameters (0=unsigned, 1=signed)
 * @param[in]  pBuffer_a scratch buffer for:
 *              out factor: nb channels * sizeof(ai_i32) +
 *              out offset: nb channels * sizeof(ai_i32) +
 *              out shift: nb channels * sizeof(ai_i16)
 */
void forward_lite_bn_iu8ou8_ch( const ai_u8 *p_in,
                        ai_u8 *p_out,
                        ai_size n_channel_inout,
                        ai_size n_elements,
                        ai_float in_scale,
                        const ai_i32 in_zeropoint,
                        ai_float out_scale,
                        const ai_i32 out_zeropoint,
                        const ai_float *pSc_scale,
                        const ai_i8 *pSc_zeropoint,
                        const ai_i8 *pData_scale,
                        const ai_float *pBias_scale,
                        const ai_i8 *pBias_zeropoint,
                        const ai_i8 *pData_bias,
                        ai_i16 bnl_param_sign,
                        ai_i32 scratch_size,
                        ai_i16 *pBuffer_a);
#endif    /* LITE_BN_INTEGER_H */
