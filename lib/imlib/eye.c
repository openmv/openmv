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
 * Pupil localization using image gradients. See Fabian Timm's paper for details.
 */
#include "imlib.h"
#include "fmath.h"

static void find_gradients(image_t *src, array_t *gradients, int x_off, int y_off, int box_w, int box_h) {
    for (int y = y_off; y < y_off + box_h - 3; y++) {
        for (int x = x_off; x < x_off + box_w - 3; x++) {
            int vx = 0, vy = 0, w = src->w;
            // sobel_kernel
            vx = src->data[(y + 0) * w + x + 0]
                 - src->data[(y + 0) * w + x + 2]
                 + (src->data[(y + 1) * w + x + 0] << 1)
                 - (src->data[(y + 1) * w + x + 2] << 1)
                 + src->data[(y + 2) * w + x + 0]
                 - src->data[(y + 2) * w + x + 2];

            // sobel_kernel
            vy = src->data[(y + 0) * w + x + 0]
                 + (src->data[(y + 0) * w + x + 1] << 1)
                 + src->data[(y + 0) * w + x + 2]
                 - src->data[(y + 2) * w + x + 0]
                 - (src->data[(y + 2) * w + x + 1] << 1)
                 - src->data[(y + 2) * w + x + 2];

            float m = fast_sqrtf(vx * vx + vy * vy);
            if (m > 200) {
                vec_t *v = m_malloc(sizeof(vec_t));
                v->m = m;
                v->x = vx / m;
                v->y = vy / m;
                v->cx = x + 1;
                v->cy = y + 1;
                array_push_back(gradients, v);
            }
        }
    }
}

// TODO use the gradients median not average
static void filter_gradients(array_t *gradients) {
    float total_m = 0.0f;
    for (int i = 0; i < array_length(gradients); i++) {
        vec_t *v = (vec_t *) array_at(gradients, i);
        total_m += v->m;
    }

    float avg_m = total_m / array_length(gradients);

    for (int i = 0; i < array_length(gradients); i++) {
        vec_t *v = (vec_t *) array_at(gradients, i);
        float diff = (v->m - avg_m) * (v->m - avg_m);
        if (fast_sqrtf(diff) > 100) {
            array_erase(gradients, i);
        }
    }
}

static void find_iris(image_t *src, array_t *gradients, int x_off, int y_off, int box_w, int box_h, point_t *e) {
    int max_x = 0;
    int max_y = 0;
    float max_dot = 0.0f;

    for (int y = y_off; y < y_off + box_h; y++) {
        for (int x = x_off; x < x_off + box_w; x++) {
            float sum_dot = 0.0f;
            for (int i = 0; i < array_length(gradients); i++) {
                // get gradient vector  g
                vec_t *v = (vec_t *) array_at(gradients, i);

                // get vector from gradient to centor d
                vec_t d = {x - v->cx, y - v->cy};

                // normalize d vector
                float m = fast_sqrtf(d.x * d.x + d.y * d.y);
                d.x = d.x / m;
                d.y = d.y / m;

                // compute the dot product d.g
                float t = (d.x * v->x) + (d.y * v->y);

                // d,g should point the same direction
                if (t > 0.0) {
                    // dark centres are more likely to be pupils than
                    // bright centres, so we use the grayscale value as weight.
                    sum_dot += t * t * (255 - src->data[y * src->w + x]);
                }
            }
            sum_dot = sum_dot / array_length(gradients);

            if (sum_dot > max_dot) {
                max_dot = sum_dot;
                max_x = x;
                max_y = y;
            }
        }
    }

    e->x = max_x;
    e->y = max_y;
}

// This function should be called on an ROI detected with the eye Haar cascade.
void imlib_find_iris(image_t *src, point_t *iris, rectangle_t *roi) {
    array_t *iris_gradients;
    array_alloc(&iris_gradients, m_free);

    // Tune these offsets to skip eyebrows and reduce window size
    int box_w = roi->w - ((int) (0.15f * roi->w));
    int box_h = roi->h - ((int) (0.40f * roi->h));
    int x_off = roi->x + ((int) (0.15f * roi->w));
    int y_off = roi->y + ((int) (0.40f * roi->h));

    // find gradients with strong magnitudes
    find_gradients(src, iris_gradients,  x_off, y_off, box_w, box_h);

    // filter gradients
    filter_gradients(iris_gradients);

    // search for iriss
    find_iris(src, iris_gradients, x_off, y_off, box_w, box_h, iris);

    array_free(iris_gradients);
}
