/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "imlib.h"

const int hough_divide = 2; // divides theta and rho accumulators

void imlib_find_lines(list_t *out, image_t *ptr, rectangle_t *roi, int x_stride, int y_stride, float threshold, int theta_margin, int rho_margin)
{
    const int r_diag_len = fast_roundf(fast_sqrtf((roi->w * roi->w) + (roi->h * roi->h)));
    const int r_diag_len_div = (r_diag_len + hough_divide - 1) / hough_divide;
    const int theta_size = 1 + ((180 + hough_divide - 1) / hough_divide) + 1; // left & right padding
    const int r_size = (r_diag_len_div * 2) + 1; // -r_diag_len to r_diag_len
    uint32_t *acc = fb_alloc0(sizeof(uint32_t) * theta_size * r_size);

    uint32_t acc_max = 0;
    switch (ptr->bpp) {
        case IMAGE_BPP_BINARY: {
            for (int y = roi->y + 1, yy = roi->y + roi->h - 1; y < yy; y += y_stride) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                for (int x = roi->x + (y % x_stride) + 1, xx = roi->x + roi->w - 1; x < xx; x += x_stride) {
                    int pixel; // Sobel Algorithm Below... w/ Scharr...
                    int x_acc = 0;
                    int y_acc = 0;

                    row_ptr -= roi->w;

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x - 1));
                    x_acc += pixel * -3; // x[0,0] -> pixel * -3
                    y_acc += pixel * -3; // y[0,0] -> pixel * -3

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                                          // x[0,1] -> pixel * 0
                    y_acc += pixel * -10; // y[0,1] -> pixel * -10

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x + 1));
                    x_acc += pixel * +3; // x[0,2] -> pixel * +3
                    y_acc += pixel * -3; // y[0,2] -> pixel * -3

                    row_ptr += roi->w;

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x - 1));
                    x_acc += pixel * -10; // x[1,0] -> pixel * -10
                                          // y[1,0] -> pixel * 0

                    // pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                    // x[1,1] -> pixel * 0
                    // y[1,1] -> pixel * 0

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x + 1));
                    x_acc += pixel * +10; // x[1,2] -> pixel * +10
                                          // y[1,2] -> pixel * 0

                    row_ptr += roi->w;

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x - 1));
                    x_acc += pixel * -3; // x[2,0] -> pixel * -3
                    y_acc += pixel * +3;  // y[2,0] -> pixel * +3

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                                          // x[2,1] -> pixel * 0
                    y_acc += pixel * +10; // y[2,1] -> pixel * +10

                    pixel = COLOR_BINARY_TO_GRAYSCALE(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x + 1));
                    x_acc += pixel * +3; // x[2,2] -> pixel * +3
                    y_acc += pixel * +3; // y[2,2] -> pixel * +3

                    row_ptr -= roi->w;

                    int theta = fast_roundf(fast_atan2f(y_acc, x_acc) * 57.295780) % 180; // * (180 / PI)
                    if (theta < 0) theta += 180;
                    int rho = (fast_roundf(((x - roi->x) * cos_table[theta]) + ((y - roi->y) * sin_table[theta])) / hough_divide) + r_diag_len_div;
                    int acc_index = (rho * theta_size) + ((theta / hough_divide) + 1); // add offset

                    int acc_value = acc[acc_index] + fast_sqrtf((x_acc * x_acc) + (y_acc * y_acc));
                    if (acc_value > acc_max) acc_max = acc_value;
                    acc[acc_index] = acc_value;
                }
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            for (int y = roi->y + 1, yy = roi->y + roi->h - 1; y < yy; y += y_stride) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                for (int x = roi->x + (y % x_stride) + 1, xx = roi->x + roi->w - 1; x < xx; x += x_stride) {
                    int pixel; // Sobel Algorithm Below... w/ Scharr...
                    int x_acc = 0;
                    int y_acc = 0;

                    row_ptr -= roi->w;

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x - 1);
                    x_acc += pixel * -3; // x[0,0] -> pixel * -3
                    y_acc += pixel * -3; // y[0,0] -> pixel * -3

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                                          // x[0,1] -> pixel * 0
                    y_acc += pixel * -10; // y[0,1] -> pixel * -10

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x + 1);
                    x_acc += pixel * +3; // x[0,2] -> pixel * +3
                    y_acc += pixel * -3; // y[0,2] -> pixel * -3

                    row_ptr += roi->w;

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x - 1);
                    x_acc += pixel * -10; // x[1,0] -> pixel * -10
                                          // y[1,0] -> pixel * 0

                    // pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    // x[1,1] -> pixel * 0
                    // y[1,1] -> pixel * 0

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x + 1);
                    x_acc += pixel * +10; // x[1,2] -> pixel * +10
                                          // y[1,2] -> pixel * 0

                    row_ptr += roi->w;

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x - 1);
                    x_acc += pixel * -3; // x[2,0] -> pixel * -3
                    y_acc += pixel * +3;  // y[2,0] -> pixel * +3

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                                          // x[2,1] -> pixel * 0
                    y_acc += pixel * +10; // y[2,1] -> pixel * +10

                    pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x + 1);
                    x_acc += pixel * +3; // x[2,2] -> pixel * +3
                    y_acc += pixel * +3; // y[2,2] -> pixel * +3

                    row_ptr -= roi->w;

                    int theta = fast_roundf(fast_atan2f(y_acc, x_acc) * 57.295780) % 180; // * (180 / PI)
                    if (theta < 0) theta += 180;
                    int rho = (fast_roundf(((x - roi->x) * cos_table[theta]) + ((y - roi->y) * sin_table[theta])) / hough_divide) + r_diag_len_div;
                    int acc_index = (rho * theta_size) + ((theta / hough_divide) + 1); // add offset

                    int acc_value = acc[acc_index] + fast_sqrtf((x_acc * x_acc) + (y_acc * y_acc));
                    if (acc_value > acc_max) acc_max = acc_value;
                    acc[acc_index] = acc_value;
                }
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int y = roi->y + 1, yy = roi->y + roi->h - 1; y < yy; y += y_stride) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                for (int x = roi->x + (y % x_stride) + 1, xx = roi->x + roi->w - 1; x < xx; x += x_stride) {
                    int pixel; // Sobel Algorithm Below... w/ Scharr...
                    int x_acc = 0;
                    int y_acc = 0;

                    row_ptr -= roi->w;

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x - 1));
                    x_acc += pixel * -3; // x[0,0] -> pixel * -3
                    y_acc += pixel * -3; // y[0,0] -> pixel * -3

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                                          // x[0,1] -> pixel * 0
                    y_acc += pixel * -10; // y[0,1] -> pixel * -10

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x + 1));
                    x_acc += pixel * +3; // x[0,2] -> pixel * +3
                    y_acc += pixel * -3; // y[0,2] -> pixel * -3

                    row_ptr += roi->w;

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x - 1));
                    x_acc += pixel * -10; // x[1,0] -> pixel * -10
                                          // y[1,0] -> pixel * 0

                    // pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                    // x[1,1] -> pixel * 0
                    // y[1,1] -> pixel * 0

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x + 1));
                    x_acc += pixel * +10; // x[1,2] -> pixel * +10
                                          // y[1,2] -> pixel * 0

                    row_ptr += roi->w;

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x - 1));
                    x_acc += pixel * -3; // x[2,0] -> pixel * -3
                    y_acc += pixel * +3;  // y[2,0] -> pixel * +3

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                                          // x[2,1] -> pixel * 0
                    y_acc += pixel * +10; // y[2,1] -> pixel * +10

                    pixel = COLOR_RGB565_TO_GRAYSCALE(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x + 1));
                    x_acc += pixel * +3; // x[2,2] -> pixel * +3
                    y_acc += pixel * +3; // y[2,2] -> pixel * +3

                    row_ptr -= roi->w;

                    int theta = fast_roundf(fast_atan2f(y_acc, x_acc) * 57.295780) % 180; // * (180 / PI)
                    if (theta < 0) theta += 180;
                    int rho = (fast_roundf(((x - roi->x) * cos_table[theta]) + ((y - roi->y) * sin_table[theta])) / hough_divide) + r_diag_len_div;
                    int acc_index = (rho * theta_size) + ((theta / hough_divide) + 1); // add offset

                    int acc_value = acc[acc_index] + fast_sqrtf((x_acc * x_acc) + (y_acc * y_acc));
                    if (acc_value > acc_max) acc_max = acc_value;
                    acc[acc_index] = acc_value;
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    list_init(out, sizeof(find_lines_list_lnk_data_t));

    if (acc_max) {
        uint32_t acc_threshold = fast_roundf(acc_max * threshold);

        for (int y = 1, yy = r_size - 1; y < yy; y++) {
            uint32_t *row_ptr = acc + (theta_size * y);

            for (int x = 1, xx = theta_size - 1; x < xx; x++) {
                if ((row_ptr[x] >= acc_threshold)
                &&  (row_ptr[x] > row_ptr[x-theta_size+1])
                &&  (row_ptr[x] > row_ptr[x-theta_size])
                &&  (row_ptr[x] > row_ptr[x-theta_size+1])
                &&  (row_ptr[x] > row_ptr[x-1])
                &&  (row_ptr[x] > row_ptr[x+1])
                &&  (row_ptr[x] > row_ptr[x+theta_size-1])
                &&  (row_ptr[x] > row_ptr[x+theta_size])
                &&  (row_ptr[x] > row_ptr[x+theta_size+1])) {

                    find_lines_list_lnk_data_t lnk_line;
                    memset(&lnk_line, 0, sizeof(find_lines_list_lnk_data_t));

                    lnk_line.magnitude = ((float) row_ptr[x]) / ((float) acc_max);
                    lnk_line.theta = (x - 1) * hough_divide; // remove offset
                    lnk_line.rho = (y - r_diag_len_div) * hough_divide;

                    list_push_back(out, &lnk_line);
                }
            }
        }
    }

    fb_free(); // acc

    float magnitude_max = 0;
    for (;;) { // Merge overlapping.
        bool merge_occured = false;

        list_t out_temp;
        list_init(&out_temp, sizeof(find_lines_list_lnk_data_t));

        while (list_size(out)) {
            find_lines_list_lnk_data_t lnk_line;
            list_pop_front(out, &lnk_line);

            for (size_t k = 0, l = list_size(out); k < l; k++) {
                find_lines_list_lnk_data_t tmp_line;
                list_pop_front(out, &tmp_line);

                int theta_0_temp = lnk_line.theta;
                int theta_1_temp = tmp_line.theta;
                int rho_0_temp = lnk_line.rho;
                int rho_1_temp = tmp_line.rho;

                if (rho_0_temp < 0) {
                    rho_0_temp = -rho_0_temp;
                    theta_0_temp += 180;
                }

                if (rho_1_temp < 0) {
                    rho_1_temp = -rho_1_temp;
                    theta_1_temp += 180;
                }

                int theta_diff = abs(theta_0_temp - theta_1_temp);
                int theta_diff_2 = (theta_diff >= 180) ? (360 - theta_diff) : theta_diff;

                bool theta_merge = theta_diff_2 < theta_margin;
                bool rho_merge = abs(rho_0_temp - rho_1_temp) < rho_margin;

                if (theta_merge && rho_merge) {
                    float magnitude = lnk_line.magnitude + tmp_line.magnitude;
                    float sin_mean = ((sin_table[theta_0_temp] * lnk_line.magnitude) + (sin_table[theta_1_temp] * tmp_line.magnitude)) / magnitude;
                    float cos_mean = ((cos_table[theta_0_temp] * lnk_line.magnitude) + (cos_table[theta_1_temp] * tmp_line.magnitude)) / magnitude;

                    lnk_line.theta = fast_roundf(fast_atan2f(sin_mean, cos_mean) * 57.295780) % 360; // * (180 / PI)
                    if (lnk_line.theta < 0) lnk_line.theta += 360;
                    lnk_line.rho = fast_roundf(((rho_0_temp * lnk_line.magnitude) + (rho_1_temp * tmp_line.magnitude)) / magnitude);
                    lnk_line.magnitude = magnitude;

                    if (lnk_line.theta >= 180) {
                        lnk_line.rho = -lnk_line.rho;
                        lnk_line.theta -= 180;
                    }

                    merge_occured = true;
                } else {
                    list_push_back(out, &tmp_line);
                }
            }

            if (lnk_line.magnitude > magnitude_max) magnitude_max = lnk_line.magnitude;
            list_push_back(&out_temp, &lnk_line);
        }

        list_copy(out, &out_temp);

        if (!merge_occured) {
            break;
        }
    }

    for (list_lnk_t *i = iterator_start_from_head(out); i; i = iterator_next(i)) {
        find_lines_list_lnk_data_t lnk_line;
        iterator_get(out, i, &lnk_line);

        if ((45 <= lnk_line.theta) && (lnk_line.theta < 135)) {
            // y = (r - x cos(t)) / sin(t)
            lnk_line.line.x1 = 0;
            lnk_line.line.y1 = fast_roundf((lnk_line.rho - (lnk_line.line.x1 * cos_table[lnk_line.theta])) / sin_table[lnk_line.theta]);
            lnk_line.line.x2 = roi->w - 1;
            lnk_line.line.y2 = fast_roundf((lnk_line.rho - (lnk_line.line.x2 * cos_table[lnk_line.theta])) / sin_table[lnk_line.theta]);
        } else {
            // x = (r - y sin(t)) / cos(t);
            lnk_line.line.y1 = 0;
            lnk_line.line.x1 = fast_roundf((lnk_line.rho - (lnk_line.line.y1 * sin_table[lnk_line.theta])) / cos_table[lnk_line.theta]);
            lnk_line.line.y2 = roi->h - 1;
            lnk_line.line.x2 = fast_roundf((lnk_line.rho - (lnk_line.line.y2 * sin_table[lnk_line.theta])) / cos_table[lnk_line.theta]);
        }

        lnk_line.line.x1 += roi->x;
        lnk_line.line.y1 += roi->y;
        lnk_line.line.x2 += roi->x;
        lnk_line.line.y2 += roi->y;
        lnk_line.magnitude /= magnitude_max;

        iterator_set(out, i, &lnk_line);
    }
}
