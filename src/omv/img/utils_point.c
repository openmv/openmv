/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include "utils_point.h"

void utils_point_init(utils_point_t *ptr, int x, int y)
{
    ptr->x = x;
    ptr->y = y;
}

void utils_point_copy(utils_point_t *dst, utils_point_t *src)
{
    memcpy(dst, src, sizeof(utils_point_t));
}

int utils_point_get_x(utils_point_t *ptr)
{
    return ptr->x;
}

int utils_point_get_y(utils_point_t *ptr)
{
    return ptr->y;
}

void utils_point_set_x(utils_point_t *ptr, int x)
{
    ptr->x = x;
}

void utils_point_set_y(utils_point_t *ptr, int y)
{
    ptr->y = y;
}

bool utils_point_equal(utils_point_t *ptr0, utils_point_t *ptr1)
{
    int x0 = ptr0->x, y0 = ptr0->y, x1 = ptr1->x, y1 = ptr1->y;
    return (x0 == x1) && (y0 == y1);
}

size_t utils_point_quadrance(utils_point_t *ptr0, utils_point_t *ptr1)
{
    int x0 = ptr0->x, y0 = ptr0->y, x1 = ptr1->x, y1 = ptr1->y;
    int delta_x = x0 - x1, delta_y = y0 - y1;
    return (delta_x * delta_x) + (delta_y * delta_y);
}
