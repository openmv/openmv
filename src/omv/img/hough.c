/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Hough Transform.
 *
 */
#include <stdio.h>
#include <math.h>
#include "imlib.h"
#include "xalloc.h"
#include "fb_alloc.h"

static inline line_t *line_alloc(int x1, int y1, int x2, int y2)
{
    line_t *l = xalloc(sizeof(line_t));
    l->x1=x1; l->y1=y1; l->x2=x2; l->y2=y2;
    return l;
}

array_t *imlib_find_lines(image_t *src, rectangle_t *roi, int threshold)
{
    array_t *lines;
    array_alloc(&lines, xfree);

    int s = src->w; // stride
    int w = roi->x+roi->w;
    int h = roi->y+roi->h;

    int htw = 180;
    int hth = sqrtf(w*w + h*h)*2;
    uint8_t *ht = fb_alloc0(htw*hth*sizeof*ht);

    // Transform to ht Space
    for (int y=roi->y; y<h; y++) {
        for (int x=roi->x; x<w; x++) {
            if (src->data[y*s+x] > 250) {
                for (int t=0; t<htw; t++) {
                    int r = (int)roundf((x*cos_table[t] + y*sin_table[t] + (hth/2)));
                    ht[r*htw+t]++;
                }
            }
        }
    }

    for(int r=0; r<hth; r++) {
        for(int t=0; t<htw; t++) {
            if (ht[r*htw+t] > threshold) {
                int x1=0, y1=0, x2=0, y2=0;
                if(t >= 45 && t <= 135) {
                    //y = (r - x cos(t)) / sin(t)
                    x1 = 0;
                    y1 = ((float)(r-(hth/2)) - (x1 * cos_table[t])) / sin_table[t];
                    x2 = w;
                    y2 = ((float)(r-(hth/2)) - (x2 * cos_table[t])) / sin_table[t];
                } else {
                    //x = (r - y sin(t)) / cos(t);
                    y1 = 0;
                    x1 = ((float)(r-(hth/2)) - (y1 * sin_table[t])) / cos_table[t];
                    y2 = h;
                    x2 = ((float)(r-(hth/2)) - (y2 * sin_table[t])) / cos_table[t];
                }
                array_push_back(lines, line_alloc(x1, y1, x2, y2));
            }
        }
    }

    fb_free();
    return lines;
}
