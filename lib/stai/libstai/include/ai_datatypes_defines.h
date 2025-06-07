/**
  ******************************************************************************
  * @file    ai_datatypes_defines.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of AI platform private APIs types
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
#ifndef AI_DATATYPES_DEFINES_H
#define AI_DATATYPES_DEFINES_H

#include "ai_platform.h"
#include "core_assert.h"


/*!
 * @defgroup datatypes_defines Internal Datatypes Defines Header
 * @brief Data structures used internally to implement neural networks
 *
 */

/* define to track datatypes used by codegen */
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
#define AI_ASSERT(expr) \
  CORE_ASSERT(expr)


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
  // #define AI_FORCE_INLINE             static __forceinline
  #define AI_FORCE_INLINE             static __inline
  #define AI_HINT_INLINE              static __inline
  #define AI_ALIGNED_TYPE(type, x)    type __declspec(align(x))
  #define AI_INTERFACE_ENTRY         __declspec(dllexport)
#elif defined(__ICCARM__) || defined (__IAR_SYSTEMS_ICC__)
  #define AI_DECLARE_STATIC           static inline
  // #define AI_FORCE_INLINE             static _Pragma("inline=forced")  // TODO: check this definition!
  #define AI_FORCE_INLINE             static inline
  #define AI_HINT_INLINE              static inline
  #define AI_ALIGNED_TYPE(type, x)    type
  #define AI_INTERFACE_ENTRY          /* AI_INTERFACE_ENTRY */
#elif defined(__GNUC__)
  #define AI_DECLARE_STATIC           static __inline
  #define AI_FORCE_INLINE             static __inline
  #define AI_HINT_INLINE              static __inline
  #define AI_ALIGNED_TYPE(type, x)    type __attribute__ ((aligned(x)))
  #define AI_INTERFACE_ENTRY          /* AI_INTERFACE_ENTRY */
#else /* _MSC_VER */
  #define AI_DECLARE_STATIC           static __inline
  // #define AI_FORCE_INLINE             static __forceinline
  #define AI_FORCE_INLINE             static __inline
  #define AI_HINT_INLINE              static __inline
  #define AI_ALIGNED_TYPE(type, x)    type __attribute__ ((aligned(x)))
  #define AI_INTERFACE_ENTRY         __attribute__((visibility("default")))
#endif /* _MSC_VER */

/******************************************************************************/
#define AI_ALIGN_MASKED(value, mask)    ( ((value)+(mask))&(~(mask)) )

#define AI_GET_VERSION_STRING(major, minor, micro) \
          AI_STRINGIFY_ARG(major) "." \
          AI_STRINGIFY_ARG(minor) "." \
          AI_STRINGIFY_ARG(micro) \


#define AI_PACK_TENSORS_PTR(...) \
  AI_PACK(__VA_ARGS__)

#define AI_PACK_INFO(size_)   (ai_tensor_info[1]) { { \
    .buffer = (ai_buffer[size_])AI_STRUCT_INIT, \
    .state = (ai_tensor_state[size_])AI_STRUCT_INIT, \
} }

#define AI_CR                         "\r\n"

#if (defined HAS_AI_DEBUG || defined HAS_DEBUG_LIB)
#include <stdio.h>
#define AI_DEBUG(...)                __VA_ARGS__
#define AI_DEBUG_PRINT(fmt, ...)     { printf(fmt, ##__VA_ARGS__); }
#else
#define AI_DEBUG(...)                AI_WRAP_FUNC(/*AI_DEBUG*/)
#define AI_DEBUG_PRINT(fmt, ...)     AI_WRAP_FUNC(/*AI_DEBUG_PRINT*/)
#endif

#define AI_FLAG_SET(mask, flag)       (mask) |= (flag)
#define AI_FLAG_UNSET(mask, flag)     (mask) &= (~(flag))
#define AI_FLAG_IS_SET(mask, flag)    ((flag)==((mask)&(flag)))

#endif    /*AI_DATATYPES_DEFINES_H*/
