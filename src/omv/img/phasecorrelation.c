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

void imlib_phasecorrelate(image_t *img0, image_t *img1, float *x_offset, float *y_offset, float *response)
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

    float sum = 0;
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
            sum += f_r;
        }
    }

    *response = max / sum; // normalize this to [0:1]

    float f_sum = 0;
    float f_off_x = 0;
    float f_off_y = 0;
    for (int i = -1; i < 1; i++) {
        for (int j = -1; j < 1; j++) {

            // Wrap around
            int new_x = off_x + j;
            if (new_x < 0) new_x += img0->w;
            if (new_x >= img0->w) new_x -= img0->w;

            // Wrap around
            int new_y = off_y + i;
            if (new_y < 0) new_y += img0->h;
            if (new_y >= img0->h) new_y -= img0->h;

            // Compute centroid.
            int index = (new_y * (w * 2)) + new_x;
            float f_r = fft0.data[index];
            f_off_x += (off_x + j) * f_r; // don't use new_x here
            f_off_y += (off_y + i) * f_r; // don't use new_y here
            f_sum += f_r;
        }
    }

    f_off_x /= f_sum;
    f_off_y /= f_sum;

    // FFT Shift X
    if (f_off_x >= (img0->w/2)) {
        *x_offset = f_off_x - img0->w;
    } else {
        *x_offset = f_off_x;
    }

    // FFT Shift Y
    if (f_off_y >= (img0->h/2)) {
        *y_offset = -(f_off_y - img0->h);
    } else {
        *y_offset = -f_off_y;
    }

    fft2d_dealloc(); // fft1
    fft2d_dealloc(); // fft0
}
