/**
  ******************************************************************************
  * @file    ai_platform.h
  * @author  AST Embedded Analytics Research Platform
  * @date    01-May-2017
  * @brief   Definitions of AI platform public APIs types
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

#ifndef __AI_PLATFORM_H__
#define __AI_PLATFORM_H__
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define AI_PLATFORM_API_MAJOR           1
#define AI_PLATFORM_API_MINOR           1
#define AI_PLATFORM_API_MICRO           0

/******************************************************************************/
#ifdef __cplusplus
#define AI_API_DECLARE_BEGIN extern "C" {
#define AI_API_DECLARE_END }
#else
#define AI_API_DECLARE_BEGIN    /* AI_API_DECLARE_BEGIN */
#define AI_API_DECLARE_END      /* AI_API_DECLARE_END */
#endif

/******************************************************************************/
#define AI_CONCAT_ARG(a, b)     a ## b
#define AI_CONCAT(a, b)         AI_CONCAT_ARG(a, b)

/******************************************************************************/
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
#elif defined(__CC_ARM)
  #define AI_API_ENTRY          __attribute__((visibility("default")))
  #define AI_ALIGNED(x)         __attribute__((aligned (x)))
  /* Keil disallows anonymous union initialization by default */
  #pragma anon_unions
#elif defined(__GNUC__)
  #define AI_API_ENTRY          __attribute__((visibility("default")))
  #define AI_ALIGNED(x)         __attribute__((aligned(x)))
#else
  /* Dynamic libraries are not supported by the compiler */
  #define AI_API_ENTRY          /* AI_API_ENTRY */
  #define AI_ALIGNED(x)         /* AI_ALIGNED(x) */
#endif

#define AI_HANDLE_PTR(ptr_)             ((ai_handle)(ptr_))
#define AI_HANDLE_NULL                  AI_HANDLE_PTR(0)

#define AI_HANDLE_FUNC_PTR(func)  ((ai_handle_func)(func))

#define AI_UNUSED(x)    (void)(x);

#define AI_DEPRECATED   /* AI_DEPRECATED */

#define AI_LEGACY       /* AI_LEGACY */

#ifndef __GNUC__
  #define AI_STRUCT_INIT  {0}
#else
#define AI_STRUCT_INIT  {}
#endif

#define AI_ERROR_FMT   AIU32_FMT

#define AI_IS_UNSIGNED(type) \
  ((((type)0) - 1) > 0)

#define AI_CUSTOM_SIZE(type) \
  (ai_custom_type_signature)((AI_IS_UNSIGNED(type)) \
    ? (0x80|(sizeof(type)&0x7f)) : (sizeof(type)&0x7f))

#define AI_NETWORK_PARAMS_INIT(params_, activations_) { \
  .params = params_, \
  .activations = activations_ }

/*! ai_buffer format handlers *************************************************/

/*!
 * @enum buffer format definition
 * @ingroup ai_platform
 *
 * 32 bit signed format list.
 */
typedef int32_t ai_buffer_format;

#define AI_BUFFER_FMT_TYPE_NONE          (0x0)
#define AI_BUFFER_FMT_TYPE_FLOAT         (0x1)
#define AI_BUFFER_FMT_TYPE_Q             (0x2)

#define AI_BUFFER_FMT_FLAG_CONST         (0x1U<<30)
#define AI_BUFFER_FMT_FLAG_STATIC        (0x1U<<29)
#define AI_BUFFER_FMT_FLAG_IS_IO         (0x1U<<27)

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
    AI_BUFFER_FMT_PACK(float_bit_, 0x1, 24) | \
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
  (AI_BUFFER_FMT_OBJ(fmt_) & 0x01FFFFFF)

#define AI_BUFFER_FORMAT(buf_) \
  AI_BUFFER_FMT_GET((buf_)->format)
#define AI_BUFFER_WIDTH(buf_) \
  ((buf_)->width)
#define AI_BUFFER_HEIGHT(buf_) \
  ((buf_)->height)
#define AI_BUFFER_CHANNELS(buf_) \
  ((buf_)->channels)
#define AI_BUFFER_N_BATCHES(buf_) \
  ((buf_)->n_batches)
#define AI_BUFFER_DATA(buf_, type_) \
  ((type_*)((buf_)->data))

#define AI_BUFFER_SIZE(buf_) \
  (((buf_)->width) * ((buf_)->height) * ((buf_)->channels))

#define AI_BUFFER_BYTE_SIZE(count_, fmt_) \
  ( (((count_) * AI_BUFFER_FMT_GET_BITS(fmt_))+4) >> 3 )


#define AI_BUFFER_OBJ_INIT(format_, h_, w_, ch_, n_batches_, data_) \
{ .format = (ai_buffer_format)(format_), \
  .n_batches = (n_batches_), \
  .height = (h_), \
  .width = (w_), \
  .channels = (ch_), \
  .data = (ai_handle)(data_) \
}

#define AI_BUFFER_OBJ_INIT_STATIC(type_, format_, h_, w_, ch_, n_batches_, ...) \
{ .format = (ai_buffer_format)(format_), \
  .n_batches = (n_batches_), \
  .height = (h_), \
  .width = (w_), \
  .channels = (ch_), \
  .data = (ai_handle)((type_[(h_)*(w_)*(ch_)*(n_batches_)]){__VA_ARGS__}) \
}

/*!
 * @enum buffer formats enum list
 * @ingroup ai_platform
 *
 * List of supported ai_buffer format types.
 */
enum {
  AI_BUFFER_FORMAT_NONE     = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_NONE, 0, 0,  0, 0),
  AI_BUFFER_FORMAT_FLOAT    = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_FLOAT, 1, 1, 32, 0),

  AI_BUFFER_FORMAT_U8       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  8, 0),
  AI_BUFFER_FORMAT_U16      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0, 16, 0),
  AI_BUFFER_FORMAT_S8       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  8, 0),
  AI_BUFFER_FORMAT_S16      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0, 16, 0),

  AI_BUFFER_FORMAT_Q        = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  0, 0),
  AI_BUFFER_FORMAT_Q7       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0,  8, 7),
  AI_BUFFER_FORMAT_Q15      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 1, 0, 16, 15),

  AI_BUFFER_FORMAT_UQ       = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  0, 0),
  AI_BUFFER_FORMAT_UQ7      = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0,  8, 7),
  AI_BUFFER_FORMAT_UQ15     = AI_BUFFER_FMT_SET(AI_BUFFER_FMT_TYPE_Q, 0, 0, 16, 15),
};

/******************************************************************************/
#define AI_ERROR_INIT(type_, code_)   { \
            .type = AI_ERROR_##type_, \
            .code = AI_ERROR_CODE_##code_ }

/* printf formats */
#ifdef REISC
  #define SSIZET_FMT  "%lu"
  #define AII32_FMT   "%ld"
  #define AIU32_FMT   "%lu"
#else  /* REISC */
  #define SSIZET_FMT  "%u"
  #define AII32_FMT   "%d"
  #define AIU32_FMT   "%u"
#endif /* REISC */

typedef uint8_t ai_custom_type_signature;

typedef void* ai_handle;

typedef void (*ai_handle_func)(void*);

typedef float ai_float;
typedef double ai_double;

typedef bool ai_bool;

typedef uint32_t ai_size;

typedef uintptr_t ai_uptr;

typedef unsigned int ai_uint;
typedef uint8_t  ai_u8;
typedef uint16_t ai_u16;
typedef uint32_t ai_u32;
typedef uint64_t ai_u64;

typedef int     ai_int;
typedef int8_t  ai_i8;
typedef int16_t ai_i16;
typedef int32_t ai_i32;
typedef int64_t ai_i64;

typedef uint32_t    ai_signature;

/*!
 * @struct ai_error
 * @ingroup ai_platform
 * @brief Structure encoding details about the last error.
 */
typedef struct ai_error_ {
  ai_u32   type : 8;    /*!< Error type represented by @ref ai_error_type */
  ai_u32   code : 24;   /*!< Error code represented by @ref ai_error_code */
} ai_error;

/*!
 * @struct ai_buffer
 * @ingroup ai_platform
 * @brief Memory buffer storing data (optional) with a shape, size and type.
 * This datastruct is used also for network querying, where the data field may
 * may be NULL.
 */
typedef struct ai_buffer_ {
  ai_buffer_format        format;     /*!< buffer format */
  ai_u16                  n_batches;  /*!< number of batches in the buffer */
  ai_u16                  height;     /*!< buffer height dimension */
  ai_u16                  width;      /*!< buffer width dimension */
  ai_u32                  channels;   /*!< buffer number of channels */
  ai_handle               data;       /*!< pointer to buffer data */
} ai_buffer;

/* enums section */

/*!
 * @enum ai_error_type
 * @ingroup ai_platform
 *
 * Generic enum to list network error types.
 */
typedef enum {
  AI_ERROR_NONE                   = 0x00,     /*!< No error */
  AI_ERROR_TOOL_PLATFORM_MISMATCH = 0x01,
  AI_ERROR_TYPES_MISMATCH         = 0x02,
  AI_ERROR_INVALID_HANDLE         = 0x10,
  AI_ERROR_INVALID_STATE          = 0x11,
  AI_ERROR_INVALID_INPUT          = 0x12,
  AI_ERROR_INVALID_OUTPUT         = 0x13,
  AI_ERROR_INVALID_PARAM          = 0x14,
  AI_ERROR_INVALID_SIGNATURE      = 0x15,
  AI_ERROR_INIT_FAILED            = 0x30,
  AI_ERROR_ALLOCATION_FAILED      = 0x31,
  AI_ERROR_DEALLOCATION_FAILED    = 0x32,
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
 * Datastructure to pass parameters to the network initialization.
 */
typedef struct ai_network_params_ {
  ai_buffer   params;         /*! info about params buffer(required!) */
  ai_buffer   activations;    /*! info about activations buffer (required!) */
} ai_network_params;

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

  ai_u32                          n_macc;

  ai_u16                          n_inputs;
  ai_u16                          n_outputs;
  ai_buffer*                      inputs;
  ai_buffer*                      outputs;

  ai_buffer                       activations;
  ai_buffer                       params;

  ai_u32                          n_nodes;

  ai_signature                    signature;
} ai_network_report;

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

typedef enum {
  AI_PAD_CONSTANT = 0x0,
  AI_PAD_REFLECT,
  AI_PAD_EDGE,
} ai_pad_mode;

#endif /*__AI_PLATFORM_H__*/
