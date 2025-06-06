/**
  ******************************************************************************
  * @file    layers_wrappers.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform generic layers datatypes
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
#ifndef _LAYERS_WRAPPERS_H
#define _LAYERS_WRAPPERS_H

#include "layers_common.h"

/*!
 * @defgroup layers_wrappers Runtime Wrapper Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_tflite_wrapper
 * @ingroup layers_generic
 * @brief TimeDelay layer with sparse kernel
 */

typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_tflite_wrapper_ {
  AI_NODE_COMMON_FIELDS_DECLARE
  const ai_array*    init_data;
} ai_layer_tflite_wrapper;

AI_API_DECLARE_END

#endif    /* _LAYERS_WRAPPERS_H */

