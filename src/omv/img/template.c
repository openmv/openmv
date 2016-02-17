/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Template matching with NCC (Normalized Cross Correlation).
 *
 */
#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>
float imlib_template_match(struct image *f,  struct image *t, struct rectangle *r)
{
    int den_b=0;
    float corr =0.0;

    struct integral_image sum;
    struct integral_image *f_imgs=&sum;

    /* get integeral image */
    imlib_integral_image_alloc(f_imgs, f->w, f->h);
    imlib_integral_image(f, f_imgs);

    /* get normalized template sum of squares */
    int t_mean = imlib_image_mean(t);
    for (int i=0; i < (t->w*t->h); i++) {
        int c = (int)t->data[i]-t_mean;
        den_b += c*c;
    }

    for (int v=0; v < f->h - t->h; v+=3) {
    for (int u=0; u < f->w - t->w; u+=3) {
        int num = 0;
        int den_a=0;
        uint32_t f_sum  =imlib_integral_lookup(f_imgs, u, v, t->w, t->h);
        int f_mean = f_sum / (t->w*t->h);

        for (int y=v; y<t->h+v; y++) {
            for (int x=u; x<t->w+u; x++) {
                int a = (int)f->data[y*f->w+x]-f_mean;
                int b = (int)t->data[(y-v)*t->w+(x-u)]-t_mean;
                num += a*b;
                den_a += a*a;
            }
        }

        /* this overflows */
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

    imlib_integral_image_free(f_imgs);
    return corr;
}
