/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
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
 * Image statistics Python module.
 */
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/runtime.h"

#include "imlib.h"
#include "umalloc.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"
#include "py_image_stats.h"

#ifdef IMLIB_ENABLE_GET_SIMILARITY
const qstr similarity_fields[] = {
    MP_QSTR_mean, MP_QSTR_stdev, MP_QSTR_min, MP_QSTR_max
};

#endif // IMLIB_ENABLE_GET_SIMILARITY

// Statistics Object //
static const qstr statistics_fields[] = {
    MP_QSTR_mean, MP_QSTR_median, MP_QSTR_mode, MP_QSTR_stdev,
    MP_QSTR_min, MP_QSTR_max, MP_QSTR_lq, MP_QSTR_uq,
    MP_QSTR_l_mean, MP_QSTR_l_median, MP_QSTR_l_mode, MP_QSTR_l_stdev,
    MP_QSTR_l_min, MP_QSTR_l_max, MP_QSTR_l_lq, MP_QSTR_l_uq,
    MP_QSTR_a_mean, MP_QSTR_a_median, MP_QSTR_a_mode, MP_QSTR_a_stdev,
    MP_QSTR_a_min, MP_QSTR_a_max, MP_QSTR_a_lq, MP_QSTR_a_uq,
    MP_QSTR_b_mean, MP_QSTR_b_median, MP_QSTR_b_mode, MP_QSTR_b_stdev,
    MP_QSTR_b_min, MP_QSTR_b_max, MP_QSTR_b_lq, MP_QSTR_b_uq,
};

mp_obj_t py_statistics_attrtuple(statistics_t *stats) {
    mp_obj_t items[] = {
        mp_obj_new_int(stats->LMean),  mp_obj_new_int(stats->LMedian),
        mp_obj_new_int(stats->LMode),  mp_obj_new_int(stats->LSTDev),
        mp_obj_new_int(stats->LMin),   mp_obj_new_int(stats->LMax),
        mp_obj_new_int(stats->LLQ),    mp_obj_new_int(stats->LUQ),
        mp_obj_new_int(stats->LMean),  mp_obj_new_int(stats->LMedian),
        mp_obj_new_int(stats->LMode),  mp_obj_new_int(stats->LSTDev),
        mp_obj_new_int(stats->LMin),   mp_obj_new_int(stats->LMax),
        mp_obj_new_int(stats->LLQ),    mp_obj_new_int(stats->LUQ),
        mp_obj_new_int(stats->AMean),  mp_obj_new_int(stats->AMedian),
        mp_obj_new_int(stats->AMode),  mp_obj_new_int(stats->ASTDev),
        mp_obj_new_int(stats->AMin),   mp_obj_new_int(stats->AMax),
        mp_obj_new_int(stats->ALQ),    mp_obj_new_int(stats->AUQ),
        mp_obj_new_int(stats->BMean),  mp_obj_new_int(stats->BMedian),
        mp_obj_new_int(stats->BMode),  mp_obj_new_int(stats->BSTDev),
        mp_obj_new_int(stats->BMin),   mp_obj_new_int(stats->BMax),
        mp_obj_new_int(stats->BLQ),    mp_obj_new_int(stats->BUQ),
    };
    return mp_obj_new_attrtuple(statistics_fields, MP_ARRAY_SIZE(statistics_fields), items);
}

// Percentile Object //
static const qstr percentile_fields[] = {
    MP_QSTR_value, MP_QSTR_l_value, MP_QSTR_a_value, MP_QSTR_b_value
};

// Threshold Object //
static const qstr threshold_fields[] = {
    MP_QSTR_value, MP_QSTR_l_value, MP_QSTR_a_value, MP_QSTR_b_value
};

// Histogram Object //
#define py_histogram_obj_size    3
static void py_histogram_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    py_histogram_obj_t *self = self_in;
    switch (self->pixfmt) {
        case PIXFORMAT_BINARY: {
            mp_printf(print, "{\"bins\":");
            mp_obj_print_helper(print, self->LBins, kind);
            mp_printf(print, "}");
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            mp_printf(print, "{\"bins\":");
            mp_obj_print_helper(print, self->LBins, kind);
            mp_printf(print, "}");
            break;
        }
        case PIXFORMAT_RGB565: {
            mp_printf(print, "{\"l_bins\":");
            mp_obj_print_helper(print, self->LBins, kind);
            mp_printf(print, ", \"a_bins\":");
            mp_obj_print_helper(print, self->ABins, kind);
            mp_printf(print, ", \"b_bins\":");
            mp_obj_print_helper(print, self->BBins, kind);
            mp_printf(print, "}");
            break;
        }
        default: {
            mp_printf(print, "{}");
            break;
        }
    }
}

static mp_obj_t py_histogram_subscr(mp_obj_t self_in, mp_obj_t index, mp_obj_t value) {
    if (value == MP_OBJ_SENTINEL) {
        // load
        py_histogram_obj_t *self = self_in;
        if (MP_OBJ_IS_TYPE(index, &mp_type_slice)) {
            mp_bound_slice_t slice;
            if (!mp_seq_get_fast_slice_indexes(py_histogram_obj_size, index, &slice)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("only slices with step=1 (aka None) are supported"));
            }
            mp_obj_tuple_t *result = mp_obj_new_tuple(slice.stop - slice.start, NULL);
            mp_seq_copy(result->items, &(self->LBins) + slice.start, result->len, mp_obj_t);
            return result;
        }
        switch (mp_get_index(self->base.type, py_histogram_obj_size, index, false)) {
            case 0: return self->LBins;
            case 1: return self->ABins;
            case 2: return self->BBins;
        }
    }
    return MP_OBJ_NULL; // op not supported
}

mp_obj_t py_histogram_bins(mp_obj_t self_in) {
    return ((py_histogram_obj_t *) MP_OBJ_TO_PTR(self_in))->LBins;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_bins_obj, py_histogram_bins);

mp_obj_t py_histogram_l_bins(mp_obj_t self_in) {
    return ((py_histogram_obj_t *) MP_OBJ_TO_PTR(self_in))->LBins;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_l_bins_obj, py_histogram_l_bins);

mp_obj_t py_histogram_a_bins(mp_obj_t self_in) {
    return ((py_histogram_obj_t *) MP_OBJ_TO_PTR(self_in))->ABins;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_a_bins_obj, py_histogram_a_bins);

mp_obj_t py_histogram_b_bins(mp_obj_t self_in) {
    return ((py_histogram_obj_t *) MP_OBJ_TO_PTR(self_in))->BBins;
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_b_bins_obj, py_histogram_b_bins);

static void py_histogram_to_hist(py_histogram_obj_t *self, histogram_t *hist) {
    mp_obj_list_t *l_bins = MP_OBJ_TO_PTR(self->LBins);
    mp_obj_list_t *a_bins = MP_OBJ_TO_PTR(self->ABins);
    mp_obj_list_t *b_bins = MP_OBJ_TO_PTR(self->BBins);

    hist->LBinCount = l_bins->len;
    hist->ABinCount = a_bins->len;
    hist->BBinCount = b_bins->len;
    hist->LBins = uma_malloc(hist->LBinCount * sizeof(float), UMA_DTCM);
    hist->ABins = uma_malloc(hist->ABinCount * sizeof(float), UMA_DTCM);
    hist->BBins = uma_malloc(hist->BBinCount * sizeof(float), UMA_DTCM);

    for (int i = 0; i < hist->LBinCount; i++) {
        hist->LBins[i] = mp_obj_get_float_to_f(l_bins->items[i]);
    }
    for (int i = 0; i < hist->ABinCount; i++) {
        hist->ABins[i] = mp_obj_get_float_to_f(a_bins->items[i]);
    }
    for (int i = 0; i < hist->BBinCount; i++) {
        hist->BBins[i] = mp_obj_get_float_to_f(b_bins->items[i]);
    }
}

void py_histogram_free_hist(histogram_t *hist) {
    uma_free(hist->BBins);
    uma_free(hist->ABins);
    uma_free(hist->LBins);
}

mp_obj_t py_histogram_get_percentile(mp_obj_t self_in, mp_obj_t percentile) {
    py_histogram_obj_t *self = MP_OBJ_TO_PTR(self_in);
    histogram_t hist;
    py_histogram_to_hist(self, &hist);

    percentile_t p;
    imlib_get_percentile(&p, self->pixfmt, &hist, mp_obj_get_float_to_f(percentile));
    py_histogram_free_hist(&hist);

    mp_obj_t items[] = {
        mp_obj_new_int(p.LValue),
        mp_obj_new_int(p.LValue),
        mp_obj_new_int(p.AValue),
        mp_obj_new_int(p.BValue),
    };
    return mp_obj_new_attrtuple(percentile_fields, MP_ARRAY_SIZE(percentile_fields), items);
}
static MP_DEFINE_CONST_FUN_OBJ_2(py_histogram_get_percentile_obj, py_histogram_get_percentile);

mp_obj_t py_histogram_get_threshold(mp_obj_t self_in) {
    py_histogram_obj_t *self = MP_OBJ_TO_PTR(self_in);
    histogram_t hist;
    py_histogram_to_hist(self, &hist);

    threshold_t t;
    imlib_get_threshold(&t, self->pixfmt, &hist);
    py_histogram_free_hist(&hist);

    mp_obj_t items[] = {
        mp_obj_new_int(t.LValue),
        mp_obj_new_int(t.LValue),
        mp_obj_new_int(t.AValue),
        mp_obj_new_int(t.BValue),
    };
    return mp_obj_new_attrtuple(threshold_fields, MP_ARRAY_SIZE(threshold_fields), items);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_get_threshold_obj, py_histogram_get_threshold);

mp_obj_t py_histogram_get_statistics(mp_obj_t self_in) {
    py_histogram_obj_t *self = MP_OBJ_TO_PTR(self_in);
    histogram_t hist;
    py_histogram_to_hist(self, &hist);

    statistics_t stats;
    imlib_get_statistics(&stats, self->pixfmt, &hist);
    py_histogram_free_hist(&hist);

    return py_statistics_attrtuple(&stats);
}
static MP_DEFINE_CONST_FUN_OBJ_1(py_histogram_get_statistics_obj, py_histogram_get_statistics);

static const mp_rom_map_elem_t py_histogram_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_bins), MP_ROM_PTR(&py_histogram_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_l_bins), MP_ROM_PTR(&py_histogram_l_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_a_bins), MP_ROM_PTR(&py_histogram_a_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_b_bins), MP_ROM_PTR(&py_histogram_b_bins_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_percentile), MP_ROM_PTR(&py_histogram_get_percentile_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_threshold), MP_ROM_PTR(&py_histogram_get_threshold_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_stats), MP_ROM_PTR(&py_histogram_get_statistics_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_statistics), MP_ROM_PTR(&py_histogram_get_statistics_obj) },
    { MP_ROM_QSTR(MP_QSTR_statistics), MP_ROM_PTR(&py_histogram_get_statistics_obj) }
};

static MP_DEFINE_CONST_DICT(py_histogram_locals_dict, py_histogram_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    py_histogram_type,
    MP_QSTR_histogram,
    MP_TYPE_FLAG_NONE,
    print, py_histogram_print,
    subscr, py_histogram_subscr,
    locals_dict, &py_histogram_locals_dict
    );

