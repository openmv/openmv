/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Memory allocation functions.
 *
 */
#include <mp.h>
#include "mdefs.h"
#include "xalloc.h"

void *xalloc_fail()
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Out of Memory!!"));
    return NULL;
}

void *xalloc(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (mem == NULL) {
        return xalloc_fail();
    }
    return mem;
}

void *xalloc0(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (mem == NULL) {
        return xalloc_fail();
    }
    memset(mem, 0, size);
    return mem;
}

void xfree(void *ptr)
{
    gc_free(ptr);
}

void *xrealloc(void *ptr, uint32_t size)
{
    ptr = gc_realloc(ptr, size);
    if (ptr == NULL) {
        return xalloc_fail();
    }
    return ptr;
}

