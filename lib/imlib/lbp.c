/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
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
 * LBPu2 8,2 Operator.
 * Note: The distance function uses weights optimized for face recognition.
 * Note: See Timo Ahonen's "Face Recognition with Local Binary Patterns".
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "imlib.h"
#include "file_utils.h"
#ifdef IMLIB_ENABLE_FIND_LBP

#define LBP_HIST_SIZE      (59) //58 uniform hist + 1
#define LBP_NUM_REGIONS    (7)  //7x7 regions
#define LBP_DESC_SIZE      (LBP_NUM_REGIONS * LBP_NUM_REGIONS * LBP_HIST_SIZE)

const static int8_t lbp_weights[49] = {
    2, 1, 1, 1, 1, 1, 2,
    2, 4, 4, 1, 4, 4, 2,
    1, 1, 1, 0, 1, 1, 1,
    0, 1, 1, 0, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 2, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 0,
};

const static uint8_t uniform_tbl[256] = {
    0,   1,  2,  3,  4, 58,  5,  6,  7, 58, 58, 58,  8, 58,  9, 10,
    11, 58, 58, 58, 58, 58, 58, 58, 12, 58, 58, 58, 13, 58, 14, 15,
    16, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    17, 58, 58, 58, 58, 58, 58, 58, 18, 58, 58, 58, 19, 58, 20, 21,
    22, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    23, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    24, 58, 58, 58, 58, 58, 58, 58, 25, 58, 58, 58, 26, 58, 27, 28,
    29, 30, 58, 31, 58, 58, 58, 32, 58, 58, 58, 58, 58, 58, 58, 33,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 34,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 35,
    36, 37, 58, 38, 58, 58, 58, 39, 58, 58, 58, 58, 58, 58, 58, 40,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 41,
    42, 43, 58, 44, 58, 58, 58, 45, 58, 58, 58, 58, 58, 58, 58, 46,
    47, 48, 58, 49, 58, 58, 58, 50, 51, 52, 58, 53, 54, 55, 56, 57
};

uint8_t *imlib_lbp_desc(image_t *image, rectangle_t *roi) {
    int s = image->w; //stride
    int RX = roi->w / LBP_NUM_REGIONS;
    int RY = roi->h / LBP_NUM_REGIONS;
    uint8_t *data = image->data;
    uint8_t *desc = m_malloc0(LBP_DESC_SIZE);

    for (int y = roi->y; y < (roi->y + roi->h) - 3; y++) {
        int y_idx = ((y - roi->y) / RY) * LBP_NUM_REGIONS;
        for (int x = roi->x; x < (roi->x + roi->w) - 3; x++) {
            uint8_t lbp = 0;
            uint8_t p = data[(y + 1) * s + x + 1];
            int hist_idx = y_idx + (x - roi->x) / RX;

            lbp |= (data[(y + 0) * s + x + 0] >= p) << 0;
            lbp |= (data[(y + 0) * s + x + 1] >= p) << 1;
            lbp |= (data[(y + 0) * s + x + 2] >= p) << 2;
            lbp |= (data[(y + 1) * s + x + 2] >= p) << 3;
            lbp |= (data[(y + 2) * s + x + 2] >= p) << 4;
            lbp |= (data[(y + 2) * s + x + 1] >= p) << 5;
            lbp |= (data[(y + 2) * s + x + 0] >= p) << 6;
            lbp |= (data[(y + 1) * s + x + 0] >= p) << 7;

            desc[hist_idx * LBP_HIST_SIZE + uniform_tbl[lbp]]++;
        }
    }
    return desc;
}

int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1) {
    uint32_t sum = 0;
    for (int i = 0; i < LBP_DESC_SIZE; i++) {
        int w = lbp_weights[i / LBP_HIST_SIZE];
        sum += w * ((((d0[i] - d1[i]) * (d0[i] - d1[i])) / IM_MAX((d0[i] + d1[i]), 1)));
    }
    return sum;
}

int imlib_lbp_desc_save(file_t *fp, uint8_t *desc) {
    // Write descriptor using VFS abstraction
    file_write(fp, desc, LBP_DESC_SIZE);
    return 0;  // Success
}

int imlib_lbp_desc_load(file_t *fp, uint8_t **desc) {
    *desc = NULL;
    uint8_t *hist = m_malloc(LBP_DESC_SIZE);

    // Read descriptor using VFS abstraction
    // file_read will raise exception on error
    file_read(fp, hist, LBP_DESC_SIZE);
    *desc = hist;

    return 0;  // Success
}
#endif //IMLIB_ENABLE_FIND_LBP
