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
 * Fast math unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST

#include <math.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "fmath.h"
#include "unittest.h"

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

// Module definition
static const mp_rom_map_elem_t unittest_fmath_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_fmath) },
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
};

static MP_DEFINE_CONST_DICT(unittest_fmath_module_globals, unittest_fmath_module_globals_table);

const mp_obj_module_t unittest_fmath_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_fmath_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_fmath, unittest_fmath_module);

#endif // MICROPY_PY_UNITTEST
