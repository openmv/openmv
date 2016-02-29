/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Generic image convolution function.
 *
 */
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"

// krn_s == 0 -> 1x1 kernel
// krn_s == 1 -> 3x3 kernel
// ...
// krn_s == n -> ((n*2)+1)x((n*2)+1) kernel
//
// pixel = (krn_sum / m) + b
//
// http://www.fmwconcepts.com/imagemagick/digital_image_filtering.pdf

void imlib_morph(image_t *img, const int ksize, const int8_t *krn, const float m, const int b)
{
    int brows = ksize + 1;
    uint8_t *buffer = fb_alloc(img->w * brows * img->bpp);
    if (IM_IS_GS(img)) {
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                int acc = 0;
                int ptr = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            acc += krn[ptr++] * IM_GET_GS_PIXEL(img, x+k, y+j);
                        }
                    }
                }
                acc = (acc * m) + b; // scale, offset, and clamp
                acc = IM_MAX(IM_MIN(acc, IM_MAX_GS), 0);
                // We're writing into the buffer like if it were a window.
                buffer[((y%brows)*img->w)+x] = acc;
            }
            if (y>=ksize) {
                memcpy(img->pixels+((y-ksize)*img->w),
                       buffer+(((y-ksize)%brows)*img->w),
                       img->w * sizeof(uint8_t));
            }
        }
        for (int y=img->h-ksize; y<img->h; y++) {
            memcpy(img->pixels+(y*img->w),
                   buffer+((y%brows)*img->w),
                   img->w * sizeof(uint8_t));
        }
    } else {
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                int r_acc = 0;
                int g_acc = 0;
                int b_acc = 0;
                int ptr = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                             const uint16_t pixel = IM_GET_RGB565_PIXEL(img, x+k, y+j);
                             r_acc += krn[ptr] * IM_R565(pixel);
                             g_acc += krn[ptr] * IM_G565(pixel);
                             b_acc += krn[ptr++] * IM_B565(pixel);
                        }
                    }
                }
                r_acc = (r_acc * m) + b; // scale, offset, and clamp
                r_acc = IM_MAX(IM_MIN(r_acc, IM_MAX_R5), 0);
                g_acc = (g_acc * m) + b; // scale, offset, and clamp
                g_acc = IM_MAX(IM_MIN(g_acc, IM_MAX_G6), 0);
                b_acc = (b_acc * m) + b; // scale, offset, and clamp
                b_acc = IM_MAX(IM_MIN(b_acc, IM_MAX_B5), 0);
                // We're writing into the buffer like if it were a window.
                ((uint16_t *) buffer)[((y%brows)*img->w)+x] = IM_RGB565(r_acc, g_acc, b_acc);
            }
            if (y>=ksize) {
                memcpy(((uint16_t *) img->pixels)+((y-ksize)*img->w),
                       ((uint16_t *) buffer)+(((y-ksize)%brows)*img->w),
                       img->w * sizeof(uint16_t));
            }
        }
        for (int y=img->h-ksize; y<img->h; y++) {
            memcpy(((uint16_t *) img->pixels)+(y*img->w),
                   ((uint16_t *) buffer)+((y%brows)*img->w),
                   img->w * sizeof(uint16_t));
        }
    }
    fb_free();
}
