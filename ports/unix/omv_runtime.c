/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Unix OpenMV runtime.
 */
#include "py/runtime.h"
#include "py/gc.h"
#include "framebuffer.h"
#include "imlib.h"
#include "board_config.h"
#include "umalloc.h"

static char __attribute__((aligned(32))) omv_sb_memory[OMV_SB_MEMORY_SIZE];
static char __attribute__((aligned(32))) omv_uma_memory[OMV_UMA_MEMORY_SIZE];
static bool omv_unix_active;

void *framebuffer_get_stream_buffer(size_t *size) {
    *size = sizeof(omv_sb_memory);
    return omv_sb_memory;
}

void omv_unix_init(void) {
    if (omv_unix_active) {
        return;
    }
    uma_init();
    uma_pool_add(omv_uma_memory, sizeof(omv_uma_memory), UMA_DEFAULT);
    imlib_init();
    imlib_last_poll_cyc = omv_cycles_now();
    framebuffer_init0();
    omv_unix_active = true;
}

void omv_unix_deinit(void) {
    if (!omv_unix_active) {
        return;
    }
    // VM finalisers can own persistent UMA blocks and need the live allocator.
    gc_sweep_all();
    imlib_deinit();
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    uma_free(fb->raw_base);
    fb->raw_base = NULL;
    framebuffer_init0();
    uma_collect();
    uma_init();
    omv_unix_active = false;
}
