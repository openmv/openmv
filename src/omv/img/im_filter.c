/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Filter Functions.
 * 
 * Filter functions bypass the default line processing in sensor.c, and pre-process lines.
 * Processing is done on the fly, i.e. filters are called from after each line is received.
 *
 * Note: bpp is the target bpp, not the line bpp (the line is always 2 bytes per pixel) if the target bpp is 1
 * it means the image currently being read is going to be Grayscale, and the function needs to output w*1bpp.
 */
#include <stdint.h>
#include "imlib.h"

// RGB565 to YUV table
extern const int8_t yuv_table[196608];

void im_filter_bw(uint8_t *src, uint8_t *dst, int size, int bpp, void *args)
{
    int lower = ((int*)args)[0];
    int upper = ((int*)args)[1];

    if (bpp == 1) {
        // Extract Y channel from YUV and process
        for (int i=0; i<size; i++) {
            dst[i] = (src[i<<1] >= lower && src[i<<1] <= upper) ? 0xFF : 0;
        }
    } else {
        // Lookup Y channel from RGB2YUV 
        uint16_t *srcrgb = (uint16_t*) src;
        uint16_t *dstrgb = (uint16_t*) dst;
        for (int i=0; i<size; i++) {
            int y = yuv_table[srcrgb[i] * 3 + 0]+128;
            dstrgb[i] = (y >= lower && y <= upper) ? 0xFFFF : 0;
        }
    }
}


void im_filter_skin(uint8_t *src, uint8_t *dst, int size, int bpp, void *args)
{
    if (bpp == 1) {
        // Kinda works
        for (int i=0; i<size; i+=2, src+=4) {
            uint8_t y0 = src[0];
            uint8_t u  = src[1];
            uint8_t y1 = src[2];
            uint8_t v  = src[3];
            // YCbCr
            dst[i+0] = (y0>80 && u>85 && u<135 && v>135 && v<180) ? 255 : 0;
            dst[i+1] = (y1>80 && u>85 && u<135 && v>135 && v<180) ? 255 : 0;

        }
    } else {
        // This doesn't work
        uint16_t *srcrgb = (uint16_t*) src;
        uint16_t *dstrgb = (uint16_t*) dst;
        for (int i=0; i<size; i++) {
            int r = IM_R528(srcrgb[i]); 
            int g = IM_G628(srcrgb[i]); 
            int b = IM_B528(srcrgb[i]); 
            //int y = yuv_table[srcrgb[i] * 3 + 0] + 128;
            int u = (int) yuv_table[srcrgb[i] * 3 + 1] + 128;
            int v = (int) yuv_table[srcrgb[i] * 3 + 2] + 128;
            // From "Skin Segmentation Using YUV and RGB Color Spaces" Zaher Hamid Al-Tairi
            dstrgb[i] = (u>80  && u<130 &&
                         v>136 && v<200 && v>u   &&
                         r>80  && g>30  && b>15  &&
                         (((r-g)*(r-g)) > 225)) ? srcrgb[i] : 0;
        }
    }
}
