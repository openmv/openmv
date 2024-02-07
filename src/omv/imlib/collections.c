/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Common data structures.
 */
#include "imlib.h"
#define CHAR_BITS     (sizeof(char) * 8)
#define CHAR_MASK     (CHAR_BITS - 1)
#define CHAR_SHIFT    IM_LOG2(CHAR_MASK)

// Bitmap
void bitmap_alloc(bitmap_t *ptr, size_t size) {
    ptr->size = size;
    ptr->data = (char *) fb_alloc0(((size + CHAR_MASK) >> CHAR_SHIFT) * sizeof(char), FB_ALLOC_NO_HINT);
}

void bitmap_free(bitmap_t *ptr) {
    if (ptr->data) {
        fb_free();
    }
}

void bitmap_clear(bitmap_t *ptr) {
    memset(ptr->data, 0, ((ptr->size + CHAR_MASK) >> CHAR_SHIFT) * sizeof(char));
}

void bitmap_bit_set(bitmap_t *ptr, size_t index) {
    ptr->data[index >> CHAR_SHIFT] |= 1 << (index & CHAR_MASK);
}

bool bitmap_bit_get(bitmap_t *ptr, size_t index) {
    return (ptr->data[index >> CHAR_SHIFT] >> (index & CHAR_MASK)) & 1;
}

// LIFO
void lifo_alloc(lifo_t *ptr, size_t size, size_t data_len) {
    ptr->len = 0;
    ptr->size = size;
    ptr->data_len = data_len;
    ptr->data = (char *) fb_alloc(size * data_len, FB_ALLOC_NO_HINT);
}

void lifo_alloc_all(lifo_t *ptr, size_t *size, size_t data_len) {
    uint32_t tmp_size;
    ptr->data = (char *) fb_alloc_all(&tmp_size, FB_ALLOC_NO_HINT);
    ptr->data_len = data_len;
    ptr->size = tmp_size / data_len;
    ptr->len = 0;
    *size = ptr->size;
}

void lifo_free(lifo_t *ptr) {
    if (ptr->data) {
        fb_free();
    }
}

void lifo_clear(lifo_t *ptr) {
    ptr->len = 0;
}

size_t lifo_size(lifo_t *ptr) {
    return ptr->len;
}

bool lifo_is_not_empty(lifo_t *ptr) {
    return ptr->len;
}

bool lifo_is_not_full(lifo_t *ptr) {
    return ptr->len != ptr->size;
}

void lifo_enqueue(lifo_t *ptr, void *data) {
    memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);

    ptr->len += 1;
}

void lifo_dequeue(lifo_t *ptr, void *data) {
    if (data) {
        memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
    }

    ptr->len -= 1;
}

void lifo_poke(lifo_t *ptr, void *data) {
    memcpy(ptr->data + (ptr->len * ptr->data_len), data, ptr->data_len);
}

void lifo_peek(lifo_t *ptr, void *data) {
    memcpy(data, ptr->data + ((ptr->len - 1) * ptr->data_len), ptr->data_len);
}

// FIFO
void fifo_alloc(fifo_t *ptr, size_t size, size_t data_len) {
    ptr->head = 0;
    ptr->tail = 0;
    ptr->len = 0;
    ptr->size = size;
    ptr->data_len = data_len;
    ptr->data = (char *) fb_alloc(size * data_len, FB_ALLOC_NO_HINT);
}

void fifo_alloc_all(fifo_t *ptr, size_t *size, size_t data_len) {
    uint32_t tmp_size;
    ptr->data = (char *) fb_alloc_all(&tmp_size, FB_ALLOC_NO_HINT);
    ptr->data_len = data_len;
    ptr->size = tmp_size / data_len;
    ptr->len = 0;
    ptr->tail = 0;
    ptr->head = 0;
    *size = ptr->size;
}

void fifo_free(fifo_t *ptr) {
    if (ptr->data) {
        fb_free();
    }
}

void fifo_clear(fifo_t *ptr) {
    ptr->head = 0;
    ptr->tail = 0;
    ptr->len = 0;
}

size_t fifo_size(fifo_t *ptr) {
    return ptr->len;
}

bool fifo_is_not_empty(fifo_t *ptr) {
    return ptr->len;
}

bool fifo_is_not_full(fifo_t *ptr) {
    return ptr->len != ptr->size;
}

void fifo_enqueue(fifo_t *ptr, void *data) {
    memcpy(ptr->data + (ptr->head * ptr->data_len), data, ptr->data_len);

    size_t temp = ptr->head + 1;

    if (temp == ptr->size) {
        temp = 0;
    }

    ptr->head = temp;
    ptr->len += 1;
}

void fifo_dequeue(fifo_t *ptr, void *data) {
    if (data) {
        memcpy(data, ptr->data + (ptr->tail * ptr->data_len), ptr->data_len);
    }

    size_t temp = ptr->tail + 1;

    if (temp == ptr->size) {
        temp = 0;
    }

    ptr->tail = temp;
    ptr->len -= 1;
}

void fifo_poke(fifo_t *ptr, void *data) {
    memcpy(ptr->data + (ptr->head * ptr->data_len), data, ptr->data_len);
}

void fifo_peek(fifo_t *ptr, void *data) {
    memcpy(data, ptr->data + (ptr->tail * ptr->data_len), ptr->data_len);
}

// Linked List
void list_init(list_t *ptr, size_t data_len) {
    ptr->head = NULL;
    ptr->tail = NULL;
    ptr->size = 0;
    ptr->data_len = data_len;
}

void list_copy(list_t *dst, list_t *src) {
    memcpy(dst, src, sizeof(list_t));
}

void list_free(list_t *ptr) {
    for (list_lnk_t *i = ptr->head; i; ) {
        list_lnk_t *j = i->next;
        xfree(i);
        i = j;
    }
}

void list_clear(list_t *ptr) {
    list_free(ptr);

    ptr->head = NULL;
    ptr->tail = NULL;
    ptr->size = 0;
}

size_t list_size(list_t *ptr) {
    return ptr->size;
}

void list_push_front(list_t *ptr, void *data) {
    list_lnk_t *tmp = (list_lnk_t *) xalloc(sizeof(list_lnk_t) + ptr->data_len);
    memcpy(tmp->data, data, ptr->data_len);

    if (ptr->size++) {
        tmp->next = ptr->head;
        tmp->prev = NULL;
        ptr->head->prev = tmp;
        ptr->head = tmp;
    } else {
        tmp->next = NULL;
        tmp->prev = NULL;
        ptr->head = tmp;
        ptr->tail = tmp;
    }
}

void list_push_back(list_t *ptr, void *data) {
    list_lnk_t *tmp = (list_lnk_t *) xalloc(sizeof(list_lnk_t) + ptr->data_len);
    memcpy(tmp->data, data, ptr->data_len);

    if (ptr->size++) {
        tmp->next = NULL;
        tmp->prev = ptr->tail;
        ptr->tail->next = tmp;
        ptr->tail = tmp;
    } else {
        tmp->next = NULL;
        tmp->prev = NULL;
        ptr->head = tmp;
        ptr->tail = tmp;
    }
}

void list_pop_front(list_t *ptr, void *data) {
    list_lnk_t *tmp = ptr->head;

    if (data) {
        memcpy(data, tmp->data, ptr->data_len);
    }

    if (tmp->next) {
        tmp->next->prev = NULL;
    }
    ptr->head = tmp->next;
    ptr->size -= 1;
    xfree(tmp);
}

void list_pop_back(list_t *ptr, void *data) {
    list_lnk_t *tmp = ptr->tail;

    if (data) {
        memcpy(data, tmp->data, ptr->data_len);
    }

    tmp->prev->next = NULL;
    ptr->tail = tmp->prev;
    ptr->size -= 1;
    xfree(tmp);
}

void list_get(list_t *ptr, list_lnk_t *lnk, void *data) {
    memcpy(data, lnk->data, ptr->data_len);
}

void list_set(list_t *ptr, list_lnk_t *lnk, void *data) {
    memcpy(lnk->data, data, ptr->data_len);
}

void list_insert(list_t *ptr, list_lnk_t *lnk, void *data) {
    if (ptr->head == lnk) {
        list_push_front(ptr, data);
    } else if (!lnk) {
        list_push_back(ptr, data);
    } else {
        list_lnk_t *tmp = (list_lnk_t *) xalloc(sizeof(list_lnk_t) + ptr->data_len);
        memcpy(tmp->data, data, ptr->data_len);

        tmp->next = lnk;
        tmp->prev = lnk->prev;
        lnk->prev->next = tmp;
        lnk->prev = tmp;
        ptr->size += 1;
    }
}

void list_remove(list_t *ptr, list_lnk_t *lnk, void *data) {
    if (ptr->head == lnk) {
        list_pop_front(ptr, data);
    } else if (ptr->tail == lnk) {
        list_pop_back(ptr, data);
    } else {
        if (data) {
            memcpy(data, lnk->data, ptr->data_len);
        }

        lnk->prev->next = lnk->next;
        lnk->next->prev = lnk->prev;
        ptr->size -= 1;
        xfree(lnk);
    }
}
