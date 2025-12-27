/**
 ******************************************************************************
 * @file    ll_sw_float.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ll_sw_float low level software library module.
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

#ifndef __LL_SW_FLOAT_H__
#define __LL_SW_FLOAT_H__

#ifdef __cplusplus
extern "C"
{
#endif

  void ll_sw_forward_conv(void *sw_info_struct);
  void ll_sw_forward_gemm(void *sw_info_struct);
  void ll_sw_forward_matmul(void *sw_info_struct);
  void ll_sw_forward_pool(void *sw_info_struct);
  void ll_sw_forward_global_pool(void *sw_info_struct);
  void ll_sw_forward_activ(void *sw_info_struct);
  void ll_sw_forward_arith(void *sw_info_struct);
  void ll_sw_forward_bn(void *sw_info_struct);
  void ll_sw_forward_instance_normalization(void *sw_info_struct);
  void ll_sw_forward_lrn(void *sw_info_struct);
  void ll_sw_forward_concat(void *sw_info_struct);
  void ll_sw_forward_resize(void *sw_info_struct);
  void ll_sw_forward_reduce(void *sw_info_struct);
  void ll_sw_forward_lpnormalization(void *sw_info_struct);
  void ll_sw_forward_argmin(void *sw_info_struct);
  void ll_sw_forward_argmax(void *sw_info_struct);
  void ll_sw_forward_gather(void *sw_info_struct);
  void ll_sw_forward_sign(void *sw_info_struct);
  void ll_sw_forward_tile(void *sw_info_struct);
  void ll_sw_forward_softmax(void *sw_info_struct);

#ifdef __cplusplus
}
#endif

#endif
