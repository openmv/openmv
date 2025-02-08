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
 * Image library.
 */
#include <stdlib.h>
#include "py/obj.h"
#include "py/runtime.h"

#include "font.h"
#include "array.h"
#include "file_utils.h"
#include "imlib.h"
#include "omv_common.h"
#include "omv_gpu.h"
#include "omv_boardconfig.h"

void imlib_init_all() {
    #if (OMV_GPU_ENABLE == 1)
    omv_gpu_init();
    #endif
    #if (OMV_JPEG_CODEC_ENABLE == 1)
    imlib_hardware_jpeg_init();
    #endif
}

void imlib_deinit_all() {
    #if (OMV_GPU_ENABLE == 1)
    omv_gpu_deinit();
    #endif
    #if (OMV_JPEG_CODEC_ENABLE == 1)
    imlib_hardware_jpeg_deinit();
    #endif
}

int imlib_ksize_to_n(int ksize) {
    return ((ksize * 2) + 1) * ((ksize * 2) + 1);
}

/////////////////
// Point Stuff //
/////////////////

void point_init(point_t *ptr, int x, int y) {
    ptr->x = x;
    ptr->y = y;
}

void point_copy(point_t *dst, point_t *src) {
    memcpy(dst, src, sizeof(point_t));
}

bool point_equal_fast(point_t *ptr0, point_t *ptr1) {
    return !memcmp(ptr0, ptr1, sizeof(point_t));
}

int point_quadrance(point_t *ptr0, point_t *ptr1) {
    int delta_x = ptr0->x - ptr1->x;
    int delta_y = ptr0->y - ptr1->y;
    return (delta_x * delta_x) + (delta_y * delta_y);
}

void point_rotate(int x, int y, float r, int center_x, int center_y, int16_t *new_x, int16_t *new_y) {
    x -= center_x;
    y -= center_y;
    *new_x = (x * cosf(r)) - (y * sinf(r)) + center_x;
    *new_y = (x * sinf(r)) + (y * cosf(r)) + center_y;
}

void point_min_area_rectangle(point_t *corners, point_t *new_corners, int corners_len) {
    // Corners need to be sorted!
    int i_min = 0;
    int i_min_area = INT_MAX;
    int i_x0 = 0, i_y0 = 0;
    int i_x1 = 0, i_y1 = 0;
    int i_x2 = 0, i_y2 = 0;
    int i_x3 = 0, i_y3 = 0;
    float i_r = 0;

    // This algorithm aligns the 4 edges produced by the 4 corners to the x axis and then computes the
    // min area rect for each alignment. The smallest rect is chosen and then re-rotated and returned.
    for (int i = 0; i < corners_len; i++) {
        int16_t x0 = corners[i].x, y0 = corners[i].y;
        int x_diff = corners[(i + 1) % corners_len].x - corners[i].x;
        int y_diff = corners[(i + 1) % corners_len].y - corners[i].y;
        float r = -fast_atan2f(y_diff, x_diff);

        int16_t x1[corners_len - 1];
        int16_t y1[corners_len - 1];
        for (int j = 0, jj = corners_len - 1; j < jj; j++) {
            point_rotate(corners[(i + j + 1) % corners_len].x, corners[(i + j + 1) % corners_len].y, r, x0, y0, x1 + j, y1 + j);
        }

        int minx = x0;
        int maxx = x0;
        int miny = y0;
        int maxy = y0;
        for (int j = 0, jj = corners_len - 1; j < jj; j++) {
            minx = IM_MIN(minx, x1[j]);
            maxx = IM_MAX(maxx, x1[j]);
            miny = IM_MIN(miny, y1[j]);
            maxy = IM_MAX(maxy, y1[j]);
        }

        int area = (maxx - minx + 1) * (maxy - miny + 1);
        if (area < i_min_area) {
            i_min = i;
            i_min_area = area;
            i_x0 = minx, i_y0 = miny;
            i_x1 = maxx, i_y1 = miny;
            i_x2 = maxx, i_y2 = maxy;
            i_x3 = minx, i_y3 = maxy;
            i_r = r;
        }
    }

    point_rotate(i_x0, i_y0, -i_r, corners[i_min].x, corners[i_min].y, &new_corners[0].x, &new_corners[0].y);
    point_rotate(i_x1, i_y1, -i_r, corners[i_min].x, corners[i_min].y, &new_corners[1].x, &new_corners[1].y);
    point_rotate(i_x2, i_y2, -i_r, corners[i_min].x, corners[i_min].y, &new_corners[2].x, &new_corners[2].y);
    point_rotate(i_x3, i_y3, -i_r, corners[i_min].x, corners[i_min].y, &new_corners[3].x, &new_corners[3].y);
}

////////////////
// Line Stuff //
////////////////

// http://www.skytopia.com/project/articles/compsci/clipping.html
bool lb_clip_line(line_t *l, int x, int y, int w, int h) {
    // line is drawn if this returns true
    int xdelta = l->x2 - l->x1, ydelta = l->y2 - l->y1, p[4], q[4];
    float umin = 0, umax = 1;

    p[0] = -(xdelta);
    p[1] = +(xdelta);
    p[2] = -(ydelta);
    p[3] = +(ydelta);

    q[0] = l->x1 - (x);
    q[1] = (x + w - 1) - l->x1;
    q[2] = l->y1 - (y);
    q[3] = (y + h - 1) - l->y1;

    for (int i = 0; i < 4; i++) {
        if (p[i]) {
            float u = ((float) q[i]) / ((float) p[i]);

            if (p[i] < 0) {
                // outside to inside
                if (u > umax) {
                    return false;
                }
                if (u > umin) {
                    umin = u;
                }
            }

            if (p[i] > 0) {
                // inside to outside
                if (u < umin) {
                    return false;
                }
                if (u < umax) {
                    umax = u;
                }
            }

        } else if (q[i] < 0) {
            return false;
        }
    }

    if (umax < umin) {
        return false;
    }

    int x1_c = l->x1 + (xdelta * umin);
    int y1_c = l->y1 + (ydelta * umin);
    int x2_c = l->x1 + (xdelta * umax);
    int y2_c = l->y1 + (ydelta * umax);
    l->x1 = x1_c;
    l->y1 = y1_c;
    l->x2 = x2_c;
    l->y2 = y2_c;

    return true;
}

/////////////////////
// Rectangle Stuff //
/////////////////////

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h) {
    ptr->x = x;
    ptr->y = y;
    ptr->w = w;
    ptr->h = h;
}

void rectangle_copy(rectangle_t *dst, rectangle_t *src) {
    memcpy(dst, src, sizeof(rectangle_t));
}

bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1) {
    return !memcmp(ptr0, ptr1, sizeof(rectangle_t));
}

bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1) {
    int x0 = ptr0->x;
    int y0 = ptr0->y;
    int w0 = ptr0->w;
    int h0 = ptr0->h;
    int x1 = ptr1->x;
    int y1 = ptr1->y;
    int w1 = ptr1->w;
    int h1 = ptr1->h;
    return (x0 < (x1 + w1)) && (y0 < (y1 + h1)) && (x1 < (x0 + w0)) && (y1 < (y0 + h0));
}

void rectangle_intersected(rectangle_t *dst, rectangle_t *src) {
    int leftX = IM_MAX(dst->x, src->x);
    int topY = IM_MAX(dst->y, src->y);
    int rightX = IM_MIN(dst->x + dst->w, src->x + src->w);
    int bottomY = IM_MIN(dst->y + dst->h, src->y + src->h);
    dst->x = leftX;
    dst->y = topY;
    dst->w = rightX - leftX;
    dst->h = bottomY - topY;
}

void rectangle_united(rectangle_t *dst, rectangle_t *src) {
    int leftX = IM_MIN(dst->x, src->x);
    int topY = IM_MIN(dst->y, src->y);
    int rightX = IM_MAX(dst->x + dst->w, src->x + src->w);
    int bottomY = IM_MAX(dst->y + dst->h, src->y + src->h);
    dst->x = leftX;
    dst->y = topY;
    dst->w = rightX - leftX;
    dst->h = bottomY - topY;
}

/////////////////
// Image Stuff //
/////////////////

void image_xalloc(image_t *img, size_t size) {
    // Round the size up to ensure that the allocation is a multiple of the alignment in bytes.
    // This ensures after address alignment that the data can be modified without affecting other cache lines.
    size = ((size + OMV_ALLOC_ALIGNMENT - 1) / OMV_ALLOC_ALIGNMENT) * OMV_ALLOC_ALIGNMENT;
    img->_raw = xalloc(size + OMV_ALLOC_ALIGNMENT - 1);
    // Offset the data pointer to ensure it is aligned.
    img->data = (void *) (((uintptr_t) img->_raw + OMV_ALLOC_ALIGNMENT - 1) & ~(OMV_ALLOC_ALIGNMENT - 1));
}

void image_xalloc0(image_t *img, size_t size) {
    image_xalloc(img, size);
    memset(img->data, 0, size);
}

void image_init(image_t *ptr, int w, int h, pixformat_t pixfmt, uint32_t size, void *pixels) {
    ptr->w = w;
    ptr->h = h;
    ptr->pixfmt = pixfmt;
    ptr->size = size;
    ptr->pixels = pixels;
}

void image_copy(image_t *dst, image_t *src) {
    memcpy(dst, src, sizeof(image_t));
}

size_t image_line_size(image_t *ptr) {
    switch (ptr->pixfmt) {
        case PIXFORMAT_BINARY: {
            return IMAGE_BINARY_LINE_LEN_BYTES(ptr);
        }
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER_ANY: {
            // re-use
            return IMAGE_GRAYSCALE_LINE_LEN_BYTES(ptr);
        }
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV_ANY: {
            // re-use
            return IMAGE_RGB565_LINE_LEN_BYTES(ptr);
        }
        default: {
            return 0;
        }
    }
}

size_t image_size(image_t *ptr) {
    switch (ptr->pixfmt) {
        case PIXFORMAT_BINARY: {
            return IMAGE_BINARY_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER_ANY: {
            // re-use
            return IMAGE_GRAYSCALE_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV_ANY: {
            // re-use
            return IMAGE_RGB565_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case PIXFORMAT_COMPRESSED_ANY: {
            return ptr->size;
        }
        default: {
            return 0;
        }
    }
}

bool image_get_mask_pixel(image_t *ptr, int x, int y) {
    if ((0 <= x) && (x < ptr->w) && (0 <= y) && (y < ptr->h)) {
        switch (ptr->pixfmt) {
            case PIXFORMAT_BINARY: {
                return IMAGE_GET_BINARY_PIXEL(ptr, x, y);
            }
            case PIXFORMAT_GRAYSCALE: {
                return COLOR_GRAYSCALE_TO_BINARY(IMAGE_GET_GRAYSCALE_PIXEL(ptr, x, y));
            }
            case PIXFORMAT_RGB565: {
                return COLOR_RGB565_TO_BINARY(IMAGE_GET_RGB565_PIXEL(ptr, x, y));
            }
            default: {
                return false;
            }
        }
    }

    return false;
}

// Gamma uncompress
extern const float xyz_table[256];

const int8_t kernel_gauss_3[3 * 3] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1,
};

const int8_t kernel_gauss_5[5 * 5] = {
    1,  4,  6,  4, 1,
    4, 16, 24, 16, 4,
    6, 24, 36, 24, 6,
    4, 16, 24, 16, 4,
    1,  4,  6,  4, 1
};

const int kernel_laplacian_3[3 * 3] = {
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
};

const int kernel_high_pass_3[3 * 3] = {
    -1, -1, -1,
    -1, +8, -1,
    -1, -1, -1
};

// This function fills a grayscale image from an array of floating point numbers that are scaled
// between min and max. The image w*h must equal the floating point array w*h.
void imlib_fill_image_from_float(image_t *img, int w, int h, float *data, float min, float max,
                                 bool mirror, bool flip, bool dst_transpose, bool src_transpose) {
    float tmp = min;
    min = (min < max) ? min : max;
    max = (max > tmp) ? max : tmp;

    float diff = 255.f / (max - min);
    int w_1 = w - 1;
    int h_1 = h - 1;

    if (!src_transpose) {
        for (int y = 0; y < h; y++) {
            int y_dst = flip ? (h_1 - y) : y;
            float *raw_row = data + (y * w);
            uint8_t *row_pointer = ((uint8_t *) img->data) + (y_dst * w);
            uint8_t *t_row_pointer = ((uint8_t *) img->data) + y_dst;

            for (int x = 0; x < w; x++) {
                int x_dst = mirror ? (w_1 - x) : x;
                float raw = raw_row[x];

                if (raw < min) {
                    raw = min;
                }

                if (raw > max) {
                    raw = max;
                }

                int pixel = fast_roundf((raw - min) * diff);
                pixel = __USAT(pixel, 8);

                if (!dst_transpose) {
                    row_pointer[x_dst] = pixel;
                } else {
                    t_row_pointer[x_dst * h] = pixel;
                }
            }
        }
    } else {
        for (int x = 0; x < w; x++) {
            int x_dst = mirror ? (w_1 - x) : x;
            float *raw_row = data + (x * h);
            uint8_t *t_row_pointer = ((uint8_t *) img->data) + (x_dst * h);
            uint8_t *row_pointer = ((uint8_t *) img->data) + x_dst;

            for (int y = 0; y < h; y++) {
                int y_dst = flip ? (h_1 - y) : y;
                float raw = raw_row[y];

                if (raw < min) {
                    raw = min;
                }

                if (raw > max) {
                    raw = max;
                }

                int pixel = fast_roundf((raw - min) * diff);
                pixel = __USAT(pixel, 8);

                if (!dst_transpose) {
                    row_pointer[y_dst * w] = pixel;
                } else {
                    t_row_pointer[y_dst] = pixel;
                }
            }
        }
    }
}

int8_t imlib_rgb565_to_l(uint16_t pixel) {
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);

    y = (y > 0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);

    return IM_CLAMP(fast_floorf(116 * y) - 16, COLOR_L_MIN, COLOR_L_MAX);
}

int8_t imlib_rgb565_to_a(uint16_t pixel) {
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float x = ((r_lin * 0.4124f) + (g_lin * 0.3576f) + (b_lin * 0.1805f)) * (1.0f / 095.047f);
    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);

    x = (x > 0.008856f) ? fast_cbrtf(x) : ((x * 7.787037f) + 0.137931f);
    y = (y > 0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);

    return __SSAT(fast_floorf(500 * (x - y)), 8);
}

int8_t imlib_rgb565_to_b(uint16_t pixel) {
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);
    float z = ((r_lin * 0.0193f) + (g_lin * 0.1192f) + (b_lin * 0.9505f)) * (1.0f / 108.883f);

    y = (y > 0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);
    z = (z > 0.008856f) ? fast_cbrtf(z) : ((z * 7.787037f) + 0.137931f);

    return __SSAT(fast_floorf(200 * (y - z)), 8);
}

// https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions
// https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation
uint16_t imlib_lab_to_rgb(uint8_t l, int8_t a, int8_t b) {
    float x = ((l + 16) * 0.008621f) + (a * 0.002f);
    float y = ((l + 16) * 0.008621f);
    float z = ((l + 16) * 0.008621f) - (b * 0.005f);

    x = ((x > 0.206897f) ? (x * x * x) : ((0.128419f * x) - 0.017713f)) * 095.047f;
    y = ((y > 0.206897f) ? (y * y * y) : ((0.128419f * y) - 0.017713f)) * 100.000f;
    z = ((z > 0.206897f) ? (z * z * z) : ((0.128419f * z) - 0.017713f)) * 108.883f;

    float r_lin = ((x * +3.2406f) + (y * -1.5372f) + (z * -0.4986f)) / 100.0f;
    float g_lin = ((x * -0.9689f) + (y * +1.8758f) + (z * +0.0415f)) / 100.0f;
    float b_lin = ((x * +0.0557f) + (y * -0.2040f) + (z * +1.0570f)) / 100.0f;

    r_lin = (r_lin > 0.0031308f) ? ((1.055f * powf(r_lin, 0.416666f)) - 0.055f) : (r_lin * 12.92f);
    g_lin = (g_lin > 0.0031308f) ? ((1.055f * powf(g_lin, 0.416666f)) - 0.055f) : (g_lin * 12.92f);
    b_lin = (b_lin > 0.0031308f) ? ((1.055f * powf(b_lin, 0.416666f)) - 0.055f) : (b_lin * 12.92f);

    uint32_t red = __USAT(fast_floorf(r_lin * COLOR_R8_MAX), 8);
    uint32_t green = __USAT(fast_floorf(g_lin * COLOR_G8_MAX), 8);
    uint32_t blue = __USAT(fast_floorf(b_lin * COLOR_B8_MAX), 8);

    return COLOR_R8_G8_B8_TO_RGB565(red, green, blue);
}

// https://en.wikipedia.org/wiki/YCbCr -> JPEG Conversion
uint16_t imlib_yuv_to_rgb(uint8_t y, int8_t u, int8_t v) {
    uint32_t r = __USAT(y + ((91881 * v) >> 16), 8);
    uint32_t g = __USAT(y - (((22554 * u) + (46802 * v)) >> 16), 8);
    uint32_t b = __USAT(y + ((116130 * u) >> 16), 8);

    return COLOR_R8_G8_B8_TO_RGB565(r, g, b);
}

////////////////////////////////////////////////////////////////////////////////

#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
static save_image_format_t imblib_parse_extension(image_t *img, const char *path) {
    size_t l = strlen(path);
    const char *p = path + l;
    if (l >= 5) {
        if (((p[-1] == 'g') || (p[-1] == 'G'))
            && ((p[-2] == 'e') || (p[-2] == 'E'))
            && ((p[-3] == 'p') || (p[-3] == 'P'))
            && ((p[-4] == 'j') || (p[-4] == 'J'))
            && ((p[-5] == '.') || (p[-5] == '.'))) {
            // Will convert to JPG if not.
            return FORMAT_JPG;
        }
    }
    if (l >= 4) {
        if (((p[-1] == 'g') || (p[-1] == 'G'))
            && ((p[-2] == 'p') || (p[-2] == 'P'))
            && ((p[-3] == 'j') || (p[-3] == 'J'))
            && ((p[-4] == '.') || (p[-4] == '.'))) {
            // Will convert to JPG if not.
            return FORMAT_JPG;
        } else if (((p[-1] == 'g') || (p[-1] == 'G'))
                   && ((p[-2] == 'n') || (p[-2] == 'N'))
                   && ((p[-3] == 'p') || (p[-3] == 'P'))
                   && ((p[-4] == '.') || (p[-4] == '.'))) {
            // Will convert to PNG if not.
            return FORMAT_PNG;
        } else if (((p[-1] == 'p') || (p[-1] == 'P'))
                   && ((p[-2] == 'm') || (p[-2] == 'M'))
                   && ((p[-3] == 'b') || (p[-3] == 'B'))
                   && ((p[-4] == '.') || (p[-4] == '.'))) {
            if (IM_IS_JPEG(img) || IM_IS_BAYER(img)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image is not BMP!"));
            }
            return FORMAT_BMP;
        } else if (((p[-1] == 'm') || (p[-1] == 'M'))
                   && ((p[-2] == 'p') || (p[-2] == 'P'))
                   && ((p[-3] == 'p') || (p[-3] == 'P'))
                   && ((p[-4] == '.') || (p[-4] == '.'))) {
            if (!IM_IS_RGB565(img)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image is not PPM!"));
            }
            return FORMAT_PNM;
        } else if (((p[-1] == 'm') || (p[-1] == 'M'))
                   && ((p[-2] == 'g') || (p[-2] == 'G'))
                   && ((p[-3] == 'p') || (p[-3] == 'P'))
                   && ((p[-4] == '.') || (p[-4] == '.'))) {
            if (!IM_IS_GS(img)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image is not PGM!"));
            }
            return FORMAT_PNM;
        } else if (((p[-1] == 'w') || (p[-1] == 'W'))
                   && ((p[-2] == 'a') || (p[-2] == 'A'))
                   && ((p[-3] == 'r') || (p[-3] == 'R'))
                   && ((p[-4] == '.') || (p[-4] == '.'))) {
            if (!IM_IS_BAYER(img)) {
                mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Image is not BAYER!"));
            }
            return FORMAT_RAW;
        }

    }
    return FORMAT_DONT_CARE;
}

bool imlib_read_geometry(FIL *fp, image_t *img, const char *path, img_read_settings_t *rs) {
    char magic[4];
    file_open(fp, path, false, FA_READ | FA_OPEN_EXISTING);
    file_read(fp, &magic, 4);
    file_close(fp);

    bool vflipped = false;
    if ((magic[0] == 'P')
        && ((magic[1] == '2') || (magic[1] == '3')
            || (magic[1] == '5') || (magic[1] == '6'))) {
        // PPM
        rs->format = FORMAT_PNM;
        file_open(fp, path, true, FA_READ | FA_OPEN_EXISTING);
        ppm_read_geometry(fp, img, path, &rs->ppm_rs);
    } else if ((magic[0] == 'B') && (magic[1] == 'M')) {
        // BMP
        rs->format = FORMAT_BMP;
        file_open(fp, path, true, FA_READ | FA_OPEN_EXISTING);
        vflipped = bmp_read_geometry(fp, img, path, &rs->bmp_rs);
    } else if ((magic[0] == 0xFF) && (magic[1] == 0xD8)) {
        // JPG
        rs->format = FORMAT_JPG;
        file_open(fp, path, false, FA_READ | FA_OPEN_EXISTING);
        jpeg_read_geometry(fp, img, path, &rs->jpg_rs);
        file_buffer_on(fp);
    } else if ((magic[0] == 0x89) && (magic[1] == 0x50) && (magic[2] == 0x4E) && (magic[3] == 0x47)) {
        // PNG
        rs->format = FORMAT_PNG;
        file_open(fp, path, false, FA_READ | FA_OPEN_EXISTING);
        png_read_geometry(fp, img, path, &rs->png_rs);
        file_buffer_on(fp);
    } else {
        file_raise_format(NULL);
    }
    imblib_parse_extension(img, path); // Enforce extension!
    return vflipped;
}
#endif  //IMLIB_ENABLE_IMAGE_FILE_IO

#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
void imlib_load_image(image_t *img, const char *path) {
    FIL fp;
    char magic[4];
    file_open(&fp, path, false, FA_READ | FA_OPEN_EXISTING);
    file_read(&fp, &magic, 4);
    file_close(&fp);

    if ((magic[0] == 'P')
        && ((magic[1] == '2') || (magic[1] == '3')
            || (magic[1] == '5') || (magic[1] == '6'))) {
        // PPM
        ppm_read(img, path);
    } else if ((magic[0] == 'B') && (magic[1] == 'M')) {
        // BMP
        bmp_read(img, path);
    } else if ((magic[0] == 0xFF) && (magic[1] == 0xD8)) {
        // JPEG
        jpeg_read(img, path);
    } else if ((magic[0] == 0x89) && (magic[1] == 0x50) && (magic[2] == 0x4E) && (magic[3] == 0x47)) {
        // PNG
        png_read(img, path);
    } else {
        file_raise_format(NULL);
    }
    imblib_parse_extension(img, path); // Enforce extension!
}

void imlib_save_image(image_t *img, const char *path, rectangle_t *roi, int quality) {
    switch (imblib_parse_extension(img, path)) {
        case FORMAT_BMP:
            bmp_write_subimg(img, path, roi);
            break;
        case FORMAT_PNM:
            ppm_write_subimg(img, path, roi);
            break;
        case FORMAT_RAW: {
            FIL fp;
            file_open(&fp, path, false, FA_WRITE | FA_CREATE_ALWAYS);
            file_write(&fp, img->pixels, img->w * img->h);
            file_close(&fp);
            break;
        }
        case FORMAT_JPG:
            jpeg_write(img, path, quality);
            break;
        case FORMAT_PNG:
            png_write(img, path);
            break;
        case FORMAT_DONT_CARE:
            // Path doesn't have an extension.
            if (IM_IS_JPEG(img)) {
                char *new_path = strcat(strcpy(fb_alloc(strlen(path) + 5, 0), path), ".jpg");
                jpeg_write(img, new_path, quality);
                fb_free();
            } else if (img->pixfmt == PIXFORMAT_PNG) {
                char *new_path = strcat(strcpy(fb_alloc(strlen(path) + 5, 0), path), ".png");
                png_write(img, new_path);
                fb_free();
            } else if (IM_IS_BAYER(img)) {
                FIL fp;
                char *new_path = strcat(strcpy(fb_alloc(strlen(path) + 5, 0), path), ".raw");
                file_open(&fp, new_path, false, FA_WRITE | FA_CREATE_ALWAYS);
                file_write(&fp, img->pixels, img->w * img->h);
                file_close(&fp);
                fb_free();
            } else {
                // RGB or GS, save as BMP.
                char *new_path = strcat(strcpy(fb_alloc(strlen(path) + 5, 0), path), ".bmp");
                bmp_write_subimg(img, new_path, roi);
                fb_free();
            }
            break;
    }
}
#endif //IMLIB_ENABLE_IMAGE_FILE_IO

////////////////////////////////////////////////////////////////////////////////

void imlib_zero(image_t *img, image_t *mask, bool invert) {
    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (image_get_mask_pixel(mask, x, y) ^ invert) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, 0);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (image_get_mask_pixel(mask, x, y) ^ invert) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, 0);
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (image_get_mask_pixel(mask, x, y) ^ invert) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, 0);
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

#ifdef IMLIB_ENABLE_LENS_CORR
// A simple algorithm for correcting lens distortion.
// See http://www.tannerhelland.com/4743/simple-algorithm-correcting-lens-distortion/
void imlib_lens_corr(image_t *img, float strength, float zoom, float x_corr, float y_corr) {
    int w = img->w;
    int h = img->h;
    int halfWidth = w / 2;
    int halfHeight = h / 2;
    float maximum_diameter = fast_sqrtf((w * w) + (h * h));
    float lens_corr_diameter = strength / maximum_diameter;
    zoom = 1 / zoom;

    // Convert percentage offset to pixels from center of image
    int x_off = w * x_corr;
    int y_off = h * y_corr;

    // Create a tmp copy of the image to pull pixels from.
    size_t size = image_size(img);
    void *data = fb_alloc(size, 0);
    memcpy(data, img->data, size);
    memset(img->data, 0, size);

    int maximum_radius = fast_ceilf(maximum_diameter / 2) + 1; // +1 inclusive of final value
    float *precalculated_table = fb_alloc(maximum_radius * sizeof(float), 0);

    for (int i = 0; i < maximum_radius; i++) {
        float r = lens_corr_diameter * i;
        precalculated_table[i] = (fast_atanf(r) / r) * zoom;
    }

    int down_adj = halfHeight + y_off;
    int up_adj = h - 1 - halfHeight + y_off;
    int right_adj = halfWidth + x_off;
    int left_adj = w - 1 - halfWidth + x_off;

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *tmp = (uint32_t *) data;

            for (int y = 0; y < halfHeight; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *row_ptr2 = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, h - 1 - y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;

                for (int x = 0; x < halfWidth; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float precalculated = precalculated_table[(int) fast_sqrtf(newX2 + newY2)];
                    int sourceY = fast_roundf(precalculated * newY); // rounding is necessary
                    int sourceX = fast_roundf(precalculated * newX); // rounding is necessary
                    int sourceY_down = down_adj + sourceY;
                    int sourceY_up = up_adj - sourceY;
                    int sourceX_right = right_adj + sourceX;
                    int sourceX_left = left_adj - sourceX;

                    // plot the 4 symmetrical pixels
                    // top 2 pixels
                    if (sourceY_down >= 0 && sourceY_down < h) {
                        uint32_t *ptr = tmp + (((w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sourceY_down);

                        if (sourceX_right >= 0 && sourceX_right < w) {
                            uint8_t pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX_right);
                            IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, pixel);
                        }

                        if (sourceX_left >= 0 && sourceX_left < w) {
                            uint8_t pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX_left);
                            IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, w - 1 - x, pixel);
                        }
                    }

                    // bottom 2 pixels
                    if (sourceY_up >= 0 && sourceY_up < h) {
                        uint32_t *ptr = tmp + (((w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sourceY_up);

                        if (sourceX_right >= 0 && sourceX_right < w) {
                            uint8_t pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX_right);
                            IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr2, x, pixel);
                        }

                        if (sourceX_left >= 0 && sourceX_left < w) {
                            uint8_t pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX_left);
                            IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr2, w - 1 - x, pixel);
                        }
                    }
                }
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *tmp = (uint8_t *) data;

            for (int y = 0; y < halfHeight; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *row_ptr2 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, h - 1 - y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;

                for (int x = 0; x < halfWidth; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float precalculated = precalculated_table[(int) fast_sqrtf(newX2 + newY2)];
                    int sourceY = fast_roundf(precalculated * newY); // rounding is necessary
                    int sourceX = fast_roundf(precalculated * newX); // rounding is necessary
                    int sourceY_down = down_adj + sourceY;
                    int sourceY_up = up_adj - sourceY;
                    int sourceX_right = right_adj + sourceX;
                    int sourceX_left = left_adj - sourceX;

                    // plot the 4 symmetrical pixels
                    // top 2 pixels
                    if (sourceY_down >= 0 && sourceY_down < h) {
                        uint8_t *ptr = tmp + (w * sourceY_down);

                        if (sourceX_right >= 0 && sourceX_right < w) {
                            row_ptr[x] = ptr[sourceX_right];
                        }

                        if (sourceX_left >= 0 && sourceX_left < w) {
                            row_ptr[w - 1 - x] = ptr[sourceX_left];
                        }
                    }

                    // bottom 2 pixels
                    if (sourceY_up >= 0 && sourceY_up < h) {
                        uint8_t *ptr = tmp + (w * sourceY_up);

                        if (sourceX_right >= 0 && sourceX_right < w) {
                            row_ptr2[x] = ptr[sourceX_right];
                        }

                        if (sourceX_left >= 0 && sourceX_left < w) {
                            row_ptr2[w - 1 - x] = ptr[sourceX_left];
                        }
                    }
                }
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *tmp = (uint16_t *) data;

            for (int y = 0; y < halfHeight; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *row_ptr2 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, h - 1 - y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;

                for (int x = 0; x < halfWidth; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float precalculated = precalculated_table[(int) fast_sqrtf(newX2 + newY2)];
                    int sourceY = fast_roundf(precalculated * newY); // rounding is necessary
                    int sourceX = fast_roundf(precalculated * newX); // rounding is necessary
                    int sourceY_down = down_adj + sourceY;
                    int sourceY_up = up_adj - sourceY;
                    int sourceX_right = right_adj + sourceX;
                    int sourceX_left = left_adj - sourceX;

                    // plot the 4 symmetrical pixels
                    // top 2 pixels
                    if (sourceY_down >= 0 && sourceY_down < h) {
                        uint16_t *ptr = tmp + (w * sourceY_down);

                        if (sourceX_right >= 0 && sourceX_right < w) {
                            row_ptr[x] = ptr[sourceX_right];
                        }

                        if (sourceX_left >= 0 && sourceX_left < w) {
                            row_ptr[w - 1 - x] = ptr[sourceX_left];
                        }
                    }

                    // bottom 2 pixels
                    if (sourceY_up >= 0 && sourceY_up < h) {
                        uint16_t *ptr = tmp + (w * sourceY_up);

                        if (sourceX_right >= 0 && sourceX_right < w) {
                            row_ptr2[x] = ptr[sourceX_right];
                        }

                        if (sourceX_left >= 0 && sourceX_left < w) {
                            row_ptr2[w - 1 - x] = ptr[sourceX_left];
                        }
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    fb_free(); // precalculated_table
    fb_free(); // data
}
#endif //IMLIB_ENABLE_LENS_CORR

////////////////////////////////////////////////////////////////////////////////

int imlib_image_mean(image_t *src, int *r_mean, int *g_mean, int *b_mean) {
    int r_s = 0;
    int g_s = 0;
    int b_s = 0;
    int n = src->w * src->h;

    switch (src->pixfmt) {
        case PIXFORMAT_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            for (int i = 0; i < n; i++) {
                r_s += src->pixels[i];
            }
            *r_mean = r_s / n;
            *g_mean = r_s / n;
            *b_mean = r_s / n;
            break;
        }
        case PIXFORMAT_RGB565: {
            for (int i = 0; i < n; i++) {
                uint16_t p = ((uint16_t *) src->pixels)[i];
                r_s += COLOR_RGB565_TO_R8(p);
                g_s += COLOR_RGB565_TO_G8(p);
                b_s += COLOR_RGB565_TO_B8(p);
            }
            *r_mean = r_s / n;
            *g_mean = g_s / n;
            *b_mean = b_s / n;
            break;
        }
        default: {
            break;
        }
    }

    return 0;
}

// One pass standard deviation.
int imlib_image_std(image_t *src) {
    int w = src->w;
    int h = src->h;
    int n = w * h;
    uint8_t *data = src->pixels;

    uint32_t s = 0, sq = 0;
    for (int i = 0; i < n; i += 2) {
        s += data[i + 0] + data[i + 1];
        uint32_t tmp = __PKHBT(data[i + 0], data[i + 1], 16);
        sq = __SMLAD(tmp, tmp, sq);
    }

    if (n % 2) {
        s += data[n - 1];
        sq += data[n - 1] * data[n - 1];
    }

    /* mean */
    int m = s / n;

    /* variance */
    uint32_t v = sq / n - (m * m);

    /* std */
    return fast_sqrtf(v);
}

void imlib_sepconv3(image_t *img, const int8_t *krn, const float m, const int b) {
    int ksize = 3;
    // TODO: Support RGB
    int *buffer = fb_alloc(img->w * sizeof(*buffer) * 2, 0);

    // NOTE: This doesn't deal with borders right now. Adding if
    // statements in the inner loop will slow it down significantly.
    for (int y = 0; y < img->h - ksize; y++) {
        for (int x = 0; x < img->w; x++) {
            int acc = 0;
            //if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j))
            acc = __SMLAD(krn[0], IM_GET_GS_PIXEL(img, x, y + 0), acc);
            acc = __SMLAD(krn[1], IM_GET_GS_PIXEL(img, x, y + 1), acc);
            acc = __SMLAD(krn[2], IM_GET_GS_PIXEL(img, x, y + 2), acc);
            buffer[((y % 2) * img->w) + x] = acc;
        }
        if (y > 0) {
            // flush buffer
            for (int x = 0; x < img->w - ksize; x++) {
                int acc = 0;
                acc = __SMLAD(krn[0], buffer[((y - 1) % 2) * img->w + x + 0], acc);
                acc = __SMLAD(krn[1], buffer[((y - 1) % 2) * img->w + x + 1], acc);
                acc = __SMLAD(krn[2], buffer[((y - 1) % 2) * img->w + x + 2], acc);
                acc = (acc * m) + b; // scale, offset, and clamp
                acc = __USAT(acc, 8);
                IM_SET_GS_PIXEL(img, (x + 1), (y), acc);
            }
        }
    }
    fb_free();
}
