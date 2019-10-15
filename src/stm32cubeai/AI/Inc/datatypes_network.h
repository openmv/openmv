/**
  ******************************************************************************
  * @file    datatypes_network.h
  * @author  AST Embedded Analytics Research Platform
  * @date    30-Aug-2017
  * @brief   Definitions of code generated network types
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __DATATYPES_NETWORK_H__
#define __DATATYPES_NETWORK_H__
#pragma once

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


typedef ai_u32 ai_shape_dimension;
typedef ai_i32 ai_stride_dimension;
typedef ai_u32 ai_array_size;


AI_API_DECLARE_END

#endif /*__DATATYPES_NETWORK_H__*/
