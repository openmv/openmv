#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdint.h>
#include <stdbool.h>
#include "array.h"
typedef struct point {
    uint16_t x;
    uint16_t y;
} point_t;

typedef struct size {
    int w;
    int h;
} wsize_t;

typedef struct rectangle {
    int x;
    int y;
    int w;
    int h;
} rectangle_t;

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
    union {
        struct {
            uint8_t c0;
            uint8_t c1;
            uint8_t c2;
            uint8_t c3;
        };

        struct {
            uint16_t s0;
            uint16_t s1;
        };
        struct {
            uint32_t i;
        };
    };
}vec_t;

typedef struct cluster {
    array_t *points;
    point_t centroid;
} cluster_t;

#define SURF_DESC_SIZE  (32)
typedef struct ipoint {
  /* Coordinates of the detected interest point */
  float x, y;
  /* Detected scale */
  float scale;
  /* Orientation measured anti-clockwise from +ve x-axis */
  float orientation;
  /* Sign of laplacian for fast matching purposes */
  int laplacian;
  /* Vector of descriptor components */
  float descriptor[SURF_DESC_SIZE];
  /* Placeholds for point motion */
  float dx, dy;
  /* Used to store cluster index */
  int clusterIndex;
} i_point_t;

typedef struct response_layer {
  int width;
  int height;
  int step;
  int filter;
} response_layer_t;

typedef struct surf {
    image_t *img;       /* Image to find Ipoints in */
    i_image_t *i_img;    /* Integral image */
    array_t *ipts;      /* Reference to vector of Ipoints */
    array_t *rmap;      /* Response map */
    bool upright;       /* Run in rotation invariant mode? */
    int octaves;        /* Number of octaves to calculate */
    int intervals;      /* Number of intervals per octave */
    int init_sample;    /* Initial sampling step */
    float thresh;        /* Blob response threshold */
} surf_t;

/* Haar cascade struct */
typedef struct cascade {
    /* Step size of filter window shifting */
    int step;
    /* scaling step size */
    float scale_factor;
    /* number of stages in the cascade */
    int  n_stages;
    /* number of features in the cascade */
    int  n_features;
    /* number of rectangles in the cascade */
    int  n_rectangles;
    /* size of the window used in the training set */
    struct size window;
    /* pointer to current scaled image in the pyramid */
    struct image *img;
    /* pointer to current integral image */
    struct integral_image sum;
    /* haar cascade arrays */
    uint8_t *stages_array;
    int16_t *stages_thresh_array;
    int16_t *tree_thresh_array;
    int16_t *alpha1_array;
    int16_t *alpha2_array;
    int8_t *num_rectangles_array;
    int8_t *weights_array;
    int8_t *rectangles_array;
} cascade_t;


/* sqrt functions */
uint16_t sqrt_q16(uint16_t a);
uint32_t sqrt_q32(uint32_t a);

/* Point functions */
point_t *point_alloc(int x, int y);
int point_equal(point_t *p1, point_t *p2);
float point_distance(point_t *p1, point_t *p2);

/* Rectangle functions */
rectangle_t *rectangle_alloc();
rectangle_t *rectangle_clone(rectangle_t *r);
void rectangle_add(rectangle_t *r0, rectangle_t *r1);
void rectangle_div(rectangle_t *r0, int c);
int rectangle_intersects(rectangle_t *r0, rectangle_t *r1);
struct array *rectangle_merge(struct array *r);

/* Clustering functions */
array_t *cluster_kmeans(array_t *points, int k);

/* Dela E on RGB/HSV/LAB */
uint16_t imlib_lab_distance(struct color *c0, struct color *c1);
uint16_t imlib_rgb_distance(struct color *c0, struct color *c1);
uint16_t imlib_rgb_distance(struct color *c0, struct color *c1);

/* Color space conversion */
void imlib_rgb_to_lab(struct color *rgb, struct color *lab);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);

/* Image filtering functions */
void imlib_histeq(struct image *src);
void imlib_median_filter(image_t *src, int r);
void imlib_erosion_filter(struct image *src, uint8_t *kernel, int k_size);
void imlib_threshold(struct image *image, struct color *color, int threshold);
array_t *imlib_count_blobs(struct image *image);

/* Integral image functions */
void imlib_integral_image(struct image *src, struct integral_image *sum);
void imlib_integral_image_sq(struct image *src, struct integral_image *sum);
uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h);

/* Template matching */
int imlib_save_template(struct image *image, const char *path);
int imlib_load_template(struct image *image, const char *path);
float imlib_template_match(struct image *image, struct image *template, struct rectangle *r);

/* Haar/VJ */
int imlib_load_cascade(struct cascade* cascade, const char *path);
struct array *imlib_detect_objects(struct image *image, struct cascade* cascade);

void imlib_scale_image(struct image *src, struct image *dst);
void imlib_draw_rectangle(struct image *image, struct rectangle *r);
int imlib_image_mean(struct image *src);
void imlib_subimage(struct image *src_img, struct image *dst_img, int x_off, int y_off);
void imlib_blit(struct image *dst_img, struct image *src_img, int x_off, int y_off);
#endif //__IMLIB_H__
