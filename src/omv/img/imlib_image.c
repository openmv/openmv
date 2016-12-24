/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <mp.h>
#include <string.h>
#include "imlib_image.h"

void imlib_image_init(imlib_image_t *ptr, imlib_image_type_t type, utils_size_t *geometry)
{
    ptr->geometry.w = geometry->w;
    ptr->geometry.h = geometry->h;
    ptr->type = type;
    ptr->size = 0;
    ptr->data = NULL;
}

void imlib_image_copy(imlib_image_t *dst, imlib_image_t *src)
{
    memcpy(dst, src, sizeof(imlib_image_t));
}

void imlib_image_check_overlap(imlib_image_t *ptr, utils_rectangle_t *rect)
{
    utils_rectangle_t temp;
    temp.p.x = 0;
    temp.p.y = 0;
    temp.s.w = ptr->geometry.w;
    temp.s.h = ptr->geometry.h;
    if (!utils_rectangle_overlap(rect, &temp)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Invalid ROI!"));
    }
}

void imlib_image_intersected(imlib_image_t *ptr, utils_rectangle_t *rect)
{
    utils_rectangle_t temp;
    temp.p.x = 0;
    temp.p.y = 0;
    temp.s.w = ptr->geometry.w;
    temp.s.h = ptr->geometry.h;
    utils_rectangle_intersected(rect, &temp);
}
