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
#include "simd.h"

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
// SIMD Tests (simd.h)
// ============================================================================

// Test vdup - duplicate scalar to all lanes
// Note: ARM MVE intrinsics require constant lane indices, so we can't use loops
static mp_obj_t test_simd_vdup(void) {
    // Test vdup_u8 - check first lane (all lanes have same value)
    v128_t v = vdup_u8(0x42);
    if (vget_u8(v, 0) != 0x42) {
        return mp_const_false;
    }

    // Test vdup_s8 with negative value
    v = vdup_s8(-10);
    if (vget_s8(v, 0) != -10) {
        return mp_const_false;
    }

    // Test vdup_u16
    v = vdup_u16(0x1234);
    if (vget_u16(v, 0) != 0x1234) {
        return mp_const_false;
    }

    // Test vdup_s16 with negative value
    v = vdup_s16(-1000);
    if (vget_s16(v, 0) != -1000) {
        return mp_const_false;
    }

    // Test vdup_u32
    v = vdup_u32(0xDEADBEEF);
    if (vget_u32(v, 0) != 0xDEADBEEF) {
        return mp_const_false;
    }

    // Test vdup_s32 with negative value
    v = vdup_s32(-100000);
    if (vget_s32(v, 0) != -100000) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vdup_obj, test_simd_vdup);

// Test vidup - incrementing sequence
// Note: ARM MVE intrinsics require constant lane indices
static mp_obj_t test_simd_vidup(void) {
    // Test vidup_u8 with start=0, increment=1
    // Expected: [0, 1, 2, 3, ...]
    v128_t v = vidup_u8(0, 1);
    if (vget_u8(v, 0) != 0 || vget_u8(v, 1) != 1 ||
        vget_u8(v, 2) != 2 || vget_u8(v, 3) != 3) {
        return mp_const_false;
    }

    // Test vidup_u8 with start=10, increment=2
    // Expected: [10, 12, 14, 16, ...]
    v = vidup_u8(10, 2);
    if (vget_u8(v, 0) != 10 || vget_u8(v, 1) != 12 ||
        vget_u8(v, 2) != 14 || vget_u8(v, 3) != 16) {
        return mp_const_false;
    }

    // Test vidup_u16 with start=100, increment=4
    // Expected: [100, 104, ...]
    v = vidup_u16(100, 4);
    if (vget_u16(v, 0) != 100 || vget_u16(v, 1) != 104) {
        return mp_const_false;
    }

    // Test vidup_u32 with start=1000, increment=1
    // Expected: [1000, 1001, ...]
    v = vidup_u32(1000, 1);
    if (vget_u32(v, 0) != 1000) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vidup_obj, test_simd_vidup);

// Test vget/vset lane access
static mp_obj_t test_simd_vget_vset(void) {
    v128_t v = vdup_u32(0);

    // Test vset and vget for u8
    v = vset_u8(v, 0, 0x11);
    v = vset_u8(v, 1, 0x22);
    v = vset_u8(v, 2, 0x33);
    v = vset_u8(v, 3, 0x44);
    if (vget_u8(v, 0) != 0x11 || vget_u8(v, 1) != 0x22 ||
        vget_u8(v, 2) != 0x33 || vget_u8(v, 3) != 0x44) {
        return mp_const_false;
    }

    // Test vset and vget for u16
    v = vdup_u32(0);
    v = vset_u16(v, 0, 0xABCD);
    v = vset_u16(v, 1, 0x1234);
    if (vget_u16(v, 0) != 0xABCD || vget_u16(v, 1) != 0x1234) {
        return mp_const_false;
    }

    // Test vset and vget for u32
    v = vset_u32(v, 0, 0xCAFEBABE);
    if (vget_u32(v, 0) != 0xCAFEBABE) {
        return mp_const_false;
    }

    // Test signed variants
    v = vdup_s32(0);
    v = vset_s8(v, 0, -128);
    v = vset_s8(v, 1, 127);
    if (vget_s8(v, 0) != -128 || vget_s8(v, 1) != 127) {
        return mp_const_false;
    }

    v = vdup_s32(0);
    v = vset_s16(v, 0, -32768);
    v = vset_s16(v, 1, 32767);
    if (vget_s16(v, 0) != -32768 || vget_s16(v, 1) != 32767) {
        return mp_const_false;
    }

    v = vset_s32(v, 0, -2147483647);
    if (vget_s32(v, 0) != -2147483647) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vget_vset_obj, test_simd_vget_vset);

// Test vector addition - verifies ALL lanes produce correct results
static mp_obj_t test_simd_vadd(void) {
    // Test vadd_u32: all lanes should compute 100 + 50 = 150
    v128_t a = vdup_u32(100);
    v128_t b = vdup_u32(50);
    v128_t c = vadd_u32(a, b);
    v128_t expected = vdup_u32(150);
    v128_t diff = veor_u32(c, expected);  // XOR: 0 if all lanes match
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vadd_s32 with negative: all lanes should compute -100 + 50 = -50
    a = vdup_s32(-100);
    b = vdup_s32(50);
    c = vadd_s32(a, b);
    expected = vdup_s32(-50);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vadd_n_u32: all lanes should compute 1000 + 234 = 1234
    a = vdup_u32(1000);
    c = vadd_n_u32(a, 234);
    expected = vdup_u32(1234);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vadd_n_s32 with negative: all lanes should compute 100 + (-150) = -50
    a = vdup_s32(100);
    c = vadd_n_s32(a, -150);
    expected = vdup_s32(-50);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vadd_obj, test_simd_vadd);

// Test vector subtraction - verifies ALL lanes produce correct results
static mp_obj_t test_simd_vsub(void) {
    v128_t a, b, c, expected, diff;

    // Test vsub_u8: all lanes should compute 100 - 30 = 70
    a = vdup_u8(100);
    b = vdup_u8(30);
    c = vsub_u8(a, b);
    expected = vdup_u8(70);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vsub_s8 with negative result: all lanes should compute 10 - 30 = -20
    a = vdup_s8(10);
    b = vdup_s8(30);
    c = vsub_s8(a, b);
    expected = vdup_s8(-20);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vsub_u16: all lanes should compute 1000 - 400 = 600
    a = vdup_u16(1000);
    b = vdup_u16(400);
    c = vsub_u16(a, b);
    expected = vdup_u16(600);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vsub_s16 with negative: all lanes should compute -100 - 200 = -300
    a = vdup_s16(-100);
    b = vdup_s16(200);
    c = vsub_s16(a, b);
    expected = vdup_s16(-300);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vsub_n_u32: all lanes should compute 1000 - 100 = 900
    a = vdup_u32(1000);
    c = vsub_n_u32(a, 100);
    expected = vdup_u32(900);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vsub_n_s32: all lanes should compute 50 - 100 = -50
    a = vdup_s32(50);
    c = vsub_n_s32(a, 100);
    expected = vdup_s32(-50);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vsub_obj, test_simd_vsub);

// Test vector multiplication - verifies ALL lanes produce correct results
static mp_obj_t test_simd_vmul(void) {
    v128_t a, b, c, expected, diff;

    // Test vmul_u32: all lanes should compute 7 * 8 = 56
    a = vdup_u32(7);
    b = vdup_u32(8);
    c = vmul_u32(a, b);
    expected = vdup_u32(56);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmul_s32 with negative: all lanes should compute -5 * 10 = -50
    a = vdup_s32(-5);
    b = vdup_s32(10);
    c = vmul_s32(a, b);
    expected = vdup_s32(-50);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmul_n_u16: all lanes should compute 100 * 5 = 500
    a = vdup_u16(100);
    c = vmul_n_u16(a, 5);
    expected = vdup_u16(500);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmul_n_u32: all lanes should compute 1000 * 7 = 7000
    a = vdup_u32(1000);
    c = vmul_n_u32(a, 7);
    expected = vdup_u32(7000);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmul_n_s16 with negative: all lanes should compute -100 * 3 = -300
    a = vdup_s16(-100);
    c = vmul_n_s16(a, 3);
    expected = vdup_s16(-300);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmul_n_s32: all lanes should compute 50 * -4 = -200
    a = vdup_s32(50);
    c = vmul_n_s32(a, -4);
    expected = vdup_s32(-200);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vmul_obj, test_simd_vmul);

// Test multiply-accumulate - verifies ALL lanes produce correct results
static mp_obj_t test_simd_vmla(void) {
    v128_t a, b, c, r, expected, diff;

    // Test vmla_u32: all lanes should compute 5 * 10 + 7 = 57
    a = vdup_u32(5);
    b = vdup_u32(10);
    c = vdup_u32(7);
    r = vmla_u32(a, b, c);
    expected = vdup_u32(57);
    diff = veor_u32(r, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmla_s32 with negative: all lanes should compute -3 * 4 + 20 = 8
    a = vdup_s32(-3);
    b = vdup_s32(4);
    c = vdup_s32(20);
    r = vmla_s32(a, b, c);
    expected = vdup_s32(8);
    diff = veor_u32(r, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmla_n_u16: all lanes should compute 10 * 3 + 5 = 35
    a = vdup_u16(10);
    c = vdup_u16(5);
    r = vmla_n_u16(a, 3, c);
    expected = vdup_u16(35);
    diff = veor_u32(r, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmla_n_u32: all lanes should compute 100 * 7 + 50 = 750
    a = vdup_u32(100);
    c = vdup_u32(50);
    r = vmla_n_u32(a, 7, c);
    expected = vdup_u32(750);
    diff = veor_u32(r, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmla_n_s16 with negative: all lanes should compute -5 * 10 + 100 = 50
    a = vdup_s16(-5);
    c = vdup_s16(100);
    r = vmla_n_s16(a, 10, c);
    expected = vdup_s16(50);
    diff = veor_u32(r, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vmla_n_s32: all lanes should compute 20 * 5 + (-50) = 50
    a = vdup_s32(20);
    c = vdup_s32(-50);
    r = vmla_n_s32(a, 5, c);
    expected = vdup_s32(50);
    diff = veor_u32(r, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vmla_obj, test_simd_vmla);

// Test logical operations: AND, OR, XOR - verifies ALL lanes produce correct results
static mp_obj_t test_simd_vand_vorr_veor(void) {
    v128_t a, b, c, expected, diff;

    // Test vand_u32: all lanes should compute 0xFF00FF00 & 0x0F0F0F0F = 0x0F000F00
    a = vdup_u32(0xFF00FF00);
    b = vdup_u32(0x0F0F0F0F);
    c = vand_u32(a, b);
    expected = vdup_u32(0x0F000F00);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vorr_u32: all lanes should compute 0xFF000000 | 0x00FF0000 = 0xFFFF0000
    a = vdup_u32(0xFF000000);
    b = vdup_u32(0x00FF0000);
    c = vorr_u32(a, b);
    expected = vdup_u32(0xFFFF0000);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test veor_u32: all lanes should compute 0xAAAAAAAA ^ 0xFFFFFFFF = 0x55555555
    a = vdup_u32(0xAAAAAAAA);
    b = vdup_u32(0xFFFFFFFF);
    c = veor_u32(a, b);
    expected = vdup_u32(0x55555555);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vand_s32: all lanes should compute 0x7FFFFFFF & 0x0000FFFF = 0x0000FFFF
    a = vdup_s32(0x7FFFFFFF);
    b = vdup_s32(0x0000FFFF);
    c = vand_s32(a, b);
    expected = vdup_s32(0x0000FFFF);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test vorr_s32: all lanes should compute 0x00FF0000 | 0x000000FF = 0x00FF00FF
    a = vdup_s32(0x00FF0000);
    b = vdup_s32(0x000000FF);
    c = vorr_s32(a, b);
    expected = vdup_s32(0x00FF00FF);
    diff = veor_u32(c, expected);
    if (vget_u32(diff, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(diff, 1) != 0 || vget_u32(diff, 2) != 0 || vget_u32(diff, 3) != 0) {
        return mp_const_false;
    }
    #endif

    // Test veor_s32: all lanes should compute 0x12345678 ^ 0x12345678 = 0
    a = vdup_s32(0x12345678);
    b = vdup_s32(0x12345678);
    c = veor_s32(a, b);
    // c should be all zeros
    if (vget_u32(c, 0) != 0) {
        return mp_const_false;
    }
    #if (UINT32_VECTOR_SIZE > 1)
    if (vget_u32(c, 1) != 0 || vget_u32(c, 2) != 0 || vget_u32(c, 3) != 0) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vand_vorr_veor_obj, test_simd_vand_vorr_veor);

// Test left shift operations
// Note: ARM MVE intrinsics require constant lane indices
static mp_obj_t test_simd_vshift_left(void) {
    // Test vlsl_u16
    v128_t v = vdup_u16(0x0001);
    v128_t r = vlsl_u16(v, 4);
    if (vget_u16(r, 0) != 0x0010) {
        return mp_const_false;
    }

    // Test vlsl_u32
    v = vdup_u32(0x00000001);
    r = vlsl_u32(v, 16);
    if (vget_u32(r, 0) != 0x00010000) {
        return mp_const_false;
    }

    // Test vlsl_s16
    v = vdup_s16(1);
    r = vlsl_s16(v, 8);
    if (vget_s16(r, 0) != 256) {
        return mp_const_false;
    }

    // Test vlsl_s32
    v = vdup_s32(1);
    r = vlsl_s32(v, 24);
    if (vget_s32(r, 0) != 0x01000000) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vshift_left_obj, test_simd_vshift_left);

// Test right shift operations
// Note: ARM MVE intrinsics require constant lane indices
static mp_obj_t test_simd_vshift_right(void) {
    // Test vlsr_u16 (logical shift right)
    v128_t v = vdup_u16(0xFF00);
    v128_t r = vlsr_u16(v, 4);
    if (vget_u16(r, 0) != 0x0FF0) {
        return mp_const_false;
    }

    // Test vlsr_u32
    v = vdup_u32(0x80000000);
    r = vlsr_u32(v, 16);
    if (vget_u32(r, 0) != 0x00008000) {
        return mp_const_false;
    }

    // Test vasr_s16 (arithmetic shift right - preserves sign)
    v = vdup_s16(-256);  // 0xFF00
    r = vasr_s16(v, 4);
    if (vget_s16(r, 0) != -16) {
        return mp_const_false;
    }

    // Test vasr_s32
    v = vdup_s32(-65536);  // 0xFFFF0000
    r = vasr_s32(v, 8);
    if (vget_s32(r, 0) != -256) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vshift_right_obj, test_simd_vshift_right);

// Test shift left/right insert
// Note: ARM MVE intrinsics require constant lane indices
static mp_obj_t test_simd_vsli_vsri(void) {
    // Test vsli_u8: shift left insert (keeps lower bits of v0, inserts shifted v1 in upper bits)
    v128_t v0 = vdup_u8(0x0F);
    v128_t v1 = vdup_u8(0x05);
    v128_t r = vsli_u8(v0, v1, 4);
    // v1 << 4 = 0x50, keep lower 4 bits of v0 = 0x0F
    // Result: 0x50 | 0x0F = 0x5F
    if (vget_u8(r, 0) != 0x5F) {
        return mp_const_false;
    }

    // Test vsli_u16
    v0 = vdup_u16(0x00FF);
    v1 = vdup_u16(0x00AB);
    r = vsli_u16(v0, v1, 8);
    // v1 << 8 = 0xAB00, keep lower 8 bits of v0 = 0x00FF
    // Result: 0xAB00 | 0x00FF = 0xABFF
    if (vget_u16(r, 0) != 0xABFF) {
        return mp_const_false;
    }

    // Test vsri_u8: shift right insert (keeps upper bits of v0, inserts shifted v1 in lower bits)
    v0 = vdup_u8(0xF0);
    v1 = vdup_u8(0xA0);
    r = vsri_u8(v0, v1, 4);
    // v1 >> 4 = 0x0A, keep upper 4 bits of v0 = 0xF0
    // Result: 0xF0 | 0x0A = 0xFA
    if (vget_u8(r, 0) != 0xFA) {
        return mp_const_false;
    }

    // Test vsri_u16
    v0 = vdup_u16(0xFF00);
    v1 = vdup_u16(0xCD00);
    r = vsri_u16(v0, v1, 8);
    // v1 >> 8 = 0x00CD, keep upper 8 bits of v0 = 0xFF00
    // Result: 0xFF00 | 0x00CD = 0xFFCD
    if (vget_u16(r, 0) != 0xFFCD) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vsli_vsri_obj, test_simd_vsli_vsri);

// Test variable shift operations
static mp_obj_t test_simd_vshl(void) {
    // Test vshl_u8 - variable shift per element
    v128_t v = vdup_u8(1);
    v128_t shift = vidup_u8(0, 1);  // [0, 1, 2, 3]
    v128_t r = vshl_u8(v, shift);
    // Expected: [1, 2, 4, 8] = [1<<0, 1<<1, 1<<2, 1<<3]
    if (vget_u8(r, 0) != 1 || vget_u8(r, 1) != 2 ||
        vget_u8(r, 2) != 4 || vget_u8(r, 3) != 8) {
        return mp_const_false;
    }

    // Test vshl_u16
    v = vdup_u16(1);
    v128_t shift16 = vdup_u32(0);
    shift16 = vset_u16(shift16, 0, 4);
    shift16 = vset_u16(shift16, 1, 8);
    r = vshl_u16(v, shift16);
    if (vget_u16(r, 0) != 16 || vget_u16(r, 1) != 256) {
        return mp_const_false;
    }

    // Test vshlc - shift with carry
    uint32_t carry = 0x0F;
    v = vdup_u32(0x12345678);
    r = vshlc(v, &carry, 4);
    // After shift: 0x23456780 | 0x0F = 0x2345678F
    // Carry out: upper 4 bits of original = 0x01
    if (vget_u32(r, 0) != 0x2345678F) {
        return mp_const_false;
    }
    // Carry should now contain the bits shifted out (upper 4 bits = 0x1)
    if (carry != 0x01) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vshl_obj, test_simd_vshl);

// Test halving add
// Note: ARM MVE intrinsics require constant lane indices
static mp_obj_t test_simd_vhadd(void) {
    // Test vhadd_u8: (a + b) / 2 without overflow
    v128_t a = vdup_u8(200);
    v128_t b = vdup_u8(100);
    v128_t r = vhadd_u8(a, b);
    // (200 + 100) / 2 = 150
    if (vget_u8(r, 0) != 150) {
        return mp_const_false;
    }

    // Test vhadd_u8 with values that would overflow regular add
    a = vdup_u8(250);
    b = vdup_u8(250);
    r = vhadd_u8(a, b);
    // (250 + 250) / 2 = 250 (no overflow)
    if (vget_u8(r, 0) != 250) {
        return mp_const_false;
    }

    // Test vhadd_s8 with signed values
    a = vdup_s8(100);
    b = vdup_s8(-50);
    r = vhadd_s8(a, b);
    // (100 + (-50)) / 2 = 25
    if (vget_s8(r, 0) != 25) {
        return mp_const_false;
    }

    // Test vhadd_u16
    a = vdup_u16(60000);
    b = vdup_u16(60000);
    r = vhadd_u16(a, b);
    // (60000 + 60000) / 2 = 60000
    if (vget_u16(r, 0) != 60000) {
        return mp_const_false;
    }

    // Test vhadd_s16
    a = vdup_s16(20000);
    b = vdup_s16(-10000);
    r = vhadd_s16(a, b);
    // (20000 + (-10000)) / 2 = 5000
    if (vget_s16(r, 0) != 5000) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vhadd_obj, test_simd_vhadd);

// Test pack operations
static mp_obj_t test_simd_vpkhbt_vpkhtb(void) {
    // vpkhbt: pack half-word bottom-top
    // Takes lower 16 bits of v0 and upper 16 bits of (v1 << 16)
    v128_t v0 = vdup_u32(0);
    v0 = vset_u16(v0, 0, 0xAAAA);
    v0 = vset_u16(v0, 1, 0xBBBB);

    v128_t v1 = vdup_u32(0);
    v1 = vset_u16(v1, 0, 0xCCCC);
    v1 = vset_u16(v1, 1, 0xDDDD);

    v128_t r = vpkhbt(v0, v1);
    // Lower half from v0[0], upper half from v1[0]
    // Expected: 0xCCCCAAAA
    if (vget_u16(r, 0) != 0xAAAA || vget_u16(r, 1) != 0xCCCC) {
        return mp_const_false;
    }

    // vpkhtb: pack half-word top-bottom
    // Takes upper 16 bits of v0 and lower 16 bits of (v1 >> 16)
    v0 = vdup_u32(0);
    v0 = vset_u16(v0, 0, 0x1111);
    v0 = vset_u16(v0, 1, 0x2222);

    v1 = vdup_u32(0);
    v1 = vset_u16(v1, 0, 0x3333);
    v1 = vset_u16(v1, 1, 0x4444);

    r = vpkhtb(v0, v1);
    // Lower half from v1[1], upper half from v0[1]
    // Expected: 0x22224444
    if (vget_u16(r, 0) != 0x4444 || vget_u16(r, 1) != 0x2222) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vpkhbt_vpkhtb_obj, test_simd_vpkhbt_vpkhtb);

// Test zero/sign extension operations
static mp_obj_t test_simd_vuxtb_vsxtb(void) {
    // vuxtb16: zero-extend bytes at positions 0 and 2 to 16-bit
    v128_t v = vdup_u32(0);
    v = vset_u8(v, 0, 0x12);
    v = vset_u8(v, 1, 0x34);
    v = vset_u8(v, 2, 0x56);
    v = vset_u8(v, 3, 0x78);

    v128_t r = vuxtb16(v);
    // Should extract bytes 0 and 2, zero-extend to u16
    if (vget_u16(r, 0) != 0x0012 || vget_u16(r, 1) != 0x0056) {
        return mp_const_false;
    }

    // vuxtb16_ror8: extract bytes at positions 1 and 3
    r = vuxtb16_ror8(v);
    if (vget_u16(r, 0) != 0x0034 || vget_u16(r, 1) != 0x0078) {
        return mp_const_false;
    }

    // vsxtb16: sign-extend bytes at positions 0 and 2 to 16-bit
    v = vdup_u32(0);
    v = vset_s8(v, 0, -10);   // 0xF6
    v = vset_s8(v, 1, 20);
    v = vset_s8(v, 2, -50);   // 0xCE
    v = vset_s8(v, 3, 100);

    r = vsxtb16(v);
    if (vget_s16(r, 0) != -10 || vget_s16(r, 1) != -50) {
        return mp_const_false;
    }

    // vsxtb16_ror8: sign-extend bytes at positions 1 and 3
    r = vsxtb16_ror8(v);
    if (vget_s16(r, 0) != 20 || vget_s16(r, 1) != 100) {
        return mp_const_false;
    }

    // vuxtb32: zero-extend u16 at position 0 to u32
    v = vdup_u32(0);
    v = vset_u16(v, 0, 0xABCD);
    v = vset_u16(v, 1, 0x1234);

    r = vuxtb32(v);
    if (vget_u32(r, 0) != 0x0000ABCD) {
        return mp_const_false;
    }

    // vsxtb32: sign-extend s16 at position 0 to s32
    v = vdup_s32(0);
    v = vset_s16(v, 0, -1000);
    v = vset_s16(v, 1, 2000);

    r = vsxtb32(v);
    if (vget_s32(r, 0) != -1000) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vuxtb_vsxtb_obj, test_simd_vuxtb_vsxtb);

// Test narrow move operations
static mp_obj_t test_simd_vmov_narrow(void) {
    // vmov_u16_narrow_u8_lo: take low byte of each u16, store in low positions
    v128_t v0 = vdup_u32(0xAABBCCDD);
    v128_t v1 = vdup_u32(0);
    v1 = vset_u16(v1, 0, 0x1234);
    v1 = vset_u16(v1, 1, 0x5678);

    v128_t r = vmov_u16_narrow_u8_lo(v0, v1);
    // Low bytes from v1 (0x34, 0x78) go to positions 0 and 2
    // High bytes kept from v0
    if (vget_u8(r, 0) != 0x34 || vget_u8(r, 2) != 0x78) {
        return mp_const_false;
    }

    // vmov_u16_narrow_u8_hi: take low byte of each u16, store in high positions
    v0 = vdup_u32(0xAABBCCDD);
    v1 = vdup_u32(0);
    v1 = vset_u16(v1, 0, 0x1234);
    v1 = vset_u16(v1, 1, 0x5678);

    r = vmov_u16_narrow_u8_hi(v0, v1);
    // Low bytes from v1 shifted to positions 1 and 3
    // Low bytes kept from v0
    if (vget_u8(r, 1) != 0x34 || vget_u8(r, 3) != 0x78) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vmov_narrow_obj, test_simd_vmov_narrow);

// Test saturating narrow operations
static mp_obj_t test_simd_vusat_narrow(void) {
    // vusat_s16_narrow_u8_lo: saturate s16 to u8, store in low positions
    // Note: shift must be in range [1, 8] for ARMv8 intrinsics
    v128_t v0 = vdup_u32(0);
    v128_t v1 = vdup_s32(0);
    // Use shift=1, so values need to be 2x to get expected result
    v1 = vset_s16(v1, 0, 256);   // 256 >> 1 = 128
    v1 = vset_s16(v1, 1, 128);   // 128 >> 1 = 64

    v128_t r = vusat_s16_narrow_u8_lo(v0, v1, 1);
    if (vget_u8(r, 0) != 128 || vget_u8(r, 2) != 64) {
        return mp_const_false;
    }

    // Test with saturation - values outside u8 range after shift
    v1 = vset_s16(v1, 0, 600);   // 600 >> 1 = 300 > 255, should saturate to 255
    v1 = vset_s16(v1, 1, -20);   // -20 >> 1 = -10 < 0, should saturate to 0

    r = vusat_s16_narrow_u8_lo(v0, v1, 1);
    if (vget_u8(r, 0) != 255 || vget_u8(r, 2) != 0) {
        return mp_const_false;
    }

    // vusat_s16_narrow_u8_hi: saturate s16 to u8, store in high positions
    v0 = vdup_u32(0);
    v1 = vset_s16(v1, 0, 400);   // 400 >> 1 = 200
    v1 = vset_s16(v1, 1, 200);   // 200 >> 1 = 100

    r = vusat_s16_narrow_u8_hi(v0, v1, 1);
    if (vget_u8(r, 1) != 200 || vget_u8(r, 3) != 100) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vusat_narrow_obj, test_simd_vusat_narrow);

// Test dot product operations
static mp_obj_t test_simd_vmladav(void) {
    // vmladav_u16: sum of products of u16 pairs
    v128_t a = vdup_u32(0);
    a = vset_u16(a, 0, 10);
    a = vset_u16(a, 1, 20);

    v128_t b = vdup_u32(0);
    b = vset_u16(b, 0, 3);
    b = vset_u16(b, 1, 4);

    // 10*3 + 20*4 = 30 + 80 = 110
    uint32_t result = vmladav_u16(a, b);
    if (result != 110) {
        return mp_const_false;
    }

    // vmladav_s16: sum of products of s16 pairs
    a = vdup_s32(0);
    a = vset_s16(a, 0, -5);
    a = vset_s16(a, 1, 10);

    b = vdup_s32(0);
    b = vset_s16(b, 0, 4);
    b = vset_s16(b, 1, 3);

    // (-5)*4 + 10*3 = -20 + 30 = 10
    int32_t sresult = vmladav_s16(a, b);
    if (sresult != 10) {
        return mp_const_false;
    }

    // vmladava_u16: accumulate into existing sum
    a = vdup_u32(0);
    a = vset_u16(a, 0, 5);
    a = vset_u16(a, 1, 6);

    b = vdup_u32(0);
    b = vset_u16(b, 0, 2);
    b = vset_u16(b, 1, 3);

    // acc=100, 5*2 + 6*3 = 10 + 18 = 28, total = 128
    result = vmladava_u16(a, b, 100);
    if (result != 128) {
        return mp_const_false;
    }

    // vmladava_s16: accumulate with signed values
    a = vdup_s32(0);
    a = vset_s16(a, 0, -10);
    a = vset_s16(a, 1, 5);

    b = vdup_s32(0);
    b = vset_s16(b, 0, 2);
    b = vset_s16(b, 1, 4);

    // acc=50, (-10)*2 + 5*4 = -20 + 20 = 0, total = 50
    sresult = vmladava_s16(a, b, 50);
    if (sresult != 50) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vmladav_obj, test_simd_vmladav);

// Test type conversion operations
static mp_obj_t test_simd_vcvt(void) {
    // vcvt_u32_f32: convert u32 to f32
    v128_t v = vdup_u32(1000);
    v128_t r = vcvt_u32_f32(v);
    if (!FLOAT_EQ(r.f32[0], 1000.0f, 0.001f)) {
        return mp_const_false;
    }

    // vcvt_s32_f32: convert s32 to f32
    v = vdup_s32(-500);
    r = vcvt_s32_f32(v);
    if (!FLOAT_EQ(r.f32[0], -500.0f, 0.001f)) {
        return mp_const_false;
    }

    // vcvt_u8_f32: convert u8 values to f32 (returns 4 rows)
    v = vdup_u32(0);
    v = vset_u8(v, 0, 10);
    v = vset_u8(v, 1, 20);
    v = vset_u8(v, 2, 30);
    v = vset_u8(v, 3, 40);

    v4x_rows_t rows4 = vcvt_u8_f32(v);
    if (!FLOAT_EQ(rows4.r0.f32[0], 10.0f, 0.001f) ||
        !FLOAT_EQ(rows4.r1.f32[0], 20.0f, 0.001f) ||
        !FLOAT_EQ(rows4.r2.f32[0], 30.0f, 0.001f) ||
        !FLOAT_EQ(rows4.r3.f32[0], 40.0f, 0.001f)) {
        return mp_const_false;
    }

    // vcvt_s8_f32: convert s8 values to f32
    v = vdup_s32(0);
    v = vset_s8(v, 0, -10);
    v = vset_s8(v, 1, 20);
    v = vset_s8(v, 2, -30);
    v = vset_s8(v, 3, 40);

    rows4 = vcvt_s8_f32(v);
    if (!FLOAT_EQ(rows4.r0.f32[0], -10.0f, 0.001f) ||
        !FLOAT_EQ(rows4.r1.f32[0], 20.0f, 0.001f) ||
        !FLOAT_EQ(rows4.r2.f32[0], -30.0f, 0.001f) ||
        !FLOAT_EQ(rows4.r3.f32[0], 40.0f, 0.001f)) {
        return mp_const_false;
    }

    // vcvt_u16_f32: convert u16 values to f32 (returns 2 rows)
    v = vdup_u32(0);
    v = vset_u16(v, 0, 1000);
    v = vset_u16(v, 1, 2000);

    v2x_rows_t rows2 = vcvt_u16_f32(v);
    if (!FLOAT_EQ(rows2.r0.f32[0], 1000.0f, 0.001f) ||
        !FLOAT_EQ(rows2.r1.f32[0], 2000.0f, 0.001f)) {
        return mp_const_false;
    }

    // vcvt_s16_f32: convert s16 values to f32
    v = vdup_s32(0);
    v = vset_s16(v, 0, -1000);
    v = vset_s16(v, 1, 2000);

    rows2 = vcvt_s16_f32(v);
    if (!FLOAT_EQ(rows2.r0.f32[0], -1000.0f, 0.001f) ||
        !FLOAT_EQ(rows2.r1.f32[0], 2000.0f, 0.001f)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vcvt_obj, test_simd_vcvt);

// Test min/max operations
static mp_obj_t test_simd_vminmax(void) {
    // vminv_f32_pred: find minimum across vector elements
    v128_t v = vdup_u32(0);
    v.f32[0] = 10.0f;
    v128_predicate_t pred = vpredicate_32(1);

    float result = vminv_f32_pred(v, 100.0f, pred);
    if (!FLOAT_EQ(result, 10.0f, 0.001f)) {
        return mp_const_false;
    }

    // Test with min already smaller
    result = vminv_f32_pred(v, 5.0f, pred);
    if (!FLOAT_EQ(result, 5.0f, 0.001f)) {
        return mp_const_false;
    }

    // vmaxv_f32_pred: find maximum across vector elements
    v.f32[0] = 50.0f;
    result = vmaxv_f32_pred(v, 10.0f, pred);
    if (!FLOAT_EQ(result, 50.0f, 0.001f)) {
        return mp_const_false;
    }

    // Test with max already larger
    result = vmaxv_f32_pred(v, 100.0f, pred);
    if (!FLOAT_EQ(result, 100.0f, 0.001f)) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vminmax_obj, test_simd_vminmax);

// Test predicate operations
static mp_obj_t test_simd_predicate(void) {
    // vpredicate_8: create predicate for n u8 elements
    v128_predicate_t pred = vpredicate_8(3);
    uint32_t n = vpredicate_8_get_n(pred);
    if (n != 3) {
        return mp_const_false;
    }

    // vpredicate_16: create predicate for n u16 elements
    pred = vpredicate_16(2);
    n = vpredicate_16_get_n(pred);
    if (n != 2) {
        return mp_const_false;
    }

    // vpredicate_32: create predicate for n u32 elements
    pred = vpredicate_32(1);
    n = vpredicate_32_get_n(pred);
    if (n != 1) {
        return mp_const_false;
    }

    // Test all_lanes_active
    pred = vpredicate_8(UINT8_VECTOR_SIZE);
    if (!vpredicate_8_all_lanes_active(pred)) {
        return mp_const_false;
    }

    pred = vpredicate_8(UINT8_VECTOR_SIZE - 1);
    if (vpredicate_8_all_lanes_active(pred)) {
        return mp_const_false;
    }

    pred = vpredicate_16(UINT16_VECTOR_SIZE);
    if (!vpredicate_16_all_lanes_active(pred)) {
        return mp_const_false;
    }

    pred = vpredicate_32(UINT32_VECTOR_SIZE);
    if (!vpredicate_32_all_lanes_active(pred)) {
        return mp_const_false;
    }

    // Test vpredicate_add
    pred = vpredicate_8(2);
    pred = vpredicate_8_add(pred, 1);
    n = vpredicate_8_get_n(pred);
    if (n != 3) {
        return mp_const_false;
    }

    pred = vpredicate_16(1);
    pred = vpredicate_16_add(pred, 1);
    n = vpredicate_16_get_n(pred);
    if (n != 2) {
        return mp_const_false;
    }

    // Test vpredicate_*_get_mask functions
    // vpredicate_8_get_mask: for n=2, mask should have 2 bits set
    pred = vpredicate_8(2);
    uint32_t mask = vpredicate_8_get_mask(pred);
    if (mask != 0x03) {
        // (1 << 2) - 1 = 3
        return mp_const_false;
    }

    // vpredicate_16_get_mask: for n=2, mask depends on architecture
    pred = vpredicate_16(2);
    mask = vpredicate_16_get_mask(pred);
    // On ARMv7: (1 << (2*2)) - 1 = 15, on ARMv8: pred value
    if (mask == 0) {
        return mp_const_false;
    }

    // vpredicate_32_get_mask: for n=1
    pred = vpredicate_32(1);
    mask = vpredicate_32_get_mask(pred);
    // On ARMv7: (1 << (1*4)) - 1 = 15, on ARMv8: pred value
    if (mask == 0) {
        return mp_const_false;
    }

    // vpredicate_64_get_mask: for n=1 (if vector size >= 8)
    #if (VECTOR_SIZE_BYTES >= 8)
    pred = vpredicate_64(1);
    mask = vpredicate_64_get_mask(pred);
    if (mask == 0) {
        return mp_const_false;
    }
    #endif

    // Test vpredicate_32_add
    pred = vpredicate_32(0);
    pred = vpredicate_32_add(pred, 1);
    n = vpredicate_32_get_n(pred);
    if (n != 1) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_predicate_obj, test_simd_predicate);

// Test load/store operations
// Note: ARM MVE intrinsics require constant lane indices
static mp_obj_t test_simd_vldr_vstr(void) {
    uint8_t buf8[16] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00
    };
    uint16_t buf16[8] = {0x1122, 0x3344, 0x5566, 0x7788, 0x99AA, 0xBBCC, 0xDDEE, 0xFF00};

    // vldr_u8: load u8 vector - check first few elements
    v128_t v = vldr_u8(buf8);
    if (vget_u8(v, 0) != 0x11 || vget_u8(v, 1) != 0x22 ||
        vget_u8(v, 2) != 0x33 || vget_u8(v, 3) != 0x44) {
        return mp_const_false;
    }

    // vldr_u8_pred: load with predicate
    v128_predicate_t pred = vpredicate_8(2);
    v = vldr_u8_pred(buf8 + 4, pred);
    if (vget_u8(v, 0) != 0x55 || vget_u8(v, 1) != 0x66) {
        return mp_const_false;
    }

    // vldr_u16: load u16 vector - check first few elements
    v = vldr_u16(buf16);
    if (vget_u16(v, 0) != 0x1122 || vget_u16(v, 1) != 0x3344) {
        return mp_const_false;
    }

    // vldr_u16_pred: load with predicate
    pred = vpredicate_16(1);
    v = vldr_u16_pred(buf16 + 2, pred);
    if (vget_u16(v, 0) != 0x5566) {
        return mp_const_false;
    }

    // vstr_u8: store u8 vector - verify via memcmp
    uint8_t out8[16] = {0};
    v = vdup_u8(0xAB);
    vstr_u8(out8, v);
    // Check first few bytes were stored
    if (out8[0] != 0xAB || out8[1] != 0xAB || out8[2] != 0xAB || out8[3] != 0xAB) {
        return mp_const_false;
    }

    // vstr_u8_pred: store with predicate
    memset(out8, 0, sizeof(out8));
    v = vdup_u8(0xCD);
    pred = vpredicate_8(2);
    vstr_u8_pred(out8, v, pred);
    if (out8[0] != 0xCD || out8[1] != 0xCD || out8[2] != 0) {
        return mp_const_false;
    }

    // vstr_u16: store u16 vector - verify first few elements
    uint16_t out16[8] = {0};
    v = vdup_u16(0x1234);
    vstr_u16(out16, v);
    if (out16[0] != 0x1234 || out16[1] != 0x1234) {
        return mp_const_false;
    }

    // vstr_u16_pred: store with predicate
    memset(out16, 0, sizeof(out16));
    v = vdup_u16(0x5678);
    pred = vpredicate_16(1);
    vstr_u16_pred(out16, v, pred);
    if (out16[0] != 0x5678 || out16[1] != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vldr_vstr_obj, test_simd_vldr_vstr);

// Test interleaved load/store
static mp_obj_t test_simd_vld2_vst2(void) {
    // vld2_u8: load interleaved pairs
    // Input: [A0, B0, A1, B1, A2, B2, A3, B3, ...]
    // Output: r0=[A0,A1,A2,...], r1=[B0,B1,B2,...]
    // On ARMv8, vld2q loads 32 bytes (16 pairs), so buffer must be at least 32 bytes
    uint8_t buf8[32] = {
        0x10, 0x20, 0x11, 0x21, 0x12, 0x22, 0x13, 0x23,
        0x14, 0x24, 0x15, 0x25, 0x16, 0x26, 0x17, 0x27,
        0x18, 0x28, 0x19, 0x29, 0x1A, 0x2A, 0x1B, 0x2B,
        0x1C, 0x2C, 0x1D, 0x2D, 0x1E, 0x2E, 0x1F, 0x2F
    };

    v2x_rows_t rows = vld2_u8(buf8);
    // Check first 4 elements
    if (vget_u8(rows.r0, 0) != 0x10 || vget_u8(rows.r0, 1) != 0x11 ||
        vget_u8(rows.r0, 2) != 0x12 || vget_u8(rows.r0, 3) != 0x13) {
        return mp_const_false;
    }
    if (vget_u8(rows.r1, 0) != 0x20 || vget_u8(rows.r1, 1) != 0x21 ||
        vget_u8(rows.r1, 2) != 0x22 || vget_u8(rows.r1, 3) != 0x23) {
        return mp_const_false;
    }

    // vst2_u8: store interleaved pairs
    // On ARMv8, vst2q writes 32 bytes, so buffer must be at least 32 bytes
    uint8_t out8[32] = {0};
    rows.r0 = vdup_u8(0xAA);
    rows.r1 = vdup_u8(0xBB);
    rows.r0 = vset_u8(rows.r0, 0, 0xA0);
    rows.r0 = vset_u8(rows.r0, 1, 0xA1);
    rows.r1 = vset_u8(rows.r1, 0, 0xB0);
    rows.r1 = vset_u8(rows.r1, 1, 0xB1);
    vst2_u8(out8, rows);
    // Expected: [A0, B0, A1, B1, ...]
    if (out8[0] != 0xA0 || out8[1] != 0xB0 ||
        out8[2] != 0xA1 || out8[3] != 0xB1) {
        return mp_const_false;
    }

    // vld2_u16: load interleaved u16 pairs
    // On ARMv8, vld2q for u16 loads 32 bytes (8 pairs), so buffer must be at least 16 elements
    uint16_t buf16[16] = {
        0x1000, 0x2000, 0x1001, 0x2001,
        0x1002, 0x2002, 0x1003, 0x2003,
        0x1004, 0x2004, 0x1005, 0x2005,
        0x1006, 0x2006, 0x1007, 0x2007
    };

    rows = vld2_u16(buf16);
    if (vget_u16(rows.r0, 0) != 0x1000 || vget_u16(rows.r0, 1) != 0x1001) {
        return mp_const_false;
    }
    if (vget_u16(rows.r1, 0) != 0x2000 || vget_u16(rows.r1, 1) != 0x2001) {
        return mp_const_false;
    }

    // vst2_u16: store interleaved u16 pairs
    // On ARMv8, vst2q writes 32 bytes, so buffer must be at least 16 elements
    uint16_t out16[16] = {0};
    rows.r0 = vdup_u16(0xAAAA);
    rows.r1 = vdup_u16(0xBBBB);
    rows.r0 = vset_u16(rows.r0, 0, 0xA000);
    rows.r0 = vset_u16(rows.r0, 1, 0xA001);
    rows.r1 = vset_u16(rows.r1, 0, 0xB000);
    rows.r1 = vset_u16(rows.r1, 1, 0xB001);
    vst2_u16(out16, rows);
    if (out16[0] != 0xA000 || out16[1] != 0xB000 ||
        out16[2] != 0xA001 || out16[3] != 0xB001) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vld2_vst2_obj, test_simd_vld2_vst2);

// Test memory copy operations
static mp_obj_t test_simd_vmemcpy(void) {
    // vmemcpy_8: byte-aligned copy
    uint8_t src8[32] = {0};
    uint8_t dst8[32] = {0};
    for (int i = 0; i < 32; i++) {
        src8[i] = i;
    }
    vmemcpy_8(dst8, src8, 17);  // Copy 17 bytes
    for (int i = 0; i < 17; i++) {
        if (dst8[i] != src8[i]) {
            return mp_const_false;
        }
    }

    // vmemcpy_16: 2-byte aligned copy
    uint16_t src16[16] = {0};
    uint16_t dst16[16] = {0};
    for (int i = 0; i < 16; i++) {
        src16[i] = 0x1000 + i;
    }
    vmemcpy_16(dst16, src16, 20);  // Copy 20 bytes (10 u16)
    for (int i = 0; i < 10; i++) {
        if (dst16[i] != src16[i]) {
            return mp_const_false;
        }
    }

    // vmemcpy_32: 4-byte aligned copy
    uint32_t src32[8] = {0};
    uint32_t dst32[8] = {0};
    for (int i = 0; i < 8; i++) {
        src32[i] = 0xDEAD0000 + i;
    }
    vmemcpy_32(dst32, src32, 20);  // Copy 20 bytes (5 u32)
    for (int i = 0; i < 5; i++) {
        if (dst32[i] != src32[i]) {
            return mp_const_false;
        }
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vmemcpy_obj, test_simd_vmemcpy);

// Test RGB color conversion operations - HIGH PRIORITY
static mp_obj_t test_simd_rgb_convert(void) {
    // Test vrgb_rgb565_to_pixels888: convert RGB565 to separate R, G, B channels
    // RGB565 format: RRRRRGGGGGGBBBBB (5-6-5 bits)
    // Test with pure red: R=31, G=0, B=0 -> 0xF800
    v128_t rgb565 = vdup_u32(0);
    rgb565 = vset_u16(rgb565, 0, 0xF800);  // Pure red
    rgb565 = vset_u16(rgb565, 1, 0x07E0);  // Pure green

    vrgb_pixels_t pixels = vrgb_rgb565_to_pixels888(rgb565);

    // Red pixel: R should be ~248 (0xF8), G=0, B=0
    uint8_t r0 = vget_u8(pixels.r, 0);
    uint8_t g0 = vget_u8(pixels.g, 0);
    uint8_t b0 = vget_u8(pixels.b, 0);
    // R: 31 << 3 = 248, with extension bits
    if (r0 < 240 || g0 > 10 || b0 > 10) {
        return mp_const_false;
    }

    // Green pixel: R=0, G should be ~252 (0xFC), B=0
    uint8_t r1 = vget_u8(pixels.r, 2);
    uint8_t g1 = vget_u8(pixels.g, 2);
    uint8_t b1 = vget_u8(pixels.b, 2);
    if (r1 > 10 || g1 < 240 || b1 > 10) {
        return mp_const_false;
    }

    // Test vrgb_pixels_to_rgb565: convert R, G, B channels back to RGB565
    pixels.r = vdup_u32(0);
    pixels.g = vdup_u32(0);
    pixels.b = vdup_u32(0);

    // Set up pure red pixel (R=255, G=0, B=0)
    pixels.r = vset_u8(pixels.r, 0, 255);
    pixels.g = vset_u8(pixels.g, 0, 0);
    pixels.b = vset_u8(pixels.b, 0, 0);

    // Set up pure blue pixel (R=0, G=0, B=255)
    pixels.r = vset_u8(pixels.r, 2, 0);
    pixels.g = vset_u8(pixels.g, 2, 0);
    pixels.b = vset_u8(pixels.b, 2, 255);

    rgb565 = vrgb_pixels_to_rgb565(pixels);

    // Red pixel should be 0xF800 (or close to it)
    uint16_t red = vget_u16(rgb565, 0);
    if ((red & 0xF800) != 0xF800) {
        // Check red bits are set
        return mp_const_false;
    }

    // Blue pixel should be 0x001F (or close to it)
    uint16_t blue = vget_u16(rgb565, 1);
    if ((blue & 0x001F) != 0x001F) {
        // Check blue bits are set
        return mp_const_false;
    }

    // Test vrgb_pixels_to_grayscale: convert RGB to grayscale
    // Y = (R * 38 + G * 75 + B * 15) / 128
    pixels.r = vdup_u32(0);
    pixels.g = vdup_u32(0);
    pixels.b = vdup_u32(0);

    // Pure white (R=255, G=255, B=255) should give Y close to 255
    pixels.r = vset_u8(pixels.r, 0, 255);
    pixels.g = vset_u8(pixels.g, 0, 255);
    pixels.b = vset_u8(pixels.b, 0, 255);

    // Pure black (R=0, G=0, B=0) should give Y = 0
    pixels.r = vset_u8(pixels.r, 2, 0);
    pixels.g = vset_u8(pixels.g, 2, 0);
    pixels.b = vset_u8(pixels.b, 2, 0);

    v128_t gray = vrgb_pixels_to_grayscale(pixels);

    // White should give high grayscale value
    // (255*38 + 255*75 + 255*15) / 128 = 255 * 128 / 128 = 255
    uint8_t white_y = vget_u8(gray, 0);
    if (white_y < 200) {
        // Should be close to 255
        return mp_const_false;
    }

    // Black should give 0
    uint8_t black_y = vget_u8(gray, 2);
    if (black_y != 0) {
        return mp_const_false;
    }

    // Test round-trip conversion: RGB888 -> RGB565 -> RGB888
    // This tests both directions together
    pixels.r = vdup_u32(0);
    pixels.g = vdup_u32(0);
    pixels.b = vdup_u32(0);

    // Set a mid-range color
    pixels.r = vset_u8(pixels.r, 0, 128);
    pixels.g = vset_u8(pixels.g, 0, 64);
    pixels.b = vset_u8(pixels.b, 0, 192);

    // Convert to RGB565
    rgb565 = vrgb_pixels_to_rgb565(pixels);

    // Convert back to RGB888
    vrgb_pixels_t pixels2 = vrgb_rgb565_to_pixels888(rgb565);

    // Values should be close (within quantization error)
    r0 = vget_u8(pixels2.r, 0);
    g0 = vget_u8(pixels2.g, 0);
    b0 = vget_u8(pixels2.b, 0);

    // Allow for quantization error (RGB565 has 5-6-5 bits precision)
    if (abs((int) r0 - 128) > 8 || abs((int) g0 - 64) > 4 || abs((int) b0 - 192) > 8) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_rgb_convert_obj, test_simd_rgb_convert);

// Test 64-bit operations (only available when VECTOR_SIZE_BYTES >= 8)
static mp_obj_t test_simd_64bit(void) {
    #if (VECTOR_SIZE_BYTES >= 8)
    // Test vset_u64 and vget_u64
    v128_t v = vdup_u32(0);
    v = vset_u64(v, 0, 0x123456789ABCDEF0ULL);
    if (vget_u64(v, 0) != 0x123456789ABCDEF0ULL) {
        return mp_const_false;
    }

    // Test vset_s64 and vget_s64
    v = vdup_s32(0);
    v = vset_s64(v, 0, -123456789012345LL);
    if (vget_s64(v, 0) != -123456789012345LL) {
        return mp_const_false;
    }

    // Test vpredicate_64
    v128_predicate_t pred = vpredicate_64(1);
    uint32_t n = vpredicate_64_get_n(pred);
    if (n != 1) {
        return mp_const_false;
    }

    // Test vpredicate_64_all_lanes_active
    pred = vpredicate_64(UINT64_VECTOR_SIZE);
    if (!vpredicate_64_all_lanes_active(pred)) {
        return mp_const_false;
    }

    // Test vpredicate_64_add
    pred = vpredicate_64(0);
    pred = vpredicate_64_add(pred, 1);
    n = vpredicate_64_get_n(pred);
    if (n != 1) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_64bit_obj, test_simd_64bit);

// Test gather/scatter operations
static mp_obj_t test_simd_gather_scatter(void) {
    // Test vldr_u8_gather - load bytes at offset positions
    uint8_t buf8[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    v128_t offsets = vdup_u32(0);
    offsets = vset_u8(offsets, 0, 0);   // Load buf8[0]
    offsets = vset_u8(offsets, 1, 4);   // Load buf8[4]
    offsets = vset_u8(offsets, 2, 8);   // Load buf8[8]
    offsets = vset_u8(offsets, 3, 12);  // Load buf8[12]

    v128_t v = vldr_u8_gather(buf8, offsets);
    if (vget_u8(v, 0) != 0x00 || vget_u8(v, 1) != 0x44 ||
        vget_u8(v, 2) != 0x88 || vget_u8(v, 3) != 0xCC) {
        return mp_const_false;
    }

    // Test vldr_u8_widen_u32_gather_pred
    v128_t offsets32 = vdup_u32(2);  // Load buf8[2]
    v128_predicate_t pred = vpredicate_32(1);
    v = vldr_u8_widen_u32_gather_pred(buf8, offsets32, pred);
    if (vget_u32(v, 0) != 0x22) {
        return mp_const_false;
    }

    // Test vldr_s8_widen_s32_gather_pred
    int8_t sbuf8[8] = {-10, -20, -30, -40, 10, 20, 30, 40};
    offsets32 = vdup_u32(1);  // Load sbuf8[1]
    v = vldr_s8_widen_s32_gather_pred(sbuf8, offsets32, pred);
    if (vget_s32(v, 0) != -20) {
        return mp_const_false;
    }

    // Test vldr_u16_widen_u32_gather_pred
    uint16_t buf16[8] = {0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000};
    offsets32 = vdup_u32(3);  // Load buf16[3]
    v = vldr_u16_widen_u32_gather_pred(buf16, offsets32, pred);
    if (vget_u32(v, 0) != 0x4000) {
        return mp_const_false;
    }

    // Test vldr_s16_widen_s32_gather_pred
    int16_t sbuf16[8] = {-1000, -2000, -3000, -4000, 1000, 2000, 3000, 4000};
    offsets32 = vdup_u32(2);  // Load sbuf16[2]
    v = vldr_s16_widen_s32_gather_pred(sbuf16, offsets32, pred);
    if (vget_s32(v, 0) != -3000) {
        return mp_const_false;
    }

    // Test vldr_f32_gather_pred
    float32_t fbuf[4] = {1.5f, 2.5f, 3.5f, 4.5f};
    offsets32 = vdup_u32(1);  // Load fbuf[1]
    v = vldr_f32_gather_pred(fbuf, offsets32, pred);
    if (!FLOAT_EQ(v.f32[0], 2.5f, 0.001f)) {
        return mp_const_false;
    }

    // Test vstr_f32_scatter
    float32_t fout[8] = {0};
    // On ARMv8, scatter stores all 4 lanes using their respective offsets
    // Set different offsets for each lane and matching values
    offsets32 = vdup_u32(0);
    offsets32 = vset_u32(offsets32, 0, 1);  // Lane 0 -> fout[1]
    #if (VECTOR_SIZE_BYTES == 16)
    offsets32 = vset_u32(offsets32, 1, 3);  // Lane 1 -> fout[3]
    offsets32 = vset_u32(offsets32, 2, 5);  // Lane 2 -> fout[5]
    offsets32 = vset_u32(offsets32, 3, 7);  // Lane 3 -> fout[7]
    #endif
    v = vdup_u32(0);
    v.f32[0] = 99.5f;
    #if (VECTOR_SIZE_BYTES == 16)
    v.f32[1] = 88.5f;
    v.f32[2] = 77.5f;
    v.f32[3] = 66.5f;
    #endif
    vstr_f32_scatter(fout, offsets32, v);
    if (!FLOAT_EQ(fout[1], 99.5f, 0.001f)) {
        return mp_const_false;
    }
    #if (VECTOR_SIZE_BYTES == 16)
    if (!FLOAT_EQ(fout[3], 88.5f, 0.001f) || !FLOAT_EQ(fout[5], 77.5f, 0.001f)) {
        return mp_const_false;
    }
    #endif

    // Test vstr_u16_narrow_u8_scatter
    uint8_t out8[8] = {0};
    v128_t offsets16 = vdup_u32(0);
    offsets16 = vset_u16(offsets16, 0, 1);  // Store to out8[1]
    offsets16 = vset_u16(offsets16, 1, 3);  // Store to out8[3]
    v = vdup_u32(0);
    v = vset_u16(v, 0, 0x00AB);  // Low byte is 0xAB
    v = vset_u16(v, 1, 0x00CD);  // Low byte is 0xCD
    vstr_u16_narrow_u8_scatter(out8, offsets16, v);
    if (out8[1] != 0xAB || out8[3] != 0xCD) {
        return mp_const_false;
    }

    // Test vldr_u32_gather_unaligned
    // ARMv8 (VECTOR_SIZE_BYTES=16): gathers 16 bytes using offsets.u8[0..15]
    // ARMv7 (VECTOR_SIZE_BYTES=4): loads 1 u32 from p + offsets.u32[0]
    uint8_t ubuf[16] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00
    };
    #if (VECTOR_SIZE_BYTES == 16)
    // Set up byte offsets for gather: gather bytes 0,1,2,3,...,15
    v128_t gather_offsets = vidup_u8(0, 1);  // 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    v = vldr_u32_gather_unaligned(ubuf, gather_offsets);
    // Result should have ubuf[0..15] gathered into v.u8[0..15]
    if (vget_u8(v, 0) != 0x11 || vget_u8(v, 4) != 0x55 ||
        vget_u8(v, 8) != 0x99 || vget_u8(v, 15) != 0x00) {
        return mp_const_false;
    }
    #else
    offsets32 = vdup_u32(0);  // Load from offset 0
    v = vldr_u32_gather_unaligned(ubuf, offsets32);
    // Should load 4 bytes starting at ubuf[0]: 0x44332211 (little-endian)
    if (vget_u32(v, 0) != 0x44332211) {
        return mp_const_false;
    }
    #endif

    // Test vldr_u32_gather_pred_x4_unaligned
    uint8_t row0[16] = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    uint8_t row1[16] = {
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
    };
    uint8_t row2[16] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
    };
    uint8_t row3[16] = {
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F
    };

    v4x_row_ptrs_t rowptrs;
    rowptrs.p0.u8 = row0;
    rowptrs.p1.u8 = row1;
    rowptrs.p2.u8 = row2;
    rowptrs.p3.u8 = row3;

    #if (VECTOR_SIZE_BYTES == 16)
    v128_t goff = vidup_u8(0, 1);  // byte offsets 0..15
    pred = vpredicate_32(UINT32_VECTOR_SIZE);
    v4x_rows_t rows4 = vldr_u32_gather_pred_x4_unaligned(rowptrs, 0, goff, pred);
    // row0 bytes gathered: 0x10, 0x11, 0x12, ..., 0x1F
    if (vget_u8(rows4.r0, 0) != 0x10 || vget_u8(rows4.r0, 15) != 0x1F) {
        return mp_const_false;
    }
    #else
    offsets32 = vdup_u32(0);  // Offset 0 for gather
    // On ARMv7, predicate represents bytes to load: >3 for u32, >2 for u16, else u8
    pred = 4;  // Load 4 bytes (u32)
    v4x_rows_t rows4 = vldr_u32_gather_pred_x4_unaligned(rowptrs, 0, offsets32, pred);
    // Should load from each row starting at offset 0
    if (vget_u32(rows4.r0, 0) != 0x13121110) {
        return mp_const_false;
    }
    #endif

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_gather_scatter_obj, test_simd_gather_scatter);

// Test pack with rotate variants
static mp_obj_t test_simd_pack_rotate(void) {
    // vpkhbt_ror8: pack with rotate by 8
    v128_t v0 = vdup_u32(0);
    v0 = vset_u16(v0, 0, 0xAABB);
    v0 = vset_u16(v0, 1, 0xCCDD);

    v128_t v1 = vdup_u32(0);
    v1 = vset_u32(v1, 0, 0x11223344);

    v128_t r = vpkhbt_ror8(v0, v1);
    // Lower 16 bits from v0[0] = 0xAABB
    // Upper 16 bits from v1[0] >> 8 = 0x00112233 >> 8 = 0x1122
    if (vget_u16(r, 0) != 0xAABB) {
        return mp_const_false;
    }

    // vpkhtb_ror8: pack top-bottom with rotate by 8
    v0 = vdup_s32(0);
    v0 = vset_s16(v0, 0, 0x1111);
    v0 = vset_s16(v0, 1, 0x2222);

    v1 = vdup_s32(0);
    v1 = vset_s32(v1, 0, 0x33445566);

    r = vpkhtb_ror8(v0, v1);
    // Lower 16 bits from v1[0] >> 8 = 0x334455
    // Upper 16 bits from v0[1] = 0x2222
    if (vget_s16(r, 1) != 0x2222) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_pack_rotate_obj, test_simd_pack_rotate);

// Test length-limited interleaved load/store
static mp_obj_t test_simd_vld2_vst2_len(void) {
    // vld2_u8_len: load only specified number of bytes
    uint8_t buf8[8] = {0xA0, 0xB0, 0xA1, 0xB1, 0xA2, 0xB2, 0xA3, 0xB3};

    v2x_rows_t rows = vld2_u8_len(buf8, 4);  // Only load 4 bytes (2 pairs)
    if (vget_u8(rows.r0, 0) != 0xA0 || vget_u8(rows.r0, 1) != 0xA1) {
        return mp_const_false;
    }
    if (vget_u8(rows.r1, 0) != 0xB0 || vget_u8(rows.r1, 1) != 0xB1) {
        return mp_const_false;
    }

    // vst2_u16_len: store only specified number of elements
    uint16_t out16[8] = {0};
    rows.r0 = vdup_u16(0x1111);
    rows.r1 = vdup_u16(0x2222);
    rows.r0 = vset_u16(rows.r0, 0, 0xA000);
    rows.r0 = vset_u16(rows.r0, 1, 0xA001);
    rows.r1 = vset_u16(rows.r1, 0, 0xB000);
    rows.r1 = vset_u16(rows.r1, 1, 0xB001);
    vst2_u16_len(out16, rows, 4);  // Store only 4 elements (2 pairs)
    if (out16[0] != 0xA000 || out16[1] != 0xB000 ||
        out16[2] != 0xA001 || out16[3] != 0xB001) {
        return mp_const_false;
    }
    // Element 4 should be untouched
    if (out16[4] != 0) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_vld2_vst2_len_obj, test_simd_vld2_vst2_len);

// Test RGB pixel store operations
static mp_obj_t test_simd_rgb_store(void) {
    // Test vrgb_pixels_store_grayscale
    uint8_t gray_out[16] = {0};
    vrgb_pixels_t pixels;
    pixels.r = vdup_u32(0);
    pixels.g = vdup_u32(0);
    pixels.b = vdup_u32(0);

    // White pixel (R=255, G=255, B=255) -> grayscale ~255
    pixels.r = vset_u8(pixels.r, 0, 255);
    pixels.g = vset_u8(pixels.g, 0, 255);
    pixels.b = vset_u8(pixels.b, 0, 255);

    v128_predicate_t pred = vpredicate_16(1);
    vrgb_pixels_store_grayscale(gray_out, 0, pixels, pred);
    // Should store grayscale value at position 0
    if (gray_out[0] < 200) {
        // Should be close to 255
        return mp_const_false;
    }

    // Test vrgb_pixels_store_rgb565
    uint16_t rgb565_out[8] = {0};
    pixels.r = vdup_u32(0);
    pixels.g = vdup_u32(0);
    pixels.b = vdup_u32(0);

    // Pure red pixel
    pixels.r = vset_u8(pixels.r, 0, 255);
    pixels.g = vset_u8(pixels.g, 0, 0);
    pixels.b = vset_u8(pixels.b, 0, 0);

    pred = vpredicate_16(1);
    vrgb_pixels_store_rgb565(rgb565_out, 0, pixels, pred);
    // Should have red bits set (0xF800)
    if ((rgb565_out[0] & 0xF800) != 0xF800) {
        return mp_const_false;
    }

    // Test vrgb_pixels_store_binary
    uint32_t binary_out[2] = {0};
    pixels.r = vdup_u32(0);
    pixels.g = vdup_u32(0);
    pixels.b = vdup_u32(0);

    // White pixel -> binary 1
    pixels.r = vset_u8(pixels.r, 0, 255);
    pixels.g = vset_u8(pixels.g, 0, 255);
    pixels.b = vset_u8(pixels.b, 0, 255);

    pred = vpredicate_16(1);
    vrgb_pixels_store_binary(binary_out, 0, pixels, pred);
    // Should have bit 0 set
    if ((binary_out[0] & 1) != 1) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_rgb_store_obj, test_simd_rgb_store);

// Test miscellaneous remaining functions
static mp_obj_t test_simd_misc(void) {
    // Test vmul_n_f32
    v128_t v;
    v.f32[0] = 2.5f;
    v128_t r = vmul_n_f32(v, 4.0f);
    if (!FLOAT_EQ(r.f32[0], 10.0f, 0.001f)) {
        return mp_const_false;
    }

    // Test vmemcpy_64 (only available when VECTOR_SIZE_BYTES >= 8)
    #if (VECTOR_SIZE_BYTES >= 8)
    uint64_t src64[4] = {
        0x1122334455667788ULL, 0xAABBCCDDEEFF0011ULL,
        0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL
    };
    uint64_t dst64[4] = {0};
    vmemcpy_64(dst64, src64, 16);  // Copy 16 bytes (2 uint64)
    if (dst64[0] != 0x1122334455667788ULL || dst64[1] != 0xAABBCCDDEEFF0011ULL) {
        return mp_const_false;
    }
    #endif

    // Test vidup_u32_unaligned
    // This function creates byte offsets for gather operations
    // ARMv8: Creates complex pattern for vldrbq_gather_offset
    // ARMv7: Just returns {start} in lane 0
    v = vidup_u32_unaligned(100, 4);
    #if (VECTOR_SIZE_BYTES == 16)
    // On ARMv8, check that the first byte offset starts at 100
    if (vget_u8(v, 0) != 100) {
        return mp_const_false;
    }
    #else
    // On ARMv7, just returns {start}
    if (vget_u32(v, 0) != 100) {
        return mp_const_false;
    }
    #endif

    // Test vsli_u32
    v128_t v0 = vdup_u32(0x000000FF);
    v128_t v1 = vdup_u32(0x000000AB);
    r = vsli_u32(v0, v1, 16);
    // v1 << 16 = 0x00AB0000, keep lower 16 bits of v0 = 0x000000FF
    // Result: 0x00AB00FF
    if (vget_u32(r, 0) != 0x00AB00FF) {
        return mp_const_false;
    }

    // Test vsri_u32
    v0 = vdup_u32(0xFF000000);
    v1 = vdup_u32(0xAB000000);
    r = vsri_u32(v0, v1, 16);
    // v1 >> 16 = 0x0000AB00, keep upper 16 bits of v0 = 0xFF000000
    // Result: 0xFF00AB00
    if (vget_u32(r, 0) != 0xFF00AB00) {
        return mp_const_false;
    }

    // Test vstr_u16_narrow_u8_pred
    uint8_t out8[8] = {0};
    v = vdup_u32(0);
    v = vset_u16(v, 0, 0x00EE);  // Low byte = 0xEE
    v = vset_u16(v, 1, 0x00FF);  // Low byte = 0xFF
    v128_predicate_t pred = vpredicate_16(2);
    vstr_u16_narrow_u8_pred(out8, v, pred);
    if (out8[0] != 0xEE || out8[1] != 0xFF) {
        return mp_const_false;
    }

    // Test vldr_u8_widen_u16_pred
    uint8_t buf8[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    pred = vpredicate_16(2);
    v = vldr_u8_widen_u16_pred(buf8, pred);
    if (vget_u16(v, 0) != 0x0011 || vget_u16(v, 1) != 0x0022) {
        return mp_const_false;
    }

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_simd_misc_obj, test_simd_misc);

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

    // SIMD tests
    { MP_ROM_QSTR(MP_QSTR_test_simd_vdup), MP_ROM_PTR(&test_simd_vdup_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vidup), MP_ROM_PTR(&test_simd_vidup_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vget_vset), MP_ROM_PTR(&test_simd_vget_vset_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vadd), MP_ROM_PTR(&test_simd_vadd_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vsub), MP_ROM_PTR(&test_simd_vsub_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vmul), MP_ROM_PTR(&test_simd_vmul_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vmla), MP_ROM_PTR(&test_simd_vmla_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vand_vorr_veor), MP_ROM_PTR(&test_simd_vand_vorr_veor_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vshift_left), MP_ROM_PTR(&test_simd_vshift_left_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vshift_right), MP_ROM_PTR(&test_simd_vshift_right_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vsli_vsri), MP_ROM_PTR(&test_simd_vsli_vsri_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vshl), MP_ROM_PTR(&test_simd_vshl_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vhadd), MP_ROM_PTR(&test_simd_vhadd_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vpkhbt_vpkhtb), MP_ROM_PTR(&test_simd_vpkhbt_vpkhtb_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vuxtb_vsxtb), MP_ROM_PTR(&test_simd_vuxtb_vsxtb_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vmov_narrow), MP_ROM_PTR(&test_simd_vmov_narrow_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vusat_narrow), MP_ROM_PTR(&test_simd_vusat_narrow_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vmladav), MP_ROM_PTR(&test_simd_vmladav_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vcvt), MP_ROM_PTR(&test_simd_vcvt_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vminmax), MP_ROM_PTR(&test_simd_vminmax_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_predicate), MP_ROM_PTR(&test_simd_predicate_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vldr_vstr), MP_ROM_PTR(&test_simd_vldr_vstr_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vld2_vst2), MP_ROM_PTR(&test_simd_vld2_vst2_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vmemcpy), MP_ROM_PTR(&test_simd_vmemcpy_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_rgb_convert), MP_ROM_PTR(&test_simd_rgb_convert_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_64bit), MP_ROM_PTR(&test_simd_64bit_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_gather_scatter), MP_ROM_PTR(&test_simd_gather_scatter_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_pack_rotate), MP_ROM_PTR(&test_simd_pack_rotate_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_vld2_vst2_len), MP_ROM_PTR(&test_simd_vld2_vst2_len_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_rgb_store), MP_ROM_PTR(&test_simd_rgb_store_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_simd_misc), MP_ROM_PTR(&test_simd_misc_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_module_globals, unittest_module_globals_table);

const mp_obj_module_t unittest_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest, unittest_module);

#endif // MICROPY_PY_UNITTEST
