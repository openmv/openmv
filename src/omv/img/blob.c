/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Blob count.
 *
 */
#include "xalloc.h"
#include "imlib.h"
#include <arm_math.h>

blob_t *blob_alloc(int x, int y, int w, int h, int id, int c)
{
    blob_t *blob = xalloc(sizeof(*blob));
    blob->x = x;
    blob->y = y;
    blob->w = w;
    blob->h = h;
    blob->id = id;
    blob->c = c;
    return blob;
}

void blob_add_point(blob_t *blob, int px, int py)
{
    /* expand blob */
    if (px < blob->x) {
        blob->x = px;
    }

    if (py < blob->y) {
        blob->y = py;
    }

    if (px > blob->w) {
        blob->w = px;
    }

    if (py > blob->h) {
        blob->h = py;
    }
}

array_t *imlib_count_blobs(struct image *image)
{
    array_t *blobs;
    blob_t *blob;

    array_alloc(&blobs, xfree);
    uint8_t *pixels = (uint8_t*) image->pixels;

    // points array
    int p_size = 100;
    point_t *points = xalloc(p_size*sizeof*points);

    for (int y=0; y<image->h; y++) {
        for (int x=0; x<image->w; x++) {
            blob = NULL;
            uint8_t label = pixels[y*image->w+x];

            if (label) {
                int w,e;
                int p_idx =0;
                int p_max = 1;

                // set initial point
                points[0].x=x;
                points[0].y=y;

                // alloc new blob
                blob = blob_alloc(image->w, image->h, 0, 0, label, 0);
                while(p_idx<p_max) {
                    point_t *p=&points[p_idx++];
                    blob_add_point(blob, p->x, p->y);
                    // scan west
                    for (w=p->x-1; w>=0 && pixels[p->y*image->w+w]==label; w--) {
                        blob_add_point(blob, w, p->y);
                    }

                    // scan east
                    for (e=p->x+1; e<image->w && pixels[p->y*image->w+e]==label; e++) {
                        blob_add_point(blob, e, p->y);
                    }

                    // scan north and south rows, add a point only if it's the last
                    // point or the last connected point in a row, this saves some memory
                    // and other points on this segment will still be reachable from this one.

                    // add points on north row
                    for (int i=w+1; i<e; i++) {
                        pixels[p->y*image->w+i]=0;
                        if ((p->y-1) > 0 && pixels[(p->y-1)*image->w+i]==label) {
                            if (i==(e-1) || (pixels[(p->y-1)*image->w+i+1]!=label)) {
                                points[p_max].x = i;
                                points[p_max].y = p->y-1;

                                if (++p_max == p_size) {
                                    p_size +=100;
                                    points = xrealloc(points, p_size*sizeof*points);
                                }
                            }
                        }
                    }

                    // add points on south row
                    for (int i=w+1; i<e; i++) {
                        if ((p->y+1) < image->h && pixels[(p->y+1)*image->w+i]==label) {
                            if (i==(e-1) || (pixels[(p->y+1)*image->w+i+1]!=label)) {
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

                if (blob) {
                    blob->w = blob->w - blob->x;
                    blob->h = blob->h - blob->y;
                    // discard small blobs
                    if (blob->w > 10 && blob->h > 10) {
                        array_push_back(blobs, blob);
                    } else {
                        xfree(blob);
                    }
                }
            }
        }
    }
    xfree(points);
    return blobs;
}
