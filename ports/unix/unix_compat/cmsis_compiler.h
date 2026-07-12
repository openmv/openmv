/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Stub for CMSIS cmsis_compiler.h on non-ARM hosts.
 *
 * The real cmsis_compiler.h pulls in cmsis_gcc.h whenever __GNUC__ is
 * defined, and cmsis_gcc.h provides ARM-specific assembly intrinsics
 * that don't compile on x86. The unix_compat include path is prepended
 * ahead of lib/cmsis/include so this file shadows the real one for the
 * unix build. The compiler-attribute aliases that the embedded code
 * uses are already provided in omv_force_include.h, so this file is
 * effectively a no-op include guard.
 */
#ifndef __CMSIS_COMPILER_H_UNIX_STUB__
#define __CMSIS_COMPILER_H_UNIX_STUB__
#endif
