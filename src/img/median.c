#include <libmp.h>
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

#define SWAP(x)\
   ({ uint16_t _x = (x); \
    (((_x & 0xff)<<8 |(_x & 0xff00) >> 8));})

typedef struct {
    int n;
    union {
        int h[2];
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
            uint16_t c = SWAP(((uint16_t*)im->pixels)[i*im->w+col]);
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
            uint16_t c = SWAP(((uint16_t*)im->pixels)[i*im->w+col]);
            h->r[R(c)]++;
            h->g[G(c)]++;
            h->b[B(c)]++;
            h->n++;
        }
    }
}

void init_histo(image_t *im, int row, int size, histo_t *h)
{
    //memset(h, 0, sizeof(histo_t));
    h->n = 0;
    h->h[0] = h->h[1]=0;
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
    int k_rows = size+1;
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
            data[(row%k_rows)*in->w+col] = SWAP(((r << 11) | (g << 5) | b));
        }
        if ((row+1)%k_rows==0) {
            memcpy((uint16_t*)in->data+((row/k_rows)*k_rows*in->w), data, (k_rows*in->w*sizeof(*data)));
        }
    }

    xfree(h);
    xfree(data);
}

#if 0
void median_filter_gs(image_t *in, int size)
{
    histo_t *h = xalloc(sizeof(*h));
    uint8_t *data = (uint8_t*) (in->data+(in->w * in->h*2));

    for (int row = 0; row<in->h; row ++) {
        for (int col = 0; col<in->w; col++) {
            if (!col) {
                init_histo(in, row, size, h);
            } else {
                del_pixels(in, row, col - size, size, h);
                add_pixels(in, row, col + size, size, h);
            }
            data[row*in->w+col]=(uint8_t)median(h->h, h->n);
        }
    }
    memcpy(in->data, data, (in->w*in->h));

}
#else
void median_filter_gs(image_t *in, int size)
{
    int k_rows = (2*size+1)*2;
    histo_t *h = xalloc(sizeof(*h));
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
#endif

void imlib_median_filter(image_t *in, int size)
{
    if (in->bpp == 1) {
        //median_filter_test(in, size);
        median_filter_gs(in, size);
    } else {
        median_filter_rgb(in, size);
    }
}
