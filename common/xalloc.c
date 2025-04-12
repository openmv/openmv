/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * Memory allocation functions.
 */
#include <string.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "xalloc.h"

NORETURN static void xalloc_fail(uint32_t size) {
    mp_raise_msg_varg(&mp_type_MemoryError,
                      MP_ERROR_TEXT("memory allocation failed, allocating %u bytes"), (uint) size);
}

// returns null pointer without error if size==0
void *xalloc(uint32_t size) {
    void *mem = gc_alloc(size, false);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    return mem;
}

// returns null pointer without error if size==0
void *xalloc_try_alloc(uint32_t size) {
    return gc_alloc(size, false);
}

// returns null pointer without error if size==0
void *xalloc0(uint32_t size) {
    void *mem = gc_alloc(size, false);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    memset(mem, 0, size);
    return mem;
}

// returns without error if mem==null
void xfree(void *mem) {
    gc_free(mem);
}

// returns null pointer without error if size==0
// allocs if mem==null and size!=0
// frees if mem!=null and size==0
void *xrealloc(void *mem, uint32_t size) {
    mem = gc_realloc(mem, size, true);
    if (size && (mem == NULL)) {
        xalloc_fail(size);
    }
    return mem;
}
