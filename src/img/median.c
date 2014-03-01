#include <libmp.h>
#include "xalloc.h"
#include "imlib.h"
#include <math.h>
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
    int r[32];
    int g[64];
    int b[32];
} color_histo_t;

void del_pixels(image_t * im, int row, int col, int size, color_histo_t *h)
{
    int i;
    uint16_t c;

    if (col < 0 || col >= im->w) return;
    for (i = row - size; i <= row + size && i < im->h; i++) {
        if (i < 0) continue;
        c = SWAP(((uint16_t*)im->pixels)[i*im->w+col]);
        h->r[R(c)]--;
        h->g[G(c)]--;
        h->b[B(c)]--;
        h->n--;
    }
}

void add_pixels(image_t * im, int row, int col, int size, color_histo_t *h)
{
    int i;
    uint16_t c;

    if (col < 0 || col >= im->w) return;
    for (i = row - size; i <= row + size && i < im->h; i++) {
        if (i < 0) continue;
        c = SWAP(((uint16_t*)im->pixels)[i*im->w+col]);
        h->r[R(c)]++;
        h->g[G(c)]++;
        h->b[B(c)]++;
        h->n++;
    }
}

void init_histo(image_t *im, int row, int size, color_histo_t *h)
{
    memset(h, 0, sizeof(color_histo_t));
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

void imlib_median_filter(image_t *in, int size)
{
    uint16_t r,g,b;
    color_histo_t *h = xalloc(sizeof(*h));
    uint16_t *data = (uint16_t*) (in->data+(in->w * in->h*2));

    for (int row = 0; row<in->h; row ++) {
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
            data[row*in->w+col] = SWAP(((r << 11) | (g << 5) | b));
        }
    }

    memcpy(in->data, data, (in->w*in->h*2));
}
