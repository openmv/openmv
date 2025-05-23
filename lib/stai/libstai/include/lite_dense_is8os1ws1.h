/**
  ******************************************************************************
  * @file    lite_dense_is8os1ws1.h
  * @author  AIS
  * @brief   header file of AI platform lite dense kernel datatypes
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
#ifndef LITE_DENSE_IS8OS1WS1_H
#define LITE_DENSE_IS8OS1WS1_H


#include "ai_lite_interface.h"

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Forward function for a dense layer with signed 8 bits input,
 *        binary weights and binary output.
 * @ingroup lite_dense_is8os1ws1
 * @param out_ptr The pointer to output buffer.
  *@param data_in_init_ptr The pointer to input buffer.
 * @param weights_ptr The pointer to weights.
 * @param scratch_ptr The pointer to scratch buffer.
 * @param scratch_size The value of scratch tensor size.
 * @param n_channel_out The number of channels of the output, i.e.,
 *                      the number of dense hidden neurons.
 * @param n_channel_in The number of channels of the input.
 * @param scale_ptr The pointer to scale buffer of BN.
 * @param offset_ptr The pointer to offset buffer of BN.
 */
LITE_API_ENTRY
void forward_lite_dense_is8os1ws1_bn_fxp(ai_pbits *out_ptr,
                                         const ai_i8 *data_in_init_ptr,
                                         const ai_pbits *weights_ptr,
                                         ai_i32 *scratch_ptr,
                                         const ai_u32 scratch_size,
                                         const ai_u32 n_channel_out,
                                         const ai_u32 n_channel_in,
                                         const ai_i32 *threshold_ptr);

#endif    /*LITE_DENSE_IS8OS1WS1_H*/
