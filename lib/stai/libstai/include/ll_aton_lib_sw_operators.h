/**
 ******************************************************************************
 * @file    ll_aton_lib_sw_operators.h
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Header file of ATON library for pure SW operators
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

#ifndef __LL_ATON_LIB_SW_OPERATORS_H
#define __LL_ATON_LIB_SW_OPERATORS_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "ll_aton_NN_interface.h"
#include "ll_aton_caches_interface.h"
#include "ll_aton_lib.h"

#ifdef __cplusplus
extern "C"
{
#endif

// define C++ `inf` and `NaN` values
#define inf (INFINITY)
#define NaN (NAN)

#define __LL_PAD_FRAMING_DMA_MIN_BUFF_LEN 2700
#define __LL_PAD_FILLING_DMA_MIN_BUFF_LEN 1200

  // Uncomment beyond line to get runtime information about beyond SW operator's execution
  // #define DUMP_DEBUG_SW_OPS

  // Uncomment beyond line to dump operation results for `Pad` operator
  // #define DUMP_RESULTS_PAD_OP

#define LL_LIB_NBYTES(x) ((x) + 7) >> 3

  /**
   * @}
   */

  /**
   * @brief  performs a split operation on a (multi-dimensional) matrix
   * @param  input tensor shape structure
   * @param  aton_canonical is input tensor ATON canonical
   * @param  output tensors shape structures
   * @param  noutput number of of output tensors
   * @param  rank rank of split operation's input matrix
   * @param  split_onnx_axis which axis to split on (ONNX value)
   * @param  leading_dims product of leading dimensions
   * @param  split_case optimization to be used
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Split function
   *  * @{
   *   */
  int LL_ATON_LIB_Split(const LL_Buffer_InfoTypeDef *input, bool aton_canonical,
                        const LL_Buffer_InfoTypeDef (*outputs)[], uint32_t noutputs, uint32_t rank,
                        uint32_t split_onnx_axis, uint32_t leading_dims, int split_case, int dma_in, int dma_out);

  /**
   * @brief  performs a slice operation on a (multi-dimensional) matrix
   * @param  input tensor shape structure
   * @param  output tensor shape structure
   * @param  slice_rank rank of slice operation's input matrix
   * @param  slice_starts 1-D tensor of starting indices of corresponding axis from 0 to rank-1
   * @param  slice_ends 1-D tensor of ending indices (exclusive) of corresponding axis from 0 to rank-1
   * @param  slice_steps 1-D tensor of slice step of corresponding axis from 0 to rank-1
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Slice function
   *  * @{
   *   */
  int LL_ATON_LIB_Slice(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                        const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets, uint32_t slice_rank,
                        const int32_t *slice_starts, const int32_t *slice_ends, const int32_t *slice_steps);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a `SpaceToDepth` operation on a 4-dimensional matrix purely in SW
   * @param  input tensor shape structure
   * @param  output tensor shape structure
   * @param  bs_h height dimension of blocksize of `SpaceToDepth` operation
   * @param  bs_w width dimension of blocksize of `SpaceToDepth` operation
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_SpaceToDepth function
   *  * @{
   *   */
  int LL_ATON_LIB_SW_SpaceToDepth(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                                  const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                                  uint32_t bs_h, uint32_t bs_w);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a `DepthToSpace` operation on a 4-dimensional matrix purely in SW
   * @param  input tensor shape structure
   * @param  output tensor shape structure
   * @param  bs_h height dimension of blocksize of `DepthToSpace` operation
   * @param  bs_w width dimension of blocksize of `DepthToSpace` operation
   * @param  mode of `DepthToSpace` operation (0 = `DCR` - depth-column-row, 1 = `CRD` - column-row-depth)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_DepthToSpace function
   *  * @{
   *   */
  int LL_ATON_LIB_SW_DepthToSpace(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                                  const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                                  uint32_t bs_h, uint32_t bs_w, uint32_t mode);

  /**
   *  * @}
   *   */

  /**
   * @brief  performs a transpose operation on a (multi-dimensional) matrix
   * @param  input tensor shape structure
   * @param  output tensor shape structure
   * @param  perm permutation to apply
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Transpose function
   *  * @{
   *   */
  int LL_ATON_LIB_Transpose(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                            const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                            const uint8_t *perm);
  /**
   *  * @}
   *   */

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
                               size_t tensor_rank, int dma_in, int dma_out);
  typedef int (*pad_callback_func_t)(void *);
  typedef struct
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
  } __ll_pad_sw_params_t;

  /**
   *  * @}
   *   */

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
   * @return Error code
   */
  /** @defgroup LL_ATON_LIB_Pad_Standard function
   *  * @{
   *   */
  int LL_ATON_LIB_Pad_4Loop(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                            unsigned char *output_start, unsigned char *output_end, unsigned char *output_limit,
                            int32_t constant_value, uint8_t nbytes, uint32_t *negative_4loop, uint32_t *positive_4loop,
                            int dma_in, int dma_out);
  /**
   * @}
   */

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_LIB_SW_OPERATORS_H

/**
 * @}
 */
