/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Min/Max/Midpoint Filtering.
 *
 */
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"

// krn_s == 0 -> 1x1 kernel
// krn_s == 1 -> 3x3 kernel
// ...
// krn_s == n -> ((n*2)+1)x((n*2)+1) kernel

// bias == 0 to 256 -> 0.0 to 1.0 (0.0==min filter, 1.0==max filter)

void imlib_midpoint_filter(image_t *img, const int ksize, const int bias, bool threshold, int offset, bool invert)
{
    int min_bias = (256-bias);
    int max_bias = bias;
    int brows = ksize + 1;
    uint8_t *buffer = fb_alloc(img->w * brows * img->bpp);
    if (IM_IS_GS(img)) {
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                int min = 255, max = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint8_t pixel = IM_GET_GS_PIXEL(img, x+k, y+j);
                            min = IM_MIN(min, pixel);
                            max = IM_MAX(max, pixel);
                        }
                    }
                }
                // We're writing into the buffer like if it were a window.
                int pixel = ((min*min_bias)+(max*max_bias))>>8;
                buffer[((y%brows)*img->w)+x] =  (!threshold) ? pixel : ((((pixel-offset)<IM_GET_GS_PIXEL(img, x, y))^invert) ? 255 : 0);
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
                int r_min = 255, r_max = 0;
                int g_min = 255, g_max = 0;
                int b_min = 255, b_max = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint16_t pixel = IM_GET_RGB565_PIXEL(img, x+k, y+j);
                            int red = IM_R565(pixel);
                            int green = IM_G565(pixel);
                            int blue = IM_B565(pixel);
                            r_min = IM_MIN(r_min, red);
                            r_max = IM_MAX(r_max, red);
                            g_min = IM_MIN(g_min, green);
                            g_max = IM_MAX(g_max, green);
                            b_min = IM_MIN(b_min, blue);
                            b_max = IM_MAX(b_max, blue);
                        }
                    }
                }
                // We're writing into the buffer like if it were a window.
                uint16_t pixel = IM_RGB565(((r_min*min_bias)+(r_max*max_bias))>>8,
                                           ((g_min*min_bias)+(g_max*max_bias))>>8,
                                           ((b_min*min_bias)+(b_max*max_bias))>>8);
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
