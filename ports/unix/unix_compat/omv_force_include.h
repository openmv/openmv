/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Force-included header for Unix builds. Sets up macros and stubs that
 * embedded code expects from the ARM CMSIS / linker environment but that
 * have no native equivalent on a host build.
 */
#ifndef __OMV_FORCE_INCLUDE_H__
#define __OMV_FORCE_INCLUDE_H__

// Ensure stdarg.h is included early for va_start (used by MicroPython's
// mp_vprintf).
#include <stdarg.h>
#include <stdint.h>

// fmath.h does `#include CMSIS_MCU_H`, so the macro must be set before
// any header that pulls fmath.h in.
#ifndef CMSIS_MCU_H
#define CMSIS_MCU_H <stdint.h>
#endif

// Pre-trip cmsis_gcc.h's include guard so its ARM-specific assembly
// intrinsics body isn't read on the host. The unix port doesn't pull in
// CMSIS DSP sources, so the only intrinsics that need stubbing here are
// the ones imlib actually uses: __SSAT, __USAT, __CLZ, __ROR.
#ifndef __arm__
#ifndef __CMSIS_GCC_H
#define __CMSIS_GCC_H
#endif
#ifndef __STATIC_INLINE
#define __STATIC_INLINE                       static inline
#endif
#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE                  static inline __attribute__((always_inline))
#endif
#ifndef __INLINE
#define __INLINE                              inline
#endif
#ifndef __ASM
#define __ASM                                 __asm
#endif
#ifndef __WEAK
#define __WEAK                                __attribute__((weak))
#endif
#ifndef __PACKED
#define __PACKED                              __attribute__((packed))
#endif
#ifndef __PACKED_STRUCT
#define __PACKED_STRUCT                       struct __attribute__((packed))
#endif
#ifndef __PACKED_UNION
#define __PACKED_UNION                        union __attribute__((packed))
#endif
#ifndef __ALIGNED
#define __ALIGNED(x)                          __attribute__((aligned(x)))
#endif
#ifndef __NO_RETURN
#define __NO_RETURN                           __attribute__((__noreturn__))
#endif
#ifndef __USED
#define __USED                                __attribute__((used))
#endif

// Standard CMSIS arithmetic intrinsics that imlib expects from
// cmsis_gcc.h. CMSIS DSP code isn't pulled in for the unix build, so we
// provide portable C versions here once.
__STATIC_FORCEINLINE uint8_t __CLZ(uint32_t data) {
    return data == 0U ? 32U : (uint8_t) __builtin_clz(data);
}

__STATIC_FORCEINLINE int32_t __SSAT(int32_t val, uint32_t sat) {
    if (sat >= 1U && sat <= 32U) {
        const int32_t max = (int32_t) ((1U << (sat - 1U)) - 1U);
        const int32_t min = -1 - max;
        if (val > max) {
            return max;
        } else if (val < min) {
            return min;
        }
    }
    return val;
}

__STATIC_FORCEINLINE uint32_t __USAT(int32_t val, uint32_t sat) {
    // Match CMSIS spec: saturate negatives to 0 for any sat in [0, 32].
    if (val < 0) {
        return 0U;
    }
    if (sat < 32U) {
        const uint32_t max = (1U << sat) - 1U;
        if ((uint32_t) val > max) {
            return max;
        }
    }
    return (uint32_t) val;
}

__STATIC_FORCEINLINE uint32_t __ROR(uint32_t op1, uint32_t op2) {
    op2 &= 31U;
    return op2 == 0U ? op1 : (op1 >> op2) | (op1 << (32U - op2));
}
#endif

// Additional OpenMV-specific intrinsics (__USAT_ASR, __USAT16, __USAD8,
// __SSUB16, __REV, __REV16, __RBIT) live in
// ports/unix/unix_compat/arm_math.h, force-included after this one.

// Stub ARM Cortex-M intrinsics for non-ARM hosts. CMSIS DSP files build
// with -U on these macros (see modules/micropython.mk) so they fall back
// to whatever CMSIS Core would provide; on the host none of those is
// reachable so this gates by host architecture, not by ARM_MATH_CM7.
#ifndef __arm__
#define __WFI() do {} while (0)
#define __WFE() do {} while (0)
#define __SEV() do {} while (0)
#define __NOP() do {} while (0)
#define __DSB() do {} while (0)
#define __ISB() do {} while (0)
#define __DMB() do {} while (0)
#endif

// Stubs for DWT cycle counter and SystemCoreClock referenced by
// common/omv_cycles.h. Cycle counting is meaningless without the ARM
// hardware unit; these stubs only satisfy the link step. Use a constant
// nominal CPU frequency so the OMV_CYCLES_PER_US/MS divisions compile.
// The guard is on the host architecture rather than ARM_MATH_CM7
// (which is defined for the whole build to enable CMSIS DSP).
#ifndef __arm__
#ifndef OMV_CPU_FREQ_HZ
#define OMV_CPU_FREQ_HZ                       (1000000000UL)
#endif
typedef struct {
    volatile uint32_t CYCCNT;
} _omv_unix_dwt_t;
extern _omv_unix_dwt_t *DWT;
#endif

#endif // __OMV_FORCE_INCLUDE_H__
