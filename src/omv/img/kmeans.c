/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Kmeans clustering.
 *
 */
#include <float.h>
#include <limits.h>
#include <arm_math.h>
#include "imlib.h"
#include "array.h"
#include "xalloc.h"

extern uint32_t rng_randint(uint32_t min, uint32_t max);

static cluster_t *cluster_alloc(int cx, int cy)
{
    cluster_t *c=NULL;
    c = xalloc(sizeof(*c));
    if (c != NULL) {
        /* initial centroid */
        c->centroid.x = cx;
        c->centroid.y = cy;
        array_alloc(&c->points, NULL);
    }
    return c;
}

static void cluster_free(void *c)
{
    cluster_t *cl = c;
    array_free(cl->points);
    xfree(cl);
}

static void cluster_reset(array_t *clusters)
{
    int k = array_length(clusters);

    /* reset clusters */
    for (int j=0; j<k; j++) {
         cluster_t *cl = array_at(clusters, j);
//         array_resize(cl->points, 0);
         array_free(cl->points);
         array_alloc(&cl->points, NULL);
    }
}

static int cluster_update(array_t *clusters)
{
    int k = array_length(clusters);

    /* update clusters */
    for (int j=0; j<k; j++) {
        point_t sum={0,0};
        cluster_t *cl = array_at(clusters, j);
        point_t old_c = cl->centroid;
        int cl_size = array_length(cl->points);

        /* sum all points in this cluster */
        for (int i=0; i<cl_size; i++) {
            point_t *p = array_at(cl->points, i);
            sum.x += p->x;
            sum.y += p->y;
        }

        cl->centroid.x = sum.x/cl_size;
        cl->centroid.y = sum.y/cl_size;
        if (point_equal(&cl->centroid, &old_c)) {
            /* cluster centroid didn't move */
            return 0;
        }
    }

    return 1;
}

static void cluster_points(array_t *clusters, array_t *points)
{
    int n = array_length(points);
    int k = array_length(clusters);

    for (int i=0; i<n; i++) {
        float distance = FLT_MAX;
        cluster_t *cl_nearest = NULL;
        point_t *p = array_at(points, i);

        for (int j=0; j<k; j++) {
            cluster_t *cl = array_at(clusters, j);
            float d = point_distance(p, &cl->centroid);
            if (d < distance) {
                distance = d;
                cl_nearest = cl;
            }
        }
        if (cl_nearest == NULL) {
            __asm__ volatile ("BKPT");
        }
        /* copy point and add to cluster */
        array_push_back(cl_nearest->points, p);
    }
}

array_t *cluster_kmeans(array_t *points, int k)
{
    array_t *clusters=NULL;
    /* alloc clusters array */
    array_alloc(&clusters, cluster_free);

    /* select K clusters randomly */
    for (int i=0; i<k; i++) {
        int pidx = rng_randint(0, array_length(points)-1);
        point_t *p = array_at(points, pidx);
        array_push_back(clusters, cluster_alloc(p->x, p->y));
    }

    int cl_changed = 1;
    do {
        /* reset clusters */
        cluster_reset(clusters);

        /* add points to clusters */
        cluster_points(clusters, points);

        /* update centroids */
        cl_changed = cluster_update(clusters);

    } while (cl_changed);

    return clusters;
}
