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
 * Color correction matrix unit tests.
 */
#include "imlib_config.h"

#if MICROPY_PY_UNITTEST && MICROPY_PY_CSI

#include <math.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "fmath.h"
#include "omv_csi.h"
#include "unittest.h"

// Raw R/G/B averages of the calibration illuminants, warmest to coolest.
static const float ccm_rgb[3][3] = {
    { 100, 100, 50 },
    { 80, 100, 80 },
    { 50, 100, 100 },
};

static const float ccm_chroma[3][2] = {
    OMV_CSI_CCM_CHROMA(100, 100, 50),
    OMV_CSI_CCM_CHROMA(80, 100, 80),
    OMV_CSI_CCM_CHROMA(50, 100, 100),
};

static omv_csi_ccm_t ccm_entries[3];

static const omv_csi_ccm_table_t *ccm_table(size_t count) {
    static omv_csi_ccm_table_t table;

    for (int i = 0; i < 3; i++) {
        omv_csi_ccm_t *e = &ccm_entries[i];
        e->chroma[0] = ccm_chroma[i][0];
        e->chroma[1] = ccm_chroma[i][1];

        // Each entry has a different saturation, with rows that sum to 1.
        float s = 1.5f + i * 0.5f;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                e->coeffs[r][c] = r == c ? s : (1.0f - s) / 2.0f;
            }
            e->offsets[r] = i * 4.0f;
        }
    }

    table.count = count;
    table.entries = ccm_entries;
    return &table;
}

static bool ccm_equal(const omv_csi_ccm_t *out, const omv_csi_ccm_t *a, const omv_csi_ccm_t *b, float t) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (!FLOAT_EQ(out->coeffs[r][c], a->coeffs[r][c] + t * (b->coeffs[r][c] - a->coeffs[r][c]), 0.001f)) {
                return false;
            }
        }
        if (!FLOAT_EQ(out->offsets[r], a->offsets[r] + t * (b->offsets[r] - a->offsets[r]), 0.001f)) {
            return false;
        }
    }
    return true;
}

static bool ccm_is_unity(const omv_csi_ccm_t *out) {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (!FLOAT_EQ(out->coeffs[r][c], r == c ? 1.0f : 0.0f, 0.001f)) {
                return false;
            }
        }
        if (!FLOAT_EQ(out->offsets[r], 0.0f, 0.001f)) {
            return false;
        }
    }
    return true;
}

// The compile-time chroma matches logf() at runtime.
static mp_obj_t test_ccm_chroma(void) {
    for (int i = 0; i < 3; i++) {
        if (!FLOAT_EQ(ccm_chroma[i][0], logf(ccm_rgb[i][0] / ccm_rgb[i][1]), 0.0001f) ||
            !FLOAT_EQ(ccm_chroma[i][1], logf(ccm_rgb[i][2] / ccm_rgb[i][1]), 0.0001f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_chroma_obj, test_ccm_chroma);

// A missing or empty table leaves the CCM unchanged.
static mp_obj_t test_ccm_empty(void) {
    omv_csi_ccm_t out;
    const float rgb[3] = { 80.0f, 100.0f, 80.0f };

    if (omv_csi_ccm_compute(NULL, rgb, 100.0f, 0.0f, &out) ||
        omv_csi_ccm_compute(ccm_table(0), rgb, 100.0f, 0.0f, &out)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_empty_obj, test_ccm_empty);

// Each calibration illuminant gets its own matrix.
static mp_obj_t test_ccm_calibration_points(void) {
    const omv_csi_ccm_table_t *table = ccm_table(3);

    for (int i = 0; i < 3; i++) {
        omv_csi_ccm_t out;
        if (!omv_csi_ccm_compute(table, ccm_rgb[i], 100.0f, 0.0f, &out) ||
            !ccm_equal(&out, &ccm_entries[i], &ccm_entries[i], 0.0f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_calibration_points_obj, test_ccm_calibration_points);

// Illuminants past either end of the polyline clamp to the end matrices.
static mp_obj_t test_ccm_clamp_ends(void) {
    const omv_csi_ccm_table_t *table = ccm_table(3);
    const float warm[3] = { 200.0f, 100.0f, 10.0f };
    const float cool[3] = { 10.0f, 100.0f, 200.0f };
    omv_csi_ccm_t out;

    if (!omv_csi_ccm_compute(table, warm, 100.0f, 0.0f, &out) ||
        !ccm_equal(&out, &ccm_entries[0], &ccm_entries[0], 0.0f)) {
        return mp_const_false;
    }
    if (!omv_csi_ccm_compute(table, cool, 100.0f, 0.0f, &out) ||
        !ccm_equal(&out, &ccm_entries[2], &ccm_entries[2], 0.0f)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_clamp_ends_obj, test_ccm_clamp_ends);

// An illuminant between two calibration points blends their matrices.
static mp_obj_t test_ccm_blend(void) {
    const omv_csi_ccm_table_t *table = ccm_table(3);
    omv_csi_ccm_t out;

    // The log-space midpoint of the first two illuminants.
    float mid0 = (ccm_entries[0].chroma[0] + ccm_entries[1].chroma[0]) / 2.0f;
    float mid1 = (ccm_entries[0].chroma[1] + ccm_entries[1].chroma[1]) / 2.0f;
    const float rgb[3] = { expf(mid0) * 100.0f, 100.0f, expf(mid1) * 100.0f };

    if (!omv_csi_ccm_compute(table, rgb, 100.0f, 0.0f, &out) ||
        !ccm_equal(&out, &ccm_entries[0], &ccm_entries[1], 0.5f)) {
        return mp_const_false;
    }

    // Blended rows still sum to 1 so that neutrals stay neutral.
    for (int r = 0; r < 3; r++) {
        if (!FLOAT_EQ(out.coeffs[r][0] + out.coeffs[r][1] + out.coeffs[r][2], 1.0f, 0.001f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_blend_obj, test_ccm_blend);

// Very dark and very bright frames fade to unity.
static mp_obj_t test_ccm_fade(void) {
    const omv_csi_ccm_table_t *table = ccm_table(3);
    const omv_csi_ccm_t unity = {
        .coeffs = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };
    const float unity_luma[] = { 0.0f, 8.0f, 220.0f, 255.0f };
    const float half_luma[] = { 20.0f, 185.0f };
    const float full_luma[] = { 32.0f, 100.0f, 150.0f };
    omv_csi_ccm_t out;

    for (size_t i = 0; i < OMV_ARRAY_SIZE(unity_luma); i++) {
        if (!omv_csi_ccm_compute(table, ccm_rgb[1], unity_luma[i], 0.0f, &out) || !ccm_is_unity(&out)) {
            return mp_const_false;
        }
    }
    for (size_t i = 0; i < OMV_ARRAY_SIZE(half_luma); i++) {
        if (!omv_csi_ccm_compute(table, ccm_rgb[1], half_luma[i], 0.0f, &out) ||
            !ccm_equal(&out, &unity, &ccm_entries[1], 0.5f)) {
            return mp_const_false;
        }
    }
    for (size_t i = 0; i < OMV_ARRAY_SIZE(full_luma); i++) {
        if (!omv_csi_ccm_compute(table, ccm_rgb[1], full_luma[i], 0.0f, &out) ||
            !ccm_equal(&out, &ccm_entries[1], &ccm_entries[1], 0.0f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_fade_obj, test_ccm_fade);

// High sensor gains fade to unity, and a table without a gain range never fades.
static mp_obj_t test_ccm_gain_fade(void) {
    const omv_csi_ccm_table_t *table = ccm_table(3);
    omv_csi_ccm_table_t gain_table = *table;
    const omv_csi_ccm_t unity = {
        .coeffs = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };
    omv_csi_ccm_t out;

    gain_table.gain_db[0] = 18.0f;
    gain_table.gain_db[1] = 24.0f;

    if (!omv_csi_ccm_compute(&gain_table, ccm_rgb[1], 100.0f, 0.0f, &out) ||
        !ccm_equal(&out, &ccm_entries[1], &ccm_entries[1], 0.0f)) {
        return mp_const_false;
    }
    if (!omv_csi_ccm_compute(&gain_table, ccm_rgb[1], 100.0f, 21.0f, &out) ||
        !ccm_equal(&out, &unity, &ccm_entries[1], 0.5f)) {
        return mp_const_false;
    }
    if (!omv_csi_ccm_compute(&gain_table, ccm_rgb[1], 100.0f, 30.0f, &out) || !ccm_is_unity(&out)) {
        return mp_const_false;
    }
    // The darker of the luminance and gain fades wins.
    if (!omv_csi_ccm_compute(&gain_table, ccm_rgb[1], 8.0f, 0.0f, &out) || !ccm_is_unity(&out)) {
        return mp_const_false;
    }
    if (!omv_csi_ccm_compute(table, ccm_rgb[1], 100.0f, 30.0f, &out) ||
        !ccm_equal(&out, &ccm_entries[1], &ccm_entries[1], 0.0f)) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_gain_fade_obj, test_ccm_gain_fade);

// Averages on the calibration illuminants' polyline are unchanged, off it they move to the closest
// point, past the ends they clamp to the end, and green is kept.
static mp_obj_t test_ccm_project(void) {
    const omv_csi_ccm_table_t *table = ccm_table(3);

    for (int i = 0; i < 3; i++) {
        float rgb[3] = { ccm_rgb[i][0], ccm_rgb[i][1], ccm_rgb[i][2] };
        if (!omv_csi_ccm_project(table, rgb) || fabsf(rgb[0] - ccm_rgb[i][0]) > 1e-3f ||
            fabsf(rgb[1] - ccm_rgb[i][1]) > 1e-3f || fabsf(rgb[2] - ccm_rgb[i][2]) > 1e-3f) {
            return mp_const_false;
        }
    }

    // Moved off the middle of the warmest segment toward magenta, it lands back on the middle.
    float magenta[3] = { 97.8988f, 100.0f, 66.0171f };
    if (!omv_csi_ccm_project(table, magenta) || fabsf(magenta[0] - 100.0f * sqrtf(0.8f)) > 1e-2f ||
        magenta[1] != 100.0f || fabsf(magenta[2] - 100.0f * sqrtf(0.4f)) > 1e-2f) {
        return mp_const_false;
    }

    float warm[3] = { 120.0f, 100.0f, 30.0f };
    if (!omv_csi_ccm_project(table, warm) || fabsf(warm[0] - 100.0f) > 1e-3f || fabsf(warm[2] - 50.0f) > 1e-3f) {
        return mp_const_false;
    }

    // Without at least two illuminants there's no polyline, and the averages are unchanged.
    float rgb[3] = { 120.0f, 100.0f, 30.0f };
    if (omv_csi_ccm_project(NULL, rgb) || omv_csi_ccm_project(ccm_table(1), rgb) ||
        rgb[0] != 120.0f || rgb[2] != 30.0f) {
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_project_obj, test_ccm_project);

// A single-entry table always returns that entry.
static mp_obj_t test_ccm_single(void) {
    const omv_csi_ccm_table_t *table = ccm_table(1);

    for (int i = 0; i < 3; i++) {
        omv_csi_ccm_t out;
        if (!omv_csi_ccm_compute(table, ccm_rgb[i], 100.0f, 0.0f, &out) ||
            !ccm_equal(&out, &ccm_entries[0], &ccm_entries[0], 0.0f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_ccm_single_obj, test_ccm_single);

// Module definition
static const mp_rom_map_elem_t unittest_ccm_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_ccm) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_chroma), MP_ROM_PTR(&test_ccm_chroma_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_empty), MP_ROM_PTR(&test_ccm_empty_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_calibration_points), MP_ROM_PTR(&test_ccm_calibration_points_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_clamp_ends), MP_ROM_PTR(&test_ccm_clamp_ends_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_blend), MP_ROM_PTR(&test_ccm_blend_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_fade), MP_ROM_PTR(&test_ccm_fade_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_gain_fade), MP_ROM_PTR(&test_ccm_gain_fade_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_project), MP_ROM_PTR(&test_ccm_project_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_ccm_single), MP_ROM_PTR(&test_ccm_single_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_ccm_module_globals, unittest_ccm_module_globals_table);

const mp_obj_module_t unittest_ccm_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_ccm_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_ccm, unittest_ccm_module);

#endif // MICROPY_PY_UNITTEST && MICROPY_PY_CSI
