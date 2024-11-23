/**
  ******************************************************************************
  * @file    stai.h
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
#ifndef STAI_H
#define STAI_H

#include <stdint.h>
#include <stddef.h>

#define STAI_API_VERSION_MAJOR        (1)
#define STAI_API_VERSION_MINOR        (0)
#define STAI_API_VERSION_MICRO        (0)

#define STAI_TOOLS_VERSION_MAJOR      (2)
#define STAI_TOOLS_VERSION_MINOR      (0)
#define STAI_TOOLS_VERSION_MICRO      (0)

/*****************************************************************************/
#ifdef __cplusplus
#define STAI_API_DECLARE_BEGIN    extern "C" {
#define STAI_API_DECLARE_END      }
#else
#include <stdbool.h>
#define STAI_API_DECLARE_BEGIN    /* STAI_API_DECLARE_BEGIN */
#define STAI_API_DECLARE_END      /* STAI_API_DECLARE_END   */
#endif

/*****************************************************************************/
#define STAI_PACK(...) \
  __VA_ARGS__

#define STAI_UNUSED(x) \
  (void)(x);

#if defined(_MSC_VER)
  #define STAI_API_ENTRY          __declspec(dllexport)
  #define STAI_ALIGNED(x)         __declspec(align(x))
#elif defined(__ICCARM__) || defined (__IAR_SYSTEMS_ICC__)
  #define STAI_API_ENTRY          /* STAI_API_ENTRY */
  #define STAI_ALIGNED(x)         _STAI_ALIGNED_X(x)
  #define _STAI_ALIGNED_XY(x, y)  x ## y
  #define _STAI_ALIGNED_X(x)      _STAI_ALIGNED_XY(_STAI_ALIGNED_,x)
  #define _STAI_ALIGNED_1         _Pragma("data_alignment = 1")
  #define _STAI_ALIGNED_2         _Pragma("data_alignment = 2")
  #define _STAI_ALIGNED_4         _Pragma("data_alignment = 4")
  #define _STAI_ALIGNED_8         _Pragma("data_alignment = 8")
  #define _STAI_ALIGNED_16        _Pragma("data_alignment = 16")
  #define _STAI_ALIGNED_32        _Pragma("data_alignment = 32")
  #define _STAI_ALIGNED_64        _Pragma("data_alignment = 64")
#elif defined(__CC_ARM)
  #define STAI_API_ENTRY          __attribute__((visibility("default")))
  #define STAI_ALIGNED(x)         __attribute__((aligned (x)))
  /* Keil disallows anonymous union initialization by default */
  #pragma anon_unions
#elif defined(__GNUC__)
  // #define STAI_API_ENTRY          __attribute__((visibility("default")))
  #define STAI_API_ENTRY          /* STAI_API_ENTRY */
  #define STAI_ALIGNED(x)         __attribute__((aligned(x)))
#else
  /* Dynamic libraries are not supported by the compiler */
  #define STAI_API_ENTRY                /* STAI_API_ENTRY */
  #define STAI_ALIGNED(x)               /* STAI_ALIGNED(x) */
#endif


/*****************************************************************************/
/*** Formats field getters/setters Macros Section and family enums         ***/

typedef enum {
  STAI_FORMAT_TYPE_NONE           = 0x00,
  STAI_FORMAT_TYPE_FLOAT          = 0x01,
  STAI_FORMAT_TYPE_Q              = 0x02,
  STAI_FORMAT_TYPE_BOOL           = 0x03,
} stai_format_type;

/*  Get format type */
#define STAI_FORMAT_GET_TYPE(format) \
  ((stai_format_type)(((format)>>17) & 0xF))

/*  Get format sign : 0 or 1 allowed */
#define STAI_FORMAT_GET_SIGN(format) \
  (((format)>>23) & 0x1)

/*  Get format bits - including sign, fractional bits. padding bits are not included */
#define STAI_FORMAT_GET_BITS(format) \
  (((format)>>7) & 0x7F)

/*  Get format integer bits, i.e. integer bits are bits minus sign minus fractional bits */
#define STAI_FORMAT_GET_IBITS(format) \
  (STAI_FORMAT_GET_BITS(format) - STAI_FORMAT_GET_FBITS(format) - STAI_FORMAT_GET_SIGN(format))

/*  Get format fractional bits */
#define STAI_FORMAT_GET_FBITS(format) \
  ((((format)>>0) & 0x7F) - 64)


/*****************************************************************************/
#define STAI_NETWORK_CONTEXT_DECLARE(_context_name, _context_size) \
  STAI_ALIGNED(8) \
  stai_network _context_name[_context_size] = {0};


/*****************************************************************************/
/**  I/O FLAGS Section  **/
#define STAI_FLAG_NONE                            (0x0)

/* buffer is preallocated by system */
#define STAI_FLAG_PREALLOCATED                    (0x1 << 0)

/* buffer address field `buffer_descriptor::address` may not be used by the user and functions `get_buffer_address()`,
   `set_buffer_address()` must be used instead */
#define STAI_FLAG_INDIRECT                        (0x1 << 1)

/* buffer may be replaced by user */
#define STAI_FLAG_REPLACEABLE                     (0x1 << 2)

/* buffer content may be overridden by network */
#define STAI_FLAG_OVERRIDE                        (0x1 << 3)

#define STAI_FLAG_CHANNEL_FIRST                   (0x1 << 6)
#define STAI_FLAG_CHANNEL_LAST                    (0x1 << 7)
#define STAI_FLAG_HAS_BATCH                       (0x1 << 8)

/**  Network FLAGS Section  **/
#define STAI_FLAG_ACTIVATIONS                     (0x1 << 26)
#define STAI_FLAG_INPUTS                          (0x1 << 27)
#define STAI_FLAG_OUTPUTS                         (0x1 << 28)
#define STAI_FLAG_STATES                          (0x1 << 29)
#define STAI_FLAG_WEIGHTS                         (0x1 << 30)


/*****************************************************************************/
STAI_API_DECLARE_BEGIN

/*  STAI Enums Section  */
/* return codes >= STAI_ERROR_GENERIC are errors.
   all codes < STAI_ERROR_GENERIC are available for managing extended set of implementation specific return codes
   APIs track only 1 error: the 1st one generated while calling the APIs. No multiple error handling considered.
   NOTE: need also to provide proper handling of a new return code in stai_get_return_code_name() implementation.
*/
typedef enum {
  STAI_SUCCESS                                   = 0x000000,
  STAI_RUNNING_NO_WFE                            = 0x000010,
  STAI_RUNNING_WFE                               = 0x000011,
  STAI_DONE                                      = 0x000012,
  STAI_ERROR_GENERIC                             = 0x020000,
  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS       = 0x020001,
  STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE      = 0x030000,
  STAI_ERROR_NETWORK_INVALID_CONTEXT_SIZE        = 0x030001,
  STAI_ERROR_NETWORK_INVALID_CONTEXT_ALIGNMENT   = 0x030002,
  STAI_ERROR_NETWORK_INVALID_INFO                = 0x030010,
  STAI_ERROR_NETWORK_INVALID_RUN                 = 0x030020,
  STAI_ERROR_NETWORK_INVALID_RUNTIME             = 0x030030,
  STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR     = 0x040000,
  STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_NUM     = 0x040001,
  STAI_ERROR_NETWORK_INVALID_IN_PTR              = 0x040010,
  STAI_ERROR_NETWORK_INVALID_IN_NUM              = 0x040011,
  STAI_ERROR_NETWORK_INVALID_OUT_PTR             = 0x040020,
  STAI_ERROR_NETWORK_INVALID_OUT_NUM             = 0x040021,
  STAI_ERROR_NETWORK_INVALID_STATES_PTR          = 0x040030,
  STAI_ERROR_NETWORK_INVALID_STATES_NUM          = 0x040031,
  STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR         = 0x040040,
  STAI_ERROR_NETWORK_INVALID_WEIGHTS_NUM         = 0x040041,
  STAI_ERROR_NETWORK_INVALID_CALLBACK            = 0x040050,
  STAI_ERROR_NOT_IMPLEMENTED                     = 0x040060,
  STAI_ERROR_INVALID_BUFFER_ALIGNMENT            = 0x040070,

  /* asynchronous execution errors */
  STAI_ERROR_NETWORK_STILL_RUNNING               = 0x050000,

  /* platform (de-)init errors */
  STAI_ERROR_STAI_INIT_FAILED                    = 0x060000,
  STAI_ERROR_STAI_DEINIT_FAILED                  = 0x060001,
} stai_return_code;


/*!
 * @enum buffer formats enum list
 * @ingroup ai_platform
 *
 * List of supported ai_buffer format types.
 */
typedef enum {
  STAI_FORMAT_NONE              = 0x00000040,

  STAI_FORMAT_FLOAT32           = 0x00821040,
  STAI_FORMAT_FLOAT64           = 0x00822040,

  STAI_FORMAT_U1                = 0x000400c0,
  STAI_FORMAT_U8                = 0x00040440,
  STAI_FORMAT_U16               = 0x00040840,
  STAI_FORMAT_U32               = 0x00041040,
  STAI_FORMAT_U64               = 0x00042040,

  STAI_FORMAT_S1                = 0x008400c0,
  STAI_FORMAT_S8                = 0x00840440,
  STAI_FORMAT_S16               = 0x00840840,
  STAI_FORMAT_S32               = 0x00841040,
  STAI_FORMAT_S64               = 0x00842040,

  STAI_FORMAT_Q                 = 0x00840040,
  STAI_FORMAT_Q7                = 0x00840447,
  STAI_FORMAT_Q15               = 0x0084084f,

  STAI_FORMAT_UQ                = 0x00040040,
  STAI_FORMAT_UQ7               = 0x00040447,
  STAI_FORMAT_UQ15              = 0x0004084f,

  STAI_FORMAT_BOOL              = 0x00060440,
} stai_format;


typedef enum {
  STAI_MODE_SYNC                 = 0x01,
  STAI_MODE_ASYNC                = 0x02,
} stai_run_mode;


/*****************************************************************************/

/* 32bit mask to code flags about e.g. I/O buffers or network properties */
typedef int32_t           stai_flags;


/* Opaque network handler exposed to application */
/* it is defined as int8 to allow static memory allocation of context on app side
   as an opaque uint8_t byte buffer to ensure alignment to 8 bytes */
typedef uint8_t           stai_network;

/* Generic byte pointer for byte offset addressing */
typedef uint8_t*          stai_ptr;

/* Generic type size */
typedef uint32_t          stai_size;

/* Packed bits arrays are padded to 32bits */
typedef int32_t           stai_pbits;


/*****************************************************************************/
typedef struct {
  stai_size         size;
  stai_ptr const *  data;
} stai_array_const_ptr;

typedef struct {
  stai_size         size;
  stai_ptr*         data;
} stai_array_ptr;


typedef struct {
  stai_size         size;
  const float*      data;
} stai_array_f32;


typedef struct {
  stai_size         size;
  const int8_t*     data;
} stai_array_s8;


typedef struct {
  stai_size         size;
  const uint8_t*    data;
} stai_array_u8;


typedef struct {
  stai_size         size;
  const int16_t*    data;
} stai_array_s16;


typedef struct {
  stai_size         size;
  const uint16_t*   data;
} stai_array_u16;


typedef struct {
  stai_size         size;
  const int32_t*    data;
} stai_array_s32;


typedef struct {
  stai_size         size;
  const uint32_t*   data;
} stai_array_u32;


typedef struct {
  uint8_t           major;
  uint8_t           minor;
  uint8_t           micro;
  uint8_t           reserved;
} stai_version;


typedef stai_array_s32 stai_shape;


typedef struct {
  stai_size            size;
  uintptr_t            address;
  stai_flags           flags;
  // stai_format          format;
} stai_buffer;


typedef struct {
  stai_size            size_bytes;
  stai_flags           flags;
  stai_format          format;
  stai_shape           shape;
  stai_array_f32       scale;
  stai_array_s16       zeropoint;
  const char*          name;
} stai_tensor;


/*****************************************************************************/
typedef struct {
  /* Original model signature */
  const char*             model_signature;

  /* C generated model info */
  const char*             c_compile_datetime;
  const char*             c_model_name;
  const char*             c_model_datetime;
  uint32_t                c_model_signature;

  stai_version            runtime_version;
  stai_version            tool_version;
  stai_version            api_version;

  /* C model macc operations */
  uint64_t                n_macc;

  /* C model graph nodes */
  uint32_t                n_nodes;

  /* C model flags */
  int32_t                 flags;

  /* Number of c model network I/O, Activations, Weights, States */
  uint16_t                n_inputs;
  uint16_t                n_outputs;
  uint16_t                n_activations;
  uint16_t                n_weights;
  uint16_t                n_states;

  /* I/O networks tensors c_model info (as arrays of size n_inputs/n)outputs */
  const stai_tensor*      inputs;
  const stai_tensor*      outputs;

  /* Activations, weights, states info */
  const stai_tensor*      activations;
  const stai_tensor*      weights;
  const stai_tensor*      states;
} stai_network_info;


/*****************************************************************************/
/* C struct used in file network_details.h to provide info about a node      */
typedef struct {
  const int32_t id;
  const int16_t type;
  const stai_array_s32 input_tensors;
  const stai_array_s32 output_tensors;
} stai_node_details;


/*****************************************************************************/
/* C struct used in file network_details.h to provide info about the network */
typedef struct {
  const stai_tensor * const tensors;
  const stai_node_details * const nodes;
  const uint32_t n_nodes;
} stai_network_details;


/* C enum to list supported compilers */
typedef enum {
  STAI_COMPILER_ID_NONE           = 0x00,
  STAI_COMPILER_ID_GCC            = 0x01,
  STAI_COMPILER_ID_GHS            = 0x10,
  STAI_COMPILER_ID_HIGHTECH       = 0x20,
  STAI_COMPILER_ID_IAR            = 0x30,
  STAI_COMPILER_ID_KEIL           = 0x40,
  STAI_COMPILER_ID_KEIL_AC6       = 0x50,
} stai_compiler_id;


/* C struct to provide info about ST.AI C runtime */
typedef struct {
  stai_version            api_version;      /* X.Y.Z version of the ST Edge AI APIs */
  stai_version            runtime_version;  /* X.Y.Z version of the runtime */
  stai_version            tools_version;    /* version of the tool compatible with the run-time */
  uint32_t                runtime_build;    /* 32bit run-time identifier  (i.e. build info) */
  stai_compiler_id        compiler_id;      /* compiler ID enum */
  const char*             compiler_desc;    /* string with a short description of the compiler */
} stai_runtime_info;


/*****************************************************************************/
/**  STAI event callback and datatypes definition                           **/

#define STAI_EVENT_NONE                     (0x00)

typedef int32_t stai_event_type;

/**
 * This is the generic network event callback signature
 * @param cb_cookie is the opaque handler provided by caller (i.e. the application)
 * @param event_type is the unique event type identifier
 * @param event is the the opaque handler to event payload struct (Optional, may be NULL)
 **/
typedef void (*stai_event_cb)(
  void* cb_cookie,
  const stai_event_type event_type,
  const void* event_payload);


/*****************************************************************************/
/**  ST.AI Runtime APIs section                                             **/
/**  Initialize ST Edge AI runtime **/
STAI_API_ENTRY
stai_return_code stai_runtime_init(void);

/**  De-initialize ST Edge AI runtime **/
STAI_API_ENTRY
stai_return_code stai_runtime_deinit(void);

/**  Get ST Edge AI runtime info (versions, build, compiler, etc.) **/
STAI_API_ENTRY
stai_return_code stai_runtime_get_info(stai_runtime_info* info);

/** Set ST Edge AI runtime callback **/
STAI_API_ENTRY
stai_return_code stai_runtime_set_callback(const stai_event_cb cb, void *cb_cookie);

/*****************************************************************************/
/**  Helpers routines  **/
/**  Helper API to generate a stai_format enum with provided info  **/
STAI_API_ENTRY
stai_format stai_init_format(
  const stai_format_type type, const int8_t sign, const int32_t bits, const int32_t fbits);

STAI_API_DECLARE_END

#endif    /* STAI_H */
