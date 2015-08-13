/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <arm_math.h>
#include <stm32f4xx.h>
#include "array.h"
#include "imlib.h"
#include "ff.h"
#include "xalloc.h"
#include "mdefs.h"
#include "font.h"
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

/* XYZ lookup table */
extern const float xyz_table[256];
/* RGB565->LAB lookup */
extern const int8_t lab_table[196608];
/* Grayscale [0..255] to rainbox lookup */
extern const uint16_t rainbow_table[256];

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
    float v[3];
    float x,y,z;
    const float c = 16.0f/ 116.0f;

    v[0] = xyz_table[rgb->vec[0]];
    v[1] = xyz_table[rgb->vec[1]];
    v[2] = xyz_table[rgb->vec[2]];

    x = (v[0] * 0.4124f + v[1] * 0.3576f + v[2] * 0.1805f) / 95.047f  ;
    y = (v[0] * 0.2126f + v[1] * 0.7152f + v[2] * 0.0722f) / 100.0f   ;
    z = (v[0] * 0.0193f + v[1] * 0.1192f + v[2] * 0.9505f) / 108.883f ;

    x = (x>0.008856f)? fast_cbrtf(x) : (x * 7.787f) + c;
    y = (y>0.008856f)? fast_cbrtf(y) : (y * 7.787f) + c;
    z = (z>0.008856f)? fast_cbrtf(z) : (z * 7.787f) + c;

    lab->L = (int8_t) (116.0f * y-16.0f);
    lab->A = (int8_t) (500.0f * (x-y));
    lab->B = (int8_t) (200.0f * (y-z));
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

void imlib_erode(image_t *src, int ksize)
{
    int c = ksize/2;// center pixel
    int k_rows = (ksize+1)*2;
    uint8_t *dst = xalloc0(src->w * k_rows);

    for (int y=1; y<src->h-ksize; y++) {
        for (int x=1; x<src->w-ksize; x++) {
            int i = y*src->w+x;
            int di = (y%k_rows)*src->w+x;
            if ((dst[di] = src->pixels[i])==0) {
                continue;
            }

            for (int j=-c; j<c; j++) {
                for (int k=-c; k<c; k++) {
                    if (src->pixels[(y+j)*src->w+x+k]==0) {
                        dst[di]=0;
                        goto done;
                    }
                }
            }
            done:;
        }

        if ((y+1)%k_rows==0) {
            memcpy(src->pixels+((y/k_rows)*k_rows*src->w), dst, src->w*k_rows);
        }
    }
    xfree(dst);
}

void imlib_dilate(image_t *src, int ksize)
{
    int c = ksize/2;// center pixel
    int k_rows = (ksize+1)*2;
    uint8_t *dst = xalloc0(src->w * k_rows);

    for (int y=1; y<src->h-ksize; y++) {
        for (int x=1; x<src->w-ksize; x++) {
            int i = y*src->w+x;
            int di = (y%k_rows)*src->w+x;
            if ((dst[di] = src->pixels[i])) {
                continue;
            }

            for (int j=-c; j<c; j++) {
                for (int k=-c; k<c; k++) {
                    if (src->pixels[(y+j)*src->w+x+k]) {
                        dst[di]=src->pixels[(y+j)*src->w+x+k];
                        goto done;
                    }
                }
            }
            done:;
        }

        if ((y+1)%k_rows==0) {
            memcpy(src->pixels+((y/k_rows)*k_rows*src->w), dst, src->w*k_rows);
        }
    }
    xfree(dst);
}

void imlib_morph(struct image *src, uint8_t *kernel, int ksize)
{

}

void imlib_invert(image_t *src)
{
    int size = src->w*src->h;
    for (int i=0; i<size; i++) {
        src->pixels[i] = ~src->pixels[i];
    }
}

void imlib_binary(image_t *src, int threshold)
{
    int size = src->w*src->h;
    for (int i=0; i<size; i++) {
        src->pixels[i] = (src->pixels[i]>threshold? 255:0);
    }
}

#ifdef OPENMV1 
void imlib_threshold(image_t *src, image_t *dst, color_t *color, int color_size, int threshold)
{
    struct color rgb, lab;
    /* Square threshold */
    threshold *= threshold;

    /* Convert reference RGB888 to LAB */
    for (int c=0; c<color_size; c++) {
        rgb.r = color[c].r;
        rgb.g = color[c].g;
        rgb.b = color[c].b;
        imlib_rgb_to_lab(&rgb, &color[c]);
    }

    uint16_t *pixels = (uint16_t*) src->pixels;

    for (int y=0; y<src->h; y++) {
        int i=y*src->w;
        for (int x=0; x<src->w; x++) {
            uint32_t p = pixels[i+x];
            rgb.r = ((p>>3)&0x1F)*255/31;
            rgb.g = ((p&0x07)<<3)|(p>>13)*255/63;
            rgb.b = ((p>>8)&0x1F)*255/31;
            imlib_rgb_to_lab(&rgb, &lab);
            
            dst->pixels[i+x] = 0;
            for (int c=0; c<color_size; c++) {
                uint32_t sum =(color[c].L-lab.L) * (color[c].L-lab.L) +
                              (color[c].A-lab.A) * (color[c].A-lab.A) +
                              (color[c].B-lab.B) * (color[c].B-lab.B);
                if (sum<threshold) {
                    /* set pixel if within threshold */
                    dst->pixels[i+x] = c+1; //sets color label c+1
                    break;
                }
            }
        }
    }
}
#else
void imlib_threshold(image_t *src, image_t *dst, color_t *color, int color_size, int threshold)
{
    /* Square threshold */
    threshold *= threshold;

    /* Convert reference RGB888 to LAB */
    for (int c=0; c<color_size; c++) {
        uint16_t r = color[c].r*31/255;
        uint16_t g = color[c].g*63/255;
        uint16_t b = color[c].b*31/255;
        uint32_t rgb = SWAP((r << 11) | (g << 5) | b) * 3;

        color[c].L = lab_table[rgb];
        color[c].A = lab_table[rgb+1];
        color[c].B = lab_table[rgb+2];
    }

    uint16_t *pixels = (uint16_t*) src->pixels;

    for (int y=0; y<src->h; y++) {
        int i=y*src->w;
        for (int x=0; x<src->w; x++) {
            // mult by 3 for lab_table lookup
            uint32_t rgb = pixels[i+x]*3;
            dst->pixels[i+x] = 0;
            for (int c=0; c<color_size; c++) {
                // TODO
                uint32_t sum =(color[c].L-lab_table[rgb])   * (color[c].L-lab_table[rgb])   +
                              (color[c].A-lab_table[rgb+1]) * (color[c].A-lab_table[rgb+1]) +
                              (color[c].B-lab_table[rgb+2]) * (color[c].B-lab_table[rgb+2]);
                if (sum<threshold) {
                    /* set pixel if within threshold */
                    dst->pixels[i+x] = c+1; //sets color label c+1
                    break;
                }
            }
        }
    }
}
#endif

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

void imlib_blit_bytes(struct image *src, struct image *dst, int x_off, int y_off)
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

void imlib_blit_gs_to_rgb(struct image *src, struct image *dst, int x_off, int y_off)
{
    int x, y;
    uint8_t  *srcp = src->data;
    uint16_t *dstp = (uint16_t*) dst->data;

    for (y=y_off; y<src->h+y_off; y++) {
        for (x=x_off; x<src->w+x_off; x++) {
            uint8_t p =*srcp++;
            dstp[y*dst->w+x]= ((uint16_t)p<<8)|p;
        }
    }
}

void imlib_blit(struct image *src, struct image *dst, int x_off, int y_off)
{
    if (src->bpp == dst->bpp) {
        imlib_blit_bytes(src, dst, x_off, y_off);
    } else if (src->bpp == 1 && dst->bpp == 2) {
        imlib_blit_gs_to_rgb(src, dst, x_off, y_off);
    } else if (src->bpp == 2 && dst->bpp == 1) {
        //imlib_blit_rgb_to_gs(src, dst, x_off, y_off);
    }
}

void imlib_blend(struct image *src, struct image *dst, int x_off, int y_off, uint8_t alpha)
{
    uint16_t i,r, g, b;

    uint16_t spix, dpix;
    uint16_t *srcp = (uint16_t *)src->pixels;
    uint16_t *dstp = (uint16_t *)dst->pixels;

    uint32_t v0, vr, vg, vb;
    v0 = __PKHBT((256-alpha), alpha, 16);
    for (int y=y_off; y<src->h+y_off; y++) {
        i=y*dst->w;
        for (int x=x_off; x<src->w+x_off; x++) {
            spix = *srcp++;
            dpix = dstp[i+x];
            vr = __PKHBT(R565(dpix), R565(spix), 16);
            vg = __PKHBT(G565(dpix), G565(spix), 16);
            vb = __PKHBT(B565(dpix), B565(spix), 16);
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

void imlib_draw_circle(struct image *image, int cx, int cy, int r, color_t *color)
{
    int x = r, y = 0;
    int radiusError = 1-x;
    uint16_t c = RGB565(color->r, color->g, color->b);
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

void imlib_draw_line(image_t *src, int x0, int y0, int x1, int y1)
{
    int dx, dy, sx, sy, err, e2;

    if (x0<0||y0<0||x1>src->w||y1>src->h) {
        return;
    }

    dx = abs(x1-x0);
    dy = abs(y1-y0);
    sx = x0<x1 ? 1 : -1;
    sy = y0<y1 ? 1 : -1;
    err = (dx>dy ? dx : -dy)/2;

    for (;;) {
        src->data[src->w*y0+x0]=0xFF;
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

// TODO check image bounds
void imlib_draw_string(image_t *src, int x_off, int y_off, const char *str, color_t *c)
{
    const glyph_t *g;
    uint8_t *srcp8 = (uint8_t*)src->pixels;
    uint16_t *srcp16 = (uint16_t*)src->pixels;
    uint16_t color = RGB565(c->r, c->g, c->b);

    for(char c; (c=*str); str++) {
        if (c < ' ' || c > '~') {
            continue;
        }
        g = &font[c-' '];
        for (int y=0; y<g->h; y++) {
            for (int x=0; x<g->w; x++) {
                if (g->data[y] & (0x80>>x)){
                    if (src->bpp == 1) {
                        srcp8[(y_off+y)*src->w+x_off+x]=color;
                    } else {
                        srcp16[(y_off+y)*src->w+x_off+x]=color;
                    }
                }
            }
        }
        x_off += g->w;
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
