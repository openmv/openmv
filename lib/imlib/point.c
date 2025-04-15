/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Point functions.
 */
#include "imlib.h"
#include "xalloc.h"

point_t *point_alloc(int16_t x, int16_t y) {
    point_t *p = xalloc(sizeof(point_t));
    p->x = x;
    p->y = y;
    return p;
}

bool point_equal(point_t *p1, point_t *p2) {
    return ((p1->x == p2->x) && (p1->y == p2->y));
}

float point_distance(point_t *p1, point_t *p2) {
    float sum = 0.0f;
    sum += (p1->x - p2->x) * (p1->x - p2->x);
    sum += (p1->y - p2->y) * (p1->y - p2->y);
    return fast_sqrtf(sum);
}
