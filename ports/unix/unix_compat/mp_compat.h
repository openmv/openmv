/*
 * MicroPython API compatibility layer
 * Handles differences between OpenMV's expected API and current MicroPython
 */
#ifndef __MP_COMPAT_H__
#define __MP_COMPAT_H__

// This header is force-included before any source file
// It provides compatibility wrappers for API changes

// During qstr extraction with NO_QSTR, MicroPython sets MICROPY_ERROR_REPORTING to NONE
// but we have MICROPY_ROM_TEXT_COMPRESSION=1, which conflicts. Override during qstr extraction.
#ifdef NO_QSTR
#undef MICROPY_ROM_TEXT_COMPRESSION
#define MICROPY_ROM_TEXT_COMPRESSION (0)
#endif

// MicroPython's m_free now requires size parameter, but OpenMV code doesn't track sizes
// We provide an actual m_free function in py_unix_stubs.c since OpenMV passes it as a function pointer
// (macros cannot be passed as function pointers)
#ifndef NO_QSTR
#include "py/gc.h"

// Forward declare py_clock_type for OpenMV's modified modtime.c
// (OpenMV patches MicroPython's modtime.c to include clock module)
extern const struct _mp_obj_type_t py_clock_type;
#endif

#endif // __MP_COMPAT_H__
