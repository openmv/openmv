/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * module stubs when compiling for unix port.
 *
 */
#include <math.h>
#include <string.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "py_helper.h"
#include "framebuffer.h"
#include "imlib.h"

#define MODULE_UNAVAILABLE(mod) \
static const mp_map_elem_t mod ## _globals_dict_table[] = { \
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_ ## mod)  }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__),    (mp_obj_t)&py_func_unavailable_obj    }, \
}; \
MP_DEFINE_CONST_DICT(mod ## _globals_dict, mod ## _globals_dict_table); \
const mp_obj_module_t mod ## _module = { \
    .base = { &mp_type_module }, \
    .globals = (mp_obj_t)&mod ## _globals_dict, \
};

MODULE_UNAVAILABLE(cpufreq);
MODULE_UNAVAILABLE(fir);
MODULE_UNAVAILABLE(nn);
MODULE_UNAVAILABLE(lcd);
MODULE_UNAVAILABLE(sensor);

// Framebuffer implementation for Unix port using official framebuffer.c

// Static framebuffer memory for Unix port
// Main framebuffer: 1MB for large images (e.g., VGA RGB565 = ~614KB)
// Streaming buffer: 512KB for preview/streaming (can be disabled if not needed)
#define OMV_FB_MEMORY_SIZE (1024 * 1024)
#define OMV_SB_MEMORY_SIZE (512 * 1024)

// Provide linker symbols expected by framebuffer.c
// These act as start/end markers for framebuffer memory regions
char _fb_memory_start[OMV_FB_MEMORY_SIZE] __attribute__((aligned(32)));
char _fb_memory_end[0] __attribute__((aligned(1)));

char _sb_memory_start[OMV_SB_MEMORY_SIZE] __attribute__((aligned(32)));
char _sb_memory_end[0] __attribute__((aligned(1)));

// Stub for fb_alloc_sp() - used by framebuffer_resize for dynamic sizing
// Unix port uses static buffers, so return end of main framebuffer memory
char *fb_alloc_sp() {
    return _fb_memory_start + OMV_FB_MEMORY_SIZE;
}

// Note: framebuffer_update_preview() is provided by framebuffer.c
// It returns early if streaming buffer is disabled, so no override needed

// Fast math stubs - only include functions not already defined as macros
// Our fmath.h defines most as macros, but some files don't include it

#ifndef fast_atan2f
float fast_atan2f(float y, float x) {
    return atan2f(y, x);
}
#endif

#ifndef fast_log
float fast_log(float x) {
    return logf(x);
}
#endif

#ifndef fast_expf
float fast_expf(float x) {
    return expf(x);
}
#endif

#ifndef fast_cbrtf
float fast_cbrtf(float x) {
    return cbrtf(x);
}
#endif

// Unaligned memory copy - on Unix we can just use memcpy
void unaligned_memcpy(void *dst, const void *src, size_t n) {
    memcpy(dst, src, n);
}

// Note: fb_alloc functions are now properly implemented in fb_alloc.c for Unix port
// using GC-based allocation instead of stubs

// Note: m_free is provided by MicroPython's py/malloc.c when MICROPY_MALLOC_USES_ALLOCATED_SIZE=0

// Additional fast math function
#ifndef fast_atanf
float fast_atanf(float x) {
    return atanf(x);
}
#endif
