/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Memory allocation functions.
 *
 */
#include <mp.h>
#include "xalloc.h"

NORETURN static void xalloc_fail()
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Out of normal MicroPython Heap Memory!"
        " Please reduce the resolution of the image you are running this algorithm on to bypass this issue!"));
}

// returns null pointer without error if size==0
void *xalloc(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (size && (mem == NULL)) {
        xalloc_fail();
    }
    return mem;
}

// returns null pointer without error if size==0
void *xalloc_try_alloc(uint32_t size)
{
    return gc_alloc(size, false);
}

// returns null pointer without error if size==0
void *xalloc0(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (size && (mem == NULL)) {
        xalloc_fail();
    }
    memset(mem, 0, size);
    return mem;
}

// returns without error if mem==null
void xfree(void *mem)
{
    gc_free(mem);
}

// returns null pointer without error if size==0
// allocs if mem==null and size!=0
// frees if mem!=null and size==0
void *xrealloc(void *mem, uint32_t size)
{
    mem = gc_realloc(mem, size, true);
    if (size && (mem == NULL)) {
        xalloc_fail();
    }
    return mem;
}
