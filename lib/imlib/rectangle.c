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
 * Rectangle functions.
 */
#include "imlib.h"
#include "array.h"

rectangle_t *rectangle_alloc(int16_t x, int16_t y, int16_t w, int16_t h) {
    rectangle_t *r = m_malloc(sizeof(rectangle_t));
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    return r;
}

bool rectangle_equal(rectangle_t *r1, rectangle_t *r2) {
    return ((r1->x == r2->x) && (r1->y == r2->y) && (r1->w == r2->w) && (r1->h == r2->h));
}

bool rectangle_intersects(rectangle_t *r1, rectangle_t *r2) {
    return  ((r1->x < (r2->x + r2->w)) &&
             (r1->y < (r2->y + r2->h)) &&
             ((r1->x + r1->w) > r2->x) &&
             ((r1->y + r1->h) > r2->y));
}

// Determine subimg even if it is going off the edge of the main image.
bool rectangle_subimg(image_t *img, rectangle_t *r, rectangle_t *r_out) {
    rectangle_t r_img;
    r_img.x = 0;
    r_img.y = 0;
    r_img.w = img->w;
    r_img.h = img->h;
    bool result = rectangle_intersects(&r_img, r);
    if (result) {
        int r_img_x2 = r_img.x + r_img.w;
        int r_img_y2 = r_img.y + r_img.h;
        int r_x2 = r->x + r->w;
        int r_y2 = r->y + r->h;
        r_out->x = IM_MAX(r_img.x, r->x);
        r_out->y = IM_MAX(r_img.y, r->y);
        r_out->w = IM_MIN(r_img_x2, r_x2) - r_out->x;
        r_out->h = IM_MIN(r_img_y2, r_y2) - r_out->y;
    }
    return result;
}

// This isn't for actually combining the rects standardly, but, to instead
// find the average rectangle between a bunch of overlapping rectangles.
static void rectangle_add(rectangle_t *r1, rectangle_t *r2) {
    r1->x += r2->x;
    r1->y += r2->y;
    r1->w += r2->w;
    r1->h += r2->h;
}

// This isn't for actually combining the rects standardly, but, to instead
// find the average rectangle between a bunch of overlapping rectangles.
static void rectangle_div(rectangle_t *r, int c) {
    r->x /= c;
    r->y /= c;
    r->w /= c;
    r->h /= c;
}

array_t *rectangle_merge(array_t *rectangles) {
    array_t *objects; array_alloc(&objects, m_free);
    array_t *overlap; array_alloc(&overlap, m_free);
    /* merge overlapping detections */
    while (array_length(rectangles)) {
        /* check for overlapping detections */
        rectangle_t *rect = (rectangle_t *) array_take(rectangles, 0);
        for (int j = 0; j < array_length(rectangles); j++) {
            // do not cache bound
            if (rectangle_intersects(rect, (rectangle_t *) array_at(rectangles, j))) {
                array_push_back(overlap, array_take(rectangles, j--));
            }
        }
        /* add the overlapping detections */
        int count = array_length(overlap);
        for (int i = 0; i < count; i++) {
            rectangle_t *overlap_rect = (rectangle_t *) array_pop_back(overlap);
            rectangle_add(rect, overlap_rect);
            m_free(overlap_rect);
        }
        /* average the overlapping detections */
        rectangle_div(rect, count + 1);
        array_push_back(objects, rect);
    }
    array_free(rectangles);
    array_free(overlap);
    return objects;
}

// Expands a bounding box with a point.
// After adding all points sub x from w and y from h.
void rectangle_expand(rectangle_t *r, int x, int y) {
    if (x < r->x) {
        r->x = x;
    }
    if (y < r->y) {
        r->y = y;
    }
    if (x > r->w) {
        r->w = x;
    }
    if (y > r->h) {
        r->h = y;
    }
}

float rectangle_iou(rectangle_t *r1, rectangle_t *r2) {
    int x1 = IM_MAX(r1->x, r2->x);
    int y1 = IM_MAX(r1->y, r2->y);
    int x2 = IM_MIN(r1->x + r1->w, r2->x + r2->w);
    int y2 = IM_MIN(r1->y + r1->h, r2->y + r2->h);
    int w = IM_MAX(0, x2 - x1);
    int h = IM_MAX(0, y2 - y1);
    int rect_intersection = w * h;
    int rect_union = (r1->w * r1->h) + (r2->w * r2->h) - rect_intersection;
    return ((float) rect_intersection) / ((float) rect_union);
}

// Adds a bounding box to the list of bounding boxes in descending order of score.
void rectangle_nms_add_bounding_box(list_t *bounding_boxes, bounding_box_lnk_data_t *box) {
    // Insertion sort bounding boxes by score.
    list_lnk_t *it = bounding_boxes->head;
    for (; it; it = it->next) {
        if (box->score > ((bounding_box_lnk_data_t *) it->data)->score) {
            list_insert(bounding_boxes, it, box);
            break;
        }
    }

    if (!it) {
        list_push_back(bounding_boxes, box);
    }
}

// Soft non-max supress the list of bounding boxes. Returns the maximum label index of the new list.
int rectangle_nms_get_bounding_boxes(list_t *bounding_boxes, float threshold, float sigma) {
    // Soft non-max suppression with a Gaussian is used below, as this provides the best results.
    // A Gaussian is used to apply a soft score penalty to overlapping boxes. On loop entry,
    // "bounding_boxes" is sorted, but after each iteration, the next highest score must be picked
    // again, given that the score penalty changes the order.
    float sigma_scale = (sigma > 0.0f) ? (-1.0f / sigma) : 0.0f;

    list_t nms_bounding_boxes;
    list_init(&nms_bounding_boxes, sizeof(bounding_box_lnk_data_t));

    int max_label_index = 0;

    // The first detection has the higest score since the list is sorted.
    list_lnk_t *max_it = bounding_boxes->head;
    while (list_size(bounding_boxes)) {
        bounding_box_lnk_data_t lnk_data;
        memcpy(&lnk_data, max_it->data, bounding_boxes->data_len);
        list_move_back(&nms_bounding_boxes, bounding_boxes, max_it);

        float max_score = 0.0f;
        for (list_lnk_t *it = bounding_boxes->head; it; ) {
            bounding_box_lnk_data_t *lnk_data2 = list_get_data(it);

            // Advance to next now as "it" will be invalid if we remove the current item.
            list_lnk_t *old_it = it;
            it = it->next;

            float iou = rectangle_iou(&lnk_data.rect, &lnk_data2->rect);
            // Do not use fast_expf() as it does not output 1 when it's input is 0.
            // This will cause the scores of non-overlapping bounding boxes to decay.
            lnk_data2->score *= expf(sigma_scale * iou * iou);

            if (lnk_data2->score < threshold) {
                list_remove(bounding_boxes, old_it, NULL);
            } else if (lnk_data2->score > max_score) {
                max_score = lnk_data2->score;
                max_it = old_it;
            }
        }

        // Find the maximum label index for the output list.
        max_label_index = IM_MAX(lnk_data.label_index, max_label_index);
    }

    // Set the original list pointers to equal the new list.
    memcpy(bounding_boxes, &nms_bounding_boxes, sizeof(list_t));
    return max_label_index;
}

void rectangle_map_bounding_boxes(list_t *bounding_boxes, int window_w, int window_h, rectangle_t *roi) {
    float x_scale = roi->w / ((float) window_w);
    float y_scale = roi->h / ((float) window_h);
    // MAX == KeepAspectRatioByExpanding - MIN == KeepAspectRatio
    float scale = IM_MIN(x_scale, y_scale);
    int x_offset = fast_floorf((roi->w - (window_w * scale)) / 2.0f) + roi->x;
    int y_offset = fast_floorf((roi->h - (window_h * scale)) / 2.0f) + roi->y;

    list_for_each(it, bounding_boxes) {
        rectangle_t *rect = &((bounding_box_lnk_data_t *) it->data)->rect;
        rect->x = fast_floorf((rect->x * scale) + x_offset);
        rect->y = fast_floorf((rect->y * scale) + y_offset);
        rect->w = fast_floorf(rect->w * scale);
        rect->h = fast_floorf(rect->h * scale);
    }
}
