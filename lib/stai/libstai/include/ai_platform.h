/**
  ******************************************************************************
  * @file    ai_platform.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Definitions of AI platform public APIs types
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
#ifndef AI_PLATFORM_H
#define AI_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#ifndef AI_PLATFORM_API_MAJOR
#define AI_PLATFORM_API_MAJOR           (1)
#endif
#ifndef AI_PLATFORM_API_MINOR
#define AI_PLATFORM_API_MINOR           (2)
#endif
#ifndef AI_PLATFORM_API_MICRO
#define AI_PLATFORM_API_MICRO           (0)
#endif

#define AI_PLATFORM_API_VERSION \
  AI_VERSION(AI_PLATFORM_API_MAJOR, \
             AI_PLATFORM_API_MINOR, \
             AI_PLATFORM_API_MICRO)


#ifndef AI_TOOLS_API_VERSION_MAJOR
#define AI_TOOLS_API_VERSION_MAJOR      (1)
#endif
#ifndef AI_TOOLS_API_VERSION_MINOR
#define AI_TOOLS_API_VERSION_MINOR      (5)
#endif
#ifndef AI_TOOLS_API_VERSION_MICRO
#define AI_TOOLS_API_VERSION_MICRO      (0)
#endif

/*****************************************************************************/
#define AI_TOOLS_API_VERSION \
  AI_VERSION(AI_TOOLS_API_VERSION_MAJOR, \
             AI_TOOLS_API_VERSION_MINOR, \
             AI_TOOLS_API_VERSION_MICRO)

#define AI_TOOLS_API_VERSION_1_3 \
  AI_VERSION(1, 3, 0)

#define AI_TOOLS_API_VERSION_1_4 \
  AI_VERSION(1, 4, 0)

#define AI_TOOLS_API_VERSION_1_5 \
  AI_VERSION(1, 5, 0)

/*****************************************************************************/
#ifdef __cplusplus
#define AI_API_DECLARE_BEGIN extern "C" {
#define AI_API_DECLARE_END }
#define ai_register             /* register */
#else
#include <stdbool.h>
#define AI_API_DECLARE_BEGIN    /* AI_API_DECLARE_BEGIN */
#define AI_API_DECLARE_END      /* AI_API_DECLARE_END   */
#define ai_register             register
#endif

/*****************************************************************************/
#define AI_FLAG_NONE            (0x0)

/*****************************************************************************/
AI_API_DECLARE_BEGIN

/*!
 * @typedef ai_flags
 * @ingroup ai_platform
 * @brief bitmask for flags management
 */
typedef uint32_t ai_flags;

/*****************************************************************************/
#define AI_CONCAT_ARG(a, b)     a ## b
#define AI_CONCAT(a, b)         AI_CONCAT_ARG(a, b)

/*!  AI_CAST SECTION                       ***********************************/
#define AI_CAST(type_, expr_)            ((type_)(expr_))

/*****************************************************************************/
#define AI_MAGIC_SIGNATURE \
  (0xa1facade)

#define AI_PACK(...) \
  __VA_ARGS__

/*****************************************************************************/
#define AI_SHAPE_BCWH             (0x01u)

/*!
 * @typedef ai_shape_dimension
 * @ingroup ai_platform
 * @brief shape dimension type to be used in shape related structs @ref ai_buffer_shape
 */
typedef uint32_t ai_shape_dimension;

/*****************************************************************************/
#if defined(_MSC_VER)
  #define AI_API_ENTRY          __declspec(dllexport)
  #define AI_ALIGNED(x)         /* AI_ALIGNED(x) */
#elif defined(__ICCARM__) || defined (__IAR_SYSTEMS_ICC__)
  #define AI_API_ENTRY          /* AI_API_ENTRY */
  #define AI_ALIGNED(x)         AI_CONCAT(AI_ALIGNED_,x)
  #define AI_ALIGNED_1          _Pragma("data_alignment = 1")
  #define AI_ALIGNED_2          _Pragma("data_alignment = 2")
  #define AI_ALIGNED_4          _Pragma("data_alignment = 4")
  #define AI_ALIGNED_8          _Pragma("data_alignment = 8")
  #define AI_ALIGNED_16         _Pragma("data_alignment = 16")
  #define AI_ALIGNED_32         _Pragma("data_alignment = 32")
#elif defined(__CC_ARM)
  #define AI_API_ENTRY          __attribute__((visibility("default")))
  #define AI_ALIGNED(x)         __attribute__((aligned (x)))
  /* Keil disallows anonymous union initialization by default */
  #pragma anon_unions
#elif defined(__GNUC__)
  //#define AI_API_ENTRY          __attribute__((visibility("default")))
  #define AI_API_ENTRY          /* AI_API_ENTRY */
  #define AI_ALIGNED(x)         __attribute__((aligned(x)))
#else
  /* Dynamic libraries are not supported by the compiler */
  #define AI_API_ENTRY                /* AI_API_ENTRY */
  #define AI_ALIGNED(x)               /* AI_ALIGNED(x) */
#endif

#define AI_HANDLE_PTR(ptr_)           ((ai_handle)(ptr_))
#define AI_HANDLE_NULL                AI_HANDLE_PTR(NULL)

#define AI_HANDLE_FUNC_PTR(func)      ((ai_handle_func)(func))

#define AI_UNUSED(x)                  (void)(x);

#define AI_DEPRECATED                 /* AI_DEPRECATED */

#define AI_LEGACY                     /* AI_LEGACY */

#define AI_MAGIC_MARKER               (0xA1FACADE)


#if defined(__cplusplus)
  #define AI_STRUCT_INIT              {}
  #define AI_C_ARRAY_INIT             {}
#else
  #define AI_STRUCT_INIT              {0}
  #define AI_C_ARRAY_INIT             {0}
#endif

#define AI_ERROR_FMT                  AIU32_FMT

#define AI_IS_UNSIGNED(type) \
  ((((type)0) - 1) > 0)

#define AI_CUSTOM_SIZE(type) \
  (ai_custom_type_signature)((AI_IS_UNSIGNED(type)) \
    ? (0x80|(sizeof(type)&0x7f)) : (sizeof(type)&0x7f))

/*! network buffers struct handlers *******************************************/
#ifdef __cplusplus

#define AI_NETWORK_PARAMS_INIT(params_, activations_) \
{ \
  {{ params_, activations_ }} \
}

#define AI_NETWORK_BUFFERS_INIT(weights_buffers_, activations_buffers_) \
{ \
  AI_MAGIC_SIGNATURE, AI_PACK(weights_buffers_), AI_PACK(activations_buffers_) \
}

#else

#define AI_NETWORK_PARAMS_INIT(params_, activations_) \
{ \
  .params = params_, \
  .activations = activations_ \
}

#define AI_NETWORK_BUFFERS_INIT(weights_buffers_, activations_buffers_) \
{ \
  .map_signature = AI_MAGIC_SIGNATURE, \
  .map_weights = AI_PACK(weights_buffers_), \
  .map_activations = AI_PACK(activations_buffers_) \
}

#endif    // __cplusplus


/*! binary padded bits macro helpers *****************************************/
#define AI_PBITS_MASK \
  (0x1F)

#define AI_PBITS_SHIFTS \
  (5)

#define AI_PBITS_PADDED_BYTES_COUNT(bits_) \
  (((ai_u32)(bits_) + 7) >> 3)

#define AI_PBITS_PADDED_WORDS_COUNT(bits_) \
  (((ai_size)(bits_) + AI_PBITS_MASK) >> AI_PBITS_SHIFTS)

#define AI_PBITS_GET_WORD(word_ptr_, bits_) \
  (((ai_pbits*)(word_ptr_)) + ((bits_) >> AI_PBITS_SHIFTS))

#define AI_PAD_CHANNELS(format_, channels_) \
  ((AI_BUFFER_FMT_GET_BITS(format_)==1) ? (AI_PBITS_PADDED_WORDS_COUNT(channels_) << AI_PBITS_SHIFTS) : (channels_))


/*! ai_intq_info struct handlers *********************************************/
#define INTQ_CONST    const
// #define INTQ_CONST

#define AI_INTQ_INFO_LIST(list_) \
  ((list_)->info)

#define AI_INTQ_INFO_LIST_FLAGS(list_) \
  ((list_) ? (list_)->flags : 0)

#define AI_INTQ_INFO_LIST_SIZE(list_) \
  ((list_) ? (list_)->size : 0)

#define AI_HAS_INTQ_INFO_LIST(list_) \
  ((list_) ? (((list_)->info) && ((list_)->size>0)) : false)

#define AI_INTQ_INFO_LIST_SCALE(list_, type_, pos_) \
  (((list_) && (list_)->info && ((pos_)<(list_)->size)) \
   ? ((type_*)((list_)->info->scale))[(pos_)] : 1.0f)

#define AI_INTQ_INFO_LIST_ZEROPOINT(list_, type_, pos_) \
  (((list_) && (list_)->info && ((pos_)<(list_)->size)) \
   ? ((type_*)((list_)->info->zeropoint))[(pos_)] : 0)

/*! ai_buffer format handlers ************************************************/

/*!
 * @enum buffer format definition
 * @ingroup ai_platform
 *
 * 32 bit signed format list.
 */
typedef int32_t ai_buffer_format;

/*! ai_buffer_meta flags & macros ********************************************/
#define AI_BUFFER_META_HAS_INTQ_INFO        (0x1U << 0)
#define AI_BUFFER_META_FLAG_SCALE_FLOAT     (0x1U << 0)
#define AI_BUFFER_META_FLAG_ZEROPOINT_U8    (0x1U << 1)
#define AI_BUFFER_META_FLAG_ZEROPOINT_S8    (0x1U << 2)
#define AI_BUFFER_META_FLAG_ZEROPOINT_U16   (0x1U << 3)
#define AI_BUFFER_META_FLAG_ZEROPOINT_S16   (0x1U << 4)

/*! ai_buffer format variable flags & macros *********************************/
#define AI_BUFFER_FMT_MASK                  (0x01FFFFFF)
#define AI_BUFFER_FMT_TYPE_NONE             (0x0)
#define AI_BUFFER_FMT_TYPE_FLOAT            (0x1)
#define AI_BUFFER_FMT_TYPE_Q                (0x2)
#define AI_BUFFER_FMT_TYPE_BOOL             (0x3)

#define AI_BUFFER_FMT_FLAG_CONST            (0x1U<<30)
#define AI_BUFFER_FMT_FLAG_STATIC           (0x1U<<29)
#define AI_BUFFER_FMT_FLAG_IS_IO            (0x1U<<27)
#define AI_BUFFER_FMT_FLAG_PERSISTENT       (0x1U<<29)


#define AI_BUFFER_FMT_PACK(value_, mask_, bits_) \
  ( ((value_) & (mask_)) << (bits_) )

#define AI_BUFFER_FMT_UNPACK(fmt_, mask_, bits_) \
  ( (AI_BUFFER_FMT_OBJ(fmt_) >> (bits_)) & (mask_) )

#define AI_BUFFER_FMT_OBJ(fmt_) \
  ((ai_buffer_format)(fmt_))

#define AI_BUFFER_FMT_GET_FLOAT(fmt_) \
  AI_BUFFER_FMT_UNPACK(fmt_, 0x1, 24)

#define AI_BUFFER_FMT_GET_SIGN(fmt_) \
  AI_BUFFER_FMT_UNPACK(fmt_, 0x1, 23)

#define AI_BUFFER_FMT_GET_TYPE(fmt_) \
  AI_BUFFER_FMT_UNPACK(fmt_, 0xF, 17)

#define AI_BUFFER_FMT_GET_BITS(fmt_) \
  AI_BUFFER_FMT_UNPACK(fmt_, 0x7F, 7)

#define AI_BUFFER_FMT_SET_BITS(bits_) \
  AI_BUFFER_FMT_PACK((bits_), 0x7F, 7)

#define AI_BUFFER_FMT_GET_FBITS(fmt_) \
  ( (ai_i8)AI_BUFFER_FMT_UNPACK(fmt_, 0x7F, 0) - 64 )

#define AI_BUFFER_FMT_SET_FBITS(fbits_) \
  AI_BUFFER_FMT_PACK((fbits_)+64, 0x7F, 0)

#define AI_BUFFER_FMT_SET(type_id_, sign_bit_, float_bit_, bits_, fbits_) \
  AI_BUFFER_FMT_OBJ( \
    AI_BUFFER_FMT_PACK(0, 0x1, 24) | \
    AI_BUFFER_FMT_PACK(sign_bit_, 0x1, 23) | \
    AI_BUFFER_FMT_PACK(0, 0x3, 21) | \
    AI_BUFFER_FMT_PACK(type_id_, 0xF, 17) | \
    AI_BUFFER_FMT_PACK(0, 0x7, 14) | \
    AI_BUFFER_FMT_SET_BITS(bits_) | \
    AI_BUFFER_FMT_SET_FBITS(fbits_) \
  )

#define AI_BUFFER_FMT_SET_COMPLEX(type_id_, sign_bit_, bits_, fbits_) \
  AI_BUFFER_FMT_OBJ( \
    AI_BUFFER_FMT_PACK(1, 0x1, 24) | \
    AI_BUFFER_FMT_PACK(sign_bit_, 0x1, 23) | \
    AI_BUFFER_FMT_PACK(0, 0x3, 21) | \
    AI_BUFFER_FMT_PACK(type_id_, 0xF, 17) | \
    AI_BUFFER_FMT_PACK(0, 0x7, 14) | \
    AI_BUFFER_FMT_SET_BITS(bits_) | \
    AI_BUFFER_FMT_SET_FBITS(fbits_) \
  )

#define AI_BUFFER_FMT_SAME(fmt1_, fmt2_) \
  ( AI_BUFFER_FMT_GET(fmt1_) == AI_BUFFER_FMT_GET(fmt2_) )

#define AI_BUFFER_FMT_GET(fmt_) \
  (AI_BUFFER_FMT_OBJ(fmt_) & AI_BUFFER_FMT_MASK)

#define AI_BUFFER_FORMAT(buf_) \
  AI_BUFFER_FMT_GET((buf_)->format)


/*!
 * @define shape type index
 * @ingroup ai_platform
 * @brief positional ID for generic shapes C structs
 */
#define AI_SHAPE_EXTENSION      (0x5)
#define AI_SHAPE_DEPTH          (0x4)
#define AI_SHAPE_HEIGHT         (0x3)
#define AI_SHAPE_WIDTH          (0x2)
#define AI_SHAPE_CHANNEL        (0x1)
#define AI_SHAPE_IN_CHANNEL     (0x0)
#define AI_SHAPE_BATCH          (0x0)
#define AI_SHAPE_TIME           (0x0)


AI_DEPRECATED
#define AI_BUFFER_WIDTH(buf_) \
  ((buf_)->shape.data[AI_SHAPE_WIDTH])

AI_DEPRECATED
#define AI_BUFFER_HEIGHT(buf_) \
  ((buf_)->shape.data[AI_SHAPE_HEIGHT])

AI_DEPRECATED
#define AI_BUFFER_CHANNELS(buf_) \
  ((buf_)->shape.data[AI_SHAPE_CHANNEL])

AI_DEPRECATED
#define AI_BUFFER_N_BATCHES(buf_) \
  ((buf_)->shape.data[AI_SHAPE_BATCH])

#define AI_BUFFER_DATA(buf_, type_) \
  ((type_*)((buf_)->data))

#define AI_BUFFER_META_INFO(buf_) \
  ((buf_)->meta_info)

#define AI_BUFFER_META_INFO_INTQ(meta_) \
  ((meta_) && ((meta_)->flags & AI_BUFFER_META_HAS_INTQ_INFO)) \
    ? ((meta_)->intq_info) : NULL

#define AI_BUFFER_META_INFO_INTQ_GET_SIZE(meta_) \
  ( (AI_BUFFER_META_INFO_INTQ(meta_)) \
    ? AI_INTQ_INFO_LIST_SIZE(AI_BUFFER_META_INFO_INTQ(meta_)) \
    : 0 )

#define AI_BUFFER_META_INFO_INTQ_GET_SCALE(meta_, pos_) \
  ( (AI_BUFFER_META_INFO_INTQ(meta_)) \
    ? AI_INTQ_INFO_LIST_SCALE(AI_BUFFER_META_INFO_INTQ(meta_), ai_float, pos_) \
    : 0 )

#define AI_BUFFER_META_INFO_INTQ_GET_ZEROPOINT(meta_, pos_) \
  ( (AI_BUFFER_META_INFO_INTQ(meta_)) \
    ? ((AI_INTQ_INFO_LIST_FLAGS(AI_BUFFER_META_INFO_INTQ(meta_))&AI_BUFFER_META_FLAG_ZEROPOINT_U8) \
      ? AI_INTQ_INFO_LIST_ZEROPOINT(AI_BUFFER_META_INFO_INTQ(meta_), ai_u8, pos_) \
      : AI_INTQ_INFO_LIST_ZEROPOINT(AI_BUFFER_META_INFO_INTQ(meta_), ai_i8, pos_) ) \
    : 0 )

#define AI_BUFFER_META_INFO_INIT(flags_, intq_info_) { \
  .flags = (flags_), \
  .intq_info = AI_PACK(intq_info_) \
}

#define AI_BUFFER_SIZE(buf_) \
  ai_buffer_get_size(buf_, true)

#define AI_BUFFER_SIZE_UNPAD(buf_) \
  ai_buffer_get_size(buf_, false)

#define AI_BUFFER_BYTE_SIZE(count_, fmt_) \
  ai_buffer_get_byte_size(count_, fmt_)

#define AI_BUFFER_FLAGS(buf_) \
  ((buf_) ? (buf_)->flags : 0x0)

#define AI_BUFFER_SHAPE_INIT(type_, size_, ...) \
{ \
  .type = (type_), \
  .size  = (size_), \
  .data  = (ai_shape_dimension[]){ __VA_ARGS__ } \
}

#define AI_BUFFER_SHAPE_INIT_FROM_ARRAY(type_, size_, array_ptr_) \
{ \
  .type = (type_), \
  .size  = (size_), \
  .data  = (ai_shape_dimension*)(array_ptr_) \
}

#define AI_BUFFER_SHAPE_SIZE(buf_) \
  ((buf_) ? (buf_)->shape.size : 0)

#define AI_BUFFER_SHAPE_TYPE(buf_) \
  ((buf_) ? (buf_)->shape.type : 0)

#if defined(HAS_AI_ASSERT) && defined(AI_ASSERT)

#define AI_BUFFER_SET_SHAPE_ELEM(buf_, pos_, value_) { \
  AI_ASSERT(buf_) \
  (buf_)->shape.data[pos_] = (value_); \
}

#define AI_BUFFER_SHAPE_ELEM(buf_, pos_) \
  (((pos_)<AI_BUFFER_SHAPE_SIZE(buf_)) ? (buf_)->shape.data[pos_] : 0)

#else

#define AI_BUFFER_SET_SHAPE_ELEM(buf_, pos_, value_) { \
  (buf_)->shape.data[pos_] = (value_); \
}

#define AI_BUFFER_SHAPE_ELEM(buf_, pos_) \
  (buf_)->shape.data[pos_]

#endif


AI_DEPRECATED
#define AI_BUFFER_OBJ_INIT(format_, h_, w_, ch_, n_batches_, data_) \
{ .format = (ai_buffer_format)(format_), \
  .data = (ai_handle)(data_), \
  .meta_info = NULL, \
  .flags = AI_FLAG_NONE, \
  .size = (h_) * (w_) * AI_PAD_CHANNELS(format_, ch_), \
  .shape = AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, (n_batches_), (ch_), (w_), (h_)), \
}

/* 7.1 new macro API */
#define AI_BUFFER_INIT(flags_, format_, shape_, size_, meta_info_, data_) \
{ .format = (ai_buffer_format)(format_), \
  .data = (ai_handle)(data_), \
  .meta_info = (meta_info_), \
  .flags = (flags_), \
  .size = (size_), \
  .shape = AI_PACK(shape_) \
}

/* 7.1 new macro API */
#define AI_BUFFER_INIT_STATIC(type_, flags_, format_, shape_, size_, meta_info_, ...) \
{ .format = (ai_buffer_format)(format_), \
  .data = (ai_handle)((type_[]){__VA_ARGS__}), \
  .meta_info = (meta_info_), \
  .flags = (flags_), \
  .size = (size_), \
  .shape = AI_PACK(shape_) \
}

/*****************************************************************************/
#define AI_NETWORK_BUFFERS_FIELD_DECLARE \
  ai_signature      map_signature;      /*! structure signature (required!) */ \
  ai_buffer_array   map_weights;        /*! info about weights array buffers (required!) */ \
  ai_buffer_array   map_activations;    /*! info about activations array buffers (required!) */

#define AI_NETWORK_PARAMS_FIELDS_DECLARE \
union { \
  struct { \
    ai_buffer         params;         /*! info about params buffer(required!) */ \
    ai_buffer         activations;    /*! info about activations buffer (required!) */ \
  }; \
  struct { \
    AI_NETWORK_BUFFERS_FIELD_DECLARE \
  }; \
};

/*****************************************************************************/
#define AI_BUFFER_ARRAY_OBJ_INIT(flags_, size_, buffer_array_) \
{ \
  .flags = (ai_u16)(flags_), \
  .size = (ai_u16)(size_), \
  .buffer = (ai_buffer*)(buffer_array_) \
}

#define AI_BUFFER_ARRAY_OBJ_INIT_STATIC(flags_, size_, ...) \
{ \
  .flags = (ai_u16)(flags_), \
  .size = (ai_u16)(size_), \
  .buffer = (ai_buffer*)((ai_buffer[]){__VA_ARGS__}) \
}

#define AI_BUFFER_ARRAY_SANE(buf_array_) \
  ai_buffer_array_sane(buf_array_)

#define AI_BUFFER_ARRAY_FLAGS(buf_array_) \
  ((AI_BUFFER_ARRAY_SANE(buf_array_)) ? (buf_array_)->flags : AI_FLAG_NONE)

#define AI_BUFFER_ARRAY_SIZE(buf_array_) \
  ((AI_BUFFER_ARRAY_SANE(buf_array_)) ? (buf_array_)->size : 0)

#define AI_BUFFER_ARRAY_ITEM(buf_array_, pos_) \
  ((AI_BUFFER_ARRAY_SANE(buf_array_)) ? ((buf_array_)->buffer + (pos_)) : NULL)

#define AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(buf_array_, pos_, address_) \
  ai_buffer_array_item_set_address(buf_array_, pos_, address_)


/*!
 * @enum buffer formats enum list
 * @ingroup ai_platform
 *
 * List of supported ai_buffer format types.
 */
enum {
  AI_BUFFER_FORMAT_NONE     = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_NONE, 0, 0,  0, 0),
  AI_BUFFER_FORMAT_FLOAT    = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_FLOAT, 1, 0, 32, 0),

  AI_BUFFER_FORMAT_U1       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  1, 0),
  AI_BUFFER_FORMAT_U8       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  8, 0),
  AI_BUFFER_FORMAT_U16      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0, 16, 0),
  AI_BUFFER_FORMAT_U32      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0, 32, 0),

  AI_BUFFER_FORMAT_S1       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  1, 0),
  AI_BUFFER_FORMAT_S8       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  8, 0),
  AI_BUFFER_FORMAT_S16      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0, 16, 0),
  AI_BUFFER_FORMAT_S32      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0, 32, 0),

  AI_BUFFER_FORMAT_Q        = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  0, 0),
  AI_BUFFER_FORMAT_Q7       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  8, 7),
  AI_BUFFER_FORMAT_Q15      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0, 16, 15),

  AI_BUFFER_FORMAT_UQ       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  0, 0),
  AI_BUFFER_FORMAT_UQ7      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  8, 7),
  AI_BUFFER_FORMAT_UQ15     = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0, 16, 15),

  AI_BUFFER_FORMAT_BOOL     = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_BOOL, 0, 0, 8, 0),
};

/*****************************************************************************/
#define AI_ERROR_INIT(type_, code_)   { \
  .type = AI_ERROR_##type_, \
  .code = AI_ERROR_CODE_##code_ \
}

/* printf formats */
#define SSIZET_FMT  "%" PRIu32
#define AII32_FMT   "%" PRId32
#define AIU32_FMT   "%" PRIu32
#define AII64_FMT   "%" PRId64
#define AIU64_FMT   "%" PRIu64


#define AI_VERSION(major_, minor_, micro_) \
  (((major_)<<24) | ((minor_)<<16) | ((micro_)<<8))


typedef uint8_t       ai_custom_type_signature;

typedef void*         ai_handle;
typedef const void*   ai_handle_const;

typedef float         ai_float;
typedef double        ai_double;

typedef bool          ai_bool;

typedef char          ai_char;

typedef uint32_t      ai_size;
typedef int16_t       ai_short_size;

typedef uintptr_t     ai_uptr;

typedef unsigned int  ai_uint;
typedef uint8_t       ai_u8;
typedef uint16_t      ai_u16;
typedef uint32_t      ai_u32;
typedef uint64_t      ai_u64;

typedef int           ai_int;
typedef int8_t        ai_i8;
typedef int16_t       ai_i16;
typedef int32_t       ai_i32;
typedef int64_t       ai_i64;

typedef uint64_t      ai_macc;

typedef int32_t       ai_pbits;

typedef uint32_t      ai_signature;

typedef void (*ai_handle_func)(ai_handle);

/*****************************************************************************/
/*!
 * @struct ai_error
 * @ingroup ai_platform
 * @brief Structure encoding details about the last error.
 */
typedef struct ai_error_ {
  ai_u32   type : 8;    /*!< Error type represented by @ref ai_error_type */
  ai_u32   code : 24;   /*!< Error code represented by @ref ai_error_code */
} ai_error;

/*****************************************************************************/
/*!
 * @struct ai_intq_info
 * @ingroup ai_platform
 * @brief an element of the ai_intq_info_list entry. It reports an array for the
 * scale and zeropoint values for each buffer. Optional flags are also present
 */
typedef struct ai_intq_info_ {
  INTQ_CONST ai_float*  scale;
  INTQ_CONST ai_handle  zeropoint;
} ai_intq_info;

/*!
 * @struct ai_intq_info_list
 * @ingroup ai_platform
 * @brief list reporting meta info for quantized networks integer support
 * when size > 1 it means a per channel out quantization
 */
typedef struct ai_intq_info_list_ {
  ai_u16          flags;  /*!< optional flags to store intq info attributes */
  ai_u16          size;   /*!< number of elements in the the intq_info list  */
  INTQ_CONST ai_intq_info* info;  /*!< pointer to an array of metainfo
                                   * associated to the intq_info list */
} ai_intq_info_list;

/*****************************************************************************/
/*!
 * @struct ai_buffer_meta_info
 * @ingroup ai_platform
 * @brief Optional meta attributes associated with the I/O buffer.
 * This datastruct is used also for network querying, where the data field may
 * may be NULL.
 */
typedef struct ai_buffer_meta_info_ {
  ai_u32                  flags;      /*!< meta info flags */
  ai_intq_info_list*      intq_info;  /*!< meta info related to integer format */
} ai_buffer_meta_info;

/*!
 * @struct ai_buffer_shape
 * @ingroup ai_platform
 * @brief Memory buffer shape datatype definition.
 */
typedef struct ai_buffer_shape_ {
  ai_u32                type  : 8;          /*!< shape type: reserved for compatibility */
  ai_u32                size  : 24;         /*!< size: shape cardinality */
  ai_shape_dimension*   data;               /*!< pointer to shape tuple array */
} ai_buffer_shape;

/*!
 * @struct ai_buffer
 * @ingroup ai_platform
 * @brief Memory buffer storing data (optional) with a shape, size and type.
 * This datastruct is used also for network querying, where the data field may
 * may be NULL.
 */
typedef struct ai_buffer_ {
  ai_buffer_format        format;     /*!< buffer format */
  ai_handle               data;       /*!< pointer to buffer data */
  ai_buffer_meta_info*    meta_info;  /*!< pointer to buffer metadata info */
  /* New 7.1 fields */
  ai_flags                flags;      /*!< shape optional flags */
  ai_size                 size;       /*!< number of elements of the buffer (including optional padding) */
  ai_buffer_shape         shape;      /*!< n-dimensional shape info */
} ai_buffer;

/*!
 * @struct ai_buffer_array
 * @ingroup ai_platform
 * @brief Array of @ref ai_buffer.
 */
typedef struct ai_buffer_array_ {
  ai_u16                  flags;      /*!< buffer array flags */
  ai_u16                  size;       /*!< buffer array size */
  ai_buffer*              buffer;     /*!< buffer array buffers pointer */
} ai_buffer_array;

/* enums section */

/*!
 * @enum ai_error_type
 * @ingroup ai_platform
 *
 * Generic enum to list network error types.
 */
typedef enum {
  AI_ERROR_NONE                         = 0x00,     /*!< No error */
  AI_ERROR_TOOL_PLATFORM_API_MISMATCH   = 0x01,
  AI_ERROR_TYPES_MISMATCH               = 0x02,
  AI_ERROR_INVALID_HANDLE               = 0x10,
  AI_ERROR_INVALID_STATE                = 0x11,
  AI_ERROR_INVALID_INPUT                = 0x12,
  AI_ERROR_INVALID_OUTPUT               = 0x13,
  AI_ERROR_INVALID_PARAM                = 0x14,
  AI_ERROR_INVALID_SIGNATURE            = 0x15,
  AI_ERROR_INVALID_SIZE                 = 0x16,
  AI_ERROR_INVALID_VALUE                = 0x17,
  AI_ERROR_INIT_FAILED                  = 0x30,
  AI_ERROR_ALLOCATION_FAILED            = 0x31,
  AI_ERROR_DEALLOCATION_FAILED          = 0x32,
  AI_ERROR_CREATE_FAILED                = 0x33,
} ai_error_type;

/*!
 * @enum ai_error_code
 * @ingroup ai_platform
 *
 * Generic enum to list network error codes.
 */
typedef enum {
  AI_ERROR_CODE_NONE                = 0x0000,    /*!< No error */
  AI_ERROR_CODE_NETWORK             = 0x0010,
  AI_ERROR_CODE_NETWORK_PARAMS      = 0x0011,
  AI_ERROR_CODE_NETWORK_WEIGHTS     = 0x0012,
  AI_ERROR_CODE_NETWORK_ACTIVATIONS = 0x0013,
  AI_ERROR_CODE_LAYER               = 0x0014,
  AI_ERROR_CODE_TENSOR              = 0x0015,
  AI_ERROR_CODE_ARRAY               = 0x0016,
  AI_ERROR_CODE_INVALID_PTR         = 0x0017,
  AI_ERROR_CODE_INVALID_SIZE        = 0x0018,
  AI_ERROR_CODE_INVALID_FORMAT      = 0x0019,
  AI_ERROR_CODE_OUT_OF_RANGE        = 0x0020,
  AI_ERROR_CODE_INVALID_BATCH       = 0x0021,
  AI_ERROR_CODE_MISSED_INIT         = 0x0030,
  AI_ERROR_CODE_IN_USE              = 0x0040,
  AI_ERROR_CODE_LOCK                = 0x0041,
} ai_error_code;

/*!
 * @struct ai_platform_version
 * @ingroup ai_platform
 * @brief Datastruct storing platform version info
 */
typedef struct ai_platform_version_ {
  ai_u8               major;
  ai_u8               minor;
  ai_u8               micro;
  ai_u8               reserved;
} ai_platform_version;

/*!
 * @struct ai_network_params
 * @ingroup ai_platform
 *
 * Datastructure to pass parameters during network initialization.
 */
typedef struct ai_network_params_ {
  AI_NETWORK_PARAMS_FIELDS_DECLARE
} ai_network_params;

/*!
 * @struct ai_network_buffers
 * @ingroup ai_platform
 *
 * Datastructure to pass network buffers during network initialization.
 */
typedef struct ai_network_buffers_ {
  AI_NETWORK_BUFFERS_FIELD_DECLARE
} ai_network_buffers;

/*!
 * @struct ai_network_report
 * @ingroup ai_platform
 *
 * Datastructure to query a network report with some relevant network detail.
 */
typedef struct ai_network_report_ {
  const char*                     model_name;
  const char*                     model_signature;
  const char*                     model_datetime;

  const char*                     compile_datetime;

  const char*                     runtime_revision;
  ai_platform_version             runtime_version;

  const char*                     tool_revision;
  ai_platform_version             tool_version;
  ai_platform_version             tool_api_version;

  ai_platform_version             api_version;
  ai_platform_version             interface_api_version;

  ai_macc                         n_macc;

  ai_u16                          n_inputs;
  ai_u16                          n_outputs;
  ai_buffer*                      inputs;
  ai_buffer*                      outputs;

  AI_NETWORK_PARAMS_FIELDS_DECLARE

  ai_u32                          n_nodes;

  ai_signature                    signature;
} ai_network_report;

/*!
 * @enum ai_scatter_nd_reduction
 * @ingroup ai_platform
 * @brief reduction operation in scatter_nd layer
 */
typedef enum {
  AI_SCATTER_ND_NONE = 0x0,
  AI_SCATTER_ND_OPERATION
} ai_scatter_nd_reduction;

/*!
 * @enum ai_upsample_mode
 * @ingroup ai_platform
 * @brief allowed mode in upsample layer
 */
typedef enum {
  AI_UPSAMPLE_ZEROS = 0x0,
  AI_UPSAMPLE_NEAREST,
  AI_UPSAMPLE_BILINEAR,
  AI_UPSAMPLE_TRILINEAR
} ai_upsample_mode;

/*!
 * @enum ai_resize_mode
 * @ingroup ai_platform
 * @brief allowed mode in resize layer
 */
typedef enum {
  AI_RESIZE_ZEROS = 0x0,
  AI_RESIZE_NEAREST,
  AI_RESIZE_LINEAR,
  AI_RESIZE_CUBIC
} ai_resize_mode;

/*!
 * @enum ai_coord_transf_mode
 * @ingroup ai_platform
 * @brief coordinate_transformation_mode in resize layer
 */
typedef enum {
  AI_HALF_PIXEL = 0x0,
  AI_PYTORCH_HALF_PIXEL,
  AI_ALIGN_CORNERS,
  AI_ASYMMETRIC,
  AI_TF_HALF_PIXEL_FOR_NN,
  AI_TF_CROP_AND_RESIZE
} ai_coord_transf_mode;

typedef enum {
  AI_ROUND_PREFER_FLOOR = 0x0,
  AI_ROUND_PREFER_CEIL,
  AI_ROUND_FLOOR,
  AI_ROUND_CEIL
} ai_nearest_mode;

typedef enum {
  AI_PAD_CONSTANT = 0x0,
  AI_PAD_REFLECT,
  AI_PAD_EDGE,
  AI_PAD_8BIT_CH1ST_CONSTANT,
} ai_pad_mode;

#define OUTPUT_PADDING_FLAG (1 << 0)
#define CHANNEL_FIRST_FLAG  (1 << 1)
/* Padding pattern supported:         */
/* 0 = (1, 1, 1,1),  1 = (0, 0, 2, 2) */
#define CHANNEL_PADDING_PATTERN  (1 << 2)
/* Carefull when changing those definitions
   bit0 shall always select output padding (Valid vs Same)
   bit1 shall always select Channel first /channel lst format
   bit2 shall always select padding pattern (1, 1, 1, 1) (stride1) or (0, 0, 2, 2) (stride2)
*/
typedef enum {
  AI_LAYER_FORMAT_CHANNEL_LAST_VALID  = 0x0,
  AI_LAYER_FORMAT_CHANNEL_LAST_SAME  = 0x1,
  AI_LAYER_FORMAT_CHANNEL_FIRST_VALID = 0x2,
  AI_LAYER_FORMAT_CHANNEL_FIRST_SAME = 0x3,
  AI_LAYER_FORMAT_CHANNEL_FIRST_SAME2 = 0x7,
} ai_layer_format_type;

/*! ai_platform public APIs **************************************************/

/*!
 * @brief get the total number of elements of an ai_buffer.
 * @ingroup ai_platform
 * @param buffer a pointer to an @ref ai_buffer
 * @param with_padding when true it considers also padded elements
 * @return the number of elements of the buffer (with/without padded ones)
 */
AI_API_ENTRY
ai_size ai_buffer_get_size(const ai_buffer* buffer, const ai_bool with_padding);

/*!
 * @brief get the size in bytes of an ai_buffer (given the number of elements and format).
 * @ingroup ai_platform
 * @param count the number of elements composing the buffer
 * @param fmt the format of the ai_buffer
 * @return the size in bytes of the buffer
 */
AI_API_ENTRY
ai_size ai_buffer_get_byte_size(const ai_size count, const ai_buffer_format fmt);

/*!
 * @brief get total size in bytes of a buffer array.
 * @ingroup ai_platform
 * @param barray a pointer to the buffer array
 * @return the total size in bytes of all the buffer arrays
 */
AI_API_ENTRY
ai_bool ai_buffer_array_is_empty(const ai_buffer_array* barray);

/*!
 * @brief get total size in bytes of a buffer array.
 * @ingroup ai_platform
 * @param barray a pointer to the buffer array
 * @return the total size in bytes of all the buffer arrays
 */
AI_API_ENTRY
ai_bool ai_buffer_array_is_valid(const ai_buffer_array* barray);

/*!
 * @brief check if a buffer array is valid - i.e. not empty.
 * @ingroup ai_platform
 * @param barray a pointer to the buffer array
 * @return true if the array is consistent and not empty, false otherwise
 */
AI_API_ENTRY
ai_bool ai_buffer_array_sane(const ai_buffer_array* barray);

/*!
 * @brief get total size in bytes of a buffer array.
 * @ingroup ai_platform
 * @param barray a pointer to the buffer array
 * @return the total size in bytes of all the buffer arrays
 */
AI_API_ENTRY
ai_size ai_buffer_array_get_byte_size(const ai_buffer_array* barray);

/*!
 * @brief set the address of buffer array item @pos
 * @ingroup ai_platform
 * @param barray a pointer to the buffer array
 * @param pos the index of the element in the array
 * @param address the address to set
 * @return true if successful, false otherwise
 */
AI_API_ENTRY
ai_bool ai_buffer_array_item_set_address(
  ai_buffer_array* barray, const ai_u32 pos, ai_handle address);

AI_API_DECLARE_END

#endif /*AI_PLATFORM_H*/
