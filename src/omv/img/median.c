/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * O(N) median_gs filter with histograms.
 *
 */
#include <arm_math.h>
#include "imlib.h"
#include "fb_alloc.h"

typedef struct {
    int n;
    union {
        int h[256];
        struct {
            int r[32];
            int g[64];
            int b[32];
        };
    };
} histo_t;

void del_pixels(image_t * im, int row, int col, int size, histo_t *h)
{
    if (im->bpp==1) {
        for (int i = row - size; i <= row + size && i < im->h; i++) {
            if (i < 0) continue;
            h->h[im->pixels[i*im->w+col]]--;
            h->n--;
        }
    } else {
        for (int i = row - size; i <= row + size && i < im->h; i++) {
            if (i < 0) continue;
            uint16_t c = ((uint16_t*)im->pixels)[i*im->w+col];
            h->r[IM_R565(c)]--;
            h->g[IM_G565(c)]--;
            h->b[IM_B565(c)]--;
            h->n--;
        }
    }
}

void add_pixels(image_t * im, int row, int col, int size, histo_t *h)
{
    if (im->bpp==1) {
        for (int i = row - size; i <= row + size && i < im->h; i++) {
            if (i < 0) continue;
            h->h[im->pixels[i*im->w+col]]++;
            h->n++;
        }
    } else {
        for (int i = row - size; i <= row + size && i < im->h; i++) {
            if (i < 0) continue;
            uint16_t c = ((uint16_t*)im->pixels)[i*im->w+col];
            h->r[IM_R565(c)]++;
            h->g[IM_G565(c)]++;
            h->b[IM_B565(c)]++;
            h->n++;
        }
    }
}

void init_histo(image_t *im, int row, int size, histo_t *h)
{
    memset(h, 0, sizeof(histo_t));
    for (int j = 0; j < size && j < im->w; j++) {
        add_pixels(im, row, j, size, h);
    }
}

uint8_t median_gs(histo_t *h)
{
    int i, n;
    for (i=0, n=h->n/2; (n -= h->h[i]) >0; i++) {
    }
    return i;
}

uint16_t median_rgb(histo_t *h)
{
    int rx = h->n/2;
    int gx = h->n/2;
    int bx = h->n/2;
    uint16_t r=0, g=0, b=0;

    for (int i=0; (rx>0 || gx>0 || bx>0); i++) {
        if (rx > 0) {
            rx -= h->r[i];
            r = i;
        }

        if (gx > 0) {
            gx -= h->g[i];
            g = i;
        }

        if (bx > 0) {
            bx -= h->b[i];
            b = i;
        }
    }

    return IM_RGB565(r, g, b);
}

void median_filter_rgb(image_t *in, int size)
{
    int k_rows = 24;
    histo_t *h = fb_alloc(sizeof(*h));
    uint16_t *dst = (uint16_t*) in->data;
    uint16_t *data = fb_alloc(in->w * k_rows * sizeof(*data));

    for (int row=0; row<in->h; row++) {
        init_histo(in, row, size, h);
        data[(row%k_rows)*in->w+size] = median_rgb(h);

        for (int col=0; col<in->w; col++) {
            del_pixels(in, row, col - size, size, h);
            add_pixels(in, row, col + size, size, h);
            data[(row%k_rows)*in->w+col] = median_rgb(h);
        }

        if ((row+1)%k_rows==0) {
            memcpy(dst, data, (in->w*(k_rows-size)*2));
            dst += (in->w*(k_rows-size));
        }

        if ((row>size && (row+1)%k_rows==size) || row+1==in->h){
            memcpy(dst, data+in->w*(k_rows-size), (in->w*size*2));
            dst += (in->w*size);
        }

    }
    fb_free_all();
}

void median_filter_gs(image_t *in, int size)
{
    int k_rows = 24;
    histo_t *h = fb_alloc(sizeof(*h));
    uint8_t *dst = (uint8_t*) in->data;
    uint8_t *data = fb_alloc(in->w * k_rows * sizeof(*data));

    for (int row=0; row<in->h; row++) {
        init_histo(in, row, size, h);
        data[(row%k_rows)*in->w+size] = median_gs(h);

        for (int col=0; col<in->w; col++) {
            del_pixels(in, row, col - size, size, h);
            add_pixels(in, row, col + size, size, h);
            data[(row%k_rows)*in->w+col] = median_gs(h);
        }

        if ((row+1)%k_rows==0) {
            memcpy(dst, data, (in->w*(k_rows-size)));
            dst += (in->w*(k_rows-size));
        }

        if ((row>size && (row+1)%k_rows==size) || row+1==in->h){
            memcpy(dst, data+in->w*(k_rows-size), (in->w*size));
            dst += (in->w*size);
        }

    }
    fb_free_all();
}

void imlib_median_filter(image_t *in, int size)
{
    if (in->bpp == 1) {
        //median_filter_test(in, size);
        median_filter_gs(in, size);
    } else {
        median_filter_rgb(in, size);
    }
}
