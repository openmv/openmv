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
// The faster routine needs to start on an even column
// and not next to the top or bottom edge to avoid boundary checks on each pixel
    if (xoffs & 1 || y == 0 || y >= yoffs+h-1) {
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
        } // for x
    } else { // faster way
//
// Theory of operation:
// The Bayer pattern from the sensor looks like this:
// +---+---+---+---+---+---+
// | B | G | B | G | B | G |
// +---+---+---+---+---+---+
// | G |*R*|*G*| R | G | R | * = Example of current pair of pixels being processed
// +---+---+---+---+---+---+ Each iteration below will advance 2 pixels to the right
// | B | G | B | G | B | G |
// +---+---+---+---+---+---+
// | G | R | G | R | G | R |
// +---+---+---+---+---+---+
// Each of the color stimuli above is stored as 1 byte
// The slower algorithm above reads each byte around the current pixel individually to
// average the colors together to simulate the colors not present at the current pixel
// e.g. At location 0,0, only the blue value is present; red and green must be estimated from
// neighboring pixels
//
// The optimized algorithm below minimizes memory accesses by reading 2 bytes at a time
// and re-using the last pair as it progresses from left to right. Since the ARM CPU enforces a
// memory policy of generating an exception on unaligned reads, we read 16-bits at a time and
// OR them into a 32-bit variable to hold on to the pixels left and right of the current pair.
// This way we can work on 2 pixels at a time from 3 32-bit variables containing 3 lines of 4 pixels.
// The variables l0,l1,l2 hold the 4 pixels (left, current left, current_right, right)
// in lines above the current (l0), current (l1) and below (l2)
// This algorithm assumes that the starting x is 0 (it doesn't have to be).
// The starting and ending pixel are treated like absolute sensor edges, so they are less accurate
// than the middle pixels.
//
        uint32_t l0,l1,l2; // 3 lines we're working with
        uint16_t *s = (uint16_t *)&img->pixels[(y * img->w) + xoffs];
        uint32_t r, g, b, w2 = img->w/2;
        l0 = s[-w2]; l1 = s[0]; l2 = s[w2];
        s++;
// For each pixel, the R8 G8 B8 values are directly shifted into R5 G6 B5
// to save a step when creating the RGB565 output
        if (y & 1) { // odd lines (G R G R G R)
            b = ((l0 & 0xff) + (l2 & 0xff)) >> 4; // left edge pixel
            g = (l1 & 0xff) >> 2;
            r = (l1 & 0xff00) >> 11;
            *rgbbuf++ = IM_RGB565(r, g, b); // first col is treated differently
            for (int x = xoffs+1; x < xoffs+w-1; x+=2) { // 'middle' pixels can use neighbors from 3x3 grid
                l0 |= (s[-w2] << 16); // shift last pixel pairs into upper 16 bits
                l1 |= (s[0] << 16);   // of the 3 line variables
                l2 |= (s[w2] << 16);
                s++; // advance source pointer one pair of pixels
                r = (l1 & 0xff00) >> 11; // (1, 0) red pixel at current-left
                g = (((l1 >> 16) & 0xff) + (l1 & 0xff) + ((l0 >> 8) & 0xff) + ((l2 >> 8) & 0xff)) >> 4;
                b = ((l0 & 0xff) + (l2 & 0xff) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 5;
                *rgbbuf++ = IM_RGB565(r, g, b);
                g = (l1 & 0xff0000) >> 18; // (0,0) green pixel at current-right
                b = (((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 4;
                r = (((l1 >> 8) & 0xff) + (l1 >> 24)) >> 4;
                *rgbbuf++ = IM_RGB565(r, g, b);
                // prepare for the next set of source pixels
                l0 >>= 16; l1 >>= 16; l2 >>= 16; // L-CL-CR-R becomes L-CL-0-0
            } // for x
            // last col
            g = ((l0 >> 8) + (l2 >> 8)) >> 3; // re-use blue from last pixel in loop
            r = (l1 >> 11);
            *rgbbuf++ = IM_RGB565(r, g, b);
        } else { // even lines (B G B G B G)
            b = (l1 & 0xff) >> 3;
            g = ((l0 & 0xff) + (l2 & 0xff)) >> 3;
            r = ((l0 & 0xff00) + (l2 & 0xff00)) >> 12; // first pixel is different
            *rgbbuf++ = IM_RGB565(r, g, b);
            for (int x = xoffs+1; x < xoffs+w-1; x+=2) { // middle part
                l0 |= (s[-w2] << 16); // grab 3 more pairs of pixels and put in upper 16-bits 
                l1 |= (s[0] << 16);
                l2 |= (s[w2] << 16);
                s++;
                g = (l1 & 0xff00) >> 10; // (1, 0) green pixel at current-left
                b = ((l1 & 0xff) + ((l1 >> 16) & 0xff)) >> 4;
                r = ((l0 & 0xff00) + (l2 & 0xff00)) >> 12;
                *rgbbuf++ = IM_RGB565(r, g, b);
                b = (l1 & 0xff0000) >> 19; // (0,0) blue pixel at current-right
                g = (((l1 >> 8) & 0xff) + (l1 >> 24) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 4;
                r = (((l0 >> 8) & 0xff) + (l0 >> 24) + ((l2 >> 8) & 0xff) + (l2 >> 24)) >> 5;
                *rgbbuf++ = IM_RGB565(r, g, b);
                // prepare for the next set of source pixels
                l0 >>= 16; l1 >>= 16; l2 >>= 16; // L-CL-CR-R becomes L-CL-0-0
            } // for x
            // last col
            g = (l1 >> 10); // re-use blue pixel from last column of loop
            r = ((l0 >> 8) + (l2 >> 8)) >> 4;
            *rgbbuf++ = IM_RGB565(r, g, b); // last pixel
        } // even lines
    } // faster way
    } // for y
}

//
// Convert a row of Bayer pixels into grayscale output
//
void imlib_bayer_to_y(image_t *img, int x_offset, int y_offset, int width, uint8_t *Y)
{
            uint16_t *s;
            uint32_t l0, l1, l2; // current, prev and next lines of current pixel(s)
            int x, xx, r, g, b = 0;
            int pitch = img->w; // keep in local var
            int w2 = pitch/2; // pitch for a uint16_t pointer
            x = x_offset; xx = x_offset+width;
            if (y_offset < 1 || y_offset >= img->h-1) { // top or bottom lines
                uint8_t *s8 = (uint8_t *)&img->pixels[(y_offset * pitch) + x_offset];
                if (y_offset & 1) { // odd line (y == img->h-1)
                    int bLastPixel = 0;
                    if (((x_offset+width) & 1) == 0 && x_offset+width >= img->w) { // flag to not read past the bottom-right edge
                        xx--; // make the loop stop on an even pixel
                        bLastPixel = 1;
                    }
                    if (x == 0) { // special case of starting from left edge
                        g = s8[0];
                        b = s8[-pitch];
                        r = s8[1];
                        *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        x++;
                    } // x == 0
                    for (; x<xx; x++) {
                        if (x & 1) { // odd pixels
                            g = (s8[-1] + s8[1]) >>1;
                            r = s8[0];
                            b = (s8[-pitch-1] + s8[-pitch+1]) >> 1;
                        } else { // even pixels
                            g = s8[0];
                            b = s8[-pitch];
                            r = (s8[-1] + s8[1]) >> 1;
                        }
                        *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    } // for x
                    if (bLastPixel) { // 1 more pixel to process
                        r = s8[0]; // on a red pixel
                        g = (s8[-pitch] + s8[-1]) >> 1; // and re-use the previous blue
                        *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    }
                } else { // even line (y==0)
                    if (x == 0) { // left edge special case
                        b = s8[0];
                        g = s8[pitch];
                        r = s8[pitch+1];
                        *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        x++;
                    }
                    for (; x<xx; x++) {
                        if (x & 1) { // odd pixels
                            b = (s8[-1] + s8[1]) >>1;
                            g = s8[0];
                            r = s8[pitch];
                        } else { // even pixels
                            b = s8[0];
                            g = (s8[1] + s8[-1]) >> 1;
                            r = s8[pitch];
                        }
                        *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    } // for x
                } // even line
                return;
            } // edge case; need to check boundary conditions
// middle of image can be converted without checking boundary conditions on each pixel
            s = (uint16_t*)&img->pixels[(y_offset * pitch) + (x_offset & 0xfffe)];
            if (x_offset == 0) { // don't read past left edge; repeat pixels instead
                l0 = s[-w2] | (s[-w2] << 16); // prep current and left pixels
                l1 = s[0] | (s[0] << 16);
                l2 = s[w2] | (s[w2] << 16);
            } else {
                l0 = s[-w2-1] | (s[-w2] << 16); // prep current and left pixels
                l1 = s[-1] | (s[0] << 16);
                l2 = s[w2-1] | (s[w2] << 16);
            }
            x = x_offset; xx = x_offset+width;
            s++;
            if (y_offset & 1) { // odd line
                if (x_offset & 1) // starting on an odd pixel, capture it differently
                {
                    r = (l1 >> 24); // (1, 0) red pixel
                    g = ((l0 >> 24) + (l2 >> 24)) >> 1;
                    b = ((l0 & 0xff0000) + (l2 & 0xff0000)) >> 17;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    x++; // advance to next pixel
                }
                for (; x<xx-1; x+=2) {
                    g = (l1 & 0xff0000) >> 16; // (0,0) green pixel
                    b = ((l0 & 0xff0000) + (l2 & 0xff0000)) >> 17;
                    r = (((l1 >> 8) & 0xff) + (l1 >> 24)) >> 1;
                    // faster to keep all calculations in integer math with 15-bit fractions
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    l0 >>= 16; l1 >>= 16; l2 >>= 16; // L-CL-CR-R becomes L-CL-0-0
                    l0 |= (s[-w2] << 16); // grab 3 more pairs of pixels and put in upper 16-bits
                    l1 |= (s[0] << 16);
                    l2 |= (s[w2] << 16);
                    s++;
                    r = (l1 & 0xff00) >> 8; // (1, 0) red pixel
                    g = (((l1 >> 16) & 0xff) + (l1 & 0xff) + ((l0 >> 8) & 0xff) + ((l2 >> 8) & 0xff)) >> 2;
                    b = ((l0 & 0xff) + (l2 & 0xff) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 2;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                } // for x
                if (x < xx) { // one final pixel to do
                    g = (l1 >> 16) & 0xff; // (0, 0) green pixel
                    b = ((l0 & 0xff0000) + (l2 & 0xff0000)) >> 17;
                    r = (((l1 >> 8) & 0xff) + (l1 >> 24)) >> 1;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g
                }
            } else { // even line
                if (x_offset & 1) // starting on an odd pixel, capture it differently
                {
                    g = (l1 >> 8) & 0xff; // (1, 0) green pixel
                    r = (((l0 >> 8) & 0xff) + ((l2 >> 8) && 0xff)) >> 1;
                    b = (((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 1;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    x++; // advance to next pixel
                }
                for (; x<xx-1; x+=2) {
                    b = (l1 & 0xff0000) >> 16; // (0,0) blue pixel
                    g = (((l1 >> 8) & 0xff) + (l1 >> 24) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 2;
                    r = (((l0 >> 8) & 0xff) + (l0 >> 24) + ((l2 >> 8) & 0xff) + (l2 >> 24)) >> 2;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    // prepare for the next set of source pixels
                    l0 >>= 16; l1 >>= 16; l2 >>= 16; // L-CL-CR-R becomes L-CL-0-0
                    l0 |= (s[-w2] << 16); // grab 3 more pairs of pixels and put in upper 16-bits
                    l1 |= (s[0] << 16);
                    l2 |= (s[w2] << 16);
                    s++;
                    g = (l1 & 0xff00) >> 8; // (1, 0) green pixel
                    b = ((l1 & 0xff) + ((l1 >> 16) & 0xff)) >> 1;
                    r = ((l0 & 0xff00) + (l2 & 0xff00)) >> 9;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                } // for x
                if (x < xx) { // one final pixel to do
                    b = (l1 & 0xff0000) >> 16; // (0,0) blue pixel
                    g = (((l1 >> 8) & 0xff) + (l1 >> 24) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 2;
                    r = (((l0 >> 8) & 0xff) + (l0 >> 24) + ((l2 >> 8) & 0xff) + (l2 >> 24)) >> 2;
                    *Y++ = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                }
            } // even line
} /* imlib_bayer_to_y() */
//
// Convert a row of Bayer pixels into binary output
//
void imlib_bayer_to_binary(image_t *img, int x_offset, int y_offset, int width, uint8_t *binary)
{
            uint16_t *s;
            uint32_t l0, l1, l2; // current, prev and next lines of current pixel(s)
            int x, xx, r, g, b = 0;
            uint8_t pixel;
            int pitch = img->w; // keep in local var
            int w2 = pitch/2; // pitch for a uint16_t pointer
            x = x_offset; xx = x_offset+width;
            if (y_offset < 1 || y_offset >= img->h-1) { // top or bottom lines
                uint8_t *s8 = (uint8_t *)&img->pixels[(y_offset * pitch) + x_offset];
                if (y_offset & 1) { // odd line (y == img->h-1)
                    int bLastPixel = 0;
                    if (((x_offset+width) & 1) == 0 && x_offset+width >= img->w) { // flag to not read past the bottom-right edge
                        xx--; // make the loop stop on an even pixel
                        bLastPixel = 1;
                    }
                    if (x == 0) { // special case of starting from left edge
                        g = s8[0];
                        b = s8[-pitch];
                        r = s8[1];
                        pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                        x++;
                    }
                    for (; x<xx; x++) {
                        if (x & 1) { // odd pixels
                            g = (s8[-1] + s8[1]) >>1;
                            r = s8[0];
                            b = (s8[-pitch-1] + s8[-pitch+1]) >> 1;
                        } else { // even pixels
                            g = s8[0];
                            b = s8[-pitch];
                            r = (s8[-1] + s8[1]) >> 1;
                        }
                        pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    } // for x
                    if (bLastPixel) { // 1 more pixel to process
                        r = s8[0]; // on a red pixel
                        g = (s8[-pitch] + s8[-1]) >> 1; // and re-use the previous blue
                        pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    }
                } else { // even line (y==0)
                    if (x == 0) { // left edge special case
                        b = s8[0];
                        g = s8[pitch];
                        r = s8[pitch+1];
                        pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                        x++;
                    }
                    for (; x<xx; x++) {
                        if (x & 1) { // odd pixels
                            b = (s8[-1] + s8[1]) >>1;
                            g = s8[0];
                            r = s8[pitch];
                        } else { // even pixels
                            b = s8[0];
                            g = (s8[1] + s8[-1]) >> 1;
                            r = s8[pitch];
                        }
                        pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                        IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    } // for x
                } // even line
                return;
            } // edge case; need to check boundary conditions
// middle of image can be converted without checking boundary conditions on each pixel
            s = (uint16_t*)&img->pixels[(y_offset * pitch) + (x_offset & 0xfffe)];
            if (x_offset == 0) { // don't read past left edge; repeat pixels instead
                l0 = s[-w2] | (s[-w2] << 16); // prep current and left pixels
                l1 = s[0] | (s[0] << 16);
                l2 = s[w2] | (s[w2] << 16);
            } else {
                l0 = s[-w2-1] | (s[-w2] << 16); // prep current and left pixels
                l1 = s[-1] | (s[0] << 16);
                l2 = s[w2-1] | (s[w2] << 16);
            }
            x = x_offset; xx = x_offset+width;
            s++;
            if (y_offset & 1) { // odd line
                if (x_offset & 1) // starting on an odd pixel, capture it differently
                {
                    r = (l1 >> 24); // (1, 0) red pixel
                    g = ((l0 >> 24) + (l2 >> 24)) >> 1;
                    b = ((l0 & 0xff0000) + (l2 & 0xff0000)) >> 17;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    x++; // advance to next pixel
                }
                for (; x<xx-1; x+=2) {
                    g = (l1 & 0xff0000) >> 16; // (0,0) green pixel
                    b = ((l0 & 0xff0000) + (l2 & 0xff0000)) >> 17;
                    r = (((l1 >> 8) & 0xff) + (l1 >> 24)) >> 1;
                    // faster to keep all calculations in integer math with 15-bit fractions
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    l0 >>= 16; l1 >>= 16; l2 >>= 16; // L-CL-CR-R becomes L-CL-0-0
                    l0 |= (s[-w2] << 16); // grab 3 more pairs of pixels and put in upper 16-bits
                    l1 |= (s[0] << 16);
                    l2 |= (s[w2] << 16);
                    s++;
                    r = (l1 & 0xff00) >> 8; // (1, 0) red pixel
                    g = (((l1 >> 16) & 0xff) + (l1 & 0xff) + ((l0 >> 8) & 0xff) + ((l2 >> 8) & 0xff)) >> 2;
                    b = ((l0 & 0xff) + (l2 & 0xff) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 2;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x+1, (pixel >> 7));
                } // for x
                if (x < xx) { // one final pixel to do
                    g = (l1 >> 16) & 0xff; // (0, 0) green pixel
                    b = ((l0 & 0xff0000) + (l2 & 0xff0000)) >> 17;
                    r = (((l1 >> 8) & 0xff) + (l1 >> 24)) >> 1;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x+1, (pixel >> 7));
                }
            } else { // even line
                if (x_offset & 1) // starting on an odd pixel, capture it differently
                {
                    g = (l1 >> 8) & 0xff; // (1, 0) green pixel
                    r = (((l0 >> 8) & 0xff) + ((l2 >> 8) && 0xff)) >> 1;
                    b = (((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 1;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    x++; // advance to next pixel
                }
                for (; x<xx-1; x+=2) {
                    b = (l1 & 0xff0000) >> 16; // (0,0) blue pixel
                    g = (((l1 >> 8) & 0xff) + (l1 >> 24) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 2;
                    r = (((l0 >> 8) & 0xff) + (l0 >> 24) + ((l2 >> 8) & 0xff) + (l2 >> 24)) >> 2;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                    // prepare for the next set of source pixels
                    l0 >>= 16; l1 >>= 16; l2 >>= 16; // L-CL-CR-R becomes L-CL-0-0
                    l0 |= (s[-w2] << 16); // grab 3 more pairs of pixels and put in upper 16-bits
                    l1 |= (s[0] << 16);
                    l2 |= (s[w2] << 16);
                    s++;
                    g = (l1 & 0xff00) >> 8; // (1, 0) green pixel
                    b = ((l1 & 0xff) + ((l1 >> 16) & 0xff)) >> 1;
                    r = ((l0 & 0xff00) + (l2 & 0xff00)) >> 9;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x+1, (pixel >> 7));
                } // for x
                if (x < xx) { // one final pixel to do
                    b = (l1 & 0xff0000) >> 16; // (0,0) blue pixel
                    g = (((l1 >> 8) & 0xff) + (l1 >> 24) + ((l0 >> 16) & 0xff) + ((l2 >> 16) & 0xff)) >> 2;
                    r = (((l0 >> 8) & 0xff) + (l0 >> 24) + ((l2 >> 8) & 0xff) + (l2 >> 24)) >> 2;
                    pixel = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    IMAGE_PUT_BINARY_PIXEL_FAST(binary, x, (pixel >> 7));
                }
            } // even line
} /* imlib_bayer_to_binary() */

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

#ifdef IMLIB_ENABLE_LENS_CORR
// A simple algorithm for correcting lens distortion.
// See http://www.tannerhelland.com/4743/simple-algorithm-correcting-lens-distortion/
void imlib_lens_corr(image_t *img, float strength, float zoom, float x_corr, float y_corr)
{
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
    void *data = fb_alloc(size, FB_ALLOC_NO_HINT);
    memcpy(data, img->data, size);
    memset(img->data, 0, size);

    int maximum_radius = fast_ceilf(maximum_diameter / 2) + 1; // +1 inclusive of final value
    float *precalculated_table = fb_alloc(maximum_radius * sizeof(float), FB_ALLOC_NO_HINT);
    
    for(int i=0; i < maximum_radius; i++) {
        float r = lens_corr_diameter * i;
        precalculated_table[i] = (fast_atanf(r) / r) * zoom;
    }

    int down_adj = halfHeight + y_off;
    int up_adj = h - 1 - halfHeight + y_off;
    int right_adj = halfWidth + x_off;
    int left_adj = w - 1 - halfWidth + x_off;

    switch(img->bpp) {
        case IMAGE_BPP_BINARY: {
            uint32_t *tmp = (uint32_t *) data;

            for (int y = 0; y < halfHeight; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *row_ptr2 = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, h-1-y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;

                for (int x = 0; x < halfWidth; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float precalculated = precalculated_table[(int)fast_sqrtf(newX2 + newY2)];
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
        case IMAGE_BPP_GRAYSCALE: {
            uint8_t *tmp = (uint8_t *) data;

            for (int y = 0; y < halfHeight; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *row_ptr2 = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, h-1-y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;

                for (int x = 0; x < halfWidth; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float precalculated = precalculated_table[(int)fast_sqrtf(newX2 + newY2)];
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
        case IMAGE_BPP_RGB565: {
            uint16_t *tmp = (uint16_t *) data;

            for (int y = 0; y < halfHeight; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *row_ptr2 = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, h-1-y);
                int newY = y - halfHeight;
                int newY2 = newY * newY;

                for (int x = 0; x < halfWidth; x++) {
                    int newX = x - halfWidth;
                    int newX2 = newX * newX;
                    float precalculated = precalculated_table[(int)fast_sqrtf(newX2 + newY2)];
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
