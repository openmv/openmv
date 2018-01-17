/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __IMLIB_H__
#define __IMLIB_H__
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <arm_math.h>
#include <ff.h>
#include "fb_alloc.h"
#include "umm_malloc.h"
#include "xalloc.h"
#include "array.h"
#include "fmath.h"
#include "collections.h"

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define IM_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

#define INT8_T_BITS     (sizeof(int8_t) * 8)
#define INT8_T_MASK     (INT8_T_BITS - 1)
#define INT8_T_SHIFT    IM_LOG2(INT8_T_MASK)

#define INT16_T_BITS    (sizeof(int16_t) * 8)
#define INT16_T_MASK    (INT16_T_BITS - 1)
#define INT16_T_SHIFT   IM_LOG2(INT16_T_MASK)

#define INT32_T_BITS    (sizeof(int32_t) * 8)
#define INT32_T_MASK    (INT32_T_BITS - 1)
#define INT32_T_SHIFT   IM_LOG2(INT32_T_MASK)

#define INT64_T_BITS    (sizeof(int64_t) * 8)
#define INT64_T_MASK    (INT64_T_BITS - 1)
#define INT64_T_SHIFT   IM_LOG2(INT64_T_MASK)

#define UINT8_T_BITS    (sizeof(uint8_t) * 8)
#define UINT8_T_MASK    (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT   IM_LOG2(UINT8_T_MASK)

#define UINT16_T_BITS   (sizeof(uint16_t) * 8)
#define UINT16_T_MASK   (UINT16_T_BITS - 1)
#define UINT16_T_SHIFT  IM_LOG2(UINT16_T_MASK)

#define UINT32_T_BITS   (sizeof(uint32_t) * 8)
#define UINT32_T_MASK   (UINT32_T_BITS - 1)
#define UINT32_T_SHIFT  IM_LOG2(UINT32_T_MASK)

#define UINT64_T_BITS   (sizeof(uint64_t) * 8)
#define UINT64_T_MASK   (UINT64_T_BITS - 1)
#define UINT64_T_SHIFT  IM_LOG2(UINT64_T_MASK)

/////////////////
// Point Stuff //
/////////////////

typedef struct point {
    int16_t x;
    int16_t y;
} point_t;

void point_init(point_t *ptr, int x, int y);
void point_copy(point_t *dst, point_t *src);
bool point_equal_fast(point_t *ptr0, point_t *ptr1);
int point_quadrance(point_t *ptr0, point_t *ptr1);

////////////////
// Line Stuff //
////////////////

typedef struct line {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} line_t;

bool lb_clip_line(line_t *l, int x, int y, int w, int h);

/////////////////////
// Rectangle Stuff //
/////////////////////

typedef struct rectangle {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} rectangle_t;

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h);
void rectangle_copy(rectangle_t *dst, rectangle_t *src);
bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1);
bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1);
void rectangle_intersected(rectangle_t *dst, rectangle_t *src);
void rectangle_united(rectangle_t *dst, rectangle_t *src);

/////////////////
// Color Stuff //
/////////////////

typedef struct color_thresholds_list_lnk_data
{
    uint8_t LMin, LMax; // or grayscale
    int8_t AMin, AMax;
    int8_t BMin, BMax;
}
color_thresholds_list_lnk_data_t;

#define COLOR_THRESHOLD_BINARY(pixel, threshold, invert) ((pixel) ^ (invert))

#define COLOR_THRESHOLD_GRAYSCALE(pixel, threshold, invert) \
({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
})

#define COLOR_THRESHOLD_RGB565(pixel, threshold, invert) \
({ \
    __typeof__ (pixel) _pixel = (pixel); \
    __typeof__ (threshold) _threshold = (threshold); \
    __typeof__ (invert) _invert = (invert); \
    uint8_t _l = COLOR_RGB565_TO_L(_pixel); \
    int8_t _a = COLOR_RGB565_TO_A(_pixel); \
    int8_t _b = COLOR_RGB565_TO_B(_pixel); \
    ((_threshold->LMin <= _l) && (_l <= _threshold->LMax) && (_threshold->AMin <= _a) && (_a <= _threshold->AMax) && (_threshold->BMin <= _b) && (_b <= _threshold->BMax)) ^ _invert; \
})

#define COLOR_BINARY_MIN 0
#define COLOR_BINARY_MAX 1

#define COLOR_GRAYSCALE_MIN 0
#define COLOR_GRAYSCALE_MAX 255

#define COLOR_R5_MIN 0
#define COLOR_R5_MAX 31
#define COLOR_G6_MIN 0
#define COLOR_G6_MAX 63
#define COLOR_B5_MIN 0
#define COLOR_B5_MAX 31

#define COLOR_R8_MIN 0
#define COLOR_R8_MAX 255
#define COLOR_G8_MIN 0
#define COLOR_G8_MAX 255
#define COLOR_B8_MIN 0
#define COLOR_B8_MAX 255

#define COLOR_L_MIN 0
#define COLOR_L_MAX 100
#define COLOR_A_MIN -128
#define COLOR_A_MAX 127
#define COLOR_B_MIN -128
#define COLOR_B_MAX 127

#define COLOR_Y_MIN -128
#define COLOR_Y_MAX 127
#define COLOR_U_MIN -128
#define COLOR_V_MAX 127
#define COLOR_U_MIN -128
#define COLOR_V_MAX 127

extern const uint8_t rb528_table[32];
extern const uint8_t g628_table[64];

#define COLOR_R5_TO_R8(color) rb528_table[color]
#define COLOR_G6_TO_G8(color) g628_table[color]
#define COLOR_B5_TO_B8(color) rb528_table[color]

extern const uint8_t rb825_table[256];
extern const uint8_t g826_table[256];

#define COLOR_R8_TO_R5(color) rb825_table[color]
#define COLOR_G8_TO_G6(color) g826_table[color]
#define COLOR_B8_TO_B5(color) rb825_table[color]

// RGB565 Stuff //

#define COLOR_RGB565_TO_R5(pixel) (((pixel) >> 3) & 0x1F)
#define COLOR_RGB565_TO_G6(pixel) \
({ \
    __typeof__ (pixel) _pixel = (pixel); \
    ((_pixel & 0x07) << 3) | (_pixel >> 13); \
})
#define COLOR_RGB565_TO_B5(pixel) (((pixel) >> 8) & 0x1F)
#define COLOR_RGB565_TO_R8(pixel) COLOR_R5_TO_R8(COLOR_RGB565_TO_R5(pixel))
#define COLOR_RGB565_TO_G8(pixel) COLOR_G6_TO_G8(COLOR_RGB565_TO_G6(pixel))
#define COLOR_RGB565_TO_B8(pixel) COLOR_B5_TO_B8(COLOR_RGB565_TO_B5(pixel))

#define COLOR_R5_G6_B5_TO_RGB565(r5, g6, b5) \
({ \
    __typeof__ (r5) _r5 = (r5); \
    __typeof__ (g6) _g6 = (g6); \
    __typeof__ (b5) _b5 = (b5); \
    (_r5 << 3) | (_g6 >> 3) | (_g6 << 13) | (_b5 << 8); \
})

#define COLOR_R8_G8_B8_TO_RGB565(r8, g8, b8) COLOR_R5_G6_B5_TO_RGB565(COLOR_R8_TO_R5(r8), COLOR_G8_TO_G6(g8), COLOR_B8_TO_B5(b8))

extern const int8_t lab_table[196608];
extern const int8_t yuv_table[196608];

#define COLOR_RGB565_TO_L(pixel) lab_table[(pixel) * 3]
#define COLOR_RGB565_TO_A(pixel) lab_table[((pixel) * 3) + 1]
#define COLOR_RGB565_TO_B(pixel) lab_table[((pixel) * 3) + 2]
#define COLOR_RGB565_TO_Y(pixel) yuv_table[(pixel) * 3]
#define COLOR_RGB565_TO_U(pixel) yuv_table[((pixel) * 3) + 1]
#define COLOR_RGB565_TO_V(pixel) yuv_table[((pixel) * 3) + 2]

// https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions
// https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation

#define COLOR_LAB_TO_RGB565(l, a, b) \
({ \
    __typeof__ (l) _l = (l); \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    float _x = ((_l + 16) * 0.008621f) + (_a * 0.002f); \
    float _y = ((_l + 16) * 0.008621f); \
    float _z = ((_l + 16) * 0.008621f) - (_b * 0.005f); \
    _x = ((_x > 0.206897f) ? (_x * _x * _x) : ((0.128419f * _x) - 0.017713f)) * 095.047f; \
    _y = ((_y > 0.206897f) ? (_y * _y * _y) : ((0.128419f * _y) - 0.017713f)) * 100.000f; \
    _z = ((_z > 0.206897f) ? (_z * _z * _z) : ((0.128419f * _z) - 0.017713f)) * 108.883f; \
    float _r_lin = ((_x * +3.2406f) + (_y * -1.5372f) + (_z * -0.4986f)) / 100.0f; \
    float _g_lin = ((_x * -0.9689f) + (_y * +1.8758f) + (_z * +0.0415f)) / 100.0f; \
    float _b_lin = ((_x * +0.0557f) + (_y * -0.2040f) + (_z * +1.0570f)) / 100.0f; \
    _r_lin = (_r_lin > 0.0031308f) ? ((1.055f * powf(_r_lin, 0.416666f)) - 0.055f) : (_r_lin * 12.92f); \
    _g_lin = (_g_lin > 0.0031308f) ? ((1.055f * powf(_g_lin, 0.416666f)) - 0.055f) : (_g_lin * 12.92f); \
    _b_lin = (_b_lin > 0.0031308f) ? ((1.055f * powf(_b_lin, 0.416666f)) - 0.055f) : (_b_lin * 12.92f); \
    unsigned int _r = IM_MAX(IM_MIN(roundf(_r_lin * COLOR_R5_MAX), COLOR_R5_MAX), COLOR_R5_MIN); \
    unsigned int _g = IM_MAX(IM_MIN(roundf(_g_lin * COLOR_G6_MAX), COLOR_G6_MAX), COLOR_G6_MIN); \
    unsigned int _b = IM_MAX(IM_MIN(roundf(_b_lin * COLOR_B5_MAX), COLOR_B5_MAX), COLOR_B5_MIN); \
    COLOR_R5_G6_B5_TO_RGB565(_r, _g, _b); \
})

// https://en.wikipedia.org/wiki/YCbCr -> JPEG Conversion

#define COLOR_YUV_TO_RGB565(y, u, v) \
({ \
    __typeof__ (y) _y = (y); \
    __typeof__ (u) _u = (u); \
    __typeof__ (v) _v = (v); \
    unsigned int _r = IM_MAX(IM_MIN(128 + _y + ((((uint32_t) ((1.402000 * 65536) + 0.5)) * _v) >> 16), COLOR_R8_MAX), COLOR_R8_MIN); \
    unsigned int _g = IM_MAX(IM_MIN(128 + _y - (((((uint32_t) ((0.344136 * 65536) + 0.5)) * _u) + (((uint32_t) ((0.714136 * 65536) + 0.5)) * _v)) >> 16), COLOR_G8_MAX), COLOR_G8_MIN); \
    unsigned int _b = IM_MAX(IM_MIN(128 + _y + ((((uint32_t) ((1.772000 * 65536) + 0.5)) * _u) >> 16), COLOR_B8_MAX), COLOR_B8_MIN); \
    COLOR_R8_G8_B8_TO_RGB565(_r, _g, _b); \
})

#define COLOR_BAYER_TO_RGB565(img, x, y, r, g, b)            \
({                                                           \
    __typeof__ (x) __x = (x);                                \
    __typeof__ (y) __y = (y);                                \
    if ((__y % 2) == 0) {                                    \
        if ((__x % 2) == 0) {                                \
            r = (IM_GET_RAW_PIXEL(img, __x-1, __y-1)  +      \
                 IM_GET_RAW_PIXEL(img, __x+1, __y-1)  +      \
                 IM_GET_RAW_PIXEL(img, __x-1, __y+1)  +      \
                 IM_GET_RAW_PIXEL(img, __x+1, __y+1)) >> 2;  \
                                                             \
            g = (IM_GET_RAW_PIXEL(img, __x,   __y-1)  +      \
                 IM_GET_RAW_PIXEL(img, __x,   __y+1)  +      \
                 IM_GET_RAW_PIXEL(img, __x-1, __y)    +      \
                 IM_GET_RAW_PIXEL(img, __x+1, __y))   >> 2;  \
                                                             \
            b = IM_GET_RAW_PIXEL(img,  __x, __y);            \
        } else {                                             \
            r = (IM_GET_RAW_PIXEL(img, __x, __y-1)  +        \
                 IM_GET_RAW_PIXEL(img, __x, __y+1)) >> 1;    \
                                                             \
            b = (IM_GET_RAW_PIXEL(img, __x-1, __y)  +        \
                 IM_GET_RAW_PIXEL(img, __x+1, __y)) >> 1;    \
                                                             \
            g =  IM_GET_RAW_PIXEL(img, __x, __y);            \
        }                                                    \
    } else {                                                 \
        if ((__x % 2) == 0) {                                \
            r = (IM_GET_RAW_PIXEL(img, __x-1, __y)  +        \
                 IM_GET_RAW_PIXEL(img, __x+1, __y)) >> 1;    \
                                                             \
            g =  IM_GET_RAW_PIXEL(img, __x, __y);            \
                                                             \
            b = (IM_GET_RAW_PIXEL(img, __x, __y-1)  +        \
                 IM_GET_RAW_PIXEL(img, __x, __y+1)) >> 1;    \
        } else {                                             \
            r = IM_GET_RAW_PIXEL(img,  __x, __y);            \
                                                             \
            g = (IM_GET_RAW_PIXEL(img, __x, __y-1)    +      \
                 IM_GET_RAW_PIXEL(img, __x, __y+1)    +      \
                 IM_GET_RAW_PIXEL(img, __x-1, __y)    +      \
                 IM_GET_RAW_PIXEL(img, __x+1, __y))   >> 2;  \
                                                             \
            b = (IM_GET_RAW_PIXEL(img, __x-1, __y-1)  +      \
                 IM_GET_RAW_PIXEL(img, __x+1, __y-1)  +      \
                 IM_GET_RAW_PIXEL(img, __x-1, __y+1)  +      \
                 IM_GET_RAW_PIXEL(img, __x+1, __y+1)) >> 2;  \
        }                                                    \
    }                                                        \
    r  = IM_R825(r);                                         \
    g  = IM_G826(g);                                         \
    b  = IM_B825(b);                                         \
})

#define COLOR_BINARY_TO_GRAYSCALE(pixel) ((pixel) * COLOR_GRAYSCALE_MAX)
#define COLOR_BINARY_TO_RGB565(pixel) COLOR_YUV_TO_RGB565((pixel) * 127, 0, 0)
#define COLOR_RGB565_TO_BINARY(pixel) (COLOR_RGB565_TO_Y(pixel) == 127)
#define COLOR_RGB565_TO_GRAYSCALE(pixel) (COLOR_RGB565_TO_Y(pixel) + 128)
#define COLOR_GRAYSCALE_TO_BINARY(pixel) ((pixel) == COLOR_GRAYSCALE_MAX)
#define COLOR_GRAYSCALE_TO_RGB565(pixel) COLOR_YUV_TO_RGB565((pixel) - 128, 0, 0)

/////////////////
// Image Stuff //
/////////////////

typedef enum image_bpp
{
    IMAGE_BPP_BINARY,       // BPP = 0
    IMAGE_BPP_GRAYSCALE,    // BPP = 1
    IMAGE_BPP_RGB565,       // BPP = 2
    IMAGE_BPP_BAYER,        // BPP = 3
    IMAGE_BPP_JPEG          // BPP > 3
}
image_bpp_t;

typedef struct image {
    int w;
    int h;
    int bpp;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
} image_t;

void image_init(image_t *ptr, int w, int h, int bpp, void *data);
void image_copy(image_t *dst, image_t *src);
uint32_t image_size(image_t *ptr);

#define IMAGE_GET_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    (((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] >> (_x & UINT32_T_MASK)) & 1; \
})

#define IMAGE_PUT_BINARY_PIXEL(image, x, y, v) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    size_t _i = (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT); \
    size_t _j = _x & UINT32_T_MASK; \
    ((uint32_t *) _image->data)[_i] = (((uint32_t *) _image->data)[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
})

#define IMAGE_CLEAR_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] &= ~(1 << (_x & UINT32_T_MASK)); \
})

#define IMAGE_SET_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] |= 1 << (_x & UINT32_T_MASK); \
})

#define IMAGE_GET_GRAYSCALE_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x]; \
})

#define IMAGE_PUT_GRAYSCALE_PIXEL(image, x, y, v) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    ((uint8_t *) _image->data)[(_image->w * _y) + _x] = _v; \
})

#define IMAGE_GET_RGB565_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data)[(_image->w * _y) + _x]; \
})

#define IMAGE_PUT_RGB565_PIXEL(image, x, y, v) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    __typeof__ (v) _v = (v); \
    ((uint16_t *) _image->data)[(_image->w * _y) + _x] = _v; \
})

#ifdef __arm__
    #define IMAGE_REV_RGB565_PIXEL(pixel) \
    ({ \
        __typeof__ (pixel) _pixel = (pixel); \
        __REV16(_pixel); \
    })
#else
    #define IMAGE_REV_RGB565_PIXEL(pixel) \
    ({ \
        __typeof__ (pixel) _pixel = (pixel); \
        ((_pixel >> 8) | (_pixel << 8)) & 0xFFFF; \
    })
#endif

#define IMAGE_COMPUTE_TARGET_SIZE_SCALE_FACTOR(target_size, source_rect) \
__typeof__ (target_size) _target_size = (target_size); \
__typeof__ (source_rect) _source_rect = (source_rect); \
int IMAGE_X_SOURCE_OFFSET = _source_rect->p.x; \
int IMAGE_Y_SOURCE_OFFSET = _source_rect->p.y; \
int IMAGE_X_TARGET_OFFSET = 0; \
int IMAGE_Y_TARGET_OFFSET = 0; \
float IMAGE_X_RATIO = ((float) _source_rect->s.w) / ((float) _target_size->w); \
float IMAGE_Y_RATIO = ((float) _source_rect->s.h) / ((float) _target_size->h); \
({ 0; })

#define IMAGE_COMPUTE_TARGET_RECT_SCALE_FACTOR(target_rect, source_rect) \
__typeof__ (target_rect) _target_rect = (target_rect); \
__typeof__ (source_rect) _source_rect = (source_rect); \
int IMAGE_X_SOURCE_OFFSET = _source_rect->p.x; \
int IMAGE_Y_SOURCE_OFFSET = _source_rect->p.y; \
int IMAGE_X_TARGET_OFFSET = _target_rect->p.x; \
int IMAGE_Y_TARGET_OFFSET = _target_rect->p.y; \
float IMAGE_X_RATIO = ((float) _source_rect->s.w) / ((float) _target_rect->s.w); \
float IMAGE_Y_RATIO = ((float) _source_rect->s.h) / ((float) _target_rect->s.h); \
({ 0; })

#define IMAGE_GET_SCALED_BINARY_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    IMAGE_GET_BINARY_PIXEL(_image, ((size_t) ((IMAGE_X_RATIO * (_x - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET); \
})

#define IMAGE_GET_SCALED_GRAYSCALE_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    IMAGE_GET_GRAYSCALE_PIXEL(_image, ((size_t) ((IMAGE_X_RATIO * (_x - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET); \
})

#define IMAGE_GET_SCALED_RGB565_PIXEL(image, x, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (x) _x = (x); \
    __typeof__ (y) _y = (y); \
    IMAGE_GET_RGB565_PIXEL(_image, ((size_t) ((IMAGE_X_RATIO * (_x - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET, ((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET); \
})

// Fast Stuff //

#define IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data) + (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y); \
})

#define IMAGE_INC_BINARY_PIXEL_ROW_PTR(row_ptr, image) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (image) _image = (image); \
    _row_ptr + ((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT); \
})

#define IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    (_row_ptr[_x >> UINT32_T_SHIFT] >> (_x & UINT32_T_MASK)) & 1; \
})

#define IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    size_t _i = _x >> UINT32_T_SHIFT; \
    size_t _j = _x & UINT32_T_MASK; \
    _row_ptr[_i] = (_row_ptr[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
})

#define IMAGE_CLEAR_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x >> UINT32_T_SHIFT] &= ~(1 << (_x & UINT32_T_MASK)); \
})

#define IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x >> UINT32_T_SHIFT] |= 1 << (_x & UINT32_T_MASK); \
})

#define IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * _y); \
})

#define IMAGE_INC_GRAYSCALE_PIXEL_ROW_PTR(row_ptr, image) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (image) _image = (image); \
    row_ptr + _image->w; \
})

#define IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
})

#define IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

#define IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->w * _y); \
})

#define IMAGE_INC_RGB565_PIXEL_ROW_PTR(row_ptr, image) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (image) _image = (image); \
    row_ptr + _image->w; \
})

#define IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    _row_ptr[_x]; \
})

#define IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, v) \
({ \
    __typeof__ (row_ptr) _row_ptr = (row_ptr); \
    __typeof__ (x) _x = (x); \
    __typeof__ (v) _v = (v); \
    _row_ptr[_x] = _v; \
})

#define IMAGE_COMPUTE_SCALED_BINARY_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint32_t *) _image->data) + (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * (((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET)); \
})

#define IMAGE_GET_SCALED_BINARY_PIXEL_FAST(row_ptr, x) IMAGE_GET_BINARY_PIXEL_FAST((row_ptr), ((size_t) ((IMAGE_X_RATIO * ((x) - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET)

#define IMAGE_COMPUTE_SCALED_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint8_t *) _image->data) + (_image->w * (((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET)); \
})

#define IMAGE_GET_SCALED_GRAYSCALE_PIXEL_FAST(row_ptr, x) IMAGE_GET_GRAYSCALE_PIXEL_FAST((row_ptr), ((size_t) ((IMAGE_X_RATIO * ((x) - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET)

#define IMAGE_COMPUTE_SCALED_RGB565_PIXEL_ROW_PTR(image, y) \
({ \
    __typeof__ (image) _image = (image); \
    __typeof__ (y) _y = (y); \
    ((uint16_t *) _image->data) + (_image->w * (((size_t) ((IMAGE_Y_RATIO * (_y - IMAGE_Y_TARGET_OFFSET)) + 0.5)) + IMAGE_Y_SOURCE_OFFSET)); \
})

#define IMAGE_GET_SCALED_RGB565_PIXEL_FAST(row_ptr, x) IMAGE_GET_RGB565_PIXEL_FAST((row_ptr), ((size_t) ((IMAGE_X_RATIO * ((x) - IMAGE_X_TARGET_OFFSET)) + 0.5)) + IMAGE_X_SOURCE_OFFSET)

// Old Image Macros - Will be refactor and removed. But, only after making sure through testing new macros work.

#define IM_SWAP16(x) __REV16(x) // Swap bottom two chars in short.
#define IM_SWAP32(x) __REV32(x) // Swap bottom two shorts in long.

// RGB565 to RGB888 conversion.
extern const uint8_t rb528_table[32];
extern const uint8_t g628_table[64];

#define IM_R528(p) \
    ({ __typeof__ (p) _p = (p); \
       rb528_table[_p]; })

#define IM_G628(p) \
    ({ __typeof__ (p) _p = (p); \
       g628_table[_p]; })

#define IM_B528(p) \
    ({ __typeof__ (p) _p = (p); \
       rb528_table[_p]; })

// RGB888 to RGB565 conversion.
extern const uint8_t rb825_table[256];
extern const uint8_t g826_table[256];

#define IM_R825(p) \
    ({ __typeof__ (p) _p = (p); \
       rb825_table[_p]; })

#define IM_G826(p) \
    ({ __typeof__ (p) _p = (p); \
       g826_table[_p]; })

#define IM_B825(p) \
    ({ __typeof__ (p) _p = (p); \
       rb825_table[_p]; })

// Split RGB565 values (note the RGB565 value is byte reversed).

#define IM_R565(p) \
    ({ __typeof__ (p) _p = (p); \
       ((_p)>>3)&0x1F; })

#define IM_G565(p) \
    ({ __typeof__ (p) _p = (p); \
       (((_p)&0x07)<<3)|((_p)>>13); })

#define IM_B565(p) \
    ({ __typeof__ (p) _p = (p); \
       ((_p)>>8)&0x1F; })

// Merge RGB565 values (note the RGB565 value is byte reversed).

#define IM_RGB565(r, g, b) \
    ({ __typeof__ (r) _r = (r); \
       __typeof__ (g) _g = (g); \
       __typeof__ (b) _b = (b); \
       ((_r)<<3)|((_g)>>3)|((_g)<<13)|((_b)<<8); })

// Image kernels
extern const int8_t kernel_gauss_3[9];
extern const int8_t kernel_gauss_5[25];
extern const int8_t kernel_laplacian_3[9];
extern const int8_t kernel_high_pass_3[9];

#define IM_RGB5652L(p) \
    ({ __typeof__ (p) _p = (p); \
       lab_table[_p * 3]; })

#define IM_RGB5652A(p) \
    ({ __typeof__ (p) _p = (p); \
       lab_table[(_p * 3) + 1]; })

#define IM_RGB5652B(p) \
    ({ __typeof__ (p) _p = (p); \
       lab_table[(_p * 3) + 2]; })

// Grayscale maxes
#define IM_MAX_GS (255)

// RGB565 maxes
#define IM_MAX_R5 (31)
#define IM_MAX_G6 (63)
#define IM_MAX_B5 (31)

// Grayscale histogram
#define IM_G_HIST_SIZE (256)
#define IM_G_HIST_OFFSET (0)

// LAB histogram
#define IM_L_HIST_SIZE (256)
#define IM_L_HIST_OFFSET (0)
#define IM_A_HIST_SIZE (256)
#define IM_A_HIST_OFFSET (256)
#define IM_B_HIST_SIZE (256)
#define IM_B_HIST_OFFSET (512)

#define IM_IS_BINARY(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp == 0; })

#define IM_IS_GS(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp == 1; })

#define IM_IS_RGB565(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp == 2; })

#define IM_IS_BAYER(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp == 3; })

#define IM_IS_JPEG(img) \
    ({ __typeof__ (img) _img = (img); \
       _img->bpp >= 4; })

#define IM_IS_MUTABLE(img) \
    ({ __typeof__ (img) _img = (img); \
       (_img->bpp == 1 || _img->bpp == 2); })

#define IM_X_INSIDE(img, x) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       (0<=_x)&&(_x<_img->w); })

#define IM_Y_INSIDE(img, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (y) _y = (y); \
       (0<=_y)&&(_y<_img->h); })

#define IM_GET_GS_PIXEL(img, x, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       ((uint8_t*)_img->pixels)[(_y*_img->w)+_x]; })

#define IM_GET_RAW_PIXEL(img, x, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       ((uint8_t*)_img->pixels)[(_y*_img->w)+_x]; })

#define IM_GET_RGB565_PIXEL(img, x, y) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       ((uint16_t*)_img->pixels)[(_y*_img->w)+_x]; })

#define IM_SET_GS_PIXEL(img, x, y, p) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       __typeof__ (p) _p = (p); \
       ((uint8_t*)_img->pixels)[(_y*_img->w)+_x]=_p; })

#define IM_SET_RGB565_PIXEL(img, x, y, p) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x); \
       __typeof__ (y) _y = (y); \
       __typeof__ (p) _p = (p); \
       ((uint16_t*)_img->pixels)[(_y*_img->w)+_x]=_p; })

#define IM_EQUAL(img0, img1) \
    ({ __typeof__ (img0) _img0 = (img0); \
       __typeof__ (img1) _img1 = (img1); \
       (_img0->w==_img1->w)&&(_img0->h==_img1->h)&&(_img0->bpp==_img1->bpp); })

typedef struct simple_color {
    uint8_t G;
    union {
        int8_t L;
        uint8_t red; // RGB888 not RGB565
    };
    union {
        int8_t A;
        uint8_t green; // RGB888 not RGB565
    };
    union {
        int8_t B;
        uint8_t blue; // RGB888 not RGB565
    };
}
simple_color_t;

typedef struct integral_image {
    int w;
    int h;
    uint32_t *data;
} i_image_t;

typedef struct {
    int w;
    int h;
    int y_offs;
    int x_ratio;
    int y_ratio;
    uint32_t **data;
    uint32_t **swap;
} mw_image_t;

typedef struct _vector {
    float x;
    float y;
    float m;
    uint16_t cx,cy;
} vec_t;

typedef struct cluster {
    int x, y, w, h;
    array_t *points;
} cluster_t;

// Return the distance between a cluster centroid and some object.
typedef float (*cluster_dist_t)(int cx, int cy, void *obj);

/* Keypoint */
typedef struct kp {
    uint16_t x;
    uint16_t y;
    uint16_t score;
    uint16_t octave;
    uint16_t angle;
    uint16_t matched;
    uint8_t desc[32];
} kp_t;

typedef struct size {
    int w;
    int h;
} wsize_t;

/* Haar cascade struct */
typedef struct cascade {
    int std;                        // Image standard deviation.
    int step;                       // Image scanning factor.
    float threshold;                // Detection threshold.
    float scale_factor;             // Image scaling factor.
    int n_stages;                   // Number of stages in the cascade.
    int n_features;                 // Number of features in the cascade.
    int n_rectangles;               // Number of rectangles in the cascade.
    struct size window;             // Detection window size.
    struct image *img;              // Grayscale image.
    mw_image_t *sum;                // Integral image.
    mw_image_t *ssq;                // Squared integral image.
    uint8_t *stages_array;          // Number of features per stage.
    int16_t *stages_thresh_array;   // Stages thresholds.
    int16_t *tree_thresh_array;     // Features threshold (1 per feature).
    int16_t *alpha1_array;          // Alpha1 array (1 per feature).
    int16_t *alpha2_array;          // Alpha2 array (1 per feature).
    int8_t *num_rectangles_array;   // Number of rectangles per features (1 per feature).
    int8_t *weights_array;          // Rectangles weights (1 per rectangle).
    int8_t *rectangles_array;       // Rectangles array.
} cascade_t;

typedef struct bmp_read_settings {
    int32_t bmp_w;
    int32_t bmp_h;
    uint16_t bmp_bpp;
    uint32_t bmp_fmt;
    uint32_t bmp_row_bytes;
} bmp_read_settings_t;

typedef struct ppm_read_settings {
    uint8_t read_int_c;
    bool read_int_c_valid;
    uint8_t ppm_fmt;
} ppm_read_settings_t;

typedef enum save_image_format {
    FORMAT_DONT_CARE,
    FORMAT_BMP,
    FORMAT_PNM,
    FORMAT_JPG,
    FORMAT_RAW,
} save_image_format_t;

typedef struct img_read_settings {
    union
    {
        bmp_read_settings_t bmp_rs;
        ppm_read_settings_t ppm_rs;
    };
    save_image_format_t format;
} img_read_settings_t;

typedef void (*line_op_t)(image_t*, int, uint8_t*, void*);

typedef enum descriptor_type {
    DESC_LBP,
    DESC_ORB,
} descriptor_t;

typedef enum edge_detector_type {
   EDGE_CANNY,
   EDGE_SIMPLE,
} edge_detector_t;

typedef enum template_match {
    SEARCH_EX,  // Exhaustive search
    SEARCH_DS,  // Diamond search
} template_match_t;

typedef enum  jpeg_subsample {
    JPEG_SUBSAMPLE_1x1 = 0x11,  // 1x1 chroma subsampling (No subsampling)
    JPEG_SUBSAMPLE_2x1 = 0x21,  // 2x2 chroma subsampling
    JPEG_SUBSAMPLE_2x2 = 0x22,  // 2x2 chroma subsampling
} jpeg_subsample_t;

typedef enum corner_detector_type {
    CORNER_FAST,
    CORNER_AGAST
} corner_detector_t;

typedef struct histogram {
    int LBinCount;
    float *LBins;
    int ABinCount;
    float *ABins;
    int BBinCount;
    float *BBins;
} histogram_t;

typedef struct percentile {
    uint8_t LValue;
    int8_t AValue;
    int8_t BValue;
} percentile_t;

typedef struct statistics {
    uint8_t LMean, LMedian, LMode, LSTDev, LMin, LMax, LLQ, LUQ;
    int8_t AMean, AMedian, AMode, ASTDev, AMin, AMax, ALQ, AUQ;
    int8_t BMean, BMedian, BMode, BSTDev, BMin, BMax, BLQ, BUQ;
} statistics_t;

typedef struct find_blobs_list_lnk_data {
    rectangle_t rect;
    uint32_t pixels;
    point_t centroid;
    float rotation;
    uint16_t code, count;
} find_blobs_list_lnk_data_t;

typedef struct find_lines_list_lnk_data {
    line_t line;
    uint32_t magnitude;
    int16_t theta, rho;
} find_lines_list_lnk_data_t;

typedef struct find_circles_list_lnk_data {
    point_t p;
    uint32_t r, magnitude;
} find_circles_list_lnk_data_t;

typedef struct find_rects_list_lnk_data {
    point_t corners[4];
    rectangle_t rect;
    uint32_t magnitude;
} find_rects_list_lnk_data_t;

typedef struct find_qrcodes_list_lnk_data {
    point_t corners[4];
    rectangle_t rect;
    size_t payload_len;
    char *payload;
    uint8_t version, ecc_level, mask, data_type;
    uint32_t eci;
} find_qrcodes_list_lnk_data_t;

typedef enum apriltag_families {
    TAG16H5 = 1,
    TAG25H7 = 2,
    TAG25H9 = 4,
    TAG36H10 = 8,
    TAG36H11 = 16,
    ARTOOLKIT = 32
} apriltag_families_t;

typedef struct find_apriltags_list_lnk_data {
    point_t corners[4];
    rectangle_t rect;
    uint16_t id;
    uint8_t family, hamming;
    point_t centroid;
    float goodness, decision_margin;
    float x_translation, y_translation, z_translation;
    float x_rotation, y_rotation, z_rotation;
} find_apriltags_list_lnk_data_t;

typedef struct find_datamatrices_list_lnk_data {
    point_t corners[4];
    rectangle_t rect;
    size_t payload_len;
    char *payload;
    uint16_t rotation;
    uint8_t rows, columns;
    uint16_t capacity, padding;
} find_datamatrices_list_lnk_data_t;

typedef enum barcodes {
    BARCODE_EAN2,
    BARCODE_EAN5,
    BARCODE_EAN8,
    BARCODE_UPCE,
    BARCODE_ISBN10,
    BARCODE_UPCA,
    BARCODE_EAN13,
    BARCODE_ISBN13,
    BARCODE_I25,
    BARCODE_DATABAR,
    BARCODE_DATABAR_EXP,
    BARCODE_CODABAR,
    BARCODE_CODE39,
    BARCODE_PDF417,
    BARCODE_CODE93,
    BARCODE_CODE128
} barcodes_t;

typedef struct find_barcodes_list_lnk_data {
    point_t corners[4];
    rectangle_t rect;
    size_t payload_len;
    char *payload;
    uint16_t type, rotation;
    int quality;
} find_barcodes_list_lnk_data_t;

/* Color space functions */
void imlib_rgb_to_lab(simple_color_t *rgb, simple_color_t *lab);
void imlib_lab_to_rgb(simple_color_t *lab, simple_color_t *rgb);
void imlib_rgb_to_grayscale(simple_color_t *rgb, simple_color_t *grayscale);
void imlib_grayscale_to_rgb(simple_color_t *grayscale, simple_color_t *rgb);
uint16_t imlib_yuv_to_rgb(uint8_t y, int8_t u, int8_t v);

/* Image file functions */
void ppm_read_geometry(FIL *fp, image_t *img, const char *path, ppm_read_settings_t *rs);
void ppm_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, ppm_read_settings_t *rs);
void ppm_read(image_t *img, const char *path);
void ppm_write_subimg(image_t *img, const char *path, rectangle_t *r);
bool bmp_read_geometry(FIL *fp, image_t *img, const char *path, bmp_read_settings_t *rs);
void bmp_read_pixels(FIL *fp, image_t *img, int line_start, int line_end, bmp_read_settings_t *rs);
void bmp_read(image_t *img, const char *path);
void bmp_write_subimg(image_t *img, const char *path, rectangle_t *r);
bool jpeg_compress(image_t *src, image_t *dst, int quality, bool realloc);
void jpeg_read_geometry(FIL *fp, image_t *img, const char *path);
void jpeg_read_pixels(FIL *fp, image_t *img);
void jpeg_read(image_t *img, const char *path);
void jpeg_write(image_t *img, const char *path, int quality);
bool imlib_read_geometry(FIL *fp, image_t *img, const char *path, img_read_settings_t *rs);
void imlib_image_operation(image_t *img, const char *path, image_t *other, line_op_t op, void *data);
void imlib_load_image(image_t *img, const char *path);
void imlib_save_image(image_t *img, const char *path, rectangle_t *roi, int quality);
void imlib_copy_image(image_t *dst, image_t *src, rectangle_t *roi);

/* GIF functions */
void gif_open(FIL *fp, int width, int height, bool color, bool loop);
void gif_add_frame(FIL *fp, image_t *img, uint16_t delay);
void gif_close(FIL *fp);

/* MJPEG functions */
void mjpeg_open(FIL *fp, int width, int height);
void mjpeg_add_frame(FIL *fp, uint32_t *frames, uint32_t *bytes, image_t *img, int quality);
void mjpeg_close(FIL *fp, uint32_t *frames, uint32_t *bytes, float fps);

/* Basic image functions */
int imlib_get_pixel(image_t *img, int x, int y);
void imlib_set_pixel(image_t *img, int x, int y, int p);

/* Point functions */
point_t *point_alloc(int16_t x, int16_t y);
bool point_equal(point_t *p1, point_t *p2);
float point_distance(point_t *p1, point_t *p2);

/* Rectangle functions */
rectangle_t *rectangle_alloc(int16_t x, int16_t y, int16_t w, int16_t h);
bool rectangle_equal(rectangle_t *r1, rectangle_t *r2);
bool rectangle_intersects(rectangle_t *r1, rectangle_t *r2);
bool rectangle_subimg(image_t *img, rectangle_t *r, rectangle_t *r_out);
array_t *rectangle_merge(array_t *rectangles);
void rectangle_expand(rectangle_t *r, int x, int y);

/* Drawing functions */
void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c);
void imlib_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c);
void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c);
void imlib_draw_string(image_t *img, int x_off, int y_off, const char *str, int c);

/* Binary functions */
void imlib_binary(image_t *img,
                  int num_thresholds, simple_color_t *l_thresholds, simple_color_t *h_thresholds,
                  bool invert);
void imlib_invert(image_t *img);
void imlib_b_and(image_t *img, const char *path, image_t *other);
void imlib_b_nand(image_t *img, const char *path, image_t *other);
void imlib_b_or(image_t *img, const char *path, image_t *other);
void imlib_b_nor(image_t *img, const char *path, image_t *other);
void imlib_b_xor(image_t *img, const char *path, image_t *other);
void imlib_b_xnor(image_t *img, const char *path, image_t *other);
void imlib_erode(image_t *img, int ksize, int threshold);
void imlib_dilate(image_t *img, int ksize, int threshold);

/* Background Subtraction (Frame Differencing) functions */
void imlib_negate(image_t *img);
void imlib_difference(image_t *img, const char *path, image_t *other);
void imlib_replace(image_t *img, const char *path, image_t *other);
void imlib_blend(image_t *img, const char *path, image_t *other, int alpha);
void imlib_max(image_t *img, const char *path, image_t *other);
void imlib_min(image_t *img, const char *path, image_t *other);

/* Image Morphing */
void imlib_morph(image_t *img, const int ksize, const int8_t *krn, const float m, const int b);

/* Separable 2D convolution */
void imlib_sepconv3(image_t *img, const int8_t *krn, const float m, const int b);

/* Image Statistics */
int imlib_image_mean(image_t *src); // grayscale only
int imlib_image_std(image_t *src); // grayscale only

/* Image Filtering */
void imlib_midpoint_filter(image_t *img, const int ksize, const int bias, bool threshold, int offset, bool invert);
void imlib_mean_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert);
void imlib_mode_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert);
void imlib_median_filter(image_t *img, const int ksize, const int percentile, bool threshold, int offset, bool invert);
void imlib_histeq(image_t *img);
void imlib_mask_ellipse(image_t *img);

/* Template Matching */
void imlib_midpoint_pool(image_t *img_i, image_t *img_o, int x_div, int y_div, const int bias);
void imlib_mean_pool(image_t *img_i, image_t *img_o, int x_div, int y_div);
float imlib_template_match_ds(image_t *image, image_t *template, rectangle_t *r);
float imlib_template_match_ex(image_t *image, image_t *template, rectangle_t *roi, int step, rectangle_t *r);

/* Clustering functions */
array_t *cluster_kmeans(array_t *points, int k, cluster_dist_t dist_func);

/* Integral image functions */
void imlib_integral_image_alloc(struct integral_image *sum, int w, int h);
void imlib_integral_image_free(struct integral_image *sum);
void imlib_integral_image(struct image *src, struct integral_image *sum);
void imlib_integral_image_sq(struct image *src, struct integral_image *sum);
void imlib_integral_image_scaled(struct image *src, struct integral_image *sum);
uint32_t imlib_integral_lookup(struct integral_image *src, int x, int y, int w, int h);

// Integral moving window
void imlib_integral_mw_alloc(mw_image_t *sum, int w, int h);
void imlib_integral_mw_free(mw_image_t *sum);
void imlib_integral_mw_scale(rectangle_t *roi, mw_image_t *sum, int w, int h);
void imlib_integral_mw(image_t *src, mw_image_t *sum);
void imlib_integral_mw_sq(image_t *src, mw_image_t *sum);
void imlib_integral_mw_shift(image_t *src, mw_image_t *sum, int n);
void imlib_integral_mw_shift_sq(image_t *src, mw_image_t *sum, int n);
void imlib_integral_mw_ss(image_t *src, mw_image_t *sum, mw_image_t *ssq, rectangle_t *roi);
void imlib_integral_mw_shift_ss(image_t *src, mw_image_t *sum, mw_image_t *ssq, rectangle_t *roi, int n);
long imlib_integral_mw_lookup(mw_image_t *sum, int x, int y, int w, int h);

/* Haar/VJ */
int imlib_load_cascade(struct cascade* cascade, const char *path);
array_t *imlib_detect_objects(struct image *image, struct cascade *cascade, struct rectangle *roi);

/* Corner detectors */
void fast_detect(image_t *image, array_t *keypoints, int threshold, rectangle_t *roi);
void agast_detect(image_t *image, array_t *keypoints, int threshold, rectangle_t *roi);

/* ORB descriptor */
array_t *orb_find_keypoints(image_t *image, bool normalized, int threshold,
        float scale_factor, int max_keypoints, corner_detector_t corner_detector, rectangle_t *roi);
int orb_match_keypoints(array_t *kpts1, array_t *kpts2, int *match, int threshold, rectangle_t *r, point_t *c, int *angle);
int orb_filter_keypoints(array_t *kpts, rectangle_t *r, point_t *c);
int orb_save_descriptor(FIL *fp, array_t *kpts);
int orb_load_descriptor(FIL *fp, array_t *kpts);
float orb_cluster_dist(int cx, int cy, void *kp);

/* LBP Operator */
uint8_t *imlib_lbp_desc(image_t *image, rectangle_t *roi);
int imlib_lbp_desc_distance(uint8_t *d0, uint8_t *d1);
int imlib_lbp_desc_save(FIL *fp, uint8_t *desc);
int imlib_lbp_desc_load(FIL *fp, uint8_t **desc);

/* Iris detector */
void imlib_find_iris(image_t *src, point_t *iris, rectangle_t *roi);

// Image filter functions
void im_filter_bw(uint8_t *src, uint8_t *dst, int size, int bpp, void *args);
void im_filter_skin(uint8_t *src, uint8_t *dst, int size, int bpp, void *args);

// Edge detection
void imlib_edge_simple(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh);
void imlib_edge_canny(image_t *src, rectangle_t *roi, int low_thresh, int high_thresh);

// HoG
void imlib_find_hog(image_t *src, rectangle_t *roi, int cell_size);

// Image Correction
void imlib_logpolar_int(image_t *dst, image_t *src, rectangle_t *roi, bool linear, bool reverse); // helper/internal
void imlib_logpolar(image_t *img, bool linear, bool reverse);
void imlib_chrominvar(image_t *img);
void imlib_illuminvar(image_t *img);
void imlib_histeq(image_t *img);
// Lens/Rotation Correction
void imlib_lens_corr(image_t *img, float strength, float zoom);
void imlib_rotation_corr(image_t *img, float x_rotation, float y_rotation,
                         float z_rotation, float x_translation, float y_translation,
                         float zoom);
// Statistics
void imlib_get_similarity(image_t *img, const char *path, image_t *other, float *avg, float *std, float *min, float *max);
void imlib_get_histogram(histogram_t *out, image_t *ptr, rectangle_t *roi);
void imlib_get_percentile(percentile_t *out, image_bpp_t bpp, histogram_t *ptr, float percentile);
void imlib_get_statistics(statistics_t *out, image_bpp_t bpp, histogram_t *ptr);
bool imlib_get_regression(find_lines_list_lnk_data_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                          list_t *thresholds, bool invert, bool robust);
// Color Tracking
void imlib_find_blobs(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                      list_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
                      bool merge, int margin,
                      bool (*threshold_cb)(void*,find_blobs_list_lnk_data_t*), void *threshold_cb_arg,
                      bool (*merge_cb)(void*,find_blobs_list_lnk_data_t*,find_blobs_list_lnk_data_t*), void *merge_cb_arg);
// Shape Detection
void pixel_magnitude(image_t *ptr, int x, int y, int *theta, uint32_t *mag); // helper/internal
size_t trace_line(image_t *ptr, line_t *l, int *theta_buffer, uint32_t *mag_buffer, point_t *point_buffer); // helper/internal
bool merge_line(line_t *big, line_t *small, unsigned int threshold); // helper/internal
void merge_alot(list_t *out, int threshold, int theta_threshold); // helper/internal
void imlib_find_lines(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                      uint32_t threshold, unsigned int theta_margin, unsigned int rho_margin);
void imlib_lsd_find_line_segments(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int merge_distance, unsigned int max_theta_diff);
void imlib_find_line_segments(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                              uint32_t threshold, unsigned int theta_margin, unsigned int rho_margin,
                              uint32_t segment_threshold);
void imlib_find_circles(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                        uint32_t threshold, unsigned int x_margin, unsigned int y_margin, unsigned int r_margin);
void imlib_find_rects(list_t *out, image_t *ptr, rectangle_t *roi,
                      uint32_t threshold);
// 1/2D Bar Codes
void imlib_find_qrcodes(list_t *out, image_t *ptr, rectangle_t *roi);
void imlib_find_apriltags(list_t *out, image_t *ptr, rectangle_t *roi, apriltag_families_t families,
                          float fx, float fy, float cx, float cy);
void imlib_find_datamatrices(list_t *out, image_t *ptr, rectangle_t *roi, int effort);
void imlib_find_barcodes(list_t *out, image_t *ptr, rectangle_t *roi);
// Template Matching
void imlib_phasecorrelate(image_t *img0, image_t *img1, rectangle_t *roi0, rectangle_t *roi1, bool logpolar, bool fix_rotation_scale,
                          float *x_translation, float *y_translation, float *rotation, float *scale, float *response);

// LeNet (CNN for character recognition)
#define LENGTH_KERNEL	5
#define LENGTH_FEATURE0	32
#define LENGTH_FEATURE1	(LENGTH_FEATURE0 - LENGTH_KERNEL + 1)
#define LENGTH_FEATURE2	(LENGTH_FEATURE1 >> 1)
#define LENGTH_FEATURE3	(LENGTH_FEATURE2 - LENGTH_KERNEL + 1)
#define	LENGTH_FEATURE4	(LENGTH_FEATURE3 >> 1)
#define LENGTH_FEATURE5	(LENGTH_FEATURE4 - LENGTH_KERNEL + 1)

#define INPUT			1
#define LAYER1			6
#define LAYER2			6
#define LAYER3			16
#define LAYER4			16
#define LAYER5			120

#define LENET_INPUT_W       (28)
#define LENET_INPUT_H       (28)
#define LENET_OUTPUT_SIZE   (10)
#define LENET_PADDING_SIZE  (2)
#define LENET_MODEL_SIZE    (51902)

typedef struct lenet5 {
    float weight0_1[INPUT][LAYER1][LENGTH_KERNEL][LENGTH_KERNEL];
    float weight2_3[LAYER2][LAYER3][LENGTH_KERNEL][LENGTH_KERNEL];
    float weight4_5[LAYER4][LAYER5][LENGTH_KERNEL][LENGTH_KERNEL];
    float weight5_6[LAYER5 * LENGTH_FEATURE5 * LENGTH_FEATURE5][LENET_OUTPUT_SIZE];

    float bias0_1[LAYER1];
    float bias2_3[LAYER3];
    float bias4_5[LAYER5];
    float bias5_6[LENET_OUTPUT_SIZE];

} lenet5_t;

typedef struct lenet5_feature {
    float input[INPUT][LENGTH_FEATURE0][LENGTH_FEATURE0];
    float layer1[LAYER1][LENGTH_FEATURE1][LENGTH_FEATURE1];
    float layer2[LAYER2][LENGTH_FEATURE2][LENGTH_FEATURE2];
    float layer3[LAYER3][LENGTH_FEATURE3][LENGTH_FEATURE3];
    float layer4[LAYER4][LENGTH_FEATURE4][LENGTH_FEATURE4];
    float layer5[LAYER5][LENGTH_FEATURE5][LENGTH_FEATURE5];
    float output[LENET_OUTPUT_SIZE];
} lenet5_feature_t;


// LeNet pretrained models.
extern const float lenet_model_num[LENET_MODEL_SIZE];

uint8_t lenet_predict(lenet5_t *lenet, image_t *src, rectangle_t *roi, float *conf);
#endif //__IMLIB_H__
