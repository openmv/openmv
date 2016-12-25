/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include "fb_alloc.h"
#include "utils_lifo.h"

void utils_lifo_alloc(utils_lifo_t *ptr, size_t size, size_t data_len)
{
    ptr->len = 0;
    ptr->size = size;
    ptr->data_len = data_len;
    ptr->data = (char *) fb_alloc(size * data_len);
}

void utils_lifo_free(utils_lifo_t *ptr)
{
    if (ptr->size * ptr->data_len) {
        fb_free();
    }
}

void utils_lifo_clear(utils_lifo_t *ptr)
{
    memset(ptr->data, 0, ptr->size * ptr->data_len);

    ptr->len = 0;
}

size_t utils_lifo_size(utils_lifo_t *ptr)
{
    return ptr->len;
}

bool utils_lifo_is_not_empty(utils_lifo_t *ptr)
{
    return ptr->len;
}

bool utils_lifo_is_not_full(utils_lifo_t *ptr)
{
    return ptr->len != ptr->size;
}

void utils_lifo_enqueue(utils_lifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);

    ptr->len += 1;
}

void utils_lifo_dequeue(utils_lifo_t *ptr, void *data)
{
    if (data) {
        memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
    }

    ptr->len -= 1;
}

void utils_lifo_peek(utils_lifo_t *ptr, void *data)
{
    memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
}

void utils_lifo_poke(utils_lifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);
}
