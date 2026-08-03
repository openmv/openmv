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
#ifndef __PY_IMAGE_STATS_H__
#define __PY_IMAGE_STATS_H__
typedef struct py_histogram_obj {
    mp_obj_base_t base;
    pixformat_t pixfmt;
    mp_obj_t LBins, ABins, BBins;
} py_histogram_obj_t;

extern const mp_obj_type_t py_histogram_type;
#ifdef IMLIB_ENABLE_GET_SIMILARITY
extern const qstr similarity_fields[4];
#endif
mp_obj_t py_statistics_attrtuple(statistics_t *stats);
void py_histogram_free_hist(histogram_t *hist);
#endif // __PY_IMAGE_STATS_H__
