/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image library.
 */
#include <stdlib.h>
#include <mp.h>
#include "font.h"
#include "array.h"
#include "ff_wrapper.h"
#include "imlib.h"
#include "common.h"
#include "omv_boardconfig.h"

/////////////////
// Point Stuff //
/////////////////

void point_init(point_t *ptr, int x, int y)
{
    ptr->x = x;
    ptr->y = y;
}

void point_copy(point_t *dst, point_t *src)
{
    memcpy(dst, src, sizeof(point_t));
}

bool point_equal_fast(point_t *ptr0, point_t *ptr1)
{
    return !memcmp(ptr0, ptr1, sizeof(point_t));
}

int point_quadrance(point_t *ptr0, point_t *ptr1)
{
    int delta_x = ptr0->x - ptr1->x;
    int delta_y = ptr0->y - ptr1->y;
    return (delta_x * delta_x) + (delta_y * delta_y);
}

void point_rotate(int x, int y, float r, int center_x, int center_y, int16_t *new_x, int16_t *new_y)
{
    x -= center_x;
    y -= center_y;
    *new_x = (x * cosf(r)) - (y * sinf(r)) + center_x;
    *new_y = (x * sinf(r)) + (y * cosf(r)) + center_y;
}

void point_min_area_rectangle(point_t *corners, point_t *new_corners, int corners_len) // Corners need to be sorted!
{
    int i_min = 0;
    int i_min_area = INT_MAX;
    int i_x0 = 0, i_y0 = 0;
    int i_x1 = 0, i_y1 = 0;
    int i_x2 = 0, i_y2 = 0;
    int i_x3 = 0, i_y3 = 0;
    float i_r = 0;

    // This algorithm aligns the 4 edges produced by the 4 corners to the x axis and then computes the
    // min area rect for each alignment. The smallest rect is choosen and then re-rotated and returned.
    for (int i = 0; i < corners_len; i++) {
        int16_t x0 = corners[i].x, y0 = corners[i].y;
        int x_diff = corners[(i+1)%corners_len].x - corners[i].x;
        int y_diff = corners[(i+1)%corners_len].y - corners[i].y;
        float r = -fast_atan2f(y_diff, x_diff);

        int16_t x1[corners_len-1];
        int16_t y1[corners_len-1];
        for (int j = 0, jj = corners_len - 1; j < jj; j++) {
            point_rotate(corners[(i+j+1)%corners_len].x, corners[(i+j+1)%corners_len].y, r, x0, y0, x1 + j, y1 + j);
        }

        int minx = x0;
        int maxx = x0;
        int miny = y0;
        int maxy = y0;
        for (int j = 0, jj = corners_len - 1; j < jj; j++) {
            minx = MIN(minx, x1[j]);
            maxx = MAX(maxx, x1[j]);
            miny = MIN(miny, y1[j]);
            maxy = MAX(maxy, y1[j]);
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
bool lb_clip_line(line_t *l, int x, int y, int w, int h) // line is drawn if this returns true
{
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

            if (p[i] < 0) { // outside to inside
                if (u > umax) return false;
                if (u > umin) umin = u;
            }

            if (p[i] > 0) { // inside to outside
                if (u < umin) return false;
                if (u < umax) umax = u;
            }

        } else if (q[i] < 0) {
            return false;
        }
    }

    if (umax < umin) return false;

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

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h)
{
    ptr->x = x;
    ptr->y = y;
    ptr->w = w;
    ptr->h = h;
}

void rectangle_copy(rectangle_t *dst, rectangle_t *src)
{
    memcpy(dst, src, sizeof(rectangle_t));
}

bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1)
{
    return !memcmp(ptr0, ptr1, sizeof(rectangle_t));
}

bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1)
{
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

void rectangle_intersected(rectangle_t *dst, rectangle_t *src)
{
    int leftX = IM_MAX(dst->x, src->x);
    int topY = IM_MAX(dst->y, src->y);
    int rightX = IM_MIN(dst->x + dst->w, src->x + src->w);
    int bottomY = IM_MIN(dst->y + dst->h, src->y + src->h);
    dst->x = leftX;
    dst->y = topY;
    dst->w = rightX - leftX;
    dst->h = bottomY - topY;
}

void rectangle_united(rectangle_t *dst, rectangle_t *src)
{
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

void image_init(image_t *ptr, int w, int h, int bpp, void *data)
{
    ptr->w = w;
    ptr->h = h;
    ptr->bpp = bpp;
    ptr->data = data;
}

void image_copy(image_t *dst, image_t *src)
{
    memcpy(dst, src, sizeof(image_t));
}

size_t image_size(image_t *ptr)
{
    switch (ptr->bpp) {
        case IMAGE_BPP_BINARY: {
            return IMAGE_BINARY_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_GRAYSCALE: {
            return IMAGE_GRAYSCALE_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_RGB565: {
            return IMAGE_RGB565_LINE_LEN_BYTES(ptr) * ptr->h;
        }
        case IMAGE_BPP_BAYER: {
            return ptr->w * ptr->h;
        }
        default: { // JPEG
            return ptr->bpp;
        }
    }
}

bool image_get_mask_pixel(image_t *ptr, int x, int y)
{
    if ((0 <= x) && (x < ptr->w) && (0 <= y) && (y < ptr->h)) {
        switch (ptr->bpp) {
            case IMAGE_BPP_BINARY: {
                return IMAGE_GET_BINARY_PIXEL(ptr, x, y);
            }
            case IMAGE_BPP_GRAYSCALE: {
                return COLOR_GRAYSCALE_TO_BINARY(IMAGE_GET_GRAYSCALE_PIXEL(ptr, x, y));
            }
            case IMAGE_BPP_RGB565: {
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

const int8_t kernel_gauss_3[3*3] = {
     1, 2, 1,
     2, 4, 2,
     1, 2, 1,
};

const int8_t kernel_gauss_5[5*5] = {
    1,  4,  6,  4, 1,
    4, 16, 24, 16, 4,
    6, 24, 36, 24, 6,
    4, 16, 24, 16, 4,
    1,  4,  6,  4, 1
};

const int kernel_laplacian_3[3*3] = {
     -1, -1, -1,
     -1,  8, -1,
     -1, -1, -1
};

const int kernel_high_pass_3[3*3] = {
    -1, -1, -1,
    -1, +8, -1,
    -1, -1, -1
};

int8_t imlib_rgb565_to_l(uint16_t pixel)
{
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);

    y = (y>0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);

    return fast_floorf(116 * y) - 16;
}

int8_t imlib_rgb565_to_a(uint16_t pixel)
{
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float x = ((r_lin * 0.4124f) + (g_lin * 0.3576f) + (b_lin * 0.1805f)) * (1.0f / 095.047f);
    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);

    x = (x>0.008856f) ? fast_cbrtf(x) : ((x * 7.787037f) + 0.137931f);
    y = (y>0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);

    return fast_floorf(500 * (x-y));
}

int8_t imlib_rgb565_to_b(uint16_t pixel)
{
    float r_lin = xyz_table[COLOR_RGB565_TO_R8(pixel)];
    float g_lin = xyz_table[COLOR_RGB565_TO_G8(pixel)];
    float b_lin = xyz_table[COLOR_RGB565_TO_B8(pixel)];

    float y = ((r_lin * 0.2126f) + (g_lin * 0.7152f) + (b_lin * 0.0722f)) * (1.0f / 100.000f);
    float z = ((r_lin * 0.0193f) + (g_lin * 0.1192f) + (b_lin * 0.9505f)) * (1.0f / 108.883f);

    y = (y>0.008856f) ? fast_cbrtf(y) : ((y * 7.787037f) + 0.137931f);
    z = (z>0.008856f) ? fast_cbrtf(z) : ((z * 7.787037f) + 0.137931f);

    return fast_floorf(200 * (y-z));
}

int8_t imlib_rgb565_to_y(uint16_t pixel)
{
    int r = COLOR_RGB565_TO_R8(pixel);
    int g = COLOR_RGB565_TO_G8(pixel);
    int b = COLOR_RGB565_TO_B8(pixel);

    return (int8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15) - 128; // .299*r + .587*g + .114*b
//    return fast_floorf((r * +0.299000f) + (g * +0.587000f) + (b * +0.114000f)) - 128;
}

int8_t imlib_rgb565_to_u(uint16_t pixel)
{
    int r = COLOR_RGB565_TO_R8(pixel);
    int g = COLOR_RGB565_TO_G8(pixel);
    int b = COLOR_RGB565_TO_B8(pixel);

    return (int8_t)(((b << 14) - (r * 5529) - (g * 10855)) >> 15); // -0.168736*r + -0.331264*g + 0.5*b
//    return fast_floorf((r * -0.168736f) + (g * -0.331264f) + (b * +0.500000f));
}

int8_t imlib_rgb565_to_v(uint16_t pixel)
{
    int r = COLOR_RGB565_TO_R8(pixel);
    int g = COLOR_RGB565_TO_G8(pixel);
    int b = COLOR_RGB565_TO_B8(pixel);

    return (int8_t)(((r << 14) - (g * 13682) - (b * 2664)) >> 15); // 0.5*r + -0.418688*g + -0.081312*b
//    return fast_floorf((r * +0.500000f) + (g * -0.418688f) + (b * -0.081312f));
}

// https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions
// https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation
uint16_t imlib_lab_to_rgb(uint8_t l, int8_t a, int8_t b)
{
    float x = ((l + 16) * 0.008621f) + (a * 0.002f);
    float y = ((l + 16) * 0.008621f);
    float z = ((l + 16) * 0.008621f) - (b * 0.005f);

    x = ((x > 0.206897f) ? (x*x*x) : ((0.128419f * x) - 0.017713f)) * 095.047f;
    y = ((y > 0.206897f) ? (y*y*y) : ((0.128419f * y) - 0.017713f)) * 100.000f;
    z = ((z > 0.206897f) ? (z*z*z) : ((0.128419f * z) - 0.017713f)) * 108.883f;

    float r_lin = ((x * +3.2406f) + (y * -1.5372f) + (z * -0.4986f)) / 100.0f;
    float g_lin = ((x * -0.9689f) + (y * +1.8758f) + (z * +0.0415f)) / 100.0f;
    float b_lin = ((x * +0.0557f) + (y * -0.2040f) + (z * +1.0570f)) / 100.0f;

    r_lin = (r_lin>0.0031308f) ? ((1.055f*powf(r_lin, 0.416666f))-0.055f) : (r_lin*12.92f);
    g_lin = (g_lin>0.0031308f) ? ((1.055f*powf(g_lin, 0.416666f))-0.055f) : (g_lin*12.92f);
    b_lin = (b_lin>0.0031308f) ? ((1.055f*powf(b_lin, 0.416666f))-0.055f) : (b_lin*12.92f);

    uint32_t red   = IM_MAX(IM_MIN(fast_floorf(r_lin * COLOR_R8_MAX), COLOR_R8_MAX), COLOR_R8_MIN);
    uint32_t green = IM_MAX(IM_MIN(fast_floorf(g_lin * COLOR_G8_MAX), COLOR_G8_MAX), COLOR_G8_MIN);
    uint32_t blue  = IM_MAX(IM_MIN(fast_floorf(b_lin * COLOR_B8_MAX), COLOR_B8_MAX), COLOR_B8_MIN);

    return COLOR_R8_G8_B8_TO_RGB565(red, green, blue);
}

// https://en.wikipedia.org/wiki/YCbCr -> JPEG Conversion
uint16_t imlib_yuv_to_rgb(uint8_t y, int8_t u, int8_t v)
{
    uint32_t r = IM_MAX(IM_MIN(y + ((91881 * v) >> 16), COLOR_R8_MAX), COLOR_R8_MIN);
    uint32_t g = IM_MAX(IM_MIN(y - (((22554 * u) + (46802 * v)) >> 16), COLOR_G8_MAX), COLOR_G8_MIN);
    uint32_t b = IM_MAX(IM_MIN(y + ((116130 * u) >> 16), COLOR_B8_MAX), COLOR_B8_MIN);

    return COLOR_R8_G8_B8_TO_RGB565(r, g, b);
}

void imlib_bayer_to_rgb565(image_t *img, int w, int h, int xoffs, int yoffs, uint16_t *rgbbuf)
{
    int r, g, b;
    for (int y=yoffs; y<yoffs+h; y++) {
        for (int x=xoffs; x<xoffs+w; x++) {
            if ((y % 2) == 0) { // Even row
                if ((x % 2) == 0) { // Even col
                    r = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x-1, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x+1, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x-1, y+1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x+1, y+1)) >> 2;

                    g = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y+1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x-1, y)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x+1, y)) >> 2;

                    b = IM_GET_RAW_PIXEL(img,  x, y);
                } else { // Odd col
                    r = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y+1)) >> 1;

                    b = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x-1, y)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x+1, y)) >> 1;

                    g =  IM_GET_RAW_PIXEL(img, x, y);
                }
            } else { // Odd row
                if ((x % 2) == 0) { // Even Col
                    r = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x-1, y)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x+1, y)) >> 1;

                    g =  IM_GET_RAW_PIXEL(img, x, y);

                    b = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y+1)) >> 1;
                } else { // Odd col
                    r = IM_GET_RAW_PIXEL(img,  x, y);

                    g = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_Y(img, x, y+1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x-1, y)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_X(img, x+1, y)) >> 2;

                    b = (IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x-1, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x+1, y-1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x-1, y+1)  +
                         IM_GET_RAW_PIXEL_CHECK_BOUNDS_XY(img, x+1, y+1)) >> 2;
                }

            }
            r = IM_R825(r);
            g = IM_G826(g);
            b = IM_B825(b);
            *rgbbuf++ = IM_RGB565(r, g, b);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////

static save_image_format_t imblib_parse_extension(image_t *img, const char *path)
{
    size_t l = strlen(path);
    const char *p = path + l;
    if (l >= 5) {
               if (((p[-1] == 'g') || (p[-1] == 'G'))
               &&  ((p[-2] == 'e') || (p[-2] == 'E'))
               &&  ((p[-3] == 'p') || (p[-3] == 'P'))
               &&  ((p[-4] == 'j') || (p[-4] == 'J'))
               &&  ((p[-5] == '.') || (p[-5] == '.'))) {
                    // Will convert to JPG if not.
                    return FORMAT_JPG;
        }
    }
    if (l >= 4) {
               if (((p[-1] == 'g') || (p[-1] == 'G'))
               &&  ((p[-2] == 'p') || (p[-2] == 'P'))
               &&  ((p[-3] == 'j') || (p[-3] == 'J'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    // Will convert to JPG if not.
                    return FORMAT_JPG;
        } else if (((p[-1] == 'p') || (p[-1] == 'P'))
               &&  ((p[-2] == 'm') || (p[-2] == 'M'))
               &&  ((p[-3] == 'b') || (p[-3] == 'B'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (IM_IS_JPEG(img) || IM_IS_BAYER(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not BMP!"));
                    }
                    return FORMAT_BMP;
        } else if (((p[-1] == 'm') || (p[-1] == 'M'))
               &&  ((p[-2] == 'p') || (p[-2] == 'P'))
               &&  ((p[-3] == 'p') || (p[-3] == 'P'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (!IM_IS_RGB565(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not PPM!"));
                    }
                    return FORMAT_PNM;
        } else if (((p[-1] == 'm') || (p[-1] == 'M'))
               &&  ((p[-2] == 'g') || (p[-2] == 'G'))
               &&  ((p[-3] == 'p') || (p[-3] == 'P'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (!IM_IS_GS(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not PGM!"));
                    }
                    return FORMAT_PNM;
        } else if (((p[-1] == 'w') || (p[-1] == 'W'))
               &&  ((p[-2] == 'a') || (p[-2] == 'A'))
               &&  ((p[-3] == 'r') || (p[-3] == 'R'))
               &&  ((p[-4] == '.') || (p[-4] == '.'))) {
                    if (!IM_IS_BAYER(img)) {
                        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Image is not BAYER!"));
                    }
                    return FORMAT_RAW;
        }

    }
    return FORMAT_DONT_CARE;
}

bool imlib_read_geometry(FIL *fp, image_t *img, const char *path, img_read_settings_t *rs)
{
    file_read_open(fp, path);
    char magic[2];
    read_data(fp, &magic, 2);
    file_close(fp);

    bool vflipped = false;
    if ((magic[0]=='P')
    && ((magic[1]=='2') || (magic[1]=='3')
    ||  (magic[1]=='5') || (magic[1]=='6'))) { // PPM
        rs->format = FORMAT_PNM;
        file_read_open(fp, path);
        file_buffer_on(fp); // REMEMBER TO TURN THIS OFF LATER!
        ppm_read_geometry(fp, img, path, &rs->ppm_rs);
    } else if ((magic[0]=='B') && (magic[1]=='M')) { // BMP
        rs->format = FORMAT_BMP;
        file_read_open(fp, path);
        file_buffer_on(fp); // REMEMBER TO TURN THIS OFF LATER!
        vflipped = bmp_read_geometry(fp, img, path, &rs->bmp_rs);
    } else {
        ff_unsupported_format(NULL);
    }
    imblib_parse_extension(img, path); // Enforce extension!
    return vflipped;
}

static void imlib_read_pixels(FIL *fp, image_t *img, int n_lines, img_read_settings_t *rs)
{
    switch (rs->format) {
        case FORMAT_BMP:
            bmp_read_pixels(fp, img, n_lines, &rs->bmp_rs);
            break;
        case FORMAT_PNM:
            ppm_read_pixels(fp, img, n_lines, &rs->ppm_rs);
            break;
        default: // won't happen
            break;
    }
}

void imlib_image_operation(image_t *img, const char *path, image_t *other, int scalar, line_op_t op, void *data)
{
    if (path) {
        uint32_t size = fb_avail() / 2;
        void *alloc = fb_alloc(size, FB_ALLOC_NO_HINT); // We have to do this before the read.
        // This code reads a window of an image in at a time and then executes
        // the line operation on each line in that window before moving to the
        // next window. The vflipped part is here because BMP files can be saved
        // vertically flipped resulting in us reading the image backwards.
        FIL fp;
        image_t temp;
        img_read_settings_t rs;
        bool vflipped = imlib_read_geometry(&fp, &temp, path, &rs);
        if (!IM_EQUAL(img, &temp)) {
            ff_not_equal(&fp);
        }
        // When processing vertically flipped images the read function will fill
        // the window up from the bottom. The read function assumes that the
        // window is equal to an image in size. However, since this is not the
        // case we shrink the window size to how many lines we're buffering.
        temp.pixels = alloc;
        // Set the max buffer height to image height.
        temp.h = IM_MIN(img->h, (size / (temp.w * temp.bpp)));
        // This should never happen unless someone forgot to free.
        if ((!temp.pixels) || (!temp.h)) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_MemoryError,
                                               "Not enough memory available!"));
        }
        for (int i=0; i<img->h; i+=temp.h) { // goes past end
            int lines = IM_MIN(temp.h, img->h-i);
            imlib_read_pixels(&fp, &temp, lines, &rs);
            for (int j=0; j<lines; j++) {
                if (!vflipped) {
                    op(img, i+j, temp.pixels+(temp.w*temp.bpp*j), data, false);
                } else {
                    op(img, (img->h-i-lines)+j, temp.pixels+(temp.w*temp.bpp*j), data, true);
                }
            }
        }
        file_buffer_off(&fp);
        file_close(&fp);
        fb_free();
    } else if (other) {
        if (!IM_EQUAL(img, other)) {
            ff_not_equal(NULL);
        }
        switch (img->bpp) {
            case IMAGE_BPP_BINARY: {
                for (int i=0, ii=img->h; i<ii; i++) {
                    op(img, i, IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(other, i), data, false);
                }
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                for (int i=0, ii=img->h; i<ii; i++) {
                    op(img, i, IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(other, i), data, false);
                }
                break;
            }
            case IMAGE_BPP_RGB565: {
                for (int i=0, ii=img->h; i<ii; i++) {
                    op(img, i, IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(other, i), data, false);
                }
                break;
            }
            default: {
                break;
            }
        }
    } else {
        switch(img->bpp) {
            case IMAGE_BPP_BINARY: {
                uint32_t *row_ptr = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img), FB_ALLOC_NO_HINT);

                for (int i=0, ii=img->w; i<ii; i++) {
                    IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, i, scalar);
                }

                for (int i=0, ii=img->h; i<ii; i++) {
                    op(img, i, row_ptr, data, false);
                }

                fb_free();
                break;
            }
            case IMAGE_BPP_GRAYSCALE: {
                uint8_t *row_ptr = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img), FB_ALLOC_NO_HINT);

                for (int i=0, ii=img->w; i<ii; i++) {
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, i, scalar);
                }

                for (int i=0, ii=img->h; i<ii; i++) {
                    op(img, i, row_ptr, data, false);
                }

                fb_free();
                break;
            }
            case IMAGE_BPP_RGB565: {
                uint16_t *row_ptr = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img), FB_ALLOC_NO_HINT);

                for (int i=0, ii=img->w; i<ii; i++) {
                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, i, scalar);
                }

                for (int i=0, ii=img->h; i<ii; i++) {
                    op(img, i, row_ptr, data, false);
                }

                fb_free();
                break;
            }
            default: {
                break;
            }
        }
    }
}

void imlib_load_image(image_t *img, const char *path)
{
    FIL fp;
    file_read_open(&fp, path);
    char magic[2];
    read_data(&fp, &magic, 2);
    file_close(&fp);

    if ((magic[0]=='P')
    && ((magic[1]=='2') || (magic[1]=='3')
    ||  (magic[1]=='5') || (magic[1]=='6'))) { // PPM
        ppm_read(img, path);
    } else if ((magic[0]=='B') && (magic[1]=='M')) { // BMP
        bmp_read(img, path);
    } else if ((magic[0]==0xFF) && (magic[1]==0xD8)) { // JPEG
        jpeg_read(img, path);
    } else {
        ff_unsupported_format(NULL);
    }
    imblib_parse_extension(img, path); // Enforce extension!
}

void imlib_save_image(image_t *img, const char *path, rectangle_t *roi, int quality)
{
    switch (imblib_parse_extension(img, path)) {
        case FORMAT_BMP:
            bmp_write_subimg(img, path, roi);
            break;
        case FORMAT_PNM:
            ppm_write_subimg(img, path, roi);
            break;
        case FORMAT_RAW: {
            FIL fp;
            file_write_open(&fp, path);
            write_data(&fp, img->pixels, img->w * img->h);
            file_close(&fp);
            break;
        }
        case FORMAT_JPG:
            jpeg_write(img, path, quality);
            break;
        case FORMAT_DONT_CARE:
            // Path doesn't have an extension.
            if (IM_IS_JPEG(img)) {
                char *new_path = strcat(strcpy(fb_alloc(strlen(path)+5, FB_ALLOC_NO_HINT), path), ".jpg");
                jpeg_write(img, new_path, quality);
                fb_free();
            } else if (IM_IS_BAYER(img)) {
                FIL fp;
                char *new_path = strcat(strcpy(fb_alloc(strlen(path)+5, FB_ALLOC_NO_HINT), path), ".raw");
                file_write_open(&fp, new_path);
                write_data(&fp, img->pixels, img->w * img->h);
                file_close(&fp);
                fb_free();
            } else { // RGB or GS, save as BMP.
                char *new_path = strcat(strcpy(fb_alloc(strlen(path)+5, FB_ALLOC_NO_HINT), path), ".bmp");
                bmp_write_subimg(img, new_path, roi);
                fb_free();
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////

void imlib_zero(image_t *img, image_t *mask, bool invert)
{
    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
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
        case IMAGE_BPP_GRAYSCALE: {
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
        case IMAGE_BPP_RGB565: {
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

// A simple algorithm for correcting lens distortion.
// See http://www.tannerhelland.com/4743/simple-algorithm-correcting-lens-distortion/
void imlib_lens_corr(image_t *img, float strength, float zoom)
{
    zoom = 1 / zoom;
    int halfWidth = img->w / 2;
    int halfHeight = img->h / 2;
    float lens_corr_radius = strength / fast_sqrtf((img->w * img->w) + (img->h * img->h));

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            // Create a temp copy of the image to pull pixels from.
            uint32_t *tmp = fb_alloc(((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * img->h, FB_ALLOC_NO_HINT);
            memcpy(tmp, img->data, ((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * img->h);
            memset(img->data, 0, ((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * img->h);

            for (int y = 0, yy = halfHeight; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *row_ptr2 = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, img->h-1-y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;
                float zoomedY = newY * zoom;

                for (int x = 0, xx = halfWidth; x < xx; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float zoomedX = newX * zoom;

                    float r = lens_corr_radius * fast_sqrtf(newX2 + newY2);
                    float theta = (r < 0.0000001f) ? 1.0f : (fast_atanf(r) / r);
                    int sourceX = halfWidth + fast_floorf(theta * zoomedX);
                    int sourceY = halfHeight + fast_floorf(theta * zoomedY);

                    if ((0 <= sourceX) && (0 <= sourceY)) {
                        uint32_t *ptr;
                        int pixel;
                        ptr = tmp + (((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * sourceY);
                        pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX);
                        IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, pixel);
                        pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, img->w-1-sourceX);
                        IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, img->w-1-x, pixel);
                        ptr = tmp + (((img->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * (img->h-1-sourceY));
                        pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, sourceX);
                        IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr2, x, pixel);
                        pixel = IMAGE_GET_BINARY_PIXEL_FAST(ptr, img->w-1-sourceX);
                        IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr2, img->w-1-x, pixel);
                    }
                }
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            // Create a temp copy of the image to pull pixels from.
            uint8_t *tmp = fb_alloc(img->w * img->h * sizeof(uint8_t), FB_ALLOC_NO_HINT);
            memcpy(tmp, img->data, img->w * img->h * sizeof(uint8_t));
            memset(img->data, 0, img->w * img->h * sizeof(uint8_t));

            for (int y = 0, yy = halfHeight; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *row_ptr2 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, img->h-1-y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;
                float zoomedY = newY * zoom;

                for (int x = 0, xx = halfWidth; x < xx; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float zoomedX = newX * zoom;

                    float r = lens_corr_radius * fast_sqrtf(newX2 + newY2);
                    float theta = (r < 0.0000001f) ? 1.0f : (fast_atanf(r) / r);
                    int sourceX = halfWidth + fast_floorf(theta * zoomedX);
                    int sourceY = halfHeight + fast_floorf(theta * zoomedY);

                    if ((0 <= sourceX) && (0 <= sourceY)) {
                        uint8_t *ptr, pixel;
                        ptr = tmp + (img->w * sourceY); // top 2 pixels
                        pixel = ptr[sourceX];
                        row_ptr[x] = pixel;
                        pixel = ptr[img->w - 1 - sourceX];
                        row_ptr[img->w - 1 - x] = pixel;
                        ptr = tmp + (img->w * (img->h - 1 - sourceY)); // bottom 2 pixels
                        pixel = ptr[sourceX];
                        row_ptr2[x] = pixel;
                        pixel = ptr[img->w - 1 - sourceX];
                        row_ptr2[img->w - 1 - x] = pixel;
                    }
                }
            }

            fb_free();
            break;
        }
        case IMAGE_BPP_RGB565: {
            // Create a temp copy of the image to pull pixels from.
            uint16_t *tmp = fb_alloc(img->w * img->h * sizeof(uint16_t), FB_ALLOC_NO_HINT);
            memcpy(tmp, img->data, img->w * img->h * sizeof(uint16_t));
            memset(img->data, 0, img->w * img->h * sizeof(uint16_t));

            for (int y = 0, yy = halfHeight; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *row_ptr2 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, img->h-1-y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;
                float zoomedY = newY * zoom;

                for (int x = 0, xx = halfWidth; x < xx; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float zoomedX = newX * zoom;

                    float r = lens_corr_radius * fast_sqrtf(newX2 + newY2);
                    float theta = (r < 0.0000001f) ? 1.0f : (fast_atanf(r) / r);
                    int sourceX = halfWidth + fast_floorf(theta * zoomedX);
                    int sourceY = halfHeight + fast_floorf(theta * zoomedY);

                    if ((0 <= sourceX) && (0 <= sourceY)) {
                    // plot the 4 symmetrical pixels
                        uint16_t *ptr;
                        int pixel;
                        ptr = tmp + (img->w * sourceY); // top 2 pixels
                        pixel = ptr[sourceX];
                        row_ptr[x] = pixel;
                        pixel = ptr[img->w - 1 - sourceX];
                        row_ptr[img->w - 1 - x] = pixel;
                        ptr = tmp + (img->w * (img->h - 1 - sourceY)); // bottom 2 pixels
                        pixel = ptr[sourceX];
                        row_ptr2[x] = pixel;
                        pixel = ptr[img->w - 1 - sourceX];
                        row_ptr2[img->w - 1 - x] = pixel; 
                    }
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

////////////////////////////////////////////////////////////////////////////////

int imlib_image_mean(image_t *src, int *r_mean, int *g_mean, int *b_mean)
{
    int r_s = 0;
    int g_s = 0;
    int b_s = 0;
    int n = src->w * src->h;

    switch(src->bpp) {
        case IMAGE_BPP_BINARY: {
            // Can't run this on a binary image.
            break;
        }
        case IMAGE_BPP_GRAYSCALE: {
            for (int i=0; i<n; i++) {
                r_s += src->pixels[i];
            }
            *r_mean = r_s/n;
            *g_mean = r_s/n;
            *b_mean = r_s/n;
            break;
        }
        case IMAGE_BPP_RGB565: {
            for (int i=0; i<n; i++) {
                uint16_t p = ((uint16_t*)src->pixels)[i];
                r_s += COLOR_RGB565_TO_R8(p);
                g_s += COLOR_RGB565_TO_G8(p);
                b_s += COLOR_RGB565_TO_B8(p);
            }
            *r_mean = r_s/n;
            *g_mean = g_s/n;
            *b_mean = b_s/n;
            break;
        }
        default: {
            break;
        }
    }

    return 0;
}

// One pass standard deviation.
int imlib_image_std(image_t *src)
{
    int w=src->w;
    int h=src->h;
    int n=w*h;
    uint8_t *data=src->pixels;

    uint32_t s=0, sq=0;
    for (int i=0; i<n; i+=2) {
        s += data[i+0]+data[i+1];
        uint32_t tmp = __PKHBT(data[i+0], data[i+1], 16);
        sq = __SMLAD(tmp, tmp, sq);
    }

    if (n%2) {
        s += data[n-1];
        sq += data[n-1]*data[n-1];
    }

    /* mean */
    int m = s/n;

    /* variance */
    uint32_t v = sq/n-(m*m);

    /* std */
    return fast_sqrtf(v);
}

void imlib_sepconv3(image_t *img, const int8_t *krn, const float m, const int b)
{
    int ksize = 3;
    // TODO: Support RGB
    int *buffer = fb_alloc(img->w * sizeof(*buffer) * 2, FB_ALLOC_NO_HINT);

    // NOTE: This doesn't deal with borders right now. Adding if
    // statements in the inner loop will slow it down significantly.
    for (int y=0; y<img->h-ksize; y++) {
        for (int x=0; x<img->w; x++) {
            int acc=0;
            //if (IM_X_INSIDE(img, x+k) && IM_Y_INSIDE(img, y+j))
            acc = __SMLAD(krn[0], IM_GET_GS_PIXEL(img, x, y + 0), acc);
            acc = __SMLAD(krn[1], IM_GET_GS_PIXEL(img, x, y + 1), acc);
            acc = __SMLAD(krn[2], IM_GET_GS_PIXEL(img, x, y + 2), acc);
            buffer[((y%2)*img->w) + x] = acc;
        }
        if (y > 0) {
            // flush buffer
            for (int x=0; x<img->w-ksize; x++) {
                int acc = 0;
                acc = __SMLAD(krn[0], buffer[((y-1)%2) * img->w + x + 0], acc);
                acc = __SMLAD(krn[1], buffer[((y-1)%2) * img->w + x + 1], acc);
                acc = __SMLAD(krn[2], buffer[((y-1)%2) * img->w + x + 2], acc);
                acc = (acc * m) + b; // scale, offset, and clamp
                acc = IM_MAX(IM_MIN(acc, IM_MAX_GS), 0);
                IM_SET_GS_PIXEL(img, (x+1), (y), acc);
            }
        }
    }
    fb_free();
}
