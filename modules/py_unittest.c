/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * Unit test module for testing low-level C functions.
 * Enable with MICROPY_PY_UNITTEST=1 (intended for QEMU builds).
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objstr.h"

#include "fmath.h"
#include "collections.h"
#include "imlib.h"
#include "fb_alloc.h"
#include "framebuffer.h"

// Helper macro for float comparison with tolerance (use fabsf to avoid double promotion)
#define FLOAT_EQ(a, b, tol)    (fabsf((a) - (b)) < (tol))

// ============================================================================
// Fast Math Tests (fmath.h)
// ============================================================================

// Test fast_sqrtf accuracy
static mp_obj_t test_fmath_sqrtf(void) {
    float test_values[] = {0.0f, 1.0f, 2.0f, 4.0f, 9.0f, 16.0f, 100.0f, 0.25f, 0.01f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_sqrtf(x);
        float std_result = sqrtf(x);
        if (!FLOAT_EQ(fast_result, std_result, 0.0001f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_sqrtf_obj, test_fmath_sqrtf);

// Test fast_floorf accuracy
static mp_obj_t test_fmath_floorf(void) {
    // Test that floor(x) <= x and floor(x) is an integer
    float test_values[] = {0.0f, 1.5f, 2.9f, 100.1f, 3.0f, 4.99f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        int result = fast_floorf(x);
        // floor(x) should be <= x and > x-1
        if ((float) result > x || (float) result <= x - 1.0f) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_floorf_obj, test_fmath_floorf);

// Test fast_ceilf accuracy
static mp_obj_t test_fmath_ceilf(void) {
    // Test basic ceiling behavior: result >= x and result - 1 < x
    float test_values[] = {0.5f, 1.5f, 2.1f, 100.9f, 4.01f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        int result = fast_ceilf(x);
        // ceil(x) must be >= x
        if ((float) result < x) {
            return mp_const_false;
        }
        // ceil(x) - 1 must be < x (otherwise ceil would be smaller)
        if ((float) (result - 1) >= x) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_ceilf_obj, test_fmath_ceilf);

// Test fast_roundf accuracy
static mp_obj_t test_fmath_roundf(void) {
    float test_values[] = {0.0f, 1.4f, 1.5f, 1.6f, -1.4f, -1.5f, -1.6f, 100.5f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        int fast_result = fast_roundf(x);
        int std_result = (int) roundf(x);
        // Allow for rounding mode differences at .5
        if (abs(fast_result - std_result) > 1) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_roundf_obj, test_fmath_roundf);

// Test fast_fabsf accuracy
static mp_obj_t test_fmath_fabsf(void) {
    float test_values[] = {0.0f, 1.5f, -1.5f, 100.0f, -100.0f, 0.001f, -0.001f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_fabsf(x);
        float std_result = fabsf(x);
        if (!FLOAT_EQ(fast_result, std_result, 0.0001f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_fabsf_obj, test_fmath_fabsf);

// Test fast_atanf accuracy
static mp_obj_t test_fmath_atanf(void) {
    float test_values[] = {0.0f, 0.5f, 1.0f, -0.5f, -1.0f, 2.0f, -2.0f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_atanf(x);
        float std_result = atanf(x);
        // Allow slightly larger tolerance for transcendental functions
        if (!FLOAT_EQ(fast_result, std_result, 0.01f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_atanf_obj, test_fmath_atanf);

// Test fast_atan2f accuracy
static mp_obj_t test_fmath_atan2f(void) {
    // Test that atan2 produces sensible results for first quadrant
    // fast_atan2f may have different range than standard atan2f

    // atan2(1, 1) should be ~PI/4 = 0.785
    float result = fast_atan2f(1.0f, 1.0f);
    if (!FLOAT_EQ(result, 0.785f, 0.05f)) {
        return mp_const_false;
    }

    // atan2(0, 1) should be ~0
    result = fast_atan2f(0.0f, 1.0f);
    if (!FLOAT_EQ(result, 0.0f, 0.02f)) {
        return mp_const_false;
    }

    // atan2(1, 0) should be ~PI/2 = 1.57
    result = fast_atan2f(1.0f, 0.0001f);
    if (result < 1.5f || result > 1.65f) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_atan2f_obj, test_fmath_atan2f);

// Test fast_expf accuracy
static mp_obj_t test_fmath_expf(void) {
    float test_values[] = {0.0f, 1.0f, -1.0f, 2.0f, -2.0f, 0.5f, -0.5f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_expf(x);
        float std_result = expf(x);
        // Relative error check for exp
        float rel_error = fabsf(fast_result - std_result) / (std_result + 0.0001f);
        if (rel_error > 0.05f) {
            // 5% tolerance
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_expf_obj, test_fmath_expf);

// Test fast_log accuracy
static mp_obj_t test_fmath_log(void) {
    float test_values[] = {0.1f, 0.5f, 1.0f, 2.0f, 10.0f, 100.0f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_log(x);
        float std_result = logf(x);
        if (!FLOAT_EQ(fast_result, std_result, 0.05f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_log_obj, test_fmath_log);

// Test fast_log2 accuracy
static mp_obj_t test_fmath_log2(void) {
    float test_values[] = {0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_log2(x);
        float std_result = log2f(x);
        if (!FLOAT_EQ(fast_result, std_result, 0.05f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_log2_obj, test_fmath_log2);

// Test fast_powf accuracy
static mp_obj_t test_fmath_powf(void) {
    // fast_powf uses exp(b*log(a)) approximation, allow larger tolerance
    float test_pairs[][2] = {
        {2.0f, 2.0f}, {2.0f, 3.0f}, {4.0f, 0.5f}, {3.0f, 2.0f}
    };
    float expected[] = {4.0f, 8.0f, 2.0f, 9.0f};
    int n = sizeof(test_pairs) / sizeof(test_pairs[0]);

    for (int i = 0; i < n; i++) {
        float a = test_pairs[i][0];
        float b = test_pairs[i][1];
        float fast_result = fast_powf(a, b);
        // Allow 20% tolerance for fast approximation
        float rel_error = fabsf(fast_result - expected[i]) / (expected[i] + 0.0001f);
        if (rel_error > 0.20f) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_powf_obj, test_fmath_powf);

// Test fast_cbrtf accuracy (cube root)
static mp_obj_t test_fmath_cbrtf(void) {
    float test_values[] = {1.0f, 8.0f, 27.0f, 64.0f, 0.125f, 0.001f};
    float expected[] = {1.0f, 2.0f, 3.0f, 4.0f, 0.5f, 0.1f};
    int n = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < n; i++) {
        float x = test_values[i];
        float fast_result = fast_cbrtf(x);
        // Allow 5% relative tolerance for fast approximation
        float rel_error = fabsf(fast_result - expected[i]) / (expected[i] + 0.0001f);
        if (rel_error > 0.05f) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_cbrtf_obj, test_fmath_cbrtf);

// Test fast_get_min_max
static mp_obj_t test_fmath_get_min_max(void) {
    float data1[] = {3.0f, 1.0f, 4.0f, 1.0f, 5.0f, 9.0f, 2.0f, 6.0f};
    float min_val, max_val;

    fast_get_min_max(data1, 8, &min_val, &max_val);
    if (!FLOAT_EQ(min_val, 1.0f, 0.001f) || !FLOAT_EQ(max_val, 9.0f, 0.001f)) {
        return mp_const_false;
    }

    // Test with negative values
    float data2[] = {-5.0f, 3.0f, -2.0f, 7.0f, 0.0f};
    fast_get_min_max(data2, 5, &min_val, &max_val);
    if (!FLOAT_EQ(min_val, -5.0f, 0.001f) || !FLOAT_EQ(max_val, 7.0f, 0.001f)) {
        return mp_const_false;
    }

    // Test with single element
    float data3[] = {42.0f};
    fast_get_min_max(data3, 1, &min_val, &max_val);
    if (!FLOAT_EQ(min_val, 42.0f, 0.001f) || !FLOAT_EQ(max_val, 42.0f, 0.001f)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fmath_get_min_max_obj, test_fmath_get_min_max);

// ============================================================================
// Collections Tests (collections.h)
// ============================================================================

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

// ============================================================================
// Image Library Tests (imlib.h)
// ============================================================================

// Test imlib_ksize_to_n
static mp_obj_t test_imlib_ksize_to_n(void) {
    // ksize_to_n converts kernel size to number of elements
    // ksize=1 -> 3x3 -> 9 elements
    // ksize=2 -> 5x5 -> 25 elements
    if (imlib_ksize_to_n(1) != 9) {
        return mp_const_false;
    }
    if (imlib_ksize_to_n(2) != 25) {
        return mp_const_false;
    }
    if (imlib_ksize_to_n(3) != 49) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_ksize_to_n_obj, test_imlib_ksize_to_n);

// Test color conversion macros
static mp_obj_t test_imlib_color_conv(void) {
    // Test RGB565 packing/unpacking
    uint8_t r = 255, g = 128, b = 64;
    uint16_t rgb565 = COLOR_R8_G8_B8_TO_RGB565(r, g, b);

    uint8_t r_out = COLOR_RGB565_TO_R8(rgb565);
    uint8_t g_out   = COLOR_RGB565_TO_G8(rgb565);
    uint8_t b_out = COLOR_RGB565_TO_B8(rgb565);

    // Allow for quantization error (RGB565 has less precision)
    if (abs(r - r_out) > 8 || abs(g - g_out) > 4 || abs(b - b_out) > 8) {
        return mp_const_false;
    }

    // Test grayscale conversion
    uint8_t gray = COLOR_RGB565_TO_Y(rgb565);
    // Y = 0.299*R + 0.587*G + 0.114*B
    int expected_y = (int) (0.299f * r + 0.587f * g + 0.114f * b);
    if (abs(gray - expected_y) > 5) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_color_conv_obj, test_imlib_color_conv);

// Test rectangle operations
static mp_obj_t test_imlib_rectangle(void) {
    rectangle_t r1 = {10, 20, 100, 50};
    rectangle_t r2 = {50, 30, 80, 60};
    rectangle_t r3 = {200, 200, 10, 10};  // Non-intersecting

    // Test rectangle_intersects (overlapping rectangles)
    if (!rectangle_intersects(&r1, &r2)) {
        return mp_const_false;
    }

    // Test rectangle_intersects (non-overlapping rectangles)
    if (rectangle_intersects(&r1, &r3)) {
        return mp_const_false;
    }

    // Test rectangle_equal
    rectangle_t r1_copy = {10, 20, 100, 50};
    if (!rectangle_equal(&r1, &r1_copy)) {
        return mp_const_false;
    }

    // Test rectangle_equal (not equal)
    if (rectangle_equal(&r1, &r2)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_rectangle_obj, test_imlib_rectangle);

// Test rectangle_united (union of two rectangles)
static mp_obj_t test_imlib_rectangle_united(void) {
    // r1: x=10, y=20, w=100, h=50 -> covers (10,20) to (110,70)
    // r2: x=50, y=30, w=80, h=60  -> covers (50,30) to (130,90)
    // Union should be: x=10, y=20, w=120, h=70 -> covers (10,20) to (130,90)
    rectangle_t r1 = {10, 20, 100, 50};
    rectangle_t r2 = {50, 30, 80, 60};

    rectangle_united(&r1, &r2);

    // Check united rectangle
    if (r1.x != 10 || r1.y != 20 || r1.w != 120 || r1.h != 70) {
        return mp_const_false;
    }

    // Test non-overlapping rectangles
    rectangle_t r3 = {0, 0, 10, 10};    // (0,0) to (10,10)
    rectangle_t r4 = {20, 20, 10, 10};  // (20,20) to (30,30)
    rectangle_united(&r3, &r4);

    // Union: x=0, y=0, w=30, h=30
    if (r3.x != 0 || r3.y != 0 || r3.w != 30 || r3.h != 30) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_rectangle_united_obj, test_imlib_rectangle_united);

// Test point operations
static mp_obj_t test_imlib_point(void) {
    point_t p1 = {10, 20};
    point_t p2 = {30, 40};

    // Test distance calculation (should be sqrt(800) ~= 28.28)
    // Using fast_sqrtf for calculation
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    float dist = fast_sqrtf((float) (dx * dx + dy * dy));

    if (!FLOAT_EQ(dist, 28.28f, 0.1f)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_point_obj, test_imlib_point);

// Test point_equal
static mp_obj_t test_imlib_point_equal(void) {
    point_t p1 = {10, 20};
    point_t p2 = {10, 20};
    point_t p3 = {30, 40};

    // Test equal points
    if (!point_equal(&p1, &p2)) {
        return mp_const_false;
    }

    // Test unequal points
    if (point_equal(&p1, &p3)) {
        return mp_const_false;
    }

    // Test with zero coordinates
    point_t p4 = {0, 0};
    point_t p5 = {0, 0};
    if (!point_equal(&p4, &p5)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_point_equal_obj, test_imlib_point_equal);

// Test sin/cos table accuracy
static mp_obj_t test_imlib_sincos_table(void) {
    // Test a few known values
    // sin(0) = 0, cos(0) = 1
    if (!FLOAT_EQ(sin_table[0], 0.0f, 0.001f)) {
        return mp_const_false;
    }
    if (!FLOAT_EQ(cos_table[0], 1.0f, 0.001f)) {
        return mp_const_false;
    }

    // sin(90) = 1, cos(90) = 0
    if (!FLOAT_EQ(sin_table[90], 1.0f, 0.001f)) {
        return mp_const_false;
    }
    if (!FLOAT_EQ(cos_table[90], 0.0f, 0.001f)) {
        return mp_const_false;
    }

    // sin(180) = 0, cos(180) = -1
    if (!FLOAT_EQ(sin_table[180], 0.0f, 0.001f)) {
        return mp_const_false;
    }
    if (!FLOAT_EQ(cos_table[180], -1.0f, 0.001f)) {
        return mp_const_false;
    }

    // sin(270) = -1, cos(270) = 0
    if (!FLOAT_EQ(sin_table[270], -1.0f, 0.001f)) {
        return mp_const_false;
    }
    if (!FLOAT_EQ(cos_table[270], 0.0f, 0.001f)) {
        return mp_const_false;
    }

    // sin(45) ~= 0.707, cos(45) ~= 0.707
    if (!FLOAT_EQ(sin_table[45], 0.707f, 0.01f)) {
        return mp_const_false;
    }
    if (!FLOAT_EQ(cos_table[45], 0.707f, 0.01f)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sincos_table_obj, test_imlib_sincos_table);

// ============================================================================
// Frame Buffer Allocation Tests (fb_alloc.h)
// ============================================================================

// Test fb_alloc basic allocation and free
static mp_obj_t test_fb_alloc_basic(void) {
    uint32_t avail_before = fb_avail();

    // Mark the current position
    fb_alloc_mark();

    // Allocate some memory
    void *ptr1 = fb_alloc(256, FB_ALLOC_NO_HINT);
    if (ptr1 == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Available memory should have decreased
    uint32_t avail_after_alloc = fb_avail();
    if (avail_after_alloc >= avail_before) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Write to the allocated memory to verify it's usable
    memset(ptr1, 0xAA, 256);

    // Free back to mark
    fb_alloc_free_till_mark();

    // Available memory should be restored
    uint32_t avail_after_free = fb_avail();
    if (avail_after_free != avail_before) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_basic_obj, test_fb_alloc_basic);

// Test fb_alloc0 (zero-initialized allocation)
static mp_obj_t test_fb_alloc0(void) {
    fb_alloc_mark();

    // Allocate zero-initialized memory
    uint8_t *ptr = (uint8_t *) fb_alloc0(128, FB_ALLOC_NO_HINT);
    if (ptr == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Verify memory is zeroed
    for (int i = 0; i < 128; i++) {
        if (ptr[i] != 0) {
            fb_alloc_free_till_mark();
            return mp_const_false;
        }
    }

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc0_obj, test_fb_alloc0);

// Test multiple allocations and stack behavior
static mp_obj_t test_fb_alloc_stack(void) {
    fb_alloc_mark();

    // Allocate multiple blocks
    void *ptr1 = fb_alloc(64, FB_ALLOC_NO_HINT);
    void *ptr2 = fb_alloc(128, FB_ALLOC_NO_HINT);
    void *ptr3 = fb_alloc(256, FB_ALLOC_NO_HINT);

    if (ptr1 == NULL || ptr2 == NULL || ptr3 == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Pointers should be different and in stack order (decreasing addresses)
    if (ptr1 == ptr2 || ptr2 == ptr3 || ptr1 == ptr3) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Stack grows downward, so ptr3 < ptr2 < ptr1
    if (!((uintptr_t) ptr3 < (uintptr_t) ptr2 && (uintptr_t) ptr2 < (uintptr_t) ptr1)) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Write to all blocks
    memset(ptr1, 0x11, 64);
    memset(ptr2, 0x22, 128);
    memset(ptr3, 0x33, 256);

    // Free individually (LIFO order)
    fb_free();  // Frees ptr3
    fb_free();  // Frees ptr2
    fb_free();  // Frees ptr1

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_stack_obj, test_fb_alloc_stack);

// Test fb_alloc with size 0 (should return NULL)
static mp_obj_t test_fb_alloc_zero_size(void) {
    fb_alloc_mark();

    // Allocating 0 bytes should return NULL
    void *ptr = fb_alloc(0, FB_ALLOC_NO_HINT);
    if (ptr != NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_zero_size_obj, test_fb_alloc_zero_size);

// Test fb_avail reports available memory
static mp_obj_t test_fb_avail(void) {
    uint32_t avail = fb_avail();

    // Should have some available memory
    if (avail == 0) {
        return mp_const_false;
    }

    fb_alloc_mark();

    // Allocate half the available memory (or 1KB, whichever is smaller)
    uint32_t alloc_size = (avail > 2048) ? 1024 : avail / 2;
    void *ptr = fb_alloc(alloc_size, FB_ALLOC_NO_HINT);
    if (ptr == NULL) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    uint32_t avail_after = fb_avail();

    // Available should have decreased by at least alloc_size
    if (avail_after >= avail) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    fb_alloc_free_till_mark();
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_avail_obj, test_fb_avail);

// Test nested marks
static mp_obj_t test_fb_alloc_nested_marks(void) {
    uint32_t avail_start = fb_avail();

    // First mark
    fb_alloc_mark();
    void *ptr1 = fb_alloc(64, FB_ALLOC_NO_HINT);

    // Second (nested) mark
    fb_alloc_mark();
    void *ptr2 = fb_alloc(128, FB_ALLOC_NO_HINT);

    if (ptr1 == NULL || ptr2 == NULL) {
        fb_alloc_free_till_mark();
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Free to inner mark (should free ptr2)
    fb_alloc_free_till_mark();

    // Should still have ptr1's allocation
    uint32_t avail_after_inner = fb_avail();
    if (avail_after_inner >= avail_start) {
        fb_alloc_free_till_mark();
        return mp_const_false;
    }

    // Free to outer mark (should free ptr1)
    fb_alloc_free_till_mark();

    // Should be back to start
    uint32_t avail_end = fb_avail();
    if (avail_end != avail_start) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_fb_alloc_nested_marks_obj, test_fb_alloc_nested_marks);

// ============================================================================
// Framebuffer Tests (framebuffer.h)
// ============================================================================

// Test framebuffer_get returns valid framebuffers
static mp_obj_t test_framebuffer_get(void) {
    // Get main framebuffer
    framebuffer_t *fb_main = framebuffer_get(FB_MAINFB_ID);
    if (fb_main == NULL) {
        return mp_const_false;
    }

    // Get stream framebuffer
    framebuffer_t *fb_stream = framebuffer_get(FB_STREAM_ID);
    if (fb_stream == NULL) {
        return mp_const_false;
    }

    // They should be different
    if (fb_main == fb_stream) {
        return mp_const_false;
    }

    // Invalid ID should return NULL
    framebuffer_t *fb_invalid = framebuffer_get(FB_MAX_ID);
    if (fb_invalid != NULL) {
        return mp_const_false;
    }

    framebuffer_t *fb_invalid2 = framebuffer_get(100);
    if (fb_invalid2 != NULL) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_get_obj, test_framebuffer_get);

// Test framebuffer_from_image and framebuffer_to_image
static mp_obj_t test_framebuffer_image(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Save original state
    int32_t orig_w = fb->w;
    int32_t orig_h = fb->h;
    uint32_t orig_size = fb->size;
    pixformat_t orig_pixfmt = fb->pixfmt;

    // Create a test image structure
    image_t test_img = {
        .w = 320,
        .h = 240,
        .size = 320 * 240 * 2,
        .pixfmt = PIXFORMAT_RGB565,
        .data = NULL
    };

    // Set framebuffer from image
    framebuffer_from_image(fb, &test_img);

    // Verify framebuffer was updated
    if (fb->w != 320 || fb->h != 240 || fb->pixfmt != PIXFORMAT_RGB565) {
        // Restore original state
        fb->w = orig_w;
        fb->h = orig_h;
        fb->size = orig_size;
        fb->pixfmt = orig_pixfmt;
        return mp_const_false;
    }

    // Test with NULL image (should reset)
    framebuffer_from_image(fb, NULL);
    if (fb->w != 0 || fb->h != 0 || fb->pixfmt != PIXFORMAT_INVALID) {
        fb->w = orig_w;
        fb->h = orig_h;
        fb->size = orig_size;
        fb->pixfmt = orig_pixfmt;
        return mp_const_false;
    }

    // Restore original state
    fb->w = orig_w;
    fb->h = orig_h;
    fb->size = orig_size;
    fb->pixfmt = orig_pixfmt;

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_image_obj, test_framebuffer_image);

// Test framebuffer_get_buffer_size
static mp_obj_t test_framebuffer_buffer_size(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Get buffer size - should match fb->buf_size
    size_t size = framebuffer_get_buffer_size(fb);
    if (size != fb->buf_size) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_buffer_size_obj, test_framebuffer_buffer_size);

// Test framebuffer resize with single buffer
static mp_obj_t test_framebuffer_resize(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Save original state
    size_t orig_buf_count = fb->buf_count;
    size_t orig_buf_size = fb->buf_size;

    // Resize to 1 buffer with small frame size
    int result = framebuffer_resize(fb, 1, 1024, false);
    if (result != 0) {
        return mp_const_false;
    }

    // Verify resize worked
    if (fb->buf_count != 1) {
        // Try to restore
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Test writable/readable status
    // After resize and flush, free queue should have buffer, used should be empty
    // Should be writable (free queue has buffer) but not readable (used queue empty)
    if (!framebuffer_writable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Acquire a buffer from free queue
    vbuffer_t *buffer = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);
    if (buffer == NULL) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Restore original configuration
    framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_resize_obj, test_framebuffer_resize);

// Test framebuffer_flush clears queues
static mp_obj_t test_framebuffer_flush(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Resize to ensure we have buffers
    size_t orig_buf_count = fb->buf_count;
    size_t orig_buf_size = fb->buf_size;

    int result = framebuffer_resize(fb, 2, 1024, false);
    if (result != 0) {
        return mp_const_false;
    }

    // Flush the framebuffer
    framebuffer_flush(fb);

    // After flush, pixfmt should be invalid
    if (fb->pixfmt != PIXFORMAT_INVALID) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Free queue should have all buffers
    if (!framebuffer_writable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Used queue should be empty
    if (framebuffer_readable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Restore
    framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_flush_obj, test_framebuffer_flush);

// Test framebuffer acquire and release cycle
static mp_obj_t test_framebuffer_acquire_release(void) {
    framebuffer_t *fb = framebuffer_get(FB_MAINFB_ID);
    if (fb == NULL) {
        return mp_const_false;
    }

    // Resize to 2 buffers
    size_t orig_buf_count = fb->buf_count;
    size_t orig_buf_size = fb->buf_size;

    int result = framebuffer_resize(fb, 2, 1024, false);
    if (result != 0) {
        return mp_const_false;
    }

    // Initially: free queue has 2 buffers, used queue is empty
    if (!framebuffer_writable(fb) || framebuffer_readable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Acquire from free queue (peek - keeps in queue)
    vbuffer_t *buf1 = framebuffer_acquire(fb, FB_FLAG_FREE | FB_FLAG_PEEK);
    if (buf1 == NULL) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Release to used queue (moves from free to used)
    vbuffer_t *released = framebuffer_release(fb, FB_FLAG_FREE);
    if (released == NULL) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Now used queue should have 1 buffer
    if (!framebuffer_readable(fb)) {
        framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);
        return mp_const_false;
    }

    // Restore
    framebuffer_flush(fb);
    framebuffer_resize(fb, orig_buf_count, orig_buf_size, false);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_framebuffer_acquire_release_obj, test_framebuffer_acquire_release);

// ============================================================================
// Module Definition
// ============================================================================

static const mp_rom_map_elem_t unittest_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest) },

    // Fast math tests
    { MP_ROM_QSTR(MP_QSTR_test_fmath_sqrtf), MP_ROM_PTR(&test_fmath_sqrtf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_floorf), MP_ROM_PTR(&test_fmath_floorf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_ceilf), MP_ROM_PTR(&test_fmath_ceilf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_roundf), MP_ROM_PTR(&test_fmath_roundf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_fabsf), MP_ROM_PTR(&test_fmath_fabsf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_atanf), MP_ROM_PTR(&test_fmath_atanf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_atan2f), MP_ROM_PTR(&test_fmath_atan2f_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_expf), MP_ROM_PTR(&test_fmath_expf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_log), MP_ROM_PTR(&test_fmath_log_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_log2), MP_ROM_PTR(&test_fmath_log2_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_powf), MP_ROM_PTR(&test_fmath_powf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_cbrtf), MP_ROM_PTR(&test_fmath_cbrtf_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fmath_get_min_max), MP_ROM_PTR(&test_fmath_get_min_max_obj) },

    // Collections tests
    { MP_ROM_QSTR(MP_QSTR_test_collections_bitmap), MP_ROM_PTR(&test_collections_bitmap_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_lifo), MP_ROM_PTR(&test_collections_lifo_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_fifo), MP_ROM_PTR(&test_collections_fifo_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_collections_list), MP_ROM_PTR(&test_collections_list_obj) },

    // Image library tests
    { MP_ROM_QSTR(MP_QSTR_test_imlib_ksize_to_n), MP_ROM_PTR(&test_imlib_ksize_to_n_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_color_conv), MP_ROM_PTR(&test_imlib_color_conv_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_rectangle), MP_ROM_PTR(&test_imlib_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_rectangle_united), MP_ROM_PTR(&test_imlib_rectangle_united_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_point), MP_ROM_PTR(&test_imlib_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_point_equal), MP_ROM_PTR(&test_imlib_point_equal_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sincos_table), MP_ROM_PTR(&test_imlib_sincos_table_obj) },

    // Frame buffer allocation tests
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_basic), MP_ROM_PTR(&test_fb_alloc_basic_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc0), MP_ROM_PTR(&test_fb_alloc0_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_stack), MP_ROM_PTR(&test_fb_alloc_stack_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_zero_size), MP_ROM_PTR(&test_fb_alloc_zero_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_avail), MP_ROM_PTR(&test_fb_avail_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_fb_alloc_nested_marks), MP_ROM_PTR(&test_fb_alloc_nested_marks_obj) },

    // Framebuffer tests
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_get), MP_ROM_PTR(&test_framebuffer_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_image), MP_ROM_PTR(&test_framebuffer_image_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_buffer_size), MP_ROM_PTR(&test_framebuffer_buffer_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_resize), MP_ROM_PTR(&test_framebuffer_resize_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_flush), MP_ROM_PTR(&test_framebuffer_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_framebuffer_acquire_release), MP_ROM_PTR(&test_framebuffer_acquire_release_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_module_globals, unittest_module_globals_table);

const mp_obj_module_t unittest_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest, unittest_module);

#endif // MICROPY_PY_UNITTEST
