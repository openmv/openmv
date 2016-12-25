/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __IMLIB_COLOR_H__
#define __IMLIB_COLOR_H__
#include <math.h>
#include <stdint.h>
#include "other_maxmin.h"

typedef struct imlib_color_thresholds_linkedlist_lnk_data
{
    uint8_t LMin, LMax;
    int8_t AMin, AMax, BMin, BMax;
    uint8_t unused0, unused1;
}
imlib_color_thresholds_linkedlist_lnk_data_t;

#define IMLIB_COLOR_BINARY_MIN 0
#define IMLIB_COLOR_BINARY_MAX 1

#define IMLIB_COLOR_GRAYSCALE_MIN 0
#define IMLIB_COLOR_GRAYSCALE_MAX 255

#define IMLIB_COLOR_R5_MIN 0
#define IMLIB_COLOR_R5_MAX 31
#define IMLIB_COLOR_G6_MIN 0
#define IMLIB_COLOR_G6_MAX 63
#define IMLIB_COLOR_B5_MIN 0
#define IMLIB_COLOR_B5_MAX 31

#define IMLIB_COLOR_R8_MIN 0
#define IMLIB_COLOR_R8_MAX 255
#define IMLIB_COLOR_G8_MIN 0
#define IMLIB_COLOR_G8_MAX 255
#define IMLIB_COLOR_B8_MIN 0
#define IMLIB_COLOR_B8_MAX 255

#define IMLIB_COLOR_L_MIN 0
#define IMLIB_COLOR_L_MAX 100
#define IMLIB_COLOR_A_MIN -128
#define IMLIB_COLOR_A_MAX 127
#define IMLIB_COLOR_B_MIN -128
#define IMLIB_COLOR_B_MAX 127

#define IMLIB_COLOR_Y_MIN 0
#define IMLIB_COLOR_Y_MAX 240
#define IMLIB_COLOR_U_MIN -128
#define IMLIB_COLOR_V_MAX 127
#define IMLIB_COLOR_U_MIN -128
#define IMLIB_COLOR_V_MAX 127

extern const uint8_t imlib_color_r5_to_r8_table[32];
extern const uint8_t imlib_color_g6_to_g8_table[64];
#define imlib_color_b5_to_b8_table imlib_color_r5_to_r8_table

#define IMLIB_COLOR_R5_TO_R8(color) imlib_color_r5_to_r8_table[color]
#define IMLIB_COLOR_G6_TO_G8(color) imlib_color_g6_to_g8_table[color]
#define IMLIB_COLOR_B5_TO_B8(color) imlib_color_b5_to_b8_table[color]

extern const uint8_t imlib_color_r8_to_r5_table[256];
extern const uint8_t imlib_color_g8_to_g6_table[256];
#define imlib_color_b8_to_b5_table imlib_color_r8_to_r5_table

#define IMLIB_COLOR_R8_TO_R5(color) imlib_color_r8_to_r5_table[color]
#define IMLIB_COLOR_G8_TO_G6(color) imlib_color_g8_to_g6_table[color]
#define IMLIB_COLOR_B8_TO_B5(color) imlib_color_b8_to_b5_table[color]

// RGB565 Stuff //

#define IMLIB_COLOR_RGB565_TO_R5(pixel) (((pixel) >> 3) & 0x1F)
#define IMLIB_COLOR_RGB565_TO_G6(pixel) \
({ \
    typeof (pixel) _pixel = (pixel); \
    ((_pixel & 0x07) << 3) | (_pixel >> 13); \
})
#define IMLIB_COLOR_RGB565_TO_B5(pixel) (((pixel) >> 8) & 0x1F)
#define IMLIB_COLOR_RGB565_TO_R8(pixel) IMLIB_COLOR_R5_TO_R8(IMLIB_COLOR_RGB565_TO_R5(pixel))
#define IMLIB_COLOR_RGB565_TO_G8(pixel) IMLIB_COLOR_G6_TO_G8(IMLIB_COLOR_RGB565_TO_G6(pixel))
#define IMLIB_COLOR_RGB565_TO_B8(pixel) IMLIB_COLOR_B5_TO_B8(IMLIB_COLOR_RGB565_TO_B5(pixel))

#define IMLIB_COLOR_R5_G6_B5_TO_RGB565(r5, g6, b5) \
({ \
    typeof (r5) _r5 = (r5); \
    typeof (g6) _g6 = (g6); \
    typeof (b5) _b5 = (b5); \
    (_r5 << 3) | (_g6 >> 3) | (_g6 << 13) | (_b5 << 8); \
})

#define IMLIB_COLOR_R8_G8_B8_TO_RGB565(r8, g8, b8) IMLIB_COLOR_R5_G6_B5_TO_RGB565(IMLIB_COLOR_R8_TO_R5(r8), IMLIB_COLOR_G8_TO_G6(g8), IMLIB_COLOR_B8_TO_B5(b8))

extern const int8_t lab_table[196608];
extern const int8_t yuv_table[196608];

#define IMLIB_COLOR_RGB565_TO_L(pixel) lab_table[(pixel) * 3]
#define IMLIB_COLOR_RGB565_TO_A(pixel) lab_table[((pixel) * 3) + 1]
#define IMLIB_COLOR_RGB565_TO_B(pixel) lab_table[((pixel) * 3) + 2]
#define IMLIB_COLOR_RGB565_TO_Y(pixel) yuv_table[(pixel) * 3]
#define IMLIB_COLOR_RGB565_TO_U(pixel) yuv_table[((pixel) * 3) + 1]
#define IMLIB_COLOR_RGB565_TO_V(pixel) yuv_table[((pixel) * 3) + 2]

// https://en.wikipedia.org/wiki/Lab_color_space -> CIELAB-CIEXYZ conversions
// https://en.wikipedia.org/wiki/SRGB -> Specification of the transformation

#define IMLIB_COLOR_LAB_TO_RGB565(l, a, b) \
({ \
    typeof (l) _l = (l); \
    typeof (a) _a = (a); \
    typeof (b) _b = (b); \
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
    unsigned int _r = OTHER_MAX(OTHER_MIN(roundf(_r_lin * IMLIB_COLOR_R5_MAX), IMLIB_COLOR_R5_MAX), IMLIB_COLOR_R5_MIN); \
    unsigned int _g = OTHER_MAX(OTHER_MIN(roundf(_g_lin * IMLIB_COLOR_G6_MAX), IMLIB_COLOR_G6_MAX), IMLIB_COLOR_G6_MIN); \
    unsigned int _b = OTHER_MAX(OTHER_MIN(roundf(_b_lin * IMLIB_COLOR_B5_MAX), IMLIB_COLOR_B5_MAX), IMLIB_COLOR_B5_MIN); \
    IMLIB_COLOR_R5_G6_B5_TO_RGB565(_r, _g, _b); \
})

// https://en.wikipedia.org/wiki/YCbCr -> JPEG Conversion

#define IMLIB_COLOR_YUV_TO_RGB565(y, u, v) \
({ \
    typeof (y) _y = (y); \
    typeof (u) _u = (u); \
    typeof (v) _v = (v); \
    unsigned int _r = OTHER_MAX(OTHER_MIN(128 + _y + ((((uint32_t) ((1.402000 * 65536) + 0.5)) * _v) >> 16), IMLIB_COLOR_R8_MAX), IMLIB_COLOR_R8_MIN); \
    unsigned int _g = OTHER_MAX(OTHER_MIN(128 + _y - (((((uint32_t) ((0.344136 * 65536) + 0.5)) * _u) + (((uint32_t) ((0.714136 * 65536) + 0.5)) * _v)) >> 16), IMLIB_COLOR_G8_MAX), IMLIB_COLOR_G8_MIN); \
    unsigned int _b = OTHER_MAX(OTHER_MIN(128 + _y + ((((uint32_t) ((1.772000 * 65536) + 0.5)) * _u) >> 16), IMLIB_COLOR_B8_MAX), IMLIB_COLOR_B8_MIN); \
    IMLIB_COLOR_R8_G8_B8_TO_RGB565(_r, _g, _b); \
})

#define IMLIB_COLOR_BINARY_TO_GRAYSCALE(pixel) ((pixel) * IMLIB_COLOR_GRAYSCALE_MAX)
#define IMLIB_COLOR_BINARY_TO_RGB565(pixel) IMLIB_COLOR_YUV_TO_RGB565((pixel) * 127, 0, 0)
#define IMLIB_COLOR_RGB565_TO_BINARY(pixel) (IMLIB_COLOR_RGB565_TO_Y(pixel) == 127)
#define IMLIB_COLOR_RGB565_TO_GRAYSCALE(pixel) (IMLIB_COLOR_RGB565_TO_Y(pixel) + 128)
#define IMLIB_COLOR_GRAYSCALE_TO_BINARY(pixel) ((pixel) == IMLIB_COLOR_GRAYSCALE_MAX)
#define IMLIB_COLOR_GRAYSCALE_TO_RGB565(pixel) IMLIB_COLOR_YUV_TO_RGB565((pixel) - 128, 0, 0)

#define IMLIB_COLOR_THRESHOLD_BINARY(pixel, threshold, invert) ((pixel) ^ (invert))
#define IMLIB_COLOR_THRESHOLD_GRAYSCALE(pixel, threshold, invert) \
({ \
    typeof (pixel) _pixel = (pixel); \
    typeof (threshold) _threshold = (threshold); \
    typeof (invert) _invert = (invert); \
    ((_threshold->LMin <= _pixel) && (_pixel <= _threshold->LMax)) ^ _invert; \
})

#define IMLIB_COLOR_THRESHOLD_RGB565(pixel, threshold, invert) \
({ \
    typeof (pixel) _pixel = (pixel); \
    typeof (threshold) _threshold = (threshold); \
    typeof (invert) _invert = (invert); \
    int8_t _l = IMLIB_COLOR_RGB565_TO_L(_pixel); \
    int8_t _a = IMLIB_COLOR_RGB565_TO_A(_pixel); \
    int8_t _b = IMLIB_COLOR_RGB565_TO_B(_pixel); \
    ((_threshold->LMin <= _l) && (_l <= _threshold->LMax) && (_threshold->AMin <= _a) && (_a <= _threshold->AMax) && (_threshold->BMin <= _b) && (_b <= _threshold->BMax)) ^ _invert; \
})

#endif /* __IMLIB_COLOR_H__ */
