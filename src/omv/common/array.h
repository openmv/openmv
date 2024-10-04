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
 * Dynamic array.
 */
#ifndef __ARRAY_H__
#define __ARRAY_H__
typedef void (*array_dtor_t) (void *);
typedef int (*array_comp_t) (const void *, const void *);
// (left < right) == negative
// (left == right) == zero
// (left > right) == positive
typedef struct {
    int index;
    int length;
    void **data;
    array_dtor_t dtor;
} array_t;
void array_alloc(array_t **a, array_dtor_t dtor);
void array_alloc_init(array_t **a, array_dtor_t dtor, int size);
void array_clear(array_t *array);
void array_free(array_t *array);
int array_length(array_t *array);
void *array_at(array_t *array, int idx);
void array_push_back(array_t *array, void *element);
void *array_pop_back(array_t *array);
void *array_take(array_t *array, int idx);
void array_erase(array_t *array, int idx);
void array_resize(array_t *array, int num);
void array_sort(array_t *array, array_comp_t comp);
void array_isort(array_t *array, array_comp_t comp);
#endif //__ARRAY_H__
