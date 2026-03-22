// SPDX-License-Identifier: MIT
//
// Copyright (C) 2025 OpenMV, LLC.
//
// imlib_rotation_corr() — uses matd/homography from upstream apriltag.
// Moved out of the old monolithic apriltag.c.

#include <string.h>
#include <math.h>
#include "imlib.h"
#include "fmath.h"
#include "fb_alloc.h"
#include "omv_umalloc.h"

#include "common/matd.h"
#include "common/zarray.h"
#include "common/homography.h"

#ifdef IMLIB_ENABLE_ROTATION_CORR
// http://jepsonsblog.blogspot.com/2012/11/rotation-in-3d-using-opencvs.html
void imlib_rotation_corr(image_t *img, float x_rotation, float y_rotation, float z_rotation,
                         float x_translation, float y_translation,
                         float zoom, float fov, float *corners) {
    // NOTE: callers (py_image.c) wrap this with fb_alloc_mark/fb_alloc_free_till_mark.

    size_t size = image_size(img);
    void *data = fb_alloc(size, FB_ALLOC_NO_HINT);

    size_t pool_size = fb_avail();
    void *pool_mem = fb_alloc(pool_size, FB_ALLOC_NO_HINT);

    uma_init();
    uma_add_pool(pool_mem, pool_size, 0);

    memcpy(data, img->data, size);
    memset(img->data, 0, size);

    int w = img->w;
    int h = img->h;
    float z = (fast_sqrtf((w * w) + (h * h)) / 2) / tanf(fov / 2);
    float z_z = z * zoom;

    matd_t *A1 = matd_create(4, 3);
    MATD_EL(A1, 0, 0) = 1;  MATD_EL(A1, 0, 1) = 0;  MATD_EL(A1, 0, 2) = -w / 2;
    MATD_EL(A1, 1, 0) = 0;  MATD_EL(A1, 1, 1) = 1;  MATD_EL(A1, 1, 2) = -h / 2;
    MATD_EL(A1, 2, 0) = 0;  MATD_EL(A1, 2, 1) = 0;  MATD_EL(A1, 2, 2) = 0;
    MATD_EL(A1, 3, 0) = 0;  MATD_EL(A1, 3, 1) = 0;  MATD_EL(A1, 3, 2) = 1;

    matd_t *RX = matd_create(4, 4);
    MATD_EL(RX, 0, 0) = 1;  MATD_EL(RX, 0, 1) = 0;                  MATD_EL(RX, 0, 2) = 0;
    MATD_EL(RX, 0, 3) = 0;
    MATD_EL(RX, 1, 0) = 0;  MATD_EL(RX, 1, 1) = +cosf(x_rotation);  MATD_EL(RX, 1, 2) = -sinf(x_rotation);
    MATD_EL(RX, 1, 3) = 0;
    MATD_EL(RX, 2, 0) = 0;  MATD_EL(RX, 2, 1) = +sinf(x_rotation);  MATD_EL(RX, 2, 2) = +cosf(x_rotation);
    MATD_EL(RX, 2, 3) = 0;
    MATD_EL(RX, 3, 0) = 0;  MATD_EL(RX, 3, 1) = 0;                  MATD_EL(RX, 3, 2) = 0;
    MATD_EL(RX, 3, 3) = 1;

    matd_t *RY = matd_create(4, 4);
    MATD_EL(RY, 0, 0) = +cosf(y_rotation);  MATD_EL(RY, 0, 1) = 0;  MATD_EL(RY, 0, 2) = -sinf(y_rotation);
    MATD_EL(RY, 0, 3) = 0;
    MATD_EL(RY, 1, 0) = 0;                  MATD_EL(RY, 1, 1) = 1;  MATD_EL(RY, 1, 2) = 0;
    MATD_EL(RY, 1, 3) = 0;
    MATD_EL(RY, 2, 0) = +sinf(y_rotation);  MATD_EL(RY, 2, 1) = 0;  MATD_EL(RY, 2, 2) = +cosf(y_rotation);
    MATD_EL(RY, 2, 3) = 0;
    MATD_EL(RY, 3, 0) = 0;                  MATD_EL(RY, 3, 1) = 0;  MATD_EL(RY, 3, 2) = 0;
    MATD_EL(RY, 3, 3) = 1;

    matd_t *RZ = matd_create(4, 4);
    MATD_EL(RZ, 0, 0) = +cosf(z_rotation);  MATD_EL(RZ, 0, 1) = -sinf(z_rotation);  MATD_EL(RZ, 0, 2) = 0;
    MATD_EL(RZ, 0, 3) = 0;
    MATD_EL(RZ, 1, 0) = +sinf(z_rotation);  MATD_EL(RZ, 1, 1) = +cosf(z_rotation);  MATD_EL(RZ, 1, 2) = 0;
    MATD_EL(RZ, 1, 3) = 0;
    MATD_EL(RZ, 2, 0) = 0;                  MATD_EL(RZ, 2, 1) = 0;                  MATD_EL(RZ, 2, 2) = 1;
    MATD_EL(RZ, 2, 3) = 0;
    MATD_EL(RZ, 3, 0) = 0;                  MATD_EL(RZ, 3, 1) = 0;                  MATD_EL(RZ, 3, 2) = 0;
    MATD_EL(RZ, 3, 3) = 1;

    matd_t *R = matd_op("M*M*M", RX, RY, RZ);

    matd_t *T = matd_create(4, 4);
    MATD_EL(T, 0, 0) = 1;   MATD_EL(T, 0, 1) = 0;   MATD_EL(T, 0, 2) = 0;   MATD_EL(T, 0, 3) = x_translation;
    MATD_EL(T, 1, 0) = 0;   MATD_EL(T, 1, 1) = 1;   MATD_EL(T, 1, 2) = 0;   MATD_EL(T, 1, 3) = y_translation;
    MATD_EL(T, 2, 0) = 0;   MATD_EL(T, 2, 1) = 0;   MATD_EL(T, 2, 2) = 1;   MATD_EL(T, 2, 3) = z;
    MATD_EL(T, 3, 0) = 0;   MATD_EL(T, 3, 1) = 0;   MATD_EL(T, 3, 2) = 0;   MATD_EL(T, 3, 3) = 1;

    matd_t *A2 = matd_create(3, 4);
    MATD_EL(A2, 0, 0) = z_z;    MATD_EL(A2, 0, 1) = 0;      MATD_EL(A2, 0, 2) = w / 2;   MATD_EL(A2, 0, 3) = 0;
    MATD_EL(A2, 1, 0) = 0;      MATD_EL(A2, 1, 1) = z_z;    MATD_EL(A2, 1, 2) = h / 2;   MATD_EL(A2, 1, 3) = 0;
    MATD_EL(A2, 2, 0) = 0;      MATD_EL(A2, 2, 1) = 0;      MATD_EL(A2, 2, 2) = 1;       MATD_EL(A2, 2, 3) = 0;

    matd_t *T1 = matd_op("M*M", R, A1);
    matd_t *T2 = matd_op("M*M", T, T1);
    matd_t *T3 = matd_op("M*M", A2, T2);
    matd_t *T4 = matd_inverse(T3);

    if (T4 && corners) {
        float corr[4];
        zarray_t *correspondences = zarray_create(sizeof(float[4]));

        corr[0] = 0;
        corr[1] = 0;
        corr[2] = corners[0];
        corr[3] = corners[1];
        zarray_add(correspondences, &corr);

        corr[0] = w - 1;
        corr[1] = 0;
        corr[2] = corners[2];
        corr[3] = corners[3];
        zarray_add(correspondences, &corr);

        corr[0] = w - 1;
        corr[1] = h - 1;
        corr[2] = corners[4];
        corr[3] = corners[5];
        zarray_add(correspondences, &corr);

        corr[0] = 0;
        corr[1] = h - 1;
        corr[2] = corners[6];
        corr[3] = corners[7];
        zarray_add(correspondences, &corr);

        matd_t *H = homography_compute(correspondences, HOMOGRAPHY_COMPUTE_FLAG_INVERSE);

        if (!H) {
            H = homography_compute(correspondences, HOMOGRAPHY_COMPUTE_FLAG_SVD);
        }

        if (H) {
            matd_t *T5 = matd_op("M*M", H, T4);
            matd_destroy(H);
            matd_destroy(T4);
            T4 = T5;
        }

        zarray_destroy(correspondences);
    }

    if (T4) {
        float T4_00 = MATD_EL(T4, 0, 0), T4_01 = MATD_EL(T4, 0, 1), T4_02 = MATD_EL(T4, 0, 2);
        float T4_10 = MATD_EL(T4, 1, 0), T4_11 = MATD_EL(T4, 1, 1), T4_12 = MATD_EL(T4, 1, 2);
        float T4_20 = MATD_EL(T4, 2, 0), T4_21 = MATD_EL(T4, 2, 1), T4_22 = MATD_EL(T4, 2, 2);

        if ((fast_fabsf(T4_20) < MATD_EPS) && (fast_fabsf(T4_21) < MATD_EPS)) {
            // warp affine
            T4_00 /= T4_22;
            T4_01 /= T4_22;
            T4_02 /= T4_22;
            T4_10 /= T4_22;
            T4_11 /= T4_22;
            T4_12 /= T4_22;
            switch (img->pixfmt) {
                case PIXFORMAT_BINARY: {
                    uint32_t *tmp = (uint32_t *) data;

                    for (int y = 0, yy = h; y < yy; y++) {
                        uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                        for (int x = 0, xx = w; x < xx; x++) {
                            int sourceX = fast_roundf(T4_00 * x + T4_01 * y + T4_02);
                            int sourceY = fast_roundf(T4_10 * x + T4_11 * y + T4_12);

                            if ((0 <= sourceX) && (sourceX < w) && (0 <= sourceY) && (sourceY < h)) {
                                uint32_t *ptr = tmp + (((w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sourceY);
                                int pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX);
                                IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, pixel);
                            }
                        }
                    }
                    break;
                }
                case PIXFORMAT_GRAYSCALE: {
                    uint8_t *tmp = (uint8_t *) data;

                    for (int y = 0, yy = h; y < yy; y++) {
                        uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                        for (int x = 0, xx = w; x < xx; x++) {
                            int sourceX = fast_roundf(T4_00 * x + T4_01 * y + T4_02);
                            int sourceY = fast_roundf(T4_10 * x + T4_11 * y + T4_12);

                            if ((0 <= sourceX) && (sourceX < w) && (0 <= sourceY) && (sourceY < h)) {
                                uint8_t *ptr = tmp + (w * sourceY);
                                int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(ptr, sourceX);
                                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, pixel);
                            }
                        }
                    }
                    break;
                }
                case PIXFORMAT_RGB565: {
                    uint16_t *tmp = (uint16_t *) data;

                    for (int y = 0, yy = h; y < yy; y++) {
                        uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                        for (int x = 0, xx = w; x < xx; x++) {
                            int sourceX = fast_roundf(T4_00 * x + T4_01 * y + T4_02);
                            int sourceY = fast_roundf(T4_10 * x + T4_11 * y + T4_12);

                            if ((0 <= sourceX) && (sourceX < w) && (0 <= sourceY) && (sourceY < h)) {
                                uint16_t *ptr = tmp + (w * sourceY);
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(ptr, sourceX);
                                IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, pixel);
                            }
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            // warp perspective
            switch (img->pixfmt) {
                case PIXFORMAT_BINARY: {
                    uint32_t *tmp = (uint32_t *) data;

                    for (int y = 0, yy = h; y < yy; y++) {
                        uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                        for (int x = 0, xx = w; x < xx; x++) {
                            float xxx = T4_00 * x + T4_01 * y + T4_02;
                            float yyy = T4_10 * x + T4_11 * y + T4_12;
                            float zzz = T4_20 * x + T4_21 * y + T4_22;
                            int sourceX = fast_roundf(xxx / zzz);
                            int sourceY = fast_roundf(yyy / zzz);

                            if ((0 <= sourceX) && (sourceX < w) && (0 <= sourceY) && (sourceY < h)) {
                                uint32_t *ptr = tmp + (((w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sourceY);
                                int pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX);
                                IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, pixel);
                            }
                        }
                    }
                    break;
                }
                case PIXFORMAT_GRAYSCALE: {
                    uint8_t *tmp = (uint8_t *) data;

                    for (int y = 0, yy = h; y < yy; y++) {
                        uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                        for (int x = 0, xx = w; x < xx; x++) {
                            float xxx = T4_00 * x + T4_01 * y + T4_02;
                            float yyy = T4_10 * x + T4_11 * y + T4_12;
                            float zzz = T4_20 * x + T4_21 * y + T4_22;
                            int sourceX = fast_roundf(xxx / zzz);
                            int sourceY = fast_roundf(yyy / zzz);

                            if ((0 <= sourceX) && (sourceX < w) && (0 <= sourceY) && (sourceY < h)) {
                                uint8_t *ptr = tmp + (w * sourceY);
                                int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(ptr, sourceX);
                                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, pixel);
                            }
                        }
                    }
                    break;
                }
                case PIXFORMAT_RGB565: {
                    uint16_t *tmp = (uint16_t *) data;

                    for (int y = 0, yy = h; y < yy; y++) {
                        uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                        for (int x = 0, xx = w; x < xx; x++) {
                            float xxx = T4_00 * x + T4_01 * y + T4_02;
                            float yyy = T4_10 * x + T4_11 * y + T4_12;
                            float zzz = T4_20 * x + T4_21 * y + T4_22;
                            int sourceX = fast_roundf(xxx / zzz);
                            int sourceY = fast_roundf(yyy / zzz);

                            if ((0 <= sourceX) && (sourceX < w) && (0 <= sourceY) && (sourceY < h)) {
                                uint16_t *ptr = tmp + (w * sourceY);
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(ptr, sourceX);
                                IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, pixel);
                            }
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        matd_destroy(T4);
    }

    matd_destroy(T3);
    matd_destroy(T2);
    matd_destroy(T1);
    matd_destroy(A2);
    matd_destroy(T);
    matd_destroy(R);
    matd_destroy(RZ);
    matd_destroy(RY);
    matd_destroy(RX);
    matd_destroy(A1);

}
#endif //IMLIB_ENABLE_ROTATION_CORR
