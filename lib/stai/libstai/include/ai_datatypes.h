
#ifndef AI_DATATYPES_H
#define AI_DATATYPES_H
/**
  ******************************************************************************
  * @file    ai_datatypes.h
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
#include <string.h>
#include "ai_platform.h"
#include "ai_platform_interface.h"

/*!
 * @defgroup datatypes Platform Interface Datatypes
 * @brief Data structures used by AI platform to implement neural networks
 *
 */

/** Count Variable Number of Arguments (up to 64 elements) *******************/
#define AI_NUMARGS(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                  \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0


/*****************************************************************************/
#define AI_PTR_ALIGN(ptr, alignment) \
  ((((ai_uptr)(ptr))+((ai_uptr)(alignment)-1))&(~((ai_uptr)(alignment)-1)))


/*!
 * @typedef ai_offset
 * @ingroup ai_datatypes_internal
 * @brief Generic index offset type
 */
typedef int32_t ai_offset;


AI_API_DECLARE_BEGIN

AI_API_DECLARE_END

#endif  /* AI_DATATYPES_H */
