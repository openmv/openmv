/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Median filtering.
 *
 */
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"
#include "fsort.h"

void imlib_median_filter(image_t *img, const int ksize, const int percentile, bool threshold, int offset, bool invert)
{
    int n = ((ksize*2)+1)*((ksize*2)+1);
    int brows = ksize + 1;
    uint8_t *buffer = fb_alloc(img->w * brows * img->bpp);
    if (IM_IS_GS(img)) {
        uint8_t data[n];
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                uint8_t *data_ptr = data;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint8_t pixel = IM_GET_GS_PIXEL(img, x+k, y+j);
                            *data_ptr++ = pixel;
                        } else {
                            *data_ptr++ = 0;
                        }
                    }
                }
                fsort(data, n);
                int median = data[percentile];
                // We're writing into the buffer like if it were a window.
                uint8_t pixel = median;
                buffer[((y%brows)*img->w)+x] = (!threshold) ? pixel : ((((pixel-offset)<IM_GET_GS_PIXEL(img, x, y))^invert) ? 255 : 0);
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
        uint8_t r_data[n];
        uint8_t g_data[n];
        uint8_t b_data[n];
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                uint8_t *r_data_ptr = r_data;
                uint8_t *g_data_ptr = g_data;
                uint8_t *b_data_ptr = b_data;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint16_t pixel = IM_GET_RGB565_PIXEL(img, x+k, y+j);
                            *r_data_ptr++ = IM_R565(pixel);
                            *g_data_ptr++ = IM_G565(pixel);
                            *b_data_ptr++ = IM_B565(pixel);
                        } else {
                            *r_data_ptr++ = 0;
                            *g_data_ptr++ = 0;
                            *b_data_ptr++ = 0;
                        }
                    }
                }
                fsort(r_data, n);
                fsort(g_data, n);
                fsort(b_data, n);
                int r_median = r_data[percentile];
                int g_median = g_data[percentile];
                int b_median = b_data[percentile];
                // We're writing into the buffer like if it were a window.
                uint16_t pixel = IM_RGB565(r_median, g_median, b_median);
                ((uint16_t *) buffer)[((y%brows)*img->w)+x] = (!threshold) ? pixel : ((((COLOR_RGB565_TO_Y(pixel)-offset)<COLOR_RGB565_TO_Y(IM_GET_RGB565_PIXEL(img, x, y)))^invert) ? 65535 : 0);
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
