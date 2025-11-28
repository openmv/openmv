/*
 * Unix port compatibility wrapper for omv_common.h
 * Includes MicroPython definitions then delegates to main OpenMV header
 */
#ifndef __OMV_COMMON_UNIX_WRAPPER_H__
#define __OMV_COMMON_UNIX_WRAPPER_H__

#include <stdint.h>
#include <stdbool.h>
// Include base MicroPython types first
#include "py/mpconfig.h"
#include "py/misc.h"  // For NORETURN and other MicroPython macros

// Compiler attributes needed by Unix code
#ifndef __weak
#define __weak __attribute__((weak))
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef __aligned
#define __aligned(x) __attribute__((aligned(x)))
#endif

// Now include the real OpenMV common header
// Relative path from ports/unix/unix_compat/ to common/
#include "../../../common/omv_common.h"

// Unix-specific override for OMV_ALIGN_TO to handle 64-bit pointers
// Redefine after including the original to provide Unix-specific implementation
#ifdef OMV_ALIGN_TO
#undef OMV_ALIGN_TO
#endif
#define OMV_ALIGN_TO(x, alignment) \
    ((uintptr_t) ((uintptr_t) (x) + (size_t) (alignment) - 1UL) & ~((size_t) (alignment) - 1UL))

#endif // __OMV_COMMON_UNIX_WRAPPER_H__
