/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013-2016 Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mode filtering.
 *
 */
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"

// krn_s == 0 -> 1x1 kernel
// krn_s == 1 -> 3x3 kernel
// ...
// krn_s == n -> ((n*2)+1)x((n*2)+1) kernel

void imlib_mode_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert)
{
    int brows = ksize + 1;
    uint8_t *buffer = fb_alloc(img->w * brows * img->bpp);
    if (IM_IS_GS(img)) {
        uint8_t *bins = fb_alloc(256);
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                memset(bins, 0, 256);
                int mcount = 0, mode = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint8_t pixel = IM_GET_GS_PIXEL(img, x+k, y+j);
                            bins[pixel]++;
                            if(bins[pixel] > mcount) {
                                mcount = bins[pixel];
                                mode = pixel;
                            }
                        }
                    }
                }
                // We're writing into the buffer like if it were a window.
                uint8_t pixel = mode;
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
        fb_free();
    } else {
        uint8_t *r_bins = fb_alloc(32);
        uint8_t *g_bins = fb_alloc(64);
        uint8_t *b_bins = fb_alloc(32);
        for (int y=0; y<img->h; y++) {
            for (int x=0; x<img->w; x++) {
                memset(r_bins, 0, 32);
                memset(g_bins, 0, 64);
                memset(b_bins, 0, 32);
                int r_mcount = 0, r_mode = 0;
                int g_mcount = 0, g_mode = 0;
                int b_mcount = 0, b_mode = 0;
                for (int j=-ksize; j<=ksize; j++) {
                    for (int k=-ksize; k<=ksize; k++) {
                        if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j)) {
                            const uint16_t pixel = IM_GET_RGB565_PIXEL(img, x+k, y+j);
                            int red = IM_R565(pixel);
                            int green = IM_G565(pixel);
                            int blue = IM_B565(pixel);
                            r_bins[red]++;
                            if(r_bins[red] > r_mcount) {
                                r_mcount = r_bins[red];
                                r_mode = red;
                            }
                            g_bins[green]++;
                            if(g_bins[green] > g_mcount) {
                                g_mcount = g_bins[green];
                                g_mode = green;
                            }
                            b_bins[blue]++;
                            if(b_bins[blue] > b_mcount) {
                                b_mcount = b_bins[blue];
                                b_mode = blue;
                            }
                        }
                    }
                }
                // We're writing into the buffer like if it were a window.
                uint16_t pixel = IM_RGB565(r_mode, g_mode, b_mode);
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
        fb_free();
        fb_free();
        fb_free();
    }
    fb_free();
}
