/*
 * Fast math compatibility wrapper for non-ARM platforms
 * This wrapper intercepts includes of fmath.h and provides compatibility implementations
 */
#ifndef __FMATH_H_COMPAT__
#define __FMATH_H_COMPAT__

#include <stdint.h>
#include <math.h>

// Map fast math functions to standard library equivalents
#define fast_sqrtf(x)   sqrtf(x)
#define fast_floorf(x)  floorf(x)
#define fast_ceilf(x)   ceilf(x)
#define fast_roundf(x)  roundf(x)
#define fast_atanf(x)   atanf(x)
#define fast_atan2f(y,x) atan2f(y,x)
#define fast_expf(x)    expf(x)
#define fast_cbrtf(x)   cbrtf(x)
#define fast_fabsf(x)   fabsf(x)
#define fast_log(x)     logf(x)
#define fast_log2(x)    log2f(x)

// Additional math functions that may be used
#define fast_powf(x, y) powf(x, y)
#define fast_sinf(x)    sinf(x)
#define fast_cosf(x)    cosf(x)

#endif // __FMATH_H_COMPAT__
