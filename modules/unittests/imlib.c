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
#include "umalloc.h"
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

static mp_obj_t test_imlib_grayscale_bindings(void) {
    static const mp_int_t values[] = {0, 127, 128, 255, -1, 256, -129, 383};
    mp_obj_t module = mp_import_name(MP_QSTR_image, mp_const_none, MP_OBJ_NEW_SMALL_INT(0));
    mp_obj_t binary = mp_load_attr(module, MP_QSTR_grayscale_to_binary);
    mp_obj_t converters[] = {
        mp_load_attr(module, MP_QSTR_grayscale_to_rgb),
        mp_load_attr(module, MP_QSTR_grayscale_to_lab),
        mp_load_attr(module, MP_QSTR_grayscale_to_yuv),
    };
    for (size_t i = 0; i < MP_ARRAY_SIZE(values); i++) {
        mp_obj_t arg = mp_obj_new_int(values[i]);
        uint8_t gray = values[i] & 255;
        if (mp_obj_get_int(mp_call_function_1(binary, arg)) != (gray > 127)) {
            return mp_const_false;
        }
        uint16_t rgb565 = COLOR_GRAYSCALE_TO_RGB565(gray);
        int expected[][3] = {
            {COLOR_RGB565_TO_R8(rgb565), COLOR_RGB565_TO_G8(rgb565), COLOR_RGB565_TO_B8(rgb565)},
            {COLOR_RGB565_TO_L(rgb565), COLOR_RGB565_TO_A(rgb565), COLOR_RGB565_TO_B(rgb565)},
            {COLOR_RGB565_TO_Y(rgb565), COLOR_RGB565_TO_U(rgb565), COLOR_RGB565_TO_V(rgb565)},
        };
        for (size_t j = 0; j < MP_ARRAY_SIZE(converters); j++) {
            mp_obj_t result = mp_call_function_1(converters[j], arg);
            mp_obj_t *items;
            mp_obj_get_array_fixed_n(result, 3, &items);
            for (size_t k = 0; k < 3; k++) {
                if (mp_obj_get_int(items[k]) != expected[j][k]) {
                    return mp_const_false;
                }
            }
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_grayscale_bindings_obj, test_imlib_grayscale_bindings);

static mp_obj_t test_imlib_binary_min_bounds(void) {
    // Padding makes an incorrect grayscale fallthrough visible without an out-of-bounds access.
    uint32_t source[16] = {0x0F0FF0F0, 0xAAAAAAAA};
    uint32_t mask_pixels[2] = {UINT32_MAX, UINT32_MAX};
    image_t mask = {.w = 64, .h = 1, .pixfmt = PIXFORMAT_BINARY, .data = (uint8_t *) mask_pixels};
    for (int masked = 0; masked <= 1; masked++) {
        uint32_t pixels[16];
        pixels[0] = pixels[1] = UINT32_MAX;
        for (size_t i = 2; i < MP_ARRAY_SIZE(pixels); i++) {
            pixels[i] = 0xDEADBEEF;
        }
        image_t img = {.w = 64, .h = 1, .pixfmt = PIXFORMAT_BINARY, .data = (uint8_t *) pixels};
        imlib_draw_row_data_t row = {
            .dst_img = &img,
            .callback_arg = masked ? &mask : NULL,
            .dst_row_override = source,
        };
        imlib_min_line_op(0, 64, 0, &row);
        if (pixels[0] != source[0] || pixels[1] != source[1]) {
            return mp_const_false;
        }
        for (size_t i = 2; i < MP_ARRAY_SIZE(pixels); i++) {
            if (pixels[i] != 0xDEADBEEF) {
                return mp_const_false;
            }
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_binary_min_bounds_obj, test_imlib_binary_min_bounds);

#if OMV_OS_UNIX
static mp_obj_t test_imlib_clahe_byte_range(void) {
    uint8_t pixels[] = {0, 127, 128, 255};
    image_t img = {.w = 2, .h = 2, .pixfmt = PIXFORMAT_GRAYSCALE, .data = pixels};
    imlib_clahe_histeq(&img, 1.0f, NULL);
    return (pixels[0] == 0 && pixels[1] == 127 && pixels[2] == 128 && pixels[3] == 255)
           ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_clahe_byte_range_obj, test_imlib_clahe_byte_range);
#endif

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

// Test sepconv3: impulse response. All pixels processed including borders.
static mp_obj_t test_imlib_sepconv3_impulse(void) {
    image_t img = { .w = 5, .h = 5, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc0(&img, image_size(&img));
    img.data[2 * 5 + 2] = 255;

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    // Check symmetric Gaussian spread from center.
    int pass = (img.data[2 * 5 + 2] == 63)    // center
               && (img.data[1 * 5 + 2] == 31) // above
               && (img.data[3 * 5 + 2] == 31) // below
               && (img.data[2 * 5 + 1] == 31) // left
               && (img.data[2 * 5 + 3] == 31) // right
               && (img.data[1 * 5 + 1] == 15) // diagonal
               && (img.data[0 * 5 + 0] == 0); // far corner
    return pass ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_impulse_obj, test_imlib_sepconv3_impulse);

// Test sepconv3: uniform 128 -> every pixel stays 128.
static mp_obj_t test_imlib_sepconv3_uniform(void) {
    image_t img = { .w = 8, .h = 8, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    memset(img.data, 128, image_size(&img));

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    for (size_t i = 0; i < image_size(&img); i++) {
        if (img.data[i] != 128) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_uniform_obj, test_imlib_sepconv3_uniform);

// Test sepconv3: all 255 stays 255 (no overflow, all pixels).
static mp_obj_t test_imlib_sepconv3_saturate(void) {
    image_t img = { .w = 6, .h = 6, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    memset(img.data, 255, image_size(&img));

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    for (size_t i = 0; i < image_size(&img); i++) {
        if (img.data[i] != 255) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_saturate_obj, test_imlib_sepconv3_saturate);

// Test sepconv3: 3x3 uniform -> still works (border clamping).
static mp_obj_t test_imlib_sepconv3_min_size(void) {
    image_t img = { .w = 3, .h = 3, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    memset(img.data, 100, image_size(&img));

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    // Uniform input -> every pixel should stay 100.
    for (size_t i = 0; i < image_size(&img); i++) {
        if (img.data[i] != 100) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_min_size_obj, test_imlib_sepconv3_min_size);

// Test sepconv3: wide step edge to exercise SIMD path.
// Checks x positions at vector boundaries (4, 16, 32).
static mp_obj_t test_imlib_sepconv3_wide(void) {
    int w = 64, h = 8;
    image_t img = { .w = w, .h = h, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    for (int y = 0; y < h; y++) {
        memset(img.data + y * w, (y >= 4) ? 255 : 0, w);
    }

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    // Vert on y=3: rows{2,3,4}={0,0,255} -> v=255. Horiz uniform: 4*255/16=63.
    // Vert on y=4: rows{3,4,5}={0,255,255} -> v=765. Horiz: 4*765/16=191.
    int check[] = {0, 1, 3, 4, 15, 16, 31, 32, 62, 63};
    for (int i = 0; i < 10; i++) {
        int x = check[i];
        if (img.data[3 * w + x] != 63 || img.data[4 * w + x] != 191) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_wide_obj, test_imlib_sepconv3_wide);

// Test sepconv3: 1x1 image -- must not crash.
static mp_obj_t test_imlib_sepconv3_1x1(void) {
    image_t img = { .w = 1, .h = 1, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    img.data[0] = 200;

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    // 1x1 uniform: border clamp makes all neighbors equal -> identity.
    return (img.data[0] == 200) ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_1x1_obj, test_imlib_sepconv3_1x1);

// Test sepconv3: 2x2 image -- smaller than kernel.
static mp_obj_t test_imlib_sepconv3_2x2(void) {
    image_t img = { .w = 2, .h = 2, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    memset(img.data, 50, image_size(&img));

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    for (size_t i = 0; i < image_size(&img); i++) {
        if (img.data[i] != 50) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_2x2_obj, test_imlib_sepconv3_2x2);

// Test sepconv3: odd widths not a multiple of SIMD vector size (5, 7, 13).
// Uniform input so we can check all pixels easily.
static mp_obj_t test_imlib_sepconv3_odd_sizes(void) {
    int sizes[] = {5, 7, 13};
    for (int s = 0; s < 3; s++) {
        int w = sizes[s], h = sizes[s];
        image_t img = { .w = w, .h = h, .pixfmt = PIXFORMAT_GRAYSCALE };
        image_alloc(&img, image_size(&img));
        memset(img.data, 77, image_size(&img));

        imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

        for (int i = 0; i < w * h; i++) {
            if (img.data[i] != 77) {
                return mp_const_false;
            }
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_odd_sizes_obj, test_imlib_sepconv3_odd_sizes);

// Test sepconv3: tall narrow image (2xN) -- width < SIMD vector size,
// only scalar path runs for horizontal, vertical has many rows.
static mp_obj_t test_imlib_sepconv3_tall_narrow(void) {
    image_t img = { .w = 2, .h = 32, .pixfmt = PIXFORMAT_GRAYSCALE };
    image_alloc(&img, image_size(&img));
    memset(img.data, 180, image_size(&img));

    imlib_sepconv3(&img, kernel_gauss_3, 1.0f / 16.0f, 0);

    for (size_t i = 0; i < image_size(&img); i++) {
        if (img.data[i] != 180) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_imlib_sepconv3_tall_narrow_obj, test_imlib_sepconv3_tall_narrow);

// Module definition
static const mp_rom_map_elem_t unittest_imlib_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_imlib) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_ksize_to_n), MP_ROM_PTR(&test_imlib_ksize_to_n_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_color_conv), MP_ROM_PTR(&test_imlib_color_conv_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_grayscale_bindings), MP_ROM_PTR(&test_imlib_grayscale_bindings_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_binary_min_bounds), MP_ROM_PTR(&test_imlib_binary_min_bounds_obj) },
    #if OMV_OS_UNIX
    { MP_ROM_QSTR(MP_QSTR_test_imlib_clahe_byte_range), MP_ROM_PTR(&test_imlib_clahe_byte_range_obj) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_test_imlib_rectangle), MP_ROM_PTR(&test_imlib_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_rectangle_united), MP_ROM_PTR(&test_imlib_rectangle_united_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_point), MP_ROM_PTR(&test_imlib_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_point_equal), MP_ROM_PTR(&test_imlib_point_equal_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sincos_table), MP_ROM_PTR(&test_imlib_sincos_table_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_impulse), MP_ROM_PTR(&test_imlib_sepconv3_impulse_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_uniform), MP_ROM_PTR(&test_imlib_sepconv3_uniform_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_saturate), MP_ROM_PTR(&test_imlib_sepconv3_saturate_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_min_size), MP_ROM_PTR(&test_imlib_sepconv3_min_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_wide), MP_ROM_PTR(&test_imlib_sepconv3_wide_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_1x1), MP_ROM_PTR(&test_imlib_sepconv3_1x1_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_odd_sizes), MP_ROM_PTR(&test_imlib_sepconv3_odd_sizes_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_2x2), MP_ROM_PTR(&test_imlib_sepconv3_2x2_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_imlib_sepconv3_tall_narrow), MP_ROM_PTR(&test_imlib_sepconv3_tall_narrow_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_imlib_module_globals, unittest_imlib_module_globals_table);

const mp_obj_module_t unittest_imlib_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_imlib_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_imlib, unittest_imlib_module);

#endif // MICROPY_PY_UNITTEST
