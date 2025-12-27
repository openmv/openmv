/**
  ******************************************************************************
  * @file    layers_sm.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform softmaxcrossentropy loss grad layer datatype
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
  @verbatim
  @endverbatim
  ******************************************************************************
  */

#ifndef LAYERS_SOFTMAX_CROSSENTROPY_GRAD_H
#define LAYERS_SOFTMAX_CROSSENTROPY_GRAD_H
#pragma once

#include "layers_common.h"

/*!
 * @defgroup layers SoftMaxCrossEntropyLossGrad Layer Definitions
 * @brief definition 
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_softmaxcrossentropylossgrad
 * @ingroup layers_sm
 * @brief softmaxcrossentropylossgrad layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_softmaxcrossentropylossgrad_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_char* reduction; /*!< reduction attribute */
} ai_layer_softmaxcrossentropylossgrad;

/*!
 * @brief Computes the activations of a GEMM layer.
 * @ingroup layers
 * @param layer the layer including output and input tensors
 */
AI_INTERNAL_API
void backward_softmaxcrossentropylossgrad(ai_layer* layer);

AI_API_DECLARE_END

#endif    /*LAYERS_SOFTMAX_CROSSENTROPY_GRAD_H*/
