#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>

array_t *imlib_count_blobs(struct image *image)
{
    array_t *blobs;
    array_t *points;

    array_alloc(&blobs, xfree);
    array_alloc_init(&points, xfree, 100);
    uint8_t *pixels = (uint8_t*) image->pixels;

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
                    if (array_length(points) > 500) {
                        xfree(p);
                        goto done;
                    }

                    /* add point to blob */
                    if (p->x < blob->x) {
                        blob->x = p->x;
                    }

                    if (p->y < blob->y) {
                        blob->y = p->y;
                    }

                    if (p->x > blob->w) {
                        blob->w = p->x;
                    }

                    if (p->y > blob->h) {
                        blob->h = p->y;
                    }

                    /* move west */
                    for (int n=p->x-1; n>=0 && pixels[p->y*image->w+n]; n--) {
                        pixels[p->y*image->w+n]=0;
                        if ((p->y-1) >= 0 && pixels[(p->y-1)*image->w+n]) {
                            array_push_back(points, point_alloc(n, p->y-1));

                        }
                        if ((p->y+1) < image->h && pixels[(p->y+1)*image->w+n]) {
                            array_push_back(points, point_alloc(n, p->y+1));

                        }
                    }
                    /* move east */
                    for (int n=p->x; n<image->w && pixels[p->y*image->w+n]; n++) {
                        pixels[p->y*image->w+n]=0;
                        if ((p->y-1) > 0 && pixels[(p->y-1)*image->w+n]) {
                            array_push_back(points, point_alloc(n, p->y-1));

                        }
                        if ((p->y+1) < image->h && pixels[(p->y+1)*image->w+n]) {
                            array_push_back(points, point_alloc(n, p->y+1));

                        }
                    }
                    xfree(p);
                    p = array_pop_back(points);
                }
            }
        }
    }

done:
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
