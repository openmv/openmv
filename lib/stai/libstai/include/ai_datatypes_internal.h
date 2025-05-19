/**
  ******************************************************************************
  * @file    ai_datatypes_internal.h
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
#ifndef AI_DATATYPES_INTERNAL_H
#define AI_DATATYPES_INTERNAL_H

#include "ai_datatypes.h"
#include "ai_datatypes_defines.h"

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

/*!  AI_STORAGE_KLASS SECTION              ************************************/
#define AI_STORAGE_KLASS_TYPE(s_) \
  ( (s_)->type )

#define AI_STORAGE_KLASS_SIZE(s_) \
  ( (s_)->size )

#define AI_STORAGE_KLASS_DATA(s_, type_) \
  ( (type_*)((s_)->data) )

#define AI_STORAGE_KLASS_COPY(dst_, dst_type_, src_, src_type_) \
{ \
  AI_ASSERT(AI_STORAGE_KLASS_SIZE(src_)>=AI_STORAGE_KLASS_SIZE(dst_)) \
  AI_STORAGE_KLASS_SIZE(dst_) = AI_STORAGE_KLASS_SIZE(src_); \
  for (ai_size i=0; i<AI_STORAGE_KLASS_SIZE(dst_); i++ ) { \
    AI_STORAGE_KLASS_DATA(dst_, dst_type_)[i] = \
      AI_STORAGE_KLASS_DATA(src_, src_type_)[i]; \
  } \
}

#define AI_STORAGE_KLASS_DUMP(s_, pfx_, post_, fmt_, type_) \
{ \
  AI_ASSERT(s_) \
  AI_DEBUG_PRINT(pfx_, AI_STORAGE_KLASS_SIZE(s_)) \
  for ( ai_u32 i=0; i<AI_STORAGE_KLASS_SIZE(s_); i++ ) { \
    if ( (i % 8)==0 ) { AI_DEBUG_PRINT("\n      ") } \
    AI_DEBUG_PRINT(fmt_, AI_STORAGE_KLASS_DATA(s_, type_)[i]) \
  } \
  AI_DEBUG_PRINT(post_) \
}

/*!  AI_SHAPES SECTION                     ************************************/
#define AI_SHAPE_2D_H(shape_) \
  AI_SHAPE_ELEM(shape_, AI_SHAPE_2D_HEIGHT)

#define AI_SHAPE_2D_W(shape_) \
  AI_SHAPE_ELEM(shape_, AI_SHAPE_2D_WIDTH)

#define AI_SHAPE_ELEM(shape_, pos_) \
  AI_STORAGE_KLASS_DATA(shape_, ai_shape_dimension)[pos_]

#define AI_SHAPE_GET_ELEM(shape_, pos_) \
  (((pos_) < AI_SHAPE_SIZE(shape_)) ? AI_SHAPE_ELEM(shape_, pos_) : 1)

#define AI_SHAPE_SET_ELEM(shape_, pos_, val_) \
  if ((pos_) < AI_SHAPE_SIZE(shape_)) { AI_SHAPE_ELEM(shape_, pos_) = (val_); }

#define AI_SHAPE_TYPE(shape_) \
  AI_STORAGE_KLASS_TYPE(shape_)

#define AI_SHAPE_SIZE(shape_) \
  AI_STORAGE_KLASS_SIZE(shape_)

#define AI_SHAPE_CLONE(dst_, src_) \
  AI_STORAGE_KLASS_COPY(dst_, ai_shape_dimension, src_, ai_shape_dimension)

#define AI_SHAPE_BCAST_CLONE(dst_, src_) \
{ \
  for (ai_size i = 0; i < AI_SHAPE_SIZE(dst_); i++) { \
    AI_SHAPE_SET_ELEM(dst_, i, AI_SHAPE_GET_ELEM(src_, i)); \
  } \
}

//#define AI_SHAPE_BATCH(shape_)      AI_SHAPE_ELEM((shape_), AI_SHAPE_BATCH_CHANNEL)
#define AI_SHAPE_H(shape_)          AI_SHAPE_ELEM((shape_), AI_SHAPE_HEIGHT)
#define AI_SHAPE_W(shape_)          AI_SHAPE_ELEM((shape_), AI_SHAPE_WIDTH)
#define AI_SHAPE_CH(shape_)         AI_SHAPE_ELEM((shape_), AI_SHAPE_CHANNEL)
#define AI_SHAPE_IN_CH(shape_)      AI_SHAPE_ELEM((shape_), AI_SHAPE_IN_CHANNEL)
#define AI_SHAPE_D(shape_)          ((AI_SHAPE_SIZE((shape_)) > AI_SHAPE_DEPTH) \
  ? AI_SHAPE_ELEM((shape_), AI_SHAPE_DEPTH) : 1)
#define AI_SHAPE_E(shape_)          ((AI_SHAPE_SIZE((shape_)) > AI_SHAPE_EXTENSION) \
  ? AI_SHAPE_ELEM((shape_), AI_SHAPE_EXTENSION) : 1)
#define AI_SHAPE_T(shape_)          AI_SHAPE_ELEM((shape_), AI_SHAPE_TIME)

#define AI_CONV_SHAPE_H             AI_SHAPE_W
#define AI_CONV_SHAPE_W             AI_SHAPE_CH
#define AI_CONV_SHAPE_CH            AI_SHAPE_H
#define AI_CONV_SHAPE_IN_CH         AI_SHAPE_IN_CH

/*!  AI_STRIDES SECTION                     ***********************************/
#define AI_STRIDE_2D_H(stride_) \
  AI_STRIDE_ELEM((stride_), AI_SHAPE_2D_HEIGHT)

#define AI_STRIDE_2D_W(stride_) \
  AI_STRIDE_ELEM((stride_), AI_SHAPE_2D_WIDTH)

#define AI_STRIDE_ELEM(stride_, pos_) \
  AI_STORAGE_KLASS_DATA(stride_, ai_stride_dimension)[pos_]

#define AI_STRIDE_GET_ELEM(stride_, pos_) \
  (((pos_) < AI_STRIDE_SIZE(stride_)) ? AI_STRIDE_ELEM(stride_, pos_) : 0)

#define AI_STRIDE_SET_ELEM(stride_, pos_, val_) \
  if ((pos_) < AI_STRIDE_SIZE(stride_)) AI_STRIDE_ELEM(stride_, pos_) = (val_);

#define AI_STRIDE_TYPE(stride_) \
  AI_STORAGE_KLASS_TYPE(stride_)

#define AI_STRIDE_SIZE(stride_) \
  AI_STORAGE_KLASS_SIZE(stride_)

#define AI_STRIDE_CLONE(dst_, src_) \
  AI_STORAGE_KLASS_COPY(dst_, ai_stride_dimension, src_, ai_stride_dimension)

#define AI_STRIDE_BCAST_CLONE(dst_, src_) \
{ \
  for (ai_size i=0; i<AI_STRIDE_SIZE(dst_); i++) { \
    AI_STRIDE_SET_ELEM(dst_, i, AI_STRIDE_GET_ELEM(src_, i)); \
  } \
}

//#define AI_STRIDE_BATCH(stride)     AI_STRIDE_ELEM((stride), AI_SHAPE_BATCH_CHANNEL)
#define AI_STRIDE_H(stride)         AI_STRIDE_ELEM((stride), AI_SHAPE_HEIGHT)
#define AI_STRIDE_W(stride)         AI_STRIDE_ELEM((stride), AI_SHAPE_WIDTH)
#define AI_STRIDE_CH(stride)        AI_STRIDE_ELEM((stride), AI_SHAPE_CHANNEL)
#define AI_STRIDE_IN_CH(stride)     AI_STRIDE_ELEM((stride), AI_SHAPE_IN_CHANNEL)
#define AI_STRIDE_D(stride)         ((AI_STRIDE_SIZE((stride)) >= 5) ? AI_STRIDE_ELEM((stride), AI_SHAPE_DEPTH) : 0)
#define AI_STRIDE_E(stride)         ((AI_STRIDE_SIZE((stride)) == 6) ? AI_STRIDE_ELEM((stride), AI_SHAPE_EXTENSION) : 0)
#define AI_STRIDE_T(stride)         AI_STRIDE_ELEM((stride), AI_SHAPE_TIME)

#define AI_STRIDE_SET_H(stride, val)         AI_STRIDE_SET_ELEM((stride), AI_SHAPE_HEIGHT, val)
#define AI_STRIDE_SET_W(stride, val)         AI_STRIDE_SET_ELEM((stride), AI_SHAPE_WIDTH, val)
#define AI_STRIDE_SET_CH(stride, val)        AI_STRIDE_SET_ELEM((stride), AI_SHAPE_CHANNEL, val)
#define AI_STRIDE_SET_IN_CH(stride, val)     AI_STRIDE_SET_ELEM((stride), AI_SHAPE_IN_CHANNEL, val)
#define AI_STRIDE_SET_D(stride, val)         if (AI_STRIDE_SIZE((stride)) >= 5) AI_STRIDE_SET_ELEM((stride), AI_SHAPE_DEPTH, val)
#define AI_STRIDE_SET_E(stride, val)         if (AI_STRIDE_SIZE((stride)) == 6) AI_STRIDE_SET_ELEM((stride), AI_SHAPE_EXTENSION, val)

/*!  AI_TENSORS SECTION                     ***********************************/
#define AI_TENSOR_KLASS(tensor_) \
  ((tensor_) ? (tensor_)->klass : NULL)

#define AI_TENSOR_SHAPE(tensor_) \
  (&((tensor_)->shape))

#define AI_TENSOR_STRIDE(tensor_) \
  (&((tensor_)->stride))

#define AI_TENSOR_INFO(tensor_) \
  (&((tensor_)->info))

#define AI_TENSOR_ARRAY(tensor_) \
  ((tensor_) ? (tensor_)->data : NULL)

#define AI_TENSOR_ID(tensor_) \
  ((tensor_) ? AI_TENSOR_INFO(tensor_)->id : 0)

#define AI_TENSOR_FLAGS(tensor_) \
  ((tensor_) ? AI_TENSOR_INFO(tensor_)->flags : 0)

#define AI_TENSOR_DATA_SIZE(tensor_) \
  ((tensor_) ? AI_TENSOR_INFO(tensor_)->data_size : 0)

/*!  AI_OFFSETS SECTION                     ***********************************/
//#define AI_OFFSET_BATCH(b, stride)  ((ai_ptr_offset)(b)  * AI_STRIDE_BATCH(stride))
#define AI_OFFSET_H(y, stride)      ((ai_ptr_offset)(y)  * AI_STRIDE_H(stride))
#define AI_OFFSET_W(x, stride)      ((ai_ptr_offset)(x)  * AI_STRIDE_W(stride))
#define AI_OFFSET_CH(ch, stride)    ((ai_ptr_offset)(ch) * AI_STRIDE_CH(stride))
#define AI_OFFSET_IN_CH(in_ch, stride) ((ai_ptr_offset)(in_ch) * \
                                      AI_STRIDE_IN_CH(stride))
#define AI_OFFSET_D(d, stride)      ((ai_ptr_offset)(d) * AI_STRIDE_D(stride))
#define AI_OFFSET_E(e, stride)      ((ai_ptr_offset)(e) * AI_STRIDE_E(stride))

#define AI_OFFSET_5D(y, x, d, e, ch, stride) ( \
  AI_OFFSET_H((y), (stride)) + AI_OFFSET_W((x), (stride)) + \
  AI_OFFSET_D((d), (stride)) + AI_OFFSET_E((e), (stride)) + \
  AI_OFFSET_CH((ch), (stride)) )

#define AI_OFFSET(y, x, ch, z, stride) ( \
  AI_OFFSET_H((y), (stride)) + AI_OFFSET_W((x), (stride)) + \
  AI_OFFSET_CH((ch), (stride)) + \
  ((AI_STRIDE_SIZE((stride)) == 4) ? AI_OFFSET_IN_CH((z), (stride)) : AI_OFFSET_D((z), (stride))) )

/*! @} */

#define AI_GET_CONV_OUT_SIZE(in_size, filt_size, pad_l, pad_r, filt_stride) \
  ((((in_size) - (filt_size) + (pad_l) + (pad_r)) / (filt_stride)) + 1)


/** Tensors datatypes defines handlers ****************************************/
#define AI_TENSOR_SIZE(tensor_) \
  get_tensor_size(tensor_, true)

#define AI_TENSOR_SIZE_UNPAD(tensor_) \
  get_tensor_size(tensor_, false)

#define AI_TENSOR_BYTE_SIZE(tensor_) \
  get_tensor_byte_size(tensor_)


/******************************************************************************/
#define AI_PLATFORM_VERSION_INIT(major_, minor_, micro_) \
  { .major = (major_), .minor = (minor_), .micro = (micro_), .reserved = 0x0 }


/** Integer tensor info extraction ********************************************/
#define AI_INTQ_INFO_LIST_SCALE_ARRAY(list_, type_) \
  ( ((list_) && (list_)->info) \
    ? ((type_*)((list_)->info->scale)) : NULL )

#define AI_INTQ_INFO_LIST_ZEROPOINT_ARRAY(list_, type_) \
  ( ((list_) && (list_)->info) \
    ? ((type_*)((list_)->info->zeropoint)) : NULL )

#define AI_KLASS_GET_INTQ_INFO_LIST(tensor_) \
  ((ai_intq_info_list*)((tensor_)->klass))


AI_API_DECLARE_BEGIN

/*!
 * @brief Check whether 2 shapes have identical dimensions.
 * @ingroup datatypes_internal
 * @param shape0 the 1st tensor shape to compare
 * @param shape1 the 2nd tensor shape to compare
 * @return true if shape0 and shape1 have same dimensions. false otherwise
 */
AI_DECLARE_STATIC
ai_bool ai_shape_is_same(
  const ai_shape* shape0, const ai_shape* shape1)
{
  AI_ASSERT(shape0 && shape1)
  if (AI_SHAPE_SIZE(shape0) != AI_SHAPE_SIZE(shape1))
    return false;
  ai_size dim = AI_SHAPE_SIZE(shape0);
  while ( dim>0 ) {
    dim--;
    if ( AI_SHAPE_ELEM(shape0, dim)!=AI_SHAPE_ELEM(shape1, dim) )
      return false;
  }
  return true;
}


/*!
 * @brief Check whether the shapes is 1*1*1... for a scalar value content.
 * @ingroup datatypes_internal
 * @param shape the tensor shape to evaluate
 * @return true if shape0 is scalar false otherwise
 */
AI_DECLARE_STATIC
ai_bool ai_shape_is_scalar(
  const ai_shape* shape0)
{
  ai_size dim = AI_SHAPE_SIZE(shape0);
  while (dim>0) {
    dim--;
    if (AI_SHAPE_ELEM(shape0, dim) != 1)
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
ai_bool ai_shape_is_subshape(
  const ai_shape* shape0, const ai_shape* shape1)
{
  AI_ASSERT(shape0 && shape1)
  AI_ASSERT(AI_SHAPE_SIZE(shape0)==AI_SHAPE_SIZE(shape1))
  ai_size dim = AI_SHAPE_SIZE(shape0);
  while (dim) {
    dim--;
    if ( AI_SHAPE_ELEM(shape0, dim)>AI_SHAPE_ELEM(shape1, dim) )
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
ai_size ai_shape_get_size(const ai_shape* shape)
{
  AI_ASSERT(shape)
  ai_size dim = AI_SHAPE_SIZE(shape);
  AI_ASSERT(dim > 0)
  ai_size size = 1;
  while (dim>0) {
    dim--;
    size *= AI_SHAPE_ELEM(shape, dim);
  }
  return size;
}

/*!
 * @brief Computes the size of the input image discarding the channels.
 * @ingroup datatypes_internal
 * @param shape the tensor shape
 */
AI_DECLARE_STATIC
ai_size ai_shape_get_npixels(const ai_shape* shape)
{
  AI_ASSERT(shape)
  const ai_size npixels = AI_SHAPE_W(shape) * AI_SHAPE_H(shape);
  return npixels;
}

/** APIs Section *************************************************************/
/*!
 * @brief Get packed version from major, minor, micro representaion.
 * @ingroup datatypes_internal
 * @param major major version value
 * @param minor minor version value
 * @param micro micro version value
 * @return a packed version info obtained serializing input values
 */
AI_INTERNAL_API
ai_version ai_version_get(const ai_u8 major, const ai_u8 minor, const ai_u8 micro);

/*!
 * @brief Get un-packed version from packed version representaion.
 * @ingroup datatypes_internal
 * @param version a packed varsion info
 * @return struct with de-serialized major, minor, micro values
 */
AI_INTERNAL_API
ai_platform_version ai_platform_version_get(const ai_version version);

/*!
 * @brief Map from ai_buffer data struct to ai_array data struct.
 * @ingroup datatypes_internal
 * @param buf a pointer to the ai_buffer to be mapped to ai_array
 * @return an initialized @ref ai_array struct representing same data
 */
AI_INTERNAL_API
ai_array ai_from_buffer_to_array(const ai_buffer* buf);

/*!
 * @brief Map from ai_array data struct to ai_buffer data struct.
 * @ingroup datatypes_internal
 * @param array a pointer to the ai_array to be mapped to ai_buffer
 * @return an initialized @ref ai_buffer struct representing same data
 */
AI_INTERNAL_API
ai_buffer ai_from_array_to_buffer(const ai_array* array);

/*!
 * @brief get the total number of elements of a n-dimensional tensor.
 * @ingroup datatypes_internal
 * @param t a pointer to an @ref ai_tensor
 * @param with_padding when true it considers also padded elements
 * @return the number of elements of the tensor (with/without padded ones)
 */
AI_INTERNAL_API
ai_size get_tensor_size(const ai_tensor* t, const ai_bool with_padding);

/*!
 * @brief get the total size in bytes of elements of a n-dimensional tensor (excluding padded ones).
 * @ingroup datatypes_internal
 * @param t a pointer to an @ref ai_tensor
 * @return the total size in bytes of elements of the tensor (excluding padded ones)
 */
AI_INTERNAL_API
ai_size get_tensor_byte_size(const ai_tensor* t);


AI_API_DECLARE_END

#endif /*AI_DATATYPES_INTERNAL_H*/
