/**
  ******************************************************************************
  * @file    lite_nl_list.h
  * @author  STMicroelectronics
  * @brief   header file of lite supported non-linearities routines
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

// #define LITE_NL_ENTRY(nl_id_, nl_name_, nl_op_, nl_op_args_)

/* No sentry. This is deliberate!! */

LITE_NL_ENTRY(1, abs, AI_ABS, 1)
LITE_NL_ENTRY(2, acos, AI_MATH_ACOS, 1)
LITE_NL_ENTRY(3, acosh, AI_MATH_ACOSH, 1)
LITE_NL_ENTRY(4, asin, AI_MATH_ASIN, 1)
LITE_NL_ENTRY(5, asinh, AI_MATH_ASINH, 1)
LITE_NL_ENTRY(6, atan, AI_MATH_ATAN, 1)
LITE_NL_ENTRY(7, atanh, AI_MATH_ATANH, 1)
LITE_NL_ENTRY(8, ceil, AI_CEIL, 1)
LITE_NL_ENTRY(9, cos, AI_MATH_COS, 1)
LITE_NL_ENTRY(10, cosh, AI_MATH_COSH, 1)
LITE_NL_ENTRY(11, erf, AI_MATH_ERF, 1)
LITE_NL_ENTRY(12, exp, AI_MATH_EXP, 1)
LITE_NL_ENTRY(13, floor, AI_FLOOR, 1)
LITE_NL_ENTRY(14, hardmax, /**/, 0)
LITE_NL_ENTRY(15, log, AI_MATH_LOG, 1)
LITE_NL_ENTRY(16, logistic, AI_MATH_LOGISTIC, 1)
LITE_NL_ENTRY(17, neg, AI_NEG, 1)
LITE_NL_ENTRY(18, rsqrt, AI_MATH_RSQRT, 1)
LITE_NL_ENTRY(19, sin, AI_MATH_SIN, 1)
LITE_NL_ENTRY(20, sinh, AI_MATH_SINH, 1)
LITE_NL_ENTRY(21, tan, AI_MATH_TAN, 1)
LITE_NL_ENTRY(22, square, AI_MATH_SQUARE, 1)
LITE_NL_ENTRY(23, reciprocal, AI_RECIPROCAL, 1)
LITE_NL_ENTRY(24, round, AI_ROUND, 1)
LITE_NL_ENTRY(25, sigmoid, AI_MATH_SIGMOID, 1)
LITE_NL_ENTRY(26, swish, AI_MATH_SWISH, 1)
LITE_NL_ENTRY(27, hard_swish, AI_MATH_HARD_SWISH, 1)
LITE_NL_ENTRY(28, sign, AI_SIGN, 1)
LITE_NL_ENTRY(29, sqrt, AI_MATH_SQRT, 1)
// LITE_NL_ENTRY(30, softmax, /**/, 0) // for future changes
// LITE_NL_ENTRY(31, softmax_zero_channel, /**/, 0) // for future changes
LITE_NL_ENTRY(32, soft_plus, AI_MATH_SOFT_PLUS, 1)
LITE_NL_ENTRY(33, soft_sign, AI_MATH_SOFT_SIGN, 1)
LITE_NL_ENTRY(34, tanh, AI_MATH_TANH, 1)
LITE_NL_ENTRY(35, prelu, /**/, 0)
LITE_NL_ENTRY(36, relu, AI_MATH_RELU, 1)
LITE_NL_ENTRY(37, relu_generic, /**/, 0)

LITE_NL_ENTRY(101, elu, AI_MATH_ELU, 2)
LITE_NL_ENTRY(102, relu_thresholded, AI_MATH_RELU_THRESHOLDED, 2)


LITE_NL_ENTRY(201, clip, AI_CLAMP, 3)
LITE_NL_ENTRY(202, hard_sigmoid, AI_MATH_HARD_SIGMOID, 3)
LITE_NL_ENTRY(203, selu, AI_MATH_SELU, 3)
// LITE_NL_ENTRY(204, gelu, AI_MATH_GELU, 2)


#undef LITE_NL_ENTRY
#undef LITE_NL_IIF_0
#undef LITE_NL_IIF_1
#undef LITE_NL_IIF_2
#undef LITE_NL_IIF_3
