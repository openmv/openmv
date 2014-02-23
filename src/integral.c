#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arm_math.h>
#include "imlib.h"

void imlib_integral_image(struct image *src, struct integral_image *sum)
{
    int x, y, s,t;
    unsigned char *data = src->pixels;
    typeof(*sum->data) *sumData = sum->data;

    for (y=0; y<src->h; y++) {
        s = 0;
        /* loop over the number of columns */
        for (x=0; x<src->w; x++) {
            /* sum of the current row (integer)*/
            s += data[y*src->w+x];
            t = s;
            if (y != 0) {
                t += sumData[(y-1)*src->w+x];
            }
            sumData[y*src->w+x]=t;
        }
    }
}

void imlib_integral_image_sq(struct image *src, struct integral_image *sum)
{
    int x, y, s,t;
    typeof(*src->data) *data = src->data;
    typeof(*sum->data) *sumData = sum->data;

    for (y=0; y<src->h; y++) {
        s = 0;
        /* loop over the number of columns */
        for (x=0; x<src->w; x++) {
            /* sum of the current row (integer)*/
            s += (data[y*src->w+x])*(data[y*src->w+x]);
            t = s;
            if (y != 0) {
                t += sumData[(y-1)*src->w+x];
            }
            sumData[y*src->w+x]=t;
        }
    }
}

uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h)
{
#define PIXEL_AT(x,y)\
    (src->data[src->w*(y-1)+(x-1)])

    if (x==0 && y==0) {
        return PIXEL_AT(w,h);
    } else if (y==0) {
        return PIXEL_AT(w+x, h+y) - PIXEL_AT(x, h+y);
    } else if (x==0) {
        return PIXEL_AT(w+x, h+y) - PIXEL_AT(w+x, y);
    } else {
        return PIXEL_AT(w+x, h+y) + PIXEL_AT(x, y) - PIXEL_AT(w+x, y) - PIXEL_AT(x, h+y);
    }
#undef  PIXEL_AT
}


