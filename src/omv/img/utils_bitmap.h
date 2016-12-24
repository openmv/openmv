/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_BITMAP_H__
#define __UTILS_BITMAP_H__
#include <stdbool.h>
#include <stddef.h>

typedef struct utils_bitmap
{
    size_t len, size;
    size_t *data;
}
utils_bitmap_t;

void utils_bitmap_alloc(utils_bitmap_t *ptr, size_t size); // Throws exception on failure.
void utils_bitmap_free(utils_bitmap_t *ptr); // Must be called in reverse alloc order.
void utils_bitmap_clear(utils_bitmap_t *ptr);
size_t utils_bitmap_size(utils_bitmap_t *ptr);
void utils_bitmap_bit_set(utils_bitmap_t *ptr, size_t index); // No out-of-bounds protection.
void utils_bitmap_bit_set_known_clear(utils_bitmap_t *ptr, size_t index); // No out-of-bounds protection.
void utils_bitmap_bit_clear(utils_bitmap_t *ptr, size_t index); // No out-of-bounds protection.
void utils_bitmap_bit_clear_known_set(utils_bitmap_t *ptr, size_t index); // No out-of-bounds protection.
void utils_bitmap_bit_put(utils_bitmap_t *ptr, size_t index, bool value); // No out-of-bounds protection.
bool utils_bitmap_bit_get(utils_bitmap_t *ptr, size_t index); // No out-of-bounds protection.

#endif /* __UTILS_BITMAP_H__ */
