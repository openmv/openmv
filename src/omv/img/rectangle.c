/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Rectangle functions.
 *
 */
#include <float.h>
#include <limits.h>
#include <arm_math.h>
#include "imlib.h"
#include "array.h"
#include "xalloc.h"

rectangle_t *rectangle_alloc(int x, int y, int w, int h)
{
    rectangle_t *rectangle;
    rectangle = xalloc(sizeof(*rectangle));
    rectangle->x = x;
    rectangle->y = y;
    rectangle->w = w;
    rectangle->h = h;
    return rectangle;
}

rectangle_t *rectangle_clone(rectangle_t *rect)
{
    rectangle_t *rectangle;
    rectangle = xalloc(sizeof(rectangle_t));
    memcpy(rectangle, rect, sizeof(rectangle_t));
    return rectangle;
}

void rectangle_add(rectangle_t *rect0, rectangle_t *rect1)
{
    rect0->x += rect1->x;
    rect0->y += rect1->y;
    rect0->w += rect1->w;
    rect0->h += rect1->h;
}

void rectangle_div(rectangle_t *rect0, int c)
{
    rect0->x /= c;
    rect0->y /= c;
    rect0->w /= c;
    rect0->h /= c;
}

int rectangle_intersects(rectangle_t *rect0, rectangle_t *rect1)
{
    return  ((rect0->x < (rect1->x+rect1->w)) &&
             (rect0->y < (rect1->y+rect1->h)) &&
             ((rect0->x+rect0->w) > rect1->x) &&
             ((rect0->y+rect0->h) > rect1->y));
}

array_t *rectangle_merge(array_t *rectangles)
{
    int j;
    array_t *objects;
    array_t *overlap;
    rectangle_t *rect1, *rect2;

    array_alloc(&objects, xfree);
    array_alloc(&overlap, xfree);

    /* merge overlaping detections */
    while (array_length(rectangles)) {
        /* check for overlaping detections */
        rect1 = (rectangle_t *) array_at(rectangles, 0);
        for (j=1; j<array_length(rectangles); j++) {
            rect2 = (rectangle_t *) array_at(rectangles, j);
            if (rectangle_intersects(rect1, rect2)) {
                array_push_back(overlap, rectangle_clone(rect2));
                array_erase(rectangles, j--);
            }
        }

        /* add the overlaping detections */
        int count = array_length(overlap)+1;
        while (array_length(overlap)) {
            rect2 = (rectangle_t *) array_at(overlap, 0);
            rectangle_add(rect1, rect2);
            array_erase(overlap, 0);
        }

        /* average the overlaping detections */
        rectangle_div(rect1, count);
        array_push_back(objects, rectangle_clone(rect1));
        array_erase(rectangles, 0);
    }

    array_free(overlap);
    array_free(rectangles);
    return objects;
}


