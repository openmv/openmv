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

// extmod/modtime.c (patched by OpenMV's micropython submodule) does
// `#include "py_clock.h"` to register the clock module. CFLAGS_USERMOD
// puts -I$(OMV_PORT_DIR) before -I$(OMV_MOD_DIR), so the include
// resolves to the empty stub at ports/unix/py_clock.h rather than the
// real header in modules/. Forward-declare the type here so modtime.c
// links until that include order is corrected.
#ifndef NO_QSTR
extern const struct _mp_obj_type_t py_clock_type;
#endif

#endif // __MP_COMPAT_H__
