/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Deyuv Functions
 */
#include "imlib.h"
#include "simd.h"

pixformat_t imlib_yuv_shift(pixformat_t pixfmt, int x) {
    if (x % 2) {
        return (pixfmt == PIXFORMAT_YUV422) ? PIXFORMAT_YVU422 : PIXFORMAT_YUV422;
    }

    return pixfmt;
}

#if (__ARM_ARCH >= 8)
// Converts the pixel pairs that don't start on the last column to RGB565, with the same math as
// imlib_deyuv_line(), and returns the x position for imlib_deyuv_line() to continue from. The
// source row is loaded before the destination row is stored, so the conversion can be in place.
static int vdeyuv_line_rgb565(int x_start, int x_end, int y_row, uint16_t *dst_row_ptr, image_t *src) {
    int src_w = src->w;
    int x_limit = IM_MIN(x_end, src_w - 1);

    if (x_limit <= x_start) {
        return x_start;
    }

    // Pixel pairs start at x_start, x_start + 2, ... and both pixels of each pair are written.
    int n = ((x_limit - x_start + 1) / 2) * 2;
    uint16_t *src_row_ptr = ((uint16_t *) src->data) + (y_row * src_w) + x_start;
    dst_row_ptr += x_start;
    bool yuv = (src->pixfmt == PIXFORMAT_YUV422);

    for (int i = 0; i < n; i += UINT16_VECTOR_SIZE) {
        v128_predicate_t pred = vpredicate_16(n - i);

        // Each 16-bit lane is MSB [C, Y] LSB, and each 32-bit lane is a pixel pair.
        v128_t pixels = vldr_u16_pred(src_row_ptr + i, pred);
        v128_t y = vand_u32(pixels, vdup_u16(0xff));
        v128_t c = vlsr_u16(pixels, 8);

        // Duplicate the first and second chroma value of each pair to both pixels.
        v128_t c0 = vand_u32(c, vdup_u32(0xffff));
        c0 = vorr_u32(c0, vlsl_u32(c0, 16));
        v128_t c1 = vlsr_u32(c, 16);
        c1 = vorr_u32(c1, vlsl_u32(c1, 16));

        v128_t u = vsub_s16(yuv ? c0 : c1, vdup_s16(128));
        v128_t v = vsub_s16(yuv ? c1 : c0, vdup_s16(128));

        v128_t ry = vasr_s16(vmul_n_s16(u, 179), 7);
        v128_t gy = vasr_s16(vmla_n_s16(u, 91, vmul_n_s16(v, 44)), 7);
        v128_t by = vasr_s16(vmul_n_s16(v, 227), 7);

        vrgb_pixels_t rgb;
        rgb.r = vusat_s16(vadd_s16(y, ry), 8);
        rgb.g = vusat_s16(vsub_s16(y, gy), 8);
        rgb.b = vusat_s16(vadd_s16(y, by), 8);
        vrgb_pixels_store_rgb565(dst_row_ptr, i, rgb, pred);
    }

    return x_start + n;
}
#endif

void imlib_deyuv_line(int x_start, int x_end, int y_row, void *dst_row_ptr, pixformat_t pixfmt, image_t *src) {
    int shift = (src->pixfmt == PIXFORMAT_YUV422) ? 16 : 0;
    int src_w = src->w, w_limit = src_w - 1;

    uint16_t *rowptr_yuv = ((uint16_t *) src->data) + (y_row * src_w);

    #if (__ARM_ARCH >= 8)
    if (pixfmt == PIXFORMAT_RGB565) {
        x_start = vdeyuv_line_rgb565(x_start, x_end, y_row, dst_row_ptr, src);
    }
    #endif

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
