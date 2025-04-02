/**
 ******************************************************************************
 * @file    ll_sw_integer.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   Low Level Software library for Scale-Offset Integer Format inference interfacing with EmbedNets(c)
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
#include "ll_sw_integer.h"

#include "ai_datatypes_internal.h"
#include "ai_math_helpers.h"
#include "core_convert.h"
#include "core_private.h"
#include "layers.h"
#include "ll_aton_util.h"

#define FORMAT AI_ARRAY_FORMAT_FLOAT

#define SHAPE_INIT(a_, b_, c_, d_) AI_SHAPE_INIT(4, (d_), (c_), (b_), (a_))

#define SHAPE_2D_INIT(h_, w_) AI_SHAPE_2D_INIT((w_), (h_))

#define STRIDE_INIT(a_, b_, c_, d_) AI_STRIDE_INIT(4, (d_), (c_), (b_), (a_))

#define TENSORS(...) __VA_ARGS__

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

/** QLinearMatMul forward function */
void ll_sw_forward_qlinearmatmul(/* int processor, */ void *sw_info_struct)
{
  Qlinearmatmul_sw_info *sw_info = (Qlinearmatmul_sw_info *)sw_info_struct;

  /*
  1. reshape of the weights from weights tensor -> weights_perm tensor
  2. map the matmul operation on the library dense_forward function.
          this requires 1d input (on ch) and 2d weights (ch e chin). in particular:
          - input: 1,1,NIN,1
          - weights already permuted: 1,1,NOUT,NIN
          - bias 1,1,NOUT,1
          - scratch 1,1,NIN*2+NOUT*10
          - output 1,1,NOUT,1
  */

  int32_t format;
  // array init
  format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                   : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  format = sw_info->weights.format.is_signed ? (AI_ARRAY_FORMAT_S8) : (AI_ARRAY_FORMAT_U8);
  AI_ARRAY_OBJ_DECLARE(dense_weights_array, format, sw_info->weights.mem.start_offset,
                       sw_info->weights.mem.start_offset, sw_info->weights.dim.num_elem, )
  format = sw_info->scratch.format.is_signed ? (AI_ARRAY_FORMAT_S8) : (AI_ARRAY_FORMAT_U8);
  AI_ARRAY_OBJ_DECLARE(dense_scratch0_array, format, sw_info->scratch.mem.start_offset,
                       sw_info->scratch.mem.start_offset, sw_info->scratch.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(dense_bias_array, AI_ARRAY_FORMAT_S32, sw_info->bias.mem.start_offset,
                       sw_info->bias.mem.start_offset, sw_info->bias.dim.num_elem, )
  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(dense_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format;
  uint16_t scale_format;

  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->izp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list input_intq = {.flags = (scale_format | offset_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->wzp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list weights_intq = {.flags = (scale_format | offset_format),
                                    .size = sw_info->ws.dim.num_elem,
                                    .info = (const ai_intq_info[1]){{
                                        .scale = ((float *)sw_info->ws.mem.start_offset),
                                        .zeropoint = ((void *)sw_info->wzp.mem.start_offset),
                                    }}};
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->ozp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list output_intq = {.flags = (scale_format | offset_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(dense_weights, , 0x0, 4,
                        SHAPE_INIT(sw_info->weights.dim.tensor_h, sw_info->weights.dim.tensor_w,
                                   sw_info->weights.dim.tensor_c, sw_info->weights.dim.tensor_b),
                        // ,0x0, 4, SHAPE_INIT(sw_info->weights.dim.tensor_b, sw_info->weights.dim.tensor_c,
                        // sw_info->weights.dim.tensor_h, sw_info->weights.dim.tensor_w),
                        STRIDE_INIT(sw_info->weights.stride.h, sw_info->weights.stride.w, sw_info->weights.stride.c,
                                    sw_info->weights.stride.b),
                        1, &dense_weights_array, &weights_intq);
  AI_TENSOR_OBJ_DECLARE(dense_scratch0, , 0x0, 4,
                        SHAPE_INIT(sw_info->scratch.dim.tensor_h, sw_info->scratch.dim.tensor_w,
                                   sw_info->scratch.dim.tensor_c, sw_info->scratch.dim.tensor_b),
                        STRIDE_INIT(sw_info->scratch.stride.h, sw_info->scratch.stride.w, sw_info->scratch.stride.c,
                                    sw_info->scratch.stride.b),
                        1, &dense_scratch0_array, NULL);
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);
  AI_TENSOR_OBJ_DECLARE(
      dense_bias, , 0x0, 4,
      SHAPE_INIT(sw_info->bias.dim.tensor_h, sw_info->bias.dim.tensor_c, sw_info->bias.dim.tensor_w,
                 sw_info->bias.dim.tensor_b),
      STRIDE_INIT(sw_info->bias.stride.h, sw_info->bias.stride.w, sw_info->bias.stride.c, sw_info->bias.stride.b), 1,
      &dense_bias_array, NULL);
  AI_TENSOR_OBJ_DECLARE(dense_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &dense_output_array, &output_intq);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(dense_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&dense_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&dense_weights, &dense_bias, NULL)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&dense_scratch0)))

  memset(sw_info->bias.mem.start_offset, 0x0, sw_info->bias.dim.num_elem * 4);

  // layer initialization
  AI_LAYER_OBJ_DECLARE(dense_layer, 1, DENSE_TYPE, 0x0, NULL, dense, forward_dense_integer /*_fixed*/, &dense_chain,
                       NULL, NULL, , )

  dense_layer.forward(AI_LAYER_OBJ(&dense_layer));
}

//##########################################################################################
/** QLinearMatMul forward function */
void ll_sw_forward_gemm_integer(/* int processor, */ void *sw_info_struct)
{
  Gemm_integer_sw_info *sw_info = (Gemm_integer_sw_info *)sw_info_struct;

  /*
  1. reshape of the weights from weights tensor -> weights_perm tensor
  2. map the matmul operation on the library dense_forward function.
          this requires 1d input (on ch) and 2d weights (ch e chin). in particular:
          - input: 1,1,NIN,1
          - weights already permuted: 1,1,NOUT,NIN
          - bias 1,1,NOUT,1
          - scratch 1,1,NIN*2+NOUT*10
          - output 1,1,NOUT,1
  */

  int32_t format;
  // array init
  format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                   : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  format = sw_info->weights.format.is_signed ? (AI_ARRAY_FORMAT_S8) : (AI_ARRAY_FORMAT_U8);
  AI_ARRAY_OBJ_DECLARE(dense_weights_array, format, sw_info->weights.mem.start_offset,
                       sw_info->weights.mem.start_offset, sw_info->weights.dim.num_elem, )
  format = sw_info->scratch.format.is_signed ? (AI_ARRAY_FORMAT_S8) : (AI_ARRAY_FORMAT_U8);
  AI_ARRAY_OBJ_DECLARE(dense_scratch0_array, format, sw_info->scratch.mem.start_offset,
                       sw_info->scratch.mem.start_offset, sw_info->scratch.dim.num_elem, )

  AI_ARRAY_OBJ_DECLARE(dense_bias_array, AI_ARRAY_FORMAT_S32, sw_info->bias.mem.start_offset,
                       sw_info->bias.mem.start_offset, sw_info->bias.dim.num_elem, )
  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(dense_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format;
  uint16_t scale_format;

  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->izp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list input_intq = {.flags = (scale_format | offset_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->wzp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list weights_intq = {.flags = (scale_format | offset_format),
                                    .size = sw_info->ws.dim.num_elem,
                                    .info = (const ai_intq_info[1]){{
                                        .scale = ((float *)sw_info->ws.mem.start_offset),
                                        .zeropoint = ((void *)sw_info->wzp.mem.start_offset),
                                    }}};
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->ozp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list output_intq = {.flags = (scale_format | offset_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(dense_weights, , 0x0, 4,
                        SHAPE_INIT(sw_info->weights.dim.tensor_h, sw_info->weights.dim.tensor_w,
                                   sw_info->weights.dim.tensor_c, sw_info->weights.dim.tensor_b),
                        STRIDE_INIT(sw_info->weights.stride.h, sw_info->weights.stride.w, sw_info->weights.stride.c,
                                    sw_info->weights.stride.b),
                        1, &dense_weights_array, &weights_intq);
  AI_TENSOR_OBJ_DECLARE(dense_scratch0, , 0x0, 4,
                        SHAPE_INIT(sw_info->scratch.dim.tensor_h, sw_info->scratch.dim.tensor_w,
                                   sw_info->scratch.dim.tensor_c, sw_info->scratch.dim.tensor_b),
                        STRIDE_INIT(sw_info->scratch.stride.h, sw_info->scratch.stride.w, sw_info->scratch.stride.c,
                                    sw_info->scratch.stride.b),
                        1, &dense_scratch0_array, NULL);
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);
  AI_TENSOR_OBJ_DECLARE(
      dense_bias, , 0x0, 4,
      SHAPE_INIT(sw_info->bias.dim.tensor_h, sw_info->bias.dim.tensor_c, sw_info->bias.dim.tensor_w,
                 sw_info->bias.dim.tensor_b),
      STRIDE_INIT(sw_info->bias.stride.h, sw_info->bias.stride.w, sw_info->bias.stride.c, sw_info->bias.stride.b), 1,
      &dense_bias_array, NULL);
  AI_TENSOR_OBJ_DECLARE(dense_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &dense_output_array, &output_intq);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(dense_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&dense_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&dense_weights, &dense_bias, NULL)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&dense_scratch0)))

  // layer initialization
  AI_LAYER_OBJ_DECLARE(dense_layer, 1, DENSE_TYPE, 0x0, NULL, dense, forward_dense_integer /*_fixed*/, &dense_chain,
                       NULL, NULL, , )

  dense_layer.forward(AI_LAYER_OBJ(&dense_layer));
}

//##########################################################################################
/** QuantizeLinear forward function */
void ll_sw_forward_quantizelinear(/* int processor, */ void *sw_info_struct)
{
  Quantizelinear_sw_info *sw_info = (Quantizelinear_sw_info *)sw_info_struct;

  // array init
  AI_ARRAY_OBJ_DECLARE(input_output_array, AI_ARRAY_FORMAT_FLOAT, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  int32_t format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                            : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(quantize_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->os.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, NULL);
  AI_TENSOR_OBJ_DECLARE(quantize_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &quantize_output_array, &output_intq);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(quantize_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&quantize_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  AI_LAYER_OBJ_DECLARE(quantize_layer, 1, NL_TYPE, 0x0, NULL, nl, node_convert /*_fixed*/, &quantize_chain, NULL,
                       NULL, )
  quantize_layer.forward(AI_LAYER_OBJ(&quantize_layer));
}

//##########################################################################################
/** Dequantizelinear forward function */
void ll_sw_forward_dequantizelinear(/* int processor, */ void *sw_info_struct)
{
  Dequantizelinear_sw_info *sw_info = (Dequantizelinear_sw_info *)sw_info_struct;

  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(dequantize_output_array, FORMAT, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->is.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(dequantize_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &dequantize_output_array, NULL);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(dequantize_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&dequantize_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  AI_LAYER_OBJ_DECLARE(dequantize_layer, 1, NL_TYPE, 0x0, NULL, nl, node_convert /*_fixed*/, &dequantize_chain, NULL,
                       NULL, )
  dequantize_layer.forward(AI_LAYER_OBJ(&dequantize_layer));
}

//##########################################################################################
/** QuantizeLinear forward function */
void ll_sw_forward_requantizelinear(/* int processor, */ void *sw_info_struct)
{
  Requantizelinear_sw_info *sw_info = (Requantizelinear_sw_info *)sw_info_struct;
  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(requantize_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->is.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->os.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(requantize_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &requantize_output_array, &output_intq);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(requantize_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&requantize_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  AI_LAYER_OBJ_DECLARE(requantize_layer, 1, NL_TYPE, 0x0, NULL, nl, node_convert_integer /*_fixed*/, &requantize_chain,
                       NULL, NULL, )
  requantize_layer.forward(AI_LAYER_OBJ(&requantize_layer));
}

//##########################################################################################
/** Conv integer forward function */
void ll_sw_forward_conv_integer(/* int processor, */ void *sw_info_struct)
{
  Conv_integer_sw_info *sw_info = (Conv_integer_sw_info *)sw_info_struct;

  int32_t format;
  // array init
  format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                   : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  format = sw_info->weights.format.is_signed ? (AI_ARRAY_FORMAT_S8) : (AI_ARRAY_FORMAT_U8);
  AI_ARRAY_OBJ_DECLARE(conv_weights_array, format, sw_info->weights.mem.start_offset, sw_info->weights.mem.start_offset,
                       sw_info->weights.dim.num_elem, )
  format = sw_info->scratch.format.is_signed ? (AI_ARRAY_FORMAT_S8) : (AI_ARRAY_FORMAT_U8);
  AI_ARRAY_OBJ_DECLARE(conv_scratch0_array, format, sw_info->scratch.mem.start_offset,
                       sw_info->scratch.mem.start_offset, sw_info->scratch.dim.num_elem, )
  AI_ARRAY_OBJ_DECLARE(conv_bias_array, AI_ARRAY_FORMAT_S32, sw_info->bias.mem.start_offset,
                       sw_info->bias.mem.start_offset, sw_info->bias.dim.num_elem, )
  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(conv_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format;
  uint16_t scale_format;

  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->izp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list input_intq = {.flags = (scale_format | offset_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->wzp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list weights_intq = {.flags = (scale_format | offset_format),
                                    .size = sw_info->ws.dim.num_elem,
                                    .info = (const ai_intq_info[1]){{
                                        .scale = ((float *)sw_info->ws.mem.start_offset),
                                        .zeropoint = ((void *)sw_info->wzp.mem.start_offset),
                                    }}};
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  offset_format =
      sw_info->ozp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  ai_intq_info_list output_intq = {.flags = (scale_format | offset_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(conv_weights, , 0x0, 4,
                        SHAPE_INIT(sw_info->weights.dim.tensor_b, sw_info->weights.dim.tensor_h,
                                   sw_info->weights.dim.tensor_w, sw_info->weights.dim.tensor_c),
                        STRIDE_INIT(sw_info->weights.stride.h, sw_info->weights.stride.w, sw_info->weights.stride.b,
                                    sw_info->weights.stride.b),
                        1, &conv_weights_array, &weights_intq);
  AI_TENSOR_OBJ_DECLARE(conv_scratch0, , 0x0, 4,
                        SHAPE_INIT(sw_info->scratch.dim.tensor_h, sw_info->scratch.dim.tensor_w,
                                   sw_info->scratch.dim.tensor_c, sw_info->scratch.dim.tensor_b),
                        STRIDE_INIT(sw_info->scratch.stride.h, sw_info->scratch.stride.w, sw_info->scratch.stride.c,
                                    sw_info->scratch.stride.b),
                        1, &conv_scratch0_array, NULL);
  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);
  AI_TENSOR_OBJ_DECLARE(
      conv_bias, , 0x0, 4,
      SHAPE_INIT(sw_info->bias.dim.tensor_h, sw_info->bias.dim.tensor_w, sw_info->bias.dim.tensor_c,
                 sw_info->bias.dim.tensor_b),
      STRIDE_INIT(sw_info->bias.stride.h, sw_info->bias.stride.w, sw_info->bias.stride.c, sw_info->bias.stride.b), 1,
      &conv_bias_array, NULL);
  AI_TENSOR_OBJ_DECLARE(conv_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &conv_output_array, &output_intq);
  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(conv_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&conv_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, TENSORS(&conv_weights, &conv_bias, NULL)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&conv_scratch0)))

  switch (sw_info->fwd_func)
  {
  case LL_SW_SSSA_PW_CONV:
  {
#ifdef LL_SW_DUMP_DEBUG
    LL_ATON_PRINTF("POINTWISE");
#endif
    AI_LAYER_OBJ_DECLARE(
        conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_pw_sssa8_ch /*_fixed*/, &conv_chain, NULL, NULL, ,
        .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
        .filter_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]), // controlla perche' non mi convince
        .filter_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
        .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  break;
  case LL_SW_SSSA_RGB_CONV:
  {
#ifdef LL_SW_DUMP_DEBUG
    LL_ATON_PRINTF("RGB");
#endif
    AI_LAYER_OBJ_DECLARE(
        conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_conv2d_rgb_sssa8_ch /*_fixed*/, &conv_chain, NULL, NULL,
        , .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
        .filter_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]), // controlla perche' non mi convince
        .filter_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
        .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  break;
  case LL_SW_SSSA_DW_CONV:
  {
#ifdef LL_SW_DUMP_DEBUG
    LL_ATON_PRINTF("GROUP");
#endif
    AI_LAYER_OBJ_DECLARE(
        conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_dw_dm_sssa8_ch /*_fixed*/, &conv_chain, NULL, NULL, ,
        .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
        .filter_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]), // controlla perche' non mi convince
        .filter_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
        .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  break;
  case LL_SW_SSSA_DILATED_CONV:
  {
#ifdef LL_SW_DUMP_DEBUG
    LL_ATON_PRINTF("DILATION");
#endif
    AI_LAYER_OBJ_DECLARE(
        conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_conv2d_dilated_sssa8_ch /*_fixed*/, &conv_chain, NULL,
        NULL, , .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
        .filter_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]), // controlla perche' non mi convince
        .filter_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
        .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  break;
  case LL_SW_SSSA_GENERIC_CONV:
  {
#ifdef LL_SW_DUMP_DEBUG
    LL_ATON_PRINTF("GENERAL CH");
#endif
    AI_LAYER_OBJ_DECLARE(
        conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_conv2d_sssa8_ch /*_fixed*/, &conv_chain, NULL, NULL, ,
        .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
        .filter_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]), // controlla perche' non mi convince
        .filter_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
        .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  break;
  case LL_SW_GENERIC_CONV:
  {
#ifdef LL_SW_DUMP_DEBUG
    LL_ATON_PRINTF("GENERAL UNSIGEND per CHANNEL");
#endif
    AI_LAYER_OBJ_DECLARE(
        conv_layer, 1, CONV2D_TYPE, 0x0, NULL, conv2d, forward_conv2d_integer /*_fixed*/, &conv_chain, NULL, NULL, ,
        .groups = sw_info->ngroup, .nl_params = NULL, .nl_func = NULL,
        .filter_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]), // controlla perche' non mi convince
        .filter_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
        .dilation = SHAPE_2D_INIT(sw_info->dilations[0], sw_info->dilations[1]), )
    conv_layer.forward(AI_LAYER_OBJ(&conv_layer));
  }
  break;
  default:
    break;
  }
}

//##########################################################################################
/** Element Wise forward function */
void ll_sw_forward_eltwise_integer(/* int processor, */ void *sw_info_struct)
{
  Eltwise_integer_sw_info *sw_info = (Eltwise_integer_sw_info *)sw_info_struct;

  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )
  format = sw_info->operand.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                             : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(eltwise_operand_array, format, sw_info->operand.mem.start_offset,
                       sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )
  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(eltwise_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->is.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->os.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  offset_format =
      sw_info->operand_s.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list operand_intq = {.flags = (offset_format | scale_format),
                                    .size = sw_info->operand_s.dim.num_elem,
                                    .info = (const ai_intq_info[1]){{
                                        .scale = ((float *)sw_info->operand_s.mem.start_offset),
                                        .zeropoint = ((void *)sw_info->operand_zp.mem.start_offset),
                                    }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(eltwise_operand, , 0x0, 4,
                        SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                   sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                        STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                    sw_info->operand.stride.b),
                        1, &eltwise_operand_array, &operand_intq);

  AI_TENSOR_OBJ_DECLARE(eltwise_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &eltwise_output_array, &output_intq);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(
      eltwise_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, TENSORS(&input_output), TENSORS(&eltwise_operand)),
      AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&eltwise_output)), AI_TENSOR_LIST_OBJ_EMPTY,
      AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  switch (sw_info->general.type)
  {
  case LL_SW_ARITHSUM:
  {
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_INTEGER_TYPE, 0x0, NULL, eltwise_integer, forward_eltwise_integer,
                         &eltwise_chain, NULL, NULL, , .operation = ai_sum,
                         .buffer_operation =
                             ((sw_info->general.input.format.is_signed) ? ai_sum_buffer_INT8 : ai_sum_buffer_UINT8))
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
  }
  break;
  case LL_SW_ARITHMUL:
  {
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_INTEGER_TYPE, 0x0, NULL, eltwise_integer, forward_eltwise_integer,
                         &eltwise_chain, NULL, NULL, , .operation = ai_mul,
                         .buffer_operation =
                             ((sw_info->general.input.format.is_signed) ? ai_mul_buffer_INT8 : ai_mul_buffer_UINT8))
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
    break;
  }
  case LL_SW_ARITHDIV:
  {
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_INTEGER_TYPE, 0x0, NULL, eltwise_integer, forward_eltwise_integer,
                         &eltwise_chain, NULL, NULL, , .operation = ai_div,
                         .buffer_operation =
                             ((sw_info->general.input.format.is_signed) ? ai_div_buffer_INT8 : ai_div_buffer_UINT8))
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
  }
  break;
  case LL_SW_ARITHSUB:
  {
    AI_LAYER_OBJ_DECLARE(eltwise_layer, 2, ELTWISE_INTEGER_TYPE, 0x0, NULL, eltwise_integer, forward_eltwise_integer,
                         &eltwise_chain, NULL, NULL, , .operation = ai_sub,
                         .buffer_operation =
                             ((sw_info->general.input.format.is_signed) ? ai_sub_buffer_INT8 : ai_sub_buffer_UINT8))
    eltwise_layer.forward(AI_LAYER_OBJ(&eltwise_layer));
  }
  break;
  default:
    break;
  }
}

//##########################################################################################
/** Pool forward function */
void ll_sw_forward_pool_integer(/* int processor, */ void *sw_info_struct)
{
  Pool_integer_sw_info *sw_info = (Pool_integer_sw_info *)sw_info_struct;

  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(pool_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->is.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->os.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(pool_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &pool_output_array, &output_intq);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(pool_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&pool_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  switch (sw_info->general.type)
  {
  case LL_SW_AVGPOOL:
  {
    AI_LAYER_OBJ_DECLARE(pool_layer, 1, POOL_TYPE, 0x0, NULL, pool, forward_ap_integer, &pool_chain, NULL, NULL, ,
                         .pool_size = SHAPE_2D_INIT(sw_info->k_shape[0], sw_info->k_shape[1]),
                         .pool_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]),
                         .pool_pad = SHAPE_INIT(sw_info->pads[0], sw_info->pads[1], sw_info->pads[2], sw_info->pads[3]),
                         .count_include_pad = sw_info->count_include_pad, )
    pool_layer.forward(AI_LAYER_OBJ(&pool_layer));
  }
  break;
  case LL_SW_MAXPOOL:
  {
    AI_LAYER_OBJ_DECLARE(pool_layer, 1, POOL_TYPE, 0x0, NULL, pool, forward_mp_integer, &pool_chain, NULL, NULL, ,
                         .pool_size = SHAPE_2D_INIT(sw_info->k_shape[0], sw_info->k_shape[1]),
                         .pool_stride = SHAPE_2D_INIT(sw_info->strides[0], sw_info->strides[1]),
                         .pool_pad = SHAPE_INIT(sw_info->pads[3], sw_info->pads[2], sw_info->pads[1], sw_info->pads[0]),
                         .count_include_pad = 0, )
    pool_layer.forward(AI_LAYER_OBJ(&pool_layer));
  }
  break;
  default:
    break;
  }
}

//##########################################################################################
/** Pool forward function */
void ll_sw_forward_global_pool_integer(/* int processor, */ void *sw_info_struct)
{
  Global_pool_integer_sw_info *sw_info = (Global_pool_integer_sw_info *)sw_info_struct;

  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(pool_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->is.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->os.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(pool_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &pool_output_array, &output_intq);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(pool_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&pool_output)), AI_TENSOR_LIST_OBJ_EMPTY,
                              AI_TENSOR_LIST_OBJ_EMPTY)

  // layer initialization
  switch (sw_info->general.type)
  {
  case LL_SW_AVGPOOL:
  {
    AI_LAYER_OBJ_DECLARE(pool_layer, 1, POOL_TYPE, 0x0, NULL, pool, forward_ap_integer, &pool_chain, NULL, NULL, ,
                         .pool_size =
                             SHAPE_2D_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w),
                         .pool_stride = SHAPE_2D_INIT(1, 1), .pool_pad = SHAPE_INIT(0, 0, 0, 0), )
    pool_layer.forward(AI_LAYER_OBJ(&pool_layer));
  }
  break;
  case LL_SW_MAXPOOL:
  {
    AI_LAYER_OBJ_DECLARE(pool_layer, 1, POOL_TYPE, 0x0, NULL, pool, forward_mp_integer, &pool_chain, NULL, NULL, ,
                         .pool_size =
                             SHAPE_2D_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w),
                         .pool_stride = SHAPE_2D_INIT(1, 1), .pool_pad = SHAPE_INIT(0, 0, 0, 0), )
    pool_layer.forward(AI_LAYER_OBJ(&pool_layer));
  }
  break;
  default:
    break;
  }
}

//##########################################################################################
/** Softmax forward function */
void ll_sw_forward_softmax_integer(/* int processor, */ void *sw_info_struct)
{
  Softmax_integer_sw_info *sw_info = (Softmax_integer_sw_info *)sw_info_struct;

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

  uint16_t offset_format =
      sw_info->izp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->ozp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(softmax_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &softmax_output_array, &output_intq);

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

  AI_ARRAY_OBJ_DECLARE_STATIC(sm_integer_params, ai_i32, AI_ARRAY_FORMAT_S32, AI_CONST, 3,
                              sw_info->quantized_multiplier, sw_info->left_shift, sw_info->diff_min)

  AI_LAYER_OBJ_DECLARE(sm_integer_layer, 1, SM_TYPE, 0x0, NULL, sm, forward_sm_integer, &softmax_chain, NULL, NULL, ,
                       .nl_params = &sm_integer_params, .axis = helper_emit_shape_index_axis(sw_info->axis))
  sm_integer_layer.forward(AI_LAYER_OBJ(&sm_integer_layer));
}

//##########################################################################################
/** Activ forward function */
void ll_sw_forward_activ_integer(/* int processor, */ void *sw_info_struct)
{
  Activ_integer_sw_info *sw_info = (Activ_integer_sw_info *)sw_info_struct;

  // array init
  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(activ_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->izp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->ozp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(activ_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &activ_output_array, &output_intq);

  // layer initialization
  switch (sw_info->general.type)
  {
  case LL_SW_RELU:
  {
    // tensor chain initialization
    AI_TENSOR_CHAIN_OBJ_DECLARE(activ_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_output)),
                                AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)
    AI_ARRAY_OBJ_DECLARE_STATIC(relu_params, ai_u8, AI_ARRAY_FORMAT_U8, AI_CONST, 1, sw_info->ozp.mem.start_offset[0])
    AI_LAYER_OBJ_DECLARE(activ_layer, 1, NL_TYPE, 0x0, NULL, nl, forward_relu_integer, &activ_chain, NULL, NULL, ,
                         .nl_params = AI_ARRAY_OBJ(&relu_params), )
    activ_layer.forward(AI_LAYER_OBJ(&activ_layer));
  }
  break;
  case LL_SW_PRELU:
  {
    if (sw_info->operand_s.mem.start_offset != NULL)
    {
      AI_ARRAY_OBJ_DECLARE(slope_tensor_array, format, sw_info->operand.mem.start_offset,
                           sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )
      offset_format =
          sw_info->operand.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
      scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
      ai_intq_info_list slope_intq = {.flags = (offset_format | scale_format),
                                      .size = sw_info->operand_s.dim.num_elem,
                                      .info = (const ai_intq_info[1]){{
                                          .scale = ((float *)sw_info->operand_s.mem.start_offset),
                                          .zeropoint = ((void *)sw_info->operand_zp.mem.start_offset),
                                      }}};

      AI_TENSOR_OBJ_DECLARE(slope_tensor, , 0x0, 4,
                            SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                       sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                            STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                        sw_info->operand.stride.b),
                            1, &slope_tensor_array, &slope_intq);
      // tensor chain initialization
      AI_TENSOR_CHAIN_OBJ_DECLARE(activ_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                                  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_output)),
                                  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&slope_tensor)),
                                  AI_TENSOR_LIST_OBJ_EMPTY)
      AI_ARRAY_OBJ_DECLARE_STATIC(relu_params, ai_u8, AI_ARRAY_FORMAT_U8, AI_CONST, 1, sw_info->ozp.mem.start_offset[0])
      AI_LAYER_OBJ_DECLARE(activ_layer, 1, NL_TYPE, 0x0, NULL, nl, forward_prelu_integer, &activ_chain, NULL, NULL, ,
                           .nl_params = AI_ARRAY_OBJ(&relu_params), )
      activ_layer.forward(AI_LAYER_OBJ(&activ_layer));
    }
    else
    {
      AI_ARRAY_OBJ_DECLARE(slope_tensor_array, FORMAT, sw_info->operand.mem.start_offset,
                           sw_info->operand.mem.start_offset, sw_info->operand.dim.num_elem, )
      AI_TENSOR_OBJ_DECLARE(slope_tensor, , 0x0, 4,
                            SHAPE_INIT(sw_info->operand.dim.tensor_h, sw_info->operand.dim.tensor_w,
                                       sw_info->operand.dim.tensor_c, sw_info->operand.dim.tensor_b),
                            STRIDE_INIT(sw_info->operand.stride.h, sw_info->operand.stride.w, sw_info->operand.stride.c,
                                        sw_info->operand.stride.b),
                            1, &slope_tensor_array, NULL);
      // tensor chain initialization
      AI_TENSOR_CHAIN_OBJ_DECLARE(activ_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                                  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_output)),
                                  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&slope_tensor)),
                                  AI_TENSOR_LIST_OBJ_EMPTY)
      AI_ARRAY_OBJ_DECLARE_STATIC(relu_params, ai_u8, AI_ARRAY_FORMAT_U8, AI_CONST, 1, sw_info->ozp.mem.start_offset[0])
      AI_LAYER_OBJ_DECLARE(activ_layer, 1, NL_TYPE, 0x0, NULL, nl, forward_prelu_integer, &activ_chain, NULL, NULL, ,
                           .nl_params = AI_ARRAY_OBJ(&relu_params), )
      activ_layer.forward(AI_LAYER_OBJ(&activ_layer));
    }
  }
  break;
  case LL_SW_CLIP:
  {
    // tensor chain initialization
    AI_TENSOR_CHAIN_OBJ_DECLARE(activ_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                                AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&activ_output)),
                                AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)
    AI_ARRAY_OBJ_DECLARE_STATIC(clip_layer_params, ai_float, AI_ARRAY_FORMAT_FLOAT, AI_CONST, 2, sw_info->min,
                                sw_info->max)
    AI_LAYER_OBJ_DECLARE(nonlinearity_layer, 2, NL_TYPE, 0x0, NULL, nl, forward_clip, &activ_chain, NULL, NULL, ,
                         .nl_params = AI_ARRAY_OBJ(&clip_layer_params))
    nonlinearity_layer.forward(AI_LAYER_OBJ(&nonlinearity_layer));
  }
  break;
  default:
    break;
  }
}

//##########################################################################################
/** Resize forward function */
void ll_sw_forward_resize_integer(/* int processor, */ void *sw_info_struct)
{
  Resize_integer_sw_info *sw_info = (Resize_integer_sw_info *)sw_info_struct;
  // array init

  int32_t format = sw_info->general.input.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                           : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);

  AI_ARRAY_OBJ_DECLARE(input_output_array, format, sw_info->general.input.mem.start_offset,
                       sw_info->general.input.mem.start_offset, sw_info->general.input.dim.num_elem, )

  format = sw_info->general.output.format.is_signed ? (AI_ARRAY_FORMAT_S8 | AI_FMT_FLAG_IS_IO)
                                                    : (AI_ARRAY_FORMAT_U8 | AI_FMT_FLAG_IS_IO);
  AI_ARRAY_OBJ_DECLARE(resize_output_array, format, sw_info->general.output.mem.start_offset,
                       sw_info->general.output.mem.start_offset, sw_info->general.output.dim.num_elem, )

  uint16_t offset_format =
      sw_info->izp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  uint16_t scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list input_intq = {.flags = (offset_format | scale_format),
                                  .size = sw_info->is.dim.num_elem,
                                  .info = (const ai_intq_info[1]){{
                                      .scale = ((float *)sw_info->is.mem.start_offset),
                                      .zeropoint = ((void *)sw_info->izp.mem.start_offset),
                                  }}};

  offset_format =
      sw_info->ozp.format.is_signed ? (AI_BUFFER_META_FLAG_ZEROPOINT_S8) : (AI_BUFFER_META_FLAG_ZEROPOINT_U8);
  scale_format = AI_BUFFER_META_FLAG_SCALE_FLOAT;
  ai_intq_info_list output_intq = {.flags = (offset_format | scale_format),
                                   .size = sw_info->os.dim.num_elem,
                                   .info = (const ai_intq_info[1]){{
                                       .scale = ((float *)sw_info->os.mem.start_offset),
                                       .zeropoint = ((void *)sw_info->ozp.mem.start_offset),
                                   }}};

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

  AI_TENSOR_OBJ_DECLARE(input_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.input.dim.tensor_h, sw_info->general.input.dim.tensor_w,
                                   sw_info->general.input.dim.tensor_c, sw_info->general.input.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.input.stride.h, sw_info->general.input.stride.w,
                                    sw_info->general.input.stride.c, sw_info->general.input.stride.b),
                        1, &input_output_array, &input_intq);

  AI_TENSOR_OBJ_DECLARE(resize_output, , 0x0, 4,
                        SHAPE_INIT(sw_info->general.output.dim.tensor_h, sw_info->general.output.dim.tensor_w,
                                   sw_info->general.output.dim.tensor_c, sw_info->general.output.dim.tensor_b),
                        STRIDE_INIT(sw_info->general.output.stride.h, sw_info->general.output.stride.w,
                                    sw_info->general.output.stride.c, sw_info->general.output.stride.b),
                        1, &resize_output_array, &output_intq);

  AI_TENSOR_OBJ_DECLARE(resize_scales, , 0x0, 4,
                        SHAPE_INIT(sw_info->scales.dim.tensor_h, sw_info->scales.dim.tensor_w,
                                   sw_info->scales.dim.tensor_c, sw_info->scales.dim.tensor_b),
                        STRIDE_INIT(sw_info->scales.stride.h, sw_info->scales.stride.w, sw_info->scales.stride.c,
                                    sw_info->scales.stride.b),
                        1, &resize_scales_array, NULL);

  // tensor chain initialization
  AI_TENSOR_CHAIN_OBJ_DECLARE(resize_chain, , 4, AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&input_output)),
                              AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, TENSORS(&resize_output)),
                              AI_TENSOR_LIST_OBJ_EMPTY, AI_TENSOR_LIST_OBJ_EMPTY)

  // extrapolating the scales values needed
  const ai_tensor *p = &resize_scales;
  const ai_float *s = AI_ARRAY_OBJ_DATA(p->data, ai_float);
  AI_ARRAY_OBJ_DECLARE_STATIC(rs, ai_float, AI_ARRAY_FORMAT_FLOAT, AI_CONST, 2, s[2], s[3])

  // layer initialization
  if ((ai_resize_mode)sw_info->mode == AI_RESIZE_NEAREST)
  {
    AI_LAYER_OBJ_DECLARE(resize_layer, 1, RESIZE_TYPE, 0x0, NULL, resize, forward_resize_nearest_is8os8, &resize_chain,
                         NULL, NULL, , .cubic_coeff_a = sw_info->cubic_coeff_a,
                         .exclude_outside = sw_info->exclude_outside, .extrapol_val = sw_info->extrapol_val,
                         .mode = (ai_resize_mode)sw_info->mode, .nearest_mode = (ai_nearest_mode)sw_info->nearest_mode,
                         .coord_transf_mode = (ai_coord_transf_mode)sw_info->coord_transf_mode,
                         .scales = AI_ARRAY_OBJ(&rs), .roi = resize_roi_array_ptr)
    resize_layer.forward(AI_LAYER_OBJ(&resize_layer));
  }
  else if ((ai_resize_mode)sw_info->mode == AI_RESIZE_LINEAR)
  {
    AI_LAYER_OBJ_DECLARE(resize_layer, 1, RESIZE_TYPE, 0x0, NULL, resize, forward_resize_bilinear_is8os8, &resize_chain,
                         NULL, NULL, , .cubic_coeff_a = sw_info->cubic_coeff_a,
                         .exclude_outside = sw_info->exclude_outside, .extrapol_val = sw_info->extrapol_val,
                         .mode = (ai_resize_mode)sw_info->mode, .nearest_mode = (ai_nearest_mode)sw_info->nearest_mode,
                         .coord_transf_mode = (ai_coord_transf_mode)sw_info->coord_transf_mode,
                         .scales = AI_ARRAY_OBJ(&rs), .roi = resize_roi_array_ptr)
    resize_layer.forward(AI_LAYER_OBJ(&resize_layer));
  }
  else if ((ai_resize_mode)sw_info->mode == AI_RESIZE_ZEROS)
  {
    AI_LAYER_OBJ_DECLARE(resize_layer, 1, UPSAMPLE_TYPE, 0x0, NULL, upsample, forward_upsample_zeros, &resize_chain,
                         NULL, NULL, , .mode = AI_UPSAMPLE_ZEROS, .center = false, .scales = AI_ARRAY_OBJ(&rs),
                         .nearest_mode = AI_ROUND_PREFER_FLOOR)
    resize_layer.forward(AI_LAYER_OBJ(&resize_layer));
  }
}

#endif // LL_ATON_SW_FALLBACK == 1
