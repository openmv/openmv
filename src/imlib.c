#include "imlib.h"
#include <stdlib.h>

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

float imlib_distance(struct color *c0, struct color *c1)
{
    float sum=0.0f;
    sum += (c0->r - c1->r) * (c0->r - c1->r);
    sum += (c0->g - c1->g) * (c0->g - c1->g);
    sum += (c0->b - c1->b) * (c0->b - c1->b);
//    return sqrtf(sum);
    return 0.0;
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

void imlib_color_track(struct frame_buffer *frame_buffer, struct color *color, struct point *point, int threshold)
{
    int x,y;
    uint8_t p0,p1;
    struct color rgb;
    struct color hsv;

    int pixels = 1;
    point->x = 0;
    point->y = 0;

    //to avoid sqrt we use squared values
    threshold *= threshold;

    for (y=0; y<frame_buffer->height; y++) {
        for (x=0; x<frame_buffer->width; x++) {
            int i=y*frame_buffer->width*frame_buffer->bpp+x*frame_buffer->bpp;
            p0 = frame_buffer->pixels[i];
            p1 = frame_buffer->pixels[i+1];

            /* map RGB565 to RGB888 */
            rgb.r = (uint8_t) (p0>>3) * 255/31;
            rgb.g = (uint8_t) (((p0&0x07)<<3) | (p1>>5)) * 255/63;
            rgb.b = (uint8_t) (p1&0x1F) * 255/31;

            /* convert RGB to HSV */
            imlib_rgb_to_hsv(&rgb, &hsv);

            /* difference between target Hue and pixel Hue squared */
            hsv.h = (hsv.h - color->h) * (hsv.h - color->h);

            /* add pixel if within threshold */
            if (hsv.h < threshold && hsv.s > color->s && hsv.v > color->v) { //s==pale
                pixels++;
                point->x += x;
                point->y += y;
            } else {
                x+=2;
            }
        }
    }

    if (pixels < 10) {
        point->x = 0;
        point->y = 0;
    } else {
        point->x /= pixels;
        point->y /= pixels;
    }
}

void imlib_erosion_filter(struct frame_buffer *fb, uint8_t *kernel, int k_size)
{
    int x, y, j, k;
    int w = fb->width;
    int h = fb->height;
    uint8_t *dst = calloc(w*h, 1);
    
    for (y=0; y<h-k_size; y++) {
        for (x=0; x<w-k_size; x++) {
            dst[w*(y+1)+x+1] = 255;
            for (j=0; j<k_size; j++) {
                for (k=0; k<k_size; k++) {
                  /* (y*w+x)+(j*w+k) */
                  if (fb->pixels[w*(y+j)+x+k] != (kernel[j*k_size+k]*255)) {
                    dst[w*(y+1)+x+1] = 0;
                    j=k_size;
                    break;
                  }
                }
            }
        }   
    }
    memcpy(fb->pixels, dst, w*h);
    free(dst);
}
