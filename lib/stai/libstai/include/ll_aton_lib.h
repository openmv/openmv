/**
 ******************************************************************************
 * @file    ll_aton_lib.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON LL low level lib module.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __LL_ATON_LIB_H
#define __LL_ATON_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ll_aton.h"
#include "ll_aton_NN_interface.h"
#include "ll_aton_caches_interface.h"
#include "ll_aton_lib_sw_operators.h"

#ifndef _LL_LIB_DEBUG
#define _LL_LIB_DEBUG 1
#endif

#if _LL_LIB_DEBUG
  /**
   *  * @brief library error helper types, functions, and macros
   *   */
  enum __ll_lib_errors
  {
    _ERR_UNKNOWN = 0,
    _ERR_NINPUTS,
    _ERR_NOUTPUTS,
    _ERR_AXIS,
    _ERR_FRACTIONAL,
    _ERR_DATATYPE,
    _ERR_NBITS,
    _ERR_NBITS_IN,
    _ERR_NBITS_OUT,
    _ERR_SHAPE,
    _ERR_SHAPE_IN,
    _ERR_SHAPE_OUT,
    _ERR_BUFFER,
    _ERR_BUFFER_IN,
    _ERR_BUFFER_OUT,
    _ERR_RANK,
    _ERR_MODE,
  };

  /* Forward declaration for global function to be used only by `ll_lib` library */
  extern void __ll_lib_error(int err_code, int line, const char *func);

#define __LL_LIB_ERROR(_x, _y)                                                                                         \
  {                                                                                                                    \
    __ll_lib_error(_x, __LINE__, __FUNCTION__);                                                                        \
    return _y;                                                                                                         \
  }
#else // !_LL_LIB_DEBUG
#define __LL_LIB_ERROR(_x, _y) return _y
#endif // !_LL_LIB_DEBUG

  typedef union
  {
    float f;
    int32_t i;
  } __ll_aton_union_float_int_t;

#define __LL_PAD_FRAMING_DMA_MIN_BUFF_LEN 2700
#define __LL_PAD_FILLING_DMA_MIN_BUFF_LEN 1200

  // Uncomment beyond line to dump operation results for `Pad` operator
  // #define DUMP_RESULTS_PAD_OP

  /* Note: the beyond structures & macro must be the same (integer types aside) as the one in the ATON runtime, file
   * `ll_aton_lib_sw_operators.h` */
#define NR_OF_STREAM_ENG_LOOPS 4
  struct four_axes_item
  {
    uint32_t offset_in_bytes;
    uint32_t nr_of_loops;
  };

  struct four_axes
  {
    uint32_t initial_cumulative_start_offset_in_bytes;
    uint32_t inner_bytes_to_copy;
    uint32_t nr_of_stream_eng_loops;
    uint32_t total_bytes_to_copy;
    struct four_axes_item four_items_array[NR_OF_STREAM_ENG_LOOPS];
  };

  struct __ll_pad_sw_params; // forward declaration
  typedef struct __ll_pad_sw_params __ll_pad_sw_params_t;
  typedef int (*pad_callback_func_t)(__ll_pad_sw_params_t *, const NN_Instance_TypeDef *);

  struct __ll_pad_sw_params
  {
    int8_t *in_target;
    int8_t *in_end;
    int8_t *in_limit;
    int8_t *out_target;
    int8_t *out_end;
    int8_t *out_limit;
    uint32_t consecutive_axis;
    uint32_t consecutive_bytes;
    uint8_t nbytes; // 1, 2, 3, or 4 bytes
    uint8_t mode;   // where 0 == `constant`, 1 == `reflect`, 2 ==`edge`
    int8_t *saved_in_target;
    int8_t *saved_out_target;
    size_t tensor_rank;
    int8_t *end_out_target;
    struct four_axes negative_4loop;
    struct four_axes positive_4loop;
    pad_callback_func_t callback_function;
    int dma_in;
    int dma_out;
#if defined(DUMP_RESULTS_PAD_OP)
    size_t out_size; // in bytes
#endif

    /* Please add (not deep-copy) items above this line!!! */

    const uint32_t *min_shape; // must be first deep-copy item!!!
    const int32_t *pad_in_offsets_start;
    const int32_t *pad_in_offsets_end;
    const int32_t *pad_out_offsets_start;
    const int32_t *pad_out_offsets_end;
    const int32_t *out_shape;
    const int32_t *out_offsets;
    uint32_t *indexes;
  };

#if 0
/**
 *  * @brief tensor data type info structure
 *   */
/* Note: the data type values match ONNX TensorProto.DataType enum */
typedef enum
{
  TENSORINFO_DATATYPE_UNDEFINED = 0,
  TENSORINFO_DATATYPE_FLOAT = 1,
  TENSORINFO_DATATYPE_UINT8 = 2,
  TENSORINFO_DATATYPE_INT8 = 3,
  TENSORINFO_DATATYPE_UINT16 = 4,
  TENSORINFO_DATATYPE_INT16 = 5,
  TENSORINFO_DATATYPE_INT32 = 6,
  TENSORINFO_DATATYPE_INT64 = 7,
  TENSORINFO_DATATYPE_STRING = 8,
  TENSORINFO_DATATYPE_BOOL = 9,
  TENSORINFO_DATATYPE_FLOAT16 = 10,
  TENSORINFO_DATATYPE_DOUBLE = 11,
  TENSORINFO_DATATYPE_UINT32 = 12,
  TENSORINFO_DATATYPE_UINT64 = 13,
  TENSORINFO_DATATYPE_COMPLEX64 = 14,
  TENSORINFO_DATATYPE_COMPLEX128 = 15,
  TENSORINFO_DATATYPE_BFLOAT16 = 16,
  TENSORINFO_DATATYPE_QMN = 100, // ATONN specific
} LL_LIB_TensorInfo_DataType_TypeDef;

/**
 *  * @brief tensor info structure
 *   */
typedef struct
{
  ll_aton_pointer addr_base;
  int offset_start;
  int offset_end;
  int batches; // not sure we'll support this!!!
  int nchannels;
  int fwidth;
  int fheight;
  int ndims;
  int nbits;
  int Qm;
  int Qn;
  int Qunsigned;
  int dtype; // it's a LL_LIB_TensorInfo_DataType_TypeDef
} LL_LIB_TensorInfo_TypeDef;

typedef LL_Buffer_InfoTypeDef LL_LIB_TensorInfo_TypeDef;

#define _TDIM(x) ((x) != 0 ? (x) : 1)
#define __LL_LIB_TENSOR_ELEMENTS(t)                                                                                    \
  (_TDIM((t)->batches) * _TDIM((t)->fwidth) * _TDIM((t)->fheight) * _TDIM((t)->nchannels))
#else
/* ATON canonical positions */
#define TDIM_NKERNELS  0
#define TDIM_FHEIGHT   1
#define TDIM_FWIDTH    2
#define TDIM_NCHANNELS 3

#define _TDIM(x) ((x) != 0 ? (x) : 1)

typedef LL_Buffer_InfoTypeDef LL_LIB_TensorInfo_TypeDef;

#if 0
#define __LL_LIB_TENSOR_ELEMENTS(t)                                                                                    \
  (_TDIM((t)->shape[TDIM_NKERNELS]) * _TDIM((t)->shape[TDIM_FWIDTH]) * _TDIM((t)->shape[TDIM_FHEIGHT]) *               \
   _TDIM((t)->shape[TDIM_NCHANNELS]))
#else
int __LL_LIB_TENSOR_ELEMENTS(const LL_LIB_TensorInfo_TypeDef *t);
#endif

/* ONNX canonical positions */
#define TDIM_ONNX_NKERNELS  0
#define TDIM_ONNX_NCHANNELS 1
#define TDIM_ONNX_FHEIGHT   2
#define TDIM_ONNX_FWIDTH    3
#endif

  /**
   *  * @brief Heap typedefs
   *   */
  typedef struct
  {
    /* Generic fields common to all cases */
    LL_Streng_TensorInitTypeDef g_dma_in;
    LL_Streng_TensorInitTypeDef g_dma_out;

    /* Generic fields common at least for two cases */
    unsigned int g_idx;
    int g_size;

    unsigned char *g_dst_o_src;
    unsigned int g_offset_limit;
    unsigned int g_not_continuous; // 0 or 1 used by batched version of memcpy

    unsigned int g_num_tensors;
    const void *g_tensors;

    uint32_t g_wait_mask;

    /* Special field(s) for single cases */
    union
    {
      /* Concat_Case3 */
      struct
      {
        unsigned int outer_idx;
        unsigned int in_fheight;
        unsigned int nbytes;
        unsigned int out_line_size;
        unsigned char *in_curr;
      } concat_case3;
      /* Pad */
      __ll_pad_sw_params_t pad;
    } special;
  } __ll_lib_params_t;

  typedef union
  {
    uint32_t _alignment; // align `__ll_lib_params_t` to 4 bytes
    __ll_lib_params_t heap_params;
  } __ll_lib_params_align_t;

  typedef union
  {
    uint32_t _alignment; // align `LL_Buffer_InfoTypeDef` to 4 bytes
    LL_Buffer_InfoTypeDef buffer_info;
  } __ll_lib_buffer_align_t;

/* "Heap" dedicated for the implementation of HW-accelerated operators,
 * based on maximum number of tensors to be supported */
#define __LL_MAX_TENSORS 24

#define __LL_LOWER_HEAP_SIZE ((sizeof(__ll_lib_buffer_align_t) * __LL_MAX_TENSORS) / sizeof(uint32_t))     // in words
#define __LL_LIB_HEAP_SIZE   ((sizeof(__ll_lib_params_align_t) / sizeof(uint32_t)) + __LL_LOWER_HEAP_SIZE) // in words

#ifndef offsetof
#define offsetof(st, m) ((size_t) & (((st *)0)->m))
#endif

#define __LL_DMA_PAD_MAX_DIMS                                                                                          \
  ((__LL_LOWER_HEAP_SIZE * sizeof(uint32_t)) /                                                                         \
   (sizeof(__ll_pad_sw_params_t) - offsetof(__ll_pad_sw_params_t, min_shape)))

#define __LL_DMA_MIN_BUFF_LEN 40

  /**
   * @brief  performs a concat operation according to ONNX semantics
   * @param  inputs list of input tensor info structures
   * @param  ninputs of inputs
   * @param  output tensor info structure
   * @param  axis for concatenation
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Concat function
   *  * @{
   *   */
  int LL_ATON_LIB_Concat(const LL_Buffer_InfoTypeDef *inputs, unsigned int ninputs, const LL_Buffer_InfoTypeDef *output,
                         unsigned int axis, int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs a tensor ImageToRow transfer operation using stream engines `dma_in` and `dma_out`
   * @param  list of input tensor info structures
   * @param  number of inputs
   * @param  output tensor info structures
   * @param  blocksize_h vertical dimension for the blocksize
   * @param  blocksize_w horizontal dimension for the blocksize
   * @param  stride_h vertical stride for the sliding window
   * @param  stride_w horizontal stride for the sliding window
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   *
   * @note   Supports only input and output tensors in ATON canonical format
   *
   * @note   Bit-sizes are rounded up to multiples of 8-bits
   *
   */
  /** @defgroup LL_ATON_LIB_DMA_ImageToRow function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_ImageToRow(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                                 unsigned int blocksize_w, unsigned int stride_h, unsigned int stride_w, int dma_in,
                                 int dma_out, const NN_Instance_TypeDef *nn_instance);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs a tensor SpaceToDepth transfer operation using stream engines `dma_in` and `dma_out`
   * @param  list of input tensor info structures
   * @param  number of inputs
   * @param  output tensor info structures
   * @param  blocksize_h vertical dimension for the blocksize
   * @param  blocksize_w horizontal dimension for the blocksize
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_in DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   *
   * @note   Bit-sizes are rounded up to multiples of 8-bits
   *
   */
  /** @defgroup LL_ATON_LIB_DMA_SpaceToDepth function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_SpaceToDepth(const LL_LIB_TensorInfo_TypeDef *list, unsigned int number,
                                   const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                                   unsigned int blocksize_w, int dma_in, int dma_out,
                                   const NN_Instance_TypeDef *nn_instance);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a tensor RowToImage transfer operation using stream engines `dma_in` and `dma_out`
   * @param  list of input tensor info structures
   * @param  number of inputs
   * @param  output tensor info structures
   * @param  blocksize_h vertical dimension for the blocksize
   * @param  blocksize_w horizontal dimension for the blocksize
   * @param  stride_h vertical stride for the sliding window
   * @param  stride_w horizontal stride for the sliding window
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_in DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   *
   * @note   Supports only input and output tensors in ATON canonical format
   *
   * @note   Bit-sizes are rounded up to multiples of 8-bits
   *
   */
  int LL_ATON_LIB_DMA_RowToImage(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                                 unsigned int blocksize_w, unsigned int stride_h, unsigned int stride_w, int dma_in,
                                 int dma_out, const NN_Instance_TypeDef *nn_instance);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a tensor DepthToSpace transfer operation using stream engines `dma_in` and `dma_out`
   * @param  list of input tensor info structures
   * @param  number of inputs
   * @param  output tensor info structures
   * @param  blocksize_h vertical dimension for the blocksize
   * @param  blocksize_w horizontal dimension for the blocksize
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_in DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   *
   * @note   Supports only input and output tensors in ATON canonical format
   *
   * @note   Supports only DCR (depth-column-row) order re-arrangement
   *
   * @note   Bit-sizes are rounded up to multiples of 8-bits
   *
   */
  /** @defgroup LL_ATON_LIB_DMA_DepthToSpace function
   *  * @{
   *   */

  int LL_ATON_LIB_DMA_DepthToSpace(const LL_LIB_TensorInfo_TypeDef *list, unsigned int number,
                                   const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                                   unsigned int blocksize_w, int dma_in, int dma_out,
                                   const NN_Instance_TypeDef *nn_instance);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a cast operation to/from Qmn and float
   * @param  input tensor info structure
   * @param  output tensor info structure
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Cast function
   *  * @{
   *   */
  int LL_ATON_LIB_Cast(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output, int dma_in,
                       int dma_out, const NN_Instance_TypeDef *nn_instance);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs a Softmax operation on float inputs and output operands according to ONNX semantics
   * @param  input tensor info structure
   * @param  output tensor info structure
   * @param  axis for coalescing of shape into a 2D matrix
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Softmax function
   *  * @{
   *   */
  int LL_ATON_LIB_Softmax(const LL_LIB_TensorInfo_TypeDef *, const LL_LIB_TensorInfo_TypeDef *, unsigned int, int);
  /**
   *  * @}
   *   */

  /**
   * @brief  perform split-like slice operation using DMAs
   * @param  input tensor shape structure
   * @param  outputs tensor shape structures
   * @param  tot_out_size size of output buffer
   * @param  width_in_bytes number of bytes per `memcpy`
   * @param  fheight DMA `fheight` field
   * @param  line_offset DMA `line_offset` field
   * @param  n_bits DMA channel size
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @return Error code
   */
  int LL_ATON_LIB_DMA_Outputs_Slice_SplitLike(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *output,
                                              uint32_t tot_out_size, uint32_t width_in_bytes, uint32_t fheight,
                                              uint32_t line_offset, uint8_t n_bits, int dma_in, int dma_out,
                                              const NN_Instance_TypeDef *nn_instance);
  /**
   * @brief  performs a transpose operation on a (4-dimensional) matrix using DMA
   *         currently supported permutation(s) is/are: (0, 2, 1, 3)-onnx
   * @param  input tensor shape structure
   * @param  output tensor shape structure
   * @param  target_pos target positions of input tensor
   * @param  perm_to_use permutation to apply (when using fallback to pure SW)
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Transpose function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Transpose(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                                const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                                const uint8_t *target_pos, const uint8_t *perm_to_use, int dma_in, int dma_out,
                                const NN_Instance_TypeDef *nn_instance);
  /**
   * @}
   */

  /**
   * @brief  performs a ConvInteger operation
   * @param  input tensor shape structure
   * @param  ninputs number of inputs feat, kern, bias (optional)
   * @param  output tensor shape structure
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_ConvInteger function
   *  * @{
   *   */

  int LL_ATON_LIB_ConvInteger(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                              const LL_LIB_TensorInfo_TypeDef *output, int pad_top, int pad_left, int pad_bottom,
                              int pad_right, int stride_h, int stride_w, int dilations_h, int dilation_w, int pad_value,
                              int ngroups, char *conv_name);
  /**
   * @}
   */

  /**
   * @brief  performs a channel-split memory copy operation from one input (ATON canonical) to `noutputs`
   * non-ATON-canonical outputs using stream engines `dma_in` and `dma_out`
   * @param  src source address
   * @param  outputs list of output tensor shape structures
   * @param  nr_of_outputs number of outputs
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  int __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(const LL_Buffer_InfoTypeDef *input,
                                                      const LL_Buffer_InfoTypeDef *outputs, unsigned int nr_of_outputs,
                                                      int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance);

  /**
   * @brief  performs channel-split copy operation on an input and several outputs (both in ATON canonical format) using
   * DMA
   * @param  input tensor shape structure
   * @param  outputs tensor shape structures
   * @param  nr_of_outputs number of output tensors
   * @param  leading_dims dimensions before split axis
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_out DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  int __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(const LL_Buffer_InfoTypeDef *input,
                                                   const LL_Buffer_InfoTypeDef *outputs, unsigned int nr_of_outputs,
                                                   unsigned int leading_dims, int dma_in, int dma_out,
                                                   const NN_Instance_TypeDef *nn_instance);

  /**
   * @brief  performs flat copy operation on an input and several outputs using DMA
   * @param  input tensor shape structure
   * @param  outputs tensor shape structures
   * @param  nr_of_outputs number of output tensors
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_in DMA number of DMA writing to memory
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  int __LL_ATON_LIB_DMA_Outputs_Flat_Copy(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *outputs,
                                          unsigned int nr_of_outputs, int dma_in, int dma_out,
                                          const NN_Instance_TypeDef *nn_instance);

  /**
   * @brief  performs a `Pad` operation - composed of several HW-accelerated or pure SW `framing`/`memset` and `filling`
   * functions - on a (multi-dimensional) matrix
   * @param  input start address of input tensor
   * @param  output start address of output tensor
   * @param  min_shape shape obtained by performing element-wise `min` on each axis of reduced input vs output tensor
   * @param  mode where 0 == `constant`, 1 == `reflect`, 2 ==`edge`
   * @param  nbytes 8-bit or 16-bit mode
   * @param  out_elems number of output elements
   * @param  constant_value constant value to be `memset`
   * @param  consecutive_axis last input axis with all zero start/end paddings and whose following axes also have all
   * zero paddings (or last axis)
   * @param  consecutive_elems number of consecutive elements to copy in each repetition
   * @param  pad_in_offsets_start vector containing amount (in bytes) of input padding data added at beginning of each
   *         axis
   * @param  pad_in_offsets_end vector containing amount (in bytes) of input padding data added at and of each axis
   * @param  pad_out_offsets_start vector containing amount (in bytes) of output padding data added at beginning of each
   *         axis
   * @param  pad_out_offsets_end vector containing amount (in bytes) of output padding data added at and of each axis
   * @param  out_shape shape of output tensor
   * @param  out_offsets offset of each output tensor item
   * @param  tensor_rank rank of input and output tensors
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Pad_Standard function
   *  * @{
   *   */
  int LL_ATON_LIB_Pad_Standard(unsigned char *input, unsigned char *output, unsigned char *input_limit,
                               unsigned char *output_limit, const uint32_t *min_shape, uint8_t mode, uint8_t nbytes,
                               uint32_t out_elems, int32_t constant_value, uint32_t consecutive_axis,
                               uint32_t consecutive_elems, const int32_t *pad_in_offsets_start,
                               const int32_t *pad_in_offsets_end, const int32_t *pad_out_offsets_start,
                               const int32_t *pad_out_offsets_end, const int32_t *out_shape, const int32_t *out_offsets,
                               size_t tensor_rank, int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance);
  /**
   * @}
   */
  /**
   * @brief Performs a 4-loop optimization for the `Pad` operation
   * @param  input_start Start address of input tensor
   * @param  input_end End address of input tensor
   * @param  input_limit Input tensor limit address
   * @param  output_start Start address of output tensor
   * @param  output_end End address of output tensor
   * @param  output_limit Output tensor limit address
   * @param  constant_value Value to pad with
   * @param  nbytes number of bytes per element
   * @param  negative_4loop Information for 4-loop optimization - negative padding
   * @param  positive_4loop Information for 4-loop optimization - positive padding
   * @param  dma_in Input DMA / Streaming Engine to be used
   * @param  dma_out Output DMA / Streaming Engine to be used
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @return Error code
   */
  /** @defgroup LL_ATON_LIB_Pad_Standard function
   *  * @{
   *   */
  int LL_ATON_LIB_Pad_4Loop(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                            unsigned char *output_start, unsigned char *output_end, unsigned char *output_limit,
                            int32_t constant_value, uint8_t nbytes, uint32_t *negative_4loop, uint32_t *positive_4loop,
                            int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance);
  /**
   * @}
   */

#ifdef __cplusplus
}
#endif
#endif
