/**
 * Copyright (c) 2013 Ibrahim Abd Elkader <i.abdalkader@gmail.com>
 * See the file COPYING for copying permission.
 */
#include <stdlib.h>
#include <string.h>
#include "xalloc.h"
#include "array.h"
#define ARRAY_INIT_SIZE  10
struct array {
    int index;
    int length;
    void **data;
    array_dtor dtor;
};

void array_alloc(struct array **a, array_dtor dtor)
{
    struct array *array;
    array = xalloc(sizeof(struct array));
    array->index  = 0;
    array->length = ARRAY_INIT_SIZE;
    array->dtor   = dtor;
    array->data   = xalloc(array->length*sizeof(void*));
    *a = array;
}

void array_alloc_init(struct array **a, array_dtor dtor, int size)
{
    struct array *array;
    array = xalloc(sizeof(struct array));
    array->index  = 0;
    array->length = size;
    array->dtor   = dtor;
    array->data   = xalloc(array->length*sizeof(void*));
    *a = array;
}

void array_free(struct array *array)
{
    if (array->dtor != NULL) {
        for (int i=0; i<array->index; i++){
            array->dtor(array->data[i]);
        }
    }
    xfree(array->data);
    xfree(array);
}

int  array_length(struct array *array)
{
    return array->index;
}

void *array_at(struct array *array, int idx)
{
    return array->data[idx];
}

void array_push_back(struct array *array, void *element)
{
    if (array->index == array->length-1) {
        array->length += ARRAY_INIT_SIZE;
        array->data    = xrealloc(array->data, array->length * sizeof(void*));
    }
    array->data[array->index++] = element;
}

void *array_pop_back(struct array *array)
{
    void *el=NULL;
    if (array->index) {
        el = array->data[--array->index];

    }
    return el;
}

void array_erase(struct array *array, int idx)
{
    if (array->dtor) {
        array->dtor(array->data[idx]);
    }
    if (array->index >1 && idx < array->index){
        /* Since dst is always < src we can just use memcpy */
        memcpy(array->data+idx, array->data+idx+1, (array->index-idx-1) * sizeof(void*));
    }
    array->index--;
}

void array_resize(struct array *array, int idx)
{
    //TODO realloc
    while (array->index > idx) {
        if (array->dtor != NULL) {
            array->dtor(array->data[array->index-1]);
        }
        array->index--;
    }
}

void array_sort(struct array *array, array_comp comp)
{
    qsort(array->data, array->index, sizeof(void*), comp);
}

