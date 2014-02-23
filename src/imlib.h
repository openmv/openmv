#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdint.h>
struct point {
    int x;
    int y;
};

struct size {
    int w;
    int h;
};

struct rectangle {
    int x;
    int y;
    int w;
    int h;
};

struct color {
    union {
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
    };
};

struct image {
    int w;
    int h;
    int bpp;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
};

struct integral_image {
    int w;
    int h;
    uint32_t *data;
};

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

struct cascade {
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
};

float imlib_distance(struct color *c0, struct color *c1);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);
void imlib_grayscale_to_rgb565(struct image *image);
void imlib_detect_color(struct image *image, struct color *color, struct rectangle *rectangle, int threshold);
void imlib_erosion_filter(struct image *src, uint8_t *kernel, int k_size);
void imlib_scale_image(struct image *src, struct image *dst);
void imlib_draw_rectangle(struct image *image, struct rectangle *r);
void imlib_histeq(struct image *src);
struct array *imlib_detect_objects(struct image *image, struct cascade* cascade);
int imlib_load_cascade(struct cascade* cascade, const char *path);
float imlib_template_match(struct image *image, struct image *template, struct rectangle *r);
int imlib_save_template(struct image *image, const char *path);
int imlib_load_template(struct image *image, const char *path);
int imlib_image_mean(struct image *src);
void imlib_subimage(struct image *src_img, struct image *dst_img, int x_off, int y_off);
void imlib_blit(struct image *dst_img, struct image *src_img, int x_off, int y_off);
void imlib_integral_image(struct image *src, struct integral_image *sum);
void imlib_integral_image_sq(struct image *src, struct integral_image *sum);
uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h);
#endif //__IMLIB_H__
