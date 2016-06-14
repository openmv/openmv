/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * FFT LIB - can do 1024 point real FFTs and 512 point complex FFTs
 *
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
#endif /* __FFT_H__ */
