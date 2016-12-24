/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include "fb_alloc.h"
#include "utils_fifo.h"

void utils_fifo_alloc(utils_fifo_t *ptr, size_t size, size_t data_len)
{
    ptr->head_ptr = 0;
    ptr->tail_ptr = 0;
    ptr->len = 0;
    ptr->size = size;
    ptr->data_len = data_len;
    ptr->data = (char *) fb_alloc(size * data_len);
}

void utils_fifo_free(utils_fifo_t *ptr)
{
    if (ptr->size * ptr->data_len) {
        fb_free();
    }
}

void utils_fifo_clear(utils_fifo_t *ptr)
{
    memset(ptr->data, 0, ptr->size * ptr->data_len);

    ptr->head_ptr = 0;
    ptr->tail_ptr = 0;
    ptr->len = 0;
}

size_t utils_fifo_size(utils_fifo_t *ptr)
{
    return ptr->len;
}

bool utils_fifo_is_not_empty(utils_fifo_t *ptr)
{
    return ptr->len;
}

bool utils_fifo_is_not_full(utils_fifo_t *ptr)
{
    return ptr->len != ptr->size;
}

void utils_fifo_enqueue(utils_fifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->head_ptr * ptr->data_len), data, ptr->data_len);

    size_t temp = ptr->head_ptr + 1;

    if (temp == ptr->size) {
        temp = 0;
    }

    ptr->head_ptr = temp;
    ptr->len += 1;
}

void utils_fifo_dequeue(utils_fifo_t *ptr, void *data)
{
    if (data) {
        memcpy(data, ptr->data + (ptr->tail_ptr * ptr->data_len), ptr->data_len);
    }

    size_t temp = ptr->tail_ptr + 1;

    if (temp == ptr->size) {
        temp = 0;
    }

    ptr->tail_ptr = temp;
    ptr->len -= 1;
}

void utils_fifo_peek(utils_fifo_t *ptr, void *data)
{
    memcpy(data, ptr->data + (ptr->tail_ptr * ptr->data_len), ptr->data_len);
}

void utils_fifo_poke(utils_fifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->head_ptr * ptr->data_len), data, ptr->data_len);
}
