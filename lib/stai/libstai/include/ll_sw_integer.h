/**
 ******************************************************************************
 * @file    ll_sw_integer.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ll_sw_integer low level software library module.
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

#ifndef __LL_SW_INTEGER_H__
#define __LL_SW_INTEGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

  void ll_sw_forward_dequantizelinear(void *sw_info_struct);
  void ll_sw_forward_quantizelinear(void *sw_info_struct);
  void ll_sw_forward_requantizelinear(void *sw_info_struct);
  void ll_sw_forward_qlinearconv(void *sw_info_struct);
  void ll_sw_forward_qlinearmatmul(void *sw_info_struct);
  void ll_sw_forward_conv_integer(void *sw_info_struct);
  void ll_sw_forward_pool_integer(void *sw_info_struct);
  void ll_sw_forward_global_pool_integer(void *sw_info_struct);
  void ll_sw_forward_activ_integer(void *sw_info_struct);
  void ll_sw_forward_eltwise_integer(void *sw_info_struct);
  void ll_sw_forward_gemm_integer(void *sw_info_struct);
  void ll_sw_forward_softmax_integer(void *sw_info_struct);
  void ll_sw_forward_resize_integer(void *sw_info_struct);

#ifdef __cplusplus
}
#endif

#endif
