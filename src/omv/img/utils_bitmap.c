/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include "fb_alloc.h"
#include "other_log2.h"
#include "utils_bitmap.h"

#define SIZE_T_BITS (sizeof(size_t) * 8)
#define SIZE_T_MASK (SIZE_T_BITS - 1)
#define SIZE_T_SHIFT OTHER_LOG2(SIZE_T_MASK)

void utils_bitmap_alloc(utils_bitmap_t *ptr, size_t size)
{
    ptr->len = 0;
    ptr->size = size;
    ptr->data = (size_t *) fb_alloc0(((size + SIZE_T_MASK) >> SIZE_T_SHIFT) * sizeof(size_t));
}

void utils_bitmap_free(utils_bitmap_t *ptr)
{
    if (((ptr->size + SIZE_T_MASK) >> SIZE_T_SHIFT) * sizeof(size_t)) {
        fb_free();
    }
}

void utils_bitmap_clear(utils_bitmap_t *ptr)
{
    memset(ptr->data, 0, ((ptr->size + SIZE_T_MASK) >> SIZE_T_SHIFT) * sizeof(size_t));

    ptr->len = 0;
}

size_t utils_bitmap_size(utils_bitmap_t *ptr)
{
    return ptr->len;
}

void utils_bitmap_bit_set(utils_bitmap_t *ptr, size_t index)
{
    size_t temp_mask = index & SIZE_T_MASK;
    size_t temp_shift = index >> SIZE_T_SHIFT;
    ptr->len += !((ptr->data[temp_shift] >> temp_mask) & 1);
    ptr->data[temp_shift] |= 1 << temp_mask;
}

void utils_bitmap_bit_set_known_clear(utils_bitmap_t *ptr, size_t index)
{
    ptr->len += 1;
    ptr->data[index >> SIZE_T_SHIFT] |= 1 << (index & SIZE_T_MASK);
}

void utils_bitmap_bit_clear(utils_bitmap_t *ptr, size_t index)
{
    size_t temp_mask = index & SIZE_T_MASK;
    size_t temp_shift = index >> SIZE_T_SHIFT;
    ptr->len -= (ptr->data[temp_shift] >> temp_mask) & 1;
    ptr->data[temp_shift] &= ~(1 << temp_mask);
}

void utils_bitmap_bit_clear_known_set(utils_bitmap_t *ptr, size_t index)
{
    ptr->len -= 1;
    ptr->data[index >> SIZE_T_SHIFT] &= ~(1 << (index & SIZE_T_MASK));
}

void utils_bitmap_bit_put(utils_bitmap_t *ptr, size_t index, bool value)
{
    value &= 1;
    size_t temp_mask = index & SIZE_T_MASK;
    size_t temp_shift = index >> SIZE_T_SHIFT;
    if (value) {
        ptr->len += !((ptr->data[temp_shift] >> temp_mask) & 1);
    } else {
        ptr->len -= (ptr->data[temp_shift] >> temp_mask) & 1;
    }
    ptr->data[temp_shift] = (ptr->data[temp_shift] & (~(1 << temp_mask))) | (value << temp_mask);
}

void utils_bitmap_bit_put_known_clear(utils_bitmap_t *ptr, size_t index, bool value)
{
    value &= 1;
    size_t temp_mask = index & SIZE_T_MASK;
    size_t temp_shift = index >> SIZE_T_SHIFT;
    ptr->len += value;
    ptr->data[temp_shift] = (ptr->data[temp_shift] & (~(1 << temp_mask))) | (value << temp_mask);
}

void utils_bitmap_bit_put_known_set(utils_bitmap_t *ptr, size_t index, bool value)
{
    value &= 1;
    size_t temp_mask = index & SIZE_T_MASK;
    size_t temp_shift = index >> SIZE_T_SHIFT;
    ptr->len -= !value;
    ptr->data[temp_shift] = (ptr->data[temp_shift] & (~(1 << temp_mask))) | (value << temp_mask);
}

bool utils_bitmap_bit_get(utils_bitmap_t *ptr, size_t index)
{
    return (ptr->data[index >> SIZE_T_SHIFT] >> (index & SIZE_T_MASK)) & 1;
}
