/*
 * Force-included header for Unix builds
 * This defines macros that need to be set before any other headers are included
 */
#ifndef __OMV_FORCE_INCLUDE_H__
#define __OMV_FORCE_INCLUDE_H__

// Ensure stdarg.h is included early for va_start (needed by MicroPython's mp_vprintf)
#include <stdarg.h>

// Define CMSIS_MCU_H to point to stdint.h for Unix builds
#define CMSIS_MCU_H <stdint.h>

// Stub ARM Cortex-M intrinsics
// Don't define these if we're compiling CMSIS DSP itself (it provides its own)
#ifndef ARM_MATH_CM7
#define __WFI() do {} while (0)
#define __WFE() do {} while (0)
#define __SEV() do {} while (0)
#define __NOP() do {} while (0)
#define __DSB() do {} while (0)
#define __ISB() do {} while (0)
#define __DMB() do {} while (0)
#endif

// MicroPython m_free compatibility
// We provide an actual m_free function in py_unix_stubs.c since OpenMV passes it as a function pointer
// (macros cannot be passed as function pointers)

#endif // __OMV_FORCE_INCLUDE_H__
