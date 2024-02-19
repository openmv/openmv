/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Fast approximate math functions.
 */
#ifndef __FMATH_H__
#define __FMATH_H__
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include "omv_common.h"
#include CMSIS_MCU_H
#include "arm_math.h"
#if (__ARM_ARCH < 7)
#include <math.h>
static float OMV_ATTR_ALWAYS_INLINE fast_sqrtf(float x) {
    return sqrtf(x);
}

static int OMV_ATTR_ALWAYS_INLINE fast_floorf(float x) {
    return floorf(x);
}

static int OMV_ATTR_ALWAYS_INLINE fast_ceilf(float x) {
    return ceilf(x);
}

static int OMV_ATTR_ALWAYS_INLINE fast_roundf(float x) {
    return roundf(x);
}

static float OMV_ATTR_ALWAYS_INLINE fast_fabsf(float x) {
    return fabsf(x);
}
#else
static float OMV_ATTR_ALWAYS_INLINE fast_sqrtf(float x) {
    __asm__ volatile (
        "vsqrt.f32  %[r], %[x]\n"
        : [r] "=t" (x)
        : [x] "t"  (x));
    return x;
}

static int OMV_ATTR_ALWAYS_INLINE fast_floorf(float x) {
    int i;
    __asm__ volatile (
    #if (__CORTEX_M > 4)
        "vcvtm.S32.f32  %[r], %[x]\n"
    #else
        "vcvt.S32.f32  %[r], %[x]\n"
    #endif
        : [r] "=t" (i)
        : [x] "t"  (x));
    return i;
}

static int OMV_ATTR_ALWAYS_INLINE fast_ceilf(float x) {
    int i;
    #if (__CORTEX_M > 4)
    __asm__ volatile (
        "vcvtp.S32.f32  %[r], %[x]\n"
    #else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
    #pragma GCC diagnostic ignored "-Wuninitialized"
    uint32_t max = 0x3f7fffff;
    x += *((float *) &max);
    #pragma GCC diagnostic pop
    __asm__ volatile (
        "vcvt.S32.f32  %[r], %[x]\n"
    #endif
        : [r] "=t" (i)
        : [x] "t"  (x));
    return i;
}

static int OMV_ATTR_ALWAYS_INLINE fast_roundf(float x) {
    int i;
    __asm__ volatile (
        "vcvtr.S32.F32  %[r], %[x]\n"
        : [r] "=t" (i)
        : [x] "t"  (x));
    return i;
}

static float OMV_ATTR_ALWAYS_INLINE fast_fabsf(float x) {
    __asm__ volatile (
        "vabs.f32  %[r], %[x]\n"
        : [r] "=t" (x)
        : [x] "t"  (x));
    return x;
}
#endif
float fast_atanf(float x);
float fast_atan2f(float y, float x);
float fast_expf(float x);
float fast_cbrtf(float d);
float fast_fabsf(float d);
float fast_log(float x);
float fast_log2(float x);
float fast_powf(float a, float b);
void fast_get_min_max(float *data, size_t data_len, float *p_min, float *p_max);
extern const float cos_table[360];
extern const float sin_table[360];
#endif // __FMATH_H__
