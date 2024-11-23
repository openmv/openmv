/**
  ******************************************************************************
  * @file    lite_maxpool_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite maxpool kernel datatypes
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
#ifndef LITE_POOL_F32_H
#define LITE_POOL_F32_H

#include "ai_lite_interface.h"


#define FUNC_POOL(handle) \
  ((func_pool)(handle))


/*!
 * @typedef (*func_pool)
 * @ingroup layers_pool
 * @brief Fuction pointer for generic pooling transform
 * this function pointer abstracts a generic pooling layer.
 * see @ref pool_func_ap_array_f32 as examples
 */
typedef void (*func_pool)(ai_float* in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_float* out);


/******************************************************************************/
/** Conv2d Functions Section                                                 **/
/******************************************************************************/

AI_INTERNAL_API
void pool_func_mp_array_f32(ai_float* pData_in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_float* pData_out);

AI_INTERNAL_API
void pool_func_ap_array_f32(ai_float *pData_in,
                      const ai_u16 dim_im_in_x, const ai_u16 dim_im_in_y,
                      const ai_u16 ch_im_in,
                      const ai_u16 dim_kernel_x, const ai_u16 dim_kernel_y,
                      const ai_u16 padding_x, const ai_u16 padding_y,
                      const ai_u16 stride_x, const ai_u16 stride_y,
                      const ai_u16 dim_im_out_x, const ai_u16 dim_im_out_y,
                      ai_float *pData_out);

#endif // LITE_POOL_F32_H_
