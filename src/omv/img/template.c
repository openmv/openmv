/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Template matching with NCC (Normalized Cross Correlation).
 *
 */
#include "imlib.h"
#include "xalloc.h"
float imlib_template_match(image_t *f, image_t *t, rectangle_t *roi, int step, rectangle_t *r)
{
    int den_b=0;
    float corr=0.0f;

    // Integral image
    i_image_t sum;
    imlib_integral_image_alloc(&sum, f->w, f->h);
    imlib_integral_image(f, &sum);

    // Normalized sum of squares of the template
    int t_mean = imlib_image_mean(t);
    for (int i=0; i < (t->w*t->h); i++) {
        int c = (int)t->data[i]-t_mean;
        den_b += c*c;
    }

    for (int v=roi->y; v < roi->y + roi->h - t->h; v+=step) {
    for (int u=roi->x; u < roi->x + roi->w - t->w; u+=step) {
        int num = 0;
        int den_a=0;

        // The mean of the current patch
        uint32_t f_sum = imlib_integral_lookup(&sum, u, v, t->w, t->h);
        uint32_t f_mean = f_sum / (t->w*t->h);

        // Normalized sum of squares of the image
        for (int y=v; y<t->h+v; y++) {
            for (int x=u; x<t->w+u; x++) {
                int a = (int)f->data[y*f->w+x]-f_mean;
                int b = (int)t->data[(y-v)*t->w+(x-u)]-t_mean;
                num += a*b;
                den_a += a*a;
            }
        }

        // Find normalized cross-correlation
        float c = num/(fast_sqrtf(den_a) *fast_sqrtf(den_b));

        if (c > corr) {
            corr = c;
            r->x = u;
            r->y = v;
            r->w = t->w;
            r->h = t->h;
        }
    }
    }

    imlib_integral_image_free(&sum);
    return corr;
}
