/**
 ******************************************************************************
 * @file    ll_sw_float.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Low Level Software library for Floating Point inference interfacing with EmbedNets(c)
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

#include "ll_aton_config.h"

#if LL_ATON_SW_FALLBACK == 1

#include <stdint.h>
#include <stdio.h>

#include "ll_sw.h"
#include "ll_sw_float.h"

#include "ai_datatypes_internal.h"
#include "ai_math_helpers.h"
#include "core_private.h"
#include "layers.h"
#include "ll_aton_util.h"

#define FORMAT AI_ARRAY_FORMAT_FLOAT

#define SHAPE_INIT(a_, b_, c_, d_) AI_SHAPE_INIT(4, (d_), (c_), (b_), (a_))

#define SHAPE_2D_INIT(h_, w_) AI_SHAPE_2D_INIT((w_), (h_))

#define STRIDE_INIT(a_, b_, c_, d_) AI_STRIDE_INIT(4, (d_), (c_), (b_), (a_))

#define TENSORS(...) __VA_ARGS__

/* Managing shape index to be consistent with the enum in embednets ai_platform.h */
static int helper_emit_shape_index_axis(int onnx_axis)
{
  switch (onnx_axis)
  {
  case 0:
    return AI_SHAPE_IN_CHANNEL;
  case 1:
    return AI_SHAPE_CHANNEL;
  case 2:
    return AI_SHAPE_HEIGHT; // note the switch required with case 3
  case 3:
    return AI_SHAPE_WIDTH; // note the switch required with case 2
  default:
#ifdef LL_SW_ENABLE_ASSERTS
    LL_ATON_ASSERT(0 && "axis index not supported");
#endif
    return -1;
  }
}

//##########################################################################################
/** Conv forward function */
void ll_sw_forward_conv(/* int processor, */ void *sw_info_struct)
{
  Conv_sw_info *sw_info = (Conv_sw_info *)sw_info_struct;
  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(conv_weights_array, FORMAT, sw_info->weights.mem.start_offset, sw_info->weights.mem.start_offset,
                       sw_info->weights.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(conv_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  AI_TENSOR_OBJ_DECLARE(conv_weights, , 0x0, 4,
                        SHAPE_INIT(sw_info->weights.dim.tensor_b, sw_info->weights.dim.tensor_h,
                                   sw_info->weights.dim.tensor_w, sw_info->weights.dim.tensor_c),
                        STRIDE_INIT(sw_info->weights.stride.h, sw_info->weights.stride.w, sw_info->weights.stride.b,
                                    sw_info->weights.stride.b),
                        1, &conv_weights_array, NULL);
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL);
  AI_TENSOR_OBJ_DECLARE(conv_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &conv_output_array, NULL);

  if (sw_info->bias.mem.start_offset != NULL)
  {
    AI_ARRAY_OBJ_DECLARE(conv_bias_array, FORMAT, sw_info->bias.mem.start_offset, sw_info->bias.mem.start_offset,
                         sw_info->bias.dim.num_elem, )
    AI_TENSOR_OBJ_DECLARE(
        conv_bias, , 0x0, 4,
        SHAPE_INIT(sw_info->bias.dim.tensor_h, sw_info->bias.dim.tensor_w, sw_info->bias.dim.tensor_c,
                   sw_info->bias.dim.tensor_b),
        STRIDE_INIT(sw_info->bias.stride.h, sw_info->bias.stride.w, sw_info->bias.stride.c, sw_info->bias.stride.b), 1,
        &conv_bias_array, NULL);

    // tensor chain initialization
    AI_TENSOR_CHAIN_OBJ_DECLARE(conv_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&conv_output)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&conv_weights, &conv_bias, NULL)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, TENSORS(NULL, NULL)))
    // layer initialization
    AI_LAYER_OBJ_DECLARE(conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_conv2d_if32of32wf32_group, &conv_chain,
                         NULL, NULL, , .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
                         .filter_stride = SHAPE_2D_INIT(sw_info->strides[0],
                                                        sw_info->strides[1]), // controlla perche' non mi convince
                         .filter_pad =
                             SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
                         .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  else
  {
    // tensor chain initialization
    AI_TENSOR_CHAIN_OBJ_DECLARE(conv_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&conv_output)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&conv_weights, NULL, NULL)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, TENSORS(NULL, NULL)))
    // layer initialization
    AI_LAYER_OBJ_DECLARE(conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_conv2d_if32of32wf32_group, &conv_chain,
                         NULL, NULL, , .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
                         .filter_stride = SHAPE_2D_INIT(sw_info->strides[0],
                                                        sw_info->strides[1]), // controlla perche' non mi convince
                         .filter_pad =
                             SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
                         .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
}

//##########################################################################################
/** GEMM forward function */
void ll_sw_forward_gemm(/* int processor, */ void *sw_info_struct)
{
  Gemm_sw_info *sw_info = (Gemm_sw_info *)sw_info_struct;
  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(gemm_operand_b_array, FORMAT, sw_info->operand_b.mem.start_offset,
                       sw_info->operand_b.mem.start_offset, sw_info->operand_b.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(gemm_operand_c_array, FORMAT, sw_info->operand_c.mem.start_offset,
                       sw_info->operand_c.mem.start_offset, sw_info->operand_c.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(gemm_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL);
  AI_TENSOR_OBJ_DECLARE(gemm_operand_b, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand_b.dim.tensor_h, sw_info->operand_b.dim.tensor_w,
                                   sw_info->operand_b.dim.tensor_c, sw_info->operand_b.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand_b.stride.h, sw_info->operand_b.stride.w,
                                    sw_info->operand_b.stride.c, sw_info->operand_b.stride.b),
                        1, &gemm_operand_b_array, NULL);
  AI_TENSOR_OBJ_DECLARE(gemm_operand_c, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand_c.dim.tensor_h, sw_info->operand_c.dim.tensor_w,
                                   sw_info->operand_c.dim.tensor_c, sw_info->operand_c.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand_c.stride.h, sw_info->operand_c.stride.w,
                                    sw_info->operand_c.stride.c, sw_info->operand_c.stride.b),
                        1, &gemm_operand_c_array, NULL);

  AI_TENSOR_OBJ_DECLARE(gemm_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &gemm_output_array, NULL);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(
      gemm_chain, , 4,
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&input_output, &gemm_operand_b, &gemm_operand_c)),
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&gemm_output)), AI_TENSOR_LIST_OBJ_EMPTY,
      AI_TENSOR_LIST_OBJ_EMPTY)
  // layer initialization
  AI_LAYER_OBJ_DECLARE(gemm_layer, 1, GEMM_TYPE, 0x0, NULL, gemm, forward_gemm, &gemm_chain, NULL, NULL, ,
                       .alpha = sw_info->alpha, .beta = sw_info->beta, .tA = sw_info->tA, .tB = sw_info->tB)
  gemm_layer.forward(AI_LAYER_OBJ(&gemm_layer));
}

//##########################################################################################
/** MatMul forward function */
void ll_sw_forward_matmul(/* int processor, */ void *sw_info_struct)
{
  Matmul_sw_info *sw_info = (Matmul_sw_info *)sw_info_struct;
  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(matmul_operand_b_array, FORMAT, sw_info->operand_b.mem.start_offset,
                       sw_info->operand_b.mem.start_offset, sw_info->operand_b.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(matmul_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL);
  AI_TENSOR_OBJ_DECLARE(matmul_operand_b, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand_b.dim.tensor_h, sw_info->operand_b.dim.tensor_w,
                                   sw_info->operand_b.dim.tensor_c, sw_info->operand_b.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand_b.stride.h, sw_info->operand_b.stride.w,
                                    sw_info->operand_b.stride.c, sw_info->operand_b.stride.b),
                        1, &matmul_operand_b_array, NULL);

  AI_TENSOR_OBJ_DECLARE(matmul_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &matmul_output_array, NULL);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(matmul_chain, , 4,
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, TENSORS(&input_output, &matmul_operand_b)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&matmul_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)
  // layer initialization
  AI_LAYER_OBJ_DECLARE(matmul_layer, 1, NL_TYPE, 0x0, NULL, nl, forward_matmul, &matmul_chain, NULL, NULL, , )
  matmul_layer.forward(AI_LAYER_OBJ(&matmul_layer));
}

//##########################################################################################
/** Resize forward function */
void ll_sw_forward_resize(/* int processor, */ void *sw_info_struct)
{
  Resize_sw_info *sw_info = (Resize_sw_info *)sw_info_struct;
  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(resize_scales_array, FORMAT, sw_info->scales.mem.start_offset, sw_info->scales.mem.start_offset,
                       sw_info->scales.dim.num_elem, )

  ai_array *resize_roi_array_ptr = NULL;
  ai_array resize_roi_array;
  if (sw_info->roi.mem.start_offset != NULL)
  {
    resize_roi_array = (ai_array)AI_ARRAY_OBJ_INIT(FORMAT, sw_info->roi.mem.start_offset, sw_info->roi.mem.start_offset,
                                                   sw_info->roi.dim.num_elem);
    resize_roi_array_ptr = &resize_roi_array;
  }

  AI_ARRAY_OBJ_DECLARE(resize_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL);
  AI_TENSOR_OBJ_DECLARE(resize_scales, , 0x0, 4,
                        SHAPE_INIT(sw_info->scales.dim.tensor_h, sw_info->scales.dim.tensor_w,
                                   sw_info->scales.dim.tensor_c, sw_info->scales.dim.tensor_b),
                        STRIDE_INIT(sw_info->scales.stride.h, sw_info->scales.stride.w, sw_info->scales.stride.c,
                                    sw_info->scales.stride.b),
                        1, &resize_scales_array, NULL);

  AI_TENSOR_OBJ_DECLARE(resize_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &resize_output_array, NULL);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(resize_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&resize_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  // extrapolating the scales values needed
  const ai_tensor *p = &resize_scales;
  const ai_float *s = AI_ARRAY_OBJ_DATA(p->data, ai_float);
  AI_ARRAY_OBJ_DECLARE_STATIC(rs, ai_float, AI_ARRAY_FORMAT_FLOAT, AI_CONST, 2, s[2], s[3])

  // layer initialization
  if ((ai_resize_mode)sw_info->mode == AI_RESIZE_ZEROS)
  {
    AI_LAYER_OBJ_DECLARE(resize_layer, 1, UPSAMPLE_TYPE, 0x0, NULL, upsample, forward_upsample_zeros, &resize_chain,
                         NULL, NULL, , .mode = AI_UPSAMPLE_ZEROS, .center = false, .scales = AI_ARRAY_OBJ(&rs),
                         .nearest_mode = AI_ROUND_PREFER_FLOOR)
    resize_layer.forward(AI_LAYER_OBJ(&resize_layer));
  }
  else
  {
    AI_LAYER_OBJ_DECLARE(resize_layer, 1, RESIZE_TYPE, 0x0, NULL, resize, forward_resize, &resize_chain, NULL, NULL, ,
                         .cubic_coeff_a = sw_info->cubic_coeff_a, .exclude_outside = sw_info->exclude_outside,
                         .extrapol_val = sw_info->extrapol_val, .mode = (ai_resize_mode)sw_info->mode,
                         .nearest_mode = (ai_nearest_mode)sw_info->nearest_mode,
                         .coord_transf_mode = (ai_coord_transf_mode)sw_info->coord_transf_mode,
                         .scales = AI_ARRAY_OBJ(&rs), .roi = resize_roi_array_ptr)
    resize_layer.forward(AI_LAYER_OBJ(&resize_layer));
  }
}

//##########################################################################################
/** Softmax forward function */
void ll_sw_forward_softmax(/* int processor, */ void *sw_info_struct)
{
  Softmax_sw_info *sw_info = (Softmax_sw_info *)sw_info_struct;

  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(softmax_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(sm_scratch0_array, AI_ARRAY_FORMAT_S32, sw_info->scratch.mem.start_offset,
                       sw_info->scratch.mem.start_offset, sw_info->scratch.dim.num_elem, )

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL);

  AI_TENSOR_OBJ_DECLARE(softmax_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &softmax_output_array, NULL);

  AI_TENSOR_OBJ_DECLARE(sm_scratch0, , 0x0, 4,
                        SHAPE_INIT(sw_info->scratch.dim.tensor_h, sw_info->scratch.dim.tensor_w,
                                   sw_info->scratch.dim.tensor_c, sw_info->scratch.dim.tensor_b),
                        STRIDE_INIT(sw_info->scratch.stride.h, sw_info->scratch.stride.w, sw_info->scratch.stride.c,
                                    sw_info->scratch.stride.b),
                        1, &sm_scratch0_array, NULL);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(softmax_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&softmax_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&sm_scratch0)))

  AI_LAYER_OBJ_DECLARE(sm_layer, 1, SM_TYPE, 0x0, NULL, sm, forward_sm, &softmax_chain, NULL, NULL, ,
                       .axis = helper_emit_shape_index_axis(sw_info->axis))
  sm_layer.forward(AI_LAYER_OBJ(&sm_layer));
}

//##########################################################################################
/** activ forward function  */
void ll_sw_forward_activ(/* int processor, */ void *sw_info_struct)
{
  Activ_sw_info *sw_info = (Activ_sw_info *)sw_info_struct;
  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(activ_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(activ_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &activ_output_array, NULL)
  // tensor chains init
  AI_TENSOR_CHAIN_OBJ_DECLARE(activ_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  switch (sw_info->general.type)
  {
  case LL_SW_RELU:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_relu, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_CLIP:
  {
    AI_ARRAY_OBJ_DECLARE_STATIC(clip_layer_params, ai_float, AI_ARRAY_FORMAT_FLOAT, AI_CONST, 2, sw_info->min,
                                sw_info->max)
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_clip, &activ_chain, NULL, NULL, ,
                         .nl_params = AI_ARRAY_OBJ(&clip_layer_params))
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_PRELU:
  {
    AI_ARRAY_OBJ_DECLARE(activ_operand_array, FORMAT, sw_info->operand.mem.start_offset,
                         sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )

    AI_TENSOR_OBJ_DECLARE(activ_operand, , 0x0, 4,
                          SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                     sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                          STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                      sw_info->operand.stride.b),
                          1, &activ_operand_array, NULL)

    // tensor chains init
    AI_TENSOR_CHAIN_OBJ_DECLARE(
        prelu_activ_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
        AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_output)),
        AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_operand)), AI_TENSOR_LIST_OBJ_EMPTY)

    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_prelu, &prelu_activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ELU:
  {
    // defining the array of params needed
    AI_ARRAY_OBJ_DECLARE_STATIC(elu_layer_params, ai_float, FORMAT, , 1, sw_info->alpha)

    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_elu, &activ_chain, NULL, NULL, ,
                         .nl_params = AI_ARRAY_OBJ(&elu_layer_params), )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_THRESHOLDEDRELU:
  {
    // defining the array of params needed
    AI_ARRAY_OBJ_DECLARE_STATIC(thresholdedrelu_layer_params, ai_float, FORMAT, , 1, sw_info->alpha)

    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_relu_thresholded, &activ_chain, NULL,
                         NULL, , .nl_params = AI_ARRAY_OBJ(&thresholdedrelu_layer_params), )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SELU:
  {
    // defining the array of params needed
    AI_ARRAY_OBJ_DECLARE_STATIC(selu_layer_params, ai_float, FORMAT, , 2, sw_info->alpha, sw_info->gamma)

    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_selu, &activ_chain, NULL, NULL, ,
                         .nl_params = AI_ARRAY_OBJ(&selu_layer_params), )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_CEIL:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_ceil, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_FLOOR:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_floor, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_EXP:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_exp, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_LOG:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_log, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ACOS:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_acos, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ACOSH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_acosh, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ASIN:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_asin, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ASINH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_asinh, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ATAN:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_atan, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ATANH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_atanh, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_COS:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_cos, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_COSH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_cosh, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SIN:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_sin, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SINH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_sinh, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_TANH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_tanh, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_TAN:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_tan, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SIGMOID:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_sigmoid, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SQRT:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_sqrt, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ERF:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_erf, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SOFTPLUS:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_soft_plus, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SOFTSIGN:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_soft_sign, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_HARDSIGMOID:
  {
    AI_ARRAY_OBJ_DECLARE_STATIC(hard_sigmoid_layer_params, ai_float, AI_ARRAY_FORMAT_FLOAT, AI_CONST, 2, sw_info->alpha,
                                sw_info->beta)
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_hard_sigmoid, &activ_chain, NULL, NULL,
                         , .nl_params = AI_ARRAY_OBJ(&hard_sigmoid_layer_params), )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_SWISH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_swish, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_HARDSWISH:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_hard_swish, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_HARDMAX:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_hardmax, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ABS:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_abs, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_NEG:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_neg, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_RECIPROCAL:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_reciprocal, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_ROUND:
  {
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_round, &activ_chain, NULL, NULL, ,
                         .nl_params = NULL, )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  case LL_SW_GELU:
  {
    AI_ARRAY_OBJ_DECLARE_STATIC(gelu_no_approximate_layer_params, ai_bool, AI_ARRAY_FORMAT_BOOL, AI_CONST, 1, false)
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_gelu, &activ_chain, NULL, NULL, ,
                         .nl_params = AI_ARRAY_OBJ(&gelu_no_approximate_layer_params), )
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  default:
#ifdef LL_SW_ENABLE_ASSERTS
    LL_ATON_ASSERT(0 && "NL OPERATION NOT SUPPORTED");
#endif
    break;
  }
}

//##########################################################################################
/** activ forward function  */
void ll_sw_forward_arith(/* int processor, */ void *sw_info_struct)
{
  Arith_sw_info *sw_info = (Arith_sw_info *)sw_info_struct;
  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(arith_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(arith_operand_array, FORMAT, sw_info->operand.mem.start_offset,
                       sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(arith_operand, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                   sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                    sw_info->operand.stride.b),
                        1, &arith_operand_array, NULL)
  AI_TENSOR_OBJ_DECLARE(arith_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &arith_output_array, NULL)

  // layer initialization
  func_binary op = ai_sum;                      /**< operation to apply elementwise */
  func_buffer_binary buffer_op = ai_sum_buffer; /**< operation to apply elementwise */

  switch (sw_info->general.type)
  {
  case LL_SW_ARITHADD:
  case LL_SW_ARITHSUM:
    op = ((func_binary)(ai_sum));
    buffer_op = ((func_buffer_binary)(ai_sum_buffer));
    break;
  case LL_SW_ARITHMUL:
    op = ((func_binary)(ai_mul));
    buffer_op = ((func_buffer_binary)(ai_mul_buffer));
    break;
  case LL_SW_ARITHDIV:
    op = ((func_binary)(ai_div));
    buffer_op = ((func_buffer_binary)(ai_div_buffer));
    break;
  case LL_SW_ARITHSUB:
    op = ((func_binary)(ai_sub));
    buffer_op = ((func_buffer_binary)(ai_sub_buffer));
    break;
  case LL_SW_POW:
    op = ((func_binary)(ai_pow));
    buffer_op = ((func_buffer_binary)(ai_pow_buffer));
    break;
  case LL_SW_GREATER:
    op = ((func_binary)(ai_greater));
    buffer_op = ((func_buffer_binary)(ai_greater_buffer));
    break;
  case LL_SW_GREATEROREQUAL:
    op = ((func_binary)(ai_greater_or_equal));
    buffer_op = ((func_buffer_binary)(ai_greater_or_equal_buffer));
    break;
  case LL_SW_LESS:
    op = ((func_binary)(ai_less));
    buffer_op = ((func_buffer_binary)(ai_less_buffer));
    break;
  case LL_SW_EQUAL:
    op = ((func_binary)(ai_equal));
    buffer_op = ((func_buffer_binary)(ai_equal_buffer));
    break;
  case LL_SW_LESSOREQUAL:
    op = ((func_binary)(ai_less_or_equal));
    buffer_op = ((func_buffer_binary)(ai_less_or_equal_buffer));
    break;
  case LL_SW_MIN:
    op = ((func_binary)(ai_min));
    buffer_op = ((func_buffer_binary)(ai_min_buffer));
    break;
  case LL_SW_MAX:
    op = ((func_binary)(ai_max));
    buffer_op = ((func_buffer_binary)(ai_max_buffer));
    break;
  case LL_SW_MOD:
    op = ((func_binary)(ai_floor_mod));
    buffer_op = ((func_buffer_binary)(ai_floor_mod_buffer));
    break;
  default:
#ifdef LL_SW_ENABLE_ASSERTS
    LL_ATON_ASSERT(0 && "ELEMENT WISE OPERATION NOT SUPPORTED");
#endif
    break;
  }

  if (sw_info->general.type == LL_SW_ARITHSUM && sw_info->num_of_inputs == 4)
  {
    AI_ARRAY_OBJ_DECLARE(arith_operand_array1, FORMAT, sw_info->operand1.mem.start_offset,
                         sw_info->operand1.mem.start_offset, sw_info->operand1.dim.num_elem, );
    AI_ARRAY_OBJ_DECLARE(arith_operand_array2, FORMAT, sw_info->operand2.mem.start_offset,
                         sw_info->operand2.mem.start_offset, sw_info->operand2.dim.num_elem, );
    AI_TENSOR_OBJ_DECLARE(arith_operand1, , 0x0, 4,
                          SHAPE_INIT(sw_info->operand1.dim.tensor_h, sw_info->operand1.dim.tensor_w,
                                     sw_info->operand1.dim.tensor_c, sw_info->operand1.dim.tensor_b),
                          STRIDE_INIT(sw_info->operand1.stride.h, sw_info->operand1.stride.w,
                                      sw_info->operand1.stride.c, sw_info->operand1.stride.b),
                          1, &arith_operand_array1, NULL);
    AI_TENSOR_OBJ_DECLARE(arith_operand2, , 0x0, 4,
                          SHAPE_INIT(sw_info->operand2.dim.tensor_h, sw_info->operand2.dim.tensor_w,
                                     sw_info->operand2.dim.tensor_c, sw_info->operand2.dim.tensor_b),
                          STRIDE_INIT(sw_info->operand2.stride.h, sw_info->operand2.stride.w,
                                      sw_info->operand2.stride.c, sw_info->operand2.stride.b),
                          1, &arith_operand_array2, NULL);

    // LL_ATON_PRINTF("\n\n\n #################### 4 inputs\n\n\n");
    AI_TENSOR_CHAIN_OBJ_DECLARE(
        arith_chain, , 4,
        AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 4,
                                TENSORS(&input_output, &arith_operand, &arith_operand1, &arith_operand2)),
        AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&arith_output)), AI_TENSOR_LIST_OBJ_EMPTY,
        AI_TENSOR_LIST_OBJ_EMPTY);
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_TYPE, 0x0, NULL, eltwise, forward_eltwise, &arith_chain, NULL, NULL,
                         , .operation = op, .buffer_operation = buffer_op)
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
  }
  else if (sw_info->general.type == LL_SW_ARITHSUM && sw_info->num_of_inputs == 3)
  {
    AI_ARRAY_OBJ_DECLARE(arith_operand_array1, FORMAT, sw_info->operand1.mem.start_offset,
                         sw_info->operand1.mem.start_offset, sw_info->operand1.dim.num_elem, );
    AI_TENSOR_OBJ_DECLARE(arith_operand1, , 0x0, 4,
                          SHAPE_INIT(sw_info->operand1.dim.tensor_h, sw_info->operand1.dim.tensor_w,
                                     sw_info->operand1.dim.tensor_c, sw_info->operand1.dim.tensor_b),
                          STRIDE_INIT(sw_info->operand1.stride.h, sw_info->operand1.stride.w,
                                      sw_info->operand1.stride.c, sw_info->operand1.stride.b),
                          1, &arith_operand_array1, NULL);
    // LL_ATON_PRINTF("\n\n\n #################### 3 inputs\n\n\n");
    AI_TENSOR_CHAIN_OBJ_DECLARE(
        arith_chain, , 4,
        AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&input_output, &arith_operand, &arith_operand1)),
        AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&arith_output)), AI_TENSOR_LIST_OBJ_EMPTY,
        AI_TENSOR_LIST_OBJ_EMPTY);
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_TYPE, 0x0, NULL, eltwise, forward_eltwise, &arith_chain, NULL, NULL,
                         , .operation = op, .buffer_operation = buffer_op)
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
  }
  else
  {
    // LL_ATON_PRINTF("\n\n\n #################### 2 inputs\n\n\n");
    AI_TENSOR_CHAIN_OBJ_DECLARE(arith_chain, , 4,
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, TENSORS(&input_output, &arith_operand)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&arith_output)),
                                AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY);
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_TYPE, 0x0, NULL, eltwise, forward_eltwise, &arith_chain, NULL, NULL,
                         , .operation = op, .buffer_operation = buffer_op)
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
  }
}

//##########################################################################################
/** pool forward function  */
void ll_sw_forward_pool(/* int processor, */ void *sw_info_struct)
{
  Pool_sw_info *sw_info = (Pool_sw_info *)sw_info_struct;
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(pool_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensor init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(pool_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &pool_output_array, NULL)
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(pool_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&pool_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY)

  node_func func = NULL;
  bool count_include_pad = 0;
  switch (sw_info->general.type)
  {
  case LL_SW_AVGPOOL:
    func = ((node_func)(forward_ap));
    count_include_pad = sw_info->count_include_pad;
    break;
  case LL_SW_MAXPOOL:
    func = ((node_func)(forward_mp));
    break;
  default:
    break;
  }

  AI_LAYER_OBJ_DECLARE(pool_layer, 1, POOL_TYPE, 0x0, NULL, pool, func, &pool_chain, NULL, NULL, ,
                       .pool_size = SHAPE_2D_INIT(sw_info->k_shape[0], sw_info->k_shape[1]),
                       .pool_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]),
                       .pool_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
                       .count_include_pad = count_include_pad, )
  pool_layer.forward(AI_LAYER_OBJ(&pool_layer));
}

//##########################################################################################
/** Globalpool forward function  */
void ll_sw_forward_global_pool(/* int processor, */ void *sw_info_struct)
{
  Global_pool_sw_info *sw_info = (Global_pool_sw_info *)sw_info_struct;
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(pool_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensor init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(pool_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &pool_output_array, NULL)
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(pool_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&pool_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY)

  node_func func = NULL;
  switch (sw_info->general.type)
  {
  case LL_SW_AVGPOOL:
    func = ((node_func)(forward_ap));
    break;
  case LL_SW_MAXPOOL:
    func = ((node_func)(forward_mp));
    break;
  default:
    break;
  }

  AI_LAYER_OBJ_DECLARE(pool_layer, 1, POOL_TYPE, 0x0, NULL, pool, func, &pool_chain, NULL, NULL, ,
                       .pool_size =
                           SHAPE_2D_INIT(sw_info->general.input.dim.tensor_w, sw_info->general.input.dim.tensor_w),
                       .pool_stride = SHAPE_2D_INIT(1, 1), .pool_pad = SHAPE_INIT(0, 0, 0, 0), )
  pool_layer.forward(AI_LAYER_OBJ(&pool_layer));
}

//##########################################################################################
/** ArgMax forward function  */
void ll_sw_forward_argmax(/* int processor, */ void *sw_info_struct)
{
  Argmax_sw_info *sw_info = (Argmax_sw_info *)sw_info_struct;
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(argmax_output_array, AI_ARRAY_FORMAT_S32, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensor init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(argmax_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &argmax_output_array, NULL)
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(argmax_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&argmax_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  AI_LAYER_OBJ_DECLARE(argmax_layer, 1, ARGMINMAX_TYPE, 0x0, NULL, argminmax, forward_argmax, &argmax_chain, NULL, NULL,
                       , .axis = helper_emit_shape_index_axis(sw_info->axis),
                       .select_last_index = sw_info->select_last_index, )
  argmax_layer.forward(AI_LAYER_OBJ(&argmax_layer));
}

//##########################################################################################
/** Gather forward function  */
void ll_sw_forward_gather(/* int processor, */ void *sw_info_struct)
{
  Gather_sw_info *sw_info = (Gather_sw_info *)sw_info_struct;
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(indexes_array, AI_ARRAY_FORMAT_S32, sw_info->operand.mem.start_offset,
                       sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(gather_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensor init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(indexes, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                   sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                    sw_info->operand.stride.b),
                        1, &indexes_array, NULL)
  AI_TENSOR_OBJ_DECLARE(gather_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &gather_output_array, NULL)
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(gather_chain, , 4,
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, TENSORS(&input_output), TENSORS(&indexes)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&gather_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)
  // LL_ATON_PRINTF("axis %d \n", sw_info->axis);
  // LL_ATON_PRINTF("index %d \n", *(int32_t *)(sw_info->operand.mem.start_offset));
  // LL_ATON_FFLUSH(stdout);
  AI_LAYER_OBJ_DECLARE(gather_layer, 1, GATHER_TYPE, 0x0, NULL, gather, forward_gather, &gather_chain, NULL, NULL, ,
                       .axis = helper_emit_shape_index_axis(sw_info->axis), )
  gather_layer.forward(AI_LAYER_OBJ(&gather_layer));
}

//##########################################################################################
/** ArgMin forward function  */
void ll_sw_forward_argmin(/* int processor, */ void *sw_info_struct)
{
  Argmin_sw_info *sw_info = (Argmin_sw_info *)sw_info_struct;
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(argmin_output_array, AI_ARRAY_FORMAT_S32, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensor init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(argmin_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &argmin_output_array, NULL)
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(argmin_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&argmin_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  AI_LAYER_OBJ_DECLARE(argmin_layer, 1, ARGMINMAX_TYPE, 0x0, NULL, argminmax, forward_argmin, &argmin_chain, NULL, NULL,
                       , .axis = helper_emit_shape_index_axis(sw_info->axis),
                       .select_last_index = sw_info->select_last_index, )
  argmin_layer.forward(AI_LAYER_OBJ(&argmin_layer));
}

//##########################################################################################
/** Reduce forward function  */
void ll_sw_forward_reduce(/* int processor, */ void *sw_info_struct)
{
  Reduce_sw_info *sw_info = (Reduce_sw_info *)sw_info_struct;
  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(reduce_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  // tensor init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(reduce_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &reduce_output_array, NULL)
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(reduce_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&reduce_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  if (sw_info->general.type == LL_SW_REDUCELOGSUMEXP)
  {
    AI_LAYER_OBJ_DECLARE(reduce_layer, 1, REDUCE_TYPE, 0x0, NULL, reduce_log_sum_exp, forward_reduce_log_sum_exp,
                         &reduce_chain, NULL, NULL, , .axis = helper_emit_shape_index_axis(sw_info->axis))
    reduce_layer.forward(AI_LAYER_OBJ(&reduce_layer));
  }
  else
  {
    // element wise function to be applied to perform the reduction
    func_binary func = NULL;
    // Initialization value for operation
    float value = 0.0f;

    switch (sw_info->general.type)
    {
    // case LL_SW_REDUCEMEAN:
    //     func = ai_mean
    //     break;
    case LL_SW_REDUCESUM:
      func = ai_sum;
      value = 0.0f;
      break;
    case LL_SW_REDUCEMIN:
      func = ai_min;
      value = AI_FLT_MAX;
      break;
    case LL_SW_REDUCEMAX:
      func = ai_max;
      value = 0.0f;
      break;
    case LL_SW_REDUCEPROD:
      func = ai_max;
      value = 0.0f;
      break;
    default:
#ifdef LL_SW_ENABLE_ASSERTS
      LL_ATON_ASSERT(0 && "REDUCTION OPERATIONS NOT SUPPORTED");
#endif
      break;
    }
    AI_ARRAY_OBJ_DECLARE_STATIC(value_f, ai_float, AI_ARRAY_FORMAT_FLOAT, AI_CONST, 1, value)

    AI_LAYER_OBJ_DECLARE(reduce_layer, 1, REDUCE_TYPE, 0x0, NULL, reduce, forward_reduce, &reduce_chain, NULL, NULL, ,
                         .neutral_value = (&value_f), .operation = func)

    reduce_layer.forward(AI_LAYER_OBJ(&reduce_layer));
  }
}

//##########################################################################################
/** Batch Norm forward function  */
void ll_sw_forward_bn(/* int processor, */ void *sw_info_struct)
{
  Bn_sw_info *sw_info = (Bn_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(bn_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(bn_bias_array, FORMAT, sw_info->bias.mem.start_offset, sw_info->bias.mem.start_offset,
                       sw_info->bias.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(bn_scale_array, FORMAT, sw_info->scale.mem.start_offset, sw_info->scale.mem.start_offset,
                       sw_info->scale.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(bn_mean_array, FORMAT, sw_info->mean.mem.start_offset, sw_info->mean.mem.start_offset,
                       sw_info->mean.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(bn_var_array, FORMAT, sw_info->var.mem.start_offset, sw_info->var.mem.start_offset,
                       sw_info->var.dim.num_elem, )

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(bn_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &bn_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(
      bn_bias, , 0x0, 4,
      SHAPE_INIT(sw_info->bias.dim.tensor_h, sw_info->bias.dim.tensor_w, sw_info->bias.dim.tensor_c,
                 sw_info->bias.dim.tensor_b),
      STRIDE_INIT(sw_info->bias.stride.h, sw_info->bias.stride.w, sw_info->bias.stride.c, sw_info->bias.stride.b), 1,
      &bn_bias_array, NULL);
  AI_TENSOR_OBJ_DECLARE(
      bn_scale, , 0x0, 4,
      SHAPE_INIT(sw_info->scale.dim.tensor_h, sw_info->scale.dim.tensor_w, sw_info->scale.dim.tensor_c,
                 sw_info->scale.dim.tensor_b),
      STRIDE_INIT(sw_info->scale.stride.h, sw_info->scale.stride.w, sw_info->scale.stride.c, sw_info->scale.stride.b),
      1, &bn_scale_array, NULL);
  AI_TENSOR_OBJ_DECLARE(
      bn_mean, , 0x0, 4,
      SHAPE_INIT(sw_info->mean.dim.tensor_h, sw_info->mean.dim.tensor_w, sw_info->mean.dim.tensor_c,
                 sw_info->mean.dim.tensor_b),
      STRIDE_INIT(sw_info->mean.stride.h, sw_info->mean.stride.w, sw_info->mean.stride.c, sw_info->mean.stride.b), 1,
      &bn_mean_array, NULL);
  AI_TENSOR_OBJ_DECLARE(
      bn_var, , 0x0, 4,
      SHAPE_INIT(sw_info->var.dim.tensor_h, sw_info->var.dim.tensor_w, sw_info->var.dim.tensor_c,
                 sw_info->var.dim.tensor_b),
      STRIDE_INIT(sw_info->var.stride.h, sw_info->var.stride.w, sw_info->var.stride.c, sw_info->var.stride.b), 1,
      &bn_var_array, NULL);

  const ai_tensor *t_in = &input_output;
  const ai_tensor *t_out = &bn_output;
  const ai_tensor *t_bias = &bn_bias;
  const ai_tensor *t_scale = &bn_scale;
  const ai_tensor *t_mean = &bn_mean;
  const ai_tensor *t_var = &bn_var;

  const ai_size n_elements = ai_shape_get_size(AI_TENSOR_SHAPE(t_in));
  const ai_size n_channel_in = AI_SHAPE_CH(AI_TENSOR_SHAPE(t_in));
  const ai_float *in_data = AI_ARRAY_OBJ_DATA(t_in->data, ai_float);
  ai_float *out_data = AI_ARRAY_OBJ_DATA(t_out->data, ai_float);

  AI_ASSERT(n_elements > 0 && n_channel_in > 0)
  AI_ASSERT(n_channel_in == AI_SHAPE_CH(AI_TENSOR_SHAPE(t_scale)))
  AI_ASSERT(!t_bias || n_channel_in == AI_SHAPE_CH(AI_TENSOR_SHAPE(t_bias)))

  const ai_float *scale = (const ai_float *)t_scale->data->data;
  const ai_float *bias = (const ai_float *)t_bias->data->data;
  const ai_float *mean = (const ai_float *)t_mean->data->data;
  const ai_float *var = (const ai_float *)t_var->data->data;
  const float epsilon = 0.00001f;
  for (ai_size i = 0; i < n_elements; i += n_channel_in)
  {
    for (ai_size ch = i; ch < i + n_channel_in; ++ch)
    {
      // LL_ATON_PRINTF("%f %f %f %f %f \n", in_data[ch], mean[ch-i], sqrtf(var[ch-i]), scale[ch-i], bias[ch-i]);
      out_data[ch] = ((in_data[ch] - mean[ch - i]) / (sqrtf(var[ch - i] + epsilon))) * scale[ch - i] + bias[ch - i];
    }
  }
}

//##########################################################################################
/** Instance Norm forward function  */
void ll_sw_forward_instance_normalization(/* int processor, */ void *sw_info_struct)
{
  Instance_normalization_sw_info *sw_info = (Instance_normalization_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(instanceNorm_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(instanceNorm_bias_array, FORMAT, sw_info->bias.mem.start_offset, sw_info->bias.mem.start_offset,
                       sw_info->bias.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(instanceNorm_scale_array, FORMAT, sw_info->scale.mem.start_offset,
                       sw_info->scale.mem.start_offset, sw_info->scale.dim.num_elem, )

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(instanceNorm_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &instanceNorm_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(
      instanceNorm_bias, , 0x0, 4,
      SHAPE_INIT(sw_info->bias.dim.tensor_h, sw_info->bias.dim.tensor_w, sw_info->bias.dim.tensor_c,
                 sw_info->bias.dim.tensor_b),
      STRIDE_INIT(sw_info->bias.stride.h, sw_info->bias.stride.w, sw_info->bias.stride.c, sw_info->bias.stride.b), 1,
      &instanceNorm_bias_array, NULL);
  AI_TENSOR_OBJ_DECLARE(
      instanceNorm_scale, , 0x0, 4,
      SHAPE_INIT(sw_info->scale.dim.tensor_h, sw_info->scale.dim.tensor_w, sw_info->scale.dim.tensor_c,
                 sw_info->scale.dim.tensor_b),
      STRIDE_INIT(sw_info->scale.stride.h, sw_info->scale.stride.w, sw_info->scale.stride.c, sw_info->scale.stride.b),
      1, &instanceNorm_scale_array, NULL);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(
      instanceNorm_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&instanceNorm_output)),
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&instanceNorm_scale, &instanceNorm_bias, NULL)),
      AI_TENSOR_LIST_OBJ_EMPTY, )

  // layer initialization
  AI_LAYER_OBJ_DECLARE(instanceNorm_layer, 1, INSTANCENORMALIZATION_TYPE, 0x0, NULL, instanceNormalization,
                       forward_instanceNormalization, &instanceNorm_chain, NULL, NULL, , .eps = 0.00001f, )
  instanceNorm_layer.forward(AI_LAYER_OBJ(&instanceNorm_layer));
}

//##########################################################################################
/** Lp Norm forward function  */
void ll_sw_forward_lpnormalization(/* int processor, */ void *sw_info_struct)
{
  Lpnormalization_sw_info *sw_info = (Lpnormalization_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(lpNormm_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(lpNormm_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &lpNormm_output_array, NULL)

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(lpNormm_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&lpNormm_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY, )

  // layer initialization
  AI_LAYER_OBJ_DECLARE(lpNormm_layer, 1, NORM_TYPE, 0x0, NULL, norm, forward_norm, &lpNormm_chain, NULL, NULL, ,
                       .axis = helper_emit_shape_index_axis(sw_info->axis), .exponent = sw_info->p, .scale = false)
  lpNormm_layer.forward(AI_LAYER_OBJ(&lpNormm_layer));
}

//##########################################################################################
/** Sign forward function  */
void ll_sw_forward_sign(/* int processor, */ void *sw_info_struct)
{
  Sign_sw_info *sw_info = (Sign_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(sign_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(sign_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &sign_output_array, NULL)

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(sign_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&sign_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY, )

  // layer initialization
  AI_LAYER_OBJ_DECLARE(sign_layer, 1, NL_TYPE, 0x0, NULL, nl, forward_sign, &sign_chain, NULL, NULL, , )
  sign_layer.forward(AI_LAYER_OBJ(&sign_layer));
}

//##########################################################################################
/** LRN Local Response Normalization forward function  */
void ll_sw_forward_lrn(/* int processor, */ void *sw_info_struct)
{
  Lrn_sw_info *sw_info = (Lrn_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(LRN_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(LRN_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &LRN_output_array, NULL)

  // tensor chains init
  AI_TENSOR_CHAIN_OBJ_DECLARE(LRN_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&LRN_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY)
  // TODO//layer initialization
  AI_LAYER_OBJ_DECLARE(LRN_layer, 2, LRN_TYPE, 0x0, NULL, lrn, forward_lrn, &LRN_chain, NULL, NULL, ,
                       .local_size = sw_info->size, .k = sw_info->bias, .alpha = sw_info->alpha, .beta = sw_info->beta)

  // LL_ATON_PRINTF("beta %f \n", sw_info->beta);
  // LL_ATON_PRINTF("bias %f \n", sw_info->bias);
  // LL_ATON_PRINTF("alpha %f \n", sw_info->alpha);
  // LL_ATON_PRINTF("local_size %d \n", sw_info->size);

  LRN_layer.forward(AI_LAYER_OBJ(&LRN_layer));
}

//##########################################################################################
/** Tile forward function  */
void ll_sw_forward_tile(/* int processor, */ void *sw_info_struct)
{
  Tile_sw_info *sw_info = (Tile_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(tile_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(tile_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &tile_output_array, NULL)

  // tensor chains init
  AI_TENSOR_CHAIN_OBJ_DECLARE(tile_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&tile_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY)

  AI_ARRAY_OBJ_DECLARE_STATIC(repeats, ai_i16, AI_ARRAY_FORMAT_S16, AI_CONST, 5, sw_info->repeats[2],
                              sw_info->repeats[3], sw_info->repeats[1], sw_info->repeats[0], 1)

  // TODO//layer initialization
  AI_LAYER_OBJ_DECLARE(Tile_layer, 2, TILE_TYPE, 0x0, NULL, tile, forward_tile, &tile_chain, NULL, NULL, ,
                       .repeats = AI_ARRAY_OBJ(&repeats))

  Tile_layer.forward(AI_LAYER_OBJ(&Tile_layer));
}

//##########################################################################################
/** concat forward function  */
void ll_sw_forward_concat(/* int processor, */ void *sw_info_struct)
{
  Concat_sw_info *sw_info = (Concat_sw_info *)sw_info_struct;

  AI_ARRAY_OBJ_DECLARE(input_output_array, FORMAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(concat_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(concat_operand_array, FORMAT, sw_info->operand.mem.start_offset,
                       sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )

  ai_array concat_operand_array1;
  ai_array *concat_operand_array1_ptr = NULL;
  if (sw_info->num_of_inputs >= 3)
  {
    concat_operand_array1 = (ai_array)AI_ARRAY_OBJ_INIT(
        FORMAT, sw_info->operand1.mem.start_offset, sw_info->operand1.mem.start_offset, sw_info->operand1.dim.num_elem);
    concat_operand_array1_ptr = &concat_operand_array1;
  }

  ai_array concat_operand_array2;
  ai_array *concat_operand_array2_ptr = NULL;
  if (sw_info->num_of_inputs >= 4)
  {
    concat_operand_array2 = (ai_array)AI_ARRAY_OBJ_INIT(
        FORMAT, sw_info->operand2.mem.start_offset, sw_info->operand2.mem.start_offset, sw_info->operand2.dim.num_elem);
    concat_operand_array2_ptr = &concat_operand_array2;
  }

  // tensors init
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL)
  AI_TENSOR_OBJ_DECLARE(concat_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &concat_output_array, NULL)

  AI_TENSOR_OBJ_DECLARE(concat_operand, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                   sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                    sw_info->operand.stride.b),
                        1, &concat_operand_array, NULL)

  ai_tensor concat_operand1;
  ai_tensor *concat_operand1_ptr = NULL;
  if (sw_info->num_of_inputs >= 3)
  {
    concat_operand1 =
        (ai_tensor)AI_TENSOR_OBJ_INIT(0x0, 4,
                                      SHAPE_INIT(sw_info->operand1.dim.tensor_h, sw_info->operand1.dim.tensor_w,
                                                 sw_info->operand1.dim.tensor_c, sw_info->operand1.dim.tensor_b),
                                      STRIDE_INIT(sw_info->operand1.stride.h, sw_info->operand1.stride.w,
                                                  sw_info->operand1.stride.c, sw_info->operand1.stride.b),
                                      1, concat_operand_array1_ptr, NULL);
    concat_operand1_ptr = &concat_operand1;
  }
  ai_tensor concat_operand2;
  ai_tensor *concat_operand2_ptr = NULL;
  if (sw_info->num_of_inputs >= 4)
  {
    concat_operand2 =
        (ai_tensor)AI_TENSOR_OBJ_INIT(0x0, 4,
                                      SHAPE_INIT(sw_info->operand2.dim.tensor_h, sw_info->operand2.dim.tensor_w,
                                                 sw_info->operand2.dim.tensor_c, sw_info->operand2.dim.tensor_b),
                                      STRIDE_INIT(sw_info->operand2.stride.h, sw_info->operand2.stride.w,
                                                  sw_info->operand2.stride.c, sw_info->operand2.stride.b),
                                      1, concat_operand_array2_ptr, NULL);
    concat_operand2_ptr = &concat_operand2;
  }

  // tensor chains init
  AI_TENSOR_CHAIN_OBJ_DECLARE(
      concat_chain, , 4,
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 4,
                              TENSORS(&input_output, &concat_operand, concat_operand1_ptr, concat_operand2_ptr)),
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&concat_output)), AI_TENSOR_LIST_OBJ_EMPTY,
      AI_TENSOR_LIST_OBJ_EMPTY)
  AI_LAYER_OBJ_DECLARE(concat_layer, 1, CONCAT_TYPE, 0x0, NULL, concat, forward_concat, &concat_chain, NULL, NULL, ,
                       .axis = sw_info->concat_axis)
  concat_layer.forward(AI_LAYER_OBJ(&concat_layer));
}

#endif // LL_ATON_SW_FALLBACK == 1
