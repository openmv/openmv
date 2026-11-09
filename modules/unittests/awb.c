/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * AWB stats unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST && MICROPY_PY_CSI

#include <math.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "omv_csi.h"
#include "unittest.h"

static bool awb_equal(const float a[3], const float b[3], float tolerance) {
    for (int i = 0; i < 3; i++) {
        if (fabsf(a[i] - b[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

// Without clipping the means are unchanged.
static mp_obj_t test_awb_unclip_none(void) {
    const float mean[3] = { 40.0f, 80.0f, 30.0f };
    const float clipped[3] = { 0.0f, 0.0f, 0.0f };
    float out[3];

    if (!omv_csi_stats_unclip(mean, clipped, out) || !awb_equal(out, mean, 1e-4f)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_none_obj, test_awb_unclip_none);

// Pixels clipped in all channels are removed, which recovers the means of the other pixels.
static mp_obj_t test_awb_unclip_all(void) {
    const float unclipped[3] = { 40.0f, 80.0f, 30.0f };
    const float clipped[3] = { 0.2f, 0.2f, 0.2f };
    float mean[3];
    float out[3];

    for (int i = 0; i < 3; i++) {
        mean[i] = unclipped[i] * 0.8f + 255.0f * 0.2f;
    }

    if (!omv_csi_stats_unclip(mean, clipped, out) || !awb_equal(out, unclipped, 1e-3f)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_all_obj, test_awb_unclip_all);

// Most pixels clipped in all channels is rejected.
static mp_obj_t test_awb_unclip_mostly(void) {
    const float mean[3] = { 200.0f, 220.0f, 200.0f };
    const float clipped[3] = { 0.5f, 0.7f, 0.5f };
    float out[3];

    if (omv_csi_stats_unclip(mean, clipped, out)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_mostly_obj, test_awb_unclip_mostly);

// Green clipped 4x as often as red at twice the brightness is a tail of a = 2, whose band mean is 4/3.
static mp_obj_t test_awb_unclip_tail(void) {
    const float mean[3] = { 61.5f, 97.5f, 61.5f };
    const float clipped[3] = { 0.1f, 0.4f, 0.1f };
    const float expected[3] = { 40.0f, 80.0f + 0.3f / 3.0f * 255.0f / 0.9f, 40.0f };
    float out[3];

    if (!omv_csi_stats_unclip(mean, clipped, out) || !awb_equal(out, expected, 1e-3f)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_tail_obj, test_awb_unclip_tail);

// Green clipped 2x as often as red at twice the brightness is a tail of a = 1, whose band mean is ln(2) / 0.5.
static mp_obj_t test_awb_unclip_tail_one(void) {
    const float mean[3] = { 61.5f, 97.5f, 61.5f };
    const float clipped[3] = { 0.1f, 0.2f, 0.1f };
    const float expected[3] = { 40.0f, 80.0f + 0.1f * (2.0f * logf(2.0f) - 1.0f) * 255.0f / 0.9f, 40.0f };
    float out[3];

    if (!omv_csi_stats_unclip(mean, clipped, out) || !awb_equal(out, expected, 1e-3f)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_tail_one_obj, test_awb_unclip_tail_one);

// Clipping in two channels raises both of them, and never lowers the least clipped channel.
static mp_obj_t test_awb_unclip_two(void) {
    const float mean[3] = { 70.0f, 110.0f, 50.0f };
    const float clipped[3] = { 0.15f, 0.3f, 0.05f };
    float out[3];

    if (!omv_csi_stats_unclip(mean, clipped, out)) {
        return mp_const_false;
    }

    float base[3];
    for (int i = 0; i < 3; i++) {
        base[i] = (mean[i] - 0.05f * 255.0f) / 0.95f;
    }

    if (out[0] <= base[0] || out[1] <= base[1] || fabsf(out[2] - base[2]) > 1e-3f) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_two_obj, test_awb_unclip_two);

// Stats measured on the N6 + PAG7936 land near the white balance measured with nothing clipped.
static mp_obj_t test_awb_unclip_measured(void) {
    // Bright windows (unclipped r/g 0.643, b/g 0.536) and a dim lamp lit scene (r/g 0.97, b/g 0.40).
    const float mean[2][3] = { { 48.9f, 67.2f, 41.4f }, { 53.1f, 54.8f, 23.2f } };
    const float clipped[2][3] = { { 0.036f, 0.114f, 0.023f }, { 0.035f, 0.041f, 0.0f } };
    const float expected[2][2] = { { 0.643f, 0.536f }, { 0.97f, 0.40f } };

    for (int i = 0; i < 2; i++) {
        float out[3];

        if (!omv_csi_stats_unclip(mean[i], clipped[i], out)) {
            return mp_const_false;
        }

        // Within 6% of the reference, which is about the measurement's precision.
        if (fabsf(out[0] / out[1] / expected[i][0] - 1.0f) > 0.06f ||
            fabsf(out[2] / out[1] / expected[i][1] - 1.0f) > 0.06f) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_awb_unclip_measured_obj, test_awb_unclip_measured);

// Module definition
static const mp_rom_map_elem_t unittest_awb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_awb) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_none), MP_ROM_PTR(&test_awb_unclip_none_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_all), MP_ROM_PTR(&test_awb_unclip_all_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_mostly), MP_ROM_PTR(&test_awb_unclip_mostly_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_tail), MP_ROM_PTR(&test_awb_unclip_tail_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_tail_one), MP_ROM_PTR(&test_awb_unclip_tail_one_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_two), MP_ROM_PTR(&test_awb_unclip_two_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_awb_unclip_measured), MP_ROM_PTR(&test_awb_unclip_measured_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_awb_module_globals, unittest_awb_module_globals_table);

const mp_obj_module_t unittest_awb_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_awb_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_awb, unittest_awb_module);

#endif // MICROPY_PY_UNITTEST && MICROPY_PY_CSI
