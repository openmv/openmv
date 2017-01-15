/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "imlib.h"

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
