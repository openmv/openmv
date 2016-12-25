/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_LIFO_H__
#define __UTILS_LIFO_H__
#include <stdbool.h>
#include <stddef.h>

typedef struct utils_lifo
{
    size_t len, size, data_len;
    char *data;
}
utils_lifo_t;

void utils_lifo_alloc(utils_lifo_t *ptr, size_t size, size_t data_len); // Throws exception on failure.
void utils_lifo_free(utils_lifo_t *ptr); // Must be called in reverse alloc order.
void utils_lifo_clear(utils_lifo_t *ptr);
size_t utils_lifo_size(utils_lifo_t *ptr);
bool utils_lifo_is_not_empty(utils_lifo_t *ptr);
bool utils_lifo_is_not_full(utils_lifo_t *ptr);
void utils_lifo_enqueue(utils_lifo_t *ptr, void *data); // No overflow protection.
void utils_lifo_dequeue(utils_lifo_t *ptr, void *data); // No underflow protection.
void utils_lifo_peek(utils_lifo_t *ptr, void *data); // No empty protection.
void utils_lifo_poke(utils_lifo_t *ptr, void *data); // No full protection.

#endif /* __UTILS_LIFO_H__ */
