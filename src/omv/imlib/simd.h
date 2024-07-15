/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SIMD abstraction.
 */
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <arm_math.h>
#include <cmsis_extension.h>
#include "omv_boardconfig.h"

#if (__ARM_ARCH == 8)
#define VECTOR_SIZE_BYTES  16
#else
#define VECTOR_SIZE_BYTES  4
#endif

typedef int8_t v128_s8_t __attribute__ ((vector_size (VECTOR_SIZE_BYTES)));
typedef uint8_t v128_u8_t __attribute__ ((vector_size (VECTOR_SIZE_BYTES)));

typedef int16_t v128_s16_t __attribute__ ((vector_size (VECTOR_SIZE_BYTES)));
typedef uint16_t v128_u16_t __attribute__ ((vector_size (VECTOR_SIZE_BYTES)));

typedef int32_t v128_s32_t __attribute__ ((vector_size (VECTOR_SIZE_BYTES)));
typedef uint32_t v128_u32_t __attribute__ ((vector_size (VECTOR_SIZE_BYTES)));

typedef union {
    v128_s8_t s8;
    v128_u8_t u8;

    v128_s16_t s16;
    v128_u16_t u16;

    v128_s32_t s32;
    v128_u32_t u32;
} v128_t;

static inline v128_t simd_vadd_u8(v128_t v0, v128_t v1, uint32_t acc) {
    #if (__ARM_ARCH == 7)
    return (v128_t) { .u32 = { __QADD8(v0.u32[0], v1.u32[0]) } };
    #elif (__ARM_ARCH == 8)
    #else
    return (v128_t) { .u8 = v0.u8 + v1.u8 };
    #endif
}

static inline uint32_t simd_vsada_u8(v128_t v0, v128_t v1, uint32_t acc) {
    #if (__ARM_ARCH == 7)
    return __USADA8(v0.u32[0], v1.u32[0], acc);
    #elif (__ARM_ARCH == 8)
    #else
    return abs(v0.u8[0] - v1.u8[0]) + 
           abs(v0.u8[1] - v1.u8[1]) +
           abs(v0.u8[2] - v1.u8[2]) +
           abs(v0.u8[3] - v1.u8[3]) + acc;
    #endif
}


