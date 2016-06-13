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
void alloc_fft1d_buffer(fft1d_controller_t *controller, uint8_t *data, int len);
void dealloc_fft1d_buffer();
void do_1dfft(fft1d_controller_t *controller);
void do_1difft(fft1d_controller_t *controller);
typedef struct fft2d_controller {
    image_t *img;
    rectangle_t r;
    int w_pow2, h_pow2;
    float *data;
} fft2d_controller_t;
void alloc_fft2d_buffer(fft2d_controller_t *controller, image_t *img, rectangle_t *r);
void dealloc_fft2d_buffer();
void do_2dfft(fft2d_controller_t *controller);
void do_2difft(fft2d_controller_t *controller);
#endif /* __FFT_H__ */
