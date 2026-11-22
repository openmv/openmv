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
 * AWB image stats unit tests.
 */
#include "imlib_config.h"
#include "board_config.h"

#if MICROPY_PY_UNITTEST && MICROPY_PY_CSI && defined(OMV_CSI_STATS_IMAGE_ENABLE)

#include <math.h>
#include <string.h>
#include "py/runtime.h"
#include "py/obj.h"

#include "imlib.h"
#include "omv_csi.h"
#include "unittest.h"

#define STATS_W     (64)
#define STATS_H     (32)
#define STATS_N     (STATS_W * STATS_H)

static omv_csi_t stats_csi;
static uint16_t stats_buf[STATS_N];

// Clears the averages and sets an identity xform.
static omv_csi_t *stats_reset(void) {
    omv_csi_t *csi = &stats_csi;
    memset(&csi->stats, 0, sizeof(csi->stats));
    memset(&csi->stats_xform, 0, sizeof(csi->stats_xform));
    for (int i = 0; i < 3; i++) {
        csi->stats_xform.gains[i] = 1.0f;
    }
    csi->stats_xform.gamma = 1.0f;
    return csi;
}

static void stats_fill(image_t *image, int start, int count, uint16_t pixel) {
    image->w = STATS_W;
    image->h = STATS_H;
    image->pixfmt = PIXFORMAT_RGB565;
    image->data = (uint8_t *) stats_buf;
    for (int i = start; i < start + count; i++) {
        stats_buf[i] = pixel;
    }
}

static bool stats_equal(omv_csi_t *csi, float r, float g, float b, float tol) {
    return FLOAT_EQ(csi->stats.r_avg, r, tol) && FLOAT_EQ(csi->stats.g_avg, g, tol) &&
           FLOAT_EQ(csi->stats.b_avg, b, tol);
}

// With an identity xform, the averages are the decoded pixel values.
static mp_obj_t test_csi_stats_identity(void) {
    omv_csi_t *csi = stats_reset();
    image_t image;
    uint16_t pixel = COLOR_R8_G8_B8_TO_RGB565(160, 120, 80);
    stats_fill(&image, 0, STATS_N, pixel);

    if (!omv_csi_stats_update_image(csi, &image, 0)) {
        return mp_const_false;
    }
    return mp_obj_new_bool(stats_equal(csi, COLOR_RGB565_TO_R8(pixel), COLOR_RGB565_TO_G8(pixel),
                                       COLOR_RGB565_TO_B8(pixel), 0.001f));
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_identity_obj, test_csi_stats_identity);

// The gains and CCM are undone, so applying them again to the averages gives back the pixel.
static mp_obj_t test_csi_stats_undo(void) {
    omv_csi_t *csi = stats_reset();
    omv_csi_stats_xform_t *xform = &csi->stats_xform;
    const float gains[3] = { 1.5f, 1.0f, 0.75f };
    const float ccm[3][3] = { { 1.5f, -0.25f, -0.25f }, { -0.25f, 1.5f, -0.25f }, { -0.25f, -0.25f, 1.5f } };
    const float offsets[3] = { 4.0f, -2.0f, 1.0f };
    image_t image;
    uint16_t pixel = COLOR_R8_G8_B8_TO_RGB565(150, 120, 90);
    const float p[3] = { COLOR_RGB565_TO_R8(pixel), COLOR_RGB565_TO_G8(pixel), COLOR_RGB565_TO_B8(pixel) };

    memcpy(xform->gains, gains, sizeof(gains));
    memcpy(xform->ccm, ccm, sizeof(ccm));
    memcpy(xform->offsets, offsets, sizeof(offsets));
    xform->ccm_valid = true;
    stats_fill(&image, 0, STATS_N, pixel);

    if (!omv_csi_stats_update_image(csi, &image, 0)) {
        return mp_const_false;
    }

    const float s[3] = { csi->stats.r_avg * gains[0], csi->stats.g_avg * gains[1], csi->stats.b_avg * gains[2] };
    for (int r = 0; r < 3; r++) {
        if (!FLOAT_EQ(ccm[r][0] * s[0] + ccm[r][1] * s[1] + ccm[r][2] * s[2] + offsets[r], p[r], 0.01f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_undo_obj, test_csi_stats_undo);

// Gamma is undone by scaling each pixel by (Y / 255)^(gamma - 1), which keeps its color ratios.
static mp_obj_t test_csi_stats_gamma(void) {
    omv_csi_t *csi = stats_reset();
    image_t image;
    uint16_t pixel = COLOR_R8_G8_B8_TO_RGB565(200, 160, 120);
    int r = COLOR_RGB565_TO_R8(pixel);
    int g = COLOR_RGB565_TO_G8(pixel);
    int b = COLOR_RGB565_TO_B8(pixel);
    float w = powf(((r * 77 + g * 150 + b * 29) >> 8) / 255.0f, 2.2f - 1.0f);

    csi->stats_xform.gamma = 2.2f;
    stats_fill(&image, 0, STATS_N, pixel);

    if (!omv_csi_stats_update_image(csi, &image, 0)) {
        return mp_const_false;
    }

    // The weights are 8-bit fixed point.
    if (!stats_equal(csi, r * w, g * w, b * w, 0.5f)) {
        return mp_const_false;
    }
    return mp_obj_new_bool(FLOAT_EQ(csi->stats.r_avg / csi->stats.g_avg, (float) r / g, 0.001f) &&
                           FLOAT_EQ(csi->stats.b_avg / csi->stats.g_avg, (float) b / g, 0.001f));
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_gamma_obj, test_csi_stats_gamma);

// Pixels clipped or dark in any one channel are skipped, leaving only the gray pixels.
static mp_obj_t test_csi_stats_reject(void) {
    const uint16_t bad[] = {
        COLOR_R8_G8_B8_TO_RGB565(255, 100, 100),
        COLOR_R8_G8_B8_TO_RGB565(100, 255, 100),
        COLOR_R8_G8_B8_TO_RGB565(100, 100, 255),
        COLOR_R8_G8_B8_TO_RGB565(8, 100, 100),
        COLOR_R8_G8_B8_TO_RGB565(100, 8, 100),
        COLOR_R8_G8_B8_TO_RGB565(100, 100, 8),
    };
    uint16_t gray = COLOR_R8_G8_B8_TO_RGB565(128, 128, 128);

    for (size_t i = 0; i < OMV_ARRAY_SIZE(bad); i++) {
        omv_csi_t *csi = stats_reset();
        image_t image;
        stats_fill(&image, 0, STATS_N / 2, gray);
        stats_fill(&image, STATS_N / 2, STATS_N / 2, bad[i]);

        if (!omv_csi_stats_update_image(csi, &image, 0) ||
            !stats_equal(csi, COLOR_RGB565_TO_R8(gray), COLOR_RGB565_TO_G8(gray), COLOR_RGB565_TO_B8(gray), 0.001f)) {
            return mp_const_false;
        }
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_reject_obj, test_csi_stats_reject);

// The averages are held, with a current time base, when fewer than 10% of the pixels are usable.
static mp_obj_t test_csi_stats_hold(void) {
    omv_csi_t *csi = stats_reset();
    image_t image;
    uint16_t clipped = COLOR_R8_G8_B8_TO_RGB565(255, 255, 255);
    uint16_t gray = COLOR_R8_G8_B8_TO_RGB565(128, 128, 128);
    int min_usable = (STATS_N + 9) / 10;

    // Initialize the averages to gray.
    stats_fill(&image, 0, STATS_N, gray);
    if (!omv_csi_stats_update_image(csi, &image, 0)) {
        return mp_const_false;
    }
    float avg = csi->stats.g_avg;

    // Just under 10% usable holds the averages.
    stats_fill(&image, min_usable - 1, STATS_N - min_usable + 1, clipped);
    if (omv_csi_stats_update_image(csi, &image, 1000) || csi->stats.g_avg != avg || csi->stats.last_ms != 1000) {
        return mp_const_false;
    }

    // Exactly 10% usable updates them.
    stats_fill(&image, min_usable - 1, 1, gray);
    return mp_obj_new_bool(omv_csi_stats_update_image(csi, &image, 2000) && csi->stats.last_ms == 2000);
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_hold_obj, test_csi_stats_hold);

// Only RGB565 images are measured.
static mp_obj_t test_csi_stats_pixfmt(void) {
    omv_csi_t *csi = stats_reset();
    image_t image;
    stats_fill(&image, 0, STATS_N, COLOR_R8_G8_B8_TO_RGB565(128, 128, 128));
    image.pixfmt = PIXFORMAT_GRAYSCALE;
    return mp_obj_new_bool(!omv_csi_stats_update_image(csi, &image, 0) && !csi->stats.initialized);
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_pixfmt_obj, test_csi_stats_pixfmt);

// The averages ease toward a new value with a continuous-time EMA.
static mp_obj_t test_csi_stats_ema(void) {
    omv_csi_t *csi = stats_reset();
    uint32_t r = 100, g = 100, b = 100;
    omv_csi_stats_update(csi, &r, &g, &b, 0);

    // One time constant later, the averages have moved 1 - 1/e of the way.
    r = 200;
    b = 50;
    omv_csi_stats_update(csi, &r, &g, &b, 250);
    float alpha = 1.0f - expf(-1.0f);
    return mp_obj_new_bool(stats_equal(csi, 100.0f + 100.0f * alpha, 100.0f, 100.0f - 50.0f * alpha, 0.1f));
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_ema_obj, test_csi_stats_ema);

// A freeze keeps the time base current, so the EMA eases back instead of jumping when it resumes.
static mp_obj_t test_csi_stats_freeze(void) {
    omv_csi_t *csi = stats_reset();
    uint32_t r = 100, g = 100, b = 100;
    omv_csi_stats_update(csi, &r, &g, &b, 0);

    // Saturated frames with collapsed ratios freeze the averages.
    r = 250;
    g = 250;
    b = 250;
    omv_csi_stats_update(csi, &r, &g, &b, 10000);
    if (!stats_equal(csi, 100.0f, 100.0f, 100.0f, 0.001f) || csi->stats.last_ms != 10000) {
        return mp_const_false;
    }

    // 1ms after the freeze, the averages barely move.
    r = 200;
    g = 100;
    b = 50;
    omv_csi_stats_update(csi, &r, &g, &b, 10001);
    return mp_obj_new_bool(stats_equal(csi, 100.0f, 100.0f, 100.0f, 1.0f));
}
static MP_DEFINE_CONST_FUN_OBJ_0(test_csi_stats_freeze_obj, test_csi_stats_freeze);

// Module definition
static const mp_rom_map_elem_t unittest_csi_stats_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_unittest_csi_stats) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_identity), MP_ROM_PTR(&test_csi_stats_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_undo), MP_ROM_PTR(&test_csi_stats_undo_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_gamma), MP_ROM_PTR(&test_csi_stats_gamma_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_reject), MP_ROM_PTR(&test_csi_stats_reject_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_hold), MP_ROM_PTR(&test_csi_stats_hold_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_pixfmt), MP_ROM_PTR(&test_csi_stats_pixfmt_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_ema), MP_ROM_PTR(&test_csi_stats_ema_obj) },
    { MP_ROM_QSTR(MP_QSTR_test_csi_stats_freeze), MP_ROM_PTR(&test_csi_stats_freeze_obj) },
};

static MP_DEFINE_CONST_DICT(unittest_csi_stats_module_globals, unittest_csi_stats_module_globals_table);

const mp_obj_module_t unittest_csi_stats_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &unittest_csi_stats_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_unittest_csi_stats, unittest_csi_stats_module);

#endif // MICROPY_PY_UNITTEST && MICROPY_PY_CSI && defined(OMV_CSI_STATS_IMAGE_ENABLE)
