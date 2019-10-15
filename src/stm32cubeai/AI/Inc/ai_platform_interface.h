/**
  ******************************************************************************
  * @file    ai_platform_interface.h
  * @author  AST Embedded Analytics Research Platform
  * @date    02-Aug-2018
  * @brief   Definitions of AI platform interface APIs types
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __AI_PLATFORM_INTERFACE_H__
#define __AI_PLATFORM_INTERFACE_H__
#pragma once

#include "ai_platform.h"

#include "datatypes_network.h"
#include "ai_datatypes_format.h"

/*!
 * @defgroup datatypes_interface Interface Datatypes
 * @brief Data structures and defines used to implement neural networks
 */

#define AI_PLATFORM_INTERFACE_API_MAJOR    1
#define AI_PLATFORM_INTERFACE_API_MINOR    3
#define AI_PLATFORM_INTERFACE_API_MICRO    0

/******************************************************************************/
#define AI_ERROR_TRAP(net_, type_, code_) \
          ai_platform_network_set_error((net_), AI_CONCAT(AI_ERROR_,type_), \
                            AI_CONCAT(AI_ERROR_CODE_,code_))

/******************************************************************************/
#define AI_PTR(ptr_)                     ((ai_ptr)(ptr_))
#define AI_PTR_CONST(ptr_)               ((ai_ptr_const)(ptr_))

/******************************************************************************/
#define AI_SHAPE_2D_INIT(h, w) \
  { .dimension={ (w), (h) } }

#define AI_SHAPE_ND_INIT(size_, ...) \
  { .size      = (ai_size)(size_), \
    .dimension = (ai_shape_dimension[]){ __VA_ARGS__ } }



#define AI_SHAPE_INIT(h, w, ch, in_ch)  {.dimension={ (in_ch), (ch), (w), (h) }}

/******************************************************************************/
#define AI_STRIDE_2D_INIT(h, w) \
  { .dimension={ (w), (h) } }

#define AI_STRIDE_ND_INIT(size_, ...) \
  { .size      = (ai_size)(size_), \
    .dimension = (ai_stride_dimension[]){ __VA_ARGS__ } }

#define AI_STRIDE_INIT(h, w, ch, in_ch) \
  { .dimension={ (in_ch), (ch), (w), (h) } }

/******************************************************************************/
#define AI_KLASS_OBJ(obj) \
  ((ai_klass_obj)(obj))

/*! generic handlers section **************************************************/
#define AI_OBJ_DATA(obj_, type_) \
  ((type_)(obj_)->data)

/*! ai_buffer handlers section ************************************************/
#define AI_BUFFER_OBJ(ptr) \
  ((ai_buffer*)(ptr))
  
/*! ai_array handlers section *************************************************/
#define AI_ARRAY_OBJ(ptr) \
  ((ai_array*)(ptr))

#define AI_ARRAY_OBJ_FMT(array_) \
  ((ai_array_format)(AI_ARRAY_OBJ(array_)->format))

#define AI_ARRAY_OBJ_SIZE(array_) \
  (AI_ARRAY_OBJ(array_)->size)

#define AI_ARRAY_OBJ_BYTE_SIZE(array_) \
  AI_ARRAY_GET_BYTE_SIZE(AI_ARRAY_OBJ(array_)->format, \
                         AI_ARRAY_OBJ(array_)->size)

#define AI_ARRAY_OBJ_DATA_SIZE(array_) \
  AI_ARRAY_GET_DATA_BYTE_SIZE(AI_ARRAY_OBJ(array_)->format, \
                              AI_ARRAY_OBJ(array_)->size)

#define AI_ARRAY_OBJ_DATA(array_, type_) \
  ((type_*)(AI_ARRAY_OBJ(array_)->data))

#define AI_ARRAY_OBJ_DATA_START(array_, type_) \
  ((type_*)(AI_ARRAY_OBJ(array_)->data_start))


#define AI_ARRAY_OBJ_ELEM(array_, type_, pos_) \
  AI_ARRAY_OBJ_DATA(array_, type_)[(pos_)]

#define AI_ARRAY_OBJ_INIT_STATIC(type_, format_, size_, ...) { \
  .format = AI_FMT_OBJ(format_), \
  .size = (ai_array_size)(size_), \
  .data = (ai_ptr)((type_[]){ __VA_ARGS__ }), \
  .data_start = (ai_ptr)((type_[]){ __VA_ARGS__ }), \
}

#define AI_ARRAY_OBJ_DECLARE_STATIC(name_, type_, format_, attr_, size_, ...) \
  AI_ALIGNED(4) \
  attr_ ai_array name_ = AI_ARRAY_OBJ_INIT_STATIC(type_, format_, size_, __VA_ARGS__);

#define AI_ARRAY_OBJ_INIT(format_, data_, data_start_, size_) { \
  .format = AI_FMT_OBJ(format_), \
  .size = (ai_array_size)(size_), \
  .data = AI_PTR(data_), \
  .data_start = AI_PTR(data_start_) }

#define AI_ARRAY_OBJ_DECLARE(name_, format_, data_, data_start_, size_, attr_) \
  AI_ALIGNED(4) \
  attr_ ai_array name_ = AI_ARRAY_OBJ_INIT(format_, data_, data_start_, size_);


/******************************************************************************/
#define AI_TENSOR_OBJ(ptr) \
  ((struct ai_tensor_*)(ptr))

#define AI_TENSOR_OBJ_INIT(shape_, stride_, data_array_ptr_) { \
  .data   = AI_ARRAY_OBJ(data_array_ptr_), \
  .shape  = shape_, \
  .stride = stride_, \
}

#define AI_TENSOR_OBJ_DECLARE(name_, shape_, stride_, \
                              data_array_ptr_, attr_) \
  AI_ALIGNED(4) \
  attr_ ai_tensor name_ = AI_TENSOR_OBJ_INIT(AI_PACK(shape_), AI_PACK(stride_), \
                            data_array_ptr_);

/********************************* TENSOR STATE MACROS  ***********************/
#define AI_TENSOR_STATE_OBJ_INIT(end_ptr_ , curr_ptr_, stride_, size_) \
  { (end_ptr_), (curr_ptr_), (stride_), (size_) }

/********************************* TENSOR LIST MACROS  ************************/
#define AI_TENSOR_LIST_EMPTY \
  { .tensor = (ai_tensor*[]) { NULL }, .info = NULL, \
    .size = 0, .flags = AI_FLAG_NONE }

#define AI_TENSOR_LIST_ENTRY(...) \
  { .tensor = (ai_tensor*[]) { __VA_ARGS__ }, .info = NULL, \
    .size = AI_NUMARGS(__VA_ARGS__), .flags = AI_FLAG_NONE }

#define AI_TENSOR_LIST_OBJ_DECLARE(name_, attr_, ...) \
  AI_ALIGNED(4) \
  attr_ ai_tensor_list name_ = AI_TENSOR_LIST_ENTRY(__VA_ARGS__);

/********************************* TENSOR LIST I/O MACROS  ********************/
#define AI_TENSOR_LIST_IO_ENTRY(flags_, size_, ...) \
  { .tensor = (ai_tensor*[]) { __VA_ARGS__ }, \
    .info = (ai_tensor_info[1]) { { \
              .buffer = (ai_buffer[size_])AI_STRUCT_INIT, \
              .state = (ai_tensor_state[size_])AI_STRUCT_INIT \
             } }, \
    .size = (size_), \
    .flags = (flags_) }

/********************************* TENSOR CHAIN MACROS  ***********************/
#define AI_TENSOR_CHAIN_OBJ_INIT(flags_, in_, out_, weights_, scratch_) \
  { .chain = (ai_tensor_list[]){ in_, out_, weights_, scratch_ }, \
    .size = 4, .flags = (flags_) }

#define AI_TENSOR_CHAIN_OBJ_DECLARE(name_, attr_, in_, out_, weights_, scratch_) \
  AI_ALIGNED(4) \
  attr_ ai_tensor_chain name_ = \
    AI_TENSOR_CHAIN_OBJ_INIT(AI_FLAG_NONE, AI_PACK(in_), AI_PACK(out_), \
                             AI_PACK(weights_), AI_PACK(scratch_));

/********************************* TENSOR CHAIN I/O MACROS  *******************/
#define AI_TENSOR_CHAIN_IO_OBJ_INIT(flags_, in_tensor_list_, out_tensor_list_) \
  { .chain = (ai_tensor_list[]){ in_tensor_list_, out_tensor_list_ }, \
    .size = 2, .flags = (flags_) }

#define AI_TENSOR_CHAIN_IO_OBJ_DECLARE( \
    name_, attr_, flags_, in_tensor_list_, out_tensor_list_) \
  AI_ALIGNED(4) \
  attr_ ai_tensor_chain_io name_ = \
    AI_TENSOR_CHAIN_IO_OBJ_INIT(flags_, in_tensor_list_, out_tensor_list_);

/*******************************  NETWORK SECTION  ****************************/
#define AI_NETWORK_OBJ(ptr)                     ((ai_network*)(ptr))


#define AI_NETWORK_OBJ_INIT( \
      weights_buffer_, activations_buffer_, \
      in_tensor_list_ptr_, out_tensor_list_ptr_, \
      in_node_ptr_, signature_) { \
  .magic = 0x0, \
  .signature = signature_, \
  .flags = AI_FLAG_NONE, \
  .error = AI_ERROR_INIT(NONE, NONE), \
  .n_batches = 0, \
  .batch_id = 0, \
  .params = weights_buffer_, \
  .activations = activations_buffer_, \
  .tensors = AI_TENSOR_CHAIN_IO_OBJ_INIT(AI_FLAG_NONE, \
                                         AI_PACK(in_tensor_list_ptr_), \
                                         AI_PACK(out_tensor_list_ptr_)), \
  .input_node = AI_NODE_OBJ(in_node_ptr_), \
  .current_node = AI_NODE_OBJ(NULL), \
  .klass = AI_KLASS_OBJ(NULL) }

#define AI_NETWORK_OBJ_DECLARE( \
      var_name_, \
      weights_buffer_, activations_buffer_, \
      in_tensor_list_ptr_, out_tensor_list_ptr_, \
      in_node_ptr_, signature_) \
  AI_ALIGNED(4) \
  AI_STATIC ai_network var_name_ = AI_NETWORK_OBJ_INIT( \
  AI_PACK(weights_buffer_), \
  AI_PACK(activations_buffer_), \
  AI_PACK(in_tensor_list_ptr_), \
  AI_PACK(out_tensor_list_ptr_), \
  (in_node_ptr_), (signature_));

#define AI_NETWORK_ACQUIRE_CTX(handle) \
  AI_NETWORK_OBJ(ai_platform_context_acquire(handle))


/******************************************************************************/
AI_API_DECLARE_BEGIN

/*!
 * @typedef ai_klass_obj
 * @ingroup ai_platform_interface
 * @brief handler to (private) generic subclass derivatives implementation
 */
typedef void* ai_klass_obj;

/*!
 * @typedef ai_ptr
 * @ingroup ai_platform_interface
 * @brief Byte pointer data addressing
 */
typedef uint8_t* ai_ptr;

/*!
 * @typedef ai_ptr_const
 * @ingroup ai_platform_interface
 * @brief Constant byte pointer data addressing
 */
typedef const uint8_t* ai_ptr_const;

/*!
 * @typedef ai_ptr_offset
 * @ingroup ai_platform_interface
 * @brief byte offset for computing strides
 */
typedef int32_t ai_ptr_offset;

/*!
 * @typedef ai_flags
 * @ingroup ai_platform_interface
 * @brief bitmask for flags management
 */
typedef uint32_t ai_flags;

/*!
 * @typedef ai_magic
 * @ingroup ai_platform_interface
 * @brief magic field to mark internal datatstructures
 */
typedef uint32_t ai_magic;


#define AI_CONTEXT_FIELDS \
  ai_magic     magic;  /*!< magic word to mark valid contexts datastructs*/ \
  ai_signature signature; /*!< 32bit signature for network consistency checks */

#define AI_CONTEXT_OBJ(obj)         ((ai_context*)(obj))  

/*!
 * @typedef ai_context
 * @ingroup ai_platform_interface
 * @brief Abstract internal context header exposed to codegen interface  
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_context_ {
  AI_CONTEXT_FIELDS
} ai_context;
AI_PACKED_STRUCT_END

/*!
 * @enum ai_shape_2d_type
 * @ingroup ai_platform_interface
 * @brief Codes for the 2D tensor dimensions
 */
typedef enum {
  AI_SHAPE_2D_MAX_DIMENSION = 0x2,
  AI_SHAPE_2D_HEIGHT        = 0x1,
  AI_SHAPE_2D_WIDTH         = 0x0,
} ai_shape_2d_type;


/*!
 * @struct ai_shape_2d
 * @ingroup ai_platform_interface
 * @brief Dimensions for generic 2D tensors
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_shape_2d_s {
  ai_shape_dimension dimension[AI_SHAPE_2D_MAX_DIMENSION]; /*!< 2D tensor dimensions */
} ai_shape_2d;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_stride_2d
 * @ingroup ai_platform_interface
 * @brief Stride dimensions for generic 2D tensors (in number of elements)
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_stride_2d_s {
  ai_stride_dimension dimension[AI_SHAPE_2D_MAX_DIMENSION]; /*!< 2D tensor stride */
} ai_stride_2d;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_shape_nd
 * @ingroup ai_platform_interface
 * @brief Dimensions for generic N-dimensional tensors
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_shape_nd_s {
  ai_size             size;  /*!< number of elements in the n-dimensional shape
                                (NOT number of bytes!). */
  ai_shape_dimension* dimension;
} ai_shape_nd;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_stride_nd
 * @ingroup ai_platform_interface
 * @brief Stride dimensions for generic N-dimensional tensors
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_stride_nd_s {
  ai_size              size;  /*!< number of elements in the n-dimensional stride
                                  (NOT number of bytes!). */
  ai_stride_dimension* dimension;
} ai_stride_nd;
AI_PACKED_STRUCT_END

/*!
 * @enum ai_shape_type
 * @ingroup ai_platform_interface
 * @brief Codes for the 4D tensor dimensions
 */
typedef enum {
  AI_SHAPE_MAX_DIMENSION  = 0x4,
  AI_SHAPE_HEIGHT         = 0x3,
  AI_SHAPE_WIDTH          = 0x2,
  AI_SHAPE_CHANNEL        = 0x1,
  AI_SHAPE_IN_CHANNEL     = 0x0,
//  AI_SHAPE_BATCH_CHANNEL  = 0x5,
} ai_shape_type;

/*!
 * @struct ai_shape
 * @ingroup ai_platform_interface
 * @brief Dimensions for generic 4D tensors
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_shape_s {
  ai_shape_dimension dimension[AI_SHAPE_MAX_DIMENSION]; /*!< 4D tensor dimensions */
} ai_shape;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_stride
 * @ingroup ai_platform_interface
 * @brief Stride dimensions for generic 4D tensors (in number of elements)
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_stride_s {
  ai_stride_dimension dimension[AI_SHAPE_MAX_DIMENSION]; /*!< 4D tensor stride */
} ai_stride;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_array
 * @ingroup ai_platform_interface
 * @brief Generic flattened array with size
 * and (byte) stride of each item
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_array_s {
  ai_array_format format;      /*!< array format (see @ref ai_array_format) */
  ai_array_size size;          /*!< number of elements in the array (NOT number 
                                   of bytes!). The size of the array could be 
                                   determine using  @ref AI_ARRAY_GET_BYTE_SIZE
                                   macro */
  ai_ptr data;                 /*!< pointer to data */
  ai_ptr data_start;           /*!< pointer to parent's data start address */
} ai_array;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_tensor
 * @ingroup ai_platform_interface
 * @brief Generic tensor structure for storing parameters and activations
 *
 * The data is stored in a flattened array with an implicit order given by the
 * reverse order in @ref ai_shape_dimension:
 * in_channels, channels, width, height.
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_tensor_s {
  ai_array* data;   /*!< flattened array pointer to tensor data */
  ai_shape shape;   /*!< tensor shape see @ref ai_shape */
  ai_stride stride; /*!< tensor stride see @ref ai_stride */
} ai_tensor;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_tensor_state
 * @ingroup ai_platform_interface
 * @brief state context for tensor management (used for I/O network tensors)
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_tensor_state_s {
  ai_ptr                end_ptr;   /*!< end address of the I/O tensor data buffer */
  ai_ptr                curr_ptr;  /*!< current address of the I/O tensor data buffer (for batching) */
  ai_ptr_offset         stride;    /*!< single batch buffer size (in bytes) */
  ai_size               size;      /*!< total size in bytes of the I/O tensor buffer */
} ai_tensor_state;
AI_PACKED_STRUCT_END

/*!
 * @struct ai_tensor_info
 * @ingroup ai_platform_interface
 * @brief info metadata for tensor management (used for I/O network tensors)
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_tensor_info_s {
//  union {
    ai_tensor_state*      state;
    ai_buffer*            buffer;
//  };
} ai_tensor_info;
AI_PACKED_STRUCT_END

/********************************* TENSOR CHAINS DATATYPES  *******************/
/*!
 * @enum ai_tensor_chain_type
 * @ingroup ai_platform_interface
 * @brief Enum for the different tensor chains supported in the library
 */
typedef enum {
  AI_TENSOR_CHAIN_INPUT       = 0x0,
  AI_TENSOR_CHAIN_OUTPUT      = 0x1,
  AI_TENSOR_CHAIN_WEIGHTS     = 0x2,
  AI_TENSOR_CHAIN_SCRATCH     = 0x3,
  AI_TENSOR_CHAIN_SIZE
} ai_tensor_chain_type;

/*!
 * @struct ai_tensor_list
 * @ingroup ai_platform_interface
 * @brief list (in form of arrays) of internal nodes tensor pointers
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_tensor_list_s {
  ai_tensor**      tensor; /*!< array of linked tensor pointer */
  ai_tensor_info*  info;   /*!< pointer to an array of metainfo associated to the tensors */
  ai_u16           size;   /*!< number of elements in the the tensor list */
  ai_u16           flags;  /*!< optional flags to store tensor list attributes */
} ai_tensor_list;
AI_PACKED_STRUCT_END


/*!
 * @struct ai_tensor_chain
 * @ingroup ai_platform_interface
 * @brief tensor chain datastruct for internal network nodes
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_tensor_chain_s {
  ai_tensor_list* chain; /*!< pointer to a 4 sized array see @ref ai_tensor_chain_type */
  ai_u16          size;
  ai_u16          flags;
} ai_tensor_chain;
AI_PACKED_STRUCT_END

/* forward function */
struct ai_node_s;

/*!
 * @struct ai_network
 * @ingroup layers
 * @brief Structure encoding a sequential neural network
 */
AI_PACKED_STRUCT_START
typedef AI_ALIGNED_TYPE(struct, 4) AI_PACKED ai_network_s {
  AI_CONTEXT_FIELDS
  ai_flags   flags; /*!< bitflags mask to track some network state info */
  ai_error   error; /*!< track 1st error code in the network */

  ai_u16     n_batches;     /*!< number of batches to process */
  ai_u16     batch_id;      /*!< current batch to to process btw [0, n_batches)*/
  ai_buffer  params;        /*!< params buffer data */
  ai_buffer  activations;   /*!< activations buffer data */

  ai_tensor_chain tensors;     /*!< I/O tensor chain list see @ref ai_tensor_list */

  struct ai_node_s* input_node; /*!< first node to execute */
  struct ai_node_s* current_node;  /*!< current node to execute */

  ai_klass_obj klass; /*!< opaque handler to specific network implementations */
} ai_network;
AI_PACKED_STRUCT_END

/*!
 * @brief Get platform runtime lib revision version as string.
 * @ingroup ai_platform_interface
 * @return a string containing the revision of the runtime library
 */
AI_INTERFACE_TYPE
const char* ai_platform_runtime_get_revision(void);

/*!
 * @brief Get platform runtime lib version as datastruct.
 * @ingroup ai_platform_interface
 * @return a datastruct containing the version of the runtime library
 */
AI_INTERFACE_TYPE
ai_platform_version ai_platform_runtime_get_version(void);

/*!
 * @brief Get platform public APIs version as datastruct.
 * @ingroup ai_platform_interface
 * @return a datastruct containing the version of the public APIs
 */
AI_INTERFACE_TYPE
ai_platform_version ai_platform_api_get_version(void);

/*!
 * @brief Get platform interface private  APIs version as datastruct.
 * @ingroup ai_platform_interface
 * @return a datastruct containing the version of the interface private APIs
 */
AI_INTERFACE_TYPE
ai_platform_version ai_platform_interface_api_get_version(void);

/*!
 * @brief Get platform context.
 * @ingroup ai_platform_interface
 * @return a valid context handle or NULL otherwise
 */
AI_INTERFACE_TYPE
ai_context* ai_platform_context_acquire(const ai_handle handle);

/*!
 * @brief Release platform context.
 * @ingroup ai_platform_interface
 * @return an opaque handle to the released object 
 */
AI_INTERFACE_TYPE
ai_handle ai_platform_context_release(ai_context* ctx);


/*!
 * @brief get **first** error tracked when using the network
 * @ingroup ai_platform_interface
 * @param network an opaque handler to the network context
 * @return ai_error the FIRST error generated during network processing 
 */
AI_INTERFACE_TYPE
ai_error ai_platform_network_get_error(ai_handle network);


/*!
 * @brief Set specific error code of the network. if an error is already present
 * keep it  
 * @ingroup ai_platform_interface
 * @param net_ctx a pointer to the network context
 * @param type error type as defined in @ref ai_error_type
 * @param code error code as defined in @ref ai_error_code
 * @return true if no previous errors where recorded, false if a previous error
 * is present or context is invalid
 */
AI_INTERFACE_TYPE
ai_bool ai_platform_network_set_error(
  ai_network* net_ctx, const ai_error_type type, const ai_error_code code);

/*!
 * @brief Finalize network report datastruct with I/O buffer infos
 * @ingroup ai_platform_interface
 * @return bool if the report has been finalized correctly. false otherwise
 */
AI_INTERFACE_TYPE
ai_bool ai_platform_api_get_network_report(
  ai_handle network, ai_network_report* r);

/*!
 * @brief create a network context with some error check
 * @ingroup ai_platform_interface
 * @param a pointer to an opaque handle of the network context
 * @param an (optional) pointer to the network config buffer info
 * @param net_ctx a pointer to the network context structure to initialize
 * @param tools_major major version id of the tool used to generate the network
 * @param tools_minor minor version id of the tool used to generate the network
 * @param tools_micro micro version id of the tool used to generate the network
 * @return the error during network creation or error none if ok  
 */
AI_INTERFACE_TYPE
ai_error ai_platform_network_create(
  ai_handle* network, const ai_buffer* network_config,
  ai_network* net_ctx,
  const ai_u8 tools_major, const ai_u8 tools_minor, const ai_u8 tools_micro);

/*!
 * @brief destroy a network context
 * @ingroup ai_platform_interface
 * @param network a pointer to an opaque handle of the network context
 * @return AI_HANDLE_NULL if deallocation OK, same network handle if failed 
 */
AI_INTERFACE_TYPE
ai_handle ai_platform_network_destroy(ai_handle network);

/*!
 * @brief initialize the network context
 * @ingroup ai_platform_interface
 * @param network a pointer to an opaque handle of the network context
 * @return a valid network context, NULL if initialization failed 
 */
AI_INTERFACE_TYPE
ai_network* ai_platform_network_init(
  ai_handle network, const ai_network_params* params);

/*!
 * @brief main platform runtime execute of a network
 * @ingroup ai_platform_interface
 * @param network an opaque handler to the network context
 * @param input a pointer to the input buffer data to process
 * @param output a pointer to the output buffer
 * @return the number of batches processed from the input. A result <=0 in case 
 * of error
 */
AI_INTERFACE_TYPE
ai_i32 ai_platform_network_process(
  ai_handle network, const ai_buffer* input, ai_buffer* output);

AI_API_DECLARE_END

#endif /*__AI_PLATFORM_INTERFACE_H__*/
