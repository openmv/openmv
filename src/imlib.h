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
    uint8_t *pixels;
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
    /* size of the window used in the training set */
    struct size window;
    /* pointer to current integral image */
    struct integral_image sum;
    /* pointer to current scaled image in the pyramid */
    struct image *img;
};

float imlib_distance(struct color *c0, struct color *c1);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);
void imlib_grayscale_to_rgb565(struct image *image);
void imlib_detect_color(struct image *image, struct color *color, struct rectangle *rectangle, int threshold);
void imlib_erosion_filter(struct image *src, uint8_t *kernel, int k_size);
void imlib_scale_image(struct image *src, struct image *dst);
void imlib_integral_image(struct image *src, struct integral_image *sum);
void imlib_draw_rectangle(struct image *image, struct rectangle *r);
void imlib_histeq(struct image *src);
struct array *imlib_detect_objects(struct image *image, struct cascade* cascade);
#endif //__IMLIB_H__
