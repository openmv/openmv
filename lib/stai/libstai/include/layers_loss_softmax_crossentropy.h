/**
  ******************************************************************************
  * @file    layers_scel.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform softmaxcrossentropy loss layer datatype
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

#ifndef LAYERS_SOFTMAX_CROSSENTROPY_H
#define LAYERS_SOFTMAX_CROSSENTROPY_H
#pragma once

#include "layers_common.h"

/*!
 * @defgroup layers SoftMaxCrossEntropyLoss Layer Definitions
 * @brief definition 
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_softmaxcrossentropyloss
 * @ingroup layers_scel
 * @brief softmaxcrossentropyloss layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_softmaxcrossentropyloss_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_char* reduction; /*!< reduction attribute */
} ai_layer_softmaxcrossentropyloss;

/*!
 * @brief Computes the softmax cross entropy loss
 * @ingroup layers
 * @param layer the layer including output and input tensors
 */
AI_INTERNAL_API

void forward_softmaxcrossentropyloss(ai_layer* layer);

AI_API_DECLARE_END

#endif    /*LAYERS_SOFTMAX_CROSSENTROPY_H*/
