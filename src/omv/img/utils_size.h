/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_SIZE_H__
#define __UTILS_SIZE_H__
#include <stdbool.h>
#include <stdint.h>
#define UINT16_T_MAX_VALUE 65535
#define UINT16_T_MIN_VALUE 0

typedef struct utils_size
{
    uint16_t w, h;
}
utils_size_t;

void utils_size_init(utils_size_t *ptr, unsigned int w, unsigned int h);
void utils_size_copy(utils_size_t *dst, utils_size_t *src);
unsigned int utils_size_get_w(utils_size_t *ptr);
unsigned int utils_size_get_h(utils_size_t *ptr);
void utils_size_set_w(utils_size_t *ptr, unsigned int w);
void utils_size_set_h(utils_size_t *ptr, unsigned int h);
bool utils_size_equal(utils_size_t *ptr0, utils_size_t *ptr1);
void utils_size_check(utils_size_t *ptr);

#endif /* __UTILS_SIZE_H__ */
