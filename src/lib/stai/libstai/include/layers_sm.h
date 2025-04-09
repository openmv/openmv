/**
  ******************************************************************************
  * @file    layers_sm.h
  * @author  STMicroelectronics
  * @brief   header file of AI platform non softmax layer datatype
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LAYERS_SM_H
#define LAYERS_SM_H

#include "layers_common.h"

/*!
 * @defgroup layers SoftMax Layer Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @brief Softmax normalization computed on an array of fixed point channels
 * @ingroup layers_sm
 * @param out opaque handler to output channel array
 * @param in  opaque handler to input channel array
 * @param in_size  total size (number of elements) to process on the input
 * @param channel_size number of elements of the input channel
 * @param in_channel_step number of elements to move to next input element
 * @param out_channel_step number of elements to move to next output element
 */
AI_INTERNAL_API
void sm_func_sm_array_fixed(ai_handle out, const ai_handle in,
                            const ai_size in_size,
                            const ai_size channel_size,
                            const ai_size in_channel_step,
                            const ai_size out_channel_step);

/*!
 * @brief Computes the activations of a fixed point softmax nonlinear layer.
 * @ingroup layers_sm
 * @param layer the softmax (sm) layer
 */
AI_INTERNAL_API
void forward_sm_fixed(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*LAYERS_SM_H*/

