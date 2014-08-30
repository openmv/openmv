#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>

array_t *imlib_count_blobs(struct image *image)
{
    array_t *blobs;
    array_alloc(&blobs, xfree);
    uint8_t *pixels = (uint8_t*) image->pixels;

    // points array
    int p_size = 100;
    point_t *points = xalloc(p_size*sizeof*points);

    for (int y=0; y<image->h; y++) {
        for (int x=0; x<image->w; x++) {
            int i=y*image->w+x;
            if (pixels[i]) {

                // set initial point
                points[0].x=x;
                points[0].y=y;

                // add new blob
                rectangle_t *blob = rectangle_alloc(image->w, image->h, 0, 0);
                array_push_back(blobs, blob);

                int p_idx =0;
                int p_max = 1;

                /* flood fill */
                while(p_idx<p_max) {
                    point_t *p=&points[p_idx++];

                    /* expand blob */
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

                    int w,e;
                    /* move west */
                    for (w=p->x-1; w>=0 && pixels[p->y*image->w+w]; w--) {

                    }
                    /* move east */
                    for (e=p->x; e<image->w && pixels[p->y*image->w+e]; e++) {

                    }
                    /* add points on north or south */
                    for (int i=w; i<e; i++) {
                        pixels[p->y*image->w+i]=0;
                        if ((p->y-1) > 0 && pixels[(p->y-1)*image->w+i]) {
                            points[p_max].x = i;
                            points[p_max].y = p->y-1;

                            if (++p_max == p_size) {
                                p_size +=100;
                                points = xrealloc(points, p_size*sizeof*points);
                            }

                        }

                        if ((p->y+1) < image->h && pixels[(p->y+1)*image->w+i]) {
                            points[p_max].x = i;
                            points[p_max].y = p->y+1;

                            if (++p_max == p_size) {
                                p_size +=100;
                                points = xrealloc(points, p_size*sizeof*points);
                            }

                        }

                    }
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

    xfree(points);
    return blobs;
}
