/**
  ******************************************************************************
  * @file    stai_debug.h
  * @author  STMicroelectronics
  * @brief   Definitions of ST.AI platform public APIs types
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
  */
#ifndef STAI_DEBUG_H
#define STAI_DEBUG_H

#include "stai.h"

/*****************************************************************************/
#ifdef HAS_AI_ASSERT
// Override __FILE__ macro to disable full path asserts()
// Need to add during build also -Wbuiltin-macro-redefined options to avoid warnings
#undef __FILE__
#define __FILE__ (__builtin_strrchr("/" __BASE_FILE__, '/') + 1)

#include <assert.h>

#define STAI_ASSERT(_cond) \
  do { assert(_cond); } while(0);

#else

#define STAI_ASSERT(_cond) \
  do { /* STAI_ASSERT() */ } while (0);

#endif  /* HAS_AI_ASSERT */

#define STAI_PRINT(_fmt, ...) \
  do { /* STAI_PRINT() */ } while (0);


STAI_API_DECLARE_BEGIN


/*****************************************************************************/

STAI_API_ENTRY
const char* stai_get_return_code_name(const stai_return_code code);


STAI_API_ENTRY
const char* stai_get_format_type_name(const stai_format_type fmt_type);


STAI_API_ENTRY
const char* stai_get_format_name(const stai_format fmt);


STAI_API_DECLARE_END

#endif    /* STAI_DEBUG_H */
