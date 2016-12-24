/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include <string.h>
#include "xalloc.h"
#include "utils_linkedlist.h"

void utils_linkedlist_alloc(utils_linkedlist_t *ptr, size_t data_len)
{
    ptr->head_ptr = NULL;
    ptr->tail_ptr = NULL;
    ptr->size = 0;
    ptr->data_len = data_len;
}

void utils_linkedlist_free(utils_linkedlist_t *ptr)
{
    for (utils_linkedlist_lnk_t *i = ptr->head_ptr; i; ) {
        utils_linkedlist_lnk_t *j = i->next_ptr;
        xfree(i);
        i = j;
    }
}

void utils_linkedlist_clear(utils_linkedlist_t *ptr)
{
    utils_linkedlist_free(ptr);

    ptr->head_ptr = NULL;
    ptr->tail_ptr = NULL;
    ptr->size = 0;
}

size_t utils_linkedlist_size(utils_linkedlist_t *ptr)
{
    return ptr->size;
}

void utils_linkedlist_push_front(utils_linkedlist_t *ptr, void *data)
{
    utils_linkedlist_lnk_t *tmp = (utils_linkedlist_lnk_t *) xalloc(sizeof(utils_linkedlist_lnk_t) + ptr->data_len);
    memcpy(tmp->data, data, ptr->data_len);

    if (ptr->size++) {
        tmp->next_ptr = ptr->head_ptr;
        tmp->prev_ptr = NULL;
        ptr->head_ptr->prev_ptr = tmp;
        ptr->head_ptr = tmp;
    } else {
        tmp->next_ptr = NULL;
        tmp->prev_ptr = NULL;
        ptr->head_ptr = tmp;
        ptr->tail_ptr = tmp;
    }
}

void utils_linkedlist_push_back(utils_linkedlist_t *ptr, void *data)
{
    utils_linkedlist_lnk_t *tmp = (utils_linkedlist_lnk_t *) xalloc(sizeof(utils_linkedlist_lnk_t) + ptr->data_len);
    memcpy(tmp->data, data, ptr->data_len);

    if (ptr->size++) {
        tmp->next_ptr = NULL;
        tmp->prev_ptr = ptr->tail_ptr;
        ptr->tail_ptr->next_ptr = tmp;
        ptr->tail_ptr = tmp;
    } else {
        tmp->next_ptr = NULL;
        tmp->prev_ptr = NULL;
        ptr->head_ptr = tmp;
        ptr->tail_ptr = tmp;
    }
}

void utils_linkedlist_pop_front(utils_linkedlist_t *ptr, void *data)
{
    utils_linkedlist_lnk_t *tmp = ptr->head_ptr;

    if (data) {
        memcpy(data, tmp->data, ptr->data_len);
    }

    tmp->next_ptr->prev_ptr = NULL;
    ptr->head_ptr = tmp->next_ptr;
    ptr->size -= 1;
    xfree(tmp);
}

void utils_linkedlist_pop_back(utils_linkedlist_t *ptr, void *data)
{
    utils_linkedlist_lnk_t *tmp = ptr->tail_ptr;

    if (data) {
        memcpy(data, tmp->data, ptr->data_len);
    }

    tmp->prev_ptr->next_ptr = NULL;
    ptr->tail_ptr = tmp->prev_ptr;
    ptr->size -= 1;
    xfree(tmp);
}

void utils_linkedlist_get_front(utils_linkedlist_t *ptr, void *data)
{
    memcpy(data, ptr->head_ptr->data, ptr->data_len);
}

void utils_linkedlist_get_back(utils_linkedlist_t *ptr, void *data)
{
    memcpy(data, ptr->tail_ptr->data, ptr->data_len);
}

void utils_linkedlist_set_front(utils_linkedlist_t *ptr, void *data)
{
    memcpy(ptr->head_ptr->data, data, ptr->data_len);
}

void utils_linkedlist_set_back(utils_linkedlist_t *ptr, void *data)
{
    memcpy(ptr->tail_ptr->data, data, ptr->data_len);
}

void utils_linkedlist_insert(utils_linkedlist_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        utils_linkedlist_push_front(ptr, data);
    } else if (index >= ptr->size) {
        utils_linkedlist_push_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        utils_linkedlist_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        utils_linkedlist_lnk_t *tmp = (utils_linkedlist_lnk_t *) xalloc(sizeof(utils_linkedlist_lnk_t) + ptr->data_len);
        memcpy(tmp->data, data, ptr->data_len);

        tmp->next_ptr = i;
        tmp->prev_ptr = i->prev_ptr;
        i->prev_ptr->next_ptr = tmp;
        i->prev_ptr = tmp;
        ptr->size += 1;

    } else {

        utils_linkedlist_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        utils_linkedlist_lnk_t *tmp = (utils_linkedlist_lnk_t *) xalloc(sizeof(utils_linkedlist_lnk_t) + ptr->data_len);
        memcpy(tmp->data, data, ptr->data_len);

        tmp->next_ptr = i;
        tmp->prev_ptr = i->prev_ptr;
        i->prev_ptr->next_ptr = tmp;
        i->prev_ptr = tmp;
        ptr->size += 1;
    }
}

void utils_linkedlist_remove(utils_linkedlist_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        utils_linkedlist_pop_front(ptr, data);
    } else if (index >= (ptr->size - 1)) {
        utils_linkedlist_pop_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        utils_linkedlist_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        if (data) {
            memcpy(data, i->data, ptr->data_len);
        }

        i->prev_ptr->next_ptr = i->next_ptr;
        i->next_ptr->prev_ptr = i->prev_ptr;
        ptr->size -= 1;
        xfree(i);

    } else {

        utils_linkedlist_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        if (data) {
            memcpy(data, i->data, ptr->data_len);
        }

        i->prev_ptr->next_ptr = i->next_ptr;
        i->next_ptr->prev_ptr = i->prev_ptr;
        ptr->size -= 1;
        xfree(i);
    }
}

void utils_linkedlist_get(utils_linkedlist_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        utils_linkedlist_get_front(ptr, data);
    } else if (index >= (ptr->size - 1)) {
        utils_linkedlist_get_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        utils_linkedlist_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        memcpy(data, i->data, ptr->data_len);

    } else {

        utils_linkedlist_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        memcpy(data, i->data, ptr->data_len);
    }
}

void utils_linkedlist_set(utils_linkedlist_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        utils_linkedlist_set_front(ptr, data);
    } else if (index >= (ptr->size - 1)) {
        utils_linkedlist_set_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        utils_linkedlist_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        memcpy(i->data, data, ptr->data_len);

    } else {

        utils_linkedlist_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        memcpy(i->data, data, ptr->data_len);
    }
}

utils_linkedlist_lnk_t *utils_linkedlist_start_from_head(utils_linkedlist_t *ptr)
{
    return ptr->head_ptr;
}

utils_linkedlist_lnk_t *utils_linkedlist_start_from_tail(utils_linkedlist_t *ptr)
{
    return ptr->tail_ptr;
}

utils_linkedlist_lnk_t *utils_linkedlist_lnk_next(utils_linkedlist_lnk_t *lnk)
{
    return lnk->next_ptr;
}

utils_linkedlist_lnk_t *utils_linkedlist_lnk_prev(utils_linkedlist_lnk_t *lnk)
{
    return lnk->prev_ptr;
}

void utils_linkedlist_lnk_get(utils_linkedlist_t *ptr, utils_linkedlist_lnk_t *lnk, void *data)
{
    memcpy(data, lnk->data, ptr->data_len);
}

void utils_linkedlist_lnk_set(utils_linkedlist_t *ptr, utils_linkedlist_lnk_t *lnk, void *data)
{
    memcpy(lnk->data, data, ptr->data_len);
}
