#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdint.h>
struct image {
    int width;
    int height;
    int bpp;
    uint8_t *buffer;
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

float imlib_distance(struct color *c0, struct color *c1);
void imlib_rgb_to_hsv(struct color *rgb, struct color *hsv);
void imlib_color_track(struct color *color, struct image *image, struct point *point, int threshold);
#endif //__IMLIB_H__
