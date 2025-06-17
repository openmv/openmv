#ifndef AI_MATH_HELPERS_H
#define AI_MATH_HELPERS_H
/**
  ******************************************************************************
  * @file    ai_math_helpers.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   Math helpers routines header file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "ai_lite_math_helpers.h"

//#if defined(HAS_X86) || defined(__CC_ARM) || defined(CM4) || defined(CM7)
#define _AI_CONV_2D_LOOP_UNROLLING_OPTIM
//#endif

#define STM32_DOT_INLINE_OPTIM

/* Modes for element wise integer optimized implementation */
#define AI_ELTWISE_NO_SCALAR    (0)
#define AI_ELTWISE_SCALAR1      (1)
#define AI_ELTWISE_SCALAR2      (2)
#define AI_ELTWISE_SCALAR_CH1   (3)
#define AI_ELTWISE_SCALAR_CH2   (4)


AI_API_DECLARE_BEGIN

#if defined(STM32_DOT_INLINE_OPTIM)

AI_DECLARE_STATIC
void __ai_math_dot_array(
        ai_float* out,
        const ai_float* data0,
        const ai_float* data1,
        ai_size data_size)
{
  ai_register ai_float sum = 0.0f;   /* Temporary result storage */

  /* Run the below code for Cortex-M4 and Cortex-M3 */

#if defined(_AI_CONV_2D_LOOP_UNROLLING_OPTIM)
  /* First part of the processing with loop unrolling.  Compute 16 outputs at a time.
   ** a second loop below computes the remaining 1 to 15 samples. */
  while (data_size >= 16u) {
    ai_register ai_vec4_float ch_in_f = AI_VEC4_FLOAT(data1);
    ai_register ai_vec4_float weights_in_f = AI_VEC4_FLOAT(data0);
    sum += weights_in_f.a1 * ch_in_f.a1;
    sum += weights_in_f.a2 * ch_in_f.a2;
    sum += weights_in_f.a3 * ch_in_f.a3;
    sum += weights_in_f.a4 * ch_in_f.a4;
    data1 += 4;
    data0 += 4;

    ch_in_f = AI_VEC4_FLOAT(data1);
    weights_in_f = AI_VEC4_FLOAT(data0);
    sum += weights_in_f.a1 * ch_in_f.a1;
    sum += weights_in_f.a2 * ch_in_f.a2;
    sum += weights_in_f.a3 * ch_in_f.a3;
    sum += weights_in_f.a4 * ch_in_f.a4;
    data1 += 4;
    data0 += 4;

    ch_in_f = AI_VEC4_FLOAT(data1);
    weights_in_f = AI_VEC4_FLOAT(data0);
    sum += weights_in_f.a1 * ch_in_f.a1;
    sum += weights_in_f.a2 * ch_in_f.a2;
    sum += weights_in_f.a3 * ch_in_f.a3;
    sum += weights_in_f.a4 * ch_in_f.a4;
    data1 += 4;
    data0 += 4;

    ch_in_f = AI_VEC4_FLOAT(data1);
    weights_in_f = AI_VEC4_FLOAT(data0);
    sum += weights_in_f.a1 * ch_in_f.a1;
    sum += weights_in_f.a2 * ch_in_f.a2;
    sum += weights_in_f.a3 * ch_in_f.a3;
    sum += weights_in_f.a4 * ch_in_f.a4;
    data1 += 4;
    data0 += 4;
    data_size -= 16u;
  }

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (data_size >= 4u) {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer */
    sum += (*data0++) * (*data1++);
    sum += (*data0++) * (*data1++);
    sum += (*data0++) * (*data1++);
    sum += (*data0++) * (*data1++);

    /* Decrement the loop counter */
    data_size -= 4u;
  }
#endif
  while (data_size > 0u) {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer. */
    sum += (*data0++) * (*data1++);

    /* Decrement the loop counter */
    data_size--;
  }

  /* Directly accumulate the result back in the destination buffer */
  *out += sum;
}


#undef AI_MATH_DOT_ARRAY
#define AI_MATH_DOT_ARRAY(dst, src0, src1, size) \
  { __ai_math_dot_array(dst, src0, src1, size); }

#else /* STM32_DOT_INLINE_OPTIM */

#undef AI_MATH_DOT_ARRAY
#define AI_MATH_DOT_ARRAY(dst, src0, src1, size) \
  { ai_math_dot_array(dst, src0, src1, size); }

#endif


/*!
 * @defgroup math_helpers Math helpers
 * @brief Common math functions
 *
 * Math functions are mapped to the underlying platform through those utility
 * functions. On x86 and ARM v7 they are mapped to the float math functions in
 * the C99 standard library; on MCUs they are mapped to the ARM DSP functions.
 */

/*!
 * @brief platform optimized dot product of float vectors
 *
 * Computes the dot product between vectors and adds the result to out.
 * @ingroup math_helpers
 * @param out scalar result of the dot product
 * @param data0 the first float vector
 * @param data1 the second float vector
 * @param data_size the size of both vectors
 */
AI_INTERFACE_ENTRY
void ai_math_dot_array(
        ai_float* out,
        const ai_float* data0,
        const ai_float* data1,
        const ai_size data_size);
/*!
 * @brief ErfInv a float value
 * @ingroup math_helpers
 * @param x input value
 * @return square root of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_erfinv(const ai_float x);

/*!
 * @brief platform optimized exponential on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return exponential of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_exp(const ai_float x);

/*!
 * @brief platform logical not
 * @ingroup math_helpers
 * @param x input value
 * @return not of the value
 */
AI_INTERFACE_ENTRY ai_bool ai_logical_not(const ai_bool x);

/*!
 * @brief platform optimized pow on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param e input value
 * @return pow of the value ^ e
 */
AI_INTERFACE_ENTRY ai_float ai_math_pow(const ai_float x, const ai_float e);

/*!
 * @brief platform optimized tangent on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return hyperbolic tangent of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_tanh(const ai_float x);

/*!
 * @brief platform optimized relu on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return relu of the value ( x if x>0 else 0)
 */
AI_INTERFACE_ENTRY ai_float ai_math_relu(const ai_float x);

/*!
 * @brief platform optimized parametric relu on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param slope input value
 * @return parametric relu of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_prelu(const ai_float x, const ai_float slope);

/*!
 * @brief platform optimized parametric sigmoid on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return sigmoid of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_sigmoid(const ai_float x);

/*!
 * @brief platform optimized parametric hard sigmoid on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return hard sigmoid of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_hard_sigmoid(const ai_float x); // const ai_float alpha, const ai_float beta);

/*!
 * @brief platform optimized parametric swish on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return swish of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_swish(const ai_float x);

/*!
 * @brief platform optimized parametric hard_swish on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return hard_swish of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_hard_swish(const ai_float x);

/*!
 * @brief platform optimized parametric gelu on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param aaproximate input value
 * @return gelu of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_gelu(const ai_float x, const ai_bool approximate);

/*!
 * @brief platform optimized parametric sign function on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return sign of the value
 */
AI_INTERFACE_ENTRY ai_float ai_math_sign(const ai_float x);

/*!
 * @brief optimized parametric rectified linear unit on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param slope parameter value
 * @return x if x is positive and x*slope otherwise
 */
AI_INTERFACE_ENTRY ai_float ai_fast_prelu(const ai_float x, const ai_float slope);

AI_INTERFACE_ENTRY void ai_div(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_div_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_div_buffer_INT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                           const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                           const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);
AI_INTERFACE_ENTRY void ai_div_buffer_UINT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                            const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                            const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);

AI_INTERFACE_ENTRY void ai_bitshift_right(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_right_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_right_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_right_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_right_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_right_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_right_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_right_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_left(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_left_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_left_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_left_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_left_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_left_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_bitshift_left_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_bitshift_left_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_floor_div(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_floor_div_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_floor_mod(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_floor_mod_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_mod(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mod_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mod_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_max(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_max_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_max_buffer_INT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                           const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                           const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);
AI_INTERFACE_ENTRY void ai_max_buffer_UINT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                            const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                            const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);

AI_INTERFACE_ENTRY void ai_min(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_min_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_min_buffer_INT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                           const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                           const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);
AI_INTERFACE_ENTRY void ai_min_buffer_UINT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                            const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                            const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);

AI_INTERFACE_ENTRY void ai_mul(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_mul_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_mul_buffer_INT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                           const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                           const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);
AI_INTERFACE_ENTRY void ai_mul_buffer_UINT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                            const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                            const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);

AI_INTERFACE_ENTRY void ai_pow(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_pow_buffer(ai_handle out, const ai_handle b, const ai_handle e, const ai_size loop);

AI_INTERFACE_ENTRY void ai_sub(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sub_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sub_buffer_INT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                           const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                           const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);
AI_INTERFACE_ENTRY void ai_sub_buffer_UINT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                            const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                            const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);

AI_INTERFACE_ENTRY void ai_sum(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_sum_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_sum_buffer_INT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                           const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                           const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);
AI_INTERFACE_ENTRY void ai_sum_buffer_UINT8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop,
                                            const ai_handle pScale1, const ai_handle pZp1, const ai_handle pScale2, const ai_handle pZp2,
                                            const ai_handle pScaleout, const ai_handle pZpout, const ai_i32 scalar_op);

AI_INTERFACE_ENTRY void ai_and(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_and_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_or(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_or_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_xor(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_xor_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_greater(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_greater_or_equal(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_greater_or_equal_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_greater_or_equal_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_less(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_less_or_equal(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_less_or_equal_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_less_or_equal_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_equal(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_equal_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_equal_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_not_equal(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_f32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_f32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_s32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_s32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_s16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_s16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_s8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_s8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_u32(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_u32(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_u16(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_u16(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);
AI_INTERFACE_ENTRY void ai_not_equal_u8(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_not_equal_buffer_u8(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_squared_diff(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_squared_diff_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_INTERFACE_ENTRY void ai_atan2(ai_handle out, const ai_handle a, const ai_handle b);
AI_INTERFACE_ENTRY void ai_atan2_buffer(ai_handle out, const ai_handle a, const ai_handle b, const ai_size loop);

AI_API_DECLARE_END

#endif /* AI_MATH_HELPERS_H */
