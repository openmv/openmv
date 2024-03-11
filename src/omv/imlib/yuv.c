/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Deyuv Functions
 */
#include "imlib.h"

pixformat_t imlib_yuv_shift(pixformat_t pixfmt, int x) {
    if (x % 2) {
        return (pixfmt == PIXFORMAT_YUV422) ? PIXFORMAT_YVU422 : PIXFORMAT_YUV422;
    }

    return pixfmt;
}

void imlib_deyuv_line(int x_start, int x_end, int y_row, void *dst_row_ptr, pixformat_t pixfmt, image_t *src) {
    int shift = (src->pixfmt == PIXFORMAT_YUV422) ? 16 : 0;
    int src_w = src->w, w_limit = src_w - 1;

    uint16_t *rowptr_yuv = ((uint16_t *) src->data) + (y_row * src_w);

    // If the image is an odd width this will go for the last loop and we drop the last column.
    for (int x = x_start; x < x_end; x += 2) {
        int32_t row_yuv; // signed

        // keep pixels in bounds
        if (x >= w_limit) {
            if (src_w >= 2) {
                uint32_t temp = *((uint32_t *) (rowptr_yuv + x - 1));
                row_yuv = ((temp & 0xff00) << 16) | (temp & 0xff0000) | (temp >> 16);
            } else {
                row_yuv = *((uint16_t *) (rowptr_yuv + x));
                row_yuv = ((row_yuv & 0xff) << 16) | 0x80000000;
            }
        } else {
            row_yuv = *((uint32_t *) (rowptr_yuv + x));
        }

        int y0 = row_yuv & 0xff, y1 = (row_yuv >> 16) & 0xff;

        switch (pixfmt) {
            case PIXFORMAT_BINARY: {
                uint32_t *row_ptr_32 = (uint32_t *) dst_row_ptr;
                IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr_32, x, (y0 >> 7));

                if (x != w_limit) {
                    IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr_32, x + 1, (y1 >> 7));
                }

                break;
            }
            case PIXFORMAT_GRAYSCALE: {
                uint8_t *row_ptr_8 = (uint8_t *) dst_row_ptr;
                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr_8, x, y0);

                if (x != w_limit) {
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr_8, x + 1, y1);
                }

                break;
            }
            case PIXFORMAT_RGB565: {
                uint16_t *row_ptr_16 = (uint16_t *) dst_row_ptr;

                // R = Y + (1.40200 * U)
                // G = Y - (0.34414 * V) - (0.71414 * U)
                // B = Y + (1.77200 * V)

                // R = Y + ((179 * U) >> 7)
                // G = Y - (((44 * V) - (91 * U)) >> 7)
                // B = Y + ((227 * V) >> 7)

                row_yuv ^= 0x80008000;

                int u = (row_yuv << shift) >> 24; // signed bit extraction
                int v = (row_yuv << (16 - shift)) >> 24; // signed bit extraction

                int ry = (179 * u) >> 7;
                int gy = ((44 * v) + (91 * u)) >> 7;
                int by = (227 * v) >> 7;

                int r0 = y0 + ry, g0 = y0 - gy, b0 = y0 + by;
                r0 = __USAT(r0, 8);
                g0 = __USAT(g0, 8);
                b0 = __USAT(b0, 8);
                int rgb565_0 = COLOR_R8_G8_B8_TO_RGB565(r0, g0, b0);
                IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr_16, x, rgb565_0);

                if (x != w_limit) {
                    int r1 = y1 + ry, g1 = y1 - gy, b1 = y1 + by;
                    r1 = __USAT(r1, 8);
                    g1 = __USAT(g1, 8);
                    b1 = __USAT(b1, 8);
                    int rgb565_1 = COLOR_R8_G8_B8_TO_RGB565(r1, g1, b1);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr_16, x + 1, rgb565_1);
                }

                break;
            }
            default: {
                break;
            }
        }
    }
}

void imlib_deyuv_image(image_t *dst, image_t *src) {
    for (int y = 0, src_w = src->w, src_h = src->h; y < src_h; y++) {
        void *row_ptr = NULL;

        switch (dst->pixfmt) {
            case PIXFORMAT_BINARY: {
                row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(dst, y);
                break;
            }
            case PIXFORMAT_GRAYSCALE: {
                row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(dst, y);
                break;
            }
            case PIXFORMAT_RGB565: {
                row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(dst, y);
                break;
            }
        }

        imlib_deyuv_line(0, src_w, y, row_ptr, dst->pixfmt, src);
    }
}
