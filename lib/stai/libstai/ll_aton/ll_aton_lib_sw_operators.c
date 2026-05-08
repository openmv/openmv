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

typedef const struct __ll_slice_params
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

typedef enum __ll_split_cases
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
} __ll_split_cases_t;

/**
 * @brief  performs a transpose operation on a (multi-dimensional) matrix
 * @param  input tensor shape structure
 * @param  output tensor shape structure
 * @param  perm permutation to apply
 * @retval Error code
 */
typedef const struct __ll_transp_params
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

// convert axis from ...CHW -> ...HWC
static const int __ll_aton_lut[] = {TDIM_NKERNELS, TDIM_NCHANNELS, TDIM_FHEIGHT, TDIM_FWIDTH};
#define __LL_LUT_ATON(x) (((x >= (rank - 4)) && (rank > 2)) ? (rank - 4) + __ll_aton_lut[x - (rank - 4)] : x)

// convert axis from ...HWC -> ...CHW
static const int __ll_onnx_lut[] = {TDIM_ONNX_NKERNELS, TDIM_ONNX_FHEIGHT, TDIM_ONNX_FWIDTH, TDIM_ONNX_NCHANNELS};
#define __LL_LUT_ONNX(x) (((x >= (rank - 4)) && (rank > 2)) ? (rank - 4) + __ll_onnx_lut[x - (rank - 4)] : x)

/* Helper Functions */
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

  if (((index >= (uint32_t)min) && (index < (uint32_t)max)) && (ret_numerator % ret_denominator == 0))
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

static int __ll_aton_lib_sw_outputs_flat_copy(const LL_Buffer_InfoTypeDef *input,
                                              const LL_Buffer_InfoTypeDef (*outputs)[], unsigned int nr_of_outputs)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned int i = 0; i < nr_of_outputs; i++)
  {
    output_size += LL_Buffer_len((*outputs) + i);
  }

  if (input_size != output_size)
  { // should never happen
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }

  uint8_t nbits = LL_LIB_NBYTES(input->nbits);
  for (unsigned int i = 0; i < nr_of_outputs; i++)
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
  for (unsigned int i = 0; i < nr_of_outputs; i++)
  {
    unsigned char *out_addr = ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start((*outputs) + i));
    unsigned int out_size = LL_Buffer_len((*outputs) + i);

    memcpy(out_addr, curr_in_addr, out_size);

    curr_in_addr += out_size;
  }

  return LL_ATON_OK;
}

/* beyond function assumes that input and output tensors are ATON canonical */
static void __ll_aton_lib_split_aton_canonical(const LL_Buffer_InfoTypeDef *input,
                                               const LL_Buffer_InfoTypeDef (*outputs)[], int nr_of_outputs, int rank,
                                               int split_onnx_axis)
{
  int aton_axis = __LL_LUT_ATON(split_onnx_axis);
  int tot_size = LL_Buffer_len(input);

  int start = 0;
  int stop = tot_size;
  int jump_base = 1;
  for (int i = aton_axis + 1; i < rank; i++)
    jump_base *= input->shape[__LL_LUT_ONNX(i)];
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
  for (unsigned int i = 0; i < nr_of_outputs; i++)
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
    for (unsigned int j = 0; j < frame_tot_cnt; j++)
    {
      /* repetition loop (just a single iteration => removed) */
      unsigned char *repetition_addr = addr_main_loop;

      /* line loop */
      unsigned char *line_addr = repetition_addr;
      for (unsigned int x = 0; x < fheight; x++)
      {
        /* batch loop */
        unsigned char *batch_addr = line_addr;
        for (unsigned int y = 0; y < fwidth; y++)
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

int LL_ATON_LIB_Split(const LL_Buffer_InfoTypeDef *input, bool aton_canonical, const LL_Buffer_InfoTypeDef (*outputs)[],
                      uint32_t noutputs, uint32_t rank, uint32_t split_onnx_axis, uint32_t leading_dims, int split_case,
                      int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance)
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

      __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(input, (const LL_Buffer_InfoTypeDef *)outputs, noutputs,
                                                   leading_dims, dma_in, dma_out, nn_instance);
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

      __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(input, (const LL_Buffer_InfoTypeDef *)outputs, noutputs, dma_in,
                                                      dma_out, nn_instance);
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

      __LL_ATON_LIB_DMA_Outputs_Flat_Copy(input, (const LL_Buffer_InfoTypeDef *)outputs, noutputs, dma_in, dma_out,
                                          nn_instance);
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
