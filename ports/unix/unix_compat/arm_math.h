/*
 * ARM CMSIS compatibility layer for non-ARM platforms
 * This file provides stubs/implementations for ARM CMSIS functions
 * to enable building on non-ARM platforms like Unix.
 */
#ifndef __ARM_MATH_H_COMPAT__
#define __ARM_MATH_H_COMPAT__

// Define CMSIS_MCU_H before fmath.h tries to include it
// This is needed because fmath.h has `#include CMSIS_MCU_H`
#ifndef CMSIS_MCU_H
#define CMSIS_MCU_H <stdint.h>
#endif

#include <stdint.h>
#include <math.h>

#ifdef __arm__
// On ARM, use the real ARM CMSIS library
#include_next <arm_math.h>
#else
// On non-ARM platforms (Unix, etc), provide compatibility implementations

// CMSIS types
typedef float  float32_t;
typedef double float64_t;

#ifndef PI
#define PI 3.14159265358979f
#endif

// ARM trigonometric functions - use standard library
#define arm_sin_f32(x) sinf(x)
#define arm_cos_f32(x) cosf(x)

// Saturating arithmetic intrinsics

/**
   \brief   Signed Saturate
   \param [in]  value  Value to be saturated
   \param [in]  sat    Bit position to saturate to (1..32)
   \return             Saturated value
 */
static inline int32_t __SSAT(int32_t value, uint32_t sat) {
    // Validate sat parameter to avoid undefined behavior
    if (sat == 0 || sat > 31) {
        return value; // No saturation if out of valid range
    }
    int32_t max = (1 << (sat - 1)) - 1;
    int32_t min = -(1 << (sat - 1));
    if (value > max) {
        return max;
    }
    if (value < min) {
        return min;
    }
    return value;
}

/**
   \brief   Unsigned Saturate
   \param [in]  value  Value to be saturated
   \param [in]  sat    Bit position to saturate to (0..31)
   \return             Saturated value
 */
static inline uint32_t __USAT(int32_t value, uint32_t sat) {
    // Validate sat parameter to avoid undefined behavior
    if (sat > 31) {
        // Out of range - saturate negative to 0, positive unchanged
        return (value < 0) ? 0 : (uint32_t) value;
    }
    if (value < 0) {
        return 0;
    }

    // For sat=31, max is 2^31-1 = INT32_MAX
    if (sat == 31) {
        return (uint32_t) value; // Already checked value >= 0
    }

    uint32_t max = (1U << sat) - 1;
    if ((uint32_t) value > max) {
        return max;
    }
    return (uint32_t) value;
}

// Bit manipulation intrinsics

/**
   \brief   Reverse bit order of value
   \param [in]    value  Value to reverse
   \return               Reversed value
 */
static inline uint32_t __RBIT(uint32_t value) {
    uint32_t result;
    int32_t s = (4U * 8U) - 1U;

    result = value;
    for (value >>= 1U; value; value >>= 1U) {
        result <<= 1U;
        result |= value & 1U;
        s--;
    }
    result <<= s;
    return result;
}

/**
   \brief   Dual signed multiply accumulate
   \details Performs two 16-bit signed multiplications and adds results plus accumulator
 */
static inline uint32_t __SMLAD(uint32_t val1, uint32_t val2, uint32_t val3) {
    // Extract signed 16-bit values
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);

    // Perform signed multiplication with 32-bit intermediate results
    int32_t p1 = (int32_t) a1 * (int32_t) b1;
    int32_t p2 = (int32_t) a2 * (int32_t) b2;

    return (uint32_t) (p1 + p2 + (int32_t) val3);
}

/**
   \brief   Dual signed multiply accumulate (unsigned result)
   \details Performs two 16-bit signed multiplications and adds results
 */
static inline uint32_t __SMUAD(uint32_t val1, uint32_t val2) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);
    // Use explicit 32-bit intermediates to document intent
    int32_t p1 = (int32_t) a1 * (int32_t) b1;
    int32_t p2 = (int32_t) a2 * (int32_t) b2;
    return (uint32_t) (p1 + p2);
}

/**
   \brief   Dual 16-bit saturating add
   \details Adds two pairs of 16-bit values with saturation
 */
static inline uint32_t __QADD16(uint32_t val1, uint32_t val2) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);

    int32_t r1 = (int32_t) a1 + (int32_t) b1;
    int32_t r2 = (int32_t) a2 + (int32_t) b2;

    // Saturate to 16-bit signed range
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

/**
   \brief   Count leading zeros
   \details Returns number of leading zero bits (32 for zero input)
 */
static inline uint8_t __CLZ(uint32_t value) {
    // __builtin_clz(0) is undefined behavior - handle explicitly
    if (value == 0) {
        return 32;
    }
    return (uint8_t) __builtin_clz(value);
}

/**
   \brief   Pack two 16-bit values
 */
#define __PKHBT(ARG1, ARG2, ARG3)  ( ((((uint32_t) (ARG1))          ) & 0x0000FFFFUL) | \
                                     ((((uint32_t) (ARG2)) << (ARG3)) & 0xFFFF0000UL)  )

#define __PKHTB(ARG1, ARG2, ARG3)  ( ((((uint32_t) (ARG1))          ) & 0xFFFF0000UL) | \
                                     ((((uint32_t) (ARG2)) >> (ARG3)) & 0x0000FFFFUL)  )

/**
   \brief   Reverse byte order (32 bit)
 */
#define __REV(value) __builtin_bswap32(value)

/**
   \brief   Rotate Right in unsigned value (32 bit)
 */
static inline uint32_t __ROR(uint32_t op1, uint32_t op2) {
    op2 %= 32U;
    if (op2 == 0U) {
        return op1;
    }
    return (op1 >> op2) | (op1 << (32U - op2));
}

/**
   \brief   Reverse byte order (16 bit)
 */
#define __REV16(value) __ROR(__REV(value), 16)

/**
   \brief   Unsigned Sum of Absolute Differences (SIMD)
   \details Sum of absolute differences of unsigned bytes
 */
static inline uint32_t __USAD8(uint32_t val1, uint32_t val2) {
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t byte1 = (val1 >> (i * 8)) & 0xFF;
        uint8_t byte2 = (val2 >> (i * 8)) & 0xFF;
        result += (byte1 > byte2) ? (byte1 - byte2) : (byte2 - byte1);
    }
    return result;
}

/**
   \brief   Unsigned Saturate with right shift
   \param [in]  value  Value to be saturated
   \param [in]  sat    Bit position to saturate to (0..31)
   \param [in]  shift  Right shift amount
   \return             Saturated value
 */
static inline uint32_t __USAT_ASR(int32_t value, uint32_t sat, uint32_t shift) {
    int32_t shifted = value >> shift;
    return __USAT(shifted, sat);
}

/**
   \brief   Dual 16-bit signed subtraction
   \details Performs two 16-bit signed subtractions in parallel
 */
static inline uint32_t __SSUB16(uint32_t val1, uint32_t val2) {
    int16_t a1 = (int16_t) (val1 & 0xFFFF);
    int16_t a2 = (int16_t) (val1 >> 16);
    int16_t b1 = (int16_t) (val2 & 0xFFFF);
    int16_t b2 = (int16_t) (val2 >> 16);

    uint16_t r1 = (uint16_t) (a1 - b1);
    uint16_t r2 = (uint16_t) (a2 - b2);

    return (uint32_t) r1 | ((uint32_t) r2 << 16);
}

/**
   \brief   Dual 16-bit unsigned saturate
   \details Saturates two 16-bit values in parallel
 */
static inline uint32_t __USAT16(uint32_t val, uint32_t sat) {
    // Clamp sat to valid range for 16-bit saturation
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
