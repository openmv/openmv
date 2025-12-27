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
#define LL_LIB_TENSOR_ELEMENTS(t)                                                                                      \
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
#define LL_LIB_TENSOR_ELEMENTS(t)                                                                                      \
  (_TDIM((t)->shape[TDIM_NKERNELS]) * _TDIM((t)->shape[TDIM_FWIDTH]) * _TDIM((t)->shape[TDIM_FHEIGHT]) *               \
   _TDIM((t)->shape[TDIM_NCHANNELS]))
#else
int LL_LIB_TENSOR_ELEMENTS(const LL_LIB_TensorInfo_TypeDef *t);
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
    int g_idx;
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
   * @param  list of input tensor info structures
   * @param  number of inputs
   * @param  output tensor info structure
   * @param  axis for concatenation
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Concat function
   *  * @{
   *   */
  int LL_ATON_LIB_Concat(const LL_Buffer_InfoTypeDef *, unsigned int, const LL_Buffer_InfoTypeDef *, unsigned int, int,
                         int);
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
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                                 unsigned stride_h, unsigned stride_w, int dma_in, int dma_out);
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
   *
   * @note   Bit-sizes are rounded up to multiples of 8-bits
   *
   */
  /** @defgroup LL_ATON_LIB_DMA_SpaceToDepth function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_SpaceToDepth(const LL_LIB_TensorInfo_TypeDef *, unsigned int, const LL_LIB_TensorInfo_TypeDef *,
                                   unsigned, unsigned, int, int);

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
   *
   * @note   Supports only input and output tensors in ATON canonical format
   *
   * @note   Bit-sizes are rounded up to multiples of 8-bits
   *
   */
  int LL_ATON_LIB_DMA_RowToImage(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                                 unsigned stride_h, unsigned stride_w, int dma_in, int dma_out);

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

  int LL_ATON_LIB_DMA_DepthToSpace(const LL_LIB_TensorInfo_TypeDef *, unsigned int, const LL_LIB_TensorInfo_TypeDef *,
                                   unsigned, unsigned, int, int);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a cast operation to/from Qmn and float
   * @param  input tensor info structure
   * @param  output tensor info structure
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Cast function
   *  * @{
   *   */
  int LL_ATON_LIB_Cast(const LL_LIB_TensorInfo_TypeDef *, const LL_LIB_TensorInfo_TypeDef *, int, int);
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
   * @brief  performs flat copy operation on an input and several outputs using DMA
   * @param  input tensor shape structure
   * @param  outputs tensor shape structures
   * @param  nr_of_outputs number of output tensors
   * @param  dma_in DMA number of DMA reading from memory
   * @param  dma_in DMA number of DMA writing to memory
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Outputs_Flat_Copy function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Outputs_Flat_Copy(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *outputs,
                                        unsigned int nr_of_outputs, int dma_in, int dma_out);
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
   * @param  dma_in DMA number of DMA writing to memory
   * @return Error code
   */
  int LL_ATON_LIB_DMA_Outputs_Slice_SplitLike(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *output,
                                              int32_t tot_out_size, int32_t width_in_bytes, int32_t fheight,
                                              int32_t line_offset, int8_t n_bits, int dma_in, int dma_out);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs channel-split copy operation on an input and several outputs (both in ATON canonical format) using
   * DMA
   * @param  input tensor shape structure
   * @param  outputs tensor shape structures
   * @param  nr_of_outputs number of output tensors
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(const LL_Buffer_InfoTypeDef *, const LL_Buffer_InfoTypeDef *,
                                                 unsigned int, unsigned int leading_dims, int dma_in, int dma_out);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs a channel-split memory copy operation from one input (ATON canonical) to `noutputs`
   * non-ATON-canonical outputs using stream engines `dma_in` and `dma_out`
   * @param  src source address
   * @param  outputs list of output tensor shape structures
   * @param  noutputs number of outputs
   * @retval Error code
   */
  int LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(const LL_Buffer_InfoTypeDef *input,
                                                    const LL_Buffer_InfoTypeDef *outputs, unsigned int nr_of_outputs,
                                                    int dma_in, int dma_out);
  /** @defgroup LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched function
   *  * @{
   *   */

  /**
   * @brief  performs an optimized `memset` for the `Pad` operator using DMA (aka Framing)
   * @param  output destination address of `memset` operation
   * @param  constant_value constant value to be set
   * @param  out_size number of bytes to output
   * @param  common_params parameters needed to setup DMAs and to forward to eventual callback function
   * @param  deep_copy if true, common_params vectors are copied to lower heap
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Pad_Memset function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Pad_Memset(void *output, int32_t constant_value, size_t out_size,
                                 __ll_pad_sw_params_t *common_params, bool deep_copy);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs HW accelerated filling operation for `Pad` operator
   * @param  init_common_params parameters needed to setup DMAs and to forward to eventual callback function
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Pad_Filling function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Pad_Filling(__ll_pad_sw_params_t *init_common_params);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs HW accelerated filling operation for `Pad` operator where 4-Loop optimization is possible
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Pad_4LoopFilling function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Pad_4Loop_Filling(__ll_pad_sw_params_t *common_params);
  /**
   *  * @}
   *   */

  /**
   * @brief  performs a transpose operation on a (4-dimensional) matrix using DMA
   *         currently supported permutation(s) is/are: (0, 2, 1, 3)-onnx
   * @param  input tensor shape structure
   * @param  output tensor shape structure
   * @param  target_pos target positions of input tensor
   * @param  perm_to_use permutation to apply (when using fallback to pure SW)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DMA_Transpose function
   *  * @{
   *   */
  int LL_ATON_LIB_DMA_Transpose(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                                const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                                const uint8_t *target_pos, const uint8_t *perm_to_use, int dma_in, int dma_out);

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
  /** @defgroup LL_ATON_LIB_DMA_Transpose function
   *  * @{
   *   */

  int LL_ATON_LIB_ConvInteger(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                              const LL_LIB_TensorInfo_TypeDef *output, int pad_top, int pad_left, int pad_bottom,
                              int pad_right, int stride_h, int stride_w, int dilations_h, int dilation_w, int pad_value,
                              int ngroups, char *conv_name);
  /**
   * @
   */

  /**
   * @brief Performs an asynchronous memory copy operation using DMAs
   * @param  input_start Start address of input tensor
   * @param  input_end End address of input tensor
   * @param  input_limit Input tensor limit address
   * @param  output_start Start address of output tensor
   * @param  dma_in Input DMA / Streaming Engine to be used
   * @param  dma_out Output DMA / Streaming Engine to be used
   * @return Error code
   */
  /** @defgroup LL_ATON_LIB_Async_Memcpy function
   *  * @{
   *   */
  int LL_ATON_LIB_Async_Memcpy(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                               unsigned char *output_start, int dma_in, int dma_out);
  /**
   * @}
   */

#ifdef __cplusplus
}
#endif
#endif
