/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "imlib.h"

#ifdef IMLIB_ENABLE_REMOVE_SHADOWS
// http://arma.sourceforge.net/shadows/
// http://dx.doi.org/10.1016/j.patcog.2011.10.001

#define imlib_remove_shadows_kernel_rank 1
#define imlib_remove_shadows_kernel_size ((imlib_remove_shadows_kernel_rank * 2) + 1)

typedef struct imlib_remove_shadows_line_op_state {
    uint16_t *img_lines[imlib_remove_shadows_kernel_size];
    uint16_t *other_lines[imlib_remove_shadows_kernel_size];
    uint16_t *out_lines[imlib_remove_shadows_kernel_size];
    int lines_processed;
} imlib_remove_shadows_line_op_state_t;

static void imlib_remove_shadows_sub_sub_line_op(image_t *img, int line, void *data, bool vflipped)
{
    imlib_remove_shadows_line_op_state_t *state = (imlib_remove_shadows_line_op_state_t *) data;

    state->lines_processed += 1;

    if (state->lines_processed >= imlib_remove_shadows_kernel_size) {
        int y = vflipped ? ((line - 1) + imlib_remove_shadows_kernel_size) : ((line + 1) - imlib_remove_shadows_kernel_size);
        int index = y % imlib_remove_shadows_kernel_size;
        memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y), state->out_lines[index], img->w * sizeof(uint16_t));
    }
}

static void imlib_remove_shadows_sub_line_op(image_t *img, int line, void *data, bool vflipped)
{
    imlib_remove_shadows_line_op_state_t *state = (imlib_remove_shadows_line_op_state_t *) data;

    if (state->lines_processed >= imlib_remove_shadows_kernel_rank) {
        int y = vflipped ? (line + imlib_remove_shadows_kernel_rank) : (line - imlib_remove_shadows_kernel_rank);
        int index = y % imlib_remove_shadows_kernel_size;

        for (int x = 0, xx = img->w; x < xx; x++) {
            int img_pixel = IMAGE_GET_RGB565_PIXEL_FAST(state->img_lines[index], x);
            int img_r = COLOR_RGB565_TO_R8(img_pixel);
            int img_g = COLOR_RGB565_TO_G8(img_pixel);
            int img_b = COLOR_RGB565_TO_B8(img_pixel);
            float img_v = IM_MAX(IM_MAX(img_r, img_g), img_b);
            int other_pixel = IMAGE_GET_RGB565_PIXEL_FAST(state->other_lines[index], x);
            int other_r = COLOR_RGB565_TO_R8(other_pixel);
            int other_g = COLOR_RGB565_TO_G8(other_pixel);
            int other_b = COLOR_RGB565_TO_B8(other_pixel);
            float other_v = IM_MAX(IM_MAX(other_r, other_g), other_b);
            float ratio = img_v / other_v;

            if ((0.3f < ratio) && (ratio < 1.0f)) {
                int minY = IM_MAX(y - imlib_remove_shadows_kernel_rank, 0);
                int maxY = IM_MIN(y + imlib_remove_shadows_kernel_rank, img->h - 1);
                int minX = IM_MAX(x - imlib_remove_shadows_kernel_rank, 0);
                int maxX = IM_MIN(x + imlib_remove_shadows_kernel_rank, img->w - 1);
                int windowArea = (maxX - minX + 1) * (maxY - minY + 1);
                int hDiffSum = 0;
                int sDiffSum = 0;

                for (int k_y = minY; k_y <= maxY; k_y++) {
                    int k_index = k_y % imlib_remove_shadows_kernel_size;

                    for (int k_x = minX; k_x <= maxX; k_x++) {
                        int k_img_pixel = IMAGE_GET_RGB565_PIXEL_FAST(state->img_lines[k_index], k_x);
                        int k_img_r = COLOR_RGB565_TO_R8(k_img_pixel);
                        int k_img_g = COLOR_RGB565_TO_G8(k_img_pixel);
                        int k_img_b = COLOR_RGB565_TO_B8(k_img_pixel);
                        int k_img_cmax = IM_MAX(IM_MAX(k_img_r, k_img_g), k_img_b);
                        int k_img_cmin = IM_MAX(IM_MAX(k_img_r, k_img_g), k_img_b);
                        float k_img_cdel = k_img_cmax - k_img_cmin;
                        float k_img_h = 0;
                        float k_img_s = k_img_cmax ? (k_img_cdel / k_img_cmax) : 0;
                        int k_other_pixel = IMAGE_GET_RGB565_PIXEL_FAST(state->other_lines[k_index], k_x);
                        int k_other_r = COLOR_RGB565_TO_R8(k_other_pixel);
                        int k_other_g = COLOR_RGB565_TO_G8(k_other_pixel);
                        int k_other_b = COLOR_RGB565_TO_B8(k_other_pixel);
                        int k_other_cmax = IM_MAX(IM_MAX(k_other_r, k_other_g), k_other_b);
                        int k_other_cmin = IM_MAX(IM_MAX(k_other_r, k_other_g), k_other_b);
                        float k_other_cdel = k_other_cmax - k_other_cmin;
                        float k_other_h = 0;
                        float k_other_s = k_other_cmax ? (k_other_cdel / k_other_cmax) : 0;

                        if (k_img_cdel) {
                            if (k_img_cmax == k_img_r) {
                                k_img_h = ((k_img_g - k_img_b) / k_img_cdel) + 0;
                            } else if (k_img_cmax == k_img_g) {
                                k_img_h = ((k_img_b - k_img_r) / k_img_cdel) + 2;
                            } else if (k_img_cmax == k_img_b) {
                                k_img_h = ((k_img_r - k_img_g) / k_img_cdel) + 4;
                            }
                            k_img_h *= 60;
                            if (k_img_h < 0) k_img_h += 360.0;
                        }

                        if (k_other_cdel) {
                            if (k_other_cmax == k_other_r) {
                                k_other_h = ((k_other_g - k_other_b) / k_other_cdel) + 0;
                            } else if (k_other_cmax == k_other_g) {
                                k_other_h = ((k_other_b - k_other_r) / k_other_cdel) + 2;
                            } else if (k_other_cmax == k_other_b) {
                                k_other_h = ((k_other_r - k_other_g) / k_other_cdel) + 4;
                            }
                            k_other_h *= 60;
                            if (k_other_h < 0) k_other_h += 360.0;
                        }

                        int hDiff = abs(k_img_h - k_other_h);
                        hDiffSum += (hDiff >= 90) ? (180 - hDiff) : hDiff;
                        sDiffSum += k_img_s - k_other_s;
                    }
                }

                bool hIsShadow = (hDiffSum / windowArea) < 48;
                bool sIsShadow = (sDiffSum / windowArea) < 40;
                IMAGE_PUT_RGB565_PIXEL_FAST(state->out_lines[index], x, (hIsShadow && sIsShadow) ? other_pixel : img_pixel);
            } else {
                IMAGE_PUT_RGB565_PIXEL_FAST(state->out_lines[index], x, img_pixel);
            }
        }
    }

    imlib_remove_shadows_sub_sub_line_op(img, line, data, vflipped);
}

static void imlib_remove_shadows_line_op(image_t *img, int line, void *other, void *data, bool vflipped)
{
    imlib_remove_shadows_line_op_state_t *state = (imlib_remove_shadows_line_op_state_t *) data;

    memcpy(state->img_lines[line % imlib_remove_shadows_kernel_size],
            IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, line), img->w * sizeof(uint16_t));

    memcpy(state->other_lines[line % imlib_remove_shadows_kernel_size],
            (uint16_t *) other, img->w * sizeof(uint16_t));

    imlib_remove_shadows_sub_line_op(img, line, data, vflipped);

    if (state->lines_processed == img->h) {
        for (int i = 0; i < imlib_remove_shadows_kernel_rank; i++) {
            line += vflipped ? -1 : 1;
            imlib_remove_shadows_sub_line_op(img, line, data, vflipped);
        }

        for (int i = 0; i < imlib_remove_shadows_kernel_rank; i++) {
            line += vflipped ? -1 : 1;
            imlib_remove_shadows_sub_sub_line_op(img, line, data, vflipped);
        }
    }
}

void imlib_remove_shadows(image_t *img, const char *path, image_t *other, int scalar, bool single)
{
    if (!single) {
        imlib_remove_shadows_line_op_state_t state;

        for (int i = 0; i < imlib_remove_shadows_kernel_size; i++) {
            state.img_lines[i] = fb_alloc(img->w * sizeof(uint16_t));
            state.other_lines[i] = fb_alloc(img->w * sizeof(uint16_t));
            state.out_lines[i] = fb_alloc(img->w * sizeof(uint16_t));
        }

        state.lines_processed = 0;

        imlib_image_operation(img, path, other, scalar, imlib_remove_shadows_line_op, &state);

        for (int i = 0; i < imlib_remove_shadows_kernel_size; i++) {
            fb_free();
            fb_free();
            fb_free();
        }
    } else {

        // Create Shadow Mask

        image_t temp_image;
        temp_image.w = img->w;
        temp_image.h = img->h;
        temp_image.bpp = img->bpp;
        temp_image.data = fb_alloc(image_size(img));

        memcpy(temp_image.data, img->data, image_size(img));

        rectangle_t r;
        r.x = 0;
        r.y = 0;
        r.w = temp_image.w;
        r.h = temp_image.h;

        histogram_t h;
        h.LBinCount = COLOR_L_MAX - COLOR_L_MIN + 1;
        h.ABinCount = COLOR_A_MAX - COLOR_A_MIN + 1;
        h.BBinCount = COLOR_B_MAX - COLOR_B_MIN + 1;
        h.LBins = fb_alloc(h.LBinCount * sizeof(float));
        h.ABins = fb_alloc(h.ABinCount * sizeof(float));
        h.BBins = fb_alloc(h.BBinCount * sizeof(float));
        imlib_get_histogram(&h, &temp_image, &r, NULL, false);

        statistics_t s;
        imlib_get_statistics(&s, temp_image.bpp, &h);

        int sum = 0;
        int mean = s.LMean * 0.8f;

        for (int y = 0, yy = temp_image.h; y < yy; y++) {
            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp_image, y);

            for (int x = 0, xx = temp_image.w; x < xx; x++) {
                sum += COLOR_RGB565_TO_L(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)) < mean;
            }
        }

        if (sum > ((temp_image.w * temp_image.h) / 20)) { // Don't do anything if the image is mostly flat.

            threshold_t t;
            imlib_get_threshold(&t, temp_image.bpp, &h);

            list_t thresholds;
            list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
            color_thresholds_list_lnk_data_t lnk_data;
            lnk_data.LMin = COLOR_L_MIN;
            lnk_data.AMin = COLOR_A_MIN;
            lnk_data.BMin = COLOR_B_MIN;
            lnk_data.LMax = t.LValue;
            lnk_data.AMax = COLOR_A_MAX;
            lnk_data.BMax = COLOR_B_MAX;
            list_push_back(&thresholds, &lnk_data);
            imlib_binary(&temp_image, &temp_image, &thresholds, false, false, NULL);
            list_free(&thresholds);

            imlib_erode(&temp_image, 3, 30, NULL);
            imlib_dilate(&temp_image, 1, 1, NULL);

            // Get Shadow Average

            image_t temp_image_2;
            temp_image_2.w = temp_image.w;
            temp_image_2.h = temp_image.h;
            temp_image_2.bpp = temp_image.bpp;
            temp_image_2.data = fb_alloc(image_size(&temp_image));

            memcpy(temp_image_2.data, temp_image.data, image_size(&temp_image));
            imlib_erode(&temp_image_2, 3, 48, NULL);

            int shadow_r_sum = 0;
            int shadow_g_sum = 0;
            int shadow_b_sum = 0;
            int shadow_count = 0;

            for (int y = 0, yy = temp_image_2.h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp_image_2, y);
                uint16_t *row_ptr_2 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);

                for (int x = 0, xx = temp_image_2.w; x < xx; x++) {
                    if (IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)) {
                        int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr_2, x);
                        int r = COLOR_RGB565_TO_R8(pixel);
                        int g = COLOR_RGB565_TO_G8(pixel);
                        int b = COLOR_RGB565_TO_R8(pixel);
                        shadow_r_sum += r;
                        shadow_g_sum += g;
                        shadow_b_sum += b;
                        shadow_count += 1;
                    }
                }
            }

            memcpy(temp_image_2.data, temp_image.data, image_size(&temp_image));
            imlib_invert(&temp_image_2);
            imlib_erode(&temp_image_2, 5, 120, NULL);
            imlib_invert(&temp_image_2);
            imlib_b_xor(&temp_image_2, NULL, &temp_image, 0, NULL);
            imlib_erode(&temp_image_2, 2, 24, NULL);

            int not_shadow_r_sum = 0;
            int not_shadow_g_sum = 0;
            int not_shadow_b_sum = 0;
            int not_shadow_count = 0;

            for (int y = 0, yy = temp_image_2.h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp_image_2, y);
                uint16_t *row_ptr_2 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);

                for (int x = 0, xx = temp_image_2.w; x < xx; x++) {
                    if (IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)) {
                        int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr_2, x);
                        int r = COLOR_RGB565_TO_R8(pixel);
                        int g = COLOR_RGB565_TO_G8(pixel);
                        int b = COLOR_RGB565_TO_R8(pixel);
                        not_shadow_r_sum += r;
                        not_shadow_g_sum += g;
                        not_shadow_b_sum += b;
                        not_shadow_count += 1;
                    }
                }
            }

            // Fill in the umbra... (inner part of the shadow)...

            memcpy(temp_image_2.data, temp_image.data, image_size(&temp_image));

            imlib_mean_filter(&temp_image, 2, false, 0, false, NULL);

            if (shadow_count && not_shadow_count) {

                float shadow_r_average = ((float) shadow_r_sum) / ((float) shadow_count);
                float shadow_g_average = ((float) shadow_g_sum) / ((float) shadow_count);
                float shadow_b_average = ((float) shadow_b_sum) / ((float) shadow_count);

                float not_shadow_r_average = ((float) not_shadow_r_sum) / ((float) not_shadow_count);
                float not_shadow_g_average = ((float) not_shadow_g_sum) / ((float) not_shadow_count);
                float not_shadow_b_average = ((float) not_shadow_b_sum) / ((float) not_shadow_count);

                float diff_r = not_shadow_r_average - shadow_r_average;
                float diff_g = not_shadow_g_average - shadow_g_average;
                float diff_b = not_shadow_b_average - shadow_b_average;

                for (int y = 0; y < img->h; y++) {
                    uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&temp_image, y);
                    uint16_t *row_ptr_2 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);

                    for (int x = 0; x < img->w; x++) {
                        float alpha = ((float) (COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)) - COLOR_Y_MIN)) / ((float) (COLOR_Y_MAX - COLOR_Y_MIN));
                        int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr_2, x);
                        int r = COLOR_RGB565_TO_R8(pixel);
                        int g = COLOR_RGB565_TO_G8(pixel);
                        int b = COLOR_RGB565_TO_B8(pixel);

                        int r_new = IM_MIN(IM_MAX(r + (diff_r * alpha), COLOR_R8_MIN), COLOR_R8_MAX);
                        int g_new = IM_MIN(IM_MAX(g + (diff_g * alpha), COLOR_G8_MIN), COLOR_G8_MAX);
                        int b_new = IM_MIN(IM_MAX(b + (diff_b * alpha), COLOR_B8_MIN), COLOR_B8_MAX);
                        IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr_2, x, COLOR_R8_G8_B8_TO_RGB565(r_new, g_new, b_new));
                    }
                }
            }

            // Fill in the penumbra... (outer part of the shadow)...

            memcpy(temp_image.data, temp_image_2.data, image_size(&temp_image_2));

            imlib_erode(&temp_image_2, 1, 8, NULL);
            imlib_b_xor(&temp_image, NULL, &temp_image_2, 0, NULL);
            imlib_dilate(&temp_image, 3, 0, NULL);
            imlib_median_filter(img, 2, 12, false, 0, false, &temp_image);

            fb_free(); // temp_image_2
        }

        fb_free(); // BBins
        fb_free(); // ABins
        fb_free(); // LBins

        fb_free(); // temp_image
    }
}

#undef imlib_remove_shadows_kernel_size
#undef imlib_remove_shadows_kernel_rank
#endif //IMLIB_ENABLE_REMOVE_SHADOWS

#ifdef IMLIB_ENABLE_CHROMINVAR
extern const float xyz_table[256];

void imlib_chrominvar(image_t *img)
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
                    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
                    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

                    float lin_sum = r_lin + g_lin + b_lin;

                    float r_lin_div = 0.0f;
                    float g_lin_div = 0.0f;
                    float b_lin_div = 0.0f;

                    if (lin_sum > 0.0f) {
                        lin_sum = 1.0f / lin_sum;
                        r_lin_div = r_lin * lin_sum;
                        g_lin_div = g_lin * lin_sum;
                        b_lin_div = b_lin * lin_sum;
                    }

                    int r_lin_div_int = IM_MAX(IM_MIN(r_lin_div * 255.0f, COLOR_R8_MAX), COLOR_R8_MIN);
                    int g_lin_div_int = IM_MAX(IM_MIN(g_lin_div * 255.0f, COLOR_G8_MAX), COLOR_G8_MIN);
                    int b_lin_div_int = IM_MAX(IM_MIN(b_lin_div * 255.0f, COLOR_B8_MAX), COLOR_B8_MIN);

                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, COLOR_R8_G8_B8_TO_RGB565(r_lin_div_int, g_lin_div_int, b_lin_div_int));
                }
            }

            break;
        }
        default: {
            break;
        }
    }
}
#endif //IMLIB_ENABLE_CHROMINVAR

#ifdef IMLIB_ENABLE_ILLUMINVAR
extern const uint16_t invariant_table[65536];

void imlib_illuminvar(image_t *img) // http://ai.stanford.edu/~alireza/publication/cic15.pdf
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
#ifdef IMLIB_ENABLE_INVARIANT_TABLE
                    int rgb565 = invariant_table[pixel];
#else
                    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)] + 1.0;
                    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)] + 1.0;
                    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)] + 1.0;

                    float r_lin_sharp = (r_lin *  0.9968f) + (g_lin *  0.0228f) + (b_lin * 0.0015f);
                    float g_lin_sharp = (r_lin * -0.0071f) + (g_lin *  0.9933f) + (b_lin * 0.0146f);
                    float b_lin_sharp = (r_lin *  0.0103f) + (g_lin * -0.0161f) + (b_lin * 0.9839f);

                    float lin_sharp_avg = r_lin_sharp * g_lin_sharp * b_lin_sharp;
                    lin_sharp_avg = (lin_sharp_avg > 0.0f) ? fast_cbrtf(lin_sharp_avg) : 0.0f;

                    float r_lin_sharp_div = 0.0f;
                    float g_lin_sharp_div = 0.0f;
                    float b_lin_sharp_div = 0.0f;

                    if (lin_sharp_avg > 0.0f) {
                        lin_sharp_avg = 1.0f / lin_sharp_avg;
                        r_lin_sharp_div = r_lin_sharp * lin_sharp_avg;
                        g_lin_sharp_div = g_lin_sharp * lin_sharp_avg;
                        b_lin_sharp_div = b_lin_sharp * lin_sharp_avg;
                    }

                    float r_lin_sharp_div_log = (r_lin_sharp_div > 0.0f) ? fast_log(r_lin_sharp_div) : 0.0f;
                    float g_lin_sharp_div_log = (g_lin_sharp_div > 0.0f) ? fast_log(g_lin_sharp_div) : 0.0f;
                    float b_lin_sharp_div_log = (b_lin_sharp_div > 0.0f) ? fast_log(b_lin_sharp_div) : 0.0f;

                    float chi_x = (r_lin_sharp_div_log * 0.7071f) + (g_lin_sharp_div_log * -0.7071f) + (b_lin_sharp_div_log *  0.0000f);
                    float chi_y = (r_lin_sharp_div_log * 0.4082f) + (g_lin_sharp_div_log *  0.4082f) + (b_lin_sharp_div_log * -0.8164f);

                    float e_t_x =  0.9326f;
                    float e_t_y = -0.3609f;

                    float p_th_00 = e_t_x * e_t_x;
                    float p_th_01 = e_t_x * e_t_y;
                    float p_th_10 = e_t_y * e_t_x;
                    float p_th_11 = e_t_y * e_t_y;

                    float x_th_x = (p_th_00 * chi_x) + (p_th_01 * chi_y);
                    float x_th_y = (p_th_10 * chi_x) + (p_th_11 * chi_y);

                    float r_chi = (x_th_x *  0.7071f) + (x_th_y *  0.4082f);
                    float g_chi = (x_th_x * -0.7071f) + (x_th_y *  0.4082f);
                    float b_chi = (x_th_x *  0.0000f) + (x_th_y * -0.8164f);

                    float r_chi_invariant = fast_expf(r_chi);
                    float g_chi_invariant = fast_expf(g_chi);
                    float b_chi_invariant = fast_expf(b_chi);

                    float chi_invariant_sum = r_chi_invariant + g_chi_invariant + b_chi_invariant;

                    float r_chi_invariant_m = 0.0f;
                    float g_chi_invariant_m = 0.0f;
                    float b_chi_invariant_m = 0.0f;

                    if (chi_invariant_sum > 0.0f) {
                        chi_invariant_sum = 1.0f / chi_invariant_sum;
                        r_chi_invariant_m = r_chi_invariant * chi_invariant_sum;
                        g_chi_invariant_m = g_chi_invariant * chi_invariant_sum;
                        b_chi_invariant_m = b_chi_invariant * chi_invariant_sum;
                    }

                    int r_chi_invariant_m_int = IM_MAX(IM_MIN(r_chi_invariant_m * 255.0f, COLOR_R8_MAX), COLOR_R8_MIN);
                    int g_chi_invariant_m_int = IM_MAX(IM_MIN(g_chi_invariant_m * 255.0f, COLOR_G8_MAX), COLOR_G8_MIN);
                    int b_chi_invariant_m_int = IM_MAX(IM_MIN(b_chi_invariant_m * 255.0f, COLOR_B8_MAX), COLOR_B8_MIN);

                    int rgb565 = COLOR_R8_G8_B8_TO_RGB565(r_chi_invariant_m_int, g_chi_invariant_m_int, b_chi_invariant_m_int);
#endif
                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, rgb565);
                }
            }

            break;
        }
        default: {
            break;
        }
    }
}
#endif //IMLIB_ENABLE_ILLUMINVAR
