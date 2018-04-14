/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#include "fsort.h"
#include "imlib.h"

void imlib_histeq(image_t *img, image_t *mask)
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            int a = img->w * img->h;
            float s = (COLOR_BINARY_MAX - COLOR_BINARY_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_BINARY_MAX - COLOR_BINARY_MIN + 1) * sizeof(uint32_t));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    hist[IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) - COLOR_BINARY_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_BINARY_MAX - COLOR_BINARY_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) continue;
                    int pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x,
                        fast_roundf((s * hist[pixel - COLOR_BINARY_MIN]) + COLOR_BINARY_MIN));
                }
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            int a = img->w * img->h;
            float s = (COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1) * sizeof(uint32_t));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    hist[IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) - COLOR_GRAYSCALE_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) continue;
                    int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x,
                        fast_roundf((s * hist[pixel - COLOR_GRAYSCALE_MIN]) + COLOR_GRAYSCALE_MIN));
                }
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            int a = img->w * img->h;
            float s = (COLOR_Y_MAX - COLOR_Y_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_Y_MAX - COLOR_Y_MIN + 1) * sizeof(uint32_t));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    hist[COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)) - COLOR_Y_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_Y_MAX - COLOR_Y_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) continue;
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x,
                        imlib_yuv_to_rgb(fast_roundf(s * hist[COLOR_RGB565_TO_Y(pixel) - COLOR_Y_MIN]),
                                         COLOR_RGB565_TO_U(pixel),
                                         COLOR_RGB565_TO_V(pixel)));
                }
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
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int acc = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            acc += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                        }
                    }

                    int pixel = fast_roundf(acc * over_n);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            fb_free();
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

#ifdef IMLIB_ENABLE_MEDIAN
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
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows);
            int *data = fb_alloc(n*sizeof(int));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int *data_ptr = data;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            *data_ptr++ = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                        }
                    }

                    fsort(data, n);

                    int pixel = data[int_percentile];

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
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
#endif // IMLIB_ENABLE_MEDIAN

#ifdef IMLIB_ENABLE_MODE
void imlib_mode_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows);
            int *bins = fb_alloc((COLOR_BINARY_MAX-COLOR_BINARY_MIN+1)*sizeof(int));

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    memset(bins, 0, (COLOR_BINARY_MAX-COLOR_BINARY_MIN+1)*sizeof(int));

                    int mcount = 0, mode = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
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
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
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
#endif // IMLIB_ENABLE_MODE

#ifdef IMLIB_ENABLE_MIDPOINT
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
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int min = COLOR_BINARY_MAX, max = COLOR_BINARY_MIN;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            min = IM_MIN(min, pixel);
                            max = IM_MAX(max, pixel);
                        }
                    }

                    int pixel = fast_roundf((min*min_bias)+(max*max_bias));

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            fb_free();
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
#endif // IMLIB_ENABLE_MIDPOINT

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
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int acc = 0, ptr = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            acc += krn[ptr++] * IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                        }
                    }

                    int pixel = IM_MAX(IM_MIN(fast_roundf(acc * m) + b, COLOR_BINARY_MAX), COLOR_BINARY_MIN);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            fb_free();
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

#ifdef IMLIB_ENABLE_BILATERAL
static float gaussian(float x, float sigma)
{
    return fast_expf((x * x) / (-2.0f * sigma * sigma)) / (fabsf(sigma) * 2.506628f); // sqrt(2 * PI)
}

static float distance(int x, int y)
{
    return fast_sqrtf((x * x) + (y * y));
}

void imlib_bilateral_filter(image_t *img, const int ksize, float color_sigma, float space_sigma, bool threshold, int offset, bool invert, image_t *mask)
{
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.bpp = img->bpp;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows);
            float *gi_lut = fb_alloc((COLOR_BINARY_MAX - COLOR_BINARY_MIN + 1) * sizeof(float));

            float max_color = IM_DIV(1.0f, COLOR_BINARY_MAX - COLOR_BINARY_MIN);
            for (int i = COLOR_BINARY_MIN; i <= COLOR_BINARY_MAX; i++) {
                gi_lut[i] = gaussian(i * max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float));

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int this_pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                    float i_acc = 0, w_acc = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            float w = gi_lut[abs(this_pixel - pixel)] * gs_lut[(n * (j + ksize)) + (k + ksize)];
                            i_acc += pixel * w;
                            w_acc += w;
                        }
                    }

                    int pixel = fast_roundf(IM_MIN(IM_DIV(i_acc, w_acc), COLOR_BINARY_MAX));

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = img->h - ksize, yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            fb_free();
            fb_free();
            fb_free();
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows);
            float *gi_lut = fb_alloc((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1) * sizeof(float));

            float max_color = IM_DIV(1.0f, COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN);
            for (int i = COLOR_GRAYSCALE_MIN; i <= COLOR_GRAYSCALE_MAX; i++) {
                gi_lut[i] = gaussian(i * max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float));

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int this_pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    float i_acc = 0, w_acc = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            float w = gi_lut[abs(this_pixel - pixel)] * gs_lut[(n * (j + ksize)) + (k + ksize)];
                            i_acc += pixel * w;
                            w_acc += w;
                        }
                    }

                    int pixel = fast_roundf(IM_MIN(IM_DIV(i_acc, w_acc), COLOR_GRAYSCALE_MAX));

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
            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows);
            float *r_gi_lut = fb_alloc((COLOR_R5_MAX - COLOR_R5_MIN + 1) * sizeof(float));
            float *g_gi_lut = fb_alloc((COLOR_G6_MAX - COLOR_G6_MIN + 1) * sizeof(float));
            float *b_gi_lut = fb_alloc((COLOR_B5_MAX - COLOR_B5_MIN + 1) * sizeof(float));

            float r_max_color = IM_DIV(1.0f, COLOR_R5_MAX - COLOR_R5_MIN);
            for (int i = COLOR_R5_MIN; i <= COLOR_R5_MAX; i++) {
                r_gi_lut[i] = gaussian(i * r_max_color, color_sigma);
            }

            float g_max_color = IM_DIV(1.0f, COLOR_G6_MAX - COLOR_G6_MIN);
            for (int i = COLOR_G6_MIN; i <= COLOR_G6_MAX; i++) {
                g_gi_lut[i] = gaussian(i * g_max_color, color_sigma);
            }

            float b_max_color = IM_DIV(1.0f, COLOR_B5_MAX - COLOR_B5_MIN);
            for (int i = COLOR_B5_MIN; i <= COLOR_B5_MAX; i++) {
                b_gi_lut[i] = gaussian(i * b_max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float));

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int this_pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    int r_this_pixel = COLOR_RGB565_TO_R5(this_pixel);
                    int g_this_pixel = COLOR_RGB565_TO_G6(this_pixel);
                    int b_this_pixel = COLOR_RGB565_TO_B5(this_pixel);
                    float r_i_acc = 0, r_w_acc = 0;
                    float g_i_acc = 0, g_w_acc = 0;
                    float b_i_acc = 0, b_w_acc = 0;

                    for (int j = -ksize; j <= ksize; j++) {
                        uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            int r_pixel = COLOR_RGB565_TO_R5(pixel);
                            int g_pixel = COLOR_RGB565_TO_G6(pixel);
                            int b_pixel = COLOR_RGB565_TO_B5(pixel);
                            float gs = gs_lut[(n * (j + ksize)) + (k + ksize)];
                            float r_w = r_gi_lut[abs(r_this_pixel - r_pixel)] * gs;
                            float g_w = g_gi_lut[abs(g_this_pixel - g_pixel)] * gs;
                            float b_w = b_gi_lut[abs(b_this_pixel - b_pixel)] * gs;
                            r_i_acc += r_pixel * r_w;
                            r_w_acc += r_w;
                            g_i_acc += g_pixel * g_w;
                            g_w_acc += g_w;
                            b_i_acc += b_pixel * b_w;
                            b_w_acc += b_w;
                        }
                    }

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(fast_roundf(IM_MIN(IM_DIV(r_i_acc, r_w_acc), COLOR_R5_MAX)),
                                                         fast_roundf(IM_MIN(IM_DIV(g_i_acc, g_w_acc), COLOR_G6_MAX)),
                                                         fast_roundf(IM_MIN(IM_DIV(b_i_acc, b_w_acc), COLOR_B5_MAX)));

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
            fb_free();
            break;
        }
        default: {
            break;
        }
    }
}
#endif // IMLIB_ENABLE_BILATERAL

#ifdef IMLIB_ENABLE_CARTOON
typedef struct imlib_cartoon_filter_mean_state {
    int r_acc, g_acc, b_acc, pixels;
} imlib_cartoon_filter_mean_state_t;

static void imlib_cartoon_filter_mean(image_t *img, int line, int l, int r, void *data)
{
    imlib_cartoon_filter_mean_state_t *state = (imlib_cartoon_filter_mean_state_t *) data;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                state->g_acc += IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, i);
                state->pixels += 1;
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                state->g_acc += IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, i);
                state->pixels += 1;
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, i);
                state->r_acc += COLOR_RGB565_TO_R5(pixel);
                state->g_acc += COLOR_RGB565_TO_G6(pixel);
                state->b_acc += COLOR_RGB565_TO_B5(pixel);
                state->pixels += 1;
            }
            break;
        }
        default: {
            break;
        }
    }
}

typedef struct imlib_cartoon_filter_fill_state {
    int mean;
} imlib_cartoon_filter_fill_state_t;

static void imlib_cartoon_filter_fill(image_t *img, int line, int l, int r, void *data)
{
    imlib_cartoon_filter_fill_state_t *state = (imlib_cartoon_filter_fill_state_t *) data;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, i, state->mean);
            }
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, i, state->mean);
            }
            break;
        }
        case IMAGE_BPP_RGB565: {
            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, i, state->mean);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_cartoon_filter(image_t *img, float seed_threshold, float floating_threshold, image_t *mask)
{
    image_t mean_image, fill_image;

    mean_image.w = img->w;
    mean_image.h = img->h;
    mean_image.bpp = IMAGE_BPP_BINARY;
    mean_image.data = fb_alloc0(image_size(&mean_image));

    fill_image.w = img->w;
    fill_image.h = img->h;
    fill_image.bpp = IMAGE_BPP_BINARY;
    fill_image.data = fb_alloc0(image_size(&fill_image));

    if (mask) {
        for (int y = 0, yy = fill_image.h; y < yy; y++) {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&fill_image, y);
            for (int x = 0, xx = fill_image.w; x < xx; x++) {
                if (image_get_mask_pixel(mask, x, y)) IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x);
            }
        }
    }

    int color_seed_threshold = 0;
    int color_floating_threshold = 0;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            color_seed_threshold = fast_roundf(seed_threshold * COLOR_BINARY_MAX);
            color_floating_threshold = fast_roundf(floating_threshold * COLOR_BINARY_MAX);
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            color_seed_threshold = fast_roundf(seed_threshold * COLOR_GRAYSCALE_MAX);
            color_floating_threshold = fast_roundf(floating_threshold * COLOR_GRAYSCALE_MAX);
            break;
        }
        case IMAGE_BPP_RGB565: {
            color_seed_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_roundf(seed_threshold * COLOR_R5_MAX),
                                                            fast_roundf(seed_threshold * COLOR_G6_MAX),
                                                            fast_roundf(seed_threshold * COLOR_B5_MAX));
            color_floating_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_roundf(floating_threshold * COLOR_R5_MAX),
                                                                fast_roundf(floating_threshold * COLOR_G6_MAX),
                                                                fast_roundf(floating_threshold * COLOR_B5_MAX));
            break;
        }
        default: {
            break;
        }
    }

    for (int y = 0, yy = img->h; y < yy; y++) {
        uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&mean_image, y);
        for (int x = 0, xx = img->w; x < xx; x++) {
            if (!IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) {

                imlib_cartoon_filter_mean_state_t mean_state;
                memset(&mean_state, 0, sizeof(imlib_cartoon_filter_mean_state_t));
                imlib_flood_fill_int(&mean_image, img, x, y, color_seed_threshold, color_floating_threshold,
                                     imlib_cartoon_filter_mean, &mean_state);

                imlib_cartoon_filter_fill_state_t fill_state;
                memset(&fill_state, 0, sizeof(imlib_cartoon_filter_fill_state_t));

                switch(img->bpp) {
                    case IMAGE_BPP_BINARY: {
                        fill_state.mean = mean_state.g_acc / mean_state.pixels;
                        break;
                    }
                    case IMAGE_BPP_GRAYSCALE: {
                        fill_state.mean = mean_state.g_acc / mean_state.pixels;
                        break;
                    }
                    case IMAGE_BPP_RGB565: {
                        fill_state.mean = COLOR_R5_G6_B5_TO_RGB565(mean_state.r_acc / mean_state.pixels,
                                                                   mean_state.g_acc / mean_state.pixels,
                                                                   mean_state.b_acc / mean_state.pixels);
                        break;
                    }
                    default: {
                        break;
                    }
                }

                imlib_flood_fill_int(&fill_image, img, x, y, color_seed_threshold, color_floating_threshold,
                                     imlib_cartoon_filter_fill, &fill_state);
            }
        }
    }

    fb_free();
    fb_free();
}
#endif // IMLIB_ENABLE_CARTOON
