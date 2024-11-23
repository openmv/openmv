/**
  ******************************************************************************
  * @file    lite_pool_is8os8.h
  * @author  AIS
  * @brief   header file of AI platform lite integer pooling function
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
#ifndef LITE_POOL_IS8OS8
#define LITE_POOL_IS8OS8

#include "ai_lite_interface.h"

/**
 * @brief lite function for average pooling.
 * @ingroup lite_nl_generic_integer
 * @param input The pointer to input buffer.
 * @param output The pointer to output buffer.
 * @param[in] dim_im_in_x dimension of the input width
 * @param[in] dim_im_in_y dimension of the input height
 * @param[in] ch_im_in number of the input channel
 * @param[in] dim_kernel_x dimension of the kernel width
 * @param[in] dim_kernel_y dimension of the kernel height
 * @param[in] padding_x first dimension of the padding
 * @param[in] padding_y second dimension of the padding
 * @param[in] stride_x first dimension of the stride
 * @param[in] stride_y second dimension of the stride
 * @param[in] dim_im_out_x dimension of the output width
 * @param[in] dim_im_out_y dimension of the output height
 * @param[in] in_scale     input scale
 * @param[in] in_zeropoint input zero point
 * @param[in] out_scale output scale
 * @param[in] out_zeropoint output zero point
 */
void forward_lite_avepool_is8os8( const ai_i8 *pData_in,
                                  ai_i8* pData_out,
                                  const ai_u16 dim_im_in_x,
                                  const ai_u16 dim_im_in_y,
                                  const ai_u16 ch_im_in,
                                  const ai_u16 dim_kernel_x,
                                  const ai_u16 dim_kernel_y,
                                  const ai_u16 padding_x,
                                  const ai_u16 padding_y,
                                  const ai_u16 stride_x,
                                  const ai_u16 stride_y,
                                  const ai_u16 dim_im_out_x,
                                  const ai_u16 dim_im_out_y,
                                  const ai_float in_scale,
                                  const ai_i8 in_zeropoint,
                                  const ai_float out_scale,
                                  const ai_i8 out_zeropoint);
#endif /* LITE_POOL_IS8OS8 */

