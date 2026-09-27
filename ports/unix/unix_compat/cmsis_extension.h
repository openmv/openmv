/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Stub for CMSIS cmsis_extension.h on non-ARM hosts.
 *
 * The real cmsis_extension.h provides ARM-specific intrinsics. The
 * unix_compat include path is prepended ahead of lib/cmsis/include so
 * this file shadows it for the unix build. All the macros and
 * intrinsic stubs imlib's include of <cmsis_extension.h> needs are
 * provided by omv_force_include.h (which is force-included via
 * -include for every translation unit), so this file is just an empty
 * include guard.
 */
#ifndef __CMSIS_EXTENSION_H_UNIX_STUB__
#define __CMSIS_EXTENSION_H_UNIX_STUB__
#endif
