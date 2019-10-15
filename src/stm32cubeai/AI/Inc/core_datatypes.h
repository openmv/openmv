/**
  ******************************************************************************
  * @file    core_datatypes.h
  * @author  AST Embedded Analytics Research Platform
  * @date    22-Aug-2018
  * @brief   header file of core module private defines and datatypes
  * to public nor codegen tool
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __AI_CORE_DATATYPES_H_
#define __AI_CORE_DATATYPES_H_
#pragma once
#include <stdint.h>

/*!
 * @defgroup Core Module Datatypes
 * @brief Data structures and defines used by core module
 */

/*!
 * @brief platform runtime core library version
 */
#define AI_PLATFORM_RUNTIME_MAJOR    4
#define AI_PLATFORM_RUNTIME_MINOR    0
#define AI_PLATFORM_RUNTIME_MICRO    0

#define AI_MAGIC_CONTEXT_TOKEN       (0xA1C00100)   /*!< AI Cool! Magic Token */

#define AI_MAGIC_INSPECTOR_TOKEN     (0xA1C00101)   /*!< AI Cool! Magic Token */


#define AI_ID_OBJ(id) \
  ((ai_id_obj)(id))

#define AI_C_ARRAY_COUNT(array_) \
  ( sizeof(array_) / sizeof((array_)[0]) )

/*!
 * @typedef ai_id_obj
 * @ingroup core_datatypes
 * @brief numeric identifier for generic object instances (e.g. layers, 
 * operators, etc.) It is used by codegen tool to keep tracks of specific
 * instances created 
 */
typedef uint16_t ai_id_obj;

#endif    /*__AI_CORE_DATATYPES_H_*/
