/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_RECTANGLE_H__
#define __UTILS_RECTANGLE_H__
#include "utils_point.h"
#include "utils_size.h"

typedef struct utils_rectangle
{
    utils_point_t p;
    utils_size_t s;
}
utils_rectangle_t;

void utils_rectangle_init(utils_rectangle_t *ptr, int x, int y, unsigned int w, unsigned int h);
void utils_rectangle_copy(utils_rectangle_t *dst, utils_rectangle_t *src);
int utils_rectangle_get_x(utils_rectangle_t *ptr);
int utils_rectangle_get_y(utils_rectangle_t *ptr);
unsigned int utils_rectangle_get_w(utils_rectangle_t *ptr);
unsigned int utils_rectangle_get_h(utils_rectangle_t *ptr);
void utils_rectangle_set_x(utils_rectangle_t *ptr, int x);
void utils_rectangle_set_y(utils_rectangle_t *ptr, int y);
void utils_rectangle_set_w(utils_rectangle_t *ptr, unsigned int w);
void utils_rectangle_set_h(utils_rectangle_t *ptr, unsigned int h);
bool utils_rectangle_equal(utils_rectangle_t *ptr0, utils_rectangle_t *ptr1);
bool utils_rectangle_overlap(utils_rectangle_t *ptr0, utils_rectangle_t *ptr1);
void utils_rectangle_intersected(utils_rectangle_t *dst, utils_rectangle_t *src);
void utils_rectangle_united(utils_rectangle_t *dst, utils_rectangle_t *src);

#endif /* __UTILS_RECTANGLE_H__ */
