/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Phase Correlation Function.
 *
 */
#include "fmath.h"
#include "fft.h"

void imlib_phasecorrelate(image_t *img0, image_t *img1, int *x_offset, int *y_offset)
{
    fft2d_controller_t fft0, fft1;
    rectangle_t roi0, roi1;

    roi0.x = 0;
    roi0.y = 0;
    roi0.w = img0->w;
    roi0.h = img0->h;

    roi1.x = 0;
    roi1.y = 0;
    roi1.w = img1->w;
    roi1.h = img1->h;

    alloc_fft2d_buffer(&fft0, img0, &roi0);
    alloc_fft2d_buffer(&fft1, img1, &roi1);

    do_2dfft(&fft0);
    do_2dfft(&fft1);

    int w = (1 << fft0.w_pow2);
    int h = (1 << fft0.h_pow2);
    for (int i = 0, j = w * h * 2; i < j; i += 2) {
        float ga_r = fft0.data[i+0];
        float ga_i = fft0.data[i+1];
        float gb_r = fft1.data[i+0];
        float gb_i = -fft1.data[i+1]; // complex conjugate...
        float hp_r = (ga_r*gb_r) - (ga_i*gb_i); // hadamard product
        float hp_i = (ga_i*gb_r) + (ga_r*gb_i); // hadamard product
        float mag = fast_sqrtf((hp_r*hp_r)+(hp_i*hp_i)); // magnitude
        // Replace first fft with phase correlation...
        fft0.data[i+0] = hp_r / mag;
        fft0.data[i+1] = hp_i / mag;
    }

    do_2difft(&fft0);

    float max = 0;
    int off_x = 0;
    int off_y = 0;
    for (int i = 0; i < img0->h; i++) {
        for (int j = 0; j < img0->w; j++) {
            int index = ((i * img0->w) + j) * 2;
            float f_r = fft0.data[index+0];
            // float f_i = fft0.data[index+1];
            // float mag = fast_sqrtf((f_r*f_r)+(f_i*f_i));
            if (f_r > max) {
                max = f_r;
                off_x = j;
                off_y = i;
            }
        }
    }

    if (off_x > (img0->w/2)) {
        *x_offset = off_x - img0->w;
    } else {
        *x_offset = off_x;
    }

    if (off_y > (img0->h/2)) {
        *y_offset = off_y - img0->h;
    } else {
        *y_offset = off_y;
    }

    dealloc_fft2d_buffer(); // fft1
    dealloc_fft2d_buffer(); // fft0
}
