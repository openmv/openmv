/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_POINT_H__
#define __UTILS_POINT_H__
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define INT16_T_MAX_VALUE 32768
#define INT16_T_MIN_VALUE -32768

typedef struct utils_point
{
    int16_t x, y;
}
utils_point_t;

void utils_point_init(utils_point_t *ptr, int x, int y);
void utils_point_copy(utils_point_t *dst, utils_point_t *src);
int utils_point_get_x(utils_point_t *ptr);
int utils_point_get_y(utils_point_t *ptr);
void utils_point_set_x(utils_point_t *ptr, int x);
void utils_point_set_y(utils_point_t *ptr, int y);
bool utils_point_equal(utils_point_t *ptr0, utils_point_t *ptr1);
size_t utils_point_quadrance(utils_point_t *ptr0, utils_point_t *ptr1);

#endif /* __UTILS_POINT_H__ */
