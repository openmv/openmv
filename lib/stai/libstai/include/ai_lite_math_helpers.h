#ifndef AI_LITE_MATH_HELPERS_H
#define AI_LITE_MATH_HELPERS_H
/**
  ******************************************************************************
  * @file    ai_lite_math_helpers.h
  * @author  STMicroelectronics
  * @brief   Math helpers routines header file for lite APIs.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include <math.h>
#include <limits.h>
#include <stdint.h>

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "ai_datatypes_defines.h"

#define AI_FLOAT_TOLERANCE      (6.19209290e-5F)  /* Used for small calculation
                                                     noise issues */
#define AI_FLOAT_EPSILON        (1.19209290e-7F)
#define AI_I8_EPSILON           (0.00787401F)     /* 1/(2^7 - 1)  */
#define AI_I16_EPSILON          (3.051851e-5F)    /* 1/(2^15 - 1) */

#define AI_FLT_MAX              (3.40282346638528859812e+38f)

#define AI_MIN(x,y)             ( ((x)<(y)) ? (x) : (y) )
#define AI_MAX(x,y)             ( ((x)>(y)) ? (x) : (y) )
#define AI_SIGN(x)              (((x)>0) ? 1 : -1)
#define AI_CLAMP(x, min, max)   AI_MIN(AI_MAX(x,min), max)
#define AI_ABS(x)               fabsf(x)
#define AI_ABS_DIFF(x, y)       ( ((x)>(y)) ? ((x)-(y)) : ((y)-(x)) )
#define AI_NEG(x)               ( -1 * (x) )
#define AI_NOT(x)               ( ((x)==true) ? false : true)
#define AI_RECIPROCAL(x)        ( 1.0f / (x) )
#define AI_CEIL(x)              ceilf(x)
#define AI_FLOOR(x)             floorf(x)
#define AI_FLOOR_DIV(x, y)      AI_FLOOR((x)/(y))  /* floor division: x // y */
#define AI_FLOOR_MOD(x, y)      fmodf(x, y)
#define AI_ROUND(x)             roundf(x)
#define AI_POW(x,y)             powf(x, y)

#define AI_SQUARED_DIFF(x, y)   (((x)-(y)) * ((x)-(y)))

#define AI_FLOAT_NEGATIVE_HALF        (-0.5f + AI_FLOAT_EPSILON)
#define AI_FLOAT_POSITIVE_HALF        (0.5f)


#define AI_MATH_ACOS(x)         acosf(x)
#define AI_MATH_ACOSH(x)        acoshf(x)
#define AI_MATH_ASIN(x)         asinf(x)
#define AI_MATH_ASINH(x)        asinhf(x)
#define AI_MATH_ATAN(x)         atanf(x)
#define AI_MATH_ATANH(x)        atanhf(x)
#define AI_MATH_COS(x)          cosf(x)
#define AI_MATH_COSH(x)         coshf(x)
#define AI_MATH_ERF(x)          erff(x)
#define AI_MATH_EXP(x)          expf(x)
#define AI_MATH_LOG(x)          logf(x)
#define AI_MATH_POW(x, e)       powf((x), (e))
#define AI_MATH_RSQRT(x)        (1.0f / AI_MATH_SQRT(x))
#define AI_MATH_SIN(x)          sinf(x)
#define AI_MATH_SINH(x)         sinhf(x)
#define AI_MATH_SQRT(x)         ai_math_sqrt(x)
#define AI_MATH_TAN(x)          tanf(x)
#define AI_MATH_TANH(x)         tanhf(x)
#define AI_MATH_SQUARE(x)       AI_MATH_POW(x, 2.0f)

#define AI_MATH_ACOS(x)         acosf(x)
#define AI_MATH_ACOSH(x)        acoshf(x)
#define AI_MATH_ASIN(x)         asinf(x)
#define AI_MATH_ASINH(x)        asinhf(x)
#define AI_MATH_ATAN(x)         atanf(x)
#define AI_MATH_ATANH(x)        atanhf(x)
#define AI_MATH_COS(x)          cosf(x)
#define AI_MATH_COSH(x)         coshf(x)
#define AI_MATH_ERF(x)          erff(x)
#define AI_MATH_EXP(x)          expf(x)
#define AI_MATH_LOG(x)          logf(x)
#define AI_MATH_POW(x, e)       powf((x), (e))
#define AI_MATH_RSQRT(x)        (1.0f / AI_MATH_SQRT(x))
#define AI_MATH_SIN(x)          sinf(x)
#define AI_MATH_SINH(x)         sinhf(x)
#define AI_MATH_SQRT(x)         ai_math_sqrt(x)
#define AI_MATH_TAN(x)          tanf(x)
#define AI_MATH_TANH(x)         tanhf(x)
#define AI_MATH_SQUARE(x)       AI_MATH_POW(x, 2.0f)
#define AI_MATH_RELU_TEST(x, thr, min, max) \
  (((x)<=(thr)) ? (min) : (max))

#define AI_MATH_CLIP_LINEAR_REMAP(x, alpha, beta) \
  (AI_MAX(0, AI_MIN(1, ((x) * (alpha) + (beta)))))

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

#define AI_MATH_SCALED_TANH(x, alpha, beta) \
  ((alpha)*AI_MATH_TANH((beta)*(x)))

#define AI_MATH_SIGMOID(x) \
  (1.0f / (1.0f + AI_MATH_EXP(-(x))))

#define AI_MATH_LOGISTIC(x)\
    (x < 0) ? (1.0f -(1.0f / (1.0f + AI_MATH_EXP(-AI_ABS(x))))) :\
    (1.0f / (1.0f + AI_MATH_EXP(-AI_ABS(x))))

#define AI_MATH_HARD_SIGMOID(x, alpha, beta) \
    AI_MATH_CLIP_LINEAR_REMAP(x, alpha, beta)

#define AI_MATH_GELU_NO_APPROXIMATE(x) \
    ((x / 2.0f) * (1.0f + AI_MATH_ERF(x/AI_MATH_SQRT(2.0f))))

#define AI_MATH_GELU_APPROXIMATE(x) \
    ((x / 2.0f) * (1.0f + AI_MATH_TANH(AI_MATH_SQRT(2.0f/PI)*(x + 0.044715f * AI_MATH_POW(x, 3.0f)))))

#define AI_MATH_GELU(x, approximate) \
    (((bool)approximate) ? AI_MATH_GELU_APPROXIMATE(x) : AI_MATH_GELU_NO_APPROXIMATE(x))




/* Formula with higher accuracy */
#define AI_MATH_SWISH(x) \
  ((x) * AI_MATH_SIGMOID(x))

#define AI_MATH_HARD_SWISH(x) \
  ((x) * AI_MATH_CLIP_LINEAR_REMAP(x, 1.0f/6, 0.5f))

#define AI_MATH_SOFT_PLUS(x) \
  AI_MATH_LOG(1.0f + AI_MATH_EXP(x))

#define AI_MATH_SOFT_SIGN(x) \
  ((x) / (1.0f + AI_ABS(x)))

/*!
 * @brief Round float x to the nearest integer (breaking +- 0.5 ties to the nearest even integer)
 */
static inline ai_i32 ai_round_f2i_t2e(ai_float x)
{
    x += x >= 0.0f ? 0.5f : -0.5f;
    ai_i32 i32_x = (ai_i32)x;

    if (((ai_float)i32_x == x) && ((i32_x & 0x1) != 0)) {
        ai_i32 to_nearest_even = i32_x < 0 ? 1 : -1;
        i32_x += to_nearest_even;
    }
    return i32_x;
}

static inline ai_u32 ai_round_f2u_t2e(ai_float x)
{
    x += 0.5f;
    ai_u32 u32_x = (ai_u32)x;

    if (((ai_float)u32_x) == x && ((u32_x & 0x1) != 0)) {
        u32_x -= 1;
    }
    return u32_x;
}


AI_API_DECLARE_BEGIN

/*!
 * @typedef ai_vec4_float
 * @ingroup ai_datatypes_internal
 * @brief 32bit X 4 float (optimization for embedded MCU)
 */
typedef struct {
  ai_float a1;
  ai_float a2;
  ai_float a3;
  ai_float a4;
} ai_vec4_float;


#define AI_VEC4_FLOAT(ptr_) \
  _get_vec4_float((ai_handle)(ptr_))


AI_DECLARE_STATIC
ai_vec4_float _get_vec4_float(const ai_handle fptr)
{
  return *((const ai_vec4_float*)fptr);
}

/*****************************************************************************/
typedef struct {
  ai_u16 numRows;       /**< number of rows of the matrix.     */
  ai_u16 numCols;       /**< number of columns of the matrix.  */
  ai_float *pData;      /**< points to the data of the matrix. */
} ai_matrix_f32;

/*!
  * @brief general 2D matrix initialization
  * @ingroup ai_lite_math_helpers
  * @param S pointer to S matrix
  * @param nRows number of rows of S matrix
  * @param nColumns number of columns of S matrix
  * @param pData pointer to S matrix data
  */
AI_INTERFACE_ENTRY
void st_mat_init_f32(ai_matrix_f32* S,
  const uint16_t nRows,
  const uint16_t nColumns,
  float*         pData);


/*!
  * @brief general 2D matrix multiplication on float values
  * @ingroup ai_lite_math_helpers
  * @param pSrcA pointer to A matrix
  * @param pSrcB pointer to B matrix
  * @param pSrcC pointer to C matrix/array
  * @param alpha multiplier of A*B product
  * @param beta multiplier of C
  * @param tA flag for A transpose
  * @param tB flag for B transpose
  * @param pDstY matrix result
  * @return ARM_MATH_SUCCESS in case of success, ARM_MATH_SIZE_MISMATCH else
  */
AI_INTERFACE_ENTRY
uint32_t st_mat_gemm_f32(const ai_matrix_f32* pSrcA,
                         const ai_matrix_f32* pSrcB,
                         const ai_matrix_f32* pSrcC,
                         const float alpha, const float beta,
                         const int8_t tA, const int8_t tB,
                         ai_matrix_f32 * pDstY);

/*!
 * @brief platform optimized square root on a float value
 * @ingroup ai_lite_math_helpers
 * @param x input value
 * @return square root of the value
 */
AI_INTERFACE_ENTRY
float ai_math_sqrt(const float x);

#endif /*AI_LITE_MATH_HELPERS_H*/
