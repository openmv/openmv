/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include <mp.h>
#include "utils_size.h"

void utils_size_init(utils_size_t *ptr, unsigned int w, unsigned int h)
{
    ptr->w = w;
    ptr->h = h;
}

void utils_size_copy(utils_size_t *dst, utils_size_t *src)
{
    memcpy(dst, src, sizeof(utils_size_t));
}

unsigned int utils_size_get_w(utils_size_t *ptr)
{
    return ptr->w;
}

unsigned int utils_size_get_h(utils_size_t *ptr)
{
    return ptr->h;
}

void utils_size_set_w(utils_size_t *ptr, unsigned int w)
{
    ptr->w = w;
}

void utils_size_set_h(utils_size_t *ptr, unsigned int h)
{
    ptr->h = h;
}

bool utils_size_equal(utils_size_t *ptr0, utils_size_t *ptr1)
{
    unsigned int w0 = ptr0->w, h0 = ptr0->h, w1 = ptr1->w, h1 = ptr1->h;
    return (w0 == w1) && (h0 == h1);
}

void utils_size_check(utils_size_t *ptr)
{
    if ((!ptr->w) || (!ptr->h)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Null Size!"));
    }
}
