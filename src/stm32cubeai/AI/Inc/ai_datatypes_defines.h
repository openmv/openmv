/**
  ******************************************************************************
  * @file    ai_datatypes_defines.h
  * @author  AST Embedded Analytics Research Platform
  * @date    18-Oct-2017
  * @brief   Definitions of AI platform private APIs types
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

#ifndef __AI_DATATYPES_DEFINES_H__
#define __AI_DATATYPES_DEFINES_H__
#pragma once

#include "ai_platform.h"

/*!
 * @defgroup datatypes_defines Internal Datatypes Defines Header
 * @brief Data structures used internally to implement neural networks
 *
 */

/* define to track datatypes used by codegen */
#define AI_INTERFACE_TYPE             /* AI_INTERFACE_TYPE */

#define AI_INTERNAL_API               /* AI_INTERNAL_API */

#define AI_CONST                      const
#define AI_STATIC                     static
#define AI_STATIC_CONST               static const

/******************************************************************************/
/* NOP operation used by codegen */
#define AI_NOP                        /* NOP */

#define AI_WRAP_FUNC(fn_)             do { fn_ } while (0);

#define AI_CAT(a, ...)                AI_PRIMITIVE_CAT(a, __VA_ARGS__)
#define AI_PRIMITIVE_CAT(a, ...)      a ## __VA_ARGS__

/******************************************************************************/
#ifdef HAS_AI_ASSERT
#include <assert.h>
#define AI_ASSERT(cond) \
  { assert(cond); }
#else
#define AI_ASSERT(cond) \
  AI_WRAP_FUNC(AI_NOP)
#endif  /*HAS_AI_ASSERT*/

/******************************************************************************/
#define AI_NO_PACKED_STRUCTS

/* Macro for defining packed structures (compiler dependent).
 * This just reduces memory requirements, but is not required.
 */
#if defined(AI_NO_PACKED_STRUCTS)
  /* Disable struct packing */
  #define AI_PACKED_STRUCT_START        /* AI_PACKED_STRUCT_START */
  #define AI_PACKED_STRUCT_END          /* AI_PACKED_STRUCT_END */
  #define AI_PACKED                     /* AI_PACKED */
#elif defined(__GNUC__) || defined(__clang__)
  /* For GCC and clang */
  #define AI_PACKED_STRUCT_START        /* AI_PACKED_STRUCT_START */
  #define AI_PACKED_STRUCT_END          /* AI_PACKED_STRUCT_END */
  #define AI_PACKED                     __attribute__((packed))
#elif defined(__ICCARM__) || defined (__IAR_SYSTEMS_ICC__) || defined(__CC_ARM)
  /* For IAR ARM and Keil MDK-ARM compilers */
  #define AI_PACKED_STRUCT_START        _Pragma("pack(push, 1)")
  #define AI_PACKED_STRUCT_END          _Pragma("pack(pop)")
  #define AI_PACKED                     /* AI_PACKED */
#elif defined(_MSC_VER) && (_MSC_VER >= 1500)
  /* For Microsoft Visual C++ */
  #define AI_PACKED_STRUCT_START        __pragma(pack(push, 1))
  #define AI_PACKED_STRUCT_END          __pragma(pack(pop))
  #define AI_PACKED                     /* AI_PACKED */
#else
  /* Unknown compiler */
  #define AI_PACKED_STRUCT_START        /* AI_PACKED_STRUCT_START */
  #define AI_PACKED_STRUCT_END          /* AI_PACKED_STRUCT_END */
  #define AI_PACKED                     /* AI_PACKED */
#endif    /* AI_NO_PACKED_STRUCTS */

/******************************************************************************/
#define AI_STRINGIFY_ARG(contents)      # contents
#define AI_STRINGIFY(macro_or_string)   AI_STRINGIFY_ARG (macro_or_string)

/******************************************************************************/
#if defined(_MSC_VER)
  #define AI_DECLARE_STATIC           static __inline
  #define AI_ALIGNED_TYPE(type, x)    type __declspec(align(x))
  #define AI_INTERFACE_ENTRY         __declspec(dllexport)
#elif defined(__ICCARM__) || defined (__IAR_SYSTEMS_ICC__)
  #define AI_DECLARE_STATIC           static inline
  #define AI_ALIGNED_TYPE(type, x)    type
  #define AI_INTERFACE_ENTRY          /* AI_INTERFACE_ENTRY */
#else /* _MSC_VER */
  #define AI_DECLARE_STATIC           static __inline
  #define AI_ALIGNED_TYPE(type, x)    type __attribute__ ((aligned(x)))
  #define AI_INTERFACE_ENTRY         __attribute__((visibility("default")))
#endif /* _MSC_VER */

/******************************************************************************/
#define AI_ALIGN_MASKED(value, mask)    ( ((value)+(mask))&(~(mask)) )


#define AI_GET_REVISION(major, minor, micro) ( \
            ((ai_u32)(major)<<24) | \
            ((ai_u32)(minor)<<16) | \
            ((ai_u32)(micro)<< 8) )

#define AI_GET_VERSION_STRING(major, minor, micro) \
          AI_STRINGIFY_ARG(major) "." \
          AI_STRINGIFY_ARG(minor) "." \
          AI_STRINGIFY_ARG(micro) \

#define AI_PACK(...)                    __VA_ARGS__

#define AI_CR                         "\r\n"

#ifdef HAS_AI_DEBUG
#define AI_DEBUG(expr)                expr
#else
#define AI_DEBUG(expr)                AI_WRAP_FUNC(AI_NOP)
#endif  /* HAS_AI_DEBUG */

#define AI_FLAG_NONE                  (0x0)
#define AI_FLAG_SET(mask, flag)       (mask) |= (flag)
#define AI_FLAG_UNSET(mask, flag)     (mask) &= (~(flag))
#define AI_FLAG_IS_SET(mask, flag)    ( (flag)==((mask)&(flag)) ) 

#endif    /*__AI_DATATYPES_DEFINES_H__*/
