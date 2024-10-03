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
 * FFT LIB - can do 1024 point real FFTs and 512 point complex FFTs
 */
#ifndef __FFT_H__
#define __FFT_H__
#include <stdint.h>
#include "imlib.h"
typedef struct fft1d_controller {
    uint8_t *d_pointer;
    int d_len;
    int pow2;
    float *data;
} fft1d_controller_t;
void fft1d_alloc(fft1d_controller_t *controller, uint8_t *buf, int len);
void fft1d_dealloc();
void fft1d_run(fft1d_controller_t *controller);
void ifft1d_run(fft1d_controller_t *controller);
void fft1d_mag(fft1d_controller_t *controller);
void fft1d_phase(fft1d_controller_t *controller);
void fft1d_log(fft1d_controller_t *controller);
void fft1d_exp(fft1d_controller_t *controller);
void fft1d_swap(fft1d_controller_t *controller); // a.k.a MATLAB fftshift
void fft1d_run_again(fft1d_controller_t *controller); // Do FFT again on real mag/phase of the FFT.
typedef struct fft2d_controller {
    image_t *img;
    rectangle_t r;
    int w_pow2, h_pow2;
    float *data;
} fft2d_controller_t;
void fft2d_alloc(fft2d_controller_t *controller, image_t *img, rectangle_t *r);
void fft2d_dealloc();
void fft2d_run(fft2d_controller_t *controller);
void ifft2d_run(fft2d_controller_t *controller);
void fft2d_mag(fft2d_controller_t *controller);
void fft2d_phase(fft2d_controller_t *controller);
void fft2d_log(fft2d_controller_t *controller);
void fft2d_exp(fft2d_controller_t *controller);
void fft2d_swap(fft2d_controller_t *controller); // a.k.a MATLAB fftshift
void fft2d_linpolar(fft2d_controller_t *controller);
void fft2d_logpolar(fft2d_controller_t *controller);
void fft2d_run_again(fft2d_controller_t *controller); // Do FFT again on real mag/phase of the FFT.
// END
#endif /* __FFT_H__ */
