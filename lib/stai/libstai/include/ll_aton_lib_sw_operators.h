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

  // Uncomment beyond line to get runtime information about beyond SW operator's execution
  // #define DUMP_DEBUG_SW_OPS

#define LL_LIB_NBYTES(x) ((x) + 7) >> 3

  /* Common inline helper functions */
  static inline void __ll_aton_lib_copy_element(uint8_t nbytes, int32_t index, int8_t *out_target, int8_t *in_target)
  {
    LL_ATON_LIB_UNUSED(index);

    switch (nbytes)
    {
    case 1:
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("[%d]\t%d\t@ out: 0x%lx, in: 0x%lx\n", index, *in_target, (uintptr_t)out_target,
                     (uintptr_t)in_target);
#endif
      *out_target = *in_target;
      return;

    case 2:
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("[%d]\t%d\t@ out: 0x%lx, in: 0x%lx\n", index, *((int16_t *)in_target), (uintptr_t)out_target,
                     (uintptr_t)in_target);
#endif
      LL_ATON_ASSERT((((uintptr_t)in_target) % 2) == 0);
      LL_ATON_ASSERT((((uintptr_t)out_target) % 2) == 0);

      *((int16_t *)out_target) = *((int16_t *)in_target);
      return;

    case 3: // NOTE: assuming no alignment
      *out_target++ = *in_target++;
      *out_target++ = *in_target++;
      *out_target++ = *in_target++;
      return;

    case 4:
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("[%d]\t%d\t@ out: 0x%lx, in: 0x%lx\n", index, *((int32_t *)in_target), (uintptr_t)out_target,
                     (uintptr_t)in_target);
#endif
      LL_ATON_ASSERT((((uintptr_t)in_target) % 4) == 0);
      LL_ATON_ASSERT((((uintptr_t)out_target) % 4) == 0);

      *((int32_t *)out_target) = *((int32_t *)in_target);
      return;

    default:
      LL_ATON_ASSERT(false);
      return;
    }
  }

  static inline uint32_t __ll_zero_neg(int32_t value)
  {
    uint32_t ret = 0;
    if (value > 0)
    {
      ret = value;
    }
    return ret;
  }

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
   * @param  dma_in Input DMA / Streaming Engine to be used
   * @param  dma_out Output DMA / Streaming Engine to be used
   * @param  nn_instance pointer to network instance (may not be `NULL`)
   * @retval Error code
   */
  /** @defgroup LL_ATON_LIB_Split function
   *  * @{
   *   */
  int LL_ATON_LIB_Split(const LL_Buffer_InfoTypeDef *input, bool aton_canonical,
                        const LL_Buffer_InfoTypeDef (*outputs)[], uint32_t noutputs, uint32_t rank,
                        uint32_t split_onnx_axis, uint32_t leading_dims, int split_case, int dma_in, int dma_out,
                        const NN_Instance_TypeDef *nn_instance);

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

#ifdef __cplusplus
}
#endif

#endif // __LL_ATON_LIB_SW_OPERATORS_H

/**
 * @}
 */
