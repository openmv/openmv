// SPDX-License-Identifier: MIT
//
// Copyright (C) 2026 OpenMV, LLC.
//
// Image library configuration checks. Included by imlib.h after the board's
// imlib_config.h, so that the checks see the board's overrides.
#ifndef __IMLIB_CONFIG_CHECK_H__
#define __IMLIB_CONFIG_CHECK_H__

#if defined(IMLIB_ENABLE_FIND_DISPLACEMENT) && !defined(IMLIB_ENABLE_ROTATION_CORR)
#error "IMLIB_ENABLE_FIND_DISPLACEMENT requires IMLIB_ENABLE_ROTATION_CORR"
#endif

#if !defined(IMLIB_ENABLE_APRILTAGS) && \
    (defined(IMLIB_ENABLE_FINE_APRILTAGS) || defined(IMLIB_ENABLE_HIGH_RES_APRILTAGS))
#error "AprilTag options require IMLIB_ENABLE_APRILTAGS"
#endif

#if !defined(IMLIB_ENABLE_APRILTAGS) &&                 \
    (defined(IMLIB_ENABLE_APRILTAGS_TAG16H5) ||         \
    defined(IMLIB_ENABLE_APRILTAGS_TAG25H9) ||          \
    defined(IMLIB_ENABLE_APRILTAGS_TAG36H10) ||         \
    defined(IMLIB_ENABLE_APRILTAGS_TAG36H11) ||         \
    defined(IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7) ||    \
    defined(IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12) ||   \
    defined(IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12) ||   \
    defined(IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12) || \
    defined(IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13))
#error "AprilTag families require IMLIB_ENABLE_APRILTAGS"
#endif

#if defined(IMLIB_ENABLE_APRILTAGS) &&                   \
    !defined(IMLIB_ENABLE_APRILTAGS_TAG16H5) &&          \
    !defined(IMLIB_ENABLE_APRILTAGS_TAG25H9) &&          \
    !defined(IMLIB_ENABLE_APRILTAGS_TAG36H10) &&         \
    !defined(IMLIB_ENABLE_APRILTAGS_TAG36H11) &&         \
    !defined(IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7) &&    \
    !defined(IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12) &&   \
    !defined(IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12) &&   \
    !defined(IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12) && \
    !defined(IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13)
#error "IMLIB_ENABLE_APRILTAGS requires at least one AprilTag family"
#endif

#endif // __IMLIB_CONFIG_CHECK_H__
