#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdint.h>
struct point {
    int x;
    int y;
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
            float s,v;
        };
    };
};

struct frame_buffer {
    int width;
    int height;
    int bpp;
    uint8_t *pixels;
};

float imlib_distance(struct color *c0, struct color *c1);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);
void imlib_color_track(struct frame_buffer *buffer, struct color *color, struct point *point, int threshold);
void imlib_erosion_filter(struct frame_buffer *buffer, uint8_t *kernel, int k_size);
#endif //__IMLIB_H__
