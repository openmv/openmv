/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __IMLIB_COLLECTIONS_H__
#define __IMLIB_COLLECTIONS_H__
#include <stdbool.h>
#include <stddef.h>

////////////
// im_bitmap
////////////

typedef struct im_bitmap
{
    size_t size;
    char *data;
}
im_bitmap_t;

void im_bitmap_alloc(im_bitmap_t *ptr, size_t size);
void im_bitmap_free(im_bitmap_t *ptr);
void im_bitmap_clear(im_bitmap_t *ptr);
void im_bitmap_bit_set(im_bitmap_t *ptr, size_t index);
bool im_bitmap_bit_get(im_bitmap_t *ptr, size_t index);
#define IM_BITMAP_COMPUTE_ROW_INDEX(image, y) (((image)->w)*(y))
#define IM_BITMAP_COMPUTE_INDEX(row_index, x) ((row_index)+(x))

//////////
// im_lifo
//////////

typedef struct im_lifo
{
    size_t len, size, data_len;
    char *data;
}
im_lifo_t;

void im_lifo_alloc(im_lifo_t *ptr, size_t size, size_t data_len);
void im_lifo_free(im_lifo_t *ptr);
void im_lifo_clear(im_lifo_t *ptr);
size_t im_lifo_size(im_lifo_t *ptr);
bool im_lifo_is_not_empty(im_lifo_t *ptr);
bool im_lifo_is_not_full(im_lifo_t *ptr);
void im_lifo_enqueue(im_lifo_t *ptr, void *data);
void im_lifo_dequeue(im_lifo_t *ptr, void *data);
void im_lifo_poke(im_lifo_t *ptr, void *data);
void im_lifo_peek(im_lifo_t *ptr, void *data);

//////////
// im_fifo
//////////

typedef struct im_fifo
{
    size_t head_ptr, tail_ptr, len, size, data_len;
    char *data;
}
im_fifo_t;

void im_fifo_alloc(im_fifo_t *ptr, size_t size, size_t data_len);
void im_fifo_free(im_fifo_t *ptr);
void im_fifo_clear(im_fifo_t *ptr);
size_t im_fifo_size(im_fifo_t *ptr);
bool im_fifo_is_not_empty(im_fifo_t *ptr);
bool im_fifo_is_not_full(im_fifo_t *ptr);
void im_fifo_enqueue(im_fifo_t *ptr, void *data);
void im_fifo_dequeue(im_fifo_t *ptr, void *data);
void im_fifo_poke(im_fifo_t *ptr, void *data);
void im_fifo_peek(im_fifo_t *ptr, void *data);

//////////
// im_list
//////////

typedef struct im_list_lnk
{
    struct im_list_lnk *next_ptr, *prev_ptr;
    char data[];
}
im_list_lnk_t;

typedef struct im_list
{
    im_list_lnk_t *head_ptr, *tail_ptr;
    size_t size, data_len;
}
im_list_t;

void im_list_init(im_list_t *ptr, size_t data_len);
void im_list_copy(im_list_t *dst, im_list_t *src);
void im_list_free(im_list_t *ptr);
void im_list_clear(im_list_t *ptr);
size_t im_list_size(im_list_t *ptr);
void im_list_push_front(im_list_t *ptr, void *data);
void im_list_push_back(im_list_t *ptr, void *data);
void im_list_pop_front(im_list_t *ptr, void *data);
void im_list_pop_back(im_list_t *ptr, void *data);
void im_list_get_front(im_list_t *ptr, void *data);
void im_list_get_back(im_list_t *ptr, void *data);
void im_list_set_front(im_list_t *ptr, void *data);
void im_list_set_back(im_list_t *ptr, void *data);
void im_list_insert(im_list_t *ptr, void *data, size_t index);
void im_list_remove(im_list_t *ptr, void *data, size_t index);
void im_list_get(im_list_t *ptr, void *data, size_t index);
void im_list_set(im_list_t *ptr, void *data, size_t index);

//////////////
// im_iterator
//////////////

im_list_lnk_t *im_iterator_start_from_head(im_list_t *ptr);
im_list_lnk_t *im_iterator_start_from_tail(im_list_t *ptr);
im_list_lnk_t *im_iterator_next(im_list_lnk_t *lnk);
im_list_lnk_t *im_iterator_prev(im_list_lnk_t *lnk);
void im_iterator_get(im_list_t *ptr, im_list_lnk_t *lnk, void *data);
void im_iterator_set(im_list_t *ptr, im_list_lnk_t *lnk, void *data);

#endif /* __IMLIB_COLLECTIONS_H__ */
