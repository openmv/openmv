/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Generic image statistics functions.
 *
 */
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"

int32_t *imlib_histogram(image_t *img, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return NULL;
    }
    int32_t *histogram;
    if (IM_IS_GS(img)) {
        histogram = fb_alloc0(IM_G_HIST_SIZE * sizeof(int32_t));
    } else {
        histogram = fb_alloc0((IM_L_HIST_SIZE * sizeof(int32_t)) +
                              (IM_A_HIST_SIZE * sizeof(int32_t)) +
                              (IM_B_HIST_SIZE * sizeof(int32_t)));
    }
    if (IM_IS_GS(img)) {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j);
                int y = (rect.y + i);
                histogram[IM_GET_GS_PIXEL(img, x, y)]++;
            }
        }
    } else {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j);
                int y = (rect.y + i);
                const int pixel = IM_GET_RGB565_PIXEL(img, x, y);
                histogram[IM_RGB5652L(pixel) + IM_L_HIST_OFFSET + 128]++;
                histogram[IM_RGB5652A(pixel) + IM_A_HIST_OFFSET + 128]++;
                histogram[IM_RGB5652B(pixel) + IM_B_HIST_OFFSET + 128]++;
            }
        }
    }
    return histogram;
}

void imlib_statistics(image_t *img, rectangle_t *r, statistics_t *out)
{
    memset(out, 0, sizeof(statistics_t));
    int32_t *histogram = imlib_histogram(img, r);
    if (!histogram) {
        return;
    }
    if (IM_IS_GS(img)) {
        int sum = 0, avg = 0;
        int mode_count = -1;
        bool min_flag = false;
        for (int i = IM_G_HIST_OFFSET; i < (IM_G_HIST_OFFSET + IM_G_HIST_SIZE); i++) {
            sum += histogram[i];
            avg += (i-IM_G_HIST_OFFSET) * histogram[i];
            if (histogram[i] > mode_count) {
                mode_count = histogram[i];
                out->g_mode = (i-IM_G_HIST_OFFSET);
            }
            if (histogram[i] && (!min_flag)) {
                min_flag = true;
                out->g_min = (i-IM_G_HIST_OFFSET);
            }
            if (histogram[i]) {
                out->g_max = (i-IM_G_HIST_OFFSET);
            }
        }
        out->g_mean = avg / sum;
        // lower_q = 1/4th, median = 1/2, upper_q = 3/4th
        int lq = (sum+3)/4, mid = (sum+1)/2, uq = ((sum*3)+3)/4;
        int st_dev_count = 0, median_count = 0;
        for (int i = IM_G_HIST_OFFSET; i < (IM_G_HIST_OFFSET + IM_G_HIST_SIZE); i++) {
            st_dev_count += histogram[i] *
                    ((i-IM_G_HIST_OFFSET)-out->g_mean) *
                    ((i-IM_G_HIST_OFFSET)-out->g_mean);
            if ((median_count<lq) && (lq<=(median_count+histogram[i]))) {
                out->g_lower_q = (i-IM_G_HIST_OFFSET);
            }
            if ((median_count<mid) && (mid<=(median_count+histogram[i]))) {
                out->g_median = (i-IM_G_HIST_OFFSET);
            }
            if ((median_count<uq) && (uq<=(median_count+histogram[i]))) {
                out->g_upper_q = (i-IM_G_HIST_OFFSET);
            }
            median_count += histogram[i];
        }
        out->g_st_dev = fast_sqrtf(st_dev_count / sum);
    } else {
        {
            int sum = 0, avg = 0;
            int mode_count = -1;
            bool min_flag = false;
            for (int i = IM_L_HIST_OFFSET; i < (IM_L_HIST_OFFSET + IM_L_HIST_SIZE); i++) {
                sum += histogram[i];
                avg += (i-IM_L_HIST_OFFSET-128) * histogram[i];
                if (histogram[i] > mode_count) {
                    mode_count = histogram[i];
                    out->l_mode = (i-IM_L_HIST_OFFSET-128);
                }
                if (histogram[i] && (!min_flag)) {
                    min_flag = true;
                    out->l_min = (i-IM_L_HIST_OFFSET-128);
                }
                if (histogram[i]) {
                    out->l_max = (i-IM_L_HIST_OFFSET-128);
                }
            }
            out->l_mean = avg / sum;
            // lower_q = 1/4th, median = 1/2, upper_q = 3/4th
            int lq = (sum+3)/4, mid = (sum+1)/2, uq = ((sum*3)+3)/4;
            int st_dev_count = 0, median_count = 0;
            for (int i = IM_L_HIST_OFFSET; i < (IM_L_HIST_OFFSET + IM_L_HIST_SIZE); i++) {
                st_dev_count += histogram[i] *
                        ((i-IM_L_HIST_OFFSET-128)-out->l_mean) *
                        ((i-IM_L_HIST_OFFSET-128)-out->l_mean);
                if ((median_count<lq) && (lq<=(median_count+histogram[i]))) {
                    out->l_lower_q = (i-IM_L_HIST_OFFSET-128);
                }
                if ((median_count<mid) && (mid<=(median_count+histogram[i]))) {
                    out->l_median = (i-IM_L_HIST_OFFSET-128);
                }
                if ((median_count<uq) && (uq<=(median_count+histogram[i]))) {
                    out->l_upper_q = (i-IM_L_HIST_OFFSET-128);
                }
                median_count += histogram[i];
            }
            out->l_st_dev = fast_sqrtf(st_dev_count / sum);
        }
        ////////////////////////////////////////////////////////////////////////
        {
            int sum = 0, avg = 0;
            int mode_count = -1;
            bool min_flag = false;
            for (int i = IM_A_HIST_OFFSET; i < (IM_A_HIST_OFFSET + IM_A_HIST_SIZE); i++) {
                sum += histogram[i];
                avg += (i-IM_A_HIST_OFFSET-128) * histogram[i];
                if (histogram[i] > mode_count) {
                    mode_count = histogram[i];
                    out->a_mode = (i-IM_A_HIST_OFFSET-128);
                }
                if (histogram[i] && (!min_flag)) {
                    min_flag = true;
                    out->a_min = (i-IM_A_HIST_OFFSET-128);
                }
                if (histogram[i]) {
                    out->a_max = (i-IM_A_HIST_OFFSET-128);
                }
            }
            out->a_mean = avg / sum;
            // lower_q = 1/4th, median = 1/2, upper_q = 3/4th
            int lq = (sum+3)/4, mid = (sum+1)/2, uq = ((sum*3)+3)/4;
            int st_dev_count = 0, median_count = 0;
            for (int i = IM_A_HIST_OFFSET; i < (IM_A_HIST_OFFSET + IM_A_HIST_SIZE); i++) {
                st_dev_count += histogram[i] *
                        ((i-IM_A_HIST_OFFSET-128)-out->l_mean) *
                        ((i-IM_A_HIST_OFFSET-128)-out->l_mean);
                if ((median_count<lq) && (lq<=(median_count+histogram[i]))) {
                    out->a_lower_q = (i-IM_A_HIST_OFFSET-128);
                }
                if ((median_count<mid) && (mid<=(median_count+histogram[i]))) {
                    out->a_median = (i-IM_A_HIST_OFFSET-128);
                }
                if ((median_count<uq) && (uq<=(median_count+histogram[i]))) {
                    out->a_upper_q = (i-IM_A_HIST_OFFSET-128);
                }
                median_count += histogram[i];
            }
            out->a_st_dev = fast_sqrtf(st_dev_count / sum);
        }
        ////////////////////////////////////////////////////////////////////////
        {
            int sum = 0, avg = 0;
            int mode_count = -1;
            bool min_flag = false;
            for (int i = IM_B_HIST_OFFSET; i < (IM_B_HIST_OFFSET + IM_B_HIST_SIZE); i++) {
                sum += histogram[i];
                avg += (i-IM_B_HIST_OFFSET-128) * histogram[i];
                if (histogram[i] > mode_count) {
                    mode_count = histogram[i];
                    out->b_mode = (i-IM_B_HIST_OFFSET-128);
                }
                if (histogram[i] && (!min_flag)) {
                    min_flag = true;
                    out->b_min = (i-IM_B_HIST_OFFSET-128);
                }
                if (histogram[i]) {
                    out->b_max = (i-IM_B_HIST_OFFSET-128);
                }
            }
            out->b_mean = avg / sum;
            // lower_q = 1/4th, median = 1/2, upper_q = 3/4th
            int lq = (sum+3)/4, mid = (sum+1)/2, uq = ((sum*3)+3)/4;
            int st_dev_count = 0, median_count = 0;
            for (int i = IM_B_HIST_OFFSET; i < (IM_B_HIST_OFFSET + IM_B_HIST_SIZE); i++) {
                st_dev_count += histogram[i] *
                        ((i-IM_B_HIST_OFFSET-128)-out->l_mean) *
                        ((i-IM_B_HIST_OFFSET-128)-out->l_mean);
                if ((median_count<lq) && (lq<=(median_count+histogram[i]))) {
                    out->b_lower_q = (i-IM_B_HIST_OFFSET-128);
                }
                if ((median_count<mid) && (mid<=(median_count+histogram[i]))) {
                    out->b_median = (i-IM_B_HIST_OFFSET-128);
                }
                if ((median_count<uq) && (uq<=(median_count+histogram[i]))) {
                    out->b_upper_q = (i-IM_B_HIST_OFFSET-128);
                }
                median_count += histogram[i];
            }
            out->b_st_dev = fast_sqrtf(st_dev_count / sum);
        }
    }
    fb_free();
}
