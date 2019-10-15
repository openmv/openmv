/**
  ******************************************************************************
  * @file    ai_math_helpers.h
  * @author  AST Embedded Analytics Research Platform
  * @date    01-May-2017
  * @brief   Math helpers routines header file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */ 
#ifndef __AI_MATH_HELPERS_H_
#define __AI_MATH_HELPERS_H_
#include <math.h>

#include "ai_platform.h"
#include "ai_platform_interface.h"

#define STM32_DOT_INLINE_OPTIM

#define AI_FLOAT_EPSILON_2      (6.19209290e-5F)  /* Used for small calculation 
                                                     noise issues */
#define AI_FLOAT_EPSILON        (1.19209290e-7F)
#define AI_I8_EPSILON           (0.00787401F)     /* 1/(2^7 - 1)  */
#define AI_I16_EPSILON          (3.051851e-5F)    /* 1/(2^15 - 1) */

#define AI_FLT_MAX              (3.40282346638528859812e+38f)

#define AI_MIN(x,y)             ( ((x)<(y)) ? (x) : (y) )
#define AI_MAX(x,y)             ( ((x)>(y)) ? (x) : (y) )
#define AI_CLAMP(x, min, max)   AI_MIN(AI_MAX(x,min), max)
#define AI_ROUND(x)             round(x)
#define AI_ABS(x)               fabsf(x)
#define AI_ABS_DIFF(x, y)       ( ((x)>(y)) ? ((x)-(y)) : ((y)-(x)) )

#if defined(STM32_DOT_INLINE_OPTIM)

AI_DECLARE_STATIC
void __ai_math_dot_array(
        ai_float* out,
        const ai_float* data0,
        const ai_float* data1,
        ai_size data_size)
{
  ai_float sum = 0.0f;   /* Temporary result storage */

  /* Run the below code for Cortex-M4 and Cortex-M3 */

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.    
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (data_size >= 4u)
  {
    /* C = A[0]* B[0] + A[1]* B[1] + A[2]* B[2] + .....+ A[blockSize-1]* B[blockSize-1] */
    /* Calculate dot product and then store the result in a temporary buffer */
    sum += (*data0++) * (*data1++);
    sum += (*data0++) * (*data1++);
    sum += (*data0++) * (*data1++);
    sum += (*data0++) * (*data1++);

    /* Decrement the loop counter */
    data_size -= 4u;
  }

  while (data_size > 0u)
  {
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
          __ai_math_dot_array(dst, src0, src1, size)


#else

#define AI_MATH_DOT_ARRAY(dst, src0, src1, size) \
          ai_math_dot_array(dst, src0, src1, size)

#endif
            
#define AI_MATH_SQRT(x)         ai_math_sqrt(x)
#define AI_MATH_EXP(x)          expf(x)
#define AI_MATH_POW(x, e)       powf((x), (e))
#define AI_MATH_TANH(x)         tanhf(x)
#define AI_MATH_SIGN(x)         (((x)>0) ? 1 : -1)
#define AI_MATH_LOG(x)          logf(x)

#define AI_MATH_RELU_TEST(x, thr, min, max) \
  ( ((x)<(thr)) ? (min) : (max) )

#define AI_MATH_RELU_GENERIC(x, thr, alpha, max) \
  AI_MATH_RELU_TEST(x, max, AI_MATH_RELU_GENERIC_NO_MAX(x, thr, alpha), max)

#define AI_MATH_RELU_GENERIC_NO_MAX(x, thr, alpha) \
  AI_MATH_RELU_TEST(x, thr, ((alpha)*((x)-(thr))), x)

#define AI_MATH_RELU_THRESHOLDED(x, thr) \
  AI_MATH_RELU_TEST(x, thr, 0, (x))

#define AI_MATH_LEAKY_RELU(x, neg_slope, pos_slope) \
  AI_MATH_RELU_TEST(x, 0, (x)*(neg_slope), (x)*(pos_slope))
//          ( ((x)>0) ? (x)*(pos_slope) : (x)*(neg_slope) )

#define AI_MATH_PRELU(x, slope) \
  AI_MATH_RELU_TEST(x, 0, (x)*(slope), (x))
// AI_MATH_LEAKY_RELU(x, slope, 1)

#define AI_MATH_RELU(x) \
  AI_MATH_RELU_TEST(x, 0, 0, x)
//  AI_MAX(x, 0)

#define AI_MATH_ELU(x, alpha) \
  (AI_MAX(0.0f, (x)) + AI_MIN(0.0f, (alpha) * (AI_MATH_EXP(x)-1.0f)))

#define AI_MATH_SELU(x, alpha, scale) \
  ((scale)*AI_MATH_ELU(x, alpha))

#define AI_MATH_SIGMOID(x) \
  (1.0f / (1.0f + AI_MATH_EXP(-(x))))

#define AI_MATH_HARD_SIGMOID(x) \
  (AI_MAX(0.0f, AI_MIN(1.0f, (x) * 0.2f + 0.5f)))

#define AI_MATH_SOFT_PLUS(x) \
  AI_MATH_LOG(AI_MATH_EXP(x)+1.0f)

#define AI_MATH_SOFT_SIGN(x) \
  ((x)/(AI_ABS(x)+1.0f))

AI_API_DECLARE_BEGIN

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
 * @brief platform optimized square root on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return square root of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_sqrt(const ai_float x);

/*!
 * @brief platform optimized exponential on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return exponential of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_exp(const ai_float x);

/*!
 * @brief platform optimized pow on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param e input value
 * @return pow of the value ^ e
 */
AI_INTERFACE_ENTRY
ai_float ai_math_pow(const ai_float x, const ai_float e);

/*!
 * @brief platform optimized tangent on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return hyperbolic tangent of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_tanh(const ai_float x);

/*!
 * @brief platform optimized relu on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return relu of the value ( x if x>0 else 0)
 */
AI_INTERFACE_ENTRY
ai_float ai_math_relu(const ai_float x);

/*!
 * @brief platform optimized parametric relu on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param slope input value
 * @return parametric relu of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_prelu(const ai_float x, const ai_float slope);

/*!
 * @brief platform optimized parametric sigmoid on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return sigmoid of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_sigmoid(const ai_float x);

/*!
 * @brief platform optimized parametric hard sigmoid on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return hard sigmoid of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_hard_sigmoid(const ai_float x);

/*!
 * @brief platform optimized parametric sign function on a float value
 * @ingroup math_helpers
 * @param x input value
 * @return sign of the value
 */
AI_INTERFACE_ENTRY
ai_float ai_math_sign(const ai_float x);

/*!
 * @brief optimized parametric rectified linear unit on a float value
 * @ingroup math_helpers
 * @param x input value
 * @param slope parameter value
 * @return x if x is positive and x*slope otherwise
 */
AI_INTERFACE_ENTRY
ai_float ai_fast_prelu(const ai_float x, const ai_float slope);

AI_INTERFACE_ENTRY ai_float ai_div(const ai_float a, const ai_float b);
AI_INTERFACE_ENTRY ai_float ai_max(const ai_float a, const ai_float b);
AI_INTERFACE_ENTRY ai_float ai_min(const ai_float a, const ai_float b);
AI_INTERFACE_ENTRY ai_float ai_mul(const ai_float a, const ai_float b);
AI_INTERFACE_ENTRY ai_float ai_sub(const ai_float a, const ai_float b);
AI_INTERFACE_ENTRY ai_float ai_sum(const ai_float a, const ai_float b);

AI_API_DECLARE_END

#endif /* __MATH_HELPERS_H_ */
