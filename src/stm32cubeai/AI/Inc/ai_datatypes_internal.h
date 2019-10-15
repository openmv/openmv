/**
  ******************************************************************************
  * @file    ai_datatypes_internal.h
  * @author  AST Embedded Analytics Research Platform
  * @date    01-May-2017
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

#ifndef __AI_DATATYPES_INTERNAL_H__
#define __AI_DATATYPES_INTERNAL_H__
#pragma once

#include <string.h>
#include "ai_platform.h"
#include "ai_platform_interface.h"

/*!
 * @defgroup datatypes_internal Internal Datatypes
 * @brief Data structures used internally to implement neural networks
 *
 * The layers are defined as structs; a generic layer type defines the basic
 * layer parameters and type-specific parameters are handled by specializations
 * implemented as a C union. The layers keep also a pointer to the parent
 * network and the next layer in the network.
 * The input, output and parameters are tensor with an hard-coded maximum
 * dimension of 4. Tensors are floating point arrays with a notion of size.
 * The network is a linked list of layers, and thus it stores only the pointer
 * to the first layer.
 */

/*!
 * @section Offsets
 * @ingroup datatypes_internal
 * Macros to handle (byte) stride addressing on tensors. The `AI_PTR` macro
 * is used to always cast a pointer to byte array. The macros `AI_OFFSET_X` are
 * used to compute (byte) offsets of respectively adjacents row elements, col
 * elements, channel elements and `channel_in` elements.
 * @{
 */

/** Count Variable Number of Arguments (up to 64 elements) ********************/
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

#define AI_PTR_ALIGN(ptr, alignment) \
  ( (((ai_uptr)(ptr))+((alignment)-1))&(~((alignment)-1)) )


#define AI_DIMENSION(item_, pos_) \
  ((item_).dimension[(pos_)])

/******************************************************************************/
#define AI_BITS_TO_BYTES(bits_) \
  (((bits_)+0x7) >> 3)

#define AI_BYTES_TO_BITS(bytes_) \
  ((bytes_) << 3)

/******************************************************************************/
#define AI_SHAPE_2D_H(shape_) \
  AI_DIMENSION((shape_), AI_SHAPE_2D_HEIGHT)

#define AI_SHAPE_2D_W(shape_) \
  AI_DIMENSION((shape_), AI_SHAPE_2D_WIDTH)

#define AI_SHAPE_ND_SIZE(shape_) \
  ((shape_).size)

#define AI_SHAPE_ND_ELEM(shape_, pos_) \
  AI_DIMENSION(shape_, pos_)

/******************************************************************************/
#define AI_STRIDE_2D_H(stride_) \
  AI_DIMENSION((stride_), AI_SHAPE_2D_HEIGHT)


#define AI_STRIDE_2D_W(stride_) \
  AI_DIMENSION((stride_), AI_SHAPE_2D_WIDTH)

#define AI_STRIDE_ND_SIZE(stride_) \
  ((stride_).size)

#define AI_STRIDE_ND_ELEM(stride_, pos_) \
  AI_DIMENSION(stride_, pos_)

/******************************************************************************/
//#define AI_SHAPE_BATCH(shape)       AI_DIMENSION((shape), AI_SHAPE_BATCH_CHANNEL)
#define AI_SHAPE_H(shape)           AI_DIMENSION((shape), AI_SHAPE_HEIGHT)
#define AI_SHAPE_W(shape)           AI_DIMENSION((shape), AI_SHAPE_WIDTH)
#define AI_SHAPE_CH(shape)          AI_DIMENSION((shape), AI_SHAPE_CHANNEL)
#define AI_SHAPE_IN_CH(shape)       AI_DIMENSION((shape), AI_SHAPE_IN_CHANNEL)

#define AI_CONV_SHAPE_H             AI_SHAPE_W
#define AI_CONV_SHAPE_W             AI_SHAPE_CH
#define AI_CONV_SHAPE_CH            AI_SHAPE_H
#define AI_CONV_SHAPE_IN_CH         AI_SHAPE_IN_CH

//#define AI_STRIDE_BATCH(stride)     AI_DIMENSION((stride), AI_SHAPE_BATCH_CHANNEL)
#define AI_STRIDE_H(stride)         AI_DIMENSION((stride), AI_SHAPE_HEIGHT)
#define AI_STRIDE_W(stride)         AI_DIMENSION((stride), AI_SHAPE_WIDTH)
#define AI_STRIDE_CH(stride)        AI_DIMENSION((stride), AI_SHAPE_CHANNEL)
#define AI_STRIDE_IN_CH(stride)     AI_DIMENSION((stride), AI_SHAPE_IN_CHANNEL)


//#define AI_OFFSET_BATCH(b, stride)  ((ai_ptr_offset)(b)  * AI_STRIDE_BATCH(stride))
#define AI_OFFSET_H(y, stride)      ((ai_ptr_offset)(y)  * AI_STRIDE_H(stride))
#define AI_OFFSET_W(x, stride)      ((ai_ptr_offset)(x)  * AI_STRIDE_W(stride))
#define AI_OFFSET_CH(ch, stride)    ((ai_ptr_offset)(ch) * AI_STRIDE_CH(stride))
#define AI_OFFSET_IN_CH(ch, stride) ((ai_ptr_offset)(ch) * \
                                      AI_STRIDE_IN_CH(stride))
#define AI_OFFSET(y, x, ch, in_ch, stride) ( \
  AI_OFFSET_H((y), (stride)) + AI_OFFSET_W((x), (stride)) + \
  AI_OFFSET_CH((ch), (stride)) + AI_OFFSET_IN_CH((in_ch), (stride)) )

/*! @} */

#define AI_GET_CONV_OUT_SIZE(in_size, filt_size, pad_l, pad_r, filt_stride) \
  ((((in_size) - (filt_size) + (pad_l) + (pad_r)) / (filt_stride)) + 1)


/** Tensors datatypes defines handlers ****************************************/
#define AI_TENSOR_SIZE(tensor_) \
  ( AI_SHAPE_H((tensor_)->shape) * AI_SHAPE_W((tensor_)->shape) * \
    AI_SHAPE_CH((tensor_)->shape) * AI_SHAPE_IN_CH((tensor_)->shape) )

#define AI_TENSOR_BYTE_SIZE(tensor_) \
  ( AI_SHAPE_H((tensor_)->shape) * AI_STRIDE_H((tensor_)->stride) )

/******************************************************************************/

AI_API_DECLARE_BEGIN

/*!
 * @typedef ai_offset
 * @ingroup ai_datatypes_internal
 * @brief Generic index offset type
 */
typedef int32_t ai_offset;


/*!
 * @typedef ai_vec4_float
 * @ingroup ai_datatypes_internal
 * @brief 32bit X 4 float (optimization for embedded MCU)
 */
typedef struct _ai_vec4_float {
    ai_float a1;
    ai_float a2;
    ai_float a3;
    ai_float a4;
} ai_vec4_float;


#define AI_VEC4_FLOAT(ptr_) \
  _get_vec4_float((ai_handle)(ptr_))

AI_DECLARE_STATIC
ai_vec4_float _get_vec4_float(const ai_handle fptr)
{
    return *((const ai_vec4_float*)fptr);
}

/*!
 * @typedef (*func_copy_tensor)
 * @ingroup datatypes_internal
 * @brief Fuction pointer for generic tensor copy routines
 * this function pointer abstracts a generic tensor copy routine.
 */
typedef ai_bool (*func_copy_tensor)(ai_tensor* dst, const ai_tensor* src);


/*!
 * @brief Check whether 2 shapes have identical dimensions.
 * @ingroup datatypes_internal
 * @param shape0 the 1st tensor shape to compare
 * @param shape1 the 2nd tensor shape to compare
 * @return true if shape0 and shape1 have same dimensions. false otherwise
 */
AI_DECLARE_STATIC
ai_bool ai_shape_is_same(const ai_shape* shape0, const ai_shape* shape1) {
  AI_ASSERT(shape0 && shape1)
  for (ai_size i = 0; i < AI_SHAPE_MAX_DIMENSION; ++i) {
    if ( shape0->dimension[i]!=shape1->dimension[i] )
      return false;
  }
  return true;
}

/*!
 * @brief Check if shape0 is a subshape of shape1
 * @ingroup datatypes_internal
 * @param shape0 the 1st tensor shape to compare
 * @param shape1 the 2nd tensor shape to compare
 * @return true if shape0 is a subshape of shape1 (all shape0 dimensions are
*  smallers or equal of the shape1 ones). false otherwise
 */
AI_DECLARE_STATIC
ai_bool ai_shape_is_subshape(const ai_shape* shape0, const ai_shape* shape1) {
  AI_ASSERT(shape0 && shape1)
  for (ai_size i = 0; i < AI_SHAPE_MAX_DIMENSION; ++i) {
    if ( shape0->dimension[i]>shape1->dimension[i] )
      return false;
  }
  return true;
}

/*!
 * @brief Computes the total size of a tensor given its dimensions.
 * @ingroup datatypes_internal
 * @param shape the tensor shape
 */
AI_DECLARE_STATIC
ai_size ai_shape_get_size(const ai_shape* shape) {
  AI_ASSERT(shape)
  ai_size size = 1;
  for (ai_size i = 0; i < AI_SHAPE_MAX_DIMENSION; ++i) {
    size *= shape->dimension[i];
  }

  return size;
}

/*!
 * @brief Computes the size of the input image discarding the channels.
 * @ingroup datatypes_internal
 * @param shape the tensor shape
 */
AI_DECLARE_STATIC
ai_size ai_shape_get_npixels(const ai_shape* shape) {
  AI_ASSERT(shape)
  const ai_size npixels =
      shape->dimension[AI_SHAPE_WIDTH] * shape->dimension[AI_SHAPE_HEIGHT];

  return npixels;
}

/*!
 * @brief Map from ai_buffer data struct to ai_array data struct.
 * @ingroup datatypes_internal
 * @param buf a pointer to the ai_buffer to be mapped to ai_array
 * @return an initialized @ref ai_array struct representing same data
 */
AI_DECLARE_STATIC
ai_array ai_from_buffer_to_array(const ai_buffer* buf)
{
  AI_ASSERT(buf)
  const ai_u32 size = AI_BUFFER_SIZE(buf) * buf->n_batches;

  AI_ARRAY_OBJ_DECLARE(a, AI_BUFFER_TO_ARRAY_FMT(AI_BUFFER_FMT_OBJ(buf->format)),
                       buf->data, buf->data, size, AI_CONST);
  return a;
}

/*!
 * @brief Map from ai_array data struct to ai_buffer data struct.
 * @ingroup datatypes_internal
 * @param array a pointer to the ai_array to be mapped to ai_buffer
 * @return an initialized @ref ai_buffer struct representing same data
 */
AI_DECLARE_STATIC
ai_buffer ai_from_array_to_buffer(const ai_array* array)
{
  AI_ASSERT(array)
  const ai_buffer b = AI_BUFFER_OBJ_INIT(AI_ARRAY_TO_BUFFER_FMT(array->format), \
                        1, 1, array->size, 1, array->data_start);
  return b;
}

AI_API_DECLARE_END

#endif /*__AI_DATATYPES_INTERNAL_H__*/
