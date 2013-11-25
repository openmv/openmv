/**
 * Copyright (c) 2013 Ibrahim Abd Elkader <i.abdalkader@gmail.com> 
 * See the file COPYING for copying permission.
 */
#ifndef __ARRAY_H__
#define __ARRAY_H__
struct array;
typedef void (*array_dtor)(void*);
typedef int  (*array_comp)(const void*, const void*);
void array_alloc(struct array **array, array_dtor dtor);
void array_free(struct array *array);
int array_length(struct array *array);
void array_push_back(struct array *array, void *value);
void *array_at(struct array *array, int index);
void array_sort(struct array *array, array_comp);
void array_erase(struct array *array, int index);
void array_resize(struct array *array, int index);
#endif//__ARRAY_H__
