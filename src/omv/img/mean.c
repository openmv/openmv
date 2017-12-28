/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mean filtering.
 *
 */
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"

// krn_s == 0 -> 1x1 kernel
// krn_s == 1 -> 3x3 kernel
// ...
// krn_s == n -> ((n*2)+1)x((n*2)+1) kernel

void imlib_mean_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert)
{
    int n = ((ksize*2)+1)*((ksize*2)+1);
    int brows = ksize + 1;
    uint8_t *buffer = fb_alloc(img->w * brows * img->bpp);
    if (IM_IS_GS(img)) {
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                int acc = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint8_t pixel = IM_GET_GS_PIXEL(img, x+k, y+j);
                            acc += pixel;
                        }
                    }
                }
                // We're writing into the buffer like if it were a window.
                uint8_t pixel = acc/n;
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
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                int r_acc = 0;
                int g_acc = 0;
                int b_acc = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint16_t pixel = IM_GET_RGB565_PIXEL(img, x+k, y+j);
                            r_acc += IM_R565(pixel);
                            g_acc += IM_G565(pixel);
                            b_acc += IM_B565(pixel);
                        }
                    }
                }
                // We're writing into the buffer like if it were a window.
                uint16_t pixel = IM_RGB565(r_acc/n, g_acc/n, b_acc/n);
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
