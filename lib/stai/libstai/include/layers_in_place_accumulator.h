/**
  ******************************************************************************
  * @file    layers_sm.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform inplaceaccumulatorv2 layer datatype
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

#ifndef LAYERS_IN_PLACE_ACCUMULATOR_H
#define LAYERS_IN_PLACE_ACCUMULATOR_H
#pragma once

#include "layers_common.h"

/*!
 * @defgroup layers InPlaceAccumulatorV2 Layer Definitions
 * @brief definition 
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_inplaceaccumulatorv2
 * @ingroup layers_sm
 * @brief inplaceaccumulatorv2 layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_inplaceaccumulatorv2_{
  AI_LAYER_COMMON_FIELDS_DECLARE
} ai_layer_inplaceaccumulatorv2;

/*!
 * @brief Computes the activations of a GEMM layer.
 * @ingroup layers
 * @param layer the layer including output and input tensors
 */
AI_INTERNAL_API
void backward_inplaceaccumulatorv2(ai_layer* layer);

AI_API_DECLARE_END

#endif    /*LAYERS_IN_PLACE_ACCUMULATOR_H*/
