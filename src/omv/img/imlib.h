/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library.
 *
 */
#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdbool.h>
#include <stdint.h>
#include "array.h"
#include "fmath.h"

#define IM_SWAP16(x) __REV16(x) // Swap bottom two chars in short.
#define IM_SWAP32(x) __REV32(x) // Swap bottom two shorts in long.

#define IM_MIN(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

#define IM_MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })

// RGB565 to RGB888 conversion.
extern const uint8_t rb528_table[32];
extern const uint8_t g628_table[64];

#define IM_R528(p) \
    ({ __typeof__ (p) _p = (p); \
       rb528_table[_p]; })

#define IM_G628(p) \
    ({ __typeof__ (p) _p = (p); \
       g628_table[_p]; })

#define IM_B528(p) \
    ({ __typeof__ (p) _p = (p); \
       rb528_table[_p]; })

// RGB888 to RGB565 conversion.
extern const uint8_t rb825_table[256];
extern const uint8_t g826_table[256];

#define IM_R825(p) \
    ({ __typeof__ (p) _p = (p); \
       rb825_table[_p]; })

#define IM_G826(p) \
    ({ __typeof__ (p) _p = (p); \
       g826_table[_p]; })

#define IM_B825(p) \
    ({ __typeof__ (p) _p = (p); \
       rb825_table[_p]; })

// Split RGB565 values (note the RGB565 value is byte reversed).

#define IM_R565(p) \
    ({ __typeof__ (p) _p = (p); \
       ((_p)>>3)&0x1F; })

#define IM_G565(p) \
    ({ __typeof__ (p) _p = (p); \
       (((_p)&0x07)<<3)|((_p)>>13); })

#define IM_B565(p) \
    ({ __typeof__ (p) _p = (p); \
       ((_p)>>8)&0x1F; })

// Merge RGB565 values (note the RGB565 value is byte reversed).

#define IM_RGB565(r, g, b) \
    ({ __typeof__ (r) _r = (r); \
       __typeof__ (g) _g = (g); \
       __typeof__ (b) _b = (b); \
       ((_r)<<3)|((_g)>>3)|((_g)<<13)|((_b)<<8); })

// Grayscale maxes
#define IM_MAX_GS (255)

// RGB565 maxes
#define IM_MAX_R5 (31)
#define IM_MAX_G6 (63)
#define IM_MAX_B5 (31)

#define IM_IS_NULL(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp <= 0; })

#define IM_IS_GS(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp == 1; })

#define IM_IS_RGB565(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp == 2; })

#define IM_IS_JPEG(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp >= 3; })

#define IM_X_INSIDE(img, x) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       (0<=_x)&&(_x<_img->w); })

#define IM_Y_INSIDE(img, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (y) _y = (y); \
       (0<=_y)&&(_y<_img->h); })

#define IM_GET_GS_PIXEL(img, x, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       ((uint8_t*)_img->pixels)[(_y*_img->w)+_x]; })

#define IM_GET_RGB565_PIXEL(img, x, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       ((uint16_t*)_img->pixels)[(_y*_img->w)+_x]; })

#define IM_SET_GS_PIXEL(img, x, y, p) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       __typeof__ (p) _p = (p); \
       ((uint8_t*)_img->pixels)[(_y*_img->w)+_x]=_p; })

#define IM_SET_RGB565_PIXEL(img, x, y, p) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       __typeof__ (p) _p = (p); \
       ((uint16_t*)_img->pixels)[(_y*_img->w)+_x]=_p; })

typedef struct size {
    int w;
    int h;
} wsize_t;

typedef struct point {
    int16_t x;
    int16_t y;
} point_t;

typedef struct rectangle {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} rectangle_t;

typedef struct blob {
    int x;
    int y;
    int w;
    int h;
    int c;
    int id;
} blob_t;

typedef struct color {
    union {
        uint8_t vec[3];
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        struct {
            int h;
            int s;
            int v;
        };
        struct {
            int8_t L;
            int8_t A;
            int8_t B;
        };
        struct {
            float x;
            float y;
            float z;
        };
    };
} color_t;

typedef struct image {
    int w;
    int h;
    int bpp;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
} image_t;

typedef struct integral_image {
    int w;
    int h;
    uint32_t *data;
} i_image_t;

typedef struct {
    int w;
    int h;
    int y_offs;
    int x_ratio;
    int y_ratio;
    uint32_t **data;
    uint32_t **swap;
} mw_image_t;

typedef struct _vector {
    float x;
    float y;
    float m;
    uint16_t cx,cy;
} vec_t;

typedef struct cluster {
    array_t *points;
    point_t centroid;
} cluster_t;

/* FAST/FREAK Keypoint */
typedef struct {
    uint16_t x;
    uint16_t y;
    float angle;
    uint8_t *desc;
} kp_t;

/* Haar cascade struct */
typedef struct cascade {
    int std;                        // Image standard deviation.
    int step;                       // Image scanning factor.
    float threshold;                // Detection threshold.
    float scale_factor;             // Image scaling factor.
    int n_stages;                   // Number of stages in the cascade.
    int n_features;                 // Number of features in the cascade.
    int n_rectangles;               // Number of rectangles in the cascade.
    struct size window;             // Detection window size.
    struct image *img;              // Grayscale image.
    mw_image_t *sum;                // Integral image.
    mw_image_t *ssq;                // Squared integral image.
    uint8_t *stages_array;          // Number of features per stage.
    int16_t *stages_thresh_array;   // Stages thresholds.
    int16_t *tree_thresh_array;     // Features threshold (1 per feature).
    int16_t *alpha1_array;          // Alpha1 array (1 per feature).
    int16_t *alpha2_array;          // Alpha2 array (1 per feature).
    int8_t *num_rectangles_array;   // Number of rectangles per features (1 per feature).
    int8_t *weights_array;          // Rectangles weights (1 per rectangle).
    int8_t *rectangles_array;       // Rectangles array.
} cascade_t;

typedef enum interp {
    INTERP_NEAREST,
    INTERP_BILINEAR,
    INTERP_BICUBIC
} interp_t;

int imlib_get_pixel(image_t *img, int x, int y);
void imlib_set_pixel(image_t *img, int x, int y, int p);

/* Point functions */
point_t *point_alloc(int16_t x, int16_t y);
bool point_equal(point_t *p1, point_t *p2);
float point_distance(point_t *p1, point_t *p2);

/* Rectangle functions */
rectangle_t *rectangle_alloc(int16_t x, int16_t y, int16_t w, int16_t h);
bool rectangle_equal(rectangle_t *r1, rectangle_t *r2);
bool rectangle_intersects(rectangle_t *r1, rectangle_t *r2);
bool rectangle_subimg(image_t *img, rectangle_t *r, rectangle_t *r_out);
array_t *rectangle_merge(array_t *rectangles);

/* Clustering functions */
array_t *cluster_kmeans(array_t *points, int k);

/* Dela E on RGB/HSV/LAB */
uint32_t imlib_lab_distance(struct color *c0, struct color *c1);
uint32_t imlib_rgb_distance(struct color *c0, struct color *c1);
uint32_t imlib_rgb_distance(struct color *c0, struct color *c1);

/* Color space conversion */
void imlib_rgb_to_lab(struct color *rgb, struct color *lab);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);

/* Image filtering functions */
int  imlib_image_mean(struct image *src);
void imlib_histeq(struct image *src);
void imlib_median_filter(image_t *src, int r);
void imlib_erode(image_t *src, int ksize);
void imlib_dilate(image_t *src, int ksize);
void imlib_morph(image_t *src, uint8_t *kernel, int k_size);
void imlib_invert(image_t *src);
void imlib_binary(image_t *src, int threshold);
void imlib_threshold(image_t *src, image_t *dst, color_t *color, int color_size, int threshold);
void imlib_rainbow(image_t *src, struct image *dst);
array_t *imlib_count_blobs(struct image *image);

/* Integral image functions */
void imlib_integral_image_alloc(struct integral_image *i_img, int w, int h);
void imlib_integral_image(struct image *src, struct integral_image *sum);
void imlib_integral_image_sq(struct image *src, struct integral_image *sum);
void imlib_integral_image_scaled(struct image *src, struct integral_image *sum);
uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h);

// Integral moving window 
void imlib_integral_mw_alloc(mw_image_t *sum, int w, int h);
void imlib_integral_mw_free(mw_image_t *sum);
void imlib_integral_mw_scale(image_t *src, mw_image_t *sum, int w, int h);
void imlib_integral_mw(image_t *src, mw_image_t *sum);
void imlib_integral_mw_sq(image_t *src, mw_image_t *sum);
void imlib_integral_mw_shift(image_t *src, mw_image_t *sum, int n);
void imlib_integral_mw_shift_sq(image_t *src, mw_image_t *sum, int n);
void imlib_integral_mw_ss(image_t *src, mw_image_t *sum, mw_image_t *ssq);
void imlib_integral_mw_shift_ss(image_t *src, mw_image_t *sum, mw_image_t *ssq, int n);
long imlib_integral_mw_lookup(mw_image_t *sum, int x, int y, int w, int h);

/* Template matching */
float imlib_template_match(struct image *image, struct image *template, struct rectangle *r);

/* Haar/VJ */
int imlib_load_cascade(struct cascade* cascade, const char *path);
array_t *imlib_detect_objects(struct image *image, struct cascade* cascade);

/* FAST/FREAK Feature Extractor */
kp_t *fast_detect(image_t *image, int threshold, int *ret_num_corners, rectangle_t *roi);
void freak_find_keypoints(image_t *image, kp_t *kpts, int kpts_size, bool orient_normalized, bool scale_normalized);
int16_t *freak_match_keypoints(kp_t *kpts1, int kpts1_size, kp_t *kpts2, int kpts2_size, int threshold);
int freak_save_descriptor(kp_t *kpts, int kpts_size, const char *path);
int freak_load_descriptor(kp_t **kpts_out, int *kpts_size_out, const char *path);

/* LBP Operator */
void imlib_lbp_desc(image_t *image, int div, uint8_t *desc, rectangle_t *roi);
uint8_t *imlib_lbp_cascade(image_t *image, rectangle_t *roi);
int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1);
int imlib_lbp_desc_load(const char *path, uint8_t **desc);

/* Eye detector */
void imlib_find_eyes(image_t *src, point_t *left, point_t *right, rectangle_t *roi);

/* Drawing functions */
void imlib_draw_rectangle(struct image *image, struct rectangle *r);
void imlib_draw_circle(struct image *image, int cx, int cy, int r, color_t *c);
void imlib_draw_line(image_t *src, int x0, int y0, int x1, int y1);
void imlib_draw_string(image_t *src, int x, int y, const char *str, color_t *c);

/* Misc */
void imlib_scale(struct image *src, struct image *dst, interp_t interp);
void imlib_blit(struct image *src, struct image *dst, int x_off, int y_off);
void imlib_blend(struct image *src, struct image *dst, int x_off, int y_off, uint8_t alpha);
void imlib_subimage(struct image *src, struct image *dst, int x_off, int y_off);
void jpeg_compress(image_t *src, image_t *dst, int quality);

/* Image file functions */
int ppm_read(image_t *img, const char *path);
int ppm_write(image_t *img, const char *path);
int ppm_write_subimg(image_t *img, const char *path, rectangle_t *r);
int imlib_load_image(image_t *image, const char *path);
int imlib_save_image(image_t *image, const char *path, rectangle_t *r);
#endif //__IMLIB_H__
