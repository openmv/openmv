/**
  ******************************************************************************
  * @file    lite_dw_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite integer depthwise kernel datatypes
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
#ifndef LITE_DW_DQNN_H
#define LITE_DW_DQNN_H


#include "ai_lite_interface.h"



/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles 2D DW convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_dw_is1os1ws1_bn_pad0(const ai_u32 *pDataIn_init,
                                        ai_u32 * pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        ai_float *pScratch_32,
                                        const ai_u32 n_channel_in,
                                        const ai_u32 n_channel_out,
                                        const ai_i32 width_in,
                                        const ai_i32 height_in,
                                        const ai_i32 width_out,
                                        const ai_i32 height_out,
                                        const ai_i32 filt_width,
                                        const ai_i32 filt_height,
                                        const ai_i32 filt_pad_x,
                                        const ai_i32 filt_pad_y,
                                        const ai_i32 filt_stride_x,
                                        const ai_i32 filt_stride_y,
                                        const ai_i32 *pThreshold);

/*!
 * @brief Handles 2D DW convolution with binary input, binary output and
 *        binary weights - with 0 padding (QKeras like) - Lite I/F
 *        - Optimized thanks to Optim3 assumptions
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_dw_is1os1ws1_bn_pad0_optim3(const ai_u32 *pDataIn_init,
                                              ai_u32 * pDataOut_init,
                                              const ai_u32 *pWeights_init,
                                              ai_float *pScratch_32,
                                              const ai_u32 n_channel_in,
                                              const ai_u32 n_channel_out,
                                              const ai_i32 width_in,
                                              const ai_i32 height_in,
                                              const ai_i32 width_out,
                                              const ai_i32 height_out,
                                              const ai_i32 filt_width,
                                              const ai_i32 filt_height,
                                              const ai_i32 filt_pad_x,
                                              const ai_i32 filt_pad_y,
                                              const ai_i32 filt_stride_x,
                                              const ai_i32 filt_stride_y,
                                              const ai_i32 *pThreshold);

/*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_dw_is1os1ws1_bn_pad1(const ai_u32 *pDataIn_init,
                                        ai_u32 * pDataOut_init,
                                        const ai_u32 *pWeights_init,
                                        ai_float *pScratch_32,
                                        const ai_u32 n_channel_in,
                                        const ai_u32 n_channel_out,
                                        const ai_i32 width_in,
                                        const ai_i32 height_in,
                                        const ai_i32 width_out,
                                        const ai_i32 height_out,
                                        const ai_i32 filt_width,
                                        const ai_i32 filt_height,
                                        const ai_i32 filt_pad_x,
                                        const ai_i32 filt_pad_y,
                                        const ai_i32 filt_stride_x,
                                        const ai_i32 filt_stride_y,
                                        const ai_i32 *pThreshold,
                                        const ai_i32 pad_value);

 /*!
 * @brief Handles 2D convolution with binary input, binary output and
 *        binary weights - with +1/-1 padding (Larq like) - Lite I/F
 *        - Optimized thanks to Optim3 assumptions
 * @ingroup lite_conv2d_dqnn
 */
LITE_API_ENTRY
void forward_lite_dw_is1os1ws1_bn_pad1_optim3(const ai_u32 *pDataIn_init,
                                              ai_u32 * pDataOut_init,
                                              const ai_u32 *pWeights_init,
                                              ai_float *pScratch_32,
                                              const ai_u32 n_channel_in,
                                              const ai_u32 n_channel_out,
                                              const ai_i32 width_in,
                                              const ai_i32 height_in,
                                              const ai_i32 width_out,
                                              const ai_i32 height_out,
                                              const ai_i32 filt_width,
                                              const ai_i32 filt_height,
                                              const ai_i32 filt_pad_x,
                                              const ai_i32 filt_pad_y,
                                              const ai_i32 filt_stride_x,
                                              const ai_i32 filt_stride_y,
                                              const ai_i32 *pThreshold,
                                              const ai_i32 pad_value);


#endif    /*LITE_DW_DQNN_H*/
