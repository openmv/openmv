/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include "other_maxmin.h"
#include "utils_rectangle.h"

void utils_rectangle_init(utils_rectangle_t *ptr, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    ptr->p.x = x;
    ptr->p.y = y;
    ptr->s.w = w;
    ptr->s.h = h;
}

void utils_rectangle_copy(utils_rectangle_t *dst, utils_rectangle_t *src)
{
    memcpy(dst, src, sizeof(utils_rectangle_t));
}

unsigned int utils_rectangle_get_x(utils_rectangle_t *ptr)
{
    return ptr->p.x;
}

unsigned int utils_rectangle_get_y(utils_rectangle_t *ptr)
{
    return ptr->p.y;
}

unsigned int utils_rectangle_get_w(utils_rectangle_t *ptr)
{
    return ptr->s.w;
}

unsigned int utils_rectangle_get_h(utils_rectangle_t *ptr)
{
    return ptr->s.h;
}

void utils_rectangle_set_x(utils_rectangle_t *ptr, unsigned int x)
{
    ptr->p.x = x;
}

void utils_rectangle_set_y(utils_rectangle_t *ptr, unsigned int y)
{
    ptr->p.y = y;
}

void utils_rectangle_set_w(utils_rectangle_t *ptr, unsigned int w)
{
    ptr->s.w = w;
}

void utils_rectangle_set_h(utils_rectangle_t *ptr, unsigned int h)
{
    ptr->s.h = h;
}

bool utils_rectangle_equal(utils_rectangle_t *ptr0, utils_rectangle_t *ptr1)
{
    unsigned int x0 = ptr0->p.x, y0 = ptr0->p.y, w0 = ptr0->s.w, h0 = ptr0->s.h, x1 = ptr1->p.x, y1 = ptr1->p.y, w1 = ptr1->s.w, h1 = ptr1->s.h;
    return (x0 == x1) && (y0 == y1) && (w0 == w1) && (h0 == h1);
}

bool utils_rectangle_overlap(utils_rectangle_t *ptr0, utils_rectangle_t *ptr1)
{
    unsigned int x0 = ptr0->p.x, y0 = ptr0->p.y, w0 = ptr0->s.w, h0 = ptr0->s.h, x1 = ptr1->p.x, y1 = ptr1->p.y, w1 = ptr1->s.w, h1 = ptr1->s.h;
    return (x0 < (x1 + w1)) && (y0 < (y1 + h1)) && (x1 < (x0 + w0)) && (y1 < (y0 + h0));
}

void utils_rectangle_intersected(utils_rectangle_t *dst, utils_rectangle_t *src)
{
    unsigned int leftX = OTHER_MAX(dst->p.x, src->p.x), topY = OTHER_MAX(dst->p.y, src->p.y);
    unsigned int rightX = OTHER_MIN(dst->p.x + dst->s.w, src->p.x + src->s.w), bottomY = OTHER_MIN(dst->p.y + dst->s.h, src->p.y + src->s.h);
    dst->p.x = leftX;
    dst->p.y = topY;
    dst->s.w = rightX - leftX;
    dst->s.h = bottomY - topY;
}

void utils_rectangle_united(utils_rectangle_t *dst, utils_rectangle_t *src)
{
    unsigned int leftX = OTHER_MIN(dst->p.x, src->p.x), topY = OTHER_MIN(dst->p.y, src->p.y);
    unsigned int rightX = OTHER_MAX(dst->p.x + dst->s.w, src->p.x + src->s.w), bottomY = OTHER_MAX(dst->p.y + dst->s.h, src->p.y + src->s.h);
    dst->p.x = leftX;
    dst->p.y = topY;
    dst->s.w = rightX - leftX;
    dst->s.h = bottomY - topY;
}
