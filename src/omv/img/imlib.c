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
#include "mdefs.h"
#include "font.h"
#include "fb_alloc.h"

/* XYZ lookup table */
extern const float xyz_table[256];
/* RGB565->LAB lookup */
extern const int8_t lab_table[196608];
/* Grayscale [0..255] to rainbox lookup */
extern const uint16_t rainbow_table[256];

// Get pixel (handles boundary check and image type check).
int imlib_get_pixel(image_t *img, int x, int y)
{
    return (IM_X_INSIDE(img, x) && IM_Y_INSIDE(img, y)) ?
        ( IM_IS_GS(img)
        ? IM_GET_GS_PIXEL(img, x, y)
        : IM_GET_RGB565_PIXEL(img, x, y) )
    : 0;
}

// Set pixel (handles boundary check and image type check).
void imlib_set_pixel(image_t *img, int x, int y, int p)
{
    if (IM_X_INSIDE(img, x) && IM_Y_INSIDE(img, y)) {
        if (IM_IS_GS(img)) {
            IM_SET_GS_PIXEL(img, x, y, p);
        } else {
            IM_SET_RGB565_PIXEL(img, x, y, p);
        }
    }
}

void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c)
{
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = x0<x1 ? 1 : -1;
    int sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2;
    for (;;) {
        imlib_set_pixel(img, x0, y0, c);
        if (x0==x1 && y0==y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void imlib_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c)
{
    if (rw<=0 || rh<=0) {
        return;
    }
    for (int i=rx, j=rx+rw, k=ry+rh-1; i<j; i++) {
        imlib_set_pixel(img, i, ry, c);
        imlib_set_pixel(img, i, k, c);
    }
    for (int i=ry+1, j=ry+rh-1, k=rx+rw-1; i<j; i++) {
        imlib_set_pixel(img, rx, i, c);
        imlib_set_pixel(img, k, i, c);
    }
}

void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c)
{
    int x = r, y = 0, radiusError = 1-x;
    while (x>=y) {
        imlib_set_pixel(img,  x + cx,  y + cy, c);
        imlib_set_pixel(img,  y + cx,  x + cy, c);
        imlib_set_pixel(img, -x + cx,  y + cy, c);
        imlib_set_pixel(img, -y + cx,  x + cy, c);
        imlib_set_pixel(img, -x + cx, -y + cy, c);
        imlib_set_pixel(img, -y + cx, -x + cy, c);
        imlib_set_pixel(img,  x + cx, -y + cy, c);
        imlib_set_pixel(img,  y + cx, -x + cy, c);
        y++;
        if (radiusError<0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c)
{
    const int anchor = x_off;
    for(char ch, last='\0'; (ch=*str); str++, last=ch) {
        if (last=='\r' && ch=='\n') { // handle "\r\n" strings
            continue;
        }
        if (ch=='\n' || ch=='\r') { // handle '\n' or '\r' strings
            x_off = anchor;
            y_off += font[0].h; // newline height == space height
            continue;
        }
        if (ch<' ' || ch>'~') {
            imlib_draw_rectangle(img,(x_off+1),(y_off+1),font[0].w-2,font[0].h-2,c);
            continue;
        }
        const glyph_t *g = &font[ch-' '];
        for (int y=0; y<g->h; y++) {
            for (int x=0; x<g->w; x++) {
                if (g->data[y] & (1<<(g->w-x))) {
                    imlib_set_pixel(img, (x_off+x), (y_off+y), c);
                }
            }
        }
        x_off += g->w;
    }
}

void imlib_binary(image_t *img, int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds, bool invert)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            bool in = false;
            for (int k=0; k<num_thresholds; k++) {
                in |= invert ^
                      ((l_thresholds[k].G <= pixels[i])
                   && (pixels[i] <= h_thresholds[k].G));
            }
            pixels[i] = in ? 0xFF : 0;
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            const int pixel = pixels[i] * 3;
            const int lab_l = lab_table[pixel];
            const int lab_a = lab_table[pixel + 1];
            const int lab_b = lab_table[pixel + 2];
            bool in = false;
            for (int k=0; k<num_thresholds; k++) {
                in |= invert ^
                     (((l_thresholds[k].L <= lab_l)
                   && (lab_l <= h_thresholds[k].L))
                   && ((l_thresholds[k].A <= lab_a)
                   && (lab_a <= h_thresholds[k].A))
                   && ((l_thresholds[k].B <= lab_b)
                   && (lab_b <= h_thresholds[k].B)));
            }
            pixels[i] = in ? 0xFFFF : 0;
        }
    }
}

void imlib_invert(image_t *img)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            pixels[i] = ~pixels[i];
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        for (int i=0, j=img->w*img->h; i<j; i++) {
            pixels[i] = ~pixels[i];
        }
    }
}

void imlib_and(image_t *img, const char *file, image_t *other)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        if (file) {

        } else {
            uint8_t *other_pixels = other->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] &= other_pixels[i];
            }
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        if (file) {

        } else {
            uint16_t *other_pixels = (uint16_t *) img->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] &= other_pixels[i];
            }
        }
    }
}

void imlib_nand(image_t *img, const char *file, image_t *other)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        if (file) {

        } else {
            uint8_t *other_pixels = other->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] = ~(pixels[i] & other_pixels[i]);
            }
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        if (file) {

        } else {
            uint16_t *other_pixels = (uint16_t *) img->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] = ~(pixels[i] & other_pixels[i]);
            }
        }
    }
}

void imlib_or(image_t *img, const char *file, image_t *other)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        if (file) {

        } else {
            uint8_t *other_pixels = other->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] |= other_pixels[i];
            }
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        if (file) {

        } else {
            uint16_t *other_pixels = (uint16_t *) img->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] |= other_pixels[i];
            }
        }
    }
}

void imlib_nor(image_t *img, const char *file, image_t *other)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        if (file) {

        } else {
            uint8_t *other_pixels = other->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] = ~(pixels[i] | other_pixels[i]);
            }
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        if (file) {

        } else {
            uint16_t *other_pixels = (uint16_t *) img->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] = ~(pixels[i] | other_pixels[i]);
            }
        }
    }
}

void imlib_xor(image_t *img, const char *file, image_t *other)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        if (file) {

        } else {
            uint8_t *other_pixels = other->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] ^= other_pixels[i];
            }
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        if (file) {

        } else {
            uint16_t *other_pixels = (uint16_t *) img->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] ^= other_pixels[i];
            }
        }
    }
}

void imlib_xnor(image_t *img, const char *file, image_t *other)
{
    if (IM_IS_GS(img)) {
        uint8_t *pixels = img->pixels;
        if (file) {

        } else {
            uint8_t *other_pixels = other->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] = ~(pixels[i] ^ other_pixels[i]);
            }
        }
    } else {
        uint16_t *pixels = (uint16_t *) img->pixels;
        if (file) {

        } else {
            uint16_t *other_pixels = (uint16_t *) img->pixels;
            for (int i=0, j=img->w*img->h; i<j; i++) {
                pixels[i] = ~(pixels[i] ^ other_pixels[i]);
            }
        }
    }
}

int imlib_pixels(image_t *img, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return 0;
    }

    int sum = 0;
    if (IM_IS_GS(img)) {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                sum += !!IM_GET_GS_PIXEL(img, (rect.x + j), (rect.y + i));
            }
        }
    } else {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                sum += !!IM_GET_RGB565_PIXEL(img, (rect.x + j), (rect.y + i));
            }
        }
    }
    return sum;
}

int imlib_centroid(image_t *img, int *x_center, int *y_center, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return 0;
    }

    int sum = 0, x_sum = 0, y_sum = 0;
    if (IM_IS_GS(img)) {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j);
                int y = (rect.y + i);
                if(IM_GET_GS_PIXEL(img, x, y)) {
                    sum += 1;
                    x_center += x;
                    y_center += y;
                }
            }
        }
    } else {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j);
                int y = (rect.y + i);
                if(IM_GET_RGB565_PIXEL(img, x, y)) {
                    sum += 1;
                    x_center += x;
                    y_center += y;
                }
            }
        }
    }
    *x_center = sum ? (x_sum / sum) : 0;
    *y_center = sum ? (y_sum / sum) : 0;
    return sum;
}

float imlib_orientation_radians(image_t *img, int *sum, int *x_center, int *y_center, rectangle_t *r)
{
    rectangle_t rect;
    if (!rectangle_subimg(img, r, &rect)) {
        return 0;
    }

    *sum = imlib_centroid(img, x_center, y_center, r);
    int a = 0, b = 0, c = 0;
    if (IM_IS_GS(img)) {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j);
                int y = (rect.y + i);
                if(IM_GET_GS_PIXEL(img, x, y)) {
                    a += (x - *x_center) * (x - *x_center);
                    b += (x - *x_center) * (y - *y_center);
                    c += (y - *y_center) * (y - *y_center);
                }
            }
        }
    } else {
        for (int i = 0; i < rect.h; i++) {
            for (int j = 0; j < rect.w; j++) {
                int x = (rect.x + j);
                int y = (rect.y + i);
                if(IM_GET_RGB565_PIXEL(img, x, y)) {
                    a += (x - *x_center) * (x - *x_center);
                    b += (x - *x_center) * (y - *y_center);
                    c += (y - *y_center) * (y - *y_center);
                }
            }
        }
    }
    b *= 2;
    return ((a!=c) ? fast_atan2f(b, a-c) : 0.0) / 2.0;
}

float imlib_orientation_degrees(image_t *img, int *sum, int *x_center, int *y_center, rectangle_t *r)
{
    return (imlib_orientation_radians(img, sum, x_center, y_center, r) * 180.0) / M_PI;
}

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

    min = IM_MIN(r, IM_MIN(g, b));
    max = IM_MAX(r, IM_MAX(g, b));

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
    uint8_t *dst = fb_alloc0(src->w * k_rows);

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
    fb_free();
}

void imlib_dilate(image_t *src, int ksize)
{
    int c = ksize/2;// center pixel
    int k_rows = (ksize+1)*2;
    uint8_t *dst = fb_alloc0(src->w * k_rows);

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
    fb_free();
}

#ifdef OPENMV1
void imlib_threshold(image_t *src, image_t *dst, color_t *color, int color_size, int threshold)
{
    struct color rgb, lab;
    // Square the threshold to avoid sqrt
    threshold *= threshold;

    // Convert the RGB888 color list to LAB
    for (int c=0; c<color_size; c++) {
        rgb.r = color[c].r;
        rgb.g = color[c].g;
        rgb.b = color[c].b;
        imlib_rgb_to_lab(&rgb, &color[c]);
    }

    // Source image is RGB565 (2BPP)
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
                    // Set pixel if within threshold
                    // use color index as label (c+1)
                    dst->pixels[i+x] = c+1;
                    break;
                }
            }
        }
    }
}
#else
void imlib_threshold(image_t *src, image_t *dst, color_t *color, int color_size, int threshold)
{
    // Square the threshold to avoid sqrt
    threshold *= threshold;

    // Convert the RGB888 color list to LAB
    for (int c=0; c<color_size; c++) {
        uint16_t r = color[c].r*31/255;
        uint16_t g = color[c].g*63/255;
        uint16_t b = color[c].b*31/255;
        uint32_t rgb = IM_SWAP16((r << 11) | (g << 5) | b) * 3;

        color[c].L = lab_table[rgb];
        color[c].A = lab_table[rgb+1];
        color[c].B = lab_table[rgb+2];
    }

    // Source image is RGB565 (2BPP)
    uint16_t *pixels = (uint16_t*) src->pixels;

    for (int y=0; y<src->h; y++) {
        int i=y*src->w;
        for (int x=0; x<src->w; x++) {
            // multiply by 3 to get the LAB table index
            uint32_t rgb = pixels[i+x]*3;
            dst->pixels[i+x] = 0;
            for (int c=0; c<color_size; c++) {
                // TODO optimize
                uint32_t sum =(color[c].L-lab_table[rgb])   * (color[c].L-lab_table[rgb])   +
                              (color[c].A-lab_table[rgb+1]) * (color[c].A-lab_table[rgb+1]) +
                              (color[c].B-lab_table[rgb+2]) * (color[c].B-lab_table[rgb+2]);
                if (sum<threshold) {
                    // Set pixel if within threshold
                    // use color index as label (c+1)
                    dst->pixels[i+x] = c+1;
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
            vr = __PKHBT(IM_R565(dpix), IM_R565(spix), 16);
            vg = __PKHBT(IM_G565(dpix), IM_G565(spix), 16);
            vb = __PKHBT(IM_B565(dpix), IM_B565(spix), 16);
            r = __SMUAD(v0, vr)>>8;
            g = __SMUAD(v0, vg)>>8;
            b = __SMUAD(v0, vb)>>8;
            dstp[i+x]= IM_RGB565(r, g, b);
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
            r = (int)(IM_R565(A)*(1-x_diff)*(1-y_diff) + IM_R565(B)*(x_diff)*(1-y_diff) +
                      IM_R565(C)*(y_diff)*(1-x_diff)   + IM_R565(D)*(x_diff*y_diff));

            // Yb = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
            g = (int)(IM_G565(A)*(1-x_diff)*(1-y_diff) + IM_G565(B)*(x_diff)*(1-y_diff) +
                      IM_G565(C)*(y_diff)*(1-x_diff)   + IM_G565(D)*(x_diff*y_diff));

            // Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
            b =(int)(IM_B565(A)*(1-x_diff)*(1-y_diff) + IM_B565(B)*(x_diff)*(1-y_diff) +
                     IM_B565(C)*(y_diff)*(1-x_diff)   + IM_B565(D)*(x_diff*y_diff));

            dstp[offset++] = IM_RGB565(r, g, b);
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

void imlib_histeq(struct image *src)
{
    int i, sum;
    int a = src->w*src->h;
    uint32_t hist[IM_MAX_GS+1]={0};

    /* compute image histogram */
    for (i=0; i<a; i++) {
        hist[src->pixels[i]]+=1;
    }

    /* compute the CDF */
    for (i=0, sum=0; i<IM_MAX_GS+1; i++) {
        sum += hist[i];
        hist[i] = sum;
    }

    for (i=0; i<a; i++) {
        src->pixels[i] = (uint8_t) ((IM_MAX_GS/(float)a) * hist[src->pixels[i]]);
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

// One pass standard deviation
int imlib_std(image_t *image)
{
    int w=image->w;
    int h=image->h;
    int n = w*h;
    uint8_t *data = image->pixels;

    uint32_t s=0, sq=0;
    for (int i=0; i<n; i+=2) {
        s += data[i+0] + data[i+1];
        uint32_t v0 = __PKHBT(data[i+0],
                              data[i+1], 16);
        sq = __SMLAD(v0, v0, sq);
    }

    /* mean */
    int m = s/n;

    /* variance */
    uint32_t v = sq/n-(m*m);

    /* std */
    return fast_sqrtf(v);
}
