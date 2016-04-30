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
#include <ff.h>
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

// RGB565 to LAB conversion
extern const int8_t lab_table[196608];

#define IM_RGB5652L(p) \
    ({ __typeof__ (p) _p = (p); \
       lab_table[_p * 3]; })

#define IM_RGB5652A(p) \
    ({ __typeof__ (p) _p = (p); \
       lab_table[(_p * 3) + 1]; })

#define IM_RGB5652B(p) \
    ({ __typeof__ (p) _p = (p); \
       lab_table[(_p * 3) + 2]; })

// Grayscale maxes
#define IM_MAX_GS (255)

// RGB565 maxes
#define IM_MAX_R5 (31)
#define IM_MAX_G6 (63)
#define IM_MAX_B5 (31)

// Grayscale histogram
#define IM_G_HIST_SIZE (256)
#define IM_G_HIST_OFFSET (0)

// LAB histogram
#define IM_L_HIST_SIZE (256)
#define IM_L_HIST_OFFSET (0)
#define IM_A_HIST_SIZE (256)
#define IM_A_HIST_OFFSET (256)
#define IM_B_HIST_SIZE (256)
#define IM_B_HIST_OFFSET (512)

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

#define IM_EQUAL(img0, img1) \
    ({ __typeof__ (img0) _img0 = (img0); \
       __typeof__ (img1) _img1 = (img1); \
       (_img0->w==_img1->w)&&(_img0->h==_img1->h)&&(_img0->bpp==_img1->bpp); })

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

typedef struct simple_color {
    uint8_t G;
    union {
        int8_t L;
        uint8_t red; // RGB888 not RGB565
    };
    union {
        int8_t A;
        uint8_t green; // RGB888 not RGB565
    };
    union {
        int8_t B;
        uint8_t blue; // RGB888 not RGB565
    };
}
simple_color_t;

typedef struct statistics {
    uint8_t g_mean;
    int8_t l_mean, a_mean, b_mean;
    uint8_t g_median;
    int8_t l_median, a_median, b_median;
    uint8_t g_mode;
    int8_t l_mode, a_mode, b_mode;
    uint8_t g_st_dev;
    int8_t l_st_dev, a_st_dev, b_st_dev;
    uint8_t g_min;
    int8_t l_min, a_min, b_min;
    uint8_t g_max;
    int8_t l_max, a_max, b_max;
    uint8_t g_lower_q;
    int8_t l_lower_q, a_lower_q, b_lower_q;
    uint8_t g_upper_q;
    int8_t l_upper_q, a_upper_q, b_upper_q;
} statistics_t;

typedef struct color_blob { // organized this way to pack it...
    int16_t x; // rect - 0
    int16_t y; // rect - 1
    int16_t w; // rect - 2
    int16_t h; // rect - 3
    int16_t cx; // centroid - 5
    int16_t cy; // centroid - 6
    float rotation; // rotation - 7
    uint16_t pixels; // number of pixels in merged blobs - 4
    uint16_t count; // number of blobs merged into this blob - 9
    uint32_t code; // color code index bits of merged blobs - 8
}
color_blob_t;

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
typedef struct kp {
    uint16_t x;
    uint16_t y;
    struct kp *match;
    uint8_t desc[64];
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

typedef struct bmp_read_settings {
    int32_t bmp_w;
    int32_t bmp_h;
    uint16_t bmp_bpp;
    uint32_t bmp_fmt;
    uint32_t bmp_row_bytes;
} bmp_read_settings_t;

typedef struct ppm_read_settings {
    uint8_t read_int_c;
    bool read_int_c_valid;
    uint8_t ppm_fmt;
} ppm_read_settings_t;

typedef enum save_image_format {
    FORMAT_DONT_CARE,
    FORMAT_BMP,
    FORMAT_PNM,
    FORMAT_JPG
} save_image_format_t;

typedef struct img_read_settings {
    union
    {
        bmp_read_settings_t bmp_rs;
        ppm_read_settings_t ppm_rs;
    };
    save_image_format_t format;
} img_read_settings_t;

typedef void (*line_op_t)(image_t*, int, uint8_t*);

typedef enum descriptor_type {
    DESC_LBP,
    DESC_FREAK,
} descriptor_t;

/* Color space functions */
void imlib_rgb_to_lab(simple_color_t *rgb, simple_color_t *lab);
void imlib_lab_to_rgb(simple_color_t *lab, simple_color_t *rgb);
void imlib_rgb_to_grayscale(simple_color_t *rgb, simple_color_t *grayscale);
void imlib_grayscale_to_rgb(simple_color_t *grayscale, simple_color_t *rgb);
uint16_t imlib_yuv_to_rgb(uint8_t y, int8_t u, int8_t v);

/* Image file functions */
void ppm_read_geometry(FIL *fp, image_t *img, const char *path, ppm_read_settings_t *rs);
void ppm_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, ppm_read_settings_t *rs);
void ppm_read(image_t *img, const char *path);
void ppm_write_subimg(image_t *img, const char *path, rectangle_t *r);
bool bmp_read_geometry(FIL *fp, image_t *img, const char *path, bmp_read_settings_t *rs);
void bmp_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, bmp_read_settings_t *rs);
void bmp_read(image_t *img, const char *path);
void bmp_write_subimg(image_t *img, const char *path, rectangle_t *r);
void jpeg_compress(image_t *src, image_t *dst, int quality);
void jpeg_read_geometry(FIL *fp, image_t *img, const char *path);
void jpeg_read_pixels(FIL *fp, image_t *img);
void jpeg_read(image_t *img, const char *path);
void jpeg_write(image_t *img, const char *path, int quality);
void imlib_image_operation(image_t *img, const char *path, image_t *other, line_op_t op);
void imlib_load_image(image_t *img, const char *path);
void imlib_save_image(image_t *img, const char *path, rectangle_t *roi, int quality);
void imlib_copy_image(image_t *dst, image_t *src, rectangle_t *roi);

/* GIF functions */
void gif_open(FIL *fp, int width, int height, bool color, bool loop);
void gif_add_frame(FIL *fp, image_t *img, uint16_t delay);
void gif_close(FIL *fp);

/* MJPEG functions */
void mjpeg_open(FIL *fp, int width, int height);
void mjpeg_add_frame(FIL *fp, uint32_t *frames, uint32_t *bytes, image_t *img, int quality);
void mjpeg_close(FIL *fp, uint32_t *frames, uint32_t *bytes, float fps);

/* Basic image functions */
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

/* Drawing functions */
void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c);
void imlib_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c);
void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c);
void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c);

/* Binary functions */
void imlib_binary(image_t *img,
                  int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds,
                  bool invert);
void imlib_invert(image_t *img);
void imlib_and(image_t *img, const char *path, image_t *other);
void imlib_nand(image_t *img, const char *path, image_t *other);
void imlib_or(image_t *img, const char *path, image_t *other);
void imlib_nor(image_t *img, const char *path, image_t *other);
void imlib_xor(image_t *img, const char *path, image_t *other);
void imlib_xnor(image_t *img, const char *path, image_t *other);
void imlib_erode(image_t *img, int ksize, int threshold);
void imlib_dilate(image_t *img, int ksize, int threshold);

/* Background Subtraction (Frame Differencing) functions */
void imlib_negate(image_t *img);
void imlib_difference(image_t *img, const char *path, image_t *other);
void imlib_replace(image_t *img, const char *path, image_t *other);
void imlib_blend(image_t *img, const char *path, image_t *other, int alpha);

/* Image Morphing */
void imlib_morph(image_t *img, const int ksize, const int8_t *krn, const float m, const int b);

/* Image Statistics */
int32_t *imlib_histogram(image_t *img, rectangle_t *r);
void imlib_statistics(image_t *img, rectangle_t *r, statistics_t *out);
int imlib_image_mean(image_t *src); // grayscale only
int imlib_image_std(image_t *src); // grayscale only

/* Image Filtering */
void imlib_midpoint_filter(image_t *img, const int ksize, const int bias);
void imlib_mean_filter(image_t *img, const int ksize);
void imlib_mode_filter(image_t *img, const int ksize);
void imlib_median_filter(image_t *img, const int ksize, const int percentile);
void imlib_histeq(image_t *img);

/* Color Tracking */
array_t *imlib_find_blobs(image_t *img,
                          int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds,
                          bool invert, rectangle_t *r,
                          bool (*f_fun)(void*,void*,color_blob_t*), void *f_fun_arg_0, void *f_fun_arg_1);
array_t *imlib_find_markers(array_t *blobs_list, int margin,
                            bool (*f_fun)(void*,void*,color_blob_t*), void *f_fun_arg_0, void *f_fun_arg_1);

/* Clustering functions */
array_t *cluster_kmeans(array_t *points, int k);

/* Integral image functions */
void imlib_integral_image_alloc(struct integral_image *sum, int w, int h);
void imlib_integral_image_free(struct integral_image *sum);
void imlib_integral_image(struct image *src, struct integral_image *sum);
void imlib_integral_image_sq(struct image *src, struct integral_image *sum);
void imlib_integral_image_scaled(struct image *src, struct integral_image *sum);
uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h);

// Integral moving window
void imlib_integral_mw_alloc(mw_image_t *sum, int w, int h);
void imlib_integral_mw_free(mw_image_t *sum);
void imlib_integral_mw_scale(rectangle_t *roi, mw_image_t *sum, int w, int h);
void imlib_integral_mw(image_t *src, mw_image_t *sum);
void imlib_integral_mw_sq(image_t *src, mw_image_t *sum);
void imlib_integral_mw_shift(image_t *src, mw_image_t *sum, int n);
void imlib_integral_mw_shift_sq(image_t *src, mw_image_t *sum, int n);
void imlib_integral_mw_ss(image_t *src, mw_image_t *sum, mw_image_t *ssq, rectangle_t *roi);
void imlib_integral_mw_shift_ss(image_t *src, mw_image_t *sum, mw_image_t *ssq, rectangle_t *roi, int n);
long imlib_integral_mw_lookup(mw_image_t *sum, int x, int y, int w, int h);

/* Template matching */
float imlib_template_match(struct image *image, struct image *template, struct rectangle *r);

/* Haar/VJ */
int imlib_load_cascade(struct cascade* cascade, const char *path);
array_t *imlib_detect_objects(struct image *image, struct cascade *cascade, struct rectangle *roi);

/* FAST/FREAK Feature Extractor */
void fast_detect(image_t *image, array_t *keypoints, int threshold, rectangle_t *roi);
array_t *freak_find_keypoints(image_t *image, bool normalized, int threshold, rectangle_t *roi);
int freak_match_keypoints(array_t *kpts1, array_t *kpts2, int threshold);
int freak_save_descriptor(FIL *fp, array_t *kpts);
int freak_load_descriptor(FIL *fp, array_t *kpts);

/* LBP Operator */
void imlib_lbp_desc(image_t *image, int div, uint8_t *desc, rectangle_t *roi);
uint8_t *imlib_lbp_cascade(image_t *image, rectangle_t *roi);
int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1);
int imlib_lbp_desc_save(FIL *fp, uint8_t *desc);
int imlib_lbp_desc_load(FIL *fp, uint8_t **desc);

/* Iris detector */
void imlib_find_iris(image_t *src, point_t *iris, rectangle_t *roi);

// Image filter functions
void im_filter_bw(uint8_t *src, uint8_t *dst, int size, int bpp, void *args);
void im_filter_skin(uint8_t *src, uint8_t *dst, int size, int bpp, void *args);

#endif //__IMLIB_H__
