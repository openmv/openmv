/*
 * CMSIS extension compatibility for non-ARM platforms
 */
#ifndef __CMSIS_EXTENSION_H_COMPAT__
#define __CMSIS_EXTENSION_H_COMPAT__

#ifdef __arm__
// On ARM, use the real CMSIS extension
#include_next <cmsis_extension.h>
#else
// On non-ARM platforms, provide minimal compatibility

// Compiler attribute for forcing inline
#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE static inline __attribute__((always_inline))
#endif

#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif

// No-op for non-ARM
#ifndef __NOP
#define __NOP() do {} while (0)
#endif

#endif // __arm__

#endif // __CMSIS_EXTENSION_H_COMPAT__
