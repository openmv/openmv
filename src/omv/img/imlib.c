#include <stdlib.h>
#include <string.h>
#include <arm_math.h>
#include <stm32f4xx.h>
#include "array.h"
#include "imlib.h"
#include "ff.h"
#include "xalloc.h"
#include "mdefs.h"
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define PIXEL_AT(src, x, y) \
   ({ __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
     src->data[_y*src->w+_x]; })

#define SET_PIXEL(src, x, y, c) \
   ({ __typeof__ (x) _x = (x);  \
      __typeof__ (y) _y = (y);  \
     src->data[_y*src->w+_x]=c; })

#define R565(p) \
    (uint32_t)((p>>3)&0x1F)

#define G565(p) \
    (uint32_t)(((p&0x07)<<3)|(p>>13))

#define B565(p) \
    (uint32_t)((p>>8)&0x1F)

#define RGB565(r, g, b)\
    (uint32_t)(((r&0x1F)<<3)|((g&0x3F)>>3)|(g<<13)|((b&0x1F)<<8))

#define SWAP(x)\
   ({ uint16_t _x = (x); \
    (((_x & 0xff)<<8 |(_x & 0xff00) >> 8));})

#define MAX_GRAY_LEVEL (255)

/* RGB565->LAB lookup */
extern const int8_t lab_table[65536];
/* Grayscale [0..255] to rainbox lookup */
extern const uint16_t rainbow_table[256];

const uint8_t xyz_table[256]= {
    0.083381, 0.098368, 0.114819, 0.132772, 0.152264, 0.173331, 0.196007, 0.220325,
    0.246318, 0.274017, 0.303452, 0.334654, 0.367651, 0.402472, 0.439144, 0.477695,
    0.518152, 0.560539, 0.604883, 0.651209, 0.699541, 0.749903, 0.802319, 0.856813,
    0.913406, 0.972122, 1.032982, 1.096009, 1.161225, 1.228649, 1.298303, 1.370208,
    1.444384, 1.520851, 1.599629, 1.680738, 1.764195, 1.850022, 1.938236, 2.028856,
    2.121901, 2.217388, 2.315337, 2.415763, 2.518686, 2.624122, 2.732089, 2.842604,
    2.955683, 3.071344, 3.189603, 3.310477, 3.433981, 3.560131, 3.688945, 3.820437,
    3.954624, 4.091520, 4.231141, 4.373503, 4.518620, 4.666509, 4.817182, 4.970657,
    5.126946, 5.286065, 5.448028, 5.612849, 5.780543, 5.951124, 6.124605, 6.301002,
    6.480327, 6.662594, 6.847817, 7.036010, 7.227185, 7.421357, 7.618538, 7.818742,
    8.021982, 8.228271, 8.437621, 8.650046, 8.865559, 9.084171, 9.305896, 9.530747,
    9.758735, 9.989873, 10.224173, 10.461648, 10.702310, 10.946171, 11.193243, 11.443537,
    11.697067, 11.953843, 12.213877, 12.477182, 12.743768, 13.013648, 13.286832, 13.563333,
    13.843162, 14.126329, 14.412847, 14.702727, 14.995979, 15.292615, 15.592646, 15.896084,
    16.202938, 16.513219, 16.826940, 17.144110, 17.464740, 17.788842, 18.116424, 18.447499,
    18.782077, 19.120168, 19.461783, 19.806932, 20.155625, 20.507874, 20.863687, 21.223076,
    21.586050, 21.952620, 22.322796, 22.696587, 23.074005, 23.455058, 23.839757, 24.228112,
    24.620133, 25.015828, 25.415209, 25.818285, 26.225066, 26.635560, 27.049779, 27.467731,
    27.889426, 28.314874, 28.744084, 29.177065, 29.613827, 30.054379, 30.498731, 30.946892,
    31.398871, 31.854678, 32.314321, 32.777810, 33.245154, 33.716362, 34.191442, 34.670406,
    35.153260, 35.640014, 36.130678, 36.625260, 37.123768, 37.626212, 38.132601, 38.642943,
    39.157248, 39.675523, 40.197778, 40.724021, 41.254261, 41.788507, 42.326767, 42.869050,
    43.415364, 43.965717, 44.520119, 45.078578, 45.641102, 46.207700, 46.778380, 47.353150,
    47.932018, 48.514994, 49.102085, 49.693300, 50.288646, 50.888132, 51.491767, 52.099557,
    52.711513, 53.327640, 53.947949, 54.572446, 55.201140, 55.834039, 56.471151, 57.112483,
    57.758044, 58.407842, 59.061884, 59.720179, 60.382734, 61.049557, 61.720656, 62.396039,
    63.075714, 63.759687, 64.447968, 65.140564, 65.837482, 66.538730, 67.244316, 67.954247,
    68.668531, 69.387176, 70.110189, 70.837578, 71.569350, 72.305513, 73.046074, 73.791041,
    74.540421, 75.294222, 76.052450, 76.815115, 77.582222, 78.353779, 79.129794, 79.910274,
    80.695226, 81.484657, 82.278575, 83.076988, 83.879901, 84.687323, 85.499261, 86.315721,
    87.136712, 87.962240, 88.792312, 89.626935, 90.466117, 91.309865, 92.158186, 93.011086,
    93.868573, 94.730654, 95.597335, 96.468625, 97.344529, 98.225055, 99.110210, 100.000000,
};

uint32_t imlib_lab_distance(struct color *c0, struct color *c1)
{
    uint32_t sum=0;
    sum += (c0->L - c1->L) * (c0->L - c1->L);
    sum += (c0->A - c1->A) * (c0->A - c1->A);
    sum += (c0->B - c1->B) * (c0->B - c1->B);
    return fast_sqrtf(sum);
}

uint32_t imlib_rgb_distance(struct color *c0, struct color *c1)
{
    uint32_t sum=0;
    sum += (c0->r - c1->r) * (c0->r - c1->r);
    sum += (c0->g - c1->g) * (c0->g - c1->g);
    sum += (c0->b - c1->b) * (c0->b - c1->b);
    return fast_sqrtf(sum);
}

uint32_t imlib_hsv_distance(struct color *c0, struct color *c1)
{
    uint32_t sum=0;
    sum += (c0->h - c1->h) * (c0->h - c1->h);
    sum += (c0->s - c1->s) * (c0->s - c1->s);
    sum += (c0->v - c1->v) * (c0->v - c1->v);
    return fast_sqrtf(sum);
}

void imlib_rgb_to_lab(struct color *rgb, struct color *lab)
{
    float t;
    float v[3];
    float xyz[3];
    const float c1 = 16.0f/ 116.0f;

    for (int i=0; i<3; i++) {
        t = rgb->vec[i]/255.0f;
       if (t > 0.04045f) {
           t = xyz_table[rgb->vec[i]];
       } else {
           t/= 1292.0f;
       }
       v[i]=t;
    }


    xyz[0] = (v[0] * 0.4124f + v[1] * 0.3576f + v[2] * 0.1805f) / 95.047f  ;
    xyz[1] = (v[0] * 0.2126f + v[1] * 0.7152f + v[2] * 0.0722f) / 100.0f   ;
    xyz[2] = (v[0] * 0.0193f + v[1] * 0.1192f + v[2] * 0.9505f) / 108.883f ;

   for (int i=0; i<3; i++) {
       t = xyz[i];
       if (t > 0.008856f) {
            t = fast_cbrtf(t);
       } else {
            t = (7.787f * t) + c1;
       }
       xyz[i]=t;
    }

   lab->L = (int8_t) (116.0f * xyz[1]-16.0f);
   lab->A = (int8_t) (500.0f * (xyz[0]-xyz[1]));
   lab->B = (int8_t) (200.0f * (xyz[1]-xyz[2]));
}

void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv)
{
    int min;
    int max;
    int r,g,b;
    int delta;

    /* 0..100 */
    r = rgb->r*100/255;
    g = rgb->g*100/255;
    b = rgb->b*100/255;

    min = MIN(r, MIN(g, b));
    max = MAX(r, MAX(g, b));

    if (min == max) {
        /* Black/gray/white */
        hsv->h = 0;
        hsv->s = 0;
        hsv->v = min;
    } else {
        delta = max-min;
        //scaled to avoid floats
        if (r==max) {
            hsv->h = (g-b)*100/delta;
        } else if (g==max) {
            hsv->h = 200 + (b-r)*100/delta;
        } else {
            hsv->h = 400 + (r-g)*100/delta;
        }
        hsv->h = hsv->h*60/100;
        if (hsv->h<0) {
            hsv->h+=360;
        }
        hsv->s = delta*100/max;
        hsv->v = max;
    }
}

/* converts a grayscale buffer to RGB565 to display on LCDs */
void imlib_grayscale_to_rgb565(struct image *image)
{
#if 0
    int i;
    for (i=0; i<(image->w * image->h * image->bpp); i++) {
        uint8_t y = image->pixels[i];
        uint8_t r = y*31/255;
        uint8_t g = y*63/255;
        uint8_t b = y*31/255;
        //uint16_t rgb = (r << 11) | (g << 5) | b;
    }
#endif
}

void imlib_erosion_filter(struct image *src, uint8_t *kernel, int k_size)
{
    int x, y, j, k;
    int w = src->w;
    int h = src->h;
    /* TODO */
    uint8_t *dst = xalloc0(w*h);

    for (y=0; y<h-k_size; y++) {
        for (x=0; x<w-k_size; x++) {
            dst[w*(y+1)+x+1] = 255;
            for (j=0; j<k_size; j++) {
                for (k=0; k<k_size; k++) {
                  /* (y*w+x)+(j*w+k) */
                  if (src->pixels[w*(y+j)+x+k] != (kernel[j*k_size+k]*255)) {
                    dst[w*(y+1)+x+1] = 0;
                    j=k_size;
                    break;
                  }
                }
            }
        }
    }
    memcpy(src->pixels, dst, w*h);
    xfree(dst);
}

void imlib_threshold(image_t *src, image_t *dst, struct color *color, int threshold)
{
    color_t lab1,lab2;
    uint16_t *pixels = (uint16_t*) src->pixels;

    /* Convert reference RGB to LAB */
    uint16_t r = color->r*31/255;
    uint16_t g = color->g*63/255;
    uint16_t b = color->b*31/255;

    r = SWAP((r << 11) | (g << 5) | b);
    lab1.L = lab_table[r*3];
    lab1.A = lab_table[r*3+1];
    lab1.B = lab_table[r*3+2];

    for (int y=0; y<src->h; y++) {
        for (int x=0; x<src->w; x++) {
            int i=y*src->w+x;
            lab2.L = lab_table[pixels[i]*3];
            lab2.A = lab_table[pixels[i]*3+1];
            lab2.B = lab_table[pixels[i]*3+2];
            /* add pixel if within threshold */
            if (imlib_lab_distance(&lab1, &lab2)<threshold) {
                dst->pixels[i] = 1;
            } else {
                dst->pixels[i] = 0;
            }
        }
    }
}

void imlib_rainbow(image_t *src, image_t *dst)
{
    uint8_t  *srcp = src->pixels;
    uint16_t *dstp = (uint16_t*)dst->pixels;

    for (int i=0; i<(src->w*src->h); i++) {
        dstp[i] = rainbow_table[srcp[i]];
    }
}

int imlib_image_mean(struct image *src)
{
    int s=0;
    int x,y;
    int n = src->w*src->h;

    for (y=0; y<src->h; y++) {
        for (x=0; x<src->w; x++) {
            s += src->data[src->w*y+x];
        }
    }

    /* mean */
    return s/n;
}

void imlib_subimage(struct image *src_img, struct image *dst_img, int x_off, int y_off)
{
    int x, y;
    typeof(*src_img->data) *src = src_img->data;
    typeof(*dst_img->data) *dst = dst_img->data;

    for (y=y_off; y<dst_img->h+y_off; y++) {
        for (x=x_off; x<dst_img->w+x_off; x++) {
            *dst++ = src[y*src_img->w+x];
        }
    }
}

void imlib_blit(struct image *src, struct image *dst, int x_off, int y_off)
{
    int x, y;
    typeof(*src->data) *srcp = src->data;
    typeof(*dst->data) *dstp = dst->data;

    for (y=y_off; y<src->h+y_off; y++) {
        for (x=x_off; x<src->w+x_off; x++) {
            dstp[y*dst->w+x]=*srcp++;
        }
    }
}

void imlib_blend(struct image *src, struct image *dst, int x_off, int y_off, uint8_t alpha)
{
    uint16_t i,r, g, b;

    uint16_t spix, dpix;
    uint16_t *srcp = (uint16_t *)src->pixels;
    uint16_t *dstp = (uint16_t *)dst->pixels;

    uint32_t v0, vr, vg, vb;
    v0 = (((uint32_t)alpha)<<16)|(256-alpha);

    for (int y=y_off; y<src->h+y_off; y++) {
        i=y*dst->w;
        for (int x=x_off; x<src->w+x_off; x++) {
            spix = *srcp++;
            dpix = dstp[i+x];
            vr = __SASX(R565(dpix), R565(spix));
            vg = __SASX(G565(dpix), G565(spix));
            vb = __SASX(B565(dpix), B565(spix));
            r = __SMUAD(v0, vr)>>8;
            g = __SMUAD(v0, vg)>>8;
            b = __SMUAD(v0, vb)>>8;
            dstp[i+x]= RGB565(r, g, b);
        }
    }
}

void imlib_scale_nearest(struct image *src, struct image *dst)
{
    int x, y, i, j;
    int w1 = src->w;
    int h1 = src->h;
    int w2 = dst->w;
    int h2 = dst->h;

    int rat = 0;
    if (src->bpp ==1) {
        uint8_t *t, *p;
        uint8_t *src_data = src->pixels;
        uint8_t *dst_data = dst->pixels;
        int x_ratio = (int)((w1<<16)/w2) +1;
        int y_ratio = (int)((h1<<16)/h2) +1;

        for (i=0; i<h2; i++) {
            t = dst_data + i*w2;
            y = ((i*y_ratio)>>16);
            p = src_data + y*w1;
            rat = 0;
            for (j=0; j<w2; j++) {
                x = (rat>>16);
                *t++ = p[x];
                rat += x_ratio;
            }
        }
    } else if (src->bpp==2) {
        uint16_t *t, *p;
        uint16_t *src_data = (uint16_t *)src->pixels;
        uint16_t *dst_data = (uint16_t *)dst->pixels;
        int x_ratio = (int)((w1<<16)/w2) +1;
        int y_ratio = (int)((h1<<16)/h2) +1;

        for (i=0; i<h2; i++) {
            t = dst_data + i*w2;
            y = ((i*y_ratio)>>16);
            p = src_data + y*w1;
            rat = 0;
            for (j=0; j<w2; j++) {
                x = (rat>>16);
                *t++ = p[x];
                rat += x_ratio;
            }
        }

    }
}

void imlib_scale_bilinear(struct image *src, struct image *dst)
{
    int w1 = src->w;
    int h1 = src->h;
    int w2 = dst->w;
    int h2 = dst->h;

    int offset = 0 ;
    int x, y, index;

    int r, g, b;
    uint16_t A, B, C, D;

    float x_diff, y_diff;
    float x_ratio = ((float)(w1-1))/w2 ;
    float y_ratio = ((float)(h1-1))/h2 ;
    uint16_t *srcp = (uint16_t *)src->pixels;
    uint16_t *dstp = (uint16_t *)dst->pixels;

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;
            index = y*w1+x ;

            A = srcp[index];
            B = srcp[index+1];
            C = srcp[index+w1];
            D = srcp[index+w1+1];

            // Yb = Ar(1-w)(1-h) + Br(w)(1-h) + Cr(h)(1-w) + Dr(wh)
            r = (int)(R565(A)*(1-x_diff)*(1-y_diff) + R565(B)*(x_diff)*(1-y_diff) +
                      R565(C)*(y_diff)*(1-x_diff)   + R565(D)*(x_diff*y_diff));

            // Yb = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
            g = (int)(G565(A)*(1-x_diff)*(1-y_diff) + G565(B)*(x_diff)*(1-y_diff) +
                      G565(C)*(y_diff)*(1-x_diff)   + G565(D)*(x_diff*y_diff));

            // Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
            b =(int)(B565(A)*(1-x_diff)*(1-y_diff) + B565(B)*(x_diff)*(1-y_diff) +
                     B565(C)*(y_diff)*(1-x_diff)   + B565(D)*(x_diff*y_diff));

            dstp[offset++] = RGB565(r, g, b);
        }
    }
}

void imlib_scale_bilinear_gray(struct image *src, struct image *dst)
{
    int w1 = src->w;
    int h1 = src->h;
    int w2 = dst->w;
    int h2 = dst->h;

    int offset = 0 ;
    int A, B, C, D, x, y, index, gray ;

    float x_diff, y_diff;
    float x_ratio = ((float)(w1-1))/w2 ;
    float y_ratio = ((float)(h1-1))/h2 ;

    uint8_t *srcp = src->pixels;
    uint8_t *dstp = dst->pixels;

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;

            index = y*w1+x;
            A = srcp[index];
            B = srcp[index+1];
            C = srcp[index+w1];
            D = srcp[index+w1+1];

            // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + Dwh
            gray = (int)(A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
                         C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff));

            dstp[offset++] = gray ;
        }
    }
}
void imlib_scale(struct image *src, struct image *dst, interp_t interp)
{
    switch (interp) {
        case INTERP_NEAREST:
            imlib_scale_nearest(src, dst);
            break;
        case INTERP_BILINEAR:
            if (src->bpp==2) {
                imlib_scale_bilinear(src, dst);
            } else {
                imlib_scale_bilinear_gray(src, dst);
            }
            break;
        case INTERP_BICUBIC:
            //NOT implemented
            break;
    }
}

void imlib_draw_rectangle(struct image *image, struct rectangle *r)
{
    int i;
    uint8_t c=0xFF;
    int x = MIN(MAX(r->x, 0), image->w-1);
    int y = MIN(MAX(r->y, 0), image->h-1);
    int w = (x+r->w) >= image->w ? (image->w-x-1):r->w;
    int h = (y+r->h) >= image->h ? (image->h-y-1):r->h;

    x *= image->bpp;
    w *= image->bpp;
    int col = image->w*image->bpp;

    for (i=0; i<w; i++) {
        image->pixels[y*col + x + i] = c;
        image->pixels[(y+h)*col + x + i] = c;
    }

    for (i=0; i<h; i++) {
        image->pixels[(y+i)*col + x] = c;
        image->pixels[(y+i)*col + x + w] = c;
        if (image->bpp>1) {
            image->pixels[(y+i)*col + x+1] = c;
            image->pixels[(y+i)*col + x + w+1] = c;
        }
    }

    if (image->bpp>1) {
        for (i=0; i<h; i++) {
            image->pixels[(y+i)*col + x+1] = c;
            image->pixels[(y+i)*col + x + w+1] = c;
        }
    }

}

void imlib_draw_circle(struct image *image, int cx, int cy, int r)
{
    int x = r, y = 0;
    uint8_t c = 0xff;
    int radiusError = 1-x;
    if (cx+r >= image->w || cx-r < 0 ||
        cy+r >= image->h || cy-r < 0) {
        return;
    }

    while(x >= y) {
        SET_PIXEL(image,  x + cx,  y + cy, c);
        SET_PIXEL(image,  y + cx,  x + cy, c);
        SET_PIXEL(image, -x + cx,  y + cy, c);
        SET_PIXEL(image, -y + cx,  x + cy, c);
        SET_PIXEL(image, -x + cx, -y + cy, c);
        SET_PIXEL(image, -y + cx, -x + cy, c);
        SET_PIXEL(image,  x + cx, -y + cy, c);
        SET_PIXEL(image,  y + cx, -x + cy, c);
        y++;
        if (radiusError<0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError+= 2 * (y - x + 1);
        }
    }
}

void imlib_histeq(struct image *src)
{
    int i, sum;
    int a = src->w*src->h;
    uint32_t hist[MAX_GRAY_LEVEL+1]={0};

    /* compute image histogram */
    for (i=0; i<a; i++) {
        hist[src->pixels[i]]+=1;
    }

    /* compute the CDF */
    for (i=0, sum=0; i<MAX_GRAY_LEVEL+1; i++) {
        sum += hist[i];
        hist[i] = sum;
    }

    for (i=0; i<a; i++) {
        src->pixels[i] = (uint8_t) ((MAX_GRAY_LEVEL/(float)a) * hist[src->pixels[i]]);
    }
}

/* those just call ppm for now */
int imlib_load_image(image_t *image, const char *path)
{
    return ppm_read(image, path);
}

int imlib_save_image(image_t *image, const char *path, rectangle_t *r)
{
    if (r == NULL) {
        return ppm_write(image, path);
    } else {
        return ppm_write_subimg(image, path, r);
    }
}
