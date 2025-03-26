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
 * Image processing library.
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
#include "simd.h"
#include "fb_alloc.h"
#include "file_utils.h"
#include "umm_malloc.h"
#include "xalloc.h"
#include "array.h"
#include "fmath.h"
#include "collections.h"
#include "imlib_config.h"
#include "omv_boardconfig.h"
#include "omv_common.h"

// Enables 38 TensorFlow Lite operators.
#define IMLIB_TF_DEFAULT        (1)
// Enables 78 TensofFlow Lite operators.
#define IMLIB_TF_FULLOPS        (2)

#ifndef M_PI
#define M_PI                     3.14159265f
#define M_PI_2                   1.57079632f
#define M_PI_4                   0.78539816f
#endif

#define IM_LOG2_2(x)             (((x) & 0x2ULL) ? (2) :             1)                                // NO ({ ... }) !
#define IM_LOG2_4(x)             (((x) & 0xCULL) ? (2 + IM_LOG2_2((x) >> 2)) :  IM_LOG2_2(x))          // NO ({ ... }) !
#define IM_LOG2_8(x)             (((x) & 0xF0ULL) ? (4 + IM_LOG2_4((x) >> 4)) :  IM_LOG2_4(x))         // NO ({ ... }) !
#define IM_LOG2_16(x)            (((x) & 0xFF00ULL) ? (8 + IM_LOG2_8((x) >> 8)) :  IM_LOG2_8(x))       // NO ({ ... }) !
#define IM_LOG2_32(x)            (((x) & 0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)               (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define IM_IS_SIGNED(a)          (__builtin_types_compatible_p(__typeof__(a), signed) || \
                                  __builtin_types_compatible_p(__typeof__(a), signed long))
#define IM_IS_UNSIGNED(a)        (__builtin_types_compatible_p(__typeof__(a), unsigned) || \
                                  __builtin_types_compatible_p(__typeof__(a), unsigned long))
#define IM_SIGN_COMPARE(a, b)    ((IM_IS_SIGNED(a) && IM_IS_UNSIGNED(b)) || \
                                  (IM_IS_SIGNED(b) && IM_IS_UNSIGNED(a)))

#define IM_MAX(a, b)                                    \
    ({__typeof__ (a) _a = (a); __typeof__ (b) _b = (b); \
      __builtin_choose_expr(IM_SIGN_COMPARE(_a, _b), (void) 0, (_a > _b ? _a : _b)); })

#define IM_MIN(a, b)                                    \
    ({__typeof__ (a) _a = (a); __typeof__ (b) _b = (b); \
      __builtin_choose_expr(IM_SIGN_COMPARE(_a, _b), (void) 0, (_a < _b ? _a : _b)); })

#define IM_CLAMP(x, min, max)    IM_MAX(IM_MIN((x), (max)), (min))

#define IM_DIV(a, b)             ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define IM_MOD(a, b)             ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })

#define INT8_T_BITS              (sizeof(int8_t) * 8)
#define INT8_T_MASK              (INT8_T_BITS - 1)
#define INT8_T_SHIFT             IM_LOG2(INT8_T_MASK)

#define INT16_T_BITS             (sizeof(int16_t) * 8)
#define INT16_T_MASK             (INT16_T_BITS - 1)
#define INT16_T_SHIFT            IM_LOG2(INT16_T_MASK)

#define INT32_T_BITS             (sizeof(int32_t) * 8)
#define INT32_T_MASK             (INT32_T_BITS - 1)
#define INT32_T_SHIFT            IM_LOG2(INT32_T_MASK)

#define INT64_T_BITS             (sizeof(int64_t) * 8)
#define INT64_T_MASK             (INT64_T_BITS - 1)
#define INT64_T_SHIFT            IM_LOG2(INT64_T_MASK)

#define UINT8_T_BITS             (sizeof(uint8_t) * 8)
#define UINT8_T_MASK             (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT            IM_LOG2(UINT8_T_MASK)

#define UINT16_T_BITS            (sizeof(uint16_t) * 8)
#define UINT16_T_MASK            (UINT16_T_BITS - 1)
#define UINT16_T_SHIFT           IM_LOG2(UINT16_T_MASK)

#define UINT32_T_BITS            (sizeof(uint32_t) * 8)
#define UINT32_T_MASK            (UINT32_T_BITS - 1)
#define UINT32_T_SHIFT           IM_LOG2(UINT32_T_MASK)

#define UINT64_T_BITS            (sizeof(uint64_t) * 8)
#define UINT64_T_MASK            (UINT64_T_BITS - 1)
#define UINT64_T_SHIFT           IM_LOG2(UINT64_T_MASK)

#define IM_DEG2RAD(x)            (((x) * M_PI) / 180)
#define IM_RAD2DEG(x)            (((x) * 180) / M_PI)

int imlib_ksize_to_n(int ksize);

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
void point_rotate(int x, int y, float r, int center_x, int center_y, int16_t *new_x, int16_t *new_y);
void point_min_area_rectangle(point_t *corners, point_t *new_corners, int corners_len);

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

typedef struct bounding_box_lnk_data {
    rectangle_t rect;
    float score;
    int label_index;
} bounding_box_lnk_data_t;

void rectangle_init(rectangle_t *ptr, int x, int y, int w, int h);
void rectangle_copy(rectangle_t *dst, rectangle_t *src);
bool rectangle_equal_fast(rectangle_t *ptr0, rectangle_t *ptr1);
bool rectangle_overlap(rectangle_t *ptr0, rectangle_t *ptr1);
void rectangle_intersected(rectangle_t *dst, rectangle_t *src);
void rectangle_united(rectangle_t *dst, rectangle_t *src);
float rectangle_iou(rectangle_t *r1, rectangle_t *r2);
void rectangle_nms_add_bounding_box(list_t *bounding_boxes, bounding_box_lnk_data_t *box);
int rectangle_nms_get_bounding_boxes(list_t *bounding_boxes, float threshold, float sigma);
void rectangle_map_bounding_boxes(list_t *bounding_boxes, int window_w, int window_h, rectangle_t *roi);

/////////////////
// Color Stuff //
/////////////////

typedef struct color_thresholds_list_lnk_data {
    uint8_t LMin, LMax; // or grayscale
    int8_t AMin, AMax;
    int8_t BMin, BMax;
}
color_thresholds_list_lnk_data_t;

#define COLOR_THRESHOLD_BINARY(pixel, threshold, invert)                          \
    ({                                                                            \
        __typeof__ (pixel) _pixel = (pixel);                                      \
        __typeof__ (threshold) _threshold = (threshold);                          \
        __typeof__ (invert) _invert = (invert);                                   \
        ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
    })

#define COLOR_THRESHOLD_GRAYSCALE(pixel, threshold, invert)                       \
    ({                                                                            \
        __typeof__ (pixel) _pixel = (pixel);                                      \
        __typeof__ (threshold) _threshold = (threshold);                          \
        __typeof__ (invert) _invert = (invert);                                   \
        ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
    })

#define COLOR_THRESHOLD_RGB565(pixel, threshold, invert)                  \
    ({                                                                    \
        __typeof__ (pixel) _pixel = (pixel);                              \
        __typeof__ (threshold) _threshold = (threshold);                  \
        __typeof__ (invert) _invert = (invert);                           \
        uint8_t _l = COLOR_RGB565_TO_L(_pixel);                           \
        int8_t _a = COLOR_RGB565_TO_A(_pixel);                            \
        int8_t _b = COLOR_RGB565_TO_B(_pixel);                            \
        ((_threshold->LMin <= _l) && (_l <= _threshold->LMax) &&          \
         (_threshold->AMin <= _a) && (_a <= _threshold->AMax) &&          \
         (_threshold->BMin <= _b) && (_b <= _threshold->BMax)) ^ _invert; \
    })

#define COLOR_BOUND_BINARY(pixel0, pixel1, threshold)    \
    ({                                                   \
        __typeof__ (pixel0) _pixel0 = (pixel0);          \
        __typeof__ (pixel1) _pixel1 = (pixel1);          \
        __typeof__ (threshold) _threshold = (threshold); \
        (abs(_pixel0 - _pixel1) <= _threshold);          \
    })

#define COLOR_BOUND_GRAYSCALE(pixel0, pixel1, threshold) \
    ({                                                   \
        __typeof__ (pixel0) _pixel0 = (pixel0);          \
        __typeof__ (pixel1) _pixel1 = (pixel1);          \
        __typeof__ (threshold) _threshold = (threshold); \
        (abs(_pixel0 - _pixel1) <= _threshold);          \
    })

#define COLOR_BOUND_RGB565(pixel0, pixel1, threshold)                                                         \
    ({                                                                                                        \
        __typeof__ (pixel0) _pixel0 = (pixel0);                                                               \
        __typeof__ (pixel1) _pixel1 = (pixel1);                                                               \
        __typeof__ (threshold) _threshold = (threshold);                                                      \
        (abs(COLOR_RGB565_TO_R5(_pixel0) - COLOR_RGB565_TO_R5(_pixel1)) <= COLOR_RGB565_TO_R5(_threshold)) && \
        (abs(COLOR_RGB565_TO_G6(_pixel0) - COLOR_RGB565_TO_G6(_pixel1)) <= COLOR_RGB565_TO_G6(_threshold)) && \
        (abs(COLOR_RGB565_TO_B5(_pixel0) - COLOR_RGB565_TO_B5(_pixel1)) <= COLOR_RGB565_TO_B5(_threshold));   \
    })

#define COLOR_BINARY_MIN                        0
#define COLOR_BINARY_MAX                        1
#define COLOR_GRAYSCALE_BINARY_MIN              0x00
#define COLOR_GRAYSCALE_BINARY_MAX              0xFF
#define COLOR_RGB565_BINARY_MIN                 0x0000
#define COLOR_RGB565_BINARY_MAX                 0xFFFF

#define COLOR_GRAYSCALE_MIN                     0
#define COLOR_GRAYSCALE_MAX                     255

#define COLOR_R5_MIN                            0
#define COLOR_R5_MAX                            31
#define COLOR_G6_MIN                            0
#define COLOR_G6_MAX                            63
#define COLOR_B5_MIN                            0
#define COLOR_B5_MAX                            31

#define COLOR_R8_MIN                            0
#define COLOR_R8_MAX                            255
#define COLOR_G8_MIN                            0
#define COLOR_G8_MAX                            255
#define COLOR_B8_MIN                            0
#define COLOR_B8_MAX                            255

#define COLOR_L_MIN                             0
#define COLOR_L_MAX                             100
#define COLOR_A_MIN                             -128
#define COLOR_A_MAX                             127
#define COLOR_B_MIN                             -128
#define COLOR_B_MAX                             127

#define COLOR_Y_MIN                             0
#define COLOR_Y_MAX                             255
#define COLOR_U_MIN                             -128
#define COLOR_U_MAX                             127
#define COLOR_V_MIN                             -128
#define COLOR_V_MAX                             127

// RGB565 Stuff //

#define COLOR_RGB565_TO_R5(pixel)               (((pixel) >> 11) & 0x1F)
#define COLOR_RGB565_TO_R8(pixel)             \
    ({                                        \
        __typeof__ (pixel) __pixel = (pixel); \
        __pixel = (__pixel >> 8) & 0xF8;      \
        __pixel | (__pixel >> 5);             \
    })

#define COLOR_RGB565_TO_G6(pixel)               (((pixel) >> 5) & 0x3F)
#define COLOR_RGB565_TO_G8(pixel)             \
    ({                                        \
        __typeof__ (pixel) __pixel = (pixel); \
        __pixel = (__pixel >> 3) & 0xFC;      \
        __pixel | (__pixel >> 6);             \
    })

#define COLOR_RGB565_TO_B5(pixel)               ((pixel) & 0x1F)
#define COLOR_RGB565_TO_B8(pixel)             \
    ({                                        \
        __typeof__ (pixel) __pixel = (pixel); \
        __pixel = (__pixel << 3) & 0xF8;      \
        __pixel | (__pixel >> 5);             \
    })

#define COLOR_R5_G6_B5_TO_RGB565(r5, g6, b5)    (((r5) << 11) | ((g6) << 5) | (b5))
#define COLOR_R8_G8_B8_TO_RGB565(r8, g8, b8)    ((((r8) & 0xF8) << 8) | (((g8) & 0xFC) << 3) | ((b8) >> 3))

#define COLOR_RGB888_TO_Y(r8, g8, b8)           ((((r8) * 38) + ((g8) * 75) + ((b8) * 15)) >> 7) // 0.299R + 0.587G + 0.114B
#define COLOR_RGB565_TO_Y(rgb565)                \
    ({                                           \
        __typeof__ (rgb565) __rgb565 = (rgb565); \
        int r = COLOR_RGB565_TO_R8(__rgb565);    \
        int g = COLOR_RGB565_TO_G8(__rgb565);    \
        int b = COLOR_RGB565_TO_B8(__rgb565);    \
        COLOR_RGB888_TO_Y(r, g, b);              \
    })

#define COLOR_Y_TO_RGB888(pixel)                ((pixel) * 0x010101)
#define COLOR_Y_TO_RGB565(pixel)                          \
    ({                                                    \
        __typeof__ (pixel) __pixel = (pixel);             \
        int __rb_pixel = (__pixel >> 3) & 0x1F;           \
        (__rb_pixel * 0x0801) + ((__pixel << 3) & 0x7E0); \
    })

#define COLOR_RGB888_TO_U(r8, g8, b8)           ((((r8) * -21) - ((g8) * 43) + ((b8) * 64)) >> 7) // -0.168736R - 0.331264G + 0.5B
#define COLOR_RGB565_TO_U(rgb565)                \
    ({                                           \
        __typeof__ (rgb565) __rgb565 = (rgb565); \
        int r = COLOR_RGB565_TO_R8(__rgb565);    \
        int g = COLOR_RGB565_TO_G8(__rgb565);    \
        int b = COLOR_RGB565_TO_B8(__rgb565);    \
        COLOR_RGB888_TO_U(r, g, b);              \
    })

#define COLOR_RGB888_TO_V(r8, g8, b8)           ((((r8) * 64) - ((g8) * 54) - ((b8) * 10)) >> 7) // 0.5R - 0.418688G - 0.081312B
#define COLOR_RGB565_TO_V(rgb565)                \
    ({                                           \
        __typeof__ (rgb565) __rgb565 = (rgb565); \
        int r = COLOR_RGB565_TO_R8(__rgb565);    \
        int g = COLOR_RGB565_TO_G8(__rgb565);    \
        int b = COLOR_RGB565_TO_B8(__rgb565);    \
        COLOR_RGB888_TO_V(r, g, b);              \
    })

extern const int8_t lab_table[196608 / 2];

#ifdef IMLIB_ENABLE_LAB_LUT
#define COLOR_RGB565_TO_L(pixel)                lab_table[((pixel >> 1) * 3) + 0]
#define COLOR_RGB565_TO_A(pixel)                lab_table[((pixel >> 1) * 3) + 1]
#define COLOR_RGB565_TO_B(pixel)                lab_table[((pixel >> 1) * 3) + 2]
#else
#define COLOR_RGB565_TO_L(pixel)                imlib_rgb565_to_l(pixel)
#define COLOR_RGB565_TO_A(pixel)                imlib_rgb565_to_a(pixel)
#define COLOR_RGB565_TO_B(pixel)                imlib_rgb565_to_b(pixel)
#endif

#define COLOR_LAB_TO_RGB565(l, a, b)            imlib_lab_to_rgb(l, a, b)
#define COLOR_YUV_TO_RGB565(y, u, v)            imlib_yuv_to_rgb((y) + 128, u, v)

#define COLOR_BINARY_TO_GRAYSCALE(pixel)        ((pixel) * COLOR_GRAYSCALE_MAX)
#define COLOR_BINARY_TO_RGB565(pixel)           COLOR_YUV_TO_RGB565(((pixel) ? 127 : -128), 0, 0)
#define COLOR_RGB565_TO_BINARY(pixel)           (COLOR_RGB565_TO_Y(pixel) > (((COLOR_Y_MAX - COLOR_Y_MIN) / 2) + COLOR_Y_MIN))
#define COLOR_RGB565_TO_GRAYSCALE(pixel)        COLOR_RGB565_TO_Y(pixel)
#define COLOR_GRAYSCALE_TO_BINARY(pixel)        ((pixel) > \
                                                 (((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN) / 2) + COLOR_GRAYSCALE_MIN))
#define COLOR_GRAYSCALE_TO_RGB565(pixel)        COLOR_YUV_TO_RGB565(((pixel) - 128), 0, 0)

typedef enum {
    COLOR_PALETTE_RAINBOW,
    COLOR_PALETTE_IRONBOW,
    COLOR_PALETTE_DEPTH,
    COLOR_PALETTE_EVT_DARK,
    COLOR_PALETTE_EVT_LIGHT
} color_palette_t;

// Color palette LUTs
extern const uint16_t rainbow_table[256];
extern const uint16_t ironbow_table[256];
extern const uint16_t depth_table[256];
extern const uint16_t evt_dark_table[256];
extern const uint16_t evt_light_table[256];

/////////////////
// Image Stuff //
/////////////////

// Pixel format IDs.
typedef enum {
    PIXFORMAT_ID_BINARY = 1,
    PIXFORMAT_ID_GRAY   = 2,
    PIXFORMAT_ID_RGB565 = 3,
    PIXFORMAT_ID_BAYER  = 4,
    PIXFORMAT_ID_YUV422 = 5,
    PIXFORMAT_ID_JPEG   = 6,
    PIXFORMAT_ID_PNG    = 7,
    PIXFORMAT_ID_ARGB8  = 8,
    /* Note: Update PIXFORMAT_IS_VALID when adding new formats */
} pixformat_id_t;

// Pixel sub-format IDs.
// YUV422 matches the YUV pattern of YUYV...
// YVU422 matches the YUV pattern of YVYU...
//
// BGGR matches the bayer pattern of BGBG...
//                                   GRGR...
// GBRG matches the bayer pattern of GBGB...
//                                   RGRG...
// GRBG matches the bayer pattern of GRGR...
//                                   BGBG...
// RGGB matches the bayer pattern of RGRG...
// Note: The bayer sub-format must not overflow csi.cfa_format.
typedef enum {
    SUBFORMAT_ID_GRAY8  = 0,
    SUBFORMAT_ID_GRAY16 = 1,
    SUBFORMAT_ID_BGGR   = 0,
    SUBFORMAT_ID_GBRG   = 1,
    SUBFORMAT_ID_GRBG   = 2,
    SUBFORMAT_ID_RGGB   = 3,
    SUBFORMAT_ID_YUV422 = 0,
    SUBFORMAT_ID_YVU422 = 1,
    /* Note: Update PIXFORMAT_IS_VALID when adding new formats */
} subformat_id_t;

// Pixel format Byte Per Pixel.
typedef enum {
    PIXFORMAT_BPP_BINARY = 0,
    PIXFORMAT_BPP_GRAY8  = 1,
    PIXFORMAT_BPP_GRAY16 = 2,
    PIXFORMAT_BPP_RGB565 = 2,
    PIXFORMAT_BPP_BAYER  = 1,
    PIXFORMAT_BPP_YUV422 = 2,
    PIXFORMAT_BPP_ARGB8  = 4,
    /* Note: Update PIXFORMAT_IS_VALID when adding new formats */
} pixformat_bpp_t;

// Pixel format flags.
#define PIXFORMAT_FLAGS_Y          (1 << 28) // YUV format.
#define PIXFORMAT_FLAGS_M          (1 << 27) // Mutable format.
#define PIXFORMAT_FLAGS_C          (1 << 26) // Colored format.
#define PIXFORMAT_FLAGS_J          (1 << 25) // Compressed format (JPEG/PNG).
#define PIXFORMAT_FLAGS_R          (1 << 24) // RAW/Bayer format.
#define PIXFORMAT_FLAGS_CY         (PIXFORMAT_FLAGS_C | PIXFORMAT_FLAGS_Y)
#define PIXFORMAT_FLAGS_CM         (PIXFORMAT_FLAGS_C | PIXFORMAT_FLAGS_M)
#define PIXFORMAT_FLAGS_CR         (PIXFORMAT_FLAGS_C | PIXFORMAT_FLAGS_R)
#define PIXFORMAT_FLAGS_CJ         (PIXFORMAT_FLAGS_C | PIXFORMAT_FLAGS_J)
#define IMLIB_IMAGE_MAX_SIZE(x)    ((x) & 0xFFFFFFFF)

// *INDENT-OFF*
// Each pixel format encodes flags, pixel format id and bpp as follows:
// 31......29  28  27  26  25  24  23..........16  15...........8  7.............0
// <RESERVED>  YF  MF  CF  JF  RF  <PIXFORMAT_ID>  <SUBFORMAT_ID>  <BYTES_PER_PIX>
// NOTE: Bit 31-30 must Not be used for pixformat_t to be used as mp_int_t.
typedef enum {
  PIXFORMAT_INVALID    = (0x00000000U),
  PIXFORMAT_BINARY     = (PIXFORMAT_FLAGS_M  | (PIXFORMAT_ID_BINARY << 16) | (0                   << 8) | PIXFORMAT_BPP_BINARY ),
  PIXFORMAT_GRAYSCALE  = (PIXFORMAT_FLAGS_M  | (PIXFORMAT_ID_GRAY   << 16) | (SUBFORMAT_ID_GRAY8  << 8) | PIXFORMAT_BPP_GRAY8  ),
  PIXFORMAT_RGB565     = (PIXFORMAT_FLAGS_CM | (PIXFORMAT_ID_RGB565 << 16) | (0                   << 8) | PIXFORMAT_BPP_RGB565 ),
  PIXFORMAT_ARGB8      = (PIXFORMAT_FLAGS_CM | (PIXFORMAT_ID_ARGB8  << 16) | (0                   << 8) | PIXFORMAT_BPP_ARGB8  ),
  PIXFORMAT_BAYER      = (PIXFORMAT_FLAGS_CR | (PIXFORMAT_ID_BAYER  << 16) | (SUBFORMAT_ID_BGGR   << 8) | PIXFORMAT_BPP_BAYER  ),
  PIXFORMAT_BAYER_BGGR = (PIXFORMAT_FLAGS_CR | (PIXFORMAT_ID_BAYER  << 16) | (SUBFORMAT_ID_BGGR   << 8) | PIXFORMAT_BPP_BAYER  ),
  PIXFORMAT_BAYER_GBRG = (PIXFORMAT_FLAGS_CR | (PIXFORMAT_ID_BAYER  << 16) | (SUBFORMAT_ID_GBRG   << 8) | PIXFORMAT_BPP_BAYER  ),
  PIXFORMAT_BAYER_GRBG = (PIXFORMAT_FLAGS_CR | (PIXFORMAT_ID_BAYER  << 16) | (SUBFORMAT_ID_GRBG   << 8) | PIXFORMAT_BPP_BAYER  ),
  PIXFORMAT_BAYER_RGGB = (PIXFORMAT_FLAGS_CR | (PIXFORMAT_ID_BAYER  << 16) | (SUBFORMAT_ID_RGGB   << 8) | PIXFORMAT_BPP_BAYER  ),
  PIXFORMAT_YUV        = (PIXFORMAT_FLAGS_CY | (PIXFORMAT_ID_YUV422 << 16) | (SUBFORMAT_ID_YUV422 << 8) | PIXFORMAT_BPP_YUV422 ),
  PIXFORMAT_YUV422     = (PIXFORMAT_FLAGS_CY | (PIXFORMAT_ID_YUV422 << 16) | (SUBFORMAT_ID_YUV422 << 8) | PIXFORMAT_BPP_YUV422 ),
  PIXFORMAT_YVU422     = (PIXFORMAT_FLAGS_CY | (PIXFORMAT_ID_YUV422 << 16) | (SUBFORMAT_ID_YVU422 << 8) | PIXFORMAT_BPP_YUV422 ),
  PIXFORMAT_JPEG       = (PIXFORMAT_FLAGS_CJ | (PIXFORMAT_ID_JPEG   << 16) | (0                   << 8) | 0                    ),
  PIXFORMAT_PNG        = (PIXFORMAT_FLAGS_CJ | (PIXFORMAT_ID_PNG    << 16) | (0                   << 8) | 0                    ),
  PIXFORMAT_LAST       = (0xFFFFFFFFU),
} pixformat_t;
// *INDENT-ON*

#define PIXFORMAT_MUTABLE_ANY \
        PIXFORMAT_BINARY:     \
    case PIXFORMAT_GRAYSCALE: \
    case PIXFORMAT_RGB565:    \
    case PIXFORMAT_ARGB8      \

#define PIXFORMAT_BAYER_ANY    \
    PIXFORMAT_BAYER_BGGR:      \
    case PIXFORMAT_BAYER_GBRG: \
    case PIXFORMAT_BAYER_GRBG: \
    case PIXFORMAT_BAYER_RGGB  \

#define PIXFORMAT_YUV_ANY \
    PIXFORMAT_YUV422:     \
    case PIXFORMAT_YVU422 \

#define PIXFORMAT_COMPRESSED_ANY \
    PIXFORMAT_JPEG:              \
    case PIXFORMAT_PNG           \

#define IMLIB_PIXFORMAT_IS_VALID(x) \
    ((x == PIXFORMAT_BINARY)        \
     || (x == PIXFORMAT_GRAYSCALE)  \
     || (x == PIXFORMAT_RGB565)     \
     || (x == PIXFORMAT_ARGB8)      \
     || (x == PIXFORMAT_BAYER_BGGR) \
     || (x == PIXFORMAT_BAYER_GBRG) \
     || (x == PIXFORMAT_BAYER_GRBG) \
     || (x == PIXFORMAT_BAYER_RGGB) \
     || (x == PIXFORMAT_YUV422)     \
     || (x == PIXFORMAT_YVU422)     \
     || (x == PIXFORMAT_JPEG)       \
     || (x == PIXFORMAT_PNG))       \

// *INDENT-OFF*
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define PIXFORMAT_STRUCT            \
struct {                            \
  union {                           \
    struct {                        \
        uint32_t bpp            :8; \
        uint32_t subfmt_id      :8; \
        uint32_t pixfmt_id      :8; \
        uint32_t is_bayer       :1; \
        uint32_t is_compressed  :1; \
        uint32_t is_color       :1; \
        uint32_t is_mutable     :1; \
        uint32_t is_yuv         :1; \
        uint32_t /*reserved*/   :3; \
    };                              \
    uint32_t pixfmt;                \
  };                                \
  uint32_t size; /* for compressed images */ \
}
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define PIXFORMAT_STRUCT            \
struct {                            \
  union {                           \
    struct {                        \
        uint32_t /*reserved*/   :3; \
        uint32_t is_yuv         :1; \
        uint32_t is_mutable     :1; \
        uint32_t is_color       :1; \
        uint32_t is_compressed  :1; \
        uint32_t is_bayer       :1; \
        uint32_t pixfmt_id      :8; \
        uint32_t subfmt_id      :8; \
        uint32_t bpp            :8; \
    };                              \
    uint32_t pixfmt;                \
  };                                \
  uint32_t size; /* for compressed images */ \
}
#else
#error "Byte order is not defined."
#endif
// *INDENT-ON*

typedef struct image {
    int32_t w;
    int32_t h;
    PIXFORMAT_STRUCT;
    // Keeps a reference to the GC block when used with image_xalloc/image_xalloc0.
    uint8_t *_raw;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
} image_t;

void image_xalloc(image_t *img, size_t size);
void image_xalloc0(image_t *img, size_t size);
void image_init(image_t *ptr, int w, int h, pixformat_t pixfmt, uint32_t size, void *pixels);
void image_copy(image_t *dst, image_t *src);
size_t image_line_size(image_t *ptr);
size_t image_size(image_t *ptr);
bool image_get_mask_pixel(image_t *ptr, int x, int y);

#define IMAGE_BINARY_LINE_LEN(image)             (((image)->w + UINT32_T_MASK) >> UINT32_T_SHIFT)
#define IMAGE_BINARY_LINE_LEN_BYTES(image)       (IMAGE_BINARY_LINE_LEN(image) * sizeof(uint32_t))

#define IMAGE_GRAYSCALE_LINE_LEN(image)          ((image)->w)
#define IMAGE_GRAYSCALE_LINE_LEN_BYTES(image)    (IMAGE_GRAYSCALE_LINE_LEN(image) * sizeof(uint8_t))

#define IMAGE_RGB565_LINE_LEN(image)             ((image)->w)
#define IMAGE_RGB565_LINE_LEN_BYTES(image)       (IMAGE_RGB565_LINE_LEN(image) * sizeof(uint16_t))

#define IMAGE_GET_BINARY_PIXEL(image, x, y)                                                                              \
    ({                                                                                                                   \
        __typeof__ (image) _image = (image);                                                                             \
        __typeof__ (x) _x = (x);                                                                                         \
        __typeof__ (y) _y = (y);                                                                                         \
        (((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] >> \
         (_x & UINT32_T_MASK)) & 1;                                                                                      \
    })

#define IMAGE_PUT_BINARY_PIXEL(image, x, y, v)                                                                 \
    ({                                                                                                         \
        __typeof__ (image) _image = (image);                                                                   \
        __typeof__ (x) _x = (x);                                                                               \
        __typeof__ (y) _y = (y);                                                                               \
        __typeof__ (v) _v = (v);                                                                               \
        size_t _i = (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT);           \
        size_t _j = _x & UINT32_T_MASK;                                                                        \
        ((uint32_t *) _image->data)[_i] = (((uint32_t *) _image->data)[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
    })

#define IMAGE_CLEAR_BINARY_PIXEL(image, x, y)                                                                          \
    ({                                                                                                                 \
        __typeof__ (image) _image = (image);                                                                           \
        __typeof__ (x) _x = (x);                                                                                       \
        __typeof__ (y) _y = (y);                                                                                       \
        ((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >>                                                   \
                                      UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] &= ~(1 << (_x & UINT32_T_MASK)); \
    })

#define IMAGE_SET_BINARY_PIXEL(image, x, y)                                                                                              \
    ({                                                                                                                                   \
        __typeof__ (image) _image = (image);                                                                                             \
        __typeof__ (x) _x = (x);                                                                                                         \
        __typeof__ (y) _y = (y);                                                                                                         \
        ((uint32_t *) _image->data)[(((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y) + (_x >> UINT32_T_SHIFT)] |= 1 <<             \
                                                                                                                        (_x &            \
                                                                                                                         UINT32_T_MASK); \
    })

#define IMAGE_GET_GRAYSCALE_PIXEL(image, x, y)             \
    ({                                                     \
        __typeof__ (image) _image = (image);               \
        __typeof__ (x) _x = (x);                           \
        __typeof__ (y) _y = (y);                           \
        ((uint8_t *) _image->data)[(_image->w * _y) + _x]; \
    })

#define IMAGE_PUT_GRAYSCALE_PIXEL(image, x, y, v)               \
    ({                                                          \
        __typeof__ (image) _image = (image);                    \
        __typeof__ (x) _x = (x);                                \
        __typeof__ (y) _y = (y);                                \
        __typeof__ (v) _v = (v);                                \
        ((uint8_t *) _image->data)[(_image->w * _y) + _x] = _v; \
    })

#define IMAGE_GET_RGB565_PIXEL(image, x, y)                 \
    ({                                                      \
        __typeof__ (image) _image = (image);                \
        __typeof__ (x) _x = (x);                            \
        __typeof__ (y) _y = (y);                            \
        ((uint16_t *) _image->data)[(_image->w * _y) + _x]; \
    })

#define IMAGE_PUT_RGB565_PIXEL(image, x, y, v)                   \
    ({                                                           \
        __typeof__ (image) _image = (image);                     \
        __typeof__ (x) _x = (x);                                 \
        __typeof__ (y) _y = (y);                                 \
        __typeof__ (v) _v = (v);                                 \
        ((uint16_t *) _image->data)[(_image->w * _y) + _x] = _v; \
    })

#define IMAGE_GET_YUV_PIXEL(image, x, y)                    \
    ({                                                      \
        __typeof__ (image) _image = (image);                \
        __typeof__ (x) _x = (x);                            \
        __typeof__ (y) _y = (y);                            \
        ((uint16_t *) _image->data)[(_image->w * _y) + _x]; \
    })

#define IMAGE_PUT_YUV_PIXEL(image, x, y, v)                      \
    ({                                                           \
        __typeof__ (image) _image = (image);                     \
        __typeof__ (x) _x = (x);                                 \
        __typeof__ (y) _y = (y);                                 \
        __typeof__ (v) _v = (v);                                 \
        ((uint16_t *) _image->data)[(_image->w * _y) + _x] = _v; \
    })

#define IMAGE_GET_BAYER_PIXEL(image, x, y)                 \
    ({                                                     \
        __typeof__ (image) _image = (image);               \
        __typeof__ (x) _x = (x);                           \
        __typeof__ (y) _y = (y);                           \
        ((uint8_t *) _image->data)[(_image->w * _y) + _x]; \
    })

#define IMAGE_PUT_BAYER_PIXEL(image, x, y, v)                   \
    ({                                                          \
        __typeof__ (image) _image = (image);                    \
        __typeof__ (x) _x = (x);                                \
        __typeof__ (y) _y = (y);                                \
        __typeof__ (v) _v = (v);                                \
        ((uint8_t *) _image->data)[(_image->w * _y) + _x] = _v; \
    })

// Fast Stuff //

#define IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(image, y)                                          \
    ({                                                                                        \
        __typeof__ (image) _image = (image);                                                  \
        __typeof__ (y) _y = (y);                                                              \
        ((uint32_t *) _image->data) + (((_image->w + UINT32_T_MASK) >> UINT32_T_SHIFT) * _y); \
    })

#define IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)                       \
    ({                                                                \
        __typeof__ (row_ptr) _row_ptr = (row_ptr);                    \
        __typeof__ (x) _x = (x);                                      \
        (_row_ptr[_x >> UINT32_T_SHIFT] >> (_x & UINT32_T_MASK)) & 1; \
    })

#define IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x, v)                       \
    ({                                                                   \
        __typeof__ (row_ptr) _row_ptr = (row_ptr);                       \
        __typeof__ (x) _x = (x);                                         \
        __typeof__ (v) _v = (v);                                         \
        size_t _i = _x >> UINT32_T_SHIFT;                                \
        size_t _j = _x & UINT32_T_MASK;                                  \
        _row_ptr[_i] = (_row_ptr[_i] & (~(1 << _j))) | ((_v & 1) << _j); \
    })

#define IMAGE_CLEAR_BINARY_PIXEL_FAST(row_ptr, x)                       \
    ({                                                                  \
        __typeof__ (row_ptr) _row_ptr = (row_ptr);                      \
        __typeof__ (x) _x = (x);                                        \
        _row_ptr[_x >> UINT32_T_SHIFT] &= ~(1 << (_x & UINT32_T_MASK)); \
    })

#define IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x)                      \
    ({                                                               \
        __typeof__ (row_ptr) _row_ptr = (row_ptr);                   \
        __typeof__ (x) _x = (x);                                     \
        _row_ptr[_x >> UINT32_T_SHIFT] |= 1 << (_x & UINT32_T_MASK); \
    })

#define IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(image, y) \
    ({                                                  \
        __typeof__ (image) _image = (image);            \
        __typeof__ (y) _y = (y);                        \
        ((uint8_t *) _image->data) + (_image->w * _y);  \
    })

#define IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) \
    ({                                             \
        __typeof__ (row_ptr) _row_ptr = (row_ptr); \
        __typeof__ (x) _x = (x);                   \
        _row_ptr[_x];                              \
    })

#define IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x, v) \
    ({                                                \
        __typeof__ (row_ptr) _row_ptr = (row_ptr);    \
        __typeof__ (x) _x = (x);                      \
        __typeof__ (v) _v = (v);                      \
        _row_ptr[_x] = _v;                            \
    })

#define IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(image, y)    \
    ({                                                  \
        __typeof__ (image) _image = (image);            \
        __typeof__ (y) _y = (y);                        \
        ((uint16_t *) _image->data) + (_image->w * _y); \
    })

#define IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)    \
    ({                                             \
        __typeof__ (row_ptr) _row_ptr = (row_ptr); \
        __typeof__ (x) _x = (x);                   \
        _row_ptr[_x];                              \
    })

#define IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, v) \
    ({                                             \
        __typeof__ (row_ptr) _row_ptr = (row_ptr); \
        __typeof__ (x) _x = (x);                   \
        __typeof__ (v) _v = (v);                   \
        _row_ptr[_x] = _v;                         \
    })

#define IMAGE_COMPUTE_BAYER_PIXEL_ROW_PTR(image, y)    \
    ({                                                 \
        __typeof__ (image) _image = (image);           \
        __typeof__ (y) _y = (y);                       \
        ((uint8_t *) _image->data) + (_image->w * _y); \
    })

#define IMAGE_COMPUTE_YUV_PIXEL_ROW_PTR(image, y)       \
    ({                                                  \
        __typeof__ (image) _image = (image);            \
        __typeof__ (y) _y = (y);                        \
        ((uint16_t *) _image->data) + (_image->w * _y); \
    })

static inline void linebuf_copy_binary(int32_t y, int32_t ksize, int32_t brows, image_t *buf, image_t *dst) {
    if (y >= ksize) {
        // Transfer buffer lines...
        int32_t y_offset = y - ksize;
        vmemcpy_32(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(dst, y_offset),
                   IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(buf, (y_offset % brows)),
                   IMAGE_BINARY_LINE_LEN_BYTES(dst));
    }
}

static inline void linebuf_copy_binary_tail(int32_t ksize, int32_t brows, image_t *buf, image_t *dst) {
    // Copy any remaining lines from the buffer image...
    for (int32_t y = IM_MAX(dst->h - ksize, 0); y < dst->h; y++) {
        vmemcpy_32(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(dst, y),
                   IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(buf, (y % brows)),
                   IMAGE_BINARY_LINE_LEN_BYTES(dst));
    }
}

static inline void linebuf_copy_grayscale(int32_t y, int32_t ksize, int32_t brows, image_t *buf, image_t *dst) {
    if (y >= ksize) {
        // Transfer buffer lines...
        int32_t y_offset = y - ksize;
        vmemcpy_8(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(dst, y_offset),
                  IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(buf, (y_offset % brows)),
                  IMAGE_GRAYSCALE_LINE_LEN_BYTES(dst));
    }
}

static inline void linebuf_copy_grayscale_tail(int32_t ksize, int32_t brows, image_t *buf, image_t *dst) {
    // Copy any remaining lines from the buffer image...
    for (int32_t y = IM_MAX(dst->h - ksize, 0); y < dst->h; y++) {
        vmemcpy_8(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(dst, y),
                  IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(buf, (y % brows)),
                  IMAGE_GRAYSCALE_LINE_LEN_BYTES(dst));
    }
}

static inline void linebuf_copy_rgb565(int32_t y, int32_t ksize, int32_t brows, image_t *buf, image_t *dst) {
    if (y >= ksize) {
        // Transfer buffer lines...
        int32_t y_offset = y - ksize;
        vmemcpy_16(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(dst, y_offset),
                   IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(buf, (y_offset % brows)),
                   IMAGE_RGB565_LINE_LEN_BYTES(dst));
    }
}

static inline void linebuf_copy_rgb565_tail(int32_t ksize, int32_t brows, image_t *buf, image_t *dst) {
    // Copy any remaining lines from the buffer image...
    for (int32_t y = IM_MAX(dst->h - ksize, 0); y < dst->h; y++) {
        vmemcpy_16(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(dst, y),
                   IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(buf, (y % brows)),
                   IMAGE_RGB565_LINE_LEN_BYTES(dst));
    }
}

////////////////
// JPEG Stuff //
////////////////

#define JPEG_MCU_W                 (8)
#define JPEG_MCU_H                 (8)
#define JPEG_444_GS_MCU_SIZE       ((JPEG_MCU_W) *(JPEG_MCU_H))
#define JPEG_444_YCBCR_MCU_SIZE    ((JPEG_444_GS_MCU_SIZE) * 3)
#define JPEG_422_YCBCR_MCU_SIZE    ((JPEG_444_GS_MCU_SIZE) * 4)
#define JPEG_420_YCBCR_MCU_SIZE    ((JPEG_444_GS_MCU_SIZE) * 6)

typedef enum jpeg_subsampling {
    JPEG_SUBSAMPLING_AUTO = 0,
    JPEG_SUBSAMPLING_444  = 0x11, // Chroma subsampling 4:4:4 (No subsampling)
    JPEG_SUBSAMPLING_422  = 0x21, // Chroma subsampling 4:2:2
    JPEG_SUBSAMPLING_420  = 0x22, // Chroma subsampling 4:2:0
} jpeg_subsampling_t;

// Old Image Macros - Will be refactor and removed. But, only after making sure through testing new macros work.

// Image kernels
extern const int8_t kernel_gauss_3[9];
extern const int8_t kernel_gauss_5[25];
extern const int kernel_laplacian_3[9];
extern const int kernel_high_pass_3[9];

// Grayscale maxes
#define IM_MAX_GS            (255)

#define IM_IS_BINARY(img)    ((img)->pixfmt == PIXFORMAT_BINARY)
#define IM_IS_GS(img)        ((img)->pixfmt == PIXFORMAT_GRAYSCALE)
#define IM_IS_RGB565(img)    ((img)->pixfmt == PIXFORMAT_RGB565)
#define IM_IS_BAYER(img)     ((img)->is_bayer)
#define IM_IS_JPEG(img)      ((img)->pixfmt == PIXFORMAT_JPEG)

#define IM_X_INSIDE(img, x)           \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x);       \
       (0 <= _x) && (_x < _img->w); })

#define IM_Y_INSIDE(img, y)           \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (y) _y = (y);       \
       (0 <= _y) && (_y < _img->h); })

#define IM_GET_GS_PIXEL(img, x, y)    \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x);       \
       __typeof__ (y) _y = (y);       \
       ((uint8_t *) _img->pixels)[(_y * _img->w) + _x]; })

#define IM_GET_RGB565_PIXEL(img, x, y) \
    ({ __typeof__ (img) _img = (img);  \
       __typeof__ (x) _x = (x);        \
       __typeof__ (y) _y = (y);        \
       ((uint16_t *) _img->pixels)[(_y * _img->w) + _x]; })

#define IM_SET_GS_PIXEL(img, x, y, p) \
    ({ __typeof__ (img) _img = (img); \
       __typeof__ (x) _x = (x);       \
       __typeof__ (y) _y = (y);       \
       __typeof__ (p) _p = (p);       \
       ((uint8_t *) _img->pixels)[(_y * _img->w) + _x] = _p; })

#define IM_SET_RGB565_PIXEL(img, x, y, p) \
    ({ __typeof__ (img) _img = (img);     \
       __typeof__ (x) _x = (x);           \
       __typeof__ (y) _y = (y);           \
       __typeof__ (p) _p = (p);           \
       ((uint16_t *) _img->pixels)[(_y * _img->w) + _x] = _p; })

#define IM_EQUAL(img0, img1)             \
    ({ __typeof__ (img0) _img0 = (img0); \
       __typeof__ (img1) _img1 = (img1); \
       (_img0->w == _img1->w) && (_img0->h == _img1->h) && (_img0->pixfmt = _img1->pixfmt); })

#define IM_TO_GS_PIXEL(img, x, y) \
    (img->bpp == 1 ? img->pixels[((y) * img->w) + (x)] : COLOR_RGB565_TO_Y(((uint16_t *) img->pixels)[((y) * img->w) + (x)]) )

typedef struct simple_color {
    uint8_t G;          // Gray
    union {
        int8_t L;       // LAB L
        uint8_t red;    // RGB888 Red
    };
    union {
        int8_t A;       // LAB A
        uint8_t green;  // RGB888 Green
    };
    union {
        int8_t B;       // LAB B
        uint8_t blue;   // RGB888 Blue
    };
} simple_color_t;

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
    uint16_t cx, cy;
} vec_t;

typedef struct cluster {
    int x, y, w, h;
    array_t *points;
} cluster_t;

// Return the distance between a cluster centroid and some object.
typedef float (*cluster_dist_t) (int cx, int cy, void *obj);

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

typedef struct jpg_read_settings {
    int32_t jpg_w;
    int32_t jpg_h;
    int32_t jpg_size;
} jpg_read_settings_t;

typedef struct png_read_settings {
    int32_t png_w;
    int32_t png_h;
    int32_t png_size;
} png_read_settings_t;

typedef enum save_image_format {
    FORMAT_DONT_CARE,
    FORMAT_BMP,
    FORMAT_PNM,
    FORMAT_JPG,
    FORMAT_PNG,
    FORMAT_RAW,
} save_image_format_t;

typedef struct img_read_settings {
    union {
        bmp_read_settings_t bmp_rs;
        ppm_read_settings_t ppm_rs;
        jpg_read_settings_t jpg_rs;
        png_read_settings_t png_rs;
    };
    save_image_format_t format;
} img_read_settings_t;

typedef void (*binary_morph_op_t) (image_t *, int, int, image_t *);
typedef void (*line_op_t) (image_t *, int, void *, void *, bool);
typedef void (*flood_fill_call_back_t) (image_t *, int, int, int, void *);

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

typedef struct threshold {
    uint8_t LValue;
    int8_t AValue;
    int8_t BValue;
} threshold_t;

typedef struct statistics {
    uint8_t LMean, LMedian, LMode, LSTDev, LMin, LMax, LLQ, LUQ;
    int8_t AMean, AMedian, AMode, ASTDev, AMin, AMax, ALQ, AUQ;
    int8_t BMean, BMedian, BMode, BSTDev, BMin, BMax, BLQ, BUQ;
} statistics_t;

#define FIND_BLOBS_CORNERS_RESOLUTION    20 // multiple of 4
#define FIND_BLOBS_ANGLE_RESOLUTION      (360 / FIND_BLOBS_CORNERS_RESOLUTION)

typedef struct find_blobs_list_lnk_data {
    point_t corners[FIND_BLOBS_CORNERS_RESOLUTION];
    rectangle_t rect;
    uint32_t pixels, perimeter, code, count;
    float centroid_x, centroid_y, rotation, roundness;
    uint16_t x_hist_bins_count, y_hist_bins_count, *x_hist_bins, *y_hist_bins;
    float centroid_x_acc, centroid_y_acc, rotation_acc_x, rotation_acc_y, roundness_acc;
} find_blobs_list_lnk_data_t;

typedef struct find_lines_list_lnk_data {
    line_t line;
    uint32_t magnitude;
    int16_t theta, rho;
} find_lines_list_lnk_data_t;

typedef struct find_circles_list_lnk_data {
    point_t p;
    uint16_t r, magnitude;
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
    TAG16H5   = 1,
    TAG25H7   = 2,
    TAG25H9   = 4,
    TAG36H10  = 8,
    TAG36H11  = 16,
    ARTOOLKIT = 32
} apriltag_families_t;

typedef struct find_apriltags_list_lnk_data {
    point_t corners[4];
    rectangle_t rect;
    uint16_t id;
    uint8_t family, hamming;
    float centroid_x, centroid_y;
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

typedef enum image_hint {
    IMAGE_HINT_AREA      = (1 << 0),
    IMAGE_HINT_BILINEAR  = (1 << 1),
    IMAGE_HINT_BICUBIC   = (1 << 2),
    IMAGE_HINT_HMIRROR   = (1 << 4),
    IMAGE_HINT_VFLIP     = (1 << 5),
    IMAGE_HINT_TRANSPOSE = (1 << 6),
    IMAGE_HINT_CENTER    = (1 << 7),
    IMAGE_HINT_EXTRACT_RGB_CHANNEL_FIRST = (1 << 8),
    IMAGE_HINT_APPLY_COLOR_PALETTE_FIRST = (1 << 9),
    IMAGE_HINT_SCALE_ASPECT_KEEP         = (1 << 10),
    IMAGE_HINT_SCALE_ASPECT_EXPAND       = (1 << 11),
    IMAGE_HINT_SCALE_ASPECT_IGNORE       = (1 << 12),
    IMAGE_HINT_BLACK_BACKGROUND = (1 << 31)
} image_hint_t;

typedef struct imlib_draw_row_data {
    image_t *dst_img; // user
    pixformat_t src_img_pixfmt; // user
    int rgb_channel; // user
    int alpha; // user
    const uint16_t *color_palette; // user
    const uint8_t *alpha_palette; // user
    bool black_background; // user
    void *callback; // user
    void *callback_arg; // user
    void *dst_row_override; // user
    void *row_buffer; // private
    long smuad_alpha; // private
    uint32_t *smuad_alpha_palette; // private
} imlib_draw_row_data_t;

typedef void (*imlib_draw_row_callback_t) (int x_start, int x_end, int y_row, imlib_draw_row_data_t *data);

// Library Hardware Init
void imlib_init_all();
void imlib_deinit_all();

// Generic Helper Functions
void imlib_fill_image_from_float(image_t *img, int w, int h, float *data, float min, float max,
                                 bool mirror, bool flip, bool dst_transpose, bool src_transpose);

// Bayer Image Processing
pixformat_t imlib_bayer_shift(pixformat_t pixfmt, int x, int y, bool transpose);
void imlib_debayer_ycbcr(image_t *src, rectangle_t *roi, int8_t *Y0, int8_t *CB, int8_t *CR);
void imlib_debayer_line(int x_start, int x_end, int y_row, void *dst_row_ptr, pixformat_t pixfmt, image_t *src);
void imlib_debayer_image(image_t *dst, image_t *src);
void imlib_debayer_image_awb(image_t *dst, image_t *src, bool fast, uint32_t r_out, uint32_t g_out, uint32_t b_out);

// YUV Image Processing
pixformat_t imlib_yuv_shift(pixformat_t pixfmt, int x);
void imlib_deyuv_line(int x_start, int x_end, int y_row, void *dst_row_ptr, pixformat_t pixfmt, image_t *src);
void imlib_deyuv_image(image_t *dst, image_t *src);

/* Color space functions */
int8_t imlib_rgb565_to_l(uint16_t pixel);
int8_t imlib_rgb565_to_a(uint16_t pixel);
int8_t imlib_rgb565_to_b(uint16_t pixel);
uint16_t imlib_lab_to_rgb(uint8_t l, int8_t a, int8_t b);
uint16_t imlib_yuv_to_rgb(uint8_t y, int8_t u, int8_t v);

/* Image file functions */
void ppm_read_geometry(FIL *fp, image_t *img, const char *path, ppm_read_settings_t *rs);
void ppm_read_pixels(FIL *fp, image_t *img, int n_lines, ppm_read_settings_t *rs);
void ppm_read(image_t *img, const char *path);
void ppm_write_subimg(image_t *img, const char *path, rectangle_t *r);
bool bmp_read_geometry(FIL *fp, image_t *img, const char *path, bmp_read_settings_t *rs);
void bmp_read_pixels(FIL *fp, image_t *img, int n_lines, bmp_read_settings_t *rs);
void bmp_read(image_t *img, const char *path);
void bmp_write_subimg(image_t *img, const char *path, rectangle_t *r);
#if (OMV_JPEG_CODEC_ENABLE == 1)
void imlib_hardware_jpeg_init();
void imlib_hardware_jpeg_deinit();
#endif
void jpeg_get_mcu(image_t *src, int x_offset, int y_offset, int dx, int dy,
                  int8_t *Y0, int8_t *CB, int8_t *CR);
void jpeg_decompress(image_t *dst, image_t *src);
bool jpeg_compress(image_t *src, image_t *dst, int quality, bool realloc, jpeg_subsampling_t subsampling);
bool jpeg_is_valid(image_t *img);
int jpeg_clean_trailing_bytes(int bpp, uint8_t *data);
void jpeg_read_geometry(FIL *fp, image_t *img, const char *path, jpg_read_settings_t *rs);
void jpeg_read_pixels(FIL *fp, image_t *img);
void jpeg_read(image_t *img, const char *path);
void jpeg_write(image_t *img, const char *path, int quality);
void png_decompress(image_t *dst, image_t *src);
bool png_compress(image_t *src, image_t *dst);
void png_read_geometry(FIL *fp, image_t *img, const char *path, png_read_settings_t *rs);
void png_read_pixels(FIL *fp, image_t *img);
void png_read(image_t *img, const char *path);
void png_write(image_t *img, const char *path);
bool imlib_read_geometry(FIL *fp, image_t *img, const char *path, img_read_settings_t *rs);
void imlib_image_operation(image_t *img, const char *path, image_t *other, int scalar, line_op_t op, void *data);
void imlib_load_image(image_t *img, const char *path);
void imlib_save_image(image_t *img, const char *path, rectangle_t *roi, int quality);

/* GIF functions */
void gif_open(FIL *fp, int width, int height, bool color, bool loop);
void gif_add_frame(FIL *fp, image_t *img, uint16_t delay);
void gif_close(FIL *fp);

/* MJPEG functions */
void mjpeg_open(FIL *fp, int width, int height);
void mjpeg_write(FIL *fp, int width, int height, uint32_t *frames, uint32_t *bytes,
                 image_t *img, int quality, rectangle_t *roi, int rgb_channel, int alpha,
                 const uint16_t *color_palette, const uint8_t *alpha_palette, image_hint_t hint);
void mjpeg_sync(FIL *fp, uint32_t frames, uint32_t bytes, uint32_t us_avg);
void mjpeg_close(FIL *fp, uint32_t frames, uint32_t bytes, uint32_t us_avg);

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

/* Separable 2D convolution */
void imlib_sepconv3(image_t *img, const int8_t *krn, const float m, const int b);

/* Image Statistics */
int imlib_image_mean(image_t *src, int *r_mean, int *g_mean, int *b_mean);
int imlib_image_std(image_t *src); // grayscale only

/* Template Matching */
void imlib_midpoint_pool(image_t *img_i, image_t *img_o, int x_div, int y_div, const int bias);
void imlib_mean_pool(image_t *img_i, image_t *img_o, int x_div, int y_div);
float imlib_template_match_ds(image_t *image, image_t *t, rectangle_t *r);
float imlib_template_match_ex(image_t *image, image_t *t, rectangle_t *roi, int step, rectangle_t *r);

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
int imlib_load_cascade(struct cascade *cascade, const char *path);
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

// Helper Functions
void imlib_zero(image_t *img, image_t *mask, bool invert);
void imlib_draw_row_setup(imlib_draw_row_data_t *data);
void imlib_draw_row_teardown(imlib_draw_row_data_t *data);
void imlib_draw_row(int x_start, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_draw_image_get_bounds(image_t *dst_img,
                                 image_t *src_img,
                                 int dst_x_start,
                                 int dst_y_start,
                                 float x_scale,
                                 float y_scale,
                                 rectangle_t *roi,
                                 int alpha,
                                 const uint8_t *alpha_palette,
                                 image_hint_t hint,
                                 point_t *p0,
                                 point_t *p1);
void imlib_flood_fill_int(image_t *out, image_t *img, int x, int y,
                          int seed_threshold, int floating_threshold,
                          flood_fill_call_back_t cb, void *data);
// Drawing Functions
int imlib_get_pixel(image_t *img, int x, int y);
int imlib_get_pixel_fast(image_t *img, const void *row_ptr, int x);
void imlib_set_pixel(image_t *img, int x, int y, int p);
void imlib_draw_line(image_t *img, int x0, int y0, int x1, int y1, int c, int thickness);
void imlib_draw_rectangle(image_t *img, int rx, int ry, int rw, int rh, int c, int thickness, bool fill);
void imlib_draw_circle(image_t *img, int cx, int cy, int r, int c, int thickness, bool fill);
void imlib_draw_ellipse(image_t *img, int cx, int cy, int rx, int ry, int rotation, int c, int thickness, bool fill);
void imlib_draw_string(image_t *img,
                       int x_off,
                       int y_off,
                       const char *str,
                       int c,
                       float scale,
                       int x_spacing,
                       int y_spacing,
                       bool mono_space,
                       int char_rotation,
                       bool char_hmirror,
                       bool char_vflip,
                       int string_rotation,
                       bool string_hmirror,
                       bool string_hflip);
void imlib_draw_image(image_t *dst_img,
                      image_t *src_img,
                      int dst_x_start,
                      int dst_y_start,
                      float x_scale,
                      float y_scale,
                      rectangle_t *roi,
                      int rgb_channel,
                      int alpha,
                      const uint16_t *color_palette,
                      const uint8_t *alpha_palette,
                      image_hint_t hint,
                      imlib_draw_row_callback_t callback,
                      void *callback_arg,
                      void *dst_row_override);
void imlib_flood_fill(image_t *img, int x, int y,
                      float seed_threshold, float floating_threshold,
                      int c, bool invert, bool clear_background, image_t *mask);
// ISP Functions
void imlib_awb_rgb_avg(image_t *img, uint32_t *r_out, uint32_t *g_out, uint32_t *b_out);
void imlib_awb_rgb_max(image_t *img, uint32_t *r_out, uint32_t *g_out, uint32_t *b_out);
void imlib_awb(image_t *img, uint32_t r_out, uint32_t g_out, uint32_t b_out);
void imlib_ccm(image_t *img, float *ccm, bool offset);
void imlib_gamma(image_t *img, float gamma, float scale, float offset);
// Binary Functions
void imlib_zero_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_mask_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_binary(image_t *out, image_t *img, list_t *thresholds, bool invert, bool zero, image_t *mask);
void imlib_invert(image_t *img);
void imlib_b_and_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_b_nand_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_b_or_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_b_nor_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_b_xor_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_b_xnor_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_erode(image_t *img, int ksize, int threshold, image_t *mask);
void imlib_dilate(image_t *img, int ksize, int threshold, image_t *mask);
void imlib_open(image_t *img, int ksize, int threshold, image_t *mask);
void imlib_close(image_t *img, int ksize, int threshold, image_t *mask);
void imlib_top_hat(image_t *img, int ksize, int threshold, image_t *mask);
void imlib_black_hat(image_t *img, int ksize, int threshold, image_t *mask);
// Math Functions
void imlib_add_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_sub_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_rsub_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_min_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_max_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
void imlib_difference_line_op(int x, int x_end, int y_row, imlib_draw_row_data_t *data);
// Filtering Functions
void imlib_histeq(image_t *img, image_t *mask);
void imlib_clahe_histeq(image_t *img, float clip_limit, image_t *mask);
void imlib_mean_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask);
void imlib_median_filter(image_t *img, const int ksize, float percentile, bool threshold, int offset, bool invert,
                         image_t *mask);
void imlib_mode_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask);
void imlib_midpoint_filter(image_t *img, const int ksize, float bias, bool threshold, int offset, bool invert, image_t *mask);
void imlib_morph(image_t *img,
                 const int ksize,
                 const int *krn,
                 const float m,
                 const float b,
                 bool threshold,
                 int offset,
                 bool invert,
                 image_t *mask);
void imlib_bilateral_filter(image_t *img,
                            const int ksize,
                            float color_sigma,
                            float space_sigma,
                            bool threshold,
                            int offset,
                            bool invert,
                            image_t *mask);
// Image Correction
void imlib_logpolar_int(image_t *dst, image_t *src, rectangle_t *roi, bool linear, bool reverse); // helper/internal
void imlib_logpolar(image_t *img, bool linear, bool reverse);
// Lens/Rotation Correction
void imlib_lens_corr(image_t *img, float strength, float zoom, float x_corr, float y_corr);
void imlib_rotation_corr(image_t *img, float x_rotation, float y_rotation,
                         float z_rotation, float x_translation, float y_translation,
                         float zoom, float fov, float *corners);
// Statistics
void imlib_get_similarity(image_t *img,
                          image_t *other,
                          int x_start,
                          int y_start,
                          float x_scale,
                          float y_scale,
                          rectangle_t *roi,
                          int rgb_channel,
                          int alpha,
                          const uint16_t *color_palette,
                          const uint8_t *alpha_palette,
                          image_hint_t hint,
                          bool dssim,
                          float *avg,
                          float *std,
                          float *min,
                          float *max);
void imlib_get_histogram(histogram_t *out, image_t *ptr, rectangle_t *roi, list_t *thresholds, bool invert, image_t *other);
void imlib_get_percentile(percentile_t *out, pixformat_t pixfmt, histogram_t *ptr, float percentile);
void imlib_get_threshold(threshold_t *out, pixformat_t pixfmt, histogram_t *ptr);
void imlib_get_statistics(statistics_t *out, pixformat_t pixfmt, histogram_t *ptr);
bool imlib_get_regression(find_lines_list_lnk_data_t *out,
                          image_t *ptr,
                          rectangle_t *roi,
                          unsigned int x_stride,
                          unsigned int y_stride,
                          list_t *thresholds,
                          bool invert,
                          unsigned int area_threshold,
                          unsigned int pixels_threshold,
                          bool robust);
// Color Tracking
void imlib_find_blobs(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                      list_t *thresholds, bool invert, unsigned int area_threshold, unsigned int pixels_threshold,
                      bool merge, int margin,
                      bool (*threshold_cb) (void *, find_blobs_list_lnk_data_t *), void *threshold_cb_arg,
                      bool (*merge_cb) (void *, find_blobs_list_lnk_data_t *, find_blobs_list_lnk_data_t *), void *merge_cb_arg,
                      unsigned int x_hist_bins_max, unsigned int y_hist_bins_max);
// Shape Detection
size_t trace_line(image_t *ptr, line_t *l, int *theta_buffer, uint32_t *mag_buffer, point_t *point_buffer); // helper/internal
void merge_alot(list_t *out, int threshold, int theta_threshold); // helper/internal
void imlib_find_lines(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                      uint32_t threshold, unsigned int theta_margin, unsigned int rho_margin);
void imlib_lsd_find_line_segments(list_t *out,
                                  image_t *ptr,
                                  rectangle_t *roi,
                                  unsigned int merge_distance,
                                  unsigned int max_theta_diff);
void imlib_find_line_segments(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                              uint32_t threshold, unsigned int theta_margin, unsigned int rho_margin,
                              uint32_t segment_threshold);
void imlib_find_circles(list_t *out, image_t *ptr, rectangle_t *roi, unsigned int x_stride, unsigned int y_stride,
                        uint32_t threshold, unsigned int x_margin, unsigned int y_margin, unsigned int r_margin,
                        unsigned int r_min, unsigned int r_max, unsigned int r_step);
void imlib_find_rects(list_t *out, image_t *ptr, rectangle_t *roi,
                      uint32_t threshold);
// 1/2D Bar Codes
void imlib_find_qrcodes(list_t *out, image_t *ptr, rectangle_t *roi);
void imlib_find_apriltags(list_t *out, image_t *ptr, rectangle_t *roi, apriltag_families_t families,
                          float fx, float fy, float cx, float cy);
void imlib_find_datamatrices(list_t *out, image_t *ptr, rectangle_t *roi, int effort);
void imlib_find_barcodes(list_t *out, image_t *ptr, rectangle_t *roi);
// Template Matching
void imlib_phasecorrelate(image_t *img0,
                          image_t *img1,
                          rectangle_t *roi0,
                          rectangle_t *roi1,
                          bool logpolar,
                          bool fix_rotation_scale,
                          float *x_translation,
                          float *y_translation,
                          float *rotation,
                          float *scale,
                          float *response);
// Stereo Imaging
void imlib_stereo_disparity(image_t *img, bool reversed, int max_disparity, int threshold);

array_t *imlib_selective_search(image_t *src, float t, int min_size, float a1, float a2, float a3);
#endif //__IMLIB_H__
