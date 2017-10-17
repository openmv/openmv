/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Interface for using extra frame buffer RAM as a stack.
 *
 */
#include <mp.h>
#include "fb_alloc.h"
#include "framebuffer.h"

extern char _fballoc;
static char *pointer = &_fballoc;

NORETURN void fb_alloc_fail()
{
    nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError, "FB Alloc Collision!!!"));
}

void fb_alloc_init0()
{
    pointer = &_fballoc;
}

uint32_t fb_avail()
{
    int32_t temp = pointer - ((char *) MAIN_FB_PIXELS()) - sizeof(uint32_t);

    return (temp < sizeof(uint32_t)) ? 0 : temp;
}

void fb_alloc_mark()
{
    char *new_pointer = pointer - sizeof(uint32_t);

    // Check if allocation overwrites the framebuffer pixels
    if (new_pointer < (char *) MAIN_FB_PIXELS()) {
        nlr_raise_for_fb_alloc_mark(mp_obj_new_exception_msg(&mp_type_MemoryError, "FB Alloc Collision!!!"));
    }

    // fb_alloc does not allow regions which are a size of 0 to be alloced,
    // meaning that the value below is always 8 or more but never 4. So,
    // we will use a size value of 4 as a marker in the alloc stack.
    *((uint32_t *) new_pointer) = sizeof(uint32_t); // Save size.
    pointer = new_pointer;
}

void fb_alloc_free_till_mark()
{
    while (pointer < &_fballoc) {
        int size = *((uint32_t *) pointer);
        pointer += size; // Get size and pop.
        if (size == sizeof(uint32_t)) break; // Break on first marker.
    }
}

// returns null pointer without error if size==0
void *fb_alloc(uint32_t size)
{
    if (!size) {
        return NULL;
    }

    size=((size+sizeof(uint32_t)-1)/sizeof(uint32_t))*sizeof(uint32_t);// Round Up
    char *result = pointer - size;
    char *new_pointer = result - sizeof(uint32_t);

    // Check if allocation overwrites the framebuffer pixels
    if (new_pointer < (char *) MAIN_FB_PIXELS()) {
        fb_alloc_fail();
    }

    // size is always 4/8/12/etc. so the value below must be 8 or more.
    *((uint32_t *) new_pointer) = size + sizeof(uint32_t); // Save size.
    pointer = new_pointer;
    return result;
}

// returns null pointer without error if passed size==0
void *fb_alloc0(uint32_t size)
{
    void *mem = fb_alloc(size);
    memset(mem, 0, size); // does nothing if size is zero.
    return mem;
}

void *fb_alloc_all(uint32_t *size)
{
    int32_t temp = pointer - ((char *) MAIN_FB_PIXELS()) - sizeof(uint32_t);

    if (temp < sizeof(uint32_t)) {
        *size = 0;
        return NULL;
    }

    *size = (temp / sizeof(uint32_t)) * sizeof(uint32_t); // Round Down
    char *result = pointer - *size;
    char *new_pointer = result - sizeof(uint32_t);

    // size is always 4/8/12/etc. so the value below must be 8 or more.
    *((uint32_t *) new_pointer) = *size + sizeof(uint32_t); // Save size.
    pointer = new_pointer;
    return result;
}

// returns null pointer without error if returned size==0
void *fb_alloc0_all(uint32_t *size)
{
    void *mem = fb_alloc_all(size);
    memset(mem, 0, *size); // does nothing if size is zero.
    return mem;
}

void fb_free()
{
    if (pointer < &_fballoc) {
        pointer += *((uint32_t *) pointer); // Get size and pop.
    }
}

void fb_free_all()
{
    while (pointer < &_fballoc) {
        pointer += *((uint32_t *) pointer); // Get size and pop.
    }
}
