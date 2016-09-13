/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Edge Detection.
 *
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "imlib.h"
#include "fb_alloc.h"

typedef struct gvec {
    uint16_t t;
    uint16_t g;
} gvec_t;

static const int8_t kernel_gauss_55[] = {
    2, 4,  5,  4,  2,
    4, 9,  12, 9,  4,
    5, 12, 15, 12, 5,
    4, 9,  12, 9,  4,
    2, 4,  5,  4,  2,
};


void imlib_edge_canny(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh)
{
    int w = src->w;

    gvec_t *gm = fb_alloc0(src->w*src->h*sizeof*gm);

    //1. Noise Reduction with a 5x5 Gaussian filter
    imlib_morph(src, 2, kernel_gauss_55, 1.0f/159.0f, 0.0f);

    //2. Finding Image Gradients
    for (int y=roi->y; y<roi->y+roi->h-3; y++) {
        for (int x=roi->x; x<roi->x+roi->w-3; x++) {
            int vx=0, vy=0;
            // sobel kernel in the horizontal direction
            vx  = src->data[(y+0)*w+x+0]
                - src->data[(y+0)*w+x+2]
                + (src->data[(y+1)*w+x+0]<<1)
                - (src->data[(y+1)*w+x+2]<<1)
                + src->data[(y+2)*w+x+0]
                - src->data[(y+2)*w+x+2];

            // sobel kernel in the vertical direction
            vy  = src->data[(y+0)*w+x+0]
                + (src->data[(y+0)*w+x+1]<<1)
                + src->data[(y+0)*w+x+2]
                - src->data[(y+2)*w+x+0]
                - (src->data[(y+2)*w+x+1]<<1)
                - src->data[(y+2)*w+x+2];

            // Find magnitude
            int g = (int) fast_sqrtf(vx*vx + vy*vy);
            // Find the direction and round angle to 0, 45, 90 or 135
            int t = (int) fast_fabsf((atan2f(vy, vx)*180.0f/M_PI));
            if (t < 22) {
                t = 0;
            } else if (t < 67) {
                t = 45;
            } else if (t < 112) {
                t = 90;
            } else if (t < 160) {
                t = 135;
            } else if (t <= 180) {
                t = 0;
            }

            gm[(y+1)*w+(x+1)].t = t;
            gm[(y+1)*w+(x+1)].g = g;
        }
    }

    // 3. Hysteresis Thresholding
    for (int y=roi->y; y<roi->y+roi->h-3; y++) {
        for (int x=roi->x; x<roi->x+roi->w-3; x++) {
            int i = (y+1)*w+(x+1);
            gvec_t *vc = &gm[i];
            if (vc->g >= high_thresh) {
                vc->g = vc->g;
            } else if (vc->g < low_thresh) {
                vc->g = 0;
            } else if (gm[(y+0)*w+(x+0)].g >= high_thresh ||
                       gm[(y+0)*w+(x+1)].g >= high_thresh ||
                       gm[(y+0)*w+(x+2)].g >= high_thresh ||
                       gm[(y+1)*w+(x+0)].g >= high_thresh ||
                       gm[(y+1)*w+(x+2)].g >= high_thresh ||
                       gm[(y+2)*w+(x+0)].g >= high_thresh ||
                       gm[(y+2)*w+(x+1)].g >= high_thresh ||
                       gm[(y+2)*w+(x+2)].g >= high_thresh) {
                vc->g = vc->g;
            } else {
                vc->g = 0;
            }
        }
    }

    // Clear image data
    memset(src->data, 0, src->w*src->h);

    // 4. Non-maximum Suppression
    for (int y=roi->y; y<roi->y+roi->h-3; y++) {
        for (int x=roi->x; x<roi->x+roi->w-3; x++) {
            int i = (y+1)*w+(x+1);
            gvec_t *va=NULL, *vb=NULL, *vc = &gm[i];

            switch (vc->t) {
                case 0: {
                    va = &gm[(y+1)*w+(x+0)];
                    vb = &gm[(y+1)*w+(x+2)];
                    break;
                }

                case 45: {
                    va = &gm[(y+2)*w+(x+0)];
                    vb = &gm[(y+0)*w+(x+2)];
                    break;
                }

                case 90: {
                    va = &gm[(y+2)*w+(x+1)];
                    vb = &gm[(y+0)*w+(x+1)];
                    break;
                }

                case 135: {
                    va = &gm[(y+2)*w+(x+2)];
                    vb = &gm[(y+0)*w+(x+0)];
                    break;
                }
            }

            if (vc->g > va->g && vc->g > vb->g) {
                src->data[i] = 255;
            } else {
                src->data[i] = 0;
            }
        }
    }

    fb_free();
}
