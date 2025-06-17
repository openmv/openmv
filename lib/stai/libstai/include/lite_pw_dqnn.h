/**
  ******************************************************************************
  * @file    lite_pw_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite dqnn pointwise kernel datatypes
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
#ifndef LITE_PW_DQNN_H
#define LITE_PW_DQNN_H


#include "ai_lite_interface.h"


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles point wise convolution with binary input, binary output and
 *        binary weights - Lite API version
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_pw_is1os1ws1_bn(const ai_u32 *pDataIn_init,
                                  ai_u32 *pDataOut_init,
                                  const ai_u32 *pWeights_init,
                                  const ai_u32 n_channel_in,
                                  const ai_u32 n_channel_out,
                                  const ai_i32 width_out,
                                  const ai_i32 height_out,
                                  const ai_i32 *pThreshold);

/*!
 * @brief Handles point wise convolution with binary input, binary output and
 *        binary weights - Lite API version - Optimized thanks to Optim2
 *        assumptions
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_pw_is1os1ws1_bn_optim2(const ai_u32 *pDataIn_init,
                                        ai_u32 *pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        const ai_u32 n_channel_in,
                                        const ai_u32 n_channel_out,
                                        const ai_i32 width_out,
                                        const ai_i32 height_out,
                                        const ai_i32 *pThreshold);

/*!
 * @brief Handles point wise convolution with binary input, 8-bits output and
 *        binary weights - Lite API version
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_pw_is1os8ws1_bn(const ai_u32 *pDataIn_init,
                                  ai_i8 *pDataOut_init,
                                  const ai_u32 *pWeights_init,
                                  const ai_u32 n_channel_in,
                                  const ai_u32 n_channel_out,
                                  const ai_i32 width_out,
                                  const ai_i32 height_out,
                                  const ai_float *pScale,
                                  const ai_float *pOffset);

/*!
 * @brief Handles point wise convolution with binary input, 8-bits output and
 *        binary weights - Lite API version - Optimized thanks to Optim1
 *        assumptions
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_pw_is1os8ws1_bn_optim1(const ai_u32 *pDataIn_init,
                                        ai_i8 *pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        const ai_u32 n_channel_in,
                                        const ai_u32 n_channel_out,
                                        const ai_i32 width_out,
                                        const ai_i32 height_out,
                                        const ai_float *pScale,
                                        const ai_float *pOffset);

/*!
 * @brief Handles point-wise convolution with binary input, float32 output
 *        and binary weights - Lite API version
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_pw_is1of32ws1_bn(const ai_u32 *pDataIn_init,
                                   ai_float *pDataOut_init,
                                   const ai_u32 *pWeights_init,
                                   const ai_u32 n_channel_in,
                                   const ai_u32 n_channel_out,
                                   const ai_i32 width_out,
                                   const ai_i32 height_out,
                                   const ai_float *pScale,
                                   const ai_float *pOffset);

/*!
 * @brief Handles point-wise convolution with binary input, float32 output
 *        and binary weights - Lite API version - Optimized thanks to Optim1
 *        assumptions
 * @ingroup lite_pw_dqnn
 */
LITE_API_ENTRY
void forward_lite_pw_is1of32ws1_bn_optim1(const ai_u32 *pDataIn_init,
                                         ai_float *pDataOut_init,
                                         const ai_u32 *pWeights_init,
                                         const ai_u32 n_channel_in,
                                         const ai_u32 n_channel_out,
                                         const ai_i32 width_out,
                                         const ai_i32 height_out,
                                         const ai_float *pScale,
                                         const ai_float *pOffset);


#endif    /*LITE_PW_DQNN_H*/
