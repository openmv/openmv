/**
  ******************************************************************************
  * @file    core_assert.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of core assert routine
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
#ifndef CORE_ASSERT_H
#define CORE_ASSERT_H

#ifdef HAS_AI_ASSERT
// Override __FILE__ macro to disable full path asserts()
// Need to add during build also -Wbuiltin-macro-redefined options to avoid warnings
#undef __FILE__
#define __FILE__ (__builtin_strrchr("/" __BASE_FILE__, '/') + 1)

#include <assert.h>
#define CORE_ASSERT(expr) \
  assert(expr); /* CORE_ASSERT */

#else
#define CORE_ASSERT(expr) \
  (void)0; /* CORE_ASSERT */

#endif    /* HAS_AI_ASSERT */

#endif    /* CORE_ASSERT_H */
