/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Interface for using extra frame buffer RAM as a stack.
 *
 */
#include <mp.h>
#include "framebuffer.h"
#include "fb_stack.h"

extern char _fs_cache;
static char *pointer = &_fs_cache;

NORETURN static void fb_alloc_fail()
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "Out of Memory!!!"));
}

void fb_init() {
    pointer = &_fs_cache;
}

// returns null pointer without error if size==0
void *fb_alloc(uint32_t size) {
    if (!size) {
        return NULL;
    }
    size=((size+sizeof(uint32_t)-1)/sizeof(uint32_t))*sizeof(uint32_t);// Round Up
    char *result = pointer - size;
    char *new_pointer = result - sizeof(uint32_t);
    char *end_pointer = (char *)fb->pixels; // NULL (fb->bpp <= 0)
    if ((fb->bpp == 1) || (fb->bpp == 2)) { // GRAYSCALE OR RGB565
        end_pointer += fb->w * fb->w * fb->bpp;
    } else if (fb->bpp >= 3) { // JPEG
        end_pointer += fb->bpp;
    }
    if (new_pointer < end_pointer) {
        fb_alloc_fail();
    }
    *((uint32_t *) new_pointer) = size + sizeof(uint32_t); // Save size.
    pointer = new_pointer;
    return result;
}

// returns null pointer without error if size==0
void *fb_alloc0(uint32_t size) {
    void *mem = fb_alloc(size);
    memset(mem, 0, size);
    return mem;
}

void fb_free() {
    if (pointer < &_fs_cache) {
        pointer += *((uint32_t *) pointer); // Get size and pop.
    }
}
