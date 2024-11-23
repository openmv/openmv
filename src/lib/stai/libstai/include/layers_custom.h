#ifndef LAYERS_CUSTOM_H
#define LAYERS_CUSTOM_H
/**
  ******************************************************************************
  * @file    layers_custom.h
  * @author  STMicroelectronics
  * @brief   header file of AI platform custom layers datatype
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "layers_common.h"

/*!
 * @defgroup layers_custom Custom layer definitions
 * @brief Definition of structures custom layers
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_custom
 * @ingroup layers_custom
 * @brief Custom layer wrapper
 *
 * The custom layer wrapper
 */
typedef ai_layer_stateful ai_layer_custom;


AI_API_DECLARE_END

#endif /* LAYERS_CUSTOM_H */
