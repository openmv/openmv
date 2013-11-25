#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdint.h>
struct point {
    int x;
    int y;
};

struct size {
    int width;
    int height;
};

struct rectangle {
    int x;
    int y;
    union {
        int width;
        int w;
    };
    union {
        int h;
        int height;
    };
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

struct frame_buffer {
    int width;
    int height;
    int bpp;
    union {
        uint8_t *data;
        uint8_t *pixels;
    };
};

struct integral_image {
    int width;
    int height;
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
    struct frame_buffer *img;
};

float imlib_distance(struct color *c0, struct color *c1);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);
void imlib_grayscale_to_rgb565(struct frame_buffer *fb);
void imlib_color_track(struct frame_buffer *fb, struct color *color, struct point *point, int threshold);
void imlib_erosion_filter(struct frame_buffer *fb, uint8_t *kernel, int k_size);
void imlib_scale_image(struct frame_buffer *src, struct frame_buffer *dst);
void imlib_integral_image(struct frame_buffer *src, struct integral_image *sum);
void imlib_draw_rectangle(struct frame_buffer* image, struct rectangle *r);
struct array *imlib_detect_objects(struct cascade* cascade, struct frame_buffer* fb);
#endif //__IMLIB_H__
