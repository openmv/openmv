// SPDX-License-Identifier: MIT
//
// Copyright (C) 2025 OpenMV, LLC.
//
// OpenMV wrapper for upstream AprilTag v3.4.5 (configurable fork).
// Provides imlib_find_apriltags() and imlib_find_rects().

#include "imlib.h"
#include "fb_alloc.h"
#include "fmath.h"

#include "apriltag.h"
#include "common/image_u8.h"
#include "common/matd.h"
#include "common/homography.h"
#include "common/zarray.h"
#include "common/g2d.h"

#ifdef IMLIB_ENABLE_APRILTAGS_TAG16H5
#include "tag16h5.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAG25H9
#include "tag25h9.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAG36H10
#include "tag36h10.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAG36H11
#include "tag36h11.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7
#include "tagCircle21h7.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12
#include "tagCircle49h12.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12
#include "tagCustom48h12.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12
#include "tagStandard41h12.h"
#endif
#ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13
#include "tagStandard52h13.h"
#endif

// Internal AprilTag functions.
extern int quad_update_homographies(struct quad *quad);
extern zarray_t *apriltag_quad_thresh(apriltag_detector_t *td, image_u8_t *im);
extern void refine_edges(apriltag_detector_t *td, image_u8_t *im_orig, struct quad *quad);

#ifdef IMLIB_ENABLE_APRILTAGS
void imlib_find_apriltags(list_t *out, image_t *ptr, rectangle_t *roi, apriltag_families_t families,
                          float fx, float fy, float cx, float cy) {
    // NOTE: callers (py_image.c) wrap this with fb_alloc_mark/fb_alloc_free_till_mark.

    #ifdef APRILTAG_ENABLE_UMM_ALLOC
    // Reserve fb_alloc space for the grayscale image, give the rest to UMM.
    size_t fb_alloc_need = (roi->w * roi->h) + roi->w * 3; // grayscale image
    umm_init_x(fb_avail() - fb_alloc_need);
    #endif

    apriltag_detector_t *td = apriltag_detector_create();
    td->nthreads = 1;
    td->wp = workerpool_create(td->nthreads);

    // Create and add requested tag families.
    #ifdef IMLIB_ENABLE_APRILTAGS_TAG16H5
    apriltag_family_t *tf_tag16h5 = NULL;
    if (families & TAG16H5) {
        tf_tag16h5 = tag16h5_create();
        apriltag_detector_add_family(td, tf_tag16h5);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAG25H9
    apriltag_family_t *tf_tag25h9 = NULL;
    if (families & TAG25H9) {
        tf_tag25h9 = tag25h9_create();
        apriltag_detector_add_family(td, tf_tag25h9);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAG36H10
    apriltag_family_t *tf_tag36h10 = NULL;
    if (families & TAG36H10) {
        tf_tag36h10 = tag36h10_create();
        apriltag_detector_add_family(td, tf_tag36h10);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAG36H11
    apriltag_family_t *tf_tag36h11 = NULL;
    if (families & TAG36H11) {
        tf_tag36h11 = tag36h11_create();
        apriltag_detector_add_family(td, tf_tag36h11);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7
    apriltag_family_t *tf_tagCircle21h7 = NULL;
    if (families & TAGCIRCLE21H7) {
        tf_tagCircle21h7 = tagCircle21h7_create();
        apriltag_detector_add_family(td, tf_tagCircle21h7);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12
    apriltag_family_t *tf_tagCircle49h12 = NULL;
    if (families & TAGCIRCLE49H12) {
        tf_tagCircle49h12 = tagCircle49h12_create();
        apriltag_detector_add_family(td, tf_tagCircle49h12);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12
    apriltag_family_t *tf_tagCustom48h12 = NULL;
    if (families & TAGCUSTOM48H12) {
        tf_tagCustom48h12 = tagCustom48h12_create();
        apriltag_detector_add_family(td, tf_tagCustom48h12);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12
    apriltag_family_t *tf_tagStandard41h12 = NULL;
    if (families & TAGSTANDARD41H12) {
        tf_tagStandard41h12 = tagStandard41h12_create();
        apriltag_detector_add_family(td, tf_tagStandard41h12);
    }
    #endif

    #ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13
    apriltag_family_t *tf_tagStandard52h13 = NULL;
    if (families & TAGSTANDARD52H13) {
        tf_tagStandard52h13 = tagStandard52h13_create();
        apriltag_detector_add_family(td, tf_tagStandard52h13);
    }
    #endif

    // Allocate grayscale image from frame buffer.
    image_t img;
    img.w = roi->w;
    img.h = roi->h;
    img.pixfmt = PIXFORMAT_GRAYSCALE;
    img.data = fb_alloc(image_size(&img), FB_ALLOC_CACHE_ALIGN | FB_ALLOC_PREFER_SIZE);
    imlib_draw_image(&img, ptr, 0, 0, 1.f, 1.f, roi, -1, 255, NULL, NULL, 0, NULL, NULL, NULL, NULL);

    image_u8_t im = {
        .width = roi->w,
        .height = roi->h,
        .stride = roi->w,
        .buf = img.data,
    };

    zarray_t *detections = apriltag_detector_detect(td, &im);
    list_init(out, sizeof(find_apriltags_list_lnk_data_t));

    for (int i = 0, j = zarray_size(detections); i < j; i++) {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);

        find_apriltags_list_lnk_data_t lnk_data;
        rectangle_init(&(lnk_data.rect),
                       fast_roundf(det->p[0][0]) + roi->x,
                       fast_roundf(det->p[0][1]) + roi->y, 0, 0);

        for (size_t k = 1, l = (sizeof(det->p) / sizeof(det->p[0])); k < l; k++) {
            rectangle_t temp;
            rectangle_init(&temp,
                           fast_roundf(det->p[k][0]) + roi->x,
                           fast_roundf(det->p[k][1]) + roi->y, 0, 0);
            rectangle_united(&(lnk_data.rect), &temp);
        }

        lnk_data.corners[0].x = fast_roundf(det->p[3][0]) + roi->x;
        lnk_data.corners[0].y = fast_roundf(det->p[3][1]) + roi->y;
        lnk_data.corners[1].x = fast_roundf(det->p[2][0]) + roi->x;
        lnk_data.corners[1].y = fast_roundf(det->p[2][1]) + roi->y;
        lnk_data.corners[2].x = fast_roundf(det->p[1][0]) + roi->x;
        lnk_data.corners[2].y = fast_roundf(det->p[1][1]) + roi->y;
        lnk_data.corners[3].x = fast_roundf(det->p[0][0]) + roi->x;
        lnk_data.corners[3].y = fast_roundf(det->p[0][1]) + roi->y;

        lnk_data.id = det->id;
        lnk_data.family = 0;

        #ifdef IMLIB_ENABLE_APRILTAGS_TAG16H5
        if (tf_tag16h5 && det->family == tf_tag16h5) {
            lnk_data.family |= TAG16H5;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAG25H9
        if (tf_tag25h9 && det->family == tf_tag25h9) {
            lnk_data.family |= TAG25H9;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAG36H10
        if (tf_tag36h10 && det->family == tf_tag36h10) {
            lnk_data.family |= TAG36H10;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAG36H11
        if (tf_tag36h11 && det->family == tf_tag36h11) {
            lnk_data.family |= TAG36H11;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7
        if (tf_tagCircle21h7 && det->family == tf_tagCircle21h7) {
            lnk_data.family |= TAGCIRCLE21H7;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12
        if (tf_tagCircle49h12 && det->family == tf_tagCircle49h12) {
            lnk_data.family |= TAGCIRCLE49H12;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12
        if (tf_tagCustom48h12 && det->family == tf_tagCustom48h12) {
            lnk_data.family |= TAGCUSTOM48H12;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12
        if (tf_tagStandard41h12 && det->family == tf_tagStandard41h12) {
            lnk_data.family |= TAGSTANDARD41H12;
        }
        #endif
        #ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13
        if (tf_tagStandard52h13 && det->family == tf_tagStandard52h13) {
            lnk_data.family |= TAGSTANDARD52H13;
        }
        #endif

        lnk_data.hamming = det->hamming;
        lnk_data.centroid_x = det->c[0] + roi->x;
        lnk_data.centroid_y = det->c[1] + roi->y;
        lnk_data.goodness = 1.0; // removed in upstream v3.4.5
        lnk_data.decision_margin = det->decision_margin / 255.0f;

        matd_t *pose = homography_to_pose(det->H, -fx, fy, cx, cy);

        lnk_data.x_translation = MATD_EL(pose, 0, 3);
        lnk_data.y_translation = MATD_EL(pose, 1, 3);
        lnk_data.z_translation = MATD_EL(pose, 2, 3);
        lnk_data.x_rotation = fast_atan2f(MATD_EL(pose, 2, 1), MATD_EL(pose, 2, 2));
        double r21 = MATD_EL(pose, 2, 1), r22 = MATD_EL(pose, 2, 2);
        lnk_data.y_rotation = fast_atan2f(-MATD_EL(pose, 2, 0),
                                          fast_sqrtf(r21 * r21 + r22 * r22));
        lnk_data.z_rotation = fast_atan2f(MATD_EL(pose, 1, 0), MATD_EL(pose, 0, 0));

        matd_destroy(pose);

        list_push_back(out, &lnk_data);
    }

    apriltag_detections_destroy(detections);

    apriltag_detector_destroy(td);

    // Destroy tag families.
    #ifdef IMLIB_ENABLE_APRILTAGS_TAG16H5
    if (tf_tag16h5) {
        tag16h5_destroy(tf_tag16h5);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAG25H9
    if (tf_tag25h9) {
        tag25h9_destroy(tf_tag25h9);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAG36H10
    if (tf_tag36h10) {
        tag36h10_destroy(tf_tag36h10);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAG36H11
    if (tf_tag36h11) {
        tag36h11_destroy(tf_tag36h11);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7
    if (tf_tagCircle21h7) {
        tagCircle21h7_destroy(tf_tagCircle21h7);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12
    if (tf_tagCircle49h12) {
        tagCircle49h12_destroy(tf_tagCircle49h12);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12
    if (tf_tagCustom48h12) {
        tagCustom48h12_destroy(tf_tagCustom48h12);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12
    if (tf_tagStandard41h12) {
        tagStandard41h12_destroy(tf_tagStandard41h12);
    }
    #endif
    #ifdef IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13
    if (tf_tagStandard52h13) {
        tagStandard52h13_destroy(tf_tagStandard52h13);
    }
    #endif

    #ifdef APRILTAG_ENABLE_UMM_ALLOC
    umm_print_stats();
    #endif
}
#endif //IMLIB_ENABLE_APRILTAGS

#ifdef IMLIB_ENABLE_FIND_RECTS
void imlib_find_rects(list_t *out, image_t *ptr, rectangle_t *roi, uint32_t threshold) {
    // NOTE: callers (py_image.c) wrap this with fb_alloc_mark/fb_alloc_free_till_mark.

    int r_diag_len = fast_roundf(fast_sqrtf((roi->w * roi->w) + (roi->h * roi->h))) * 2;
    #ifdef APRILTAG_ENABLE_UMM_ALLOC
    // Reserve fb_alloc space for grayscale image + trace buffers, give the rest to UMM.
    size_t fb_alloc_need = (roi->w * roi->h) + roi->w * 3 // grayscale image
                           + (sizeof(int) + sizeof(uint32_t) + sizeof(point_t)) * r_diag_len; // trace buffers
    umm_init_x(fb_avail() - fb_alloc_need);
    #endif

    apriltag_detector_t *td = apriltag_detector_create();
    td->nthreads = 1;
    td->wp = workerpool_create(td->nthreads);

    // Register dummy tag families so that apriltag_quad_thresh accepts
    // both normal and reversed-border quads (needed for rect detection).
    apriltag_family_t dummy_normal = { .width_at_border = 6, .reversed_border = false };
    apriltag_family_t dummy_reversed = { .width_at_border = 6, .reversed_border = true };
    apriltag_family_t *dnp = &dummy_normal, *drp = &dummy_reversed;
    zarray_add(td->tag_families, &dnp);
    zarray_add(td->tag_families, &drp);

    // Allocate grayscale image from frame buffer.
    image_t img;
    img.w = roi->w;
    img.h = roi->h;
    img.pixfmt = PIXFORMAT_GRAYSCALE;
    img.data = fb_alloc(image_size(&img), FB_ALLOC_CACHE_ALIGN | FB_ALLOC_PREFER_SIZE);
    imlib_draw_image(&img, ptr, 0, 0, 1.f, 1.f, roi, -1, 255, NULL, NULL, 0, NULL, NULL, NULL, NULL);

    image_u8_t im = {
        .width = roi->w,
        .height = roi->h,
        .stride = roi->w,
        .buf = img.data,
    };

    // Decimate image for quad detection (same as apriltag_detector_detect).
    image_u8_t *quad_im = &im;
    if (td->quad_decimate > 1) {
        quad_im = image_u8_decimate(&im, td->quad_decimate);
    }

    zarray_t *detections = apriltag_quad_thresh(td, quad_im);
    td->nquads = zarray_size(detections);

    // Scale quad corners back to original resolution.
    if (td->quad_decimate > 1) {
        for (int i = 0; i < zarray_size(detections); i++) {
            struct quad *q;
            zarray_get_volatile(detections, i, &q);
            for (int j = 0; j < 4; j++) {
                q->p[j][0] *= td->quad_decimate;
                q->p[j][1] *= td->quad_decimate;
            }
        }
        image_u8_destroy(quad_im);
    }

    // Refine edges and compute homographies for each quad.
    for (int i = 0; i < zarray_size(detections); i++) {
        struct quad *quad_original;
        zarray_get_volatile(detections, i, &quad_original);

        if (td->refine_edges) {
            refine_edges(td, &im, quad_original);
        }

        if (quad_update_homographies(quad_original)) {
            continue;
        }
    }

    // Remove overlapping quads.
    {
        zarray_t *poly0 = g2d_polygon_create_zeros(4);
        zarray_t *poly1 = g2d_polygon_create_zeros(4);

        for (int i0 = 0; i0 < zarray_size(detections); i0++) {
            struct quad *det0;
            zarray_get_volatile(detections, i0, &det0);

            for (int k = 0; k < 4; k++) {
                double p[2] = { det0->p[k][0], det0->p[k][1] };
                zarray_set(poly0, k, p, NULL);
            }

            for (int i1 = i0 + 1; i1 < zarray_size(detections); i1++) {
                struct quad *det1;
                zarray_get_volatile(detections, i1, &det1);

                for (int k = 0; k < 4; k++) {
                    double p[2] = { det1->p[k][0], det1->p[k][1] };
                    zarray_set(poly1, k, p, NULL);
                }

                if (g2d_polygon_overlaps_polygon(poly0, poly1)) {
                    matd_destroy(det1->H);
                    matd_destroy(det1->Hinv);
                    zarray_remove_index(detections, i1, 1);
                    i1--;
                }
            }
        }

        zarray_destroy(poly0);
        zarray_destroy(poly1);
    }

    list_init(out, sizeof(find_rects_list_lnk_data_t));

    int *theta_buffer = fb_alloc(sizeof(int) * r_diag_len, FB_ALLOC_NO_HINT);
    uint32_t *mag_buffer = fb_alloc(sizeof(uint32_t) * r_diag_len, FB_ALLOC_NO_HINT);
    point_t *point_buffer = fb_alloc(sizeof(point_t) * r_diag_len, FB_ALLOC_NO_HINT);

    for (int i = 0, j = zarray_size(detections); i < j; i++) {
        struct quad *det;
        zarray_get_volatile(detections, i, &det);

        if (!det->H) {
            continue;
        }

        line_t lines[4];
        lines[0].x1 = fast_roundf(det->p[0][0]) + roi->x; lines[0].y1 = fast_roundf(det->p[0][1]) + roi->y;
        lines[0].x2 = fast_roundf(det->p[1][0]) + roi->x; lines[0].y2 = fast_roundf(det->p[1][1]) + roi->y;
        lines[1].x1 = fast_roundf(det->p[1][0]) + roi->x; lines[1].y1 = fast_roundf(det->p[1][1]) + roi->y;
        lines[1].x2 = fast_roundf(det->p[2][0]) + roi->x; lines[1].y2 = fast_roundf(det->p[2][1]) + roi->y;
        lines[2].x1 = fast_roundf(det->p[2][0]) + roi->x; lines[2].y1 = fast_roundf(det->p[2][1]) + roi->y;
        lines[2].x2 = fast_roundf(det->p[3][0]) + roi->x; lines[2].y2 = fast_roundf(det->p[3][1]) + roi->y;
        lines[3].x1 = fast_roundf(det->p[3][0]) + roi->x; lines[3].y1 = fast_roundf(det->p[3][1]) + roi->y;
        lines[3].x2 = fast_roundf(det->p[0][0]) + roi->x; lines[3].y2 = fast_roundf(det->p[0][1]) + roi->y;

        uint32_t magnitude = 0;

        for (int k = 0; k < 4; k++) {
            if (!lb_clip_line(&lines[k], 0, 0, ptr->w, ptr->h)) {
                continue;
            }

            size_t index = trace_line(ptr, &lines[k], theta_buffer, mag_buffer, point_buffer);

            for (int m = 0; m < index; m++) {
                magnitude += mag_buffer[m];
            }
        }

        if (magnitude < threshold) {
            continue;
        }

        find_rects_list_lnk_data_t lnk_data;
        rectangle_init(&(lnk_data.rect),
                       fast_roundf(det->p[0][0]) + roi->x,
                       fast_roundf(det->p[0][1]) + roi->y, 0, 0);

        for (size_t k = 1, l = (sizeof(det->p) / sizeof(det->p[0])); k < l; k++) {
            rectangle_t temp;
            rectangle_init(&temp,
                           fast_roundf(det->p[k][0]) + roi->x,
                           fast_roundf(det->p[k][1]) + roi->y, 0, 0);
            rectangle_united(&(lnk_data.rect), &temp);
        }

        lnk_data.corners[0].x = fast_roundf(det->p[3][0]) + roi->x;
        lnk_data.corners[0].y = fast_roundf(det->p[3][1]) + roi->y;
        lnk_data.corners[1].x = fast_roundf(det->p[2][0]) + roi->x;
        lnk_data.corners[1].y = fast_roundf(det->p[2][1]) + roi->y;
        lnk_data.corners[2].x = fast_roundf(det->p[1][0]) + roi->x;
        lnk_data.corners[2].y = fast_roundf(det->p[1][1]) + roi->y;
        lnk_data.corners[3].x = fast_roundf(det->p[0][0]) + roi->x;
        lnk_data.corners[3].y = fast_roundf(det->p[0][1]) + roi->y;

        lnk_data.magnitude = magnitude;

        list_push_back(out, &lnk_data);
    }

    // Clean up quads.
    for (int i = 0; i < zarray_size(detections); i++) {
        struct quad *quad;
        zarray_get_volatile(detections, i, &quad);
        if (quad->H) {
            matd_destroy(quad->H);
        }
        if (quad->Hinv) {
            matd_destroy(quad->Hinv);
        }
    }
    zarray_destroy(detections);

    apriltag_detector_destroy(td);

    #ifdef APRILTAG_ENABLE_UMM_ALLOC
    umm_print_stats();
    #endif
}
#endif //IMLIB_ENABLE_FIND_RECTS
