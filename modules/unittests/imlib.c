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
 * Image library unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include <stdlib.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "fmath.h"
#include "imlib.h"
#include "unittest.h"

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

// Module definition
static const mp_rom_map_elem_t unittest_imlib_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_imlib) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_ksize_to_n), MP_ROM_PTR(&test_imlib_ksize_to_n_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_color_conv), MP_ROM_PTR(&test_imlib_color_conv_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_rectangle), MP_ROM_PTR(&test_imlib_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_rectangle_united), MP_ROM_PTR(&test_imlib_rectangle_united_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_point), MP_ROM_PTR(&test_imlib_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_point_equal), MP_ROM_PTR(&test_imlib_point_equal_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sincos_table), MP_ROM_PTR(&test_imlib_sincos_table_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_imlib_module_globals, unittest_imlib_module_globals_table);

const mp_obj_module_t unittest_imlib_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_imlib_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_imlib, unittest_imlib_module);

#endif // MICROPY_PY_UNITTEST
