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

    fft2d_alloc(&fft0, img0, &roi0);
    fft2d_alloc(&fft1, img1, &roi1);

    fft2d_run(&fft0);
    fft2d_run(&fft1);

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

    ifft2d_run(&fft0);

    float max = 0;
    int off_x = 0;
    int off_y = 0;
    for (int i = 0; i < img0->h; i++) {
        for (int j = 0; j < img0->w; j++) {
            // Note that the output of the FFT is packed with real data in both
            // the real and imaginary parts...
            int index = (i * (w * 2)) + j; // correct!
            float f_r = fft0.data[index];
            if (f_r > max) {
                max = f_r;
                off_x = j;
                off_y = i;
            }
        }
    }

    if (off_x > (img0->w/2)) {
        *x_offset = img0->w - off_x;
    } else {
        *x_offset = -off_x;
    }

    if (off_y > (img0->h/2)) {
        *y_offset = img0->h - off_y;
    } else {
        *y_offset = -off_y;
    }

    fft2d_dealloc(); // fft1
    fft2d_dealloc(); // fft0
}
