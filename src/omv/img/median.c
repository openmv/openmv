/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * O(N) median filter with histograms.
 *
 */
#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>
#define R8(p) \
    (uint8_t)((p>>11) * 255/31)
#define G8(p) \
    (uint8_t)(((p>>5)&0x3F)* 255/63)
#define B8(p) \
    (uint8_t)((p&0x1F) * 255/31)

#define R(p) \
    (uint8_t)((p>>11)&0x1F)
#define G(p) \
    (uint8_t)((p>>5) &0x3F)
#define B(p) \
    (uint8_t)(p&0x1F)

#define SWAP16(x) __REV16(x)

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
            uint16_t c = SWAP16(((uint16_t*)im->pixels)[i*im->w+col]);
            h->r[R(c)]--;
            h->g[G(c)]--;
            h->b[B(c)]--;
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
            uint16_t c = SWAP16(((uint16_t*)im->pixels)[i*im->w+col]);
            h->r[R(c)]++;
            h->g[G(c)]++;
            h->b[B(c)]++;
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

uint16_t median(const int *x, int n)
{
    uint16_t i;
    for (n /= 2, i = 0;(n -= x[i]) > 0; i++);
    return i;
}


void median_filter_rgb(image_t *in, int size)
{
    uint16_t r,g,b;
    //int k_rows = size+1;
    int k_rows = (2*size+1)*2;
    histo_t *h = xalloc(sizeof(*h));
    uint16_t *data = xalloc(in->w * k_rows * sizeof(*data));

    for (int row = 0; row<in->h; row++) {
        for (int col = 0; col<in->w; col++) {
            if (!col) {
                init_histo(in, row, size, h);
            } else {
                del_pixels(in, row, col - size, size, h);
                add_pixels(in, row, col + size, size, h);
            }
            r = median(h->r, h->n);
            g = median(h->g, h->n);
            b = median(h->b, h->n);
            data[(row%k_rows)*in->w+col] = SWAP16(((r << 11) | (g << 5) | b));
        }
        if ((row+1)%k_rows==0) {
            memcpy((uint16_t*)in->data+((row/k_rows)*k_rows*in->w), data, (k_rows*in->w*sizeof(*data)));
        }
    }

    xfree(h);
    xfree(data);
}

void median_filter_gs(image_t *in, int size)
{
    histo_t hist;
    histo_t *h = &hist;
    int k_rows = (2*size+1)*2;
    uint8_t *data = xalloc(in->w * k_rows * sizeof(*data));

    for (int row = 0; row<in->h; row ++) {
        for (int col = 0; col<in->w; col++) {
            if (!col) {
                init_histo(in, row, size, h);
            } else if ((col-size) > 0 && (col+size) < in->w) {
                del_pixels(in, row, col - size, size, h);
                add_pixels(in, row, col + size, size, h);
            }
            data[(row%k_rows)*in->w+col] = (uint8_t)median(h->h, h->n);
        }
        if ((row+1)%k_rows==0) {
            memcpy(in->data+((row/k_rows)*k_rows*in->w), data, (k_rows*in->w*sizeof(*data)));
        }
    }
    xfree(h);
    xfree(data);
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
