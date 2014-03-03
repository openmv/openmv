#include <libmp.h>
#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>

array_t *imlib_count_blobs(struct image *image)
{
    array_t *blobs;
    array_t *points;

    array_alloc(&blobs, xfree);
    array_alloc_init(&points, xfree, 500);
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
                point_t *p;
                while((p = array_pop_back(points))!= NULL) {
                    /* add point to blob */
                    if (p->x < blob->x) {
                        blob->x = p->x;
                    }
                    if (p->y < blob->y) {
                        blob->y = y;
                    }

                    if (p->x > blob->w) {
                        blob->w = p->x;
                    }
                    if (p->y > blob->h) {
                        blob->h = p->y;
                    }

                    if (pixels[p->y*image->w+p->x]) {
                        pixels[p->y*image->w+p->x]=0x0000;
                        array_push_back(points, point_alloc(p->x-1, p->y));
                        array_push_back(points, point_alloc(p->x+1, p->y));
                        array_push_back(points, point_alloc(p->x, p->y-1));
                        array_push_back(points, point_alloc(p->x, p->y+1));
                    }
                }
            }
        }
    }

    for (int i=0; i<array_length(blobs); i++) {
        rectangle_t *blob = array_at(blobs, i);
        if (blob->w < 10) { /* blob too small */
            //array_erase(blobs, i);
            i--;
        } else {
            blob->w = blob->w - blob->x;
            blob->h = blob->h - blob->y;
        }
    }
    //array_free(points);
    return blobs;
}
