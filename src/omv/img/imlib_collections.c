/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "imlib.h"
#define IM_CHAR_BITS (sizeof(char) * 8)
#define IM_CHAR_MASK (IM_CHAR_BITS - 1)
#define IM_CHAR_SHIFT IM_LOG2(IM_CHAR_MASK)

////////////
// im_bitmap
////////////

void im_bitmap_alloc(im_bitmap_t *ptr, size_t size)
{
    ptr->size = size;
    ptr->data = (char *) fb_alloc0(((size + IM_CHAR_MASK) >> IM_CHAR_SHIFT) * sizeof(char));
}

void im_bitmap_free(im_bitmap_t *ptr)
{
    if (ptr->data) {
        fb_free();
    }
}

void im_bitmap_clear(im_bitmap_t *ptr)
{
    memset(ptr->data, 0, ((ptr->size + IM_CHAR_MASK) >> IM_CHAR_SHIFT) * sizeof(char));
}

void im_bitmap_bit_set(im_bitmap_t *ptr, size_t index)
{
    ptr->data[index >> IM_CHAR_SHIFT] |= 1 << (index & IM_CHAR_MASK);
}

bool im_bitmap_bit_get(im_bitmap_t *ptr, size_t index)
{
    return (ptr->data[index >> IM_CHAR_SHIFT] >> (index & IM_CHAR_MASK)) & 1;
}

//////////
// im_lifo
//////////

void im_lifo_alloc(im_lifo_t *ptr, size_t size, size_t data_len)
{
    ptr->len = 0;
    ptr->size = size;
    ptr->data_len = data_len;
    ptr->data = (char *) fb_alloc(size * data_len);
}

void im_lifo_free(im_lifo_t *ptr)
{
    if (ptr->data) {
        fb_free();
    }
}

void im_lifo_clear(im_lifo_t *ptr)
{
    ptr->len = 0;
}

size_t im_lifo_size(im_lifo_t *ptr)
{
    return ptr->len;
}

bool im_lifo_is_not_empty(im_lifo_t *ptr)
{
    return ptr->len;
}

bool im_lifo_is_not_full(im_lifo_t *ptr)
{
    return ptr->len != ptr->size;
}

void im_lifo_enqueue(im_lifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);

    ptr->len += 1;
}

void im_lifo_dequeue(im_lifo_t *ptr, void *data)
{
    if (data) {
        memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
    }

    ptr->len -= 1;
}

void im_lifo_poke(im_lifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);
}

void im_lifo_peek(im_lifo_t *ptr, void *data)
{
    memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
}

//////////
// im_fifo
//////////

void im_fifo_alloc(im_fifo_t *ptr, size_t size, size_t data_len)
{
    ptr->head_ptr = 0;
    ptr->tail_ptr = 0;
    ptr->len = 0;
    ptr->size = size;
    ptr->data_len = data_len;
    ptr->data = (char *) fb_alloc(size * data_len);
}

void im_fifo_free(im_fifo_t *ptr)
{
    if (ptr->data) {
        fb_free();
    }
}

void im_fifo_clear(im_fifo_t *ptr)
{
    ptr->head_ptr = 0;
    ptr->tail_ptr = 0;
    ptr->len = 0;
}

size_t im_fifo_size(im_fifo_t *ptr)
{
    return ptr->len;
}

bool im_fifo_is_not_empty(im_fifo_t *ptr)
{
    return ptr->len;
}

bool im_fifo_is_not_full(im_fifo_t *ptr)
{
    return ptr->len != ptr->size;
}

void im_fifo_enqueue(im_fifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->head_ptr * ptr->data_len), data, ptr->data_len);

    size_t temp = ptr->head_ptr + 1;

    if (temp == ptr->size) {
        temp = 0;
    }

    ptr->head_ptr = temp;
    ptr->len += 1;
}

void im_fifo_dequeue(im_fifo_t *ptr, void *data)
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

void im_fifo_poke(im_fifo_t *ptr, void *data)
{
    memcpy(ptr->data + (ptr->head_ptr * ptr->data_len), data, ptr->data_len);
}

void im_fifo_peek(im_fifo_t *ptr, void *data)
{
    memcpy(data, ptr->data + (ptr->tail_ptr * ptr->data_len), ptr->data_len);
}

//////////
// im_list
//////////

void im_list_init(im_list_t *ptr, size_t data_len)
{
    ptr->head_ptr = NULL;
    ptr->tail_ptr = NULL;
    ptr->size = 0;
    ptr->data_len = data_len;
}

void im_list_copy(im_list_t *dst, im_list_t *src)
{
    memcpy(dst, src, sizeof(im_list_t));
}

void im_list_free(im_list_t *ptr)
{
    for (im_list_lnk_t *i = ptr->head_ptr; i; ) {
        im_list_lnk_t *j = i->next_ptr;
        xfree(i);
        i = j;
    }
}

void im_list_clear(im_list_t *ptr)
{
    im_list_free(ptr);

    ptr->head_ptr = NULL;
    ptr->tail_ptr = NULL;
    ptr->size = 0;
}

size_t im_list_size(im_list_t *ptr)
{
    return ptr->size;
}

void im_list_push_front(im_list_t *ptr, void *data)
{
    im_list_lnk_t *tmp = (im_list_lnk_t *) xalloc(sizeof(im_list_lnk_t) + ptr->data_len);
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

void im_list_push_back(im_list_t *ptr, void *data)
{
    im_list_lnk_t *tmp = (im_list_lnk_t *) xalloc(sizeof(im_list_lnk_t) + ptr->data_len);
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

void im_list_pop_front(im_list_t *ptr, void *data)
{
    im_list_lnk_t *tmp = ptr->head_ptr;

    if (data) {
        memcpy(data, tmp->data, ptr->data_len);
    }

    tmp->next_ptr->prev_ptr = NULL;
    ptr->head_ptr = tmp->next_ptr;
    ptr->size -= 1;
    xfree(tmp);
}

void im_list_pop_back(im_list_t *ptr, void *data)
{
    im_list_lnk_t *tmp = ptr->tail_ptr;

    if (data) {
        memcpy(data, tmp->data, ptr->data_len);
    }

    tmp->prev_ptr->next_ptr = NULL;
    ptr->tail_ptr = tmp->prev_ptr;
    ptr->size -= 1;
    xfree(tmp);
}

void im_list_get_front(im_list_t *ptr, void *data)
{
    memcpy(data, ptr->head_ptr->data, ptr->data_len);
}

void im_list_get_back(im_list_t *ptr, void *data)
{
    memcpy(data, ptr->tail_ptr->data, ptr->data_len);
}

void im_list_set_front(im_list_t *ptr, void *data)
{
    memcpy(ptr->head_ptr->data, data, ptr->data_len);
}

void im_list_set_back(im_list_t *ptr, void *data)
{
    memcpy(ptr->tail_ptr->data, data, ptr->data_len);
}

void im_list_insert(im_list_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        im_list_push_front(ptr, data);
    } else if (index >= ptr->size) {
        im_list_push_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        im_list_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        im_list_lnk_t *tmp = (im_list_lnk_t *) xalloc(sizeof(im_list_lnk_t) + ptr->data_len);
        memcpy(tmp->data, data, ptr->data_len);

        tmp->next_ptr = i;
        tmp->prev_ptr = i->prev_ptr;
        i->prev_ptr->next_ptr = tmp;
        i->prev_ptr = tmp;
        ptr->size += 1;

    } else {

        im_list_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        im_list_lnk_t *tmp = (im_list_lnk_t *) xalloc(sizeof(im_list_lnk_t) + ptr->data_len);
        memcpy(tmp->data, data, ptr->data_len);

        tmp->next_ptr = i;
        tmp->prev_ptr = i->prev_ptr;
        i->prev_ptr->next_ptr = tmp;
        i->prev_ptr = tmp;
        ptr->size += 1;
    }
}

void im_list_remove(im_list_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        im_list_pop_front(ptr, data);
    } else if (index >= (ptr->size - 1)) {
        im_list_pop_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        im_list_lnk_t *i = ptr->head_ptr;

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

        im_list_lnk_t *i = ptr->tail_ptr;
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

void im_list_get(im_list_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        im_list_get_front(ptr, data);
    } else if (index >= (ptr->size - 1)) {
        im_list_get_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        im_list_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        memcpy(data, i->data, ptr->data_len);

    } else {

        im_list_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        memcpy(data, i->data, ptr->data_len);
    }
}

void im_list_set(im_list_t *ptr, void *data, size_t index)
{
    if (index == 0) {
        im_list_set_front(ptr, data);
    } else if (index >= (ptr->size - 1)) {
        im_list_set_back(ptr, data);
    } else if (index < (ptr->size >> 1)) {

        im_list_lnk_t *i = ptr->head_ptr;

        while (index) {
            i = i->next_ptr;
            index -= 1;
        }

        memcpy(i->data, data, ptr->data_len);

    } else {

        im_list_lnk_t *i = ptr->tail_ptr;
        index = ptr->size - index - 1;

        while (index) {
            i = i->prev_ptr;
            index -= 1;
        }

        memcpy(i->data, data, ptr->data_len);
    }
}

///////////////////
// im_list iterator
///////////////////

im_list_lnk_t *im_iterator_start_from_head(im_list_t *ptr)
{
    return ptr->head_ptr;
}

im_list_lnk_t *im_iterator_start_from_tail(im_list_t *ptr)
{
    return ptr->tail_ptr;
}

im_list_lnk_t *im_iterator_next(im_list_lnk_t *lnk)
{
    return lnk->next_ptr;
}

im_list_lnk_t *im_iterator_prev(im_list_lnk_t *lnk)
{
    return lnk->prev_ptr;
}

void im_iterator_get(im_list_t *ptr, im_list_lnk_t *lnk, void *data)
{
    memcpy(data, lnk->data, ptr->data_len);
}

void im_iterator_set(im_list_t *ptr, im_list_lnk_t *lnk, void *data)
{
    memcpy(lnk->data, data, ptr->data_len);
}
