/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Fast approximate math functions.
 *
 */
#ifndef __FMATH_H__
#define __FMATH_H__
#include <stdint.h>
float fast_sqrtf(float x);
int fast_floorf(float x);
int fast_ceilf(float x);
int fast_roundf(float x);
float fast_atanf(float x);
float fast_atan2f(float y, float x);
float fast_expf(float x);
float fast_cbrtf(float d);
float fast_fabsf(float d);
float fast_log(float x);
float fast_log2(float x);
extern const float cos_table[360];
extern const float sin_table[360];
#endif // __FMATH_H__
