/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * MicroPython API compatibility layer.
 * Force-included before every source file in the unix variant build.
 */
#ifndef __MP_COMPAT_H__
#define __MP_COMPAT_H__

// During qstr extraction NO_QSTR is defined and MicroPython sets
// MICROPY_ERROR_REPORTING to NONE, but we have
// MICROPY_ROM_TEXT_COMPRESSION=1 which conflicts. Override here.
#ifdef NO_QSTR
#undef MICROPY_ROM_TEXT_COMPRESSION
#define MICROPY_ROM_TEXT_COMPRESSION (0)
#endif

#endif // __MP_COMPAT_H__
