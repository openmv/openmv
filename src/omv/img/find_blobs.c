/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "xalloc.h"
#include "fmath.h"
#include "imlib_color.h"
#include "utils_bitmap.h"
#include "utils_lifo.h"
#include "find_blobs.h"

#define COMPUTE_BITMAP_ROW_INDEX(image, y) (((image)->geometry.h)*(y))
#define COMPTUE_BITMAP_INDEX(row_index, x) ((row_index)+(x))

typedef struct xylf
{
    uint16_t x, y, l, r;
}
xylf_t;

void find_blobs(utils_linkedlist_t *list, imlib_image_t *ptr, utils_rectangle_t *roi,
                utils_linkedlist_t *thresholds, bool invert, unsigned int pixels_threshold,
                bool merge, int l_margin, int t_margin, int r_margin, int b_margin)
{
    utils_size_check(&(roi->s));
    imlib_image_check_overlap(ptr, roi);

    utils_rectangle_t rect;
    utils_rectangle_copy(&rect, roi);
    imlib_image_intersected(ptr, &rect);
    utils_size_check(&(rect.s));

    utils_bitmap_t bitmap;
    utils_bitmap_alloc(&bitmap, ptr->geometry.w * ptr->geometry.h);

    size_t lifo_len = (ptr->geometry.w > 1) ? (((ptr->geometry.w- 1) * 2) + (ptr->geometry.h * 2)) : ptr->geometry.h; // use perimeter...
    utils_lifo_t lifo;
    utils_lifo_alloc(&lifo, lifo_len, sizeof(xylf_t));

    utils_linkedlist_alloc(list, sizeof(find_blobs_linkedlist_lnk_data_t));

    size_t code = 0;
    for (utils_linkedlist_lnk_t *it = utils_linkedlist_start_from_head(thresholds); it; it = utils_linkedlist_lnk_next(it)) {
        imlib_color_thresholds_linkedlist_lnk_data_t lnk_data;
        utils_linkedlist_lnk_get(thresholds, it, &lnk_data);

        switch(ptr->type) {
            case IMLIB_IMAGE_TYPE_BINARY: {
                for (int y = rect.p.y, yy = rect.p.y + rect.s.h; y < yy; y++) {
                    uint32_t *row_ptr = IMLIB_IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                    size_t row_index = COMPUTE_BITMAP_ROW_INDEX(ptr, y);
                    for (int x = rect.p.x, xx = rect.p.x + rect.s.w; x < xx; x++) {
                        if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(row_index, x)))
                        && IMLIB_COLOR_THRESHOLD_BINARY(IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                            int old_x = x;

                            unsigned int blob_x1 = x;
                            unsigned int blob_y1 = y;
                            unsigned int blob_x2 = x;
                            unsigned int blob_y2 = y;
                            unsigned int blob_pixels = 0;
                            unsigned long blob_cx = 0;
                            unsigned long blob_cy = 0;
                            unsigned long long blob_a = 0; // (x - mx) * (x - mx)
                            unsigned long long blob_b = 0; // (x - mx) * (y - my)
                            unsigned long long blob_c = 0; // (y - my) * (y - my)

                            // Scanline Flood Fill Algorithm //

                            for(;;) {
                                int left = x, right = x;
                                uint32_t *row = IMLIB_IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y);
                                size_t index = COMPUTE_BITMAP_ROW_INDEX(ptr, y);

                                while ((left > rect.p.x)
                                && (!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, left - 1)))
                                && IMLIB_COLOR_THRESHOLD_BINARY(IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row, left - 1), &lnk_data, invert)) {
                                    left--;
                                }

                                while ((right < (rect.p.x + rect.s.w - 1))
                                && (!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, right + 1)))
                                && IMLIB_COLOR_THRESHOLD_BINARY(IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row, right + 1), &lnk_data, invert)) {
                                    right++;
                                }

                                blob_x1 = OTHER_MIN(blob_x1, left);
                                blob_y1 = OTHER_MIN(blob_y1, y);
                                blob_x2 = OTHER_MAX(blob_x2, right);
                                blob_y2 = OTHER_MAX(blob_y2, y);
                                for (int i = left; i <= right; i++) {
                                    utils_bitmap_bit_set_known_clear(&bitmap, COMPTUE_BITMAP_INDEX(index, i));
                                    blob_pixels += 1;
                                    blob_cx += i;
                                    blob_cy += y;
                                    blob_a += i*i;
                                    blob_b += i*y;
                                    blob_c += y*y;
                                }

                                bool break_out = false;
                                for(;;) {
                                    if (utils_lifo_size(&lifo) < lifo_len) {

                                        if (y > rect.p.y) {
                                            row = IMLIB_IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y - 1);
                                            index = COMPUTE_BITMAP_ROW_INDEX(ptr, y - 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++)
                                                if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, i)))
                                                && IMLIB_COLOR_THRESHOLD_BINARY(IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    utils_lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y - 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            if (recurse) {
                                                break;
                                            }
                                        }

                                        if (y < (rect.p.y + rect.s.h - 1)) {
                                            row = IMLIB_IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(ptr, y + 1);
                                            index = COMPUTE_BITMAP_ROW_INDEX(ptr, y + 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++)
                                                if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, i)))
                                                && IMLIB_COLOR_THRESHOLD_BINARY(IMLIB_IMAGE_GET_BINARY_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    utils_lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y + 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            if (recurse) {
                                                break;
                                            }
                                        }
                                    }

                                    if (!utils_lifo_size(&lifo)) {
                                        break_out = true;
                                        break;
                                    }

                                    xylf_t context;
                                    utils_lifo_dequeue(&lifo, &context);
                                    x = context.x;
                                    y = context.y;
                                    left = context.l;
                                    right = context.r;
                                }

                                if (break_out) {
                                    break;
                                }
                            }

                            unsigned int mx = blob_cx / blob_pixels; // x centroid
                            unsigned int my = blob_cy / blob_pixels; // y centroid
                            // http://www.cse.usf.edu/~r1k/MachineVisionBook/MachineVision.files/MachineVision_Chapter2.pdf
                            // https://www.strchr.com/standard_deviation_in_one_pass
                            blob_a -= 2 * mx * blob_cx;
                            blob_a += blob_pixels * mx * mx;
                            blob_b -= (mx * blob_cy) + (my * blob_cx);
                            blob_b += blob_pixels * mx * my;
                            blob_c -= 2 * my * blob_cy;
                            blob_c += blob_pixels * my * my;

                            find_blobs_linkedlist_lnk_data_t lnk_blob;
                            lnk_blob.rect.p.x = blob_x1;
                            lnk_blob.rect.p.y = blob_y1;
                            lnk_blob.rect.s.w = blob_x2 - blob_x1;
                            lnk_blob.rect.s.h = blob_y2 - blob_y1;
                            lnk_blob.pixels = blob_pixels;
                            lnk_blob.centroid.x = mx;
                            lnk_blob.centroid.y = my;
                            lnk_blob.rotation = (blob_a != blob_c) ? (fast_atan2f(2 * blob_b, blob_a - blob_c) / 2.0f) : 0.0f;
                            lnk_blob.code = 1 << code;
                            lnk_blob.count = 1;

                            if (blob_pixels >= pixels_threshold) {
                                utils_linkedlist_push_back(list, &lnk_blob);
                            }

                            x = old_x;
                        }
                    }
                }
                break;
            }
            case IMLIB_IMAGE_TYPE_GRAYSCALE: {
                for (int y = rect.p.y, yy = rect.p.y + rect.s.h; y < yy; y++) {
                    uint8_t *row_ptr = IMLIB_IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                    size_t row_index = COMPUTE_BITMAP_ROW_INDEX(ptr, y);
                    for (int x = rect.p.x, xx = rect.p.x + rect.s.w; x < xx; x++) {
                        if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(row_index, x)))
                        && IMLIB_COLOR_THRESHOLD_GRAYSCALE(IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                            int old_x = x;

                            unsigned int blob_x1 = x;
                            unsigned int blob_y1 = y;
                            unsigned int blob_x2 = x;
                            unsigned int blob_y2 = y;
                            unsigned int blob_pixels = 0;
                            unsigned long blob_cx = 0;
                            unsigned long blob_cy = 0;
                            unsigned long long blob_a = 0; // (x - mx) * (x - mx)
                            unsigned long long blob_b = 0; // (x - mx) * (y - my)
                            unsigned long long blob_c = 0; // (y - my) * (y - my)

                            // Scanline Flood Fill Algorithm //

                            for(;;) {
                                int left = x, right = x;
                                uint8_t *row = IMLIB_IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y);
                                size_t index = COMPUTE_BITMAP_ROW_INDEX(ptr, y);

                                while ((left > rect.p.x)
                                && (!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, left - 1)))
                                && IMLIB_COLOR_THRESHOLD_GRAYSCALE(IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, left - 1), &lnk_data, invert)) {
                                    left--;
                                }

                                while ((right < (rect.p.x + rect.s.w - 1))
                                && (!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, right + 1)))
                                && IMLIB_COLOR_THRESHOLD_GRAYSCALE(IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, right + 1), &lnk_data, invert)) {
                                    right++;
                                }

                                blob_x1 = OTHER_MIN(blob_x1, left);
                                blob_y1 = OTHER_MIN(blob_y1, y);
                                blob_x2 = OTHER_MAX(blob_x2, right);
                                blob_y2 = OTHER_MAX(blob_y2, y);
                                for (int i = left; i <= right; i++) {
                                    utils_bitmap_bit_set_known_clear(&bitmap, COMPTUE_BITMAP_INDEX(index, i));
                                    blob_pixels += 1;
                                    blob_cx += i;
                                    blob_cy += y;
                                    blob_a += i*i;
                                    blob_b += i*y;
                                    blob_c += y*y;
                                }

                                bool break_out = false;
                                for(;;) {
                                    if (utils_lifo_size(&lifo) < lifo_len) {

                                        if (y > rect.p.y) {
                                            row = IMLIB_IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y - 1);
                                            index = COMPUTE_BITMAP_ROW_INDEX(ptr, y - 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++)
                                                if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, i)))
                                                && IMLIB_COLOR_THRESHOLD_GRAYSCALE(IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    utils_lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y - 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            if (recurse) {
                                                break;
                                            }
                                        }

                                        if (y < (rect.p.y + rect.s.h - 1)) {
                                            row = IMLIB_IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(ptr, y + 1);
                                            index = COMPUTE_BITMAP_ROW_INDEX(ptr, y + 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++)
                                                if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, i)))
                                                && IMLIB_COLOR_THRESHOLD_GRAYSCALE(IMLIB_IMAGE_GET_GRAYSCALE_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    utils_lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y + 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            if (recurse) {
                                                break;
                                            }
                                        }
                                    }

                                    if (!utils_lifo_size(&lifo)) {
                                        break_out = true;
                                        break;
                                    }

                                    xylf_t context;
                                    utils_lifo_dequeue(&lifo, &context);
                                    x = context.x;
                                    y = context.y;
                                    left = context.l;
                                    right = context.r;
                                }

                                if (break_out) {
                                    break;
                                }
                            }

                            unsigned int mx = blob_cx / blob_pixels; // x centroid
                            unsigned int my = blob_cy / blob_pixels; // y centroid
                            // http://www.cse.usf.edu/~r1k/MachineVisionBook/MachineVision.files/MachineVision_Chapter2.pdf
                            // https://www.strchr.com/standard_deviation_in_one_pass
                            blob_a -= 2 * mx * blob_cx;
                            blob_a += blob_pixels * mx * mx;
                            blob_b -= (mx * blob_cy) + (my * blob_cx);
                            blob_b += blob_pixels * mx * my;
                            blob_c -= 2 * my * blob_cy;
                            blob_c += blob_pixels * my * my;

                            find_blobs_linkedlist_lnk_data_t lnk_blob;
                            lnk_blob.rect.p.x = blob_x1;
                            lnk_blob.rect.p.y = blob_y1;
                            lnk_blob.rect.s.w = blob_x2 - blob_x1;
                            lnk_blob.rect.s.h = blob_y2 - blob_y1;
                            lnk_blob.pixels = blob_pixels;
                            lnk_blob.centroid.x = mx;
                            lnk_blob.centroid.y = my;
                            lnk_blob.rotation = (blob_a != blob_c) ? (fast_atan2f(2 * blob_b, blob_a - blob_c) / 2.0f) : 0.0f;
                            lnk_blob.code = 1 << code;
                            lnk_blob.count = 1;

                            if (blob_pixels >= pixels_threshold) {
                                utils_linkedlist_push_back(list, &lnk_blob);
                            }

                            x = old_x;
                        }
                    }
                }
                break;
            }
            case IMLIB_IMAGE_TYPE_RGB565: {
                for (int y = rect.p.y, yy = rect.p.y + rect.s.h; y < yy; y++) {
                    uint16_t *row_ptr = IMLIB_IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                    size_t row_index = COMPUTE_BITMAP_ROW_INDEX(ptr, y);
                    for (int x = rect.p.x, xx = rect.p.x + rect.s.w; x < xx; x++) {
                        if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(row_index, x)))
                        && IMLIB_COLOR_THRESHOLD_RGB565(IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x), &lnk_data, invert)) {
                            int old_x = x;

                            unsigned int blob_x1 = x;
                            unsigned int blob_y1 = y;
                            unsigned int blob_x2 = x;
                            unsigned int blob_y2 = y;
                            unsigned int blob_pixels = 0;
                            unsigned long blob_cx = 0;
                            unsigned long blob_cy = 0;
                            unsigned long long blob_a = 0; // (x - mx) * (x - mx)
                            unsigned long long blob_b = 0; // (x - mx) * (y - my)
                            unsigned long long blob_c = 0; // (y - my) * (y - my)

                            // Scanline Flood Fill Algorithm //

                            for(;;) {
                                int left = x, right = x;
                                uint16_t *row = IMLIB_IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y);
                                size_t index = COMPUTE_BITMAP_ROW_INDEX(ptr, y);

                                while ((left > rect.p.x)
                                && (!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, left - 1)))
                                && IMLIB_COLOR_THRESHOLD_RGB565(IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row, left - 1), &lnk_data, invert)) {
                                    left--;
                                }

                                while ((right < (rect.p.x + rect.s.w - 1))
                                && (!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, right + 1)))
                                && IMLIB_COLOR_THRESHOLD_RGB565(IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row, right + 1), &lnk_data, invert)) {
                                    right++;
                                }

                                blob_x1 = OTHER_MIN(blob_x1, left);
                                blob_y1 = OTHER_MIN(blob_y1, y);
                                blob_x2 = OTHER_MAX(blob_x2, right);
                                blob_y2 = OTHER_MAX(blob_y2, y);
                                for (int i = left; i <= right; i++) {
                                    utils_bitmap_bit_set_known_clear(&bitmap, COMPTUE_BITMAP_INDEX(index, i));
                                    blob_pixels += 1;
                                    blob_cx += i;
                                    blob_cy += y;
                                    blob_a += i*i;
                                    blob_b += i*y;
                                    blob_c += y*y;
                                }

                                bool break_out = false;
                                for(;;) {
                                    if (utils_lifo_size(&lifo) < lifo_len) {

                                        if (y > rect.p.y) {
                                            row = IMLIB_IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y - 1);
                                            index = COMPUTE_BITMAP_ROW_INDEX(ptr, y - 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++)
                                                if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, i)))
                                                && IMLIB_COLOR_THRESHOLD_RGB565(IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    utils_lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y - 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            if (recurse) {
                                                break;
                                            }
                                        }

                                        if (y < (rect.p.y + rect.s.h - 1)) {
                                            row = IMLIB_IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(ptr, y + 1);
                                            index = COMPUTE_BITMAP_ROW_INDEX(ptr, y + 1);

                                            bool recurse = false;
                                            for (int i = left; i <= right; i++)
                                                if ((!utils_bitmap_bit_get(&bitmap, COMPTUE_BITMAP_INDEX(index, i)))
                                                && IMLIB_COLOR_THRESHOLD_RGB565(IMLIB_IMAGE_GET_RGB565_PIXEL_FAST(row, i), &lnk_data, invert)) {
                                                    xylf_t context;
                                                    context.x = x;
                                                    context.y = y;
                                                    context.l = left;
                                                    context.r = right;
                                                    utils_lifo_enqueue(&lifo, &context);
                                                    x = i;
                                                    y = y + 1;
                                                    recurse = true;
                                                    break;
                                                }
                                            if (recurse) {
                                                break;
                                            }
                                        }
                                    }

                                    if (!utils_lifo_size(&lifo)) {
                                        break_out = true;
                                        break;
                                    }

                                    xylf_t context;
                                    utils_lifo_dequeue(&lifo, &context);
                                    x = context.x;
                                    y = context.y;
                                    left = context.l;
                                    right = context.r;
                                }

                                if (break_out) {
                                    break;
                                }
                            }

                            unsigned int mx = blob_cx / blob_pixels; // x centroid
                            unsigned int my = blob_cy / blob_pixels; // y centroid
                            // http://www.cse.usf.edu/~r1k/MachineVisionBook/MachineVision.files/MachineVision_Chapter2.pdf
                            // https://www.strchr.com/standard_deviation_in_one_pass
                            blob_a -= 2 * mx * blob_cx;
                            blob_a += blob_pixels * mx * mx;
                            blob_b -= (mx * blob_cy) + (my * blob_cx);
                            blob_b += blob_pixels * mx * my;
                            blob_c -= 2 * my * blob_cy;
                            blob_c += blob_pixels * my * my;

                            find_blobs_linkedlist_lnk_data_t lnk_blob;
                            lnk_blob.rect.p.x = blob_x1;
                            lnk_blob.rect.p.y = blob_y1;
                            lnk_blob.rect.s.w = blob_x2 - blob_x1;
                            lnk_blob.rect.s.h = blob_y2 - blob_y1;
                            lnk_blob.pixels = blob_pixels;
                            lnk_blob.centroid.x = mx;
                            lnk_blob.centroid.y = my;
                            lnk_blob.rotation = (blob_a != blob_c) ? (fast_atan2f(2 * blob_b, blob_a - blob_c) / 2.0f) : 0.0f;
                            lnk_blob.code = 1 << code;
                            lnk_blob.count = 1;

                            if (blob_pixels >= pixels_threshold) {
                                utils_linkedlist_push_back(list, &lnk_blob);
                            }

                            x = old_x;
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

    utils_lifo_free(&lifo);
    utils_bitmap_free(&bitmap);

    if (merge) {
        bool merge_occured = false;

        do {
            utils_linkedlist_t temp;
            utils_linkedlist_alloc(&temp, sizeof(find_blobs_linkedlist_lnk_data_t));

            while(utils_linkedlist_size(list)) {
                find_blobs_linkedlist_lnk_data_t lnk_blob;
                utils_linkedlist_pop_front(list, &lnk_blob);

                for (size_t k = 0, l = utils_linkedlist_size(list); k < l; k++) {
                    find_blobs_linkedlist_lnk_data_t other_blob;
                    utils_linkedlist_pop_front(list, &other_blob);

                    utils_rectangle_t temp;
                    temp.p.x = other_blob.rect.p.x - l_margin;
                    temp.p.y = other_blob.rect.p.y - t_margin;
                    temp.s.w = other_blob.rect.s.w + l_margin + r_margin;
                    temp.s.h = other_blob.rect.s.h + t_margin + b_margin;

                    if (utils_rectangle_overlap(&(lnk_blob.rect), &temp)) {
                        utils_rectangle_united(&(lnk_blob.rect), &(other_blob.rect));
                        lnk_blob.centroid.x = ((lnk_blob.centroid.x * lnk_blob.pixels) + (other_blob.centroid.x * other_blob.pixels)) / (lnk_blob.pixels + other_blob.pixels);
                        lnk_blob.centroid.y = ((lnk_blob.centroid.y * lnk_blob.pixels) + (other_blob.centroid.y * other_blob.pixels)) / (lnk_blob.pixels + other_blob.pixels);
                        lnk_blob.rotation = ((lnk_blob.rotation * lnk_blob.pixels) + (other_blob.rotation * other_blob.pixels)) / (lnk_blob.pixels + other_blob.pixels);
                        lnk_blob.pixels += other_blob.pixels;
                        lnk_blob.code |= other_blob.code;
                        lnk_blob.count += other_blob.count;
                        merge_occured = true;
                    } else {
                        utils_linkedlist_push_back(list, &other_blob);
                    }
                }

                utils_linkedlist_push_back(&temp, &lnk_blob);
            }

            memcpy(&list, &temp, sizeof(utils_linkedlist_t));
        } while(!merge_occured);
    }
}
