/**
  ******************************************************************************
  * @file    datatypes_network.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of code generated network types
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef DATATYPES_NETWORK_H
#define DATATYPES_NETWORK_H

/*
 * Header to be overriden by the generated version
 * by including with <> the include directories are searched in the order
 * specified in the compiler
 * To enable the override, put the generated path before the API path
 */

#include "ai_platform.h"

AI_API_DECLARE_BEGIN

#ifdef AI_OVERRIDE_CUSTOM_TYPES
#warning "Warning: Custom Types have been already defined!\n"
#endif

#define AI_CUSTOM_TYPES_COUNT      (3)

#define AI_CUSTOM_TYPES_SIGNATURE_DECLARE(name)  \
  const ai_custom_type_signature name[AI_CUSTOM_TYPES_COUNT+1] = { \
    AI_CUSTOM_TYPES_COUNT, \
    AI_CUSTOM_SIZE(ai_shape_dimension), \
    AI_CUSTOM_SIZE(ai_stride_dimension), \
    AI_CUSTOM_SIZE(ai_array_size), \
  };


typedef ai_i32 ai_stride_dimension;
typedef ai_u32 ai_array_size;


AI_API_DECLARE_END

#endif /* DATATYPES_NETWORK_H */
