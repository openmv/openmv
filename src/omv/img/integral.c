/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Integral image.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <arm_math.h>
#include "imlib.h"
#include "xalloc.h"
#include "framebuffer.h"

void imlib_integral_image_alloc(struct integral_image *i_img, int w, int h)
{
    i_img->w = w;
    i_img->h = h;
    i_img->data = (uint32_t*) (fb->pixels+(fb->w * fb->h));
}

void imlib_integral_image(struct image *src, struct integral_image *sum)
{
    typeof(*src->data) *img_data = src->data;
    typeof(*sum->data) *sum_data = sum->data;

    // Compute first column to avoid branching
    for (int s=0, x=0; x<src->w; x++) {
        /* sum of the current row (integer) */
        s += img_data[src->w+x];
        sum_data[src->w+x] = s;
    }

    for (int y=1; y<src->h; y++) {
        /* loop over the number of columns */
        for (int s=0, x=0; x<src->w; x++) {
            /* sum of the current row (integer) */
            s += img_data[y*src->w+x];
            sum_data[y*src->w+x] = s+sum_data[(y-1)*src->w+x];
        }
    }
}

void imlib_integral_image_scaled(struct image *src, struct integral_image *sum)
{
    typeof(*src->data) *img_data = src->data;
    typeof(*sum->data) *sum_data = sum->data;

    int x_ratio = (int)((src->w<<16)/sum->w) +1;
    int y_ratio = (int)((src->h<<16)/sum->h) +1;

    // Compute first column to avoid branching
    for (int s=0, x=0; x<sum->w; x++) {
        int sx = (x*x_ratio)>>16;
        /* sum of the current row (integer) */
        s += img_data[sx];
        sum_data[x] = s;
    }

    for (int y=1; y<sum->h; y++) {
        int sy = (y*y_ratio)>>16;
        /* loop over the number of columns */
        for (int s=0, x=0; x<sum->w; x++) {
            int sx = (x*x_ratio)>>16;

            /* sum of the current row (integer) */
            s += img_data[sy*src->w+sx];
            sum_data[y*sum->w+x] = s+sum_data[(y-1)*sum->w+x];
        }
    }
}

void imlib_integral_image_sq(struct image *src, struct integral_image *sum)
{
    typeof(*src->data) *img_data = src->data;
    typeof(*sum->data) *sum_data = sum->data;

    // Compute first column to avoid branching
    for (int s=0, x=0; x<src->w; x++) {
        /* sum of the current row (integer) */
        s += img_data[src->w+x] * img_data[src->w+x];
        sum_data[src->w+x] = s;
    }

    for (int y=1; y<src->h; y++) {
        /* loop over the number of columns */
        for (int s=0, x=0; x<src->w; x++) {
            /* sum of the current row (integer) */
            s += img_data[y*src->w+x] * img_data[y*src->w+x];
            sum_data[y*src->w+x] = s+sum_data[(y-1)*src->w+x];
        }
    }

}

uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h)
{
#define PIXEL_AT(x,y)\
    (src->data[src->w*(y-1)+(x-1)])

    if (x==0 && y==0) {
        return PIXEL_AT(w,h);
    } else if (y==0) {
        return PIXEL_AT(w+x, h+y) - PIXEL_AT(x, h+y);
    } else if (x==0) {
        return PIXEL_AT(w+x, h+y) - PIXEL_AT(w+x, y);
    } else {
        return PIXEL_AT(w+x, h+y) + PIXEL_AT(x, y) - PIXEL_AT(w+x, y) - PIXEL_AT(x, h+y);
    }
#undef  PIXEL_AT
}


