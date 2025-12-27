/**
  ******************************************************************************
  * @file    layers_sm.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform inplaceaccumulatorv2 layer datatype
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

#ifndef LAYERS_RELU_GRAD_H
#define LAYERS_RELU_GRAD_H
#pragma once

#include "layers_common.h"

/*!
 * @defgroup layers relu_grad Layer Definitions
 * @brief definition 
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_ai_layer_relu_grad
 * @ingroup layers_sm
 * @brief ai_layer_relu_grad layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_relu_grad_{
  AI_LAYER_COMMON_FIELDS_DECLARE
} ai_layer_relu_grad;

/*!
 * @brief Computes the downstream gradient for ReLU.
 * @ingroup layers
 * @param layer the layer including output and input tensors
 */
AI_INTERNAL_API
void backward_relu(ai_layer* layer);

AI_API_DECLARE_END

#endif    /*LAYERS_RELU_GRAD_H*/
