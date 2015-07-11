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

mp_obj_t oom_interrupt;
void xalloc_init()
{
    oom_interrupt= mp_obj_new_exception_msg(&mp_type_OSError, "Out of Memory!!");
}

void *xalloc(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (mem == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Out of Memory!!"));
    }
    return mem;
}

void *xalloc0(uint32_t size)
{
    void *mem = gc_alloc(size, false);
    if (mem == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Out of Memory!!"));
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
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Out of Memory!!"));
    }
    return ptr;
}

