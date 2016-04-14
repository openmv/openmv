/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Blob and color code/marker detection code...
 *
 */
#include <string.h>
#include "mdefs.h"
#include "fb_alloc.h"
#include "xalloc.h"
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

array_t *imlib_find_blobs(image_t *img,
                          int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds,
                          bool invert, rectangle_t *r,
                          bool (*f_fun)(void*,void*,color_blob_t*), void *f_fun_arg_0, void *f_fun_arg_1)
{
    // We're using a modified wildfire algorithm below where instead of using a
    // the stack we use a queue along with a burn mask to filter out already
    // visited pixels. For each color blob in the image, where a color blob is
    // an area of connected pixels that all are within a threshold, the algorithm
    // computes the bounding box around all those pixels, number of pixels in the
    // blob, centroid, and blob orientation. The algorithm then returns a list
    // of all the blobs in the image. Note that blobs can be mapped back to colors
    // by their blob code number.

    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return NULL;
    }

    uint8_t *mask = init_mask(&rect);
    stack_queue_t *sq = init_stack_queue(&rect);

    array_t *blobs_list;
    array_alloc(&blobs_list, xfree);
    for (int n = 0; n < num_thresholds; n++) {
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
                    color_blob_t cb;
                    cb.x = blob_x1;
                    cb.y = blob_y1;
                    cb.w = blob_x2-blob_x1+1;
                    cb.h = blob_y2-blob_y1+1;
                    cb.pixels = blob_pixels;
                    cb.cx = mx;
                    cb.cy = my;
                    cb.rotation = o;
                    cb.code = 1<<n;
                    cb.count = 1;
                    // We allocate in the below code to sped things up.
                    if ((f_fun != NULL) && (f_fun_arg_0 != NULL) && (f_fun_arg_1 != NULL)) {
                        if (f_fun(f_fun_arg_0, f_fun_arg_1, &cb)) {
                            color_blob_t *cb2 = xalloc(sizeof(color_blob_t));
                            memcpy(cb2, &cb, sizeof(color_blob_t));
                            array_push_back(blobs_list, cb2);
                        }
                    } else {
                        if (blob_pixels >= ((img->w*img->h)/1000)) {
                            color_blob_t *cb2 = xalloc(sizeof(color_blob_t));
                            memcpy(cb2, &cb, sizeof(color_blob_t));
                            array_push_back(blobs_list, cb2);
                        }
                    }
                }
            }
        }
    }

    deinit_stack_queue();
    deinit_mask();

    return blobs_list;
}

array_t *imlib_find_markers(array_t *blobs_list, int margin,
                            bool (*f_fun)(void*,void*,color_blob_t*), void *f_fun_arg_0, void *f_fun_arg_1)
{
    // After you have a list of blobs this function will merge blobs that
    // intersect into one blob. The new merged big blob will have a bounding box
    // that surronds all the merged blobs, pixels will include all the blobs,
    // and centroids/orientations are averaged. Additionally, the new blob will
    // have an extra code value with a bit set for each color that was merged
    // into the blob along with the number of blobs merged. The color code
    // provides a nice and easy user controllable way to get an idea of what
    // colors are in a merged blob.

    if (!array_length(blobs_list)) return NULL;

    rectangle_t rect; // reusing mask from above - so we need a fake rect obj.
    rect.x = 0;
    rect.y = 0;
    rect.w = array_length(blobs_list);
    rect.h = 1;

    uint8_t *mask = init_mask(&rect);

    array_t *blobs_list_ret;
    array_alloc(&blobs_list_ret, xfree);
    for (int i = 0, ii = array_length(blobs_list); i < ii; i++) {
        if (get_not_mask_pixel(&rect, mask, i, 0)) {
            set_mask_pixel(&rect, mask, i, 0);

            color_blob_t *cb0 = array_at(blobs_list, i);

            int blob_x = cb0->x; // rect x
            int blob_y = cb0->y; // rect y
            int blob_w = cb0->w; // rect w
            int blob_h = cb0->h; // rect h
            int blob_pixels = cb0->pixels; // pixels
            int blob_cx = cb0->cx; // centroid x
            int blob_cy = cb0->cy; // centroid y
            float blob_rotation = cb0->rotation; // rotation
            int blob_code = cb0->code; // code bit
            int blob_count = cb0->count; // blob count

            for (int j = 0, jj = array_length(blobs_list); j < jj;) {
                if (get_not_mask_pixel(&rect, mask, j, 0)) {

                    color_blob_t *cb1 = array_at(blobs_list, j);

                    rectangle_t t0, t1;
                    t0.x = blob_x - margin;
                    t0.y = blob_y - margin;
                    t0.w = blob_w + (2*margin);
                    t0.h = blob_h + (2*margin);
                    t1.x = cb1->x - margin;
                    t1.y = cb1->y - margin;
                    t1.w = cb1->w + (2*margin);
                    t1.h = cb1->h + (2*margin);

                    if (rectangle_intersects(&t0, &t1)) {
                        set_mask_pixel(&rect, mask, j, 0);
                        // Compute bounding rect...
                        int x2_0 = blob_x+blob_w-1;
                        int x2_1 = cb1->x+cb1->w-1;
                        int x2 = IM_MAX(x2_0, x2_1);
                        blob_x = IM_MIN(blob_x, cb1->x);
                        blob_w = x2-blob_x+1;
                        int y2_0 = blob_y+blob_h-1;
                        int y2_1 = cb1->y+cb1->h-1;
                        int y2 = IM_MAX(y2_0, y2_1);
                        blob_y = IM_MIN(blob_y, cb1->y);
                        blob_h = y2-blob_y+1;
                        // Update tracking info...
                        blob_pixels += cb1->pixels;
                        blob_cx += cb1->cx;
                        blob_cy += cb1->cy;
                        blob_rotation += cb1->rotation;
                        blob_code |= cb1->code;
                        blob_count += cb1->count;
                        // Start over if we merged so we don't miss something.
                        // Since our rect has grown we have to recheck blobs
                        // that didn't intersect previously.
                        j = 0;
                        continue;
                    }
                }
                j += 1;
            }
            blob_cx /= blob_count;
            blob_cy /= blob_count;
            blob_rotation /= blob_count;
            // Build output object.
            color_blob_t cb;
            cb.x = blob_x;
            cb.y = blob_y;
            cb.w = blob_w;
            cb.h = blob_h;
            cb.pixels = blob_pixels;
            cb.cx = blob_cx;
            cb.cy = blob_cy;
            cb.rotation = blob_rotation;
            cb.code = blob_code;
            cb.count = blob_count;
            // We allocate in the below code to sped things up.
            if ((f_fun != NULL) && (f_fun_arg_0 != NULL) && (f_fun_arg_1 != NULL)) {
                if (f_fun(f_fun_arg_0, f_fun_arg_1, &cb)) {
                    color_blob_t *cb2 = xalloc(sizeof(color_blob_t));
                    memcpy(cb2, &cb, sizeof(color_blob_t));
                    array_push_back(blobs_list_ret, cb2);
                }
            } else {
                color_blob_t *cb2 = xalloc(sizeof(color_blob_t));
                memcpy(cb2, &cb, sizeof(color_blob_t));
                array_push_back(blobs_list_ret, cb2);
            }
        }
    }

    deinit_mask();

    return blobs_list_ret;
}
