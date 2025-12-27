/**
 ******************************************************************************
 * @file    ll_aton_lib_sw_operators.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON library for pure SW operators
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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "ll_aton_util.h" // Leave blank line after the include

#include "ll_aton_lib.h"
#include "ll_aton_lib_sw_operators.h"
#include "ll_aton_runtime.h"

/* Common data structure(s) */
typedef struct __ll_stack_lnklst
{
  struct __ll_stack_lnklst *back_link;
  uint32_t axis;
  uint32_t index;
} __ll_stack_lnklst_t;

/* Helper Functions */
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

static inline uint32_t __ll_aton_lib_calc_offset(uint32_t n, uint32_t c, uint32_t h, uint32_t w, uint32_t offset_0,
                                                 uint32_t offset_1, uint32_t offset_2, uint32_t offset_3)
{
  uint32_t result = 0;

  result += (n * offset_0);
  result += (c * offset_1);
  result += (h * offset_2);
  result += (w * offset_3);

  return result;
}

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
typedef const struct
{
  const LL_Buffer_InfoTypeDef *input;
  const uint32_t *input_axes_offsets;
  const LL_Buffer_InfoTypeDef *output;
  const uint32_t *output_axes_offsets;
  uint32_t slice_rank;
  const int32_t *slice_starts;
  const int32_t *slice_ends;
  const int32_t *slice_steps;
} __ll_slice_params_t;

static inline int32_t __ll_aton_lib_is_in_slice(const __ll_slice_params_t *common_params, __ll_stack_lnklst_t *elem)
{
  uint32_t index = elem->index;
  uint32_t axis = elem->axis;

  uint8_t slice_forwards = (common_params->slice_steps[axis] < 0) ? 0 : 1;

  int32_t ret_numerator;
  int32_t ret_denominator = abs(common_params->slice_steps[axis]);

  int32_t min;
  int32_t max;

  if (slice_forwards)
  {
    ret_numerator = (index - common_params->slice_starts[axis]);
    min = common_params->slice_starts[axis];
    max = common_params->slice_ends[axis];
  }
  else
  {
    ret_numerator = (common_params->slice_starts[axis] - index);
    max = common_params->slice_starts[axis] + 1;
    min = common_params->slice_ends[axis] + 1;
  }

  if (((index >= min) && (index < max)) && (ret_numerator % ret_denominator == 0))
  {
    LL_ATON_ASSERT(ret_numerator >= 0);
    LL_ATON_ASSERT(ret_denominator > 0);

    return (ret_numerator / ret_denominator);
  }
  else
  {
    return -1;
  }
}

static int8_t *__ll_slice_get_input_and_output_base_pos(const __ll_slice_params_t *common_params,
                                                        __ll_stack_lnklst_t *linked_stack_list, int8_t **base_in_target)
{
  LL_ATON_ASSERT(linked_stack_list != NULL);

  int8_t *in_target = (int8_t *)LL_Buffer_addr_start(common_params->input);
  int8_t *out_target = (int8_t *)LL_Buffer_addr_start(common_params->output);

  LL_ATON_ASSERT(linked_stack_list->axis == (common_params->slice_rank - 1));

  for (__ll_stack_lnklst_t *elem = linked_stack_list->back_link; elem != NULL; elem = elem->back_link)
  {
    LL_ATON_ASSERT(elem->axis < (common_params->slice_rank - 1));
    uint32_t axis = elem->axis;

    /* input */
    int32_t in_axis_size = common_params->input_axes_offsets[axis];
    in_target += (elem->index * in_axis_size);

    int32_t out_index;
    if ((out_index = __ll_aton_lib_is_in_slice(common_params, elem)) >= 0)
    {
      /* output */
      int32_t out_axis_size = common_params->output_axes_offsets[axis];
      out_target += (out_index * out_axis_size);
    }
    else
    {
      return NULL;
    }
  }

  *base_in_target = in_target;
  return out_target;
}

static inline void __ll_slice_copy_elem(const __ll_slice_params_t *common_params,
                                        __ll_stack_lnklst_t *linked_stack_list, uint32_t in_offset, uint32_t out_offset,
                                        int8_t *base_in_target, int8_t *base_out_target)
{
  const uint8_t byte_size = LL_LIB_NBYTES(common_params->input->nbits);

  int32_t out_index;
  if ((out_index = __ll_aton_lib_is_in_slice(common_params, linked_stack_list)) >= 0)
  {
    /* determine input/output position */
    int8_t *in_target = base_in_target + (linked_stack_list->index * in_offset);
    int8_t *out_target = base_out_target + (out_index * out_offset);

    __ll_aton_lib_copy_element(byte_size, out_index, out_target, in_target);
  }
}

static void __ll_aton_lib_slice(uint32_t curr_in_axis, __ll_stack_lnklst_t *back_link,
                                const __ll_slice_params_t *common_params)
{
  uint8_t slice_forwards = (common_params->slice_steps[curr_in_axis] < 0) ? 0 : 1;
  __ll_stack_lnklst_t linked_stack_list = {
      .back_link = back_link, .axis = curr_in_axis, .index = common_params->slice_starts[curr_in_axis]};

  const uint32_t index_end = common_params->slice_ends[curr_in_axis];

  if (curr_in_axis < (common_params->slice_rank - 1))
  { // intermediate axis
    if (slice_forwards)
    { // slicing forwards
      for (; linked_stack_list.index < index_end; linked_stack_list.index++)
      {
        __ll_aton_lib_slice(curr_in_axis + 1, &linked_stack_list, common_params);
      }
    }
    else
    { // slicing backwards
      do
      {
        __ll_aton_lib_slice(curr_in_axis + 1, &linked_stack_list, common_params);
        linked_stack_list.index--;
      } while (linked_stack_list.index > index_end);
    }
  }
  else
  { // last axis
    LL_ATON_ASSERT(curr_in_axis == (common_params->slice_rank - 1));

    /* Calculate input base positions */
    int8_t *base_in_target;
    int8_t *base_out_target =
        __ll_slice_get_input_and_output_base_pos(common_params, &linked_stack_list, &base_in_target);

    if (base_out_target == NULL)
      return;

    uint32_t curr_in_axis_input_offset = common_params->input_axes_offsets[curr_in_axis];
    uint32_t curr_in_axis_output_offset = common_params->output_axes_offsets[curr_in_axis];

    if (slice_forwards)
    { // slicing forwards
      for (; linked_stack_list.index < index_end; linked_stack_list.index++)
      {
        __ll_slice_copy_elem(common_params, &linked_stack_list, curr_in_axis_input_offset, curr_in_axis_output_offset,
                             base_in_target, base_out_target);
      }
    }
    else
    { // slicing backwards
      do
      {
        __ll_slice_copy_elem(common_params, &linked_stack_list, curr_in_axis_input_offset, curr_in_axis_output_offset,
                             base_in_target, base_out_target);
        linked_stack_list.index--;
      } while (linked_stack_list.index > index_end);
    }
  }
}

int LL_ATON_LIB_Slice(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                      const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets, uint32_t slice_rank,
                      const int32_t *slice_starts, const int32_t *slice_ends, const int32_t *slice_steps)
{
  if (slice_rank != input->ndims)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->ndims != output->ndims)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->nbits != output->nbits)
  { // TODO: should we support this?
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if ((input->nbits < 8) || (input->nbits > 32))
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  const __ll_slice_params_t common_params = {.input = input,
                                             .input_axes_offsets = input_axes_offsets,
                                             .output = output,
                                             .output_axes_offsets = output_axes_offsets,
                                             .slice_rank = slice_rank,
                                             .slice_starts = slice_starts,
                                             .slice_ends = slice_ends,
                                             .slice_steps = slice_steps};

  __ll_aton_lib_slice(0, NULL, &common_params);

  return LL_ATON_OK; // TODO
}

static int __ll_aton_lib_sw_outputs_flat_copy(const LL_Buffer_InfoTypeDef *input,
                                              const LL_Buffer_InfoTypeDef (*outputs)[], unsigned nr_of_outputs)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned i = 0; i < nr_of_outputs; i++)
  {
    output_size += LL_Buffer_len((*outputs) + i);
  }

  if (input_size != output_size)
  { // should never happen
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }

  uint8_t nbits = LL_LIB_NBYTES(input->nbits);
  for (unsigned i = 0; i < nr_of_outputs; i++)
  {
    if ((*outputs)[i].nbits != nbits)
    {
      __LL_LIB_ERROR(_ERR_NBITS_OUT, LL_ATON_INVALID_PARAM);
    }
  }
#endif // !NDEBUG

  if (nr_of_outputs <= 0)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  unsigned char *curr_in_addr = ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start(input));
  for (int i = 0; i < nr_of_outputs; i++)
  {
    unsigned char *out_addr = ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start((*outputs) + i));
    unsigned out_size = LL_Buffer_len((*outputs) + i);

    memcpy(out_addr, curr_in_addr, out_size);

    curr_in_addr += out_size;
  }

  return LL_ATON_OK;
}

// convert axis from ...CHW -> ...HWC
static const int aton_lut[] = {TDIM_NKERNELS, TDIM_NCHANNELS, TDIM_FHEIGHT, TDIM_FWIDTH};
#define LUT_ATON(x) (((x >= (rank - 4)) && (rank > 2)) ? (rank - 4) + aton_lut[x - (rank - 4)] : x)

// convert axis from ...HWC -> ...CHW
static const int onnx_lut[] = {TDIM_ONNX_NKERNELS, TDIM_ONNX_FHEIGHT, TDIM_ONNX_FWIDTH, TDIM_ONNX_NCHANNELS};
#define LUT_ONNX(x) (((x >= (rank - 4)) && (rank > 2)) ? (rank - 4) + onnx_lut[x - (rank - 4)] : x)

/* beyond function assumes that input and output tensors are ATON canonical */
static void __ll_aton_lib_split_aton_canonical(const LL_Buffer_InfoTypeDef *input,
                                               const LL_Buffer_InfoTypeDef (*outputs)[], int nr_of_outputs, int rank,
                                               int split_onnx_axis)
{
  int aton_axis = LUT_ATON(split_onnx_axis);
  int tot_size = LL_Buffer_len(input);

  int start = 0;
  int stop = tot_size;
  int jump_base = 1;
  for (int i = aton_axis + 1; i < rank; i++)
    jump_base *= input->shape[LUT_ONNX(i)];
  jump_base *= LL_LIB_NBYTES(input->nbits);
  int jump = jump_base * input->shape[split_onnx_axis];
  // LL_ATON_PRINTF("onnx_axis=%d, aton_axis=%d, jump_base=%d, jump=%d\n", split_onnx_axis, aton_axis, jump_base, jump);

  for (int i = 0; i < nr_of_outputs; i++)
  {
    int copy_val = (*outputs)[i].shape[split_onnx_axis] * jump_base;
    // LL_ATON_PRINTF("i=%d copy_val=%d\n", i, copy_val);

    int source;
    int dest = 0;
    for (source = start; source < stop; source += jump, dest += copy_val)
    {
      // LL_ATON_PRINTF("i=%d, dest=%d, source=%d\n", i, dest, source);
      memcpy(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start((*outputs) + i) + dest),
             ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start(input) + source), copy_val);
    }
    start += copy_val;
  }
}

/* beyond function assumes that input and output tensors are ONNX canonical */
static void __ll_aton_lib_split_onnx_canonical(const LL_Buffer_InfoTypeDef *input,
                                               const LL_Buffer_InfoTypeDef (*outputs)[], int nr_of_outputs, int rank,
                                               int split_onnx_axis)
{
  int tot_size = LL_Buffer_len(input);

  int start = 0;
  int stop = tot_size;
  int jump_base = 1;
  for (int i = split_onnx_axis + 1; i < rank; i++)
    jump_base *= input->shape[i];
  jump_base *= LL_LIB_NBYTES(input->nbits);
  int jump = jump_base * input->shape[split_onnx_axis];
  // LL_ATON_PRINTF("onnx_axis=%d, aton_axis=%d, jump_base=%d, jump=%d\n", split_onnx_axis, aton_axis, jump_base, jump);

  for (int i = 0; i < nr_of_outputs; i++)
  {
    int copy_val = (*outputs)[i].shape[split_onnx_axis] * jump_base;
    // LL_ATON_PRINTF("i=%d copy_val=%d\n", i, copy_val);

    int source;
    int dest = 0;
    for (source = start; source < stop; source += jump, dest += copy_val)
    {
      // LL_ATON_PRINTF("i=%d, dest=%d, source=%d\n", i, dest, source);
      memcpy(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start((*outputs) + i) + dest),
             ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start(input) + source), copy_val);
    }
    start += copy_val;
  }
}

/* beyond function assumes that adapted rank is equal to 3, that input tensor is ATON canonical, and that split axis
 * is channel */
static void __ll_aton_lib_split_channel_batched(const LL_Buffer_InfoTypeDef *input,
                                                const LL_Buffer_InfoTypeDef (*outputs)[], uint32_t nr_of_outputs,
                                                uint32_t rank, uint32_t split_onnx_axis)
{
  LL_ATON_ASSERT(rank >= 3); // input shape is at least 3-dimensional
  LL_ATON_ASSERT((input->batch == 0) ||
                 (input->batch == input->shape[split_onnx_axis])); // input tensor is ATON canonical
  LL_ATON_ASSERT(split_onnx_axis == (rank - 3));                   // split axis is channel

  uint32_t nbytes = LL_LIB_NBYTES(input->nbits);
  uint32_t fheight = input->shape[rank - 2];
  uint32_t fwidth = input->shape[rank - 1];
  uint32_t in_nchannels = input->shape[split_onnx_axis];
  uint32_t batch_offset = in_nchannels * nbytes;

  // assuming that all leading dimensions up to channel are equal to 1!

  unsigned char *outer_addr = ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start(input));
  for (int i = 0; i < nr_of_outputs; i++)
  {
    uint16_t out_batch = (*outputs)[i].batch;
    uint32_t batch_depth_bytes = out_batch * nbytes;
    uint32_t line_offset = in_nchannels * fwidth * nbytes;
    uint32_t loop_offset = out_batch * nbytes;
    uint32_t out_nchannels = (*outputs)[i].shape[split_onnx_axis];

    LL_ATON_ASSERT((out_nchannels % out_batch) == 0);
    uint32_t frame_tot_cnt = out_nchannels / out_batch;

    unsigned char *output_addr = ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start((*outputs) + i));

    /* main loop */
    unsigned char *addr_main_loop = outer_addr;
    for (int j = 0; j < frame_tot_cnt; j++)
    {
      /* repetition loop (just a single iteration => removed) */
      unsigned char *repetition_addr = addr_main_loop;

      /* line loop */
      unsigned char *line_addr = repetition_addr;
      for (int x = 0; x < fheight; x++)
      {
        /* batch loop */
        unsigned char *batch_addr = line_addr;
        for (int y = 0; y < fwidth; y++)
        {
          memcpy(output_addr, batch_addr, batch_depth_bytes);

          output_addr += batch_depth_bytes;
          batch_addr += batch_offset;
        }
        line_addr += line_offset;
      }
      addr_main_loop += loop_offset;
    }
    outer_addr += out_nchannels * nbytes;
  }
}

typedef enum
{ // MUST be the same as in the AtoNN compiler (file: `code_gen_info.h`)
  SW_PURE_CANONICAL = 0,
  HYBRID_CHANNEL_ATON = 1,
  SW_CHANNEL_ATON = 2,
  HYBRID_CHANNEL_BATCHED = 3,
  SW_CHANNEL_BATCHED = 4,
  DMA_CHANNEL_UNIFORM = 5,
  SW_CHANNEL_UNIFORM = 6,
  DMA_FLAT_CANONICAL = 7,
  SW_FLAT_CANONICAL = 8,
  DMA_FLAT_BATCHED = 9,
  SW_FLAT_BATCHED = 10,
  DMA_FLAT_MCD = 11,
  SW_FLAT_MCD = 12,
} _split_cases_t;

int LL_ATON_LIB_Split(const LL_Buffer_InfoTypeDef *input, bool aton_canonical, const LL_Buffer_InfoTypeDef (*outputs)[],
                      uint32_t noutputs, uint32_t rank, uint32_t split_onnx_axis, uint32_t leading_dims, int split_case,
                      int dma_in, int dma_out)
{
  if (split_onnx_axis >= rank)
  {
    __LL_LIB_ERROR(_ERR_AXIS, LL_ATON_INVALID_PARAM);
  }

  if (input->ndims != rank)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->ndims != (*outputs)[0].ndims)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->nbits != (*outputs)[0].nbits)
  { // TODO: should we support this?
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if ((input->nbits < 8) || (input->nbits > 32))
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if ((rank == 0) || (noutputs < 1))
    return LL_ATON_OK; // in case ONNX was out of specs

  switch (split_case)
  {
  case SW_PURE_CANONICAL:
  {
    LL_ATON_ASSERT(dma_in == -1);
    LL_ATON_ASSERT(dma_out == -1);

    if (aton_canonical)
    {
      __ll_aton_lib_split_aton_canonical(input, outputs, noutputs, rank, split_onnx_axis);
    }
    else
    {
      __ll_aton_lib_split_onnx_canonical(input, outputs, noutputs, rank, split_onnx_axis);
    }
  }
  break;
  case SW_CHANNEL_ATON:
    LL_ATON_ASSERT(dma_in == -1);
    LL_ATON_ASSERT(dma_out == -1);
  case HYBRID_CHANNEL_ATON:
  {
    if (!aton_canonical)
    {
      __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
    }

    if ((noutputs > __LL_MAX_TENSORS) || (split_case == SW_CHANNEL_ATON))
    {
      __ll_aton_lib_split_aton_canonical(input, outputs, noutputs, rank, split_onnx_axis);
    }
    else
    {
      LL_ATON_ASSERT(dma_in != -1);
      LL_ATON_ASSERT(dma_out != -1);

      LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(input, (const LL_Buffer_InfoTypeDef *)outputs, noutputs, leading_dims,
                                                 dma_in, dma_out);
    }
  }
  break;
  case SW_CHANNEL_BATCHED:
    LL_ATON_ASSERT(dma_in == -1);
    LL_ATON_ASSERT(dma_out == -1);
  case HYBRID_CHANNEL_BATCHED:
  {
    if (!aton_canonical)
    {
      __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
    }

    if ((noutputs > __LL_MAX_TENSORS) || (split_case == SW_CHANNEL_BATCHED))
    {
      __ll_aton_lib_split_channel_batched(input, outputs, noutputs, rank, split_onnx_axis);
    }
    else
    {
      LL_ATON_ASSERT(dma_in != -1);
      LL_ATON_ASSERT(dma_out != -1);

      LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(input, (const LL_Buffer_InfoTypeDef *)outputs, noutputs, dma_in,
                                                    dma_out);
    }
  }
  break;
  case SW_FLAT_CANONICAL:
  case SW_FLAT_BATCHED:
  case SW_CHANNEL_UNIFORM:
  case SW_FLAT_MCD:
    LL_ATON_ASSERT(dma_in == -1);
    LL_ATON_ASSERT(dma_out == -1);
  case DMA_FLAT_CANONICAL:
  case DMA_FLAT_BATCHED:
  case DMA_CHANNEL_UNIFORM:
  case DMA_FLAT_MCD:
  {
    if ((noutputs > __LL_MAX_TENSORS) || (split_case == SW_FLAT_CANONICAL) || (split_case == SW_FLAT_BATCHED) ||
        (split_case == SW_CHANNEL_UNIFORM) || (split_case == SW_FLAT_MCD))
    {
      return __ll_aton_lib_sw_outputs_flat_copy(input, outputs, noutputs);
    }
    else
    {
      LL_ATON_ASSERT(dma_in != -1);
      LL_ATON_ASSERT(dma_out != -1);

      LL_ATON_LIB_DMA_Outputs_Flat_Copy(input, (const LL_Buffer_InfoTypeDef *)outputs, noutputs, dma_in, dma_out);
    }
  }
  break;
  default:
    LL_ATON_ASSERT(false);
    break;
  }

  return LL_ATON_OK;
}

int LL_ATON_LIB_SW_SpaceToDepth(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                                const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets, uint32_t bs_h,
                                uint32_t bs_w)
{
  if (input->ndims != 4)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (output->ndims != 4)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->nbits != output->nbits)
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if ((input->nbits < 8) || (input->nbits > 32))
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  uint32_t N = input->shape[0];
  uint32_t C = input->shape[1];
  uint32_t H = input->shape[2];
  uint32_t W = input->shape[3];

  int8_t *in_base = (int8_t *)LL_Buffer_addr_start(input);
  int8_t *out_base = (int8_t *)LL_Buffer_addr_start(output);

  uint32_t output_tensor_offset_0 = output_axes_offsets[0];
  uint32_t output_tensor_offset_1 = output_axes_offsets[1];
  uint32_t output_tensor_offset_2 = output_axes_offsets[2];
  uint32_t output_tensor_offset_3 = output_axes_offsets[3];

  uint32_t input_tensor_offset_0 = input_axes_offsets[0];
  uint32_t input_tensor_offset_1 = input_axes_offsets[1];
  uint32_t input_tensor_offset_2 = input_axes_offsets[2];
  uint32_t input_tensor_offset_3 = input_axes_offsets[3];

  for (uint32_t n = 0; n < N; n++)
  {
    for (uint32_t c = 0; c < C; c++)
    {
      for (uint32_t h = 0; h < H; h++)
      {
        for (uint32_t w = 0; w < W; w++)
        {
          uint32_t h_rem = h % bs_h;
          uint32_t w_rem = w % bs_w;
          uint32_t out_c = c + (((h_rem * bs_w) + w_rem) * C);
          uint32_t out_h = (h / bs_h);
          uint32_t out_w = (w / bs_w);

          int8_t *out_target = out_base + __ll_aton_lib_calc_offset(n, out_c, out_h, out_w, output_tensor_offset_0,
                                                                    output_tensor_offset_1, output_tensor_offset_2,
                                                                    output_tensor_offset_3);
          int8_t *in_target =
              in_base + __ll_aton_lib_calc_offset(n, c, h, w, input_tensor_offset_0, input_tensor_offset_1,
                                                  input_tensor_offset_2, input_tensor_offset_3);

          __ll_aton_lib_copy_element(LL_LIB_NBYTES(input->nbits), -1, out_target, in_target);
        }
      }
    }
  }

  return LL_ATON_OK;
}

int LL_ATON_LIB_SW_DepthToSpace(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                                const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets, uint32_t bs_h,
                                uint32_t bs_w, uint32_t mode)
{
  if (input->ndims != 4)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (output->ndims != 4)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->nbits != output->nbits)
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if ((input->nbits < 8) || (input->nbits > 32))
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  uint32_t N = input->shape[0];
  uint32_t C = input->shape[1];
  uint32_t H = input->shape[2];
  uint32_t W = input->shape[3];

  uint32_t bs_hw = (bs_h * bs_w);
  uint32_t max_c = (C / bs_hw);

  int8_t *in_base = (int8_t *)LL_Buffer_addr_start(input);
  int8_t *out_base = (int8_t *)LL_Buffer_addr_start(output);

  uint32_t output_tensor_offset_0 = output_axes_offsets[0];
  uint32_t output_tensor_offset_1 = output_axes_offsets[1];
  uint32_t output_tensor_offset_2 = output_axes_offsets[2];
  uint32_t output_tensor_offset_3 = output_axes_offsets[3];

  uint32_t input_tensor_offset_0 = input_axes_offsets[0];
  uint32_t input_tensor_offset_1 = input_axes_offsets[1];
  uint32_t input_tensor_offset_2 = input_axes_offsets[2];
  uint32_t input_tensor_offset_3 = input_axes_offsets[3];

  if (mode == 0)
  { // `DCR`
    for (uint32_t n = 0; n < N; n++)
    {
      for (uint32_t c = 0; c < C; c++)
      {
        for (uint32_t h = 0; h < H; h++)
        {
          for (uint32_t w = 0; w < W; w++)
          {
            uint32_t off_h = (h * bs_h);
            uint32_t off_w = (w * bs_w);
            uint32_t out_c = c % max_c;
            uint32_t out_h = off_h + (c / (max_c * bs_w));
            uint32_t out_w = off_w + ((c / max_c) % bs_w);

            int8_t *out_target = out_base + __ll_aton_lib_calc_offset(n, out_c, out_h, out_w, output_tensor_offset_0,
                                                                      output_tensor_offset_1, output_tensor_offset_2,
                                                                      output_tensor_offset_3);
            int8_t *in_target =
                in_base + __ll_aton_lib_calc_offset(n, c, h, w, input_tensor_offset_0, input_tensor_offset_1,
                                                    input_tensor_offset_2, input_tensor_offset_3);

            __ll_aton_lib_copy_element(LL_LIB_NBYTES(input->nbits), -1, out_target, in_target);
          }
        }
      }
    }
  }
  else
  { // `CRD`
    for (uint32_t n = 0; n < N; n++)
    {
      for (uint32_t out_c = 0; out_c < max_c; out_c++)
      {
        for (uint32_t c_rem_bs_2_div_bs = 0; c_rem_bs_2_div_bs < bs_h; c_rem_bs_2_div_bs++)
        {
          for (uint32_t c_rem_bs = 0; c_rem_bs < bs_w; c_rem_bs++)
          {
            uint32_t c = c_rem_bs + (c_rem_bs_2_div_bs * bs_w) + (out_c * bs_hw);

            for (uint32_t h = 0; h < H; h++)
            {
              for (uint32_t w = 0; w < W; w++)
              {
                uint32_t off_h = (h * bs_h);
                uint32_t off_w = (w * bs_w);
                uint32_t out_h = off_h + c_rem_bs_2_div_bs;
                uint32_t out_w = off_w + c_rem_bs;

                int8_t *out_target =
                    out_base + __ll_aton_lib_calc_offset(n, out_c, out_h, out_w, output_tensor_offset_0,
                                                         output_tensor_offset_1, output_tensor_offset_2,
                                                         output_tensor_offset_3);
                int8_t *in_target =
                    in_base + __ll_aton_lib_calc_offset(n, c, h, w, input_tensor_offset_0, input_tensor_offset_1,
                                                        input_tensor_offset_2, input_tensor_offset_3);

                __ll_aton_lib_copy_element(LL_LIB_NBYTES(input->nbits), -1, out_target, in_target);
              }
            }
          }
        }
      }
    }
  }
  return LL_ATON_OK;
}

/**
 * @brief  performs a transpose operation on a (multi-dimensional) matrix
 * @param  input tensor shape structure
 * @param  output tensor shape structure
 * @param  perm permutation to apply
 * @retval Error code
 */
typedef const struct
{
  uint32_t rank;
  const uint8_t *perm;
  const uint32_t *in_shape_aton;
  const uint32_t *in_axis_off;
  const uint32_t *out_axis_off;
  const uint8_t byte_size;
  const int8_t *in_tensor;
  int8_t *out_tensor;
} __ll_transp_params_t;

static inline uint32_t __ll_transp_find_input_index(__ll_stack_lnklst_t *elem, const uint8_t *perm,
                                                    uint32_t output_axis)
{
  uint32_t input_axis = (uint32_t)perm[output_axis];
  for (; elem != NULL; elem = elem->back_link)
  {
    if (elem->axis == input_axis)
    {
      return elem->index;
    }
  }
  LL_ATON_ASSERT(false); // should never be reached
  return 0;
}

static inline int8_t *__ll_transp_get_output_base_pos(uint32_t inner_out_axis,
                                                      const __ll_transp_params_t *common_params,
                                                      __ll_stack_lnklst_t *linked_stack_list)
{
  LL_ATON_ASSERT(linked_stack_list != NULL);
  int8_t *target = common_params->out_tensor;

  for (uint32_t output_axis = 0; output_axis < common_params->rank; output_axis++)
  {
    if (output_axis == inner_out_axis)
      continue;

    uint32_t axis_size = common_params->out_axis_off[output_axis];
    uint32_t input_index = __ll_transp_find_input_index(linked_stack_list, common_params->perm, output_axis);
    target += (input_index * axis_size);
  }

  return target;
}

static int8_t *__ll_transp_get_input_base_pos(const __ll_transp_params_t *common_params,
                                              __ll_stack_lnklst_t *linked_stack_list)
{
  LL_ATON_ASSERT(linked_stack_list != NULL);

  const int8_t *target = common_params->in_tensor;

  LL_ATON_ASSERT(linked_stack_list->axis == (common_params->rank - 1));

  for (__ll_stack_lnklst_t *elem = linked_stack_list->back_link; elem != NULL; elem = elem->back_link)
  {
    LL_ATON_ASSERT(elem->axis < (common_params->rank - 1));
    uint32_t axis_size = common_params->in_axis_off[elem->axis];
    target += (elem->index * axis_size);
  }

  return (int8_t *)target;
}

static void __ll_aton_lib_transpose(uint32_t curr_in_axis, uint32_t inner_out_axis, __ll_stack_lnklst_t *back_link,
                                    const __ll_transp_params_t *common_params)
{
  __ll_stack_lnklst_t linked_stack_list = {.back_link = back_link, .axis = curr_in_axis, .index = 0};

  if (curr_in_axis < (common_params->rank - 1))
  { // intermediate axis
    for (; linked_stack_list.index < common_params->in_shape_aton[linked_stack_list.axis]; linked_stack_list.index++)
    {
      __ll_aton_lib_transpose(curr_in_axis + 1, inner_out_axis, &linked_stack_list, common_params);
    }
  }
  else
  { // last axis
    LL_ATON_ASSERT(curr_in_axis == (common_params->rank - 1));

    /* Calculate input/output base positions */
    int8_t *base_out_target = __ll_transp_get_output_base_pos(inner_out_axis, common_params, &linked_stack_list);
    int8_t *base_in_target = __ll_transp_get_input_base_pos(common_params, &linked_stack_list);
    uint32_t out_axes_offset = common_params->out_axis_off[inner_out_axis];

    const uint32_t end_index = common_params->in_shape_aton[(common_params->rank - 1)];
    const uint8_t byte_size = common_params->byte_size;

    if (byte_size != out_axes_offset)
    {
      for (; linked_stack_list.index < end_index; linked_stack_list.index++)
      {
        /* determine input/output position */
        int8_t *in_target = base_in_target + (linked_stack_list.index * byte_size);
        int8_t *out_target = base_out_target + (linked_stack_list.index * out_axes_offset);

        __ll_aton_lib_copy_element(byte_size, linked_stack_list.index, out_target, in_target);
      }
    }
    else
    {
      uint32_t size_in_bytes = (end_index - linked_stack_list.index) * byte_size; // `byte_size == out_axes_offset`

      int8_t *in_target = base_in_target + (linked_stack_list.index * byte_size);
      int8_t *out_target =
          base_out_target + (linked_stack_list.index * out_axes_offset); // `byte_size == out_axes_offset`

      memcpy(out_target, in_target, size_in_bytes);
    }
  }
}

static inline uint32_t __ll_transp_find_input_index_3or4(uint32_t *indexes, const uint8_t *perm, uint32_t output_axis)
{
  uint32_t input_axis = (uint32_t)perm[output_axis];
  return indexes[input_axis];
}

static inline int8_t *__ll_transp_get_output_base_pos_3or4(uint32_t inner_out_axis,
                                                           const __ll_transp_params_t *common_params, uint32_t *indexes)
{
  int8_t *target = common_params->out_tensor;

  for (uint32_t output_axis = 0; output_axis < common_params->rank; output_axis++)
  {
    if (output_axis == inner_out_axis)
      continue;

    LL_ATON_ASSERT((uint32_t)common_params->perm[output_axis] < (common_params->rank - 1));

    uint32_t input_index = __ll_transp_find_input_index_3or4(indexes, common_params->perm, output_axis);
    uint32_t axis_size = common_params->out_axis_off[output_axis];
    target += (input_index * axis_size);
  }

  return target;
}

static int8_t *__ll_transp_get_input_base_pos_3or4(const __ll_transp_params_t *common_params, uint32_t *indexes)
{
  const int8_t *target = common_params->in_tensor;

  for (uint32_t axis = 0; axis < (common_params->rank - 1); axis++)
  {
    uint32_t axis_size = common_params->in_axis_off[axis];
    target += (indexes[axis] * axis_size);
  }

  return (int8_t *)target;
}

static inline uint32_t __ll_transp_get_inner_out_axis(const __ll_transp_params_t *common_params)
{
  for (unsigned int i = 0; i < common_params->rank; i++)
  {
    if (((uint32_t)common_params->perm[i]) == (common_params->rank - 1))
      return i;
  }
  LL_ATON_ASSERT(false);
  return common_params->rank; // make compiler happy
}

static void __ll_aton_lib_transpose_3or4(const __ll_transp_params_t *common_params)
{
  LL_ATON_ASSERT((common_params->rank == 4) || ((common_params->rank == 3)));
  uint32_t inner_out_axis = __ll_transp_get_inner_out_axis(common_params);
  uint32_t out_axes_offset = common_params->out_axis_off[inner_out_axis];
  const uint8_t byte_size = common_params->byte_size;

  uint32_t size_n = (common_params->rank == 4) ? common_params->in_shape_aton[0] : 1;
  uint32_t size_c = (common_params->rank == 4) ? common_params->in_shape_aton[1] : common_params->in_shape_aton[0];
  uint32_t size_h = (common_params->rank == 4) ? common_params->in_shape_aton[2] : common_params->in_shape_aton[1];
  uint32_t size_w = (common_params->rank == 4) ? common_params->in_shape_aton[3] : common_params->in_shape_aton[2];

  for (uint32_t index_n = 0; index_n < size_n; index_n++)
  {
    for (uint32_t index_c = 0; index_c < size_c; index_c++)
    {
      for (uint32_t index_h = 0; index_h < size_h; index_h++)
      {
        uint32_t indexes_array[] = {index_n, index_c, index_h};
        uint32_t *indexes = (common_params->rank == 4) ? &indexes_array[0] : &indexes_array[1];

        int8_t *base_out_target = __ll_transp_get_output_base_pos_3or4(inner_out_axis, common_params, indexes);
        int8_t *base_in_target = __ll_transp_get_input_base_pos_3or4(common_params, indexes);

        if (byte_size != out_axes_offset)
        {
          for (uint32_t index_w = 0; index_w < size_w; index_w++)
          {
            /* determine input/output position */
            int8_t *in_target = base_in_target + (index_w * byte_size);
            int8_t *out_target = base_out_target + (index_w * out_axes_offset);

            __ll_aton_lib_copy_element(byte_size, index_w, out_target, in_target);
          }
        }
        else
        {
          uint32_t size_in_bytes = size_w * byte_size; // `byte_size == out_axes_offset`
          memcpy(base_out_target, base_in_target, size_in_bytes);
        }
      }
    }
  }
}

int LL_ATON_LIB_Transpose(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                          const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets, const uint8_t *perm)
{
  if (input->ndims <= 2)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->ndims != output->ndims)
  {
    __LL_LIB_ERROR(_ERR_RANK, LL_ATON_INVALID_PARAM);
  }

  if (input->nbits != output->nbits)
  { // TODO: should we support this?
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if ((input->nbits < 8) || (input->nbits > 32))
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  const __ll_transp_params_t common_params = {.perm = perm,
                                              .rank = input->ndims,
                                              .in_shape_aton = input->shape,
                                              .in_axis_off = input_axes_offsets,
                                              .out_axis_off = output_axes_offsets,
                                              .byte_size = LL_LIB_NBYTES(input->nbits),
                                              .in_tensor = (int8_t *)LL_Buffer_addr_start(input),
                                              .out_tensor = (int8_t *)LL_Buffer_addr_start(output)};

  if (input->ndims <= 4)
  {
    __ll_aton_lib_transpose_3or4(&common_params);
  }
  else
  {
    uint32_t inner_out_axis = __ll_transp_get_inner_out_axis(&common_params);
    __ll_aton_lib_transpose(0, inner_out_axis, NULL, &common_params);
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs a `Pad` operation on a (multi-dimensional) matrix
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
 * axis
 * @param  pad_in_offsets_end vector containing amount (in bytes) of input padding data added at and of each axis
 * @param  pad_out_offsets_start vector containing amount (in bytes) of output padding data added at beginning of each
 * axis
 * @param  pad_out_offsets_end vector containing amount (in bytes) of output padding data added at and of each axis
 * @retval Error code
 */
static void *__ll_aton_lib_memset16(void *dst, int16_t c, size_t n)
{
  uintptr_t rest_addr = ((uintptr_t)dst) % 4;
  int8_t *_dst = (int8_t *)dst;

  LL_ATON_ASSERT((rest_addr == 0) || (rest_addr == 2));
  if (rest_addr != 0)
  {
    *((int16_t *)dst) = (int16_t)c;
    n -= 2;
    _dst += 2;
  }

  uint32_t rest_dim = n % 4;
  LL_ATON_ASSERT((rest_dim == 0) || (rest_dim == 2));
  if (rest_dim != 0)
  {
    *((int16_t *)(_dst + n - 2)) = (int16_t)c;
    n -= 2;
  }

  LL_ATON_ASSERT((n % 4) == 0);
  LL_ATON_ASSERT((n == 0) || ((((uintptr_t)_dst) % 4) == 0));

  if (n > 0)
  {
    int32_t repetition_constant = (c << 16) | (0xFFFF & c);
    for (unsigned int i = 0; i < n; i += 4)
    {
      *((int32_t *)(_dst + i)) = (int32_t)repetition_constant;
    }
  }

  return dst;
}

static void *__ll_aton_lib_memset24(void *dst, int32_t c, size_t n) // NOTE: assuming no alignment
{
  int8_t *_dst = (int8_t *)dst;
  int32_t num_elems = (n / 3);
  LL_ATON_ASSERT((n % 3) == 0);

  for (int i = 0; i < num_elems; i++)
  {
    *_dst++ = c & 0xFF;
    *_dst++ = (c >> 8) & 0xFF;
    *_dst++ = (c >> 16) & 0xFF;
  }

  return dst;
}

static void *__ll_aton_lib_memset32(void *dst, int32_t c, size_t n)
{
  int32_t *_dst = (int32_t *)dst;
  int32_t num_elems = (n / 4);

  LL_ATON_ASSERT((((uintptr_t)dst) % 4) == 0);
  LL_ATON_ASSERT((n % 4) == 0);

  for (int i = 0; i < num_elems; i++)
  {
    *_dst++ = c;
  }

  return dst;
}

static inline void *__ll_aton_lib_memset(uint8_t nbytes, void *dst, int32_t c, size_t n)
{
  if (n > 0)
  {
    switch (nbytes)
    {
    case 1:
      return memset(dst, c, n);
    case 2:
      return __ll_aton_lib_memset16(dst, c, n);
    case 3:
      return __ll_aton_lib_memset24(dst, c, n);
    case 4:
      return __ll_aton_lib_memset32(dst, c, n);
    default:
      LL_ATON_ASSERT(false);
      return dst;
    }
  }
  else
  {
    return dst;
  }
}

static inline uint32_t zero_neg(int32_t value)
{
  uint32_t ret = 0;
  if (value > 0)
  {
    ret = value;
  }
  return ret;
}

static inline void __ll_aton_lib_reflect_inner_framing_sw(uint32_t curr_axis, uint8_t nbytes, int8_t *dst, int8_t *src,
                                                          int32_t n_elems, int32_t length, bool start)
{
  LL_ATON_ASSERT(n_elems > 0);
  if (nbytes == 2)
  {
    LL_ATON_ASSERT((((uintptr_t)src) % 2) == 0);
  }
  if (nbytes == 4)
  {
    LL_ATON_ASSERT((((uintptr_t)src) % 4) == 0);
  }

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): reflect, curr_axis=%d, dst=%p, src=%p, min_shape_elems=%d, bytes=%d\n", __func__, __LINE__,
                 curr_axis, dst, src, n_elems, length);
#if (ATON_PLAT_HAS_FFLUSH)
  LL_ATON_FFLUSH(stdout);
#endif
#endif

  bool forward = start;

  if (start)
  {
    int8_t *dst_ptr = dst + length - nbytes;
    int32_t src_idx = (n_elems > 1) ? 1 : 0;

    for (; length > 0; length -= nbytes)
    {
      LL_ATON_ASSERT(dst_ptr >= dst);
      LL_ATON_ASSERT(src_idx < n_elems);
      LL_ATON_ASSERT(src_idx >= 0);

#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): reflect, curr_axis=%d, dst=%p, src=%p, src_idx=%d, nbytes=%d\n", __func__, __LINE__,
                     curr_axis, dst_ptr, src, src_idx, nbytes);
#endif

      int8_t *src_ptr = (int8_t *)(&((int8_t *)src)[src_idx * nbytes]);
      __ll_aton_lib_copy_element(nbytes, src_idx, dst_ptr, src_ptr);

      dst_ptr -= nbytes;
      if (forward)
      {
        if (src_idx >= (n_elems - 1))
        {
          forward = false;
          src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
        }
      }
      else
      {
        if (src_idx <= 0)
        {
          forward = true;
          src_idx = (n_elems > 1) ? 1 : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
        }
      }
    }
  }
  else
  {
    int8_t *dst_ptr = dst;
#ifndef NDEBUG
    int8_t *last_ptr = dst + length;
#endif

    int32_t src_idx = (n_elems > 1) ? -1 : 0;
    for (; length > 0; length -= nbytes)
    {
      LL_ATON_ASSERT(dst_ptr < last_ptr);
      LL_ATON_ASSERT((-src_idx) < n_elems);
      LL_ATON_ASSERT(src_idx <= 0);

      int8_t *src_ptr = (int8_t *)(&((int8_t *)src)[src_idx * nbytes]);
      __ll_aton_lib_copy_element(nbytes, src_idx, dst_ptr, src_ptr);

      dst_ptr += nbytes;
      if (forward)
      {
        if (src_idx >= 0)
        {
          forward = false;
          src_idx = (n_elems > 1) ? -1 : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
        }
      }
      else
      {
        if ((-src_idx) >= (n_elems - 1))
        {
          forward = true;
          src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
        }
      }
    }
  }
}

static inline void __ll_aton_lib_edge_framing_sw(uint32_t curr_axis, uint8_t nbytes, void *dst, void *src,
                                                 int32_t length)
{
#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): memset, curr_axis=%d, dst=%p, src=%p, bytes=%d\n", __func__, __LINE__, curr_axis, dst, src,
                 length);
#endif
  __ll_aton_lib_memset(nbytes, dst, *((int32_t *)src), length);
}

static void __ll_aton_lib_pad_filling_sw(uint32_t curr_axis, __ll_pad_sw_params_t *common_params)
{
  LL_ATON_ASSERT(curr_axis <= common_params->consecutive_axis);

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): in=%lx, out=%lx, curr_axis=%u, min_dim=%u, in_start=%d, out_start=%d\n", __func__, __LINE__,
                 (uintptr_t)common_params->in_target, (uintptr_t)common_params->out_target, curr_axis,
                 common_params->min_shape[curr_axis], common_params->pad_in_offsets_start[curr_axis],
                 common_params->pad_out_offsets_start[curr_axis]);
#endif

  if (common_params->pad_in_offsets_start[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_start[curr_axis];
  }

  if (common_params->pad_out_offsets_start[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_start[curr_axis];
  }

  if (curr_axis == common_params->consecutive_axis)
  {
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): ASSIGN in=%lx, out=%lx, bytes=%u\n", __func__, __LINE__,
                   (uintptr_t)common_params->in_target, (uintptr_t)common_params->out_target,
                   common_params->consecutive_bytes);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif

    memcpy(common_params->out_target, common_params->in_target, common_params->consecutive_bytes);
    common_params->in_target += common_params->consecutive_bytes;
    common_params->out_target += common_params->consecutive_bytes;

    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }
  else
  {
    for (uint32_t i = 0; i < common_params->min_shape[curr_axis]; i++)
    {
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): i=%" PRIu32 "\n", __func__, __LINE__, i);
#endif

      __ll_aton_lib_pad_filling_sw(curr_axis + 1, common_params);
    }
  }

  if (common_params->pad_out_offsets_end[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_end[curr_axis];
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }

  if (common_params->pad_in_offsets_end[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_end[curr_axis];
  }

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): curr_axis=%u, in_end=%d, out_end=%d\n", __func__, __LINE__, curr_axis,
                 common_params->pad_in_offsets_end[curr_axis], common_params->pad_out_offsets_end[curr_axis]);
#endif
}

static void __ll_aton_lib_pad_reflect_sw(uint32_t curr_axis, __ll_pad_sw_params_t *common_params, bool fill)
{
  LL_ATON_ASSERT(curr_axis <= (common_params->tensor_rank - 1));
  LL_ATON_ASSERT(common_params->pad_in_offsets_start[curr_axis] >= 0);

  int8_t *curr_out_ptr = common_params->out_target;

  if (common_params->pad_out_offsets_start[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_start[curr_axis];
  }

  if (curr_axis == (common_params->tensor_rank - 1))
  {
    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      __ll_aton_lib_reflect_inner_framing_sw(curr_axis, common_params->nbytes, curr_out_ptr, common_params->in_target,
                                             common_params->min_shape[curr_axis],
                                             common_params->pad_out_offsets_start[curr_axis], true);
    }

    int32_t filling_bytes = common_params->min_shape[curr_axis] * common_params->nbytes;

    if (fill)
    {
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d\n", __func__, __LINE__, curr_axis,
                     (int8_t *)common_params->out_target, (int8_t *)common_params->in_target, filling_bytes);
#endif

      memcpy(common_params->out_target, common_params->in_target, filling_bytes);
    }

    common_params->in_target += filling_bytes;
    common_params->out_target += filling_bytes;

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }
  else
  {
    for (int i = 0; i < common_params->min_shape[curr_axis]; i++)
    {
      __ll_aton_lib_pad_reflect_sw(curr_axis + 1, common_params, fill);
    }

    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      int32_t pad_start = common_params->pad_out_offsets_start[curr_axis] /
                          common_params->out_offsets[curr_axis]; // i.e.: pads_start[curr_axis];
      int32_t pad_end = common_params->pad_out_offsets_end[curr_axis] /
                        common_params->out_offsets[curr_axis]; // i.e.: pads_end[curr_axis];

      LL_ATON_ASSERT(pad_end >= 0);

      int32_t nr_loops = pad_start;
      int32_t n_elems = common_params->out_shape[curr_axis] - pad_start - pad_end;
      int32_t padding_bytes = common_params->out_offsets[curr_axis];

      int8_t *dst_ptr = curr_out_ptr + common_params->pad_out_offsets_start[curr_axis] - padding_bytes;
      int8_t *src_ptr = curr_out_ptr + common_params->pad_out_offsets_start[curr_axis];

      int32_t src_idx = (n_elems > 1) ? 1 : 0;

      bool forward = true;
      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, src_idx=%d, bytes=%d, n_elems=%d, times=%d/%d\n",
                       __func__, __LINE__, curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr + (src_idx * padding_bytes),
                       src_idx, padding_bytes, n_elems, i + 1, nr_loops);
#if (ATON_PLAT_HAS_FFLUSH)
        LL_ATON_FFLUSH(stdout);
#endif
#endif
        LL_ATON_ASSERT(dst_ptr >= curr_out_ptr);
        LL_ATON_ASSERT(src_idx < n_elems);
        LL_ATON_ASSERT(src_idx >= 0);

        memcpy(dst_ptr, src_ptr + (src_idx * padding_bytes), padding_bytes);

        dst_ptr -= padding_bytes;

        if (forward)
        {
          if (src_idx >= (n_elems - 1))
          {
            forward = false;
            src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
          }
        }
        else
        {
          if (src_idx <= 0)
          {
            forward = true;
            src_idx = (n_elems > 1) ? 1 : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
          }
        }
      }
    }
  }

  if (common_params->pad_out_offsets_end[curr_axis] > 0)
  {
    if (curr_axis == (common_params->tensor_rank - 1))
    {
      int8_t *src_ptr = common_params->out_target - common_params->nbytes;
      __ll_aton_lib_reflect_inner_framing_sw(curr_axis, common_params->nbytes, common_params->out_target, src_ptr,
                                             common_params->min_shape[curr_axis],
                                             common_params->pad_out_offsets_end[curr_axis], false);
    }
    else
    {
      int32_t pad_start = common_params->pad_out_offsets_start[curr_axis] /
                          common_params->out_offsets[curr_axis]; // i.e.: pads_start[curr_axis];
      int32_t pad_end = common_params->pad_out_offsets_end[curr_axis] /
                        common_params->out_offsets[curr_axis]; // i.e.: pads_end[curr_axis];

      int32_t nr_loops = pad_end;

      int32_t padding_bytes = common_params->out_offsets[curr_axis];
      int32_t n_elems = common_params->out_shape[curr_axis] - zero_neg(pad_start) - zero_neg(pad_end);

      int8_t *dst_ptr = common_params->out_target;
      int8_t *src_ptr = common_params->out_target - padding_bytes;

      int32_t src_idx = (n_elems > 1) ? -1 : 0;

      bool forward = false;
      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, src_idx=%d, bytes=%d, n_elems=%d, times=%d/%d\n",
                       __func__, __LINE__, curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr + (src_idx * padding_bytes),
                       src_idx, padding_bytes, n_elems, i + 1, nr_loops);
#if (ATON_PLAT_HAS_FFLUSH)
        LL_ATON_FFLUSH(stdout);
#endif
#endif
        LL_ATON_ASSERT(dst_ptr < (dst_ptr + common_params->pad_out_offsets_end[curr_axis]));
        LL_ATON_ASSERT((-src_idx) < n_elems);
        LL_ATON_ASSERT(src_idx <= 0);

        memcpy(dst_ptr, src_ptr + (src_idx * padding_bytes), padding_bytes);

        dst_ptr += padding_bytes;
        if (forward)
        {
          if (src_idx >= 0)
          {
            forward = false;
            src_idx = (n_elems > 1) ? -1 : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
          }
        }
        else
        {
          if ((-src_idx) >= (n_elems - 1))
          {
            forward = true;
            src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
          }
        }
      }
    }

    common_params->out_target += common_params->pad_out_offsets_end[curr_axis];

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }

  LL_ATON_ASSERT(common_params->pad_in_offsets_end[curr_axis] >= 0);
}

static void __ll_aton_lib_pad_edge_sw(uint32_t curr_axis, __ll_pad_sw_params_t *common_params, bool fill)
{
  LL_ATON_ASSERT(curr_axis <= (common_params->tensor_rank - 1));

  if (common_params->pad_in_offsets_start[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_start[curr_axis];
  }

  int8_t *curr_out_ptr = common_params->out_target;

  if (common_params->pad_out_offsets_start[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_start[curr_axis];
  }

  if (curr_axis == (common_params->tensor_rank - 1))
  {
    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      __ll_aton_lib_edge_framing_sw(curr_axis, common_params->nbytes, curr_out_ptr, common_params->in_target,
                                    common_params->pad_out_offsets_start[curr_axis]);
    }

    int32_t filling_bytes = common_params->min_shape[curr_axis] * common_params->nbytes;

    if (fill)
    {
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d\n", __func__, __LINE__, curr_axis,
                     (int8_t *)common_params->out_target, (int8_t *)common_params->in_target, filling_bytes);
#endif

      memcpy(common_params->out_target, common_params->in_target, filling_bytes);
    }

    common_params->in_target += filling_bytes;
    common_params->out_target += filling_bytes;

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }
  else
  {
    for (int i = 0; i < common_params->min_shape[curr_axis]; i++)
    {
      __ll_aton_lib_pad_edge_sw(curr_axis + 1, common_params, fill);
    }

    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      int32_t nr_loops = common_params->pad_out_offsets_start[curr_axis] /
                         common_params->out_offsets[curr_axis]; // i.e.: pads_start[curr_axis];
      int32_t padding_bytes = common_params->pad_out_offsets_start[curr_axis] / nr_loops;

      int8_t *dst_ptr = curr_out_ptr;
      int8_t *src_ptr = curr_out_ptr + common_params->pad_out_offsets_start[curr_axis];

      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d, times=%d/%d\n", __func__, __LINE__,
                       curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr, padding_bytes, i + 1, nr_loops);
#endif
        memcpy(dst_ptr, src_ptr, padding_bytes);
        dst_ptr += padding_bytes;
      }
    }
  }

  if (common_params->pad_out_offsets_end[curr_axis] > 0)
  {
    if (curr_axis == (common_params->tensor_rank - 1))
    {
      __ll_aton_lib_edge_framing_sw(curr_axis, common_params->nbytes, common_params->out_target,
                                    (common_params->out_target - common_params->nbytes),
                                    common_params->pad_out_offsets_end[curr_axis]);
    }
    else
    {
      int32_t nr_loops = common_params->pad_out_offsets_end[curr_axis] /
                         common_params->out_offsets[curr_axis]; // i.e. pads_end[curr_axis];
      int32_t padding_bytes = common_params->pad_out_offsets_end[curr_axis] / nr_loops;

      int8_t *dst_ptr = common_params->out_target;
      int8_t *src_ptr = common_params->out_target - (common_params->pad_out_offsets_end[curr_axis] / nr_loops);

      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d, times=%d/%d\n", __func__, __LINE__,
                       curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr, padding_bytes, i + 1, nr_loops);
#endif
        memcpy(dst_ptr, src_ptr, padding_bytes);
        dst_ptr += padding_bytes;
      }
    }

    common_params->out_target += common_params->pad_out_offsets_end[curr_axis];

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }

  if (common_params->pad_in_offsets_end[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_end[curr_axis];
  }
}

static int __ll_aton_lib_pad_filling(__ll_pad_sw_params_t *common_params)
{
#ifndef NDEBUG
  extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
#endif // !NDEBUG

#if defined(DUMP_RESULTS_PAD_OP)
  int8_t *out_target = common_params->out_target;
#endif // DUMP_RESULTS_PAD_OP

  /* check if there is something to do */
  if (common_params->consecutive_bytes == 0)
  {
    goto dump_results; // nothing needs to be copied/filled in (just `memset`), so we are done!
  }

  /* fill with content */
  if ((common_params->consecutive_bytes < __LL_PAD_FILLING_DMA_MIN_BUFF_LEN) ||
      (common_params->tensor_rank > __LL_DMA_PAD_MAX_DIMS))
  { // do it without HW support
    __ll_aton_lib_pad_filling_sw(0, common_params);

    if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
    {
      if ((common_params->end_out_target - common_params->saved_out_target) > 0)
      {
        /* *** MCU cache clean operation (SW) *** */
        uint32_t size = (uintptr_t)(common_params->end_out_target) - (uintptr_t)(common_params->saved_out_target);
        LL_ATON_Cache_MCU_Clean_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->saved_out_target),
                                      size);
      }
    }

  dump_results:
#if defined(DUMP_RESULTS_PAD_OP)
    /* debug output print */
    switch (common_params->nbytes)
    {
    case 1:
    {
      int8_t *ptr = (int8_t *)out_target;
      for (uint32_t i = 0; i < common_params->out_size; i++)
      {
        LL_ATON_PRINTF("%d\n", ptr[i]);
      }
    }
    break;
    case 2:
    {
      int16_t *ptr = (int16_t *)(int8_t *)out_target;
      for (uint32_t i = 0; i < common_params->out_size / 2; i++)
      {
        LL_ATON_PRINTF("%d\n", ptr[i]);
      }
    }
    break;
    case 3: // NOTE: assuming no alignment
    {
      /* check endianness */
      const int32_t _const_val = 0x01020304;
      const int8_t *_const_val_ptr = (int8_t *)&_const_val;
      bool is_little_endian = (_const_val_ptr[0] == 0x04);

      int8_t *ptr = (int8_t *)out_target;
      if (is_little_endian)
      {
        for (uint32_t i = 0; i < common_params->out_size / 3; i += 3)
        {
          int32_t value = 0;

          value |= ptr[i];
          value |= ptr[i + 1] << 8;
          value |= ptr[i + 2] << 16;

          LL_ATON_PRINTF("%d\n", value);
        }
      }
      else
      {
        for (uint32_t i = 0; i < common_params->out_size / 3; i += 3)
        {
          int32_t value = 0;

          value |= ptr[i] << 16;
          value |= ptr[i + 1] << 8;
          value |= ptr[i + 2];

          LL_ATON_PRINTF("%d\n", value);
        }
      }
    }
    break;
    case 4:
    {
      int32_t *ptr = (int32_t *)(int8_t *)out_target;
      for (uint32_t i = 0; i < common_params->out_size / 4; i++)
      {
        LL_ATON_PRINTF("%d\n", ptr[i]);
      }
    }
    break;
    default:
      LL_ATON_ASSERT(false);
      break;
    }
#endif // DUMP_RESULTS_PAD_OP

    return LL_ATON_OK;
  }
  else
  {                                               // perform second phase in HW
    if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
    {
      common_params->callback_function = NULL;
      return LL_ATON_LIB_DMA_Pad_Filling(NULL);
    }
    else
    { // not a callback => `memset` (i.e. `framing`) has been executed in SW
      if ((common_params->end_out_target - common_params->saved_out_target) > 0)
      {
        /* *** MCU cache clean & invalidate operation (SW) *** */
        uint32_t size = (uintptr_t)(common_params->end_out_target) - (uintptr_t)(common_params->saved_out_target);
        LL_ATON_Cache_MCU_Clean_Invalidate_Range(
            ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->saved_out_target), size);
      }

      return LL_ATON_LIB_DMA_Pad_Filling(common_params);
    }
  }
}

static int __ll_aton_lib_pad_framing_sw(__ll_pad_sw_params_t *common_params)
{
#ifndef NDEBUG
  extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
#endif // !NDEBUG

  /* reset input/output target pointers */
  common_params->in_target = common_params->saved_in_target;
  common_params->out_target = common_params->saved_out_target;

#if defined(DUMP_RESULTS_PAD_OP)
  int8_t *out_target = common_params->out_target;
#endif // DUMP_RESULTS_PAD_OP

  switch (common_params->mode)
  {
  case 1: // `reflect` mode
    __ll_aton_lib_pad_reflect_sw(0, common_params, false);
    break;

  case 2: // `edge` mode
    __ll_aton_lib_pad_edge_sw(0, common_params, false);
    break;

  default:
    __LL_LIB_ERROR(_ERR_MODE, LL_ATON_INVALID_PARAM)
  }

  LL_ATON_ASSERT(common_params->callback_function != NULL); /* always "called as callback" */
  if ((common_params->end_out_target - common_params->saved_out_target) > 0)
  {
    /* *** MCU cache clean operation (SW) *** */
    uint32_t size = (uintptr_t)(common_params->end_out_target) - (uintptr_t)(common_params->saved_out_target);
    LL_ATON_Cache_MCU_Clean_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->saved_out_target), size);
  }

#if defined(DUMP_RESULTS_PAD_OP)
  /* debug output print */
  switch (common_params->nbytes)
  {
  case 1:
  {
    int8_t *ptr = (int8_t *)out_target;
    for (uint32_t i = 0; i < common_params->out_size; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 2:
  {
    int16_t *ptr = (int16_t *)(int8_t *)out_target;
    for (uint32_t i = 0; i < common_params->out_size / 2; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 3: // NOTE: assuming no alignment
  {
    /* check endianness */
    const int32_t _const_val = 0x01020304;
    const int8_t *_const_val_ptr = (int8_t *)&_const_val;
    bool is_little_endian = (_const_val_ptr[0] == 0x04);

    int8_t *ptr = (int8_t *)out_target;
    if (is_little_endian)
    {
      for (uint32_t i = 0; i < common_params->out_size / 3; i += 3)
      {
        int32_t value = 0;

        value |= ptr[i];
        value |= ptr[i + 1] << 8;
        value |= ptr[i + 2] << 16;

        LL_ATON_PRINTF("%d\n", value);
      }
    }
    else
    {
      for (uint32_t i = 0; i < common_params->out_size / 3; i += 3)
      {
        int32_t value = 0;

        value |= ptr[i] << 16;
        value |= ptr[i + 1] << 8;
        value |= ptr[i + 2];

        LL_ATON_PRINTF("%d\n", value);
      }
    }
  }
  break;
  case 4:
  {
    int32_t *ptr = (int32_t *)(int8_t *)out_target;
    for (uint32_t i = 0; i < common_params->out_size / 4; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  default:
    LL_ATON_ASSERT(false);
    break;
  }
#endif // DUMP_RESULTS_PAD_OP

  return LL_ATON_OK;
}

int LL_ATON_LIB_Pad_Standard(unsigned char *input, unsigned char *output, unsigned char *input_limit,
                             unsigned char *output_limit, const uint32_t *min_shape, uint8_t mode, uint8_t nbytes,
                             uint32_t out_elems, int32_t constant_value, uint32_t consecutive_axis,
                             uint32_t consecutive_elems, const int32_t *pad_in_offsets_start,
                             const int32_t *pad_in_offsets_end, const int32_t *pad_out_offsets_start,
                             const int32_t *pad_out_offsets_end, const int32_t *out_shape, const int32_t *out_offsets,
                             size_t tensor_rank, int dma_in, int dma_out)
{
  LL_ATON_ASSERT(dma_in >= 0);
  LL_ATON_ASSERT(dma_out >= 0);

  size_t out_size = out_elems * nbytes;
  uint32_t consecutive_bytes = consecutive_elems * nbytes;

  __ll_pad_sw_params_t common_params = {
    .in_target = (int8_t *)input,
    .out_target = (int8_t *)output,
    .in_limit = (int8_t *)input_limit,
    .out_limit = (int8_t *)output_limit,
    .nbytes = nbytes,
    .mode = mode,
    .tensor_rank = tensor_rank,
    .saved_in_target = (int8_t *)input,
    .saved_out_target = (int8_t *)output,
    .end_out_target = (int8_t *)output + out_size,
    .callback_function = NULL,
    .consecutive_axis = consecutive_axis,
    .consecutive_bytes = consecutive_bytes,
    .dma_in = dma_in,
    .dma_out = dma_out,
#if defined(DUMP_RESULTS_PAD_OP)
    .out_size = out_size,
#endif

    .min_shape = min_shape,
    .pad_in_offsets_start = pad_in_offsets_start,
    .pad_in_offsets_end = pad_in_offsets_end,
    .pad_out_offsets_start = pad_out_offsets_start,
    .pad_out_offsets_end = pad_out_offsets_end,
    .out_shape = out_shape,
    .out_offsets = out_offsets,
  };

  if (consecutive_bytes == 0)
  {
    // corner case: nothing needs to be copied/filled in (just `memset`), align with `onnxruntime (v1.2.0)`
    // reference implementation
    constant_value = 0;
  }

  switch (mode)
  {
  case 0: // `constant` mode
    /* fill output with constant value
       (TODO: trade-off between one shot and multiple single short shots - without overwriting non constant value
       ranges - still to be evaluated) */
    if ((out_size < __LL_PAD_FRAMING_DMA_MIN_BUFF_LEN) || (tensor_rank > __LL_DMA_PAD_MAX_DIMS))
    {
      __ll_aton_lib_memset(nbytes, (int8_t *)output, constant_value, out_size);
      return __ll_aton_lib_pad_filling(&common_params);
    }
    else
    {
      common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_filling;
      return LL_ATON_LIB_DMA_Pad_Memset((int8_t *)output, constant_value, out_size, &common_params,
                                        true); // first phase (in this case `framing`) is made in HW
    }

  case 1: // `reflect` mode
    if ((consecutive_bytes < __LL_PAD_FILLING_DMA_MIN_BUFF_LEN) || (tensor_rank > __LL_DMA_PAD_MAX_DIMS))
    {                                                        // do it without HW support
      __ll_aton_lib_pad_reflect_sw(0, &common_params, true); // all (i.e. `filling` & `framing`) is done in SW
    }
    else
    {
      common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_framing_sw;
      return LL_ATON_LIB_DMA_Pad_Filling(&common_params); // first phase (in this case `filling`) is made in HW
    }
    break;

  case 2: // `edge` mode
    if ((consecutive_bytes < __LL_PAD_FILLING_DMA_MIN_BUFF_LEN) || (tensor_rank > __LL_DMA_PAD_MAX_DIMS))
    {                                                     // do it without HW support
      __ll_aton_lib_pad_edge_sw(0, &common_params, true); // all is done in SW
    }
    else
    {
      common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_framing_sw;
      return LL_ATON_LIB_DMA_Pad_Filling(&common_params); // first phase (in this case `filling`) is made in HW
    }
    break;

  default:
    __LL_LIB_ERROR(_ERR_MODE, LL_ATON_INVALID_PARAM)
  }

#if defined(DUMP_RESULTS_PAD_OP)
  /* debug output print */
  switch (common_params.nbytes)
  {
  case 1:
  {
    int8_t *ptr = (int8_t *)output;
    for (uint32_t i = 0; i < common_params.out_size; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 2:
  {
    int16_t *ptr = (int16_t *)(int8_t *)output;
    for (uint32_t i = 0; i < common_params.out_size / 2; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 3: // NOTE: assuming no alignment
  {
    /* check endianness */
    const int32_t _const_val = 0x01020304;
    const int8_t *_const_val_ptr = (int8_t *)&_const_val;
    bool is_little_endian = (_const_val_ptr[0] == 0x04);

    int8_t *ptr = (int8_t *)output;
    if (is_little_endian)
    {
      for (uint32_t i = 0; i < common_params.out_size / 3; i += 3)
      {
        int32_t value = 0;

        value |= ptr[i];
        value |= ptr[i + 1] << 8;
        value |= ptr[i + 2] << 16;

        LL_ATON_PRINTF("%d\n", value);
      }
    }
    else
    {
      for (uint32_t i = 0; i < common_params.out_size / 3; i += 3)
      {
        int32_t value = 0;

        value |= ptr[i] << 16;
        value |= ptr[i + 1] << 8;
        value |= ptr[i + 2];

        LL_ATON_PRINTF("%d\n", value);
      }
    }
  }
  break;
  case 4:
  {
    int32_t *ptr = (int32_t *)(int8_t *)output;
    for (uint32_t i = 0; i < common_params.out_size / 4; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  default:
    LL_ATON_ASSERT(false);
    break;
  }
#endif // DUMP_RESULTS_PAD_OP

  return LL_ATON_OK;
}

static int __ll_aton_lib_pad_4loop_filling(__ll_pad_sw_params_t *common_params)
{
#ifndef NDEBUG
  extern NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
#endif // !NDEBUG

  if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
  {
    return LL_ATON_LIB_DMA_Pad_4Loop_Filling(NULL);
  }
  else // not a callback => `memset` (i.e. `framing`) has been executed in SW
  {
    if ((common_params->out_end - common_params->out_target) > 0)
    {
      /* *** MCU cache clean & invalidate operation (SW) *** */
      uint32_t size = (uintptr_t)(common_params->out_end) - (uintptr_t)(common_params->out_target);
      LL_ATON_Cache_MCU_Clean_Invalidate_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->out_target),
                                               size);
    }

    return LL_ATON_LIB_DMA_Pad_4Loop_Filling(common_params);
  }
}

int LL_ATON_LIB_Pad_4Loop(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                          unsigned char *output_start, unsigned char *output_end, unsigned char *output_limit,
                          int32_t constant_value, uint8_t nbytes, uint32_t *negative_4loop, uint32_t *positive_4loop,
                          int dma_in, int dma_out)
{
  LL_ATON_ASSERT(dma_in >= 0);
  LL_ATON_ASSERT(dma_out >= 0);
  LL_ATON_ASSERT(positive_4loop != NULL);

  __ll_pad_sw_params_t common_params = {
      .in_target = (int8_t *)input_start,
      .in_end = (int8_t *)input_end,
      .in_limit = (int8_t *)input_limit,
      .out_target = (int8_t *)output_start,
      .out_end = (int8_t *)output_end,
      .out_limit = (int8_t *)output_limit,
      .nbytes = nbytes,
      .callback_function = NULL,
      .dma_in = dma_in,
      .dma_out = dma_out,
      .negative_4loop = *((struct four_axes *)negative_4loop),
      .positive_4loop = *((struct four_axes *)positive_4loop),
  };

  /* fill output with constant value
     (TODO: trade-off between one shot and multiple single short shots - without overwriting non constant value
     ranges - still to be evaluated) */
  size_t out_size = (size_t)(output_end - output_start);
  if (out_size < __LL_PAD_FRAMING_DMA_MIN_BUFF_LEN)
  {
    __ll_aton_lib_memset(nbytes, (int8_t *)output_start, constant_value, out_size);
    return __ll_aton_lib_pad_4loop_filling(&common_params);
  }
  else
  {
    common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_4loop_filling;
    return LL_ATON_LIB_DMA_Pad_Memset((int8_t *)output_start, constant_value, out_size, &common_params,
                                      false); // first phase (in this case `framing`) is made in HW
  }
}
