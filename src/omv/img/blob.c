/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Blob and color code/marker detection code...
 *
 */
#include <mp.h>
#include "mdefs.h"
#include "fb_alloc.h"
#include "imlib.h"

ALWAYS_INLINE static uint8_t *init_mask(rectangle_t *roi)
{
    return fb_alloc0(((roi->w+7)/8)*roi->h);
}

ALWAYS_INLINE static void deinit_mask()
{
    fb_free();
}

ALWAYS_INLINE static void set_mask_pixel(rectangle_t *roi, uint8_t *mask, int x, int y)
{
    mask[(((roi->w+7)/8)*y)+(x/8)] |= (1 << (x%8));
}

ALWAYS_INLINE static bool get_not_mask_pixel(rectangle_t *roi, uint8_t *mask, int x, int y)
{
    return !((mask[(((roi->w+7)/8)*y)+(x/8)] >> (x%8)) & 1);
}

typedef struct stack_queue {
    int head_p, tail_p, size;
    point_t *data_p;
} stack_queue_t;

ALWAYS_INLINE static stack_queue_t *init_stack_queue(rectangle_t *roi)
{
    stack_queue_t *sq = fb_alloc(sizeof(stack_queue_t));
    sq->head_p = 0;
    sq->tail_p = 0;
    // The size here is the perimeter in pixels around the roi. It's the perimeter
    // around the roi vs the roi perimeter so that we can't run out of space while
    // executing the wildfire algorithm for new points. Additionally, this also
    // takes care of the pointer comparison issue since it will never get full.
    sq->size = (((roi->w+2)*2)-2)+(((roi->h+2)*2)-2);
    sq->data_p = fb_alloc(sq->size*sizeof(point_t));
    return sq;
}

ALWAYS_INLINE static void deinit_stack_queue()
{
    fb_free();
    fb_free();
}

ALWAYS_INLINE static void stack_queue_push(stack_queue_t *sq, int x, int y)
{
    sq->data_p[sq->head_p] = (point_t) {.x = x, .y = y};
    sq->head_p = (sq->head_p + 1) % sq->size;
}

ALWAYS_INLINE static point_t stack_queue_pop(stack_queue_t *sq)
{
    point_t p = sq->data_p[sq->tail_p];
    sq->tail_p = (sq->tail_p + 1) % sq->size;
    return p;
}

ALWAYS_INLINE static bool stack_queue_not_empty(stack_queue_t *sq)
{
    return sq->head_p != sq->tail_p;
}

ALWAYS_INLINE static bool threshold_gs(image_t *img, int x, int y, simple_color_t l_thresholds, simple_color_t h_thresholds, bool invert)
{
    int pixel = IM_GET_GS_PIXEL(img, x, y);
    return invert ^
           ((l_thresholds.G <= pixel) &&
            (pixel <= h_thresholds.G));
}

ALWAYS_INLINE static bool threshold_rgb565(image_t *img, int x, int y, simple_color_t l_thresholds, simple_color_t h_thresholds, bool invert)
{
    int pixel = IM_GET_RGB565_PIXEL(img, x, y);
    const int lab_l = IM_RGB5652L(pixel);
    const int lab_a = IM_RGB5652A(pixel);
    const int lab_b = IM_RGB5652B(pixel);
    return invert ^
           ((l_thresholds.L <= lab_l) &&
            (lab_l <= h_thresholds.L) &&
            (l_thresholds.A <= lab_a) &&
            (lab_a <= h_thresholds.A) &&
            (l_thresholds.B <= lab_b) &&
            (lab_b <= h_thresholds.B));
}

ALWAYS_INLINE static bool threshold(image_t *img, int x, int y, simple_color_t l_thresholds, simple_color_t h_thresholds, bool invert)
{
    if (IM_IS_GS(img)) {
        return threshold_gs(img, x, y, l_thresholds, h_thresholds, invert);
    } else {
        return threshold_rgb565(img, x, y, l_thresholds, h_thresholds, invert);
    }
}

mp_obj_t imlib_find_blobs(mp_obj_t img_obj, image_t *img, int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds, bool invert, rectangle_t *r, mp_obj_t filtering_fn)
{
    // We're using a modified wildfire algorithm below where instead of using a
    // the stack we use a queue along with a burn mask to filter out already
    // visited pixels. For each color blob in the image, where a color blob is
    // an area of connected pixels that all are within a threshold, the algorithm
    // computes the bounding box around all those pixels, number of pixels in the
    // blob, centroid, and blob orientation. The algorithm then returns a list
    // of blobs for each set of thresholds passed in. That is, this function
    // returns a tuple of lists of blobs.

    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return mp_const_none;
    }

    mp_obj_t blob_lists[num_thresholds];

    uint8_t *mask = init_mask(&rect);
    stack_queue_t *sq = init_stack_queue(&rect);

    for (int n = 0; n < num_thresholds; n++) {
        blob_lists[n] = mp_obj_new_list(4, NULL); // 4 is just the intial list size guess
        mp_obj_list_set_len(blob_lists[n], 0);
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j); // in img
                int y = (rect.y + i); // in img
                if (get_not_mask_pixel(&rect, mask, j, i) // in roi
                &&  threshold(img, x, y, l_thresholds[n], h_thresholds[n], invert)) { // in img
                    int blob_x1 = x;
                    int blob_y1 = y;
                    int blob_x2 = x;
                    int blob_y2 = y;
                    int blob_pixels = 1;
                    int blob_cx = x;
                    int blob_cy = y;
                    int blob_a = x*x; // equal to (x-mx)^2
                    int blob_b = x*y; // equal to (x-mx)*(y-my)
                    int blob_c = y*y; // equal to (y-my)^2
                    set_mask_pixel(&rect, mask, j, i); // in roi
                    stack_queue_push(sq, x, y); // in img
                    do {
                        point_t p = stack_queue_pop(sq);
                        for (int a = -1; a <= 1; a++) {
                            for (int b = -1; b <= 1; b++) {
                                int c = (p.x + b); // in img
                                int d = (p.y + a); // in img
                                int e = (c - rect.x); // in roi
                                int f = (d - rect.y); // in roi
                                if (IM_X_INSIDE(&rect, e) // in roi
                                &&  IM_Y_INSIDE(&rect, f) // in roi
                                &&  get_not_mask_pixel(&rect, mask, e, f) // in roi
                                &&  threshold(img, c, d, l_thresholds[n], h_thresholds[n], invert)) { // in img
                                    blob_x1 = IM_MIN(blob_x1, c);
                                    blob_y1 = IM_MIN(blob_y1, d);
                                    blob_x2 = IM_MAX(blob_x2, c);
                                    blob_y2 = IM_MAX(blob_y2, d);
                                    blob_pixels += 1;
                                    blob_cx += c;
                                    blob_cy += d;
                                    blob_a += c*c;
                                    blob_b += c*d;
                                    blob_c += d*d;
                                    set_mask_pixel(&rect, mask, e, f); // in roi
                                    stack_queue_push(sq, c, d); // in img
                                }
                            }
                        }
                    } while(stack_queue_not_empty(sq));
                    int mx = (blob_cx/blob_pixels); // x centroid
                    int my = (blob_cy/blob_pixels); // y centroid
                    // The below equations were derived by translating the orientation
                    // calculation from a double pass algorithm to single pass.
                    blob_a -= (mx*blob_cx)+(mx*blob_cx);
                    blob_a += blob_pixels*mx*mx;
                    blob_b -= (mx*blob_cy)+(my*blob_cx);
                    blob_b += blob_pixels*mx*my;
                    blob_c -= (my*blob_cy)+(my*blob_cy);
                    blob_c += blob_pixels*my*my;
                    // Compute the final blob orientation from a, b, and c sums.
                    float o = ((blob_a!=blob_c)?fast_atan2f(blob_b,blob_a-blob_c):0.0)/2.0;
                    mp_obj_t blob_tuple[10];
                    blob_tuple[0] = mp_obj_new_int(blob_x1);
                    blob_tuple[1] = mp_obj_new_int(blob_y1);
                    blob_tuple[2] = mp_obj_new_int(blob_x2-blob_x1+1);
                    blob_tuple[3] = mp_obj_new_int(blob_y2-blob_y1+1);
                    blob_tuple[4] = mp_obj_new_int(blob_pixels);
                    blob_tuple[5] = mp_obj_new_int(mx);
                    blob_tuple[6] = mp_obj_new_int(my);
                    blob_tuple[7] = mp_obj_new_float(o);
                    blob_tuple[8] = mp_obj_new_int(1<<n);
                    blob_tuple[9] = mp_obj_new_int(1);
                    mp_obj_t blob_tuple_obj = mp_obj_new_tuple(10, blob_tuple);
                    if (filtering_fn != MP_OBJ_NULL) {
                        if (mp_obj_is_true(mp_call_function_2(filtering_fn, img_obj, blob_tuple_obj))) {
                            mp_obj_list_append(blob_lists[n], blob_tuple_obj);
                        } else {
                            mp_obj_tuple_del(blob_tuple_obj);
                        }
                    } else {
                        if (blob_pixels >= ((img->w*img->h)/1000)) {
                            mp_obj_list_append(blob_lists[n], blob_tuple_obj);
                        } else {
                            mp_obj_tuple_del(blob_tuple_obj);
                        }
                    }
                }
            }
        }
    }

    deinit_stack_queue();
    deinit_mask();

    return mp_obj_new_tuple(num_thresholds, blob_lists);
}

mp_obj_t imlib_find_markers(mp_obj_t img_obj, mp_obj_t blob_lists_obj, int margin, mp_obj_t filtering_fn)
{
    // After you have a list of blobs this function will merge blobs from the
    // different colors lists that intersect into one blob. The new merged big
    // blob will have a bounding box that surronds all the merged blobs, pixels will
    // include all the blobs, and centroids/orientations are averaged. Additionally,
    // the new blob will have an extra code value with a bit set for each color
    // that was merged into the blob along with the number of blobs merged. The
    // color code provides a nice and easy user controllable way to get an idea
    // of what colors are in a merged blob.

    mp_uint_t blob_l_len;
    mp_obj_t *blob_l;
    mp_obj_get_array(blob_lists_obj, &blob_l_len, &blob_l);
    if (!blob_l_len) return mp_const_none;

    mp_uint_t blob_lists_len[blob_l_len];
    mp_obj_t *blob_lists[blob_l_len];

    rectangle_t rect; // reusing mask from above - so we need a fake rect obj.
    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = blob_l_len;

    for (mp_uint_t i = 0; i < blob_l_len; i++) {
        mp_obj_get_array(blob_l[i], &blob_lists_len[i], &blob_lists[i]);
        rect.w = IM_MAX(rect.w, blob_lists_len[i]); // find longest list
    }
    if (!rect.w) return mp_const_none;

    uint8_t *mask = init_mask(&rect);

    mp_obj_t out = mp_obj_new_list(4, NULL); // 4 is just the intial list size guess
    mp_obj_list_set_len(out, 0);
    for (mp_uint_t i = 0; i < blob_l_len; i++) {
        for (mp_uint_t j = 0; j < blob_lists_len[i]; j++) {
            if (get_not_mask_pixel(&rect, mask, j, i)) {
                set_mask_pixel(&rect, mask, j, i);

                mp_obj_t *temp0;
                mp_obj_get_array_fixed_n(blob_lists[i][j], 10, &temp0);

                int blob_x = mp_obj_get_int(temp0[0]); // rect x
                int blob_y = mp_obj_get_int(temp0[1]); // rect y
                int blob_w = mp_obj_get_int(temp0[2]); // rect w
                int blob_h = mp_obj_get_int(temp0[3]); // rect h
                int blob_pixels = mp_obj_get_int(temp0[4]); // pixels
                int blob_cx = mp_obj_get_int(temp0[5]); // centroid x
                int blob_cy = mp_obj_get_int(temp0[6]); // centroid y
                float blob_rotation = mp_obj_get_float(temp0[7]); // rotation
                int blob_code = mp_obj_get_int(temp0[8]); // code bit
                int blob_count = mp_obj_get_int(temp0[9]); // blob count

                for (mp_uint_t a = 0; a < blob_l_len; a++) {
                    for (mp_uint_t b = 0; b < blob_lists_len[a]; b++) {
                        if (get_not_mask_pixel(&rect, mask, b, a)) {

                            mp_obj_t *temp1;
                            mp_obj_get_array_fixed_n(blob_lists[a][b], 10, &temp1);

                            rectangle_t t0;
                            t0.x = blob_x - margin;
                            t0.y = blob_y - margin;
                            t0.w = blob_w + (2*margin);
                            t0.h = blob_h + (2*margin);

                            rectangle_t t1;
                            t1.x = mp_obj_get_int(temp1[0]) - margin;
                            t1.y = mp_obj_get_int(temp1[1]) - margin;
                            t1.w = mp_obj_get_int(temp1[2]) + (2*margin);
                            t1.h = mp_obj_get_int(temp1[3]) + (2*margin);

                            if (rectangle_intersects(&t0, &t1)) {
                                set_mask_pixel(&rect, mask, b, a);

                                // Compute bounding rect...
                                blob_x = IM_MIN(blob_x, t1.x);
                                blob_y = IM_MIN(blob_y, t1.y);
                                int x2_0 = t0.x+t0.w-1;
                                int x2_1 = t1.x+t1.w-1;
                                int x2 = IM_MAX(x2_0, x2_1);
                                blob_w = x2-blob_x+1;
                                int y2_0 = t0.y+t0.h-1;
                                int y2_1 = t1.y+t1.h-1;
                                int y2 = IM_MAX(y2_0, y2_1);
                                blob_h = y2-blob_y+1;
                                // Update tracking info...
                                blob_pixels += mp_obj_get_int(temp1[4]);
                                blob_cx += mp_obj_get_int(temp1[5]);
                                blob_cy += mp_obj_get_int(temp1[6]);
                                blob_rotation += mp_obj_get_float(temp1[7]);
                                blob_code |= mp_obj_get_int(temp1[8]);
                                blob_count += mp_obj_get_int(temp1[9]);
                            }
                        }
                    }
                }
                blob_cx /= blob_count;
                blob_cy /= blob_count;
                blob_rotation /= blob_count;
                // Build output object.
                mp_obj_t blob_tuple[10];
                blob_tuple[0] = mp_obj_new_int(blob_x);
                blob_tuple[1] = mp_obj_new_int(blob_y);
                blob_tuple[2] = mp_obj_new_int(blob_w);
                blob_tuple[3] = mp_obj_new_int(blob_h);
                blob_tuple[4] = mp_obj_new_int(blob_pixels);
                blob_tuple[5] = mp_obj_new_int(blob_cx);
                blob_tuple[6] = mp_obj_new_int(blob_cy);
                blob_tuple[7] = mp_obj_new_float(blob_rotation);
                blob_tuple[8] = mp_obj_new_int(blob_code);
                blob_tuple[9] = mp_obj_new_int(blob_count);
                mp_obj_t blob_tuple_obj = mp_obj_new_tuple(10, blob_tuple);
                if (filtering_fn != MP_OBJ_NULL) {
                    if (mp_obj_is_true(mp_call_function_2(filtering_fn, img_obj, blob_tuple_obj))) {
                        mp_obj_list_append(out, blob_tuple_obj);
                    } else {
                        mp_obj_tuple_del(blob_tuple_obj);
                    }
                } else {
                    mp_obj_list_append(out, blob_tuple_obj);
                }
            }
        }
    }

    deinit_mask();

    return out;
}
