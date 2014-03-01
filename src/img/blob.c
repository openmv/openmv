#include <libmp.h>
#include "xalloc.h"
#include "imlib.h"
#include <math.h>
#include <arm_math.h>

array_t *imlib_count_blobs(struct image *image)
{
    array_t *blobs;
    array_t *points;

    array_alloc(&blobs, xfree);
    array_alloc_init(&points, xfree, 300);
    uint16_t *pixels = (uint16_t*) image->pixels;

    for (int y=1; y<image->h-1; y++) {
        for (int x=1; x<image->w-1; x++) {
            int i=y*image->w+x;
            if (pixels[i]) {
                /* new blob */
                rectangle_t *blob = rectangle_alloc(image->w, image->h, 0, 0);
                array_push_back(blobs, blob);

                /* flood fill */
                array_push_back(points, point_alloc(x, y));
                while(array_length(points)) {
                    int i = array_length(points)-1;
                    point_t *p = array_at(points, i);
                    int px = p->x;
                    int py = p->y;
                    array_erase(points, i);

                    /* add point to blob */
                    if (px < blob->x) {
                        blob->x = px;
                    }
                    if (py < blob->y) {
                        blob->y = y;
                    }

                    if (px > blob->w) {
                        blob->w = px;
                    }
                    if (py > blob->h) {
                        blob->h = py;
                    }

                    if (pixels[py*image->w+px]) {
                        pixels[py*image->w+px]=0x0000;
                        array_push_back(points, point_alloc(px-1, py));
                        array_push_back(points, point_alloc(px+1, py));
                        array_push_back(points, point_alloc(px, py-1));
                        array_push_back(points, point_alloc(px, py+1));
                    }
                }
            }
        }
    }

    for (int i=0; i<array_length(blobs); i++) {
        rectangle_t *blob = array_at(blobs, i);
        if (blob->w < 10) { /* blob too small */
            array_erase(blobs, i);
            i--;
        } else {
            blob->w = blob->w - blob->x;
            blob->h = blob->h - blob->y;
        }
    }
    return blobs;
}
