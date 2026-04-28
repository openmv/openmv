/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Unix port stubs: UMA pool, framebuffer init, and embedded-only
 * symbol stubs needed at link time. Init runs from a GCC constructor
 * because the unix port piggybacks on MicroPython's own main().
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "py/runtime.h"
#include "py/gc.h"

#include "board_config.h"
#include "framebuffer.h"
#include "imlib.h"
#include "umalloc.h"

// Linker-symbol stubs for embedded code paths that link but don't run
// on host: the weak framebuffer_init0() in lib/imlib/framebuffer.c
// references &_sb_memory_start, and apriltag_config.h's stack-
// availability check reads _sstack. The unix port doesn't call
// framebuffer_init0() (init below uses framebuffer_init directly), so
// these are pure link-time satisfiers.
char _sb_memory_start;
char _sb_memory_end;
char _sstack;

// DWT cycle counter stub for common/omv_cycles.h. Always reads zero.
static _omv_unix_dwt_t omv_unix_dwt_stub;
_omv_unix_dwt_t *DWT = &omv_unix_dwt_stub;

void omv_cycles_init(void) {
}

static char *omv_unix_uma_pool;
static bool omv_unix_init_done;

// Fail with a clear message and exit cleanly (avoids SIGABRT/core
// dumps in CI for a known-bad startup state).
static void omv_unix_die(const char *msg) {
    fprintf(stderr, "openmv (pid %d): %s\n", (int) getpid(), msg);
    fflush(stderr);
    _exit(1);
}

// Sanity-check the saturating-arithmetic intrinsic stubs at startup.
// __USAT_ASR is easy to write with the argument order swapped, which
// silently corrupts filter output on host. Trap that here.
static void omv_unix_intrinsic_self_check(void) {
    if (__USAT(0xFFFF, 8) != 0xFFu ||
        __USAT(-1,     8) != 0u ||
        __USAT(0x7F,   8) != 0x7Fu ||
        __USAT(-1,    32) != 0u) {
        omv_unix_die("__USAT sanity check failed");
    }
    if (__USAT_ASR(0xFFFF, 8, 0) != 0xFFu ||
        __USAT_ASR(0x100,  8, 8) != 1u ||
        __USAT_ASR(0xFF00, 8, 8) != 0xFFu) {
        omv_unix_die("__USAT_ASR sanity check failed "
                     "(parameter order regression?)");
    }
}

__attribute__((constructor))
static void omv_unix_init(void) {
    if (omv_unix_init_done) {
        return;
    }
    omv_unix_init_done = true;

    omv_unix_intrinsic_self_check();

    omv_unix_uma_pool = malloc(OMV_UNIX_UMA_POOL_SIZE);
    if (omv_unix_uma_pool == NULL) {
        omv_unix_die("failed to allocate UMA pool");
    }
    memset(omv_unix_uma_pool, 0, OMV_UNIX_UMA_POOL_SIZE);

    // flags=0 registers as a generic pool; uma_pool_find()'s fallback
    // selects it for any allocation regardless of attributes.
    uma_init();
    uma_pool_add(omv_unix_uma_pool, OMV_UNIX_UMA_POOL_SIZE, 0);

    framebuffer_init(framebuffer_get(FB_MAINFB_ID), NULL, 0, true, true);
    framebuffer_init(framebuffer_get(FB_STREAM_ID), NULL, 0, false, false);

    imlib_init();
}

// Pair the constructor with a destructor so dlopen/dlclose cycles or
// explicit mp_init/mp_deinit from an embedder don't leak the pool.
__attribute__((destructor))
static void omv_unix_deinit(void) {
    if (!omv_unix_init_done) {
        return;
    }
    free(omv_unix_uma_pool);
    omv_unix_uma_pool = NULL;
    omv_unix_init_done = false;
}
