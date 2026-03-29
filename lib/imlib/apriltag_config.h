// SPDX-License-Identifier: MIT
//
// Copyright (C) 2025 OpenMV, LLC.
//
// AprilTag configuration for OpenMV embedded targets.
// Included by platform.h when APRILTAG_HAVE_CONFIG is defined.
#ifndef __APRILTAG_CONFIG_H__
#define __APRILTAG_CONFIG_H__
#include <stdint.h>
#include "imlib_config.h"
#include "umalloc.h"

// Feature disables
#define APRILTAG_ENABLE_PTHREADS        (0)
#define APRILTAG_ENABLE_IMAGE_IO        (0)
#define APRILTAG_ENABLE_DEBUG           (0)
#define APRILTAG_ENABLE_PROFILE         (0)
#define APRILTAG_ENABLE_TAG_NAMES       (0)

#ifndef IMLIB_ENABLE_FINE_APRILTAGS
#define APRILTAG_ENABLE_8_CONNECTIVITY  (0)
#endif

#ifndef IMLIB_ENABLE_HIGH_RES_APRILTAGS
#define APRILTAG_ENABLE_32BIT_UNIONFIND (0)
#endif

// Disable tag families not enabled in imlib_config.h
#ifndef IMLIB_ENABLE_APRILTAGS_TAG16H5
#define APRILTAG_ENABLE_TAG16H5         (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAG25H9
#define APRILTAG_ENABLE_TAG25H9         (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAG36H10
#define APRILTAG_ENABLE_TAG36H10        (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAG36H11
#define APRILTAG_ENABLE_TAG36H11        (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7
#define APRILTAG_ENABLE_TAGCIRCLE21H7   (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12
#define APRILTAG_ENABLE_TAGCIRCLE49H12  (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12
#define APRILTAG_ENABLE_TAGCUSTOM48H12  (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12
#define APRILTAG_ENABLE_TAGSTANDARD41H12 (0)
#endif
#ifndef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13
#define APRILTAG_ENABLE_TAGSTANDARD52H13 (0)
#endif

// Redirect malloc/calloc/realloc/free to UMA allocator.
#define apriltag_malloc(s)      uma_malloc(s, 0)
#define apriltag_calloc(n, s)   uma_calloc((n) * (s), 0)
#define apriltag_realloc(p, s)  uma_realloc(p, s, 0)
#define apriltag_free(p)        uma_free(p)
#define apriltag_assert(x)      ((void) 0)

// Simple strtod that avoids pulling in newlib stdio/malloc.
// Only needs to handle small integer constants used in matd_op expressions.
static inline double _apriltag_strtod(const char *s, char **endp) {
    int val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s++ - '0');
    }
    if (endp) {
        *endp = (char *) s;
    }
    return (double) val;
}
#define apriltag_strtod(s, endp) _apriltag_strtod(s, endp)

#ifndef APRILTAG_STACK_LIMIT
#define APRILTAG_STACK_LIMIT   (2048)
#endif

// Dynamic stack availability check for ptsort.
static inline size_t _apriltag_stack_avail(void) {
    extern char _sstack;
    volatile char _estack;
    intptr_t avail = (intptr_t) &_estack - (intptr_t) &_sstack - (intptr_t) APRILTAG_STACK_LIMIT;
    return (avail > 0) ? (size_t) avail : 0;
}
#define apriltag_stack_avail() _apriltag_stack_avail()

#endif // __APRILTAG_CONFIG_H__
