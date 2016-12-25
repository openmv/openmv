/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __UTILS_LINKEDLIST_H__
#define __UTILS_LINKEDLIST_H__
#include <stdbool.h>
#include <stddef.h>

typedef struct utils_linkedlist_lnk
{
    struct utils_linkedlist_lnk *next_ptr, *prev_ptr;
    char data[];
}
utils_linkedlist_lnk_t;

typedef struct utils_linkedlist
{
    utils_linkedlist_lnk_t *head_ptr, *tail_ptr;
    size_t size, data_len;
}
utils_linkedlist_t;

void utils_linkedlist_alloc(utils_linkedlist_t *ptr, size_t data_len);
void utils_linkedlist_free(utils_linkedlist_t *ptr);
void utils_linkedlist_clear(utils_linkedlist_t *ptr);
size_t utils_linkedlist_size(utils_linkedlist_t *ptr);
void utils_linkedlist_push_front(utils_linkedlist_t *ptr, void *data); // No overflow protection.
void utils_linkedlist_push_back(utils_linkedlist_t *ptr, void *data); // No overflow protection.
void utils_linkedlist_pop_front(utils_linkedlist_t *ptr, void *data); // No underflow protection.
void utils_linkedlist_pop_back(utils_linkedlist_t *ptr, void *data); // No underflow protection.
void utils_linkedlist_get_front(utils_linkedlist_t *ptr, void *data); // No empty protection.
void utils_linkedlist_get_back(utils_linkedlist_t *ptr, void *data); // No empty protection.
void utils_linkedlist_set_front(utils_linkedlist_t *ptr, void *data); // No empty protection.
void utils_linkedlist_set_back(utils_linkedlist_t *ptr, void *data); // No empty protection.
void utils_linkedlist_insert(utils_linkedlist_t *ptr, void *data, size_t index); // No overflow protection.
void utils_linkedlist_remove(utils_linkedlist_t *ptr, void *data, size_t index); // No underflow protection.
void utils_linkedlist_get(utils_linkedlist_t *ptr, void *data, size_t index);
void utils_linkedlist_set(utils_linkedlist_t *ptr, void *data, size_t index);

utils_linkedlist_lnk_t *utils_linkedlist_start_from_head(utils_linkedlist_t *ptr);
utils_linkedlist_lnk_t *utils_linkedlist_start_from_tail(utils_linkedlist_t *ptr);
utils_linkedlist_lnk_t *utils_linkedlist_lnk_next(utils_linkedlist_lnk_t *lnk);
utils_linkedlist_lnk_t *utils_linkedlist_lnk_prev(utils_linkedlist_lnk_t *lnk);
void utils_linkedlist_lnk_get(utils_linkedlist_t *ptr, utils_linkedlist_lnk_t *lnk, void *data);
void utils_linkedlist_lnk_set(utils_linkedlist_t *ptr, utils_linkedlist_lnk_t *lnk, void *data);

#endif /* __UTILS_LINKEDLIST_H__ */
