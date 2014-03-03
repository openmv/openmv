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

    for (int y=0; y<image->h; y++) {
        for (int x=0; x<image->w; x++) {
            int i=y*image->w+x;
            if (pixels[i]) {
                /* new blob */
                rectangle_t *blob = rectangle_alloc(image->w, image->h, 0, 0);
                array_push_back(blobs, blob);

                /* flood fill */
                point_t *p=point_alloc(x, y);
                while(p != NULL) {
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
                        pixels[p->y*image->w+p->x]=0;
                        if ((p->x-1) > 0) {
                            array_push_back(points, point_alloc(p->x-1, p->y));
                        }
                        if ((p->x+1) < image->w) {
                            array_push_back(points, point_alloc(p->x+1, p->y));
                        }
                        if ((p->y-1) > 0) {
                            array_push_back(points, point_alloc(p->x, p->y-1));
                        }
                        if ((p->y+1) < image->h) {
                            array_push_back(points, point_alloc(p->x, p->y+1));
                        }
                    }
                    xfree(p);
                    p = array_pop_back(points);
                }
            }
        }
    }

    for (int i=0; i<array_length(blobs); i++) {
        rectangle_t *blob = array_at(blobs, i);
//        if (blob->w < 10) { /* blob too small */
            //array_erase(blobs, i); i--;
  //      } else {
            blob->w = blob->w - blob->x;
            blob->h = blob->h - blob->y;
    //    }
    }

    array_free(points);
    return blobs;
}
