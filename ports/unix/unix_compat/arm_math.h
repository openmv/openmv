/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * ARM CMSIS arm_math.h shim for non-ARM hosts.
 *
 * On ARM, defer to the system arm_math.h via include_next. On the host
 * provide the OpenMV-specific intrinsics and the small subset of CMSIS
 * intrinsics that are not already in lib/cmsis/include/dsp/none.h. The
 * standard CMSIS DSP intrinsics (__SSAT, __USAT, __CLZ, __ROR, __QADD16,
 * __SMLAD, __SMUAD, __PKHBT, __PKHTB, clip_q63_to_q31 etc.) come from
 * none.h's portable C fallbacks; we don't redefine them here.
 */
#ifndef __ARM_MATH_H_COMPAT__
#define __ARM_MATH_H_COMPAT__

#ifndef CMSIS_MCU_H
#define CMSIS_MCU_H <stdint.h>
#endif

#include <stdint.h>
#include <math.h>

#ifdef __arm__
#include_next <arm_math.h>
#else

typedef float  float32_t;
typedef double float64_t;

#ifndef PI
#define PI 3.14159265358979f
#endif

// Standard math functions; on ARM the CMSIS-DSP fast approximations are
// used, but on the host libm is good enough for the test harness.
#define arm_sin_f32(x) sinf(x)
#define arm_cos_f32(x) cosf(x)

// Reverse bit order (cmsis_gcc.h on ARM).
static inline uint32_t __RBIT(uint32_t value) {
    uint32_t result = value;
    int32_t s = 31;
    for (value >>= 1U; value; value >>= 1U) {
        result <<= 1U;
        result |= value & 1U;
        s--;
    }
    return result << s;
}

// Reverse byte order (cmsis_gcc.h on ARM).
#define __REV(value)  __builtin_bswap32(value)

// Reverse byte order in 16-bit halves of a 32-bit word.
#define __REV16(value) ((__builtin_bswap16((uint16_t) (value))) | \
                        (((uint32_t) __builtin_bswap16((uint16_t) ((value) >> 16))) << 16))

// Pack two 16-bit values (low and high). Macro form to match CMSIS.
#define __PKHBT(ARG1, ARG2, ARG3)  ( ((((uint32_t) (ARG1))          ) & 0x0000FFFFUL) | \
                                     ((((uint32_t) (ARG2)) << (ARG3)) & 0xFFFF0000UL)  )

#define __PKHTB(ARG1, ARG2, ARG3)  ( ((((uint32_t) (ARG1))          ) & 0xFFFF0000UL) | \
                                     ((((uint32_t) (ARG2)) >> (ARG3)) & 0x0000FFFFUL)  )

// Dual signed multiply with accumulate (cmsis_gcc.h on ARM).
static inline uint32_t __SMLAD(uint32_t val1, uint32_t val2, uint32_t val3) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);
    int32_t p1 = (int32_t) a1 * (int32_t) b1;
    int32_t p2 = (int32_t) a2 * (int32_t) b2;
    return (uint32_t) (p1 + p2 + (int32_t) val3);
}

// Dual signed multiply (cmsis_gcc.h on ARM).
static inline uint32_t __SMUAD(uint32_t val1, uint32_t val2) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);
    int32_t p1 = (int32_t) a1 * (int32_t) b1;
    int32_t p2 = (int32_t) a2 * (int32_t) b2;
    return (uint32_t) (p1 + p2);
}

// Dual 16-bit saturating add (cmsis_gcc.h on ARM).
static inline uint32_t __QADD16(uint32_t val1, uint32_t val2) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);
    int32_t r1 = (int32_t) a1 + (int32_t) b1;
    int32_t r2 = (int32_t) a2 + (int32_t) b2;
    if (r1 > INT16_MAX) {
        r1 = INT16_MAX;
    }
    if (r1 < INT16_MIN) {
        r1 = INT16_MIN;
    }
    if (r2 > INT16_MAX) {
        r2 = INT16_MAX;
    }
    if (r2 < INT16_MIN) {
        r2 = INT16_MIN;
    }
    return (uint32_t) ((uint16_t) r1 | ((uint32_t) (uint16_t) r2 << 16));
}

// Sum of absolute differences across four unsigned bytes (cmsis_gcc.h).
static inline uint32_t __USAD8(uint32_t val1, uint32_t val2) {
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t b1 = (val1 >> (i * 8)) & 0xFF;
        uint8_t b2 = (val2 >> (i * 8)) & 0xFF;
        result += (b1 > b2) ? (b1 - b2) : (b2 - b1);
    }
    return result;
}

// Dual 16-bit signed subtraction (no saturation; cmsis_gcc.h provides on
// ARM, dsp/none.h has only the saturating __QSUB16 variant).
static inline uint32_t __SSUB16(uint32_t val1, uint32_t val2) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);
    uint16_t r1 = (uint16_t) (a1 - b1);
    uint16_t r2 = (uint16_t) (a2 - b2);
    return (uint32_t) r1 | ((uint32_t) r2 << 16);
}

// OpenMV-specific: unsigned saturate after right shift. Parameter order
// matches cmsis_extension.h: (value, sat, shift).
static inline uint32_t __USAT_ASR(int32_t value, uint32_t sat, uint32_t shift) {
    int32_t shifted = value >> shift;
    return (uint32_t) __USAT(shifted, sat);
}

// OpenMV-specific: dual 16-bit unsigned saturate.
static inline uint32_t __USAT16(uint32_t val, uint32_t sat) {
    if (sat > 16) {
        sat = 16;
    }
    int16_t v1 = (int16_t) (val & 0xFFFF);
    int16_t v2 = (int16_t) (val >> 16);
    uint32_t max = (sat == 16) ? 0xFFFF : ((1U << sat) - 1);
    uint16_t r1 = (v1 < 0) ? 0 : ((v1 > (int32_t) max) ? max : (uint16_t) v1);
    uint16_t r2 = (v2 < 0) ? 0 : ((v2 > (int32_t) max) ? max : (uint16_t) v2);
    return (uint32_t) r1 | ((uint32_t) r2 << 16);
}

#endif // __arm__

#endif // __ARM_MATH_H_COMPAT__
