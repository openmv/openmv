/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "imlib.h"

typedef struct imlib_similatiry_line_op_state {
    int *sumBucketsOfX, *sumBucketsOfY, *sum2BucketsOfX, *sum2BucketsOfY, *sum2Buckets;
    float similarity_sum, similarity_sum_2, similarity_min, similarity_max;
    int lines_processed;
} imlib_similatiry_line_op_state_t;

void imlib_similarity_line_op(image_t *img, int line, uint8_t *other, void *data)
{
    imlib_similatiry_line_op_state_t *state = (imlib_similatiry_line_op_state_t *) data;
    float c1 = 0, c2 = 0;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, line);
            uint32_t *other_row_ptr = (uint32_t *) other;
            for (int x = 0, xx = (img->w + 7) / 8; x < xx; x++) {
                for (int i = 0, ii = IM_MIN((img->w - (x * 8)), 8); i < ii; i++) {
                    int pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x + i);
                    int other_pixel = IMAGE_GET_BINARY_PIXEL_FAST(other_row_ptr, x + i);
                    state->sumBucketsOfX[x] += pixel;
                    state->sumBucketsOfY[x] += other_pixel;
                    state->sum2BucketsOfX[x] += pixel * pixel;
                    state->sum2BucketsOfY[x] += other_pixel * other_pixel;
                    state->sum2Buckets[x] += pixel * other_pixel;
                }
            }
            c1 = COLOR_BINARY_MAX * 0.01f;
            c2 = COLOR_BINARY_MAX * 0.03f;
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, line);
            uint8_t *other_row_ptr = (uint8_t *) other;
            for (int x = 0, xx = (img->w + 7) / 8; x < xx; x++) {
                for (int i = 0, ii = IM_MIN((img->w - (x * 8)), 8); i < ii; i++) {
                    int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x + i);
                    int other_pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(other_row_ptr, x + i);
                    state->sumBucketsOfX[x] += pixel;
                    state->sumBucketsOfY[x] += other_pixel;
                    state->sum2BucketsOfX[x] += pixel * pixel;
                    state->sum2BucketsOfY[x] += other_pixel * other_pixel;
                    state->sum2Buckets[x] += pixel * other_pixel;
                }
            }
            c1 = COLOR_GRAYSCALE_MAX * 0.01f;
            c2 = COLOR_GRAYSCALE_MAX * 0.03f;
            break;
        }
        case IMAGE_BPP_RGB565: {
            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, line);
            uint16_t *other_row_ptr = (uint16_t *) other;
            for (int x = 0, xx = (img->w + 7) / 8; x < xx; x++) {
                for (int i = 0, ii = IM_MIN((img->w - (x * 8)), 8); i < ii; i++) {
                    int pixel = COLOR_RGB565_TO_L(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x + i));
                    int other_pixel = COLOR_RGB565_TO_L(IMAGE_GET_RGB565_PIXEL_FAST(other_row_ptr, x + i));
                    state->sumBucketsOfX[x] += pixel;
                    state->sumBucketsOfY[x] += other_pixel;
                    state->sum2BucketsOfX[x] += pixel * pixel;
                    state->sum2BucketsOfY[x] += other_pixel * other_pixel;
                    state->sum2Buckets[x] += pixel * other_pixel;
                }
            }
            c1 = COLOR_L_MAX * 0.01f;
            c2 = COLOR_L_MAX * 0.03f;
            break;
        }
        default: {
            break;
        }
    }

    // https://en.wikipedia.org/wiki/Structural_similarity
    if (((state->lines_processed + 1) == img->h) || (!((state->lines_processed + 1) % 8))) {
        for (int x = 0, xx = (img->w + 7) / 8; x < xx; x++) {
            int w = IM_MIN((img->w - (x * 8)), 8);
            int h = IM_MIN((img->h - ((state->lines_processed / 8) * 8)), 8);
            int size = w * h;

            int mx = state->sumBucketsOfX[x] / size;
            int my = state->sumBucketsOfY[x] / size;
            int vx = state->sum2BucketsOfX[x] - ((mx * state->sumBucketsOfX[x]) + (mx * state->sumBucketsOfX[x])) + (size * mx * mx);
            int vy = state->sum2BucketsOfY[x] - ((my * state->sumBucketsOfY[x]) + (my * state->sumBucketsOfY[x])) + (size * my * my);
            int vxy = state->sum2Buckets[x] - ((mx * state->sumBucketsOfY[x]) + (my * state->sumBucketsOfX[x])) + (size * mx * my);

            float ssim = ( ((2*mx*my) + c1) * ((2*vxy) + c2) ) / ( ((mx*mx) + (my*my) + c1) * (vx + vy + c2) );

            state->similarity_sum += ssim;
            state->similarity_sum_2 += ssim * ssim;
            state->similarity_min = IM_MIN(state->similarity_min, ssim);
            state->similarity_max = IM_MAX(state->similarity_max, ssim);

            state->sumBucketsOfX[x] = 0;
            state->sumBucketsOfY[x] = 0;
            state->sum2BucketsOfX[x] = 0;
            state->sum2BucketsOfY[x] = 0;
            state->sum2Buckets[x] = 0;
        }
    }

    state->lines_processed += 1;
}

void imlib_get_similarity(image_t *img, const char *path, image_t *other, float *avg, float *std, float *min, float *max)
{
    int h_blocks = (img->w + 7) / 8;
    int v_blocks = (img->h + 7) / 8;
    int blocks = h_blocks * v_blocks;

    int int_h_blocks = h_blocks * sizeof(int);
    imlib_similatiry_line_op_state_t state;
    state.sumBucketsOfX = fb_alloc0(int_h_blocks);
    state.sumBucketsOfY = fb_alloc0(int_h_blocks);
    state.sum2BucketsOfX = fb_alloc0(int_h_blocks);
    state.sum2BucketsOfY = fb_alloc0(int_h_blocks);
    state.sum2Buckets = fb_alloc0(int_h_blocks);
    state.similarity_sum = 0.0f;
    state.similarity_sum_2 = 0.0f;
    state.similarity_min = FLT_MAX;
    state.similarity_max = FLT_MIN;
    state.lines_processed = 0;

    imlib_image_operation(img, path, other, imlib_similarity_line_op, (void *) &state);
    *avg = state.similarity_sum / blocks;
    *std = fast_sqrtf((state.similarity_sum_2 / blocks) - ((*avg) * (*avg)));
    *min = state.similarity_min;
    *max = state.similarity_max;

    fb_free();
    fb_free();
    fb_free();
    fb_free();
    fb_free();
}

void imlib_get_histogram(histogram_t *out, image_t *ptr, rectangle_t *roi)
{
    switch(ptr->bpp) {
        case IMAGE_BPP_BINARY: {
            memset(out->LBins, 0, out->LBinCount * sizeof(uint32_t));

            float mult = (out->LBinCount - 1) / ((float) (COLOR_BINARY_MAX - COLOR_BINARY_MIN));

            for (int y = roi->y, yy = roi->y + roi->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                for (int x = roi->x, xx = roi->x + roi->w; x < xx; x++) {
                    int pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                    ((uint32_t *) out->LBins)[fast_roundf((pixel - COLOR_BINARY_MIN) * mult)]++;
                }
            }

            float pixels = 1 / ((float) (roi->w * roi->h));

            for (int i = 0, j = out->LBinCount; i < j; i++) {
                out->LBins[i] = ((uint32_t *) out->LBins)[i] * pixels;
            }

            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            memset(out->LBins, 0, out->LBinCount * sizeof(uint32_t));

            float mult = (out->LBinCount - 1) / ((float) (COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN));

            for (int y = roi->y, yy = roi->y + roi->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                for (int x = roi->x, xx = roi->x + roi->w; x < xx; x++) {
                    int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    ((uint32_t *) out->LBins)[fast_roundf((pixel - COLOR_GRAYSCALE_MIN) * mult)]++;
                }
            }

            float pixels = 1 / ((float) (roi->w * roi->h));

            for (int i = 0, j = out->LBinCount; i < j; i++) {
                out->LBins[i] = ((uint32_t *) out->LBins)[i] * pixels;
            }

            break;
        }
        case IMAGE_BPP_RGB565: {
            memset(out->LBins, 0, out->LBinCount * sizeof(uint32_t));
            memset(out->ABins, 0, out->ABinCount * sizeof(uint32_t));
            memset(out->BBins, 0, out->BBinCount * sizeof(uint32_t));

            float l_mult = (out->LBinCount - 1) / ((float) (COLOR_L_MAX - COLOR_L_MIN));
            float a_mult = (out->ABinCount - 1) / ((float) (COLOR_A_MAX - COLOR_A_MIN));
            float b_mult = (out->BBinCount - 1) / ((float) (COLOR_B_MAX - COLOR_B_MIN));

            for (int y = roi->y, yy = roi->y + roi->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                for (int x = roi->x, xx = roi->x + roi->w; x < xx; x++) {
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    ((uint32_t *) out->LBins)[fast_roundf((COLOR_RGB565_TO_L(pixel) - COLOR_L_MIN) * l_mult)]++;
                    ((uint32_t *) out->ABins)[fast_roundf((COLOR_RGB565_TO_A(pixel) - COLOR_A_MIN) * a_mult)]++;
                    ((uint32_t *) out->BBins)[fast_roundf((COLOR_RGB565_TO_B(pixel) - COLOR_B_MIN) * b_mult)]++;
                }
            }

            float pixels = 1 / ((float) (roi->w * roi->h));

            for (int i = 0, j = out->LBinCount; i < j; i++) {
                out->LBins[i] = ((uint32_t *) out->LBins)[i] * pixels;
            }

            for (int i = 0, j = out->ABinCount; i < j; i++) {
                out->ABins[i] = ((uint32_t *) out->ABins)[i] * pixels;
            }

            for (int i = 0, j = out->BBinCount; i < j; i++) {
                out->BBins[i] = ((uint32_t *) out->BBins)[i] * pixels;
            }

            break;
        }
        default: {
            break;
        }
    }
}

void imlib_get_percentile(percentile_t *out, image_bpp_t bpp, histogram_t *ptr, float percentile)
{
    memset(out, 0, sizeof(percentile_t));
    switch(bpp) {
        case IMAGE_BPP_BINARY: {
            float mult = (COLOR_BINARY_MAX - COLOR_BINARY_MIN) / ((float) (ptr->LBinCount - 1));
            float median_count = 0;

            for (int i = 0, j = ptr->LBinCount; i < j; i++) {
                if ((median_count < percentile) && (percentile <= (median_count + ptr->LBins[i]))) {
                    out->LValue = fast_roundf((i * mult) + COLOR_BINARY_MIN);
                    break;
                }

                median_count += ptr->LBins[i];
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            float mult = (COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN) / ((float) (ptr->LBinCount - 1));
            float median_count = 0;

            for (int i = 0, j = ptr->LBinCount; i < j; i++) {
                if ((median_count < percentile) && (percentile <= (median_count + ptr->LBins[i]))) {
                    out->LValue = fast_roundf((i * mult) + COLOR_GRAYSCALE_MIN);
                    break;
                }

                median_count += ptr->LBins[i];
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            {
                float mult = (COLOR_L_MAX - COLOR_L_MIN) / ((float) (ptr->LBinCount - 1));
                float median_count = 0;

                for (int i = 0, j = ptr->LBinCount; i < j; i++) {
                    if ((median_count < percentile) && (percentile <= (median_count + ptr->LBins[i]))) {
                        out->LValue = fast_roundf((i * mult) + COLOR_L_MIN);
                        break;
                    }

                    median_count += ptr->LBins[i];
                }
            }
            {
                float mult = (COLOR_A_MAX - COLOR_A_MIN) / ((float) (ptr->ABinCount - 1));
                float median_count = 0;

                for (int i = 0, j = ptr->ABinCount; i < j; i++) {
                    if ((median_count < percentile) && (percentile <= (median_count + ptr->ABins[i]))) {
                        out->AValue = fast_roundf((i * mult) + COLOR_A_MIN);
                        break;
                    }

                    median_count += ptr->ABins[i];
                }
            }
            {
                float mult = (COLOR_B_MAX - COLOR_B_MIN) / ((float) (ptr->BBinCount - 1));
                float median_count = 0;

                for (int i = 0, j = ptr->BBinCount; i < j; i++) {
                    if ((median_count < percentile) && (percentile <= (median_count + ptr->BBins[i]))) {
                        out->BValue = fast_roundf((i * mult) + COLOR_A_MIN);
                        break;
                    }

                    median_count += ptr->BBins[i];
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_get_statistics(statistics_t *out, image_bpp_t bpp, histogram_t *ptr)
{
    memset(out, 0, sizeof(statistics_t));
    switch(bpp) {
        case IMAGE_BPP_BINARY: {
            float mult = (COLOR_BINARY_MAX - COLOR_BINARY_MIN) / ((float) (ptr->LBinCount - 1));

            float avg = 0;
            float stdev = 0;
            float median_count = 0;
            float mode_count = 0;
            bool min_flag = false;

            for (int i = 0, j = ptr->LBinCount; i < j; i++) {
                float value_f = (i * mult) + COLOR_BINARY_MIN;
                int value = fast_roundf(value_f);

                avg += value_f * ptr->LBins[i];
                stdev += value_f * value_f * ptr->LBins[i];

                if ((median_count < 0.25f) && (0.25f <= (median_count + ptr->LBins[i]))) {
                    out->LLQ = value;
                }

                if ((median_count < 0.5f) && (0.5f <= (median_count + ptr->LBins[i]))) {
                    out->LMedian = value;
                }

                if ((median_count < 0.75f) && (0.75f <= (median_count + ptr->LBins[i]))) {
                    out->LUQ = value;
                }

                if (ptr->LBins[i] > mode_count) {
                    mode_count = ptr->LBins[i];
                    out->LMode = value;
                }

                if ((ptr->LBins[i] > 0.0f) && (!min_flag)) {
                    min_flag = true;
                    out->LMin = value;
                }

                if (ptr->LBins[i] > 0.0f) {
                    out->LMax = value;
                }

                median_count += ptr->LBins[i];
            }

            out->LMean = fast_roundf(avg);
            out->LSTDev = fast_roundf(fast_sqrtf(stdev - (avg * avg)));
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            float mult = (COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN) / ((float) (ptr->LBinCount - 1));

            float avg = 0;
            float stdev = 0;
            float median_count = 0;
            float mode_count = 0;
            bool min_flag = false;

            for (int i = 0, j = ptr->LBinCount; i < j; i++) {
                float value_f = (i * mult) + COLOR_GRAYSCALE_MIN;
                int value = fast_roundf(value_f);

                avg += value_f * ptr->LBins[i];
                stdev += value_f * value_f * ptr->LBins[i];

                if ((median_count < 0.25f) && (0.25f <= (median_count + ptr->LBins[i]))) {
                    out->LLQ = value;
                }

                if ((median_count < 0.5f) && (0.5f <= (median_count + ptr->LBins[i]))) {
                    out->LMedian = value;
                }

                if ((median_count < 0.75f) && (0.75f <= (median_count + ptr->LBins[i]))) {
                    out->LUQ = value;
                }

                if (ptr->LBins[i] > mode_count) {
                    mode_count = ptr->LBins[i];
                    out->LMode = value;
                }

                if ((ptr->LBins[i] > 0.0f) && (!min_flag)) {
                    min_flag = true;
                    out->LMin = value;
                }

                if (ptr->LBins[i] > 0.0f) {
                    out->LMax = value;
                }

                median_count += ptr->LBins[i];
            }

            out->LMean = fast_roundf(avg);
            out->LSTDev = fast_roundf(fast_sqrtf(stdev - (avg * avg)));
            break;
        }
        case IMAGE_BPP_RGB565: {
            {
                float mult = (COLOR_L_MAX - COLOR_L_MIN) / ((float) (ptr->LBinCount - 1));

                float avg = 0;
                float stdev = 0;
                float median_count = 0;
                float mode_count = 0;
                bool min_flag = false;

                for (int i = 0, j = ptr->LBinCount; i < j; i++) {
                    float value_f = (i * mult) + COLOR_L_MIN;
                    int value = fast_roundf(value_f);

                    avg += value_f * ptr->LBins[i];
                    stdev += value_f * value_f * ptr->LBins[i];

                    if ((median_count < 0.25f) && (0.25f <= (median_count + ptr->LBins[i]))) {
                        out->LLQ = value;
                    }

                    if ((median_count < 0.5f) && (0.5f <= (median_count + ptr->LBins[i]))) {
                        out->LMedian = value;
                    }

                    if ((median_count < 0.75f) && (0.75f <= (median_count + ptr->LBins[i]))) {
                        out->LUQ = value;
                    }

                    if (ptr->LBins[i] > mode_count) {
                        mode_count = ptr->LBins[i];
                        out->LMode = value;
                    }

                    if ((ptr->LBins[i] > 0.0f) && (!min_flag)) {
                        min_flag = true;
                        out->LMin = value;
                    }

                    if (ptr->LBins[i] > 0.0f) {
                        out->LMax = value;
                    }

                    median_count += ptr->LBins[i];
                }

                out->LMean = fast_roundf(avg);
                out->LSTDev = fast_roundf(fast_sqrtf(stdev - (avg * avg)));
            }
            {
                float mult = (COLOR_A_MAX - COLOR_A_MIN) / ((float) (ptr->ABinCount - 1));

                float avg = 0;
                float stdev = 0;
                float median_count = 0;
                float mode_count = 0;
                bool min_flag = false;

                for (int i = 0, j = ptr->ABinCount; i < j; i++) {
                    float value_f = (i * mult) + COLOR_A_MIN;
                    int value = fast_roundf(value_f);

                    avg += value_f * ptr->ABins[i];
                    stdev += value_f * value_f * ptr->ABins[i];

                    if ((median_count < 0.25f) && (0.25f <= (median_count + ptr->ABins[i]))) {
                        out->ALQ = value;
                    }

                    if ((median_count < 0.5f) && (0.5f <= (median_count + ptr->ABins[i]))) {
                        out->AMedian = value;
                    }

                    if ((median_count < 0.75f) && (0.75f <= (median_count + ptr->ABins[i]))) {
                        out->AUQ = value;
                    }

                    if (ptr->ABins[i] > mode_count) {
                        mode_count = ptr->ABins[i];
                        out->AMode = value;
                    }

                    if ((ptr->ABins[i] > 0.0f) && (!min_flag)) {
                        min_flag = true;
                        out->AMin = value;
                    }

                    if (ptr->ABins[i] > 0.0f) {
                        out->AMax = value;
                    }

                    median_count += ptr->ABins[i];
                }

                out->AMean = fast_roundf(avg);
                out->ASTDev = fast_roundf(fast_sqrtf(stdev - (avg * avg)));
            }
            {
                float mult = (COLOR_B_MAX - COLOR_B_MIN) / ((float) (ptr->BBinCount - 1));

                float avg = 0;
                float stdev = 0;
                float median_count = 0;
                float mode_count = 0;
                bool min_flag = false;

                for (int i = 0, j = ptr->BBinCount; i < j; i++) {
                    float value_f = (i * mult) + COLOR_B_MIN;
                    int value = fast_roundf(value_f);

                    avg += value_f * ptr->BBins[i];
                    stdev += value_f * value_f * ptr->BBins[i];

                    if ((median_count < 0.25f) && (0.25f <= (median_count + ptr->BBins[i]))) {
                        out->BLQ = value;
                    }

                    if ((median_count < 0.5f) && (0.5f <= (median_count + ptr->BBins[i]))) {
                        out->BMedian = value;
                    }

                    if ((median_count < 0.75f) && (0.75f <= (median_count + ptr->BBins[i]))) {
                        out->BUQ = value;
                    }

                    if (ptr->BBins[i] > mode_count) {
                        mode_count = ptr->BBins[i];
                        out->BMode = value;
                    }

                    if ((ptr->BBins[i] > 0.0f) && (!min_flag)) {
                        min_flag = true;
                        out->BMin = value;
                    }

                    if (ptr->BBins[i] > 0.0f) {
                        out->BMax = value;
                    }

                    median_count += ptr->BBins[i];
                }

                out->BMean = fast_roundf(avg);
                out->BSTDev = fast_roundf(fast_sqrtf(stdev - (avg * avg)));
            }
            break;
        }
        default: {
            break;
        }
    }
}

static int get_median(int *array, int array_sum, int array_len)
{
    const int median_threshold = (array_sum + 1) / 2;
    int median_count = 0;

    for (int i = 0; i < array_len; i++) {
        if ((median_count < median_threshold) && (median_threshold <= (median_count + array[i]))) return i;
        median_count += array[i];
    }

    return array_len - 1;
}

static int get_median_l(long long *array, long long array_sum, int array_len)
{
    const long long median_threshold = (array_sum + 1) / 2;
    long long median_count = 0;

    for (int i = 0; i < array_len; i++) {
        if ((median_count < median_threshold) && (median_threshold <= (median_count + array[i]))) return i;
        median_count += array[i];
    }

    return array_len - 1;
}

bool imlib_get_regression(find_lines_list_lnk_data_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                          list_t *thresholds, bool invert, bool robust)
{
    bool result = false;
    memset(out, 0, sizeof(find_lines_list_lnk_data_t));

    if (!robust) { // Least Squares
        int blob_pixels = 0;
        int blob_cx = 0;
        int blob_cy = 0;
        long long blob_a = 0;
        long long blob_b = 0;
        long long blob_c = 0;

        for (list_lnk_t *it = iterator_start_from_head(thresholds); it; it = iterator_next(it)) {
            color_thresholds_list_lnk_data_t lnk_data;
            iterator_get(thresholds, it, &lnk_data);

            switch (ptr->bpp) {
                case IMAGE_BPP_BINARY: {
                    for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                        uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                        for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                            if (COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                                blob_pixels += 1;
                                blob_cx += x;
                                blob_cy += y;
                                blob_a += x*x;
                                blob_b += x*y;
                                blob_c += y*y;
                            }
                        }
                    }
                    break;
                }
                case IMAGE_BPP_GRAYSCALE: {
                    for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                        uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                        for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                            if (COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                                blob_pixels += 1;
                                blob_cx += x;
                                blob_cy += y;
                                blob_a += x*x;
                                blob_b += x*y;
                                blob_c += y*y;
                            }
                        }
                    }
                    break;
                }
                case IMAGE_BPP_RGB565: {
                    for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                        uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                        for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                            if (COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                                blob_pixels += 1;
                                blob_cx += x;
                                blob_cy += y;
                                blob_a += x*x;
                                blob_b += x*y;
                                blob_c += y*y;
                            }
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        if (blob_pixels) {
            // http://www.cse.usf.edu/~r1k/MachineVisionBook/MachineVision.files/MachineVision_Chapter2.pdf
            // https://www.strchr.com/standard_deviation_in_one_pass
            //
            // a = sigma(x*x) + (mx*sigma(x)) + (mx*sigma(x)) + (sigma()*mx*mx)
            // b = sigma(x*y) + (mx*sigma(y)) + (my*sigma(x)) + (sigma()*mx*my)
            // c = sigma(y*y) + (my*sigma(y)) + (my*sigma(y)) + (sigma()*my*my)
            //
            // blob_a = sigma(x*x)
            // blob_b = sigma(x*y)
            // blob_c = sigma(y*y)
            // blob_cx = sigma(x)
            // blob_cy = sigma(y)
            // blob_pixels = sigma()

            int mx = blob_cx / blob_pixels; // x centroid
            int my = blob_cy / blob_pixels; // y centroid
            int small_blob_a = blob_a - ((mx * blob_cx) + (mx * blob_cx)) + (blob_pixels * mx * mx);
            int small_blob_b = blob_b - ((mx * blob_cy) + (my * blob_cx)) + (blob_pixels * mx * my);
            int small_blob_c = blob_c - ((my * blob_cy) + (my * blob_cy)) + (blob_pixels * my * my);

            float rotation = ((small_blob_a != small_blob_c) ? (fast_atan2f(2 * small_blob_b, small_blob_a - small_blob_c) / 2.0f) : 1.570796f) + 1.570796f; // PI/2

            out->theta = fast_roundf(rotation * 57.295780) % 180; // * (180 / PI)
            if (out->theta < 0) out->theta += 180;
            out->rho = fast_roundf(((mx - roi->x) * cos_table[out->theta]) + ((my - roi->y) * sin_table[out->theta]));

            float part0 = (small_blob_a + small_blob_c) / 2.0f;
            float f_b = (float) small_blob_b;
            float f_a_c = (float) (small_blob_a - small_blob_c);
            float part1 = fast_sqrtf((4 * f_b * f_b) + (f_a_c * f_a_c)) / 2.0f;
            float p_add = fast_sqrtf(part0 + part1);
            float p_sub = fast_sqrtf(part0 - part1);
            float e_min = IM_MIN(p_add, p_sub);
            float e_max = IM_MAX(p_add, p_sub);
            out->magnitude = fast_roundf(e_max / e_min) - 1; // Circle -> [0, INF) -> Line

            if ((45 <= out->theta) && (out->theta < 135)) {
                // y = (r - x cos(t)) / sin(t)
                out->line.x1 = 0;
                out->line.y1 = fast_roundf((out->rho - (out->line.x1 * cos_table[out->theta])) / sin_table[out->theta]);
                out->line.x2 = roi->w - 1;
                out->line.y2 = fast_roundf((out->rho - (out->line.x2 * cos_table[out->theta])) / sin_table[out->theta]);
            } else {
                // x = (r - y sin(t)) / cos(t);
                out->line.y1 = 0;
                out->line.x1 = fast_roundf((out->rho - (out->line.y1 * sin_table[out->theta])) / cos_table[out->theta]);
                out->line.y2 = roi->h - 1;
                out->line.x2 = fast_roundf((out->rho - (out->line.y2 * sin_table[out->theta])) / cos_table[out->theta]);
            }

            if(lb_clip_line(&out->line, 0, 0, roi->w, roi->h)) {
                out->line.x1 += roi->x;
                out->line.y1 += roi->y;
                out->line.x2 += roi->x;
                out->line.y2 += roi->y;
                // Move rho too.
                out->rho += fast_roundf((roi->x * cos_table[out->theta]) + (roi->y * sin_table[out->theta]));
                result = true;
            } else {
                memset(out, 0, sizeof(find_lines_list_lnk_data_t));
            }
        }
    } else { // Theil-Sen Estimator
        int *x_histogram = fb_alloc0(ptr->w * sizeof(int)); // Not roi so we don't have to adjust, we can burn the RAM.
        int *y_histogram = fb_alloc0(ptr->h * sizeof(int)); // Not roi so we don't have to adjust, we can burn the RAM.

        long long *x_delta_histogram = fb_alloc0((2 * ptr->w) * sizeof(long long)); // Not roi so we don't have to adjust, we can burn the RAM.
        long long *y_delta_histogram = fb_alloc0((2 * ptr->h) * sizeof(long long)); // Not roi so we don't have to adjust, we can burn the RAM.

        uint32_t size;
        point_t *points = (point_t *) fb_alloc_all(&size);
        size_t points_max = size / sizeof(point_t);
        size_t points_count = 0;

        if(points_max) {
            int blob_pixels = 0;

            for (list_lnk_t *it = iterator_start_from_head(thresholds); it; it = iterator_next(it)) {
                color_thresholds_list_lnk_data_t lnk_data;
                iterator_get(thresholds, it, &lnk_data);

                switch (ptr->bpp) {
                    case IMAGE_BPP_BINARY: {
                        for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                            for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                                if (COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                                    blob_pixels += 1;
                                    x_histogram[x]++;
                                    y_histogram[y]++;

                                    if(points_count < points_max) {
                                        point_init(&points[points_count], x, y);
                                        points_count += 1;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case IMAGE_BPP_GRAYSCALE: {
                        for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                            for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                                if (COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                                    blob_pixels += 1;
                                    x_histogram[x]++;
                                    y_histogram[y]++;

                                    if(points_count < points_max) {
                                        point_init(&points[points_count], x, y);
                                        points_count += 1;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case IMAGE_BPP_RGB565: {
                        for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                            for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                                if (COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                                    blob_pixels += 1;
                                    x_histogram[x]++;
                                    y_histogram[y]++;

                                    if(points_count < points_max) {
                                        point_init(&points[points_count], x, y);
                                        points_count += 1;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            if (blob_pixels) {
                long long delta_sum = (points_count * (points_count - 1)) / 2;

                if (delta_sum) {
                    // The code below computes the average slope between all pairs of points.
                    // This is a N^2 operation that can easily blow up if the image is not threshold carefully...

                    for(int i = 0; i < points_count; i++) {
                        point_t *p0 = &points[i];
                        for(int j = i + 1; j < points_count; j++) {
                            point_t *p1 = &points[j];
                            x_delta_histogram[p0->x - p1->x + ptr->w]++; // Note we allocated 1 extra above so we can do ptr->w instead of (ptr->w-1).
                            y_delta_histogram[p0->y - p1->y + ptr->h]++; // Note we allocated 1 extra above so we can do ptr->h instead of (ptr->h-1).
                        }
                    }

                    int mx = get_median(x_histogram, blob_pixels, ptr->w); // Output doesn't need adjustment.
                    int my = get_median(y_histogram, blob_pixels, ptr->h); // Output doesn't need adjustment.
                    int mdx = get_median_l(x_delta_histogram, delta_sum, 2 * ptr->w) - ptr->w; // Fix offset.
                    int mdy = get_median_l(y_delta_histogram, delta_sum, 2 * ptr->h) - ptr->h; // Fix offset.

                    float rotation = (mdx ? fast_atan2f(mdy, mdx) : 1.570796f) + 1.570796f; // PI/2

                    out->theta = fast_roundf(rotation * 57.295780) % 180; // * (180 / PI)
                    if (out->theta < 0) out->theta += 180;
                    out->rho = fast_roundf(((mx - roi->x) * cos_table[out->theta]) + ((my - roi->y) * sin_table[out->theta]));

                    out->magnitude = fast_roundf(fast_sqrtf((mdx * mdx) + (mdy * mdy)));

                    if ((45 <= out->theta) && (out->theta < 135)) {
                        // y = (r - x cos(t)) / sin(t)
                        out->line.x1 = 0;
                        out->line.y1 = fast_roundf((out->rho - (out->line.x1 * cos_table[out->theta])) / sin_table[out->theta]);
                        out->line.x2 = roi->w - 1;
                        out->line.y2 = fast_roundf((out->rho - (out->line.x2 * cos_table[out->theta])) / sin_table[out->theta]);
                    } else {
                        // x = (r - y sin(t)) / cos(t);
                        out->line.y1 = 0;
                        out->line.x1 = fast_roundf((out->rho - (out->line.y1 * sin_table[out->theta])) / cos_table[out->theta]);
                        out->line.y2 = roi->h - 1;
                        out->line.x2 = fast_roundf((out->rho - (out->line.y2 * sin_table[out->theta])) / cos_table[out->theta]);
                    }

                    if(lb_clip_line(&out->line, 0, 0, roi->w, roi->h)) {
                        out->line.x1 += roi->x;
                        out->line.y1 += roi->y;
                        out->line.x2 += roi->x;
                        out->line.y2 += roi->y;
                        // Move rho too.
                        out->rho += fast_roundf((roi->x * cos_table[out->theta]) + (roi->y * sin_table[out->theta]));
                        result = true;
                    } else {
                        memset(out, 0, sizeof(find_lines_list_lnk_data_t));
                    }
                }
            }
        }

        fb_free(); // points
        fb_free(); // y_delta_histogram
        fb_free(); // x_delta_histogram
        fb_free(); // y_histogram
        fb_free(); // x_histogram
    }

    return result;
}
