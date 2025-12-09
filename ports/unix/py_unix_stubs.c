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
#include "omv_boardconfig.h"

#define MODULE_UNAVAILABLE(mod)                                                           \
    static const mp_map_elem_t mod ## _globals_dict_table[] = {                           \
        { MP_OBJ_NEW_QSTR(MP_QSTR___name__),    MP_OBJ_NEW_QSTR(MP_QSTR_ ## mod)  },      \
        { MP_OBJ_NEW_QSTR(MP_QSTR___init__),    (mp_obj_t) &py_func_unavailable_obj    }, \
    };                                                                                    \
    MP_DEFINE_CONST_DICT(mod ## _globals_dict, mod ## _globals_dict_table);               \
    const mp_obj_module_t mod ## _module = {                                              \
        .base = { &mp_type_module },                                                      \
        .globals = (mp_obj_t) &mod ## _globals_dict,                                      \
    };

MODULE_UNAVAILABLE(cpufreq);
MODULE_UNAVAILABLE(fir);
MODULE_UNAVAILABLE(nn);
MODULE_UNAVAILABLE(lcd);
MODULE_UNAVAILABLE(sensor);

// Framebuffer implementation for Unix port using official framebuffer.c

// Memory layout for Unix port:
// All OpenMV memory regions must be contiguous for fb_alloc to work correctly.
// We use a single struct to ensure proper layout and alignment.

#define OMV_FB_MEMORY_SIZE (1024 * 1024)    // 1MB main framebuffer
#define OMV_SB_MEMORY_SIZE (512 * 1024)     // 512KB streaming buffer
#define OMV_FB_ALLOC_SIZE (2 * 1024 * 1024) // 2MB fb_alloc temp buffer

// Macro stringification helper for asm
#define STR(x) STR2(x)
#define STR2(x) #x

// Static buffer allocation for Unix port
// IMPORTANT: fb_alloc.c does pointer arithmetic between framebuffer and fb_alloc regions,
// so they MUST be contiguous in memory. We use a struct to guarantee this.

struct __attribute__((aligned(32))) {
    char fb_memory[OMV_FB_MEMORY_SIZE];      // Main framebuffer
    char sb_memory[OMV_SB_MEMORY_SIZE];      // Streaming buffer
    char fb_alloc_memory[OMV_FB_ALLOC_SIZE]; // fb_alloc temp buffer
} omv_memory_region = {0};

// Provide _fb_alloc_end symbol expected by common/fb_alloc.c
// Unix version is a pointer variable; embedded uses linker symbol
char *_fb_alloc_end = omv_memory_region.fb_alloc_memory + OMV_FB_ALLOC_SIZE;

// Verify memory regions are contiguous (required for fb_alloc pointer arithmetic)
// The fb_alloc code does pointer arithmetic between framebuffer_pool_end() and _fb_alloc_end,
// which requires these regions to be in contiguous memory with no padding.
_Static_assert(offsetof(typeof(omv_memory_region), sb_memory) == OMV_FB_MEMORY_SIZE,
               "Memory regions must be contiguous - struct padding detected between fb_memory and sb_memory");
_Static_assert(offsetof(typeof(omv_memory_region), fb_alloc_memory) == OMV_FB_MEMORY_SIZE + OMV_SB_MEMORY_SIZE,
               "Memory regions must be contiguous - struct padding detected between sb_memory and fb_alloc_memory");

// Note: framebuffer_update_preview() is provided by framebuffer.c
// It returns early if streaming buffer is disabled, so no override needed
// Note: fb_alloc functions are provided by common/fb_alloc.c

// Fast math functions are now provided by lib/imlib/fmath.c
// This matches embedded behavior for algorithm testing consistency
// Note: fast_sqrtf, fast_floorf, fast_ceilf, fast_roundf, fast_fabsf are inline in fmath.h

// Unaligned memory copy - on Unix we can just use memcpy
void unaligned_memcpy(void *dst, const void *src, size_t n) {
    memcpy(dst, src, n);
}

// Override framebuffer_init0() for Unix port
// The default implementation uses linker symbols (&_fb_memory_start) which don't exist on Unix.
// Instead, we directly use the omv_memory_region struct members.
void framebuffer_init0() __attribute__((used));
void framebuffer_init0() {
    extern void framebuffer_init(framebuffer_t *fb, void *buff, size_t size, bool dynamic, bool enabled);
    extern framebuffer_t *framebuffer_get(size_t id);

    // Reuse the last enabled flag after resetting the state
    bool enabled = framebuffer_get(FB_STREAM_ID)->enabled;

    // Initialize the main framebuffer using struct members directly
    framebuffer_init(framebuffer_get(FB_MAINFB_ID),
                     omv_memory_region.fb_memory,
                     OMV_FB_MEMORY_SIZE, false, true);

    // Initialize the streaming buffer
    framebuffer_init(framebuffer_get(FB_STREAM_ID),
                     omv_memory_region.sb_memory,
                     OMV_SB_MEMORY_SIZE, false, enabled);

    // Reset embedded header
    framebuffer_t *fb = framebuffer_get(FB_STREAM_ID);
    memset(fb->raw_base, 0, sizeof(framebuffer_header_t));
}

// Initialize framebuffer and fb_alloc subsystems for Unix port
// This is called automatically at library load time using GCC constructor attribute
__attribute__((constructor))
static void omv_unix_init(void) {
    extern void fb_alloc_init0(void);

    fb_alloc_init0();
    framebuffer_init0();
}

// Note: m_free is provided by MicroPython's py/malloc.c when MICROPY_MALLOC_USES_ALLOCATED_SIZE=0
