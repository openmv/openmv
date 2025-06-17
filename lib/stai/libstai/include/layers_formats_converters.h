/**
  ******************************************************************************
  * @file    layers_formats_converters.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of formats converters layers
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_FORMATS_CONVERTERS_H
#define LAYERS_FORMATS_CONVERTERS_H

#include "layers_common.h"

/*!
 * @defgroup layers_formats_converters Formats Converters Layers Definition
 * @brief this group implements formats converter layers (cast, etc.)
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_cast
 * @ingroup layers_formats_converters
 * @brief C Implementation of cast layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_cast_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_array_format     to_format;    /*!< cast output format */
} ai_layer_cast;


/*****************************************************************************/
/*  Forward Functions Section                                                */
/*****************************************************************************/

/*!
 * @brief forward function for cast layer.
 * @ingroup layers_
 * @param layer template layer as an opaque pointer
 */
AI_INTERNAL_API
void forward_cast(ai_layer* layer);


AI_API_DECLARE_END

#endif    /*LAYERS_FORMATS_CONVERTERS_H*/
