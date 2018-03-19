/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "fsort.h"
#include "imlib.h"

void imlib_histeq(image_t *img)
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            int a = img->w * img->h;
            float s = (COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1)*sizeof(uint32_t));
            uint8_t *pixels = (uint8_t *) img->data;

            // Compute the image histogram
            for (int i=0; i<a; i++) {
                hist[pixels[i]-COLOR_GRAYSCALE_MIN] += 1;
            }

            // Compute the CDF
            for (int i=0, sum=0; i<(COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1); i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int i=0; i<a; i++) {
                int pixel = pixels[i];
                pixels[i] = (s * hist[pixel-COLOR_GRAYSCALE_MIN]) + COLOR_GRAYSCALE_MIN;
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            int a = img->w * img->h;
            float s = (COLOR_Y_MAX-COLOR_Y_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_Y_MAX-COLOR_Y_MIN+1)*sizeof(uint32_t));
            uint16_t *pixels = (uint16_t *) img->data;

            // Compute image histogram
            for (int i=0; i<a; i++) {
                hist[COLOR_RGB565_TO_Y(pixels[i])-COLOR_Y_MIN] += 1;
            }

            // Compute the CDF
            for (int i=0, sum=0; i<(COLOR_Y_MAX-COLOR_Y_MIN+1); i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int i=0; i<a; i++) {
                int pixel = pixels[i];
                pixels[i] = imlib_yuv_to_rgb((s * hist[COLOR_RGB565_TO_Y(pixel)-COLOR_Y_MIN]),
                                             COLOR_RGB565_TO_U(pixel),
                                             COLOR_RGB565_TO_V(pixel));
            }

            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

// ksize == 0 -> 1x1 kernel
// ksize == 1 -> 3x3 kernel
// ...
// ksize == n -> ((n*2)+1)x((n*2)+1) kernel

void imlib_mean_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    float over_n = 1.0f / (((ksize*2)+1)*((ksize*2)+1));

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int acc = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            acc += IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                        }
                    }

                    int pixel = fast_roundf(acc * over_n);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int r_acc = 0, g_acc = 0, b_acc = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            r_acc += COLOR_RGB565_TO_R5(pixel);
                            g_acc += COLOR_RGB565_TO_G6(pixel);
                            b_acc += COLOR_RGB565_TO_B5(pixel);
                        }
                    }

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(fast_roundf(r_acc * over_n),
                                                         fast_roundf(g_acc * over_n),
                                                         fast_roundf(b_acc * over_n));

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_median_filter(image_t *img, const int ksize, float percentile, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    int n = ((ksize*2)+1)*((ksize*2)+1), int_percentile = fast_roundf(percentile * (n - 1));

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows);
            int *data = fb_alloc(n*sizeof(int));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int *data_ptr = data;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            *data_ptr++ = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                        }
                    }

                    fsort(data, n);

                    int pixel = data[int_percentile];

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows);
            int *r_data = fb_alloc(n*sizeof(int));
            int *g_data = fb_alloc(n*sizeof(int));
            int *b_data = fb_alloc(n*sizeof(int));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int *r_data_ptr = r_data, *g_data_ptr = g_data, *b_data_ptr = b_data;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            *r_data_ptr++ = COLOR_RGB565_TO_R5(pixel);
                            *g_data_ptr++ = COLOR_RGB565_TO_G6(pixel);
                            *b_data_ptr++ = COLOR_RGB565_TO_B5(pixel);
                        }
                    }

                    fsort(r_data, n);
                    fsort(g_data, n);
                    fsort(b_data, n);

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(r_data[int_percentile],
                                                         g_data[int_percentile],
                                                         b_data[int_percentile]);

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
            fb_free();
            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_mode_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows);
            int *bins = fb_alloc((COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1)*sizeof(int));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    memset(bins, 0, (COLOR_GRAYSCALE_MAX-COLOR_GRAYSCALE_MIN+1)*sizeof(int));

                    int mcount = 0, mode = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            bins[pixel]++;

                            if (bins[pixel] > mcount) {
                                mcount = bins[pixel];
                                mode = pixel;
                            }
                        }
                    }

                    int pixel = mode;

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows);
            int *r_bins = fb_alloc((COLOR_R5_MAX-COLOR_R5_MIN+1)*sizeof(int));
            int *g_bins = fb_alloc((COLOR_G6_MAX-COLOR_G6_MIN+1)*sizeof(int));
            int *b_bins = fb_alloc((COLOR_B5_MAX-COLOR_B5_MIN+1)*sizeof(int));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    memset(r_bins, 0, (COLOR_R5_MAX-COLOR_R5_MIN+1)*sizeof(int));
                    memset(g_bins, 0, (COLOR_G6_MAX-COLOR_G6_MIN+1)*sizeof(int));
                    memset(b_bins, 0, (COLOR_B5_MAX-COLOR_B5_MIN+1)*sizeof(int));

                    int r_mcount = 0, r_mode = 0;
                    int g_mcount = 0, g_mode = 0;
                    int b_mcount = 0, b_mode = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            int r_pixel = COLOR_RGB565_TO_R5(pixel);
                            int g_pixel = COLOR_RGB565_TO_G6(pixel);
                            int b_pixel = COLOR_RGB565_TO_B5(pixel);
                            r_bins[r_pixel]++;
                            g_bins[g_pixel]++;
                            b_bins[b_pixel]++;

                            if (r_bins[r_pixel] > r_mcount) {
                                r_mcount = r_bins[r_pixel];
                                r_mode = r_pixel;
                            }

                            if (g_bins[g_pixel] > g_mcount) {
                                g_mcount = g_bins[g_pixel];
                                g_mode = g_pixel;
                            }

                            if (b_bins[b_pixel] > b_mcount) {
                                b_mcount = b_bins[b_pixel];
                                b_mode = b_pixel;
                            }
                        }
                    }

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(r_mode, g_mode, b_mode);

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
            fb_free();
            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_midpoint_filter(image_t *img, const int ksize, float bias, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    float max_bias = bias, min_bias = 1.0f - bias;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int min = COLOR_GRAYSCALE_MAX, max = COLOR_GRAYSCALE_MIN;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            min = IM_MIN(min, pixel);
                            max = IM_MAX(max, pixel);
                        }
                    }

                    int pixel = fast_roundf((min*min_bias)+(max*max_bias));

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int r_min = COLOR_R5_MAX, r_max = COLOR_R5_MIN;
                    int g_min = COLOR_G6_MAX, g_max = COLOR_G6_MIN;
                    int b_min = COLOR_B5_MAX, b_max = COLOR_B5_MIN;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            int r_pixel = COLOR_RGB565_TO_R5(pixel);
                            int g_pixel = COLOR_RGB565_TO_G6(pixel);
                            int b_pixel = COLOR_RGB565_TO_B5(pixel);
                            r_min = IM_MIN(r_min, r_pixel);
                            r_max = IM_MAX(r_max, r_pixel);
                            g_min = IM_MIN(g_min, g_pixel);
                            g_max = IM_MAX(g_max, g_pixel);
                            b_min = IM_MIN(b_min, b_pixel);
                            b_max = IM_MAX(b_max, b_pixel);
                        }
                    }

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(fast_roundf((r_min*min_bias)+(r_max*max_bias)),
                                                         fast_roundf((g_min*min_bias)+(g_max*max_bias)),
                                                         fast_roundf((b_min*min_bias)+(b_max*max_bias)));

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}

// http://www.fmwconcepts.com/imagemagick/digital_image_filtering.pdf

void imlib_morph(image_t *img, const int ksize, const int *krn, const float m, const int b, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int acc = 0, ptr = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            acc += krn[ptr++] * IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                        }
                    }

                    int pixel = IM_MAX(IM_MIN(fast_roundf(acc * m) + b, COLOR_GRAYSCALE_MAX), COLOR_GRAYSCALE_MIN);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int r_acc = 0, g_acc = 0, b_acc = 0, ptr = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            r_acc += krn[ptr] * COLOR_RGB565_TO_R5(pixel);
                            g_acc += krn[ptr] * COLOR_RGB565_TO_G6(pixel);
                            b_acc += krn[ptr++] * COLOR_RGB565_TO_B5(pixel);
                        }
                    }

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(IM_MAX(IM_MIN(fast_roundf(r_acc * m) + b, COLOR_R5_MAX), COLOR_R5_MIN),
                                                         IM_MAX(IM_MIN(fast_roundf(g_acc * m) + b, COLOR_G6_MAX), COLOR_G6_MIN),
                                                         IM_MAX(IM_MIN(fast_roundf(b_acc * m) + b, COLOR_B5_MAX), COLOR_B5_MIN));

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}
