/**
  ******************************************************************************
  * @file    lite_maxpool_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite dqnn maxpool kernel datatypes
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
#ifndef LITE_MAXPOOL_DQNN_H
#define LITE_MAXPOOL_DQNN_H


#include "ai_lite_interface.h"


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles maxpool with binary input and binary output - Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_is1os1(const ai_u32 *pDataIn_init,
                                 ai_u32 *pDataOut_init,
                                 const ai_i32 width_in,
                                 const ai_i32 width_out,
                                 const ai_i32 height_in,
                                 const ai_i32 height_out,
                                 const ai_u32 n_channel_in,
                                 const ai_u32 n_channel_out,
                                 const ai_i32 pool_width,
                                 const ai_i32 pool_height,
                                 const ai_i32 pool_pad_x,
                                 const ai_i32 pool_pad_y,
                                 const ai_i32 pool_stride_x,
                                 const ai_i32 pool_stride_y,
                                 const ai_u32 pool_pad_value,
                                 ai_float *pScratch_32);


/*!
 * @brief Handles maxpool with 8 bits signed input and output with a positive scale of the input- Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_is8os8_scalepos(const ai_i8 *pDataIn,
                                          ai_i8 *pDataOut,
                                          const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                                          const ai_u16 ch_im_in,
                                          const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                                          const ai_u16 padding_x, const ai_u16 padding_y,
                                          const ai_u16 stride_x, const ai_u16 stride_y,
                                          const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                                          const ai_float InOut_ScaleRatio,
                                          const ai_i8 In_ZeroPoint,
                                          const ai_i8 Out_ZeroPoint);


/*!
 * @brief Handles maxpool with 8 bits signed input and output with a negative scale of the input- Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_is8os8_scaleneg(const ai_i8 *pDataIn,
                                          ai_i8 *pDataOut,
                                          const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                                          const ai_u16 ch_im_in,
                                          const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                                          const ai_u16 padding_x, const ai_u16 padding_y,
                                          const ai_u16 stride_x, const ai_u16 stride_y,
                                          const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                                          const ai_float InOut_ScaleRatio,
                                          const ai_i8 In_ZeroPoint,
                                          const ai_i8 Out_ZeroPoint);


/*!
 * @brief Handles maxpool with 8 bits unsigned input and output with a positive scale of the input- Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_iu8ou8_scalepos(const ai_u8 *pDataIn,
                                          ai_u8 *pDataOut,
                                          const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                                          const ai_u16 ch_im_in,
                                          const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                                          const ai_u16 padding_x, const ai_u16 padding_y,
                                          const ai_u16 stride_x, const ai_u16 stride_y,
                                          const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                                          const ai_float InOut_ScaleRatio,
                                          const ai_u8 In_ZeroPoint,
                                          const ai_u8 Out_ZeroPoint);


/*!
 * @brief Handles maxpool with 8 bits unsigned input and output with a negative scale of the input- Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_iu8ou8_scaleneg(const ai_u8 *pDataIn,
                                          ai_u8 *pDataOut,
                                          const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                                          const ai_u16 ch_im_in,
                                          const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                                          const ai_u16 padding_x, const ai_u16 padding_y,
                                          const ai_u16 stride_x, const ai_u16 stride_y,
                                          const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                                          const ai_float InOut_ScaleRatio,
                                          const ai_u8 In_ZeroPoint,
                                          const ai_u8 Out_ZeroPoint);

/*!
 * @brief Handles maxpool with 16 bits signed input and output with a positive scale of the input- Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_is16os16_scalepos(const ai_i16 *pApInput,
                                            ai_i16 *pApOutput,
                                            const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                                            const ai_u16 ch_im_in,
                                            const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                                            const ai_u16 padding_x, const ai_u16 padding_y,
                                            const ai_u16 stride_x, const ai_u16 stride_y,
                                            const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                                            const ai_float InOut_ScaleRatio,
                                            const ai_i16 In_ZeroPoint,
                                            const ai_i16 Out_ZeroPoint);

/*!
 * @brief Handles maxpool with 16 bits unsigned input and output with a positive scale of the input- Lite I/F
  * @ingroup lite_maxpool_dqnn
 */
LITE_API_ENTRY
void forward_lite_maxpool_iu16ou16_scalepos(const ai_u16 *pApInput,
                                            ai_u16 *pApOutput,
                                            const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                                            const ai_u16 ch_im_in,
                                            const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                                            const ai_u16 padding_x, const ai_u16 padding_y,
                                            const ai_u16 stride_x, const ai_u16 stride_y,
                                            const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                                            const ai_float InOut_ScaleRatio,
                                            const ai_u16 In_ZeroPoint,
                                            const ai_u16 Out_ZeroPoint);

#endif    /*LITE_MAXPOOL_DQNN_H*/
