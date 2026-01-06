/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025-2026 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Collections unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include "py/runtime.h"
#include "py/obj.h"

#include "collections.h"

// Test bitmap operations
static mp_obj_t test_collections_bitmap(void) {
    bitmap_t bmp;
    bitmap_alloc(&bmp, 1000);

    // Initially all bits should be clear
    for (int i = 0; i < 100; i++) {
        if (bitmap_bit_get(&bmp, i * 10) != false) {
            bitmap_free(&bmp);
            return mp_const_false;
        }
    }

    // Set some bits
    bitmap_bit_set(&bmp, 0);
    bitmap_bit_set(&bmp, 500);
    bitmap_bit_set(&bmp, 999);

    // Check set bits
    if (!bitmap_bit_get(&bmp, 0) ||
        !bitmap_bit_get(&bmp, 500) ||
        !bitmap_bit_get(&bmp, 999)) {
        bitmap_free(&bmp);
        return mp_const_false;
    }

    // Check unset bits
    if (bitmap_bit_get(&bmp, 1) ||
        bitmap_bit_get(&bmp, 501) ||
        bitmap_bit_get(&bmp, 998)) {
        bitmap_free(&bmp);
        return mp_const_false;
    }

    // Test clear
    bitmap_clear(&bmp);
    if (bitmap_bit_get(&bmp, 0) ||
        bitmap_bit_get(&bmp, 500) ||
        bitmap_bit_get(&bmp, 999)) {
        bitmap_free(&bmp);
        return mp_const_false;
    }

    bitmap_free(&bmp);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_collections_bitmap_obj, test_collections_bitmap);

// Test LIFO (stack) operations
static mp_obj_t test_collections_lifo(void) {
    // Use stack-allocated buffer to avoid fb_alloc dependency
    lifo_t stack;
    int buffer[10];
    stack.len = 0;
    stack.size = 10;
    stack.data_len = sizeof(int);
    stack.data = (char *) buffer;

    // Initially empty
    if (lifo_size(&stack) != 0) {
        return mp_const_false;
    }

    // Test is_not_empty on empty stack (should be false)
    if (lifo_is_not_empty(&stack)) {
        return mp_const_false;
    }

    // Test is_not_full on empty stack (should be true)
    if (!lifo_is_not_full(&stack)) {
        return mp_const_false;
    }

    // Push values
    for (int i = 0; i < 5; i++) {
        int val = i * 10;
        lifo_enqueue(&stack, &val);
    }

    if (lifo_size(&stack) != 5) {
        return mp_const_false;
    }

    // Test is_not_empty with items (should be true)
    if (!lifo_is_not_empty(&stack)) {
        return mp_const_false;
    }

    // Pop and verify LIFO order
    int expected[] = {40, 30, 20, 10, 0};
    for (int i = 0; i < 5; i++) {
        int val;
        lifo_dequeue(&stack, &val);
        if (val != expected[i]) {
            return mp_const_false;
        }
    }

    if (lifo_size(&stack) != 0) {
        return mp_const_false;
    }

    // Test peek - push a value and peek at it
    int val = 100;
    lifo_enqueue(&stack, &val);

    int peek_val;
    lifo_peek(&stack, &peek_val);
    if (peek_val != 100 || lifo_size(&stack) != 1) {
        return mp_const_false;
    }

    // Fill stack to capacity and test is_not_full
    for (int i = 1; i < 10; i++) {
        int v = i;
        lifo_enqueue(&stack, &v);
    }

    // Stack is now full (10 items)
    if (lifo_is_not_full(&stack)) {
        return mp_const_false;  // Should be full, so is_not_full should be false
    }

    // No lifo_free needed - using stack-allocated buffer
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_collections_lifo_obj, test_collections_lifo);

// Test FIFO (queue) operations
static mp_obj_t test_collections_fifo(void) {
    fifo_t queue;
    fifo_alloc(&queue, 10, sizeof(int));

    // Initially empty
    if (fifo_size(&queue) != 0) {
        fifo_free(&queue);
        return mp_const_false;
    }

    // Test is_not_empty on empty queue (should be false)
    if (fifo_is_not_empty(&queue)) {
        fifo_free(&queue);
        return mp_const_false;
    }

    // Test is_not_full on empty queue (should be true)
    if (!fifo_is_not_full(&queue)) {
        fifo_free(&queue);
        return mp_const_false;
    }

    // Enqueue values
    for (int i = 0; i < 5; i++) {
        int val = i * 10;
        fifo_enqueue(&queue, &val);
    }

    if (fifo_size(&queue) != 5) {
        fifo_free(&queue);
        return mp_const_false;
    }

    // Test is_not_empty with items (should be true)
    if (!fifo_is_not_empty(&queue)) {
        fifo_free(&queue);
        return mp_const_false;
    }

    // Dequeue and verify FIFO order
    int expected[] = {0, 10, 20, 30, 40};
    for (int i = 0; i < 5; i++) {
        int val;
        fifo_dequeue(&queue, &val);
        if (val != expected[i]) {
            fifo_free(&queue);
            return mp_const_false;
        }
    }

    if (fifo_size(&queue) != 0) {
        fifo_free(&queue);
        return mp_const_false;
    }

    // Fill queue to capacity and test is_not_full
    for (int i = 0; i < 10; i++) {
        int val = i;
        fifo_enqueue(&queue, &val);
    }

    // Queue is now full (10 items)
    if (fifo_is_not_full(&queue)) {
        fifo_free(&queue);
        return mp_const_false;  // Should be full, so is_not_full should be false
    }

    fifo_free(&queue);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_collections_fifo_obj, test_collections_fifo);

// Test linked list operations
static mp_obj_t test_collections_list(void) {
    list_t lst;
    list_init(&lst, sizeof(int));

    // Initially empty
    if (list_size(&lst) != 0) {
        list_free(&lst);
        return mp_const_false;
    }

    // Push front
    for (int i = 0; i < 3; i++) {
        int val = i;
        list_push_front(&lst, &val);
    }

    // Push back
    for (int i = 10; i < 13; i++) {
        int val = i;
        list_push_back(&lst, &val);
    }

    // Should have 6 elements: 2, 1, 0, 10, 11, 12
    if (list_size(&lst) != 6) {
        list_free(&lst);
        return mp_const_false;
    }

    // Pop front and verify
    int val;
    list_pop_front(&lst, &val);
    if (val != 2) {
        list_free(&lst);
        return mp_const_false;
    }

    // Pop back and verify
    list_pop_back(&lst, &val);
    if (val != 12) {
        list_free(&lst);
        return mp_const_false;
    }

    // Clear
    list_clear(&lst);
    if (list_size(&lst) != 0) {
        list_free(&lst);
        return mp_const_false;
    }

    list_free(&lst);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_collections_list_obj, test_collections_list);

// Module definition
static const mp_rom_map_elem_t unittest_collections_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_collections) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_bitmap), MP_ROM_PTR(&test_collections_bitmap_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_lifo), MP_ROM_PTR(&test_collections_lifo_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_fifo), MP_ROM_PTR(&test_collections_fifo_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_list), MP_ROM_PTR(&test_collections_list_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_collections_module_globals, unittest_collections_module_globals_table);

const mp_obj_module_t unittest_collections_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_collections_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_collections, unittest_collections_module);

#endif // MICROPY_PY_UNITTEST
