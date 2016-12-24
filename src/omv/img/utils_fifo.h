/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_FIFO_H__
#define __UTILS_FIFO_H__
#include <stdbool.h>
#include <stddef.h>

typedef struct utils_fifo
{
    size_t head_ptr, tail_ptr, len, size, data_len;
    char *data;
}
utils_fifo_t;

void utils_fifo_alloc(utils_fifo_t *ptr, size_t size, size_t data_len); // Throws exception on failure.
void utils_fifo_free(utils_fifo_t *ptr); // Must be called in reverse alloc order.
void utils_fifo_clear(utils_fifo_t *ptr);
size_t utils_fifo_size(utils_fifo_t *ptr);
bool utils_fifo_is_not_empty(utils_fifo_t *ptr);
bool utils_fifo_is_not_full(utils_fifo_t *ptr);
void utils_fifo_enqueue(utils_fifo_t *ptr, void *data); // No overflow protection.
void utils_fifo_dequeue(utils_fifo_t *ptr, void *data); // No underflow protection.
void utils_fifo_peek(utils_fifo_t *ptr, void *data); // No empty protection.
void utils_fifo_poke(utils_fifo_t *ptr, void *data); // No empty protection.

#endif /* __UTILS_FIFO_H__ */
