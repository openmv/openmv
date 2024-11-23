/**
  ******************************************************************************
  * @file    ai_lite_interface.h
  * @author  STMicroelectronics
  * @brief   Definitions and implementations of runtime-lite codegen APIs
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef AI_LITE_INTERFACE_H
#define AI_LITE_INTERFACE_H

#include "ai_lite.h"
#include "core_assert.h"


/*****************************************************************************/
/* Generic Codegen Section */
// #ifdef HAS_LOG
#if 0
#include "core_log.h"

#define LITE_GRAPH_START(_graph_name) \
  AI_LOG_DEBUG("[LITE GRAPH START] : " _graph_name)

#define LITE_GRAPH_END(_graph_name) \
  AI_LOG_DEBUG("[LITE GRAPH END] : " _graph_name)

#else

#define LITE_GRAPH_START(_graph_name) \
  /* LITE_GRAPH_START() */

#define LITE_GRAPH_END(_graph_name) \
  /* LITE_GRAPH_END() */

#endif      /* HAS_LOG */

#define LITE_ASSERT(expr) \
  CORE_ASSERT(expr)


/*****************************************************************************/
#if defined(_MSC_VER)
  #define LITE_DECLARE_STATIC         static __inline
  #define LITE_HINT_INLINE            static __inline
  #define LITE_FORCE_INLINE           static __inline
#elif defined(__ICCARM__) || defined (__IAR_SYSTEMS_ICC__)
  #define LITE_DECLARE_STATIC         static inline
  #define LITE_HINT_INLINE            static inline
  #define LITE_FORCE_INLINE           static inline
#elif defined(__GNUC__)
  #define LITE_DECLARE_STATIC         static __inline
  #define LITE_HINT_INLINE            static __inline
  #define LITE_FORCE_INLINE           static __inline
#else
  #define LITE_DECLARE_STATIC         static __inline
  #define LITE_HINT_INLINE            static __inline
  #define LITE_FORCE_INLINE           static __inline
#endif /* _MSC_VER */

#define LITE_API_ENTRY                /* LITE_API_ENTRY */

#define LITE_PACK(...) \
  __VA_ARGS__

#define LITE_UNUSED(_elem) \
  ((void)(_elem));


/*****************************************************************************/
/* Arrays Section */

#define LITE_ARRAY_VALUES(...) \
  { LITE_PACK(__VA_ARGS__) }

#define LITE_ARRAY_DATA(_array, _type) \
  ((_type*)(_array)->data)

#define LITE_ARRAY_DATA_START(_array, _type) \
  ((_type*)(_array)->data_start)

/*****************************************************************************/
/* Tensors Section */

#define LITE_TENSOR_ARRAY(_tensor, _pos) \
  (((_tensor)->data) + (_pos))

/*****************************************************************************/
/* Tensors List Section */

#define LITE_TENSOR_LIST(_chain, _pos) \
  (&(_chain)->chain[_pos])

#define LITE_TENSOR_IN(_chain, _pos) \
  (LITE_TENSOR_LIST(_chain, 0)->tensor[_pos])

#define LITE_TENSOR_OUT(_chain, _pos) \
  (LITE_TENSOR_LIST(_chain, 1)->tensor[_pos])

#define LITE_TENSOR_WEIGHTS(_chain, _pos) \
  (LITE_TENSOR_LIST(_chain, 2)->tensor[_pos])

#define LITE_TENSOR_SCRATCHS(_chain, _pos) \
  (LITE_TENSOR_LIST(_chain, 3)->tensor[_pos])

/*****************************************************************************/
#define LITE_LAYER_ACQUIRE(name_, cast_type_, ptr_) \
  LITE_ASSERT(ptr_) \
  AI_CONCAT(ai_layer_, cast_type_)* name_ = \
    (AI_CONCAT(ai_layer_, cast_type_)*)(ptr_);

#define LITE_LAYER_RELEASE(name_, cast_type_) \
  /* LITE_LAYER_RELEASE() */


#endif    /* AI_LITE_INTERFACE_H */
