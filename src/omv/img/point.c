/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * X,Y Point.
 *
 */
#include <float.h>
#include <limits.h>
#include <arm_math.h>
#include "imlib.h"
#include "array.h"
#include "xalloc.h"

point_t *point_alloc(int x, int y)
{
    point_t *p = xalloc(sizeof(*p));
    if (p != NULL) {
        p->x = x;
        p->y = y;
    }
    return p;
}

int point_equal(point_t *p1, point_t *p2)
{
    return ((p1->x==p2->x)&&(p1->y==p2->y));
}

float point_distance(point_t *p1, point_t *p2)
{
    float sum=0.0f;
    sum += (p1->x - p2->x) * (p1->x - p2->x);
    sum += (p1->y - p2->y) * (p1->y - p2->y);
    return fast_sqrtf(sum);
}
