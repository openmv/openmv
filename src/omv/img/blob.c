/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "imlib.h"

typedef struct xylf
{
    int16_t x, y, l, r;
}
xylf_t;

void imlib_find_blobs(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                     list_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
                     bool merge, int margin,
                     bool (*threshold_cb)(void*,find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
                     bool (*merge_cb)(void*,find_blobs_list_lnk_data_t*,find_blobs_list_lnk_data_t*), void *merge_cb_arg)
{
    bitmap_t bitmap; // Same size as the image so we don't have to translate.
    bitmap_alloc(&bitmap, ptr->w * ptr->h);

    lifo_t lifo;
    size_t lifo_len;
    lifo_alloc_all(&lifo, &lifo_len, sizeof(xylf_t));

    list_init(out, sizeof(find_blobs_list_lnk_data_t));

    size_t code = 0;
    for (list_lnk_t *it = iterator_start_from_head(thresholds); it; it = iterator_next(it)) {
        color_thresholds_list_lnk_data_t lnk_data;
        iterator_get(thresholds, it, &lnk_data);

        switch(ptr->bpp) {
            case IMAGE_BPP_BINARY: {
                for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                    uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                    size_t row_index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);
                    for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                        if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(row_index, x)))
                        && COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                            int old_x = x;
                            int old_y = y;

                            int blob_x1 = x;
                            int blob_y1 = y;
                            int blob_x2 = x;
                            int blob_y2 = y;
                            int blob_pixels = 0;
                            int blob_cx = 0;
                            int blob_cy = 0;
                            long long blob_a = 0;
                            long long blob_b = 0;
                            long long blob_c = 0;

                            // Scanline Flood Fill Algorithm //

                            for(;;) {
                                int left = x, right = x;
                                uint32_t *row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                                size_t index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);

                                while ((left > roi->x)
                                && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, left - 1)))
                                && COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, left - 1), &lnk_data, invert)) {
                                    left--;
                                }

                                while ((right < (roi->x + roi->w - 1))
                                && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, right + 1)))
                                && COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, right + 1), &lnk_data, invert)) {
                                    right++;
                                }

                                blob_x1 = IM_MIN(blob_x1, left);
                                blob_y1 = IM_MIN(blob_y1, y);
                                blob_x2 = IM_MAX(blob_x2, right);
                                blob_y2 = IM_MAX(blob_y2, y);
                                for (int i = left; i <= right; i++) {
                                    bitmap_bit_set(&bitmap, BITMAP_COMPUTE_INDEX(index, i));
                                    blob_pixels += 1;
                                    blob_cx += i;
                                    blob_cy += y;
                                    blob_a += i*i;
                                    blob_b += i*y;
                                    blob_c += y*y;
                                }

                                bool break_out = false;
                                for(;;) {
                                    if (lifo_size(&lifo) < lifo_len) {

                                        if (y > roi->y) {
                                            row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y - 1);
                                            index = BITMAP_COMPUTE_ROW_INDEX(ptr, y - 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++) {
                                                if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                                                && COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y - 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            }
                                            if (recurse) {
                                                break;
                                            }
                                        }

                                        if (y < (roi->y + roi->h - 1)) {
                                            row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y + 1);
                                            index = BITMAP_COMPUTE_ROW_INDEX(ptr, y + 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++) {
                                                if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                                                && COLOR_THRESHOLD_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y + 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            }
                                            if (recurse) {
                                                break;
                                            }
                                        }
                                    }

                                    if (!lifo_size(&lifo)) {
                                        break_out = true;
                                        break;
                                    }

                                    xylf_t context;
                                    lifo_dequeue(&lifo, &context);
                                    x = context.x;
                                    y = context.y;
                                    left = context.l;
                                    right = context.r;
                                }

                                if (break_out) {
                                    break;
                                }
                            }

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

                            find_blobs_list_lnk_data_t lnk_blob;
                            lnk_blob.rect.x = blob_x1;
                            lnk_blob.rect.y = blob_y1;
                            lnk_blob.rect.w = blob_x2 - blob_x1;
                            lnk_blob.rect.h = blob_y2 - blob_y1;
                            lnk_blob.pixels = blob_pixels;
                            lnk_blob.centroid.x = mx;
                            lnk_blob.centroid.y = my;
                            lnk_blob.rotation = (small_blob_a != small_blob_c) ? (fast_atan2f(2 * small_blob_b, small_blob_a - small_blob_c) / 2.0f) : 0.0f;
                            lnk_blob.code = 1 << code;
                            lnk_blob.count = 1;

                            if (((lnk_blob.rect.w * lnk_blob.rect.h) >= area_threshold) && (lnk_blob.pixels >= pixels_threshold)
                            && ((threshold_cb_arg == NULL) || threshold_cb(threshold_cb_arg, &lnk_blob))) {
                                list_push_back(out, &lnk_blob);
                            }

                            x = old_x;
                            y = old_y;
                        }
                    }
                }
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                    uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                    size_t row_index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);
                    for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                        if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(row_index, x)))
                        && COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                            int old_x = x;
                            int old_y = y;

                            int blob_x1 = x;
                            int blob_y1 = y;
                            int blob_x2 = x;
                            int blob_y2 = y;
                            int blob_pixels = 0;
                            int blob_cx = 0;
                            int blob_cy = 0;
                            long long blob_a = 0;
                            long long blob_b = 0;
                            long long blob_c = 0;

                            // Scanline Flood Fill Algorithm //

                            for(;;) {
                                int left = x, right = x;
                                uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                                size_t index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);

                                while ((left > roi->x)
                                && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, left - 1)))
                                && COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, left - 1), &lnk_data, invert)) {
                                    left--;
                                }

                                while ((right < (roi->x + roi->w - 1))
                                && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, right + 1)))
                                && COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, right + 1), &lnk_data, invert)) {
                                    right++;
                                }

                                blob_x1 = IM_MIN(blob_x1, left);
                                blob_y1 = IM_MIN(blob_y1, y);
                                blob_x2 = IM_MAX(blob_x2, right);
                                blob_y2 = IM_MAX(blob_y2, y);
                                for (int i = left; i <= right; i++) {
                                    bitmap_bit_set(&bitmap, BITMAP_COMPUTE_INDEX(index, i));
                                    blob_pixels += 1;
                                    blob_cx += i;
                                    blob_cy += y;
                                    blob_a += i*i;
                                    blob_b += i*y;
                                    blob_c += y*y;
                                }

                                bool break_out = false;
                                for(;;) {
                                    if (lifo_size(&lifo) < lifo_len) {

                                        if (y > roi->y) {
                                            row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y - 1);
                                            index = BITMAP_COMPUTE_ROW_INDEX(ptr, y - 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++) {
                                                if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                                                && COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y - 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            }
                                            if (recurse) {
                                                break;
                                            }
                                        }

                                        if (y < (roi->y + roi->h - 1)) {
                                            row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y + 1);
                                            index = BITMAP_COMPUTE_ROW_INDEX(ptr, y + 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++) {
                                                if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                                                && COLOR_THRESHOLD_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y + 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            }
                                            if (recurse) {
                                                break;
                                            }
                                        }
                                    }

                                    if (!lifo_size(&lifo)) {
                                        break_out = true;
                                        break;
                                    }

                                    xylf_t context;
                                    lifo_dequeue(&lifo, &context);
                                    x = context.x;
                                    y = context.y;
                                    left = context.l;
                                    right = context.r;
                                }

                                if (break_out) {
                                    break;
                                }
                            }

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

                            find_blobs_list_lnk_data_t lnk_blob;
                            lnk_blob.rect.x = blob_x1;
                            lnk_blob.rect.y = blob_y1;
                            lnk_blob.rect.w = blob_x2 - blob_x1;
                            lnk_blob.rect.h = blob_y2 - blob_y1;
                            lnk_blob.pixels = blob_pixels;
                            lnk_blob.centroid.x = mx;
                            lnk_blob.centroid.y = my;
                            lnk_blob.rotation = (small_blob_a != small_blob_c) ? (fast_atan2f(2 * small_blob_b, small_blob_a - small_blob_c) / 2.0f) : 0.0f;
                            lnk_blob.code = 1 << code;
                            lnk_blob.count = 1;

                            if (((lnk_blob.rect.w * lnk_blob.rect.h) >= area_threshold) && (lnk_blob.pixels >= pixels_threshold)
                            && ((threshold_cb_arg == NULL) || threshold_cb(threshold_cb_arg, &lnk_blob))) {
                                list_push_back(out, &lnk_blob);
                            }

                            x = old_x;
                            y = old_y;
                        }
                    }
                }
                break;
            }
            case IMAGE_BPP_RGB565: {
                for (int y = roi->y, yy = roi->y + roi->h; y < yy; y += y_stride) {
                    uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                    size_t row_index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);
                    for (int x = roi->x + (y % x_stride), xx = roi->x + roi->w; x < xx; x += x_stride) {
                        if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(row_index, x)))
                        && COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                            int old_x = x;
                            int old_y = y;

                            int blob_x1 = x;
                            int blob_y1 = y;
                            int blob_x2 = x;
                            int blob_y2 = y;
                            int blob_pixels = 0;
                            int blob_cx = 0;
                            int blob_cy = 0;
                            long long blob_a = 0;
                            long long blob_b = 0;
                            long long blob_c = 0;

                            // Scanline Flood Fill Algorithm //

                            for(;;) {
                                int left = x, right = x;
                                uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                                size_t index = BITMAP_COMPUTE_ROW_INDEX(ptr, y);

                                while ((left > roi->x)
                                && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, left - 1)))
                                && COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, left - 1), &lnk_data, invert)) {
                                    left--;
                                }

                                while ((right < (roi->x + roi->w - 1))
                                && (!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, right + 1)))
                                && COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, right + 1), &lnk_data, invert)) {
                                    right++;
                                }

                                blob_x1 = IM_MIN(blob_x1, left);
                                blob_y1 = IM_MIN(blob_y1, y);
                                blob_x2 = IM_MAX(blob_x2, right);
                                blob_y2 = IM_MAX(blob_y2, y);
                                for (int i = left; i <= right; i++) {
                                    bitmap_bit_set(&bitmap, BITMAP_COMPUTE_INDEX(index, i));
                                    blob_pixels += 1;
                                    blob_cx += i;
                                    blob_cy += y;
                                    blob_a += i*i;
                                    blob_b += i*y;
                                    blob_c += y*y;
                                }

                                bool break_out = false;
                                for(;;) {
                                    if (lifo_size(&lifo) < lifo_len) {

                                        if (y > roi->y) {
                                            row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y - 1);
                                            index = BITMAP_COMPUTE_ROW_INDEX(ptr, y - 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++) {
                                                if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                                                && COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y - 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            }
                                            if (recurse) {
                                                break;
                                            }
                                        }

                                        if (y < (roi->y + roi->h - 1)) {
                                            row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y + 1);
                                            index = BITMAP_COMPUTE_ROW_INDEX(ptr, y + 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++) {
                                                if ((!bitmap_bit_get(&bitmap, BITMAP_COMPUTE_INDEX(index, i)))
                                                && COLOR_THRESHOLD_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y + 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            }
                                            if (recurse) {
                                                break;
                                            }
                                        }
                                    }

                                    if (!lifo_size(&lifo)) {
                                        break_out = true;
                                        break;
                                    }

                                    xylf_t context;
                                    lifo_dequeue(&lifo, &context);
                                    x = context.x;
                                    y = context.y;
                                    left = context.l;
                                    right = context.r;
                                }

                                if (break_out) {
                                    break;
                                }
                            }

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

                            find_blobs_list_lnk_data_t lnk_blob;
                            lnk_blob.rect.x = blob_x1;
                            lnk_blob.rect.y = blob_y1;
                            lnk_blob.rect.w = blob_x2 - blob_x1;
                            lnk_blob.rect.h = blob_y2 - blob_y1;
                            lnk_blob.pixels = blob_pixels;
                            lnk_blob.centroid.x = mx;
                            lnk_blob.centroid.y = my;
                            lnk_blob.rotation = (small_blob_a != small_blob_c) ? (fast_atan2f(2 * small_blob_b, small_blob_a - small_blob_c) / 2.0f) : 0.0f;
                            lnk_blob.code = 1 << code;
                            lnk_blob.count = 1;

                            if (((lnk_blob.rect.w * lnk_blob.rect.h) >= area_threshold) && (lnk_blob.pixels >= pixels_threshold)
                            && ((threshold_cb_arg == NULL) || threshold_cb(threshold_cb_arg, &lnk_blob))) {
                                list_push_back(out, &lnk_blob);
                            }

                            x = old_x;
                            y = old_y;
                        }
                    }
                }
                break;
            }
            default: {
                break;
            }
        }

        code += 1;
    }

    lifo_free(&lifo);
    bitmap_free(&bitmap);

    if (merge) {
        for(;;) {
            bool merge_occured = false;

            list_t out_temp;
            list_init(&out_temp, sizeof(find_blobs_list_lnk_data_t));

            while(list_size(out)) {
                find_blobs_list_lnk_data_t lnk_blob;
                list_pop_front(out, &lnk_blob);

                for (size_t k = 0, l = list_size(out); k < l; k++) {
                    find_blobs_list_lnk_data_t tmp_blob;
                    list_pop_front(out, &tmp_blob);

                    rectangle_t temp;
                    temp.x = IM_MAX(IM_MIN(tmp_blob.rect.x - margin, INT16_MAX), INT16_MIN);
                    temp.y = IM_MAX(IM_MIN(tmp_blob.rect.y - margin, INT16_MAX), INT16_MIN);
                    temp.w = IM_MAX(IM_MIN(tmp_blob.rect.w + (margin * 2), INT16_MAX), 0);
                    temp.h = IM_MAX(IM_MIN(tmp_blob.rect.h + (margin * 2), INT16_MAX), 0);

                    if (rectangle_overlap(&(lnk_blob.rect), &temp)
                    && ((merge_cb_arg == NULL) || merge_cb(merge_cb_arg, &lnk_blob, &tmp_blob))) {
                        rectangle_united(&(lnk_blob.rect), &(tmp_blob.rect));
                        lnk_blob.centroid.x = ((lnk_blob.centroid.x * lnk_blob.pixels) + (tmp_blob.centroid.x * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
                        lnk_blob.centroid.y = ((lnk_blob.centroid.y * lnk_blob.pixels) + (tmp_blob.centroid.y * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
                        float sin_mean = ((sinf(lnk_blob.rotation) * lnk_blob.pixels) + (sinf(tmp_blob.rotation) * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
                        float cos_mean = ((cosf(lnk_blob.rotation) * lnk_blob.pixels) + (cosf(tmp_blob.rotation) * tmp_blob.pixels)) / (lnk_blob.pixels + tmp_blob.pixels);
                        lnk_blob.rotation = fast_atan2f(sin_mean, cos_mean);
                        lnk_blob.pixels += tmp_blob.pixels; // won't overflow
                        lnk_blob.code |= tmp_blob.code;
                        lnk_blob.count = IM_MAX(IM_MIN(lnk_blob.count + tmp_blob.count, UINT16_MAX), 0);
                        merge_occured = true;
                    } else {
                        list_push_back(out, &tmp_blob);
                    }
                }

                list_push_back(&out_temp, &lnk_blob);
            }

            list_copy(out, &out_temp);

            if (!merge_occured) {
                break;
            }
        }
    }
}

void imlib_flood_fill_int(image_t *out, image_t *img, int x, int y,
                          int seed_threshold, int floating_threshold,
                          flood_fill_call_back_t cb, void *data)
{
    lifo_t lifo;
    size_t lifo_len;
    lifo_alloc_all(&lifo, &lifo_len, sizeof(xylf_t));

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            for(int seed_pixel = IMAGE_GET_BINARY_PIXEL(img, x, y);;) {
                int left = x, right = x;
                uint32_t *row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);

                while ((left > 0)
                && (!IMAGE_GET_BINARY_PIXEL_FAST(out_row, left - 1))
                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, left - 1), seed_pixel, seed_threshold)
                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, left - 1),
                                      IMAGE_GET_BINARY_PIXEL_FAST(row, left), floating_threshold)) {
                    left--;
                }

                while ((right < (img->w - 1))
                && (!IMAGE_GET_BINARY_PIXEL_FAST(out_row, right + 1))
                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, right + 1), seed_pixel, seed_threshold)
                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, right + 1),
                                      IMAGE_GET_BINARY_PIXEL_FAST(row, right), floating_threshold)) {
                    right++;
                }

                for (int i = left; i <= right; i++) {
                    IMAGE_SET_BINARY_PIXEL_FAST(out_row, i);
                }

                bool break_out = false;
                for(;;) {
                    if (lifo_size(&lifo) < lifo_len) {
                        uint32_t *old_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);

                        if (y > 0) {
                            row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y - 1);
                            out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y - 1);

                            bool recurse = false;
                            for (int i = left; i <= right; i++) {
                                if ((!IMAGE_GET_BINARY_PIXEL_FAST(out_row, i))
                                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, i), seed_pixel, seed_threshold)
                                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, i),
                                                      IMAGE_GET_BINARY_PIXEL_FAST(old_row, i), floating_threshold)) {
                                    xylf_t context;
                                    context.x = x;
                                    context.y = y;
                                    context.l = left;
                                    context.r = right;
                                    lifo_enqueue(&lifo, &context);
                                    x = i;
                                    y = y - 1;
                                    recurse = true;
                                    break;
                                }
                            }
                            if (recurse) {
                                break;
                            }
                        }

                        if (y < (img->h - 1)) {
                            row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y + 1);
                            out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y + 1);

                            bool recurse = false;
                            for (int i = left; i <= right; i++) {
                                if ((!IMAGE_GET_BINARY_PIXEL_FAST(out_row, i))
                                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, i), seed_pixel, seed_threshold)
                                && COLOR_BOUND_BINARY(IMAGE_GET_BINARY_PIXEL_FAST(row, i),
                                                      IMAGE_GET_BINARY_PIXEL_FAST(old_row, i), floating_threshold)) {
                                    xylf_t context;
                                    context.x = x;
                                    context.y = y;
                                    context.l = left;
                                    context.r = right;
                                    lifo_enqueue(&lifo, &context);
                                    x = i;
                                    y = y + 1;
                                    recurse = true;
                                    break;
                                }
                            }
                            if (recurse) {
                                break;
                            }
                        }
                    }

                    if (cb) cb(img, y, left, right, data);

                    if (!lifo_size(&lifo)) {
                        break_out = true;
                        break;
                    }

                    xylf_t context;
                    lifo_dequeue(&lifo, &context);
                    x = context.x;
                    y = context.y;
                    left = context.l;
                    right = context.r;
                }

                if (break_out) {
                    break;
                }
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            for(int seed_pixel = IMAGE_GET_GRAYSCALE_PIXEL(img, x, y);;) {
                int left = x, right = x;
                uint8_t *row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint32_t *out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);

                while ((left > 0)
                && (!IMAGE_GET_BINARY_PIXEL_FAST(out_row, left - 1))
                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, left - 1), seed_pixel, seed_threshold)
                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, left - 1),
                                         IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, left), floating_threshold)) {
                    left--;
                }

                while ((right < (img->w - 1))
                && (!IMAGE_GET_BINARY_PIXEL_FAST(out_row, right + 1))
                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, right + 1), seed_pixel, seed_threshold)
                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, right + 1),
                                         IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, right), floating_threshold)) {
                    right++;
                }

                for (int i = left; i <= right; i++) {
                    IMAGE_SET_BINARY_PIXEL_FAST(out_row, i);
                }

                bool break_out = false;
                for(;;) {
                    if (lifo_size(&lifo) < lifo_len) {
                        uint8_t *old_row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);

                        if (y > 0) {
                            row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y - 1);
                            out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y - 1);

                            bool recurse = false;
                            for (int i = left; i <= right; i++) {
                                if ((!IMAGE_GET_BINARY_PIXEL_FAST(out_row, i))
                                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), seed_pixel, seed_threshold)
                                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i),
                                                         IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row, i), floating_threshold)) {
                                    xylf_t context;
                                    context.x = x;
                                    context.y = y;
                                    context.l = left;
                                    context.r = right;
                                    lifo_enqueue(&lifo, &context);
                                    x = i;
                                    y = y - 1;
                                    recurse = true;
                                    break;
                                }
                            }
                            if (recurse) {
                                break;
                            }
                        }

                        if (y < (img->h - 1)) {
                            row = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + 1);
                            out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y + 1);

                            bool recurse = false;
                            for (int i = left; i <= right; i++) {
                                if ((!IMAGE_GET_BINARY_PIXEL_FAST(out_row, i))
                                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), seed_pixel, seed_threshold)
                                && COLOR_BOUND_GRAYSCALE(IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i),
                                                         IMAGE_GET_GRAYSCALE_PIXEL_FAST(old_row, i), floating_threshold)) {
                                    xylf_t context;
                                    context.x = x;
                                    context.y = y;
                                    context.l = left;
                                    context.r = right;
                                    lifo_enqueue(&lifo, &context);
                                    x = i;
                                    y = y + 1;
                                    recurse = true;
                                    break;
                                }
                            }
                            if (recurse) {
                                break;
                            }
                        }
                    }

                    if (cb) cb(img, y, left, right, data);

                    if (!lifo_size(&lifo)) {
                        break_out = true;
                        break;
                    }

                    xylf_t context;
                    lifo_dequeue(&lifo, &context);
                    x = context.x;
                    y = context.y;
                    left = context.l;
                    right = context.r;
                }

                if (break_out) {
                    break;
                }
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            for(int seed_pixel = IMAGE_GET_RGB565_PIXEL(img, x, y);;) {
                int left = x, right = x;
                uint16_t *row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint32_t *out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y);

                while ((left > 0)
                && (!IMAGE_GET_BINARY_PIXEL_FAST(out_row, left - 1))
                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, left - 1), seed_pixel, seed_threshold)
                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, left - 1),
                                      IMAGE_GET_RGB565_PIXEL_FAST(row, left), floating_threshold)) {
                    left--;
                }

                while ((right < (img->w - 1))
                && (!IMAGE_GET_BINARY_PIXEL_FAST(out_row, right + 1))
                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, right + 1), seed_pixel, seed_threshold)
                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, right + 1),
                                      IMAGE_GET_RGB565_PIXEL_FAST(row, right), floating_threshold)) {
                    right++;
                }

                for (int i = left; i <= right; i++) {
                    IMAGE_SET_BINARY_PIXEL_FAST(out_row, i);
                }

                bool break_out = false;
                for(;;) {
                    if (lifo_size(&lifo) < lifo_len) {
                        uint16_t *old_row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);

                        if (y > 0) {
                            row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y - 1);
                            out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y - 1);

                            bool recurse = false;
                            for (int i = left; i <= right; i++) {
                                if ((!IMAGE_GET_BINARY_PIXEL_FAST(out_row, i))
                                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, i), seed_pixel, seed_threshold)
                                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, i),
                                                      IMAGE_GET_RGB565_PIXEL_FAST(old_row, i), floating_threshold)) {
                                    xylf_t context;
                                    context.x = x;
                                    context.y = y;
                                    context.l = left;
                                    context.r = right;
                                    lifo_enqueue(&lifo, &context);
                                    x = i;
                                    y = y - 1;
                                    recurse = true;
                                    break;
                                }
                            }
                            if (recurse) {
                                break;
                            }
                        }

                        if (y < (img->h - 1)) {
                            row = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + 1);
                            out_row = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(out, y + 1);

                            bool recurse = false;
                            for (int i = left; i <= right; i++) {
                                if ((!IMAGE_GET_BINARY_PIXEL_FAST(out_row, i))
                                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, i), seed_pixel, seed_threshold)
                                && COLOR_BOUND_RGB565(IMAGE_GET_RGB565_PIXEL_FAST(row, i),
                                                      IMAGE_GET_RGB565_PIXEL_FAST(old_row, i), floating_threshold)) {
                                    xylf_t context;
                                    context.x = x;
                                    context.y = y;
                                    context.l = left;
                                    context.r = right;
                                    lifo_enqueue(&lifo, &context);
                                    x = i;
                                    y = y + 1;
                                    recurse = true;
                                    break;
                                }
                            }
                            if (recurse) {
                                break;
                            }
                        }
                    }

                    if (cb) cb(img, y, left, right, data);

                    if (!lifo_size(&lifo)) {
                        break_out = true;
                        break;
                    }

                    xylf_t context;
                    lifo_dequeue(&lifo, &context);
                    x = context.x;
                    y = context.y;
                    left = context.l;
                    right = context.r;
                }

                if (break_out) {
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    lifo_free(&lifo);
}
