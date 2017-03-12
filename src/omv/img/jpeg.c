/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Minimalistic JPEG baseline encoder.
 * Ported from public domain JPEG writer by Jon Olick - http://jonolick.com
 *
 * DCT implementation is based on Arai, Agui, and Nakajima's algorithm for
 * scaled DCT.
 *
 */

#include <stdio.h>
#include STM32_HAL_H
#include <arm_math.h>

#include "xalloc.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"
#include "imlib.h"

#define TIME_JPEG   (0)
#define FIX_0_382683433  ((int32_t)   98)
#define FIX_0_541196100  ((int32_t)  139)
#define FIX_0_707106781  ((int32_t)  181)
#define FIX_1_306562965  ((int32_t)  334)

#define DESCALE(x, y)   (x>>y)
#define MULTIPLY(x, y)  DESCALE((x) * (y), 8)

typedef struct {
    int idx;
    int length;
    uint8_t *buf;
    int bitc, bitb;
    bool realloc;
    bool overflow;
} jpeg_buf_t;

// Quantization tables
static float fdtbl_Y[64], fdtbl_UV[64];
static uint8_t YTable[64], UVTable[64];

// RGB565 to YUV table
extern const int8_t yuv_table[196608];

static const uint8_t s_jpeg_ZigZag[] = {
    0,  1,   5,  6, 14, 15, 27, 28,
    2,  4,   7, 13, 16, 26, 29, 42,
    3,  8,  12, 17, 25, 30, 41, 43,
    9,  11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

static const uint8_t YQT[] = {
    16, 11, 10, 16, 24,  40,  51,  61,
    12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,
    14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,
    24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

static const uint8_t UVQT[] = {
    17,18,24,47,99,99,99,99,
    18,21,26,66,99,99,99,99,
    24,26,56,99,99,99,99,99,
    47,66,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99,
    99,99,99,99,99,99,99,99
};

static const float aasf[] = {
    1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
    1.0f, 0.785694958f, 0.541196100f, 0.275899379f
};


static const uint8_t std_dc_luminance_nrcodes[] = {0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
static const uint8_t std_dc_luminance_values[] = {0,1,2,3,4,5,6,7,8,9,10,11};
static const uint8_t std_ac_luminance_nrcodes[] = {0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d};
static const uint8_t std_ac_luminance_values[] = {
    0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,
    0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,
    0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
    0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
    0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,
    0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,
    0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
};

static const uint8_t std_dc_chrominance_nrcodes[] = {0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
static const uint8_t std_dc_chrominance_values[] = {0,1,2,3,4,5,6,7,8,9,10,11};
static const uint8_t std_ac_chrominance_nrcodes[] = {0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77};
static const uint8_t std_ac_chrominance_values[] = {
    0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
    0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,
    0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,
    0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,
    0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,
    0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,
    0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
};

// Huffman tables
static const uint16_t YDC_HT[12][2] = { {0,2},{2,3},{3,3},{4,3},{5,3},{6,3},{14,4},{30,5},{62,6},{126,7},{254,8},{510,9}};
static const uint16_t UVDC_HT[12][2] = { {0,2},{1,2},{2,2},{6,3},{14,4},{30,5},{62,6},{126,7},{254,8},{510,9},{1022,10},{2046,11}};
static const uint16_t YAC_HT[256][2] = {
    {0x000A, 0x0004},{0x0000, 0x0002},{0x0001, 0x0002},{0x0004, 0x0003},{0x000B, 0x0004},{0x001A, 0x0005},{0x0078, 0x0007},{0x00F8, 0x0008},
    {0x03F6, 0x000A},{0xFF82, 0x0010},{0xFF83, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x000C, 0x0004},{0x001B, 0x0005},{0x0079, 0x0007},{0x01F6, 0x0009},{0x07F6, 0x000B},{0xFF84, 0x0010},{0xFF85, 0x0010},
    {0xFF86, 0x0010},{0xFF87, 0x0010},{0xFF88, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x001C, 0x0005},{0x00F9, 0x0008},{0x03F7, 0x000A},{0x0FF4, 0x000C},{0xFF89, 0x0010},{0xFF8A, 0x0010},{0xFF8B, 0x0010},
    {0xFF8C, 0x0010},{0xFF8D, 0x0010},{0xFF8E, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x003A, 0x0006},{0x01F7, 0x0009},{0x0FF5, 0x000C},{0xFF8F, 0x0010},{0xFF90, 0x0010},{0xFF91, 0x0010},{0xFF92, 0x0010},
    {0xFF93, 0x0010},{0xFF94, 0x0010},{0xFF95, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x003B, 0x0006},{0x03F8, 0x000A},{0xFF96, 0x0010},{0xFF97, 0x0010},{0xFF98, 0x0010},{0xFF99, 0x0010},{0xFF9A, 0x0010},
    {0xFF9B, 0x0010},{0xFF9C, 0x0010},{0xFF9D, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x007A, 0x0007},{0x07F7, 0x000B},{0xFF9E, 0x0010},{0xFF9F, 0x0010},{0xFFA0, 0x0010},{0xFFA1, 0x0010},{0xFFA2, 0x0010},
    {0xFFA3, 0x0010},{0xFFA4, 0x0010},{0xFFA5, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x007B, 0x0007},{0x0FF6, 0x000C},{0xFFA6, 0x0010},{0xFFA7, 0x0010},{0xFFA8, 0x0010},{0xFFA9, 0x0010},{0xFFAA, 0x0010},
    {0xFFAB, 0x0010},{0xFFAC, 0x0010},{0xFFAD, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x00FA, 0x0008},{0x0FF7, 0x000C},{0xFFAE, 0x0010},{0xFFAF, 0x0010},{0xFFB0, 0x0010},{0xFFB1, 0x0010},{0xFFB2, 0x0010},
    {0xFFB3, 0x0010},{0xFFB4, 0x0010},{0xFFB5, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01F8, 0x0009},{0x7FC0, 0x000F},{0xFFB6, 0x0010},{0xFFB7, 0x0010},{0xFFB8, 0x0010},{0xFFB9, 0x0010},{0xFFBA, 0x0010},
    {0xFFBB, 0x0010},{0xFFBC, 0x0010},{0xFFBD, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01F9, 0x0009},{0xFFBE, 0x0010},{0xFFBF, 0x0010},{0xFFC0, 0x0010},{0xFFC1, 0x0010},{0xFFC2, 0x0010},{0xFFC3, 0x0010},
    {0xFFC4, 0x0010},{0xFFC5, 0x0010},{0xFFC6, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01FA, 0x0009},{0xFFC7, 0x0010},{0xFFC8, 0x0010},{0xFFC9, 0x0010},{0xFFCA, 0x0010},{0xFFCB, 0x0010},{0xFFCC, 0x0010},
    {0xFFCD, 0x0010},{0xFFCE, 0x0010},{0xFFCF, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x03F9, 0x000A},{0xFFD0, 0x0010},{0xFFD1, 0x0010},{0xFFD2, 0x0010},{0xFFD3, 0x0010},{0xFFD4, 0x0010},{0xFFD5, 0x0010},
    {0xFFD6, 0x0010},{0xFFD7, 0x0010},{0xFFD8, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x03FA, 0x000A},{0xFFD9, 0x0010},{0xFFDA, 0x0010},{0xFFDB, 0x0010},{0xFFDC, 0x0010},{0xFFDD, 0x0010},{0xFFDE, 0x0010},
    {0xFFDF, 0x0010},{0xFFE0, 0x0010},{0xFFE1, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x07F8, 0x000B},{0xFFE2, 0x0010},{0xFFE3, 0x0010},{0xFFE4, 0x0010},{0xFFE5, 0x0010},{0xFFE6, 0x0010},{0xFFE7, 0x0010},
    {0xFFE8, 0x0010},{0xFFE9, 0x0010},{0xFFEA, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0xFFEB, 0x0010},{0xFFEC, 0x0010},{0xFFED, 0x0010},{0xFFEE, 0x0010},{0xFFEF, 0x0010},{0xFFF0, 0x0010},{0xFFF1, 0x0010},
    {0xFFF2, 0x0010},{0xFFF3, 0x0010},{0xFFF4, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x07F9, 0x000B},{0xFFF5, 0x0010},{0xFFF6, 0x0010},{0xFFF7, 0x0010},{0xFFF8, 0x0010},{0xFFF9, 0x0010},{0xFFFA, 0x0010},{0xFFFB, 0x0010},
    {0xFFFC, 0x0010},{0xFFFD, 0x0010},{0xFFFE, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
};

static const uint16_t UVAC_HT[256][2] = {
    {0x0000, 0x0002},{0x0001, 0x0002},{0x0004, 0x0003},{0x000A, 0x0004},{0x0018, 0x0005},{0x0019, 0x0005},{0x0038, 0x0006},{0x0078, 0x0007},
    {0x01F4, 0x0009},{0x03F6, 0x000A},{0x0FF4, 0x000C},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x000B, 0x0004},{0x0039, 0x0006},{0x00F6, 0x0008},{0x01F5, 0x0009},{0x07F6, 0x000B},{0x0FF5, 0x000C},{0xFF88, 0x0010},
    {0xFF89, 0x0010},{0xFF8A, 0x0010},{0xFF8B, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x001A, 0x0005},{0x00F7, 0x0008},{0x03F7, 0x000A},{0x0FF6, 0x000C},{0x7FC2, 0x000F},{0xFF8C, 0x0010},{0xFF8D, 0x0010},
    {0xFF8E, 0x0010},{0xFF8F, 0x0010},{0xFF90, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x001B, 0x0005},{0x00F8, 0x0008},{0x03F8, 0x000A},{0x0FF7, 0x000C},{0xFF91, 0x0010},{0xFF92, 0x0010},{0xFF93, 0x0010},
    {0xFF94, 0x0010},{0xFF95, 0x0010},{0xFF96, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x003A, 0x0006},{0x01F6, 0x0009},{0xFF97, 0x0010},{0xFF98, 0x0010},{0xFF99, 0x0010},{0xFF9A, 0x0010},{0xFF9B, 0x0010},
    {0xFF9C, 0x0010},{0xFF9D, 0x0010},{0xFF9E, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x003B, 0x0006},{0x03F9, 0x000A},{0xFF9F, 0x0010},{0xFFA0, 0x0010},{0xFFA1, 0x0010},{0xFFA2, 0x0010},{0xFFA3, 0x0010},
    {0xFFA4, 0x0010},{0xFFA5, 0x0010},{0xFFA6, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x0079, 0x0007},{0x07F7, 0x000B},{0xFFA7, 0x0010},{0xFFA8, 0x0010},{0xFFA9, 0x0010},{0xFFAA, 0x0010},{0xFFAB, 0x0010},
    {0xFFAC, 0x0010},{0xFFAD, 0x0010},{0xFFAE, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x007A, 0x0007},{0x07F8, 0x000B},{0xFFAF, 0x0010},{0xFFB0, 0x0010},{0xFFB1, 0x0010},{0xFFB2, 0x0010},{0xFFB3, 0x0010},
    {0xFFB4, 0x0010},{0xFFB5, 0x0010},{0xFFB6, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x00F9, 0x0008},{0xFFB7, 0x0010},{0xFFB8, 0x0010},{0xFFB9, 0x0010},{0xFFBA, 0x0010},{0xFFBB, 0x0010},{0xFFBC, 0x0010},
    {0xFFBD, 0x0010},{0xFFBE, 0x0010},{0xFFBF, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01F7, 0x0009},{0xFFC0, 0x0010},{0xFFC1, 0x0010},{0xFFC2, 0x0010},{0xFFC3, 0x0010},{0xFFC4, 0x0010},{0xFFC5, 0x0010},
    {0xFFC6, 0x0010},{0xFFC7, 0x0010},{0xFFC8, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01F8, 0x0009},{0xFFC9, 0x0010},{0xFFCA, 0x0010},{0xFFCB, 0x0010},{0xFFCC, 0x0010},{0xFFCD, 0x0010},{0xFFCE, 0x0010},
    {0xFFCF, 0x0010},{0xFFD0, 0x0010},{0xFFD1, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01F9, 0x0009},{0xFFD2, 0x0010},{0xFFD3, 0x0010},{0xFFD4, 0x0010},{0xFFD5, 0x0010},{0xFFD6, 0x0010},{0xFFD7, 0x0010},
    {0xFFD8, 0x0010},{0xFFD9, 0x0010},{0xFFDA, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x01FA, 0x0009},{0xFFDB, 0x0010},{0xFFDC, 0x0010},{0xFFDD, 0x0010},{0xFFDE, 0x0010},{0xFFDF, 0x0010},{0xFFE0, 0x0010},
    {0xFFE1, 0x0010},{0xFFE2, 0x0010},{0xFFE3, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x07F9, 0x000B},{0xFFE4, 0x0010},{0xFFE5, 0x0010},{0xFFE6, 0x0010},{0xFFE7, 0x0010},{0xFFE8, 0x0010},{0xFFE9, 0x0010},
    {0xFFEA, 0x0010},{0xFFEB, 0x0010},{0xFFEC, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x0000, 0x0000},{0x3FE0, 0x000E},{0xFFED, 0x0010},{0xFFEE, 0x0010},{0xFFEF, 0x0010},{0xFFF0, 0x0010},{0xFFF1, 0x0010},{0xFFF2, 0x0010},
    {0xFFF3, 0x0010},{0xFFF4, 0x0010},{0xFFF5, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
    {0x03FA, 0x000A},{0x7FC3, 0x000F},{0xFFF6, 0x0010},{0xFFF7, 0x0010},{0xFFF8, 0x0010},{0xFFF9, 0x0010},{0xFFFA, 0x0010},{0xFFFB, 0x0010},
    {0xFFFC, 0x0010},{0xFFFD, 0x0010},{0xFFFE, 0x0010},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},{0x0000, 0x0000},
};

static void jpeg_put_char(jpeg_buf_t *jpeg_buf, char c)
{
    if ((jpeg_buf->idx+1) >= jpeg_buf->length) {
        if (jpeg_buf->realloc == false) {
            // Can't realloc buffer
            jpeg_buf->overflow = true;
            return;
        }
        jpeg_buf->length += 1024;
        jpeg_buf->buf = xrealloc(jpeg_buf->buf, jpeg_buf->length);
    }

    jpeg_buf->buf[jpeg_buf->idx++]=c;
}

static void jpeg_put_bytes(jpeg_buf_t *jpeg_buf, const void *data, int size)
{
    if ((jpeg_buf->idx+size) >= jpeg_buf->length) {
        if (jpeg_buf->realloc == false) {
            // Can't realloc buffer
            jpeg_buf->overflow = true;
            return;
        }
        jpeg_buf->length += 1024;
        jpeg_buf->buf = xrealloc(jpeg_buf->buf, jpeg_buf->length);
    }

    memcpy(jpeg_buf->buf+jpeg_buf->idx, data, size);
    jpeg_buf->idx += size;
}

static void jpeg_writeBits(jpeg_buf_t *jpeg_buf, const uint16_t *bs)
{
    jpeg_buf->bitc += bs[1];
    jpeg_buf->bitb |= bs[0] << (24 - jpeg_buf->bitc);

    while (jpeg_buf->bitc > 7) {
        uint8_t c = (jpeg_buf->bitb >> 16) & 255;
        jpeg_put_char(jpeg_buf, c);
        if(c == 255) {
            jpeg_put_char(jpeg_buf, 0);
        }
        jpeg_buf->bitb <<= 8;
        jpeg_buf->bitc -= 8;
    }
}

//Huffman-encoded magnitude value
static void jpeg_calcBits(int val, uint16_t bits[2]) {
    int t1=val;
    if (val<0) {
        t1 = -val;
        val = val-1;
    }
    bits[1] = 32-__CLZ(t1);
    bits[0] = val & ((1<<bits[1])-1);
}

static int jpeg_processDU(jpeg_buf_t *jpeg_buf, int8_t *CDU, float *fdtbl, int DC, const uint16_t (*HTDC)[2], const uint16_t (*HTAC)[2])
{
    int DU[64];
    int DUQ[64];
    int z1, z2, z3, z4, z5, z11, z13;
    int t0, t1, t2, t3, t4, t5, t6, t7, t10, t11, t12, t13;
    const uint16_t EOB[2] = { HTAC[0x00][0], HTAC[0x00][1] };
    const uint16_t M16zeroes[2] = { HTAC[0xF0][0], HTAC[0xF0][1] };

    // DCT rows
    for (int i=8, *p=DU; i>0; i--, p+=8, CDU+=8) {
        t0 = CDU[0] + CDU[7];
        t1 = CDU[1] + CDU[6];
        t2 = CDU[2] + CDU[5];
        t3 = CDU[3] + CDU[4];

        t7 = CDU[0] - CDU[7];
        t6 = CDU[1] - CDU[6];
        t5 = CDU[2] - CDU[5];
        t4 = CDU[3] - CDU[4];

        // Even part
        t10 = t0 + t3;
        t13 = t0 - t3;
        t11 = t1 + t2;
        t12 = t1 - t2;
        z1 = MULTIPLY(t12 + t13, FIX_0_707106781); // c4

        p[0] = t10 + t11;
        p[4] = t10 - t11;
        p[2] = t13 + z1;
        p[6] = t13 - z1;

        // Odd part
        t10 = t4 + t5;// phase 2
        t11 = t5 + t6;
        t12 = t6 + t7;

        // The rotator is modified from fig 4-8 to avoid extra negations.
        z5 = MULTIPLY(t10 - t12, FIX_0_382683433); // c6
        z2 = MULTIPLY(t10, FIX_0_541196100) + z5; // 1.306562965f-c6
        z4 = MULTIPLY(t12, FIX_1_306562965) + z5; // 1.306562965f+c6
        z3 = MULTIPLY(t11, FIX_0_707106781); // c4
        z11 = t7 + z3;    // phase 5
        z13 = t7 - z3;

        p[5] = z13 + z2;// phase 6
        p[3] = z13 - z2;
        p[1] = z11 + z4;
        p[7] = z11 - z4;
    }

    // DCT columns
    for (int i=8, *p=DU; i>0; i--, p++) {
        t0 = p[0]  + p[56];
        t1 = p[8]  + p[48];
        t2 = p[16] + p[40];
        t3 = p[24] + p[32];

        t7 = p[0]  - p[56];
        t6 = p[8]  - p[48];
        t5 = p[16] - p[40];
        t4 = p[24] - p[32];

        // Even part
        t10 = t0 + t3;	// phase 2
        t13 = t0 - t3;
        t11 = t1 + t2;
        t12 = t1 - t2;
        z1 = MULTIPLY(t12 + t13, FIX_0_707106781); // c4

        p[0] = t10 + t11; 		// phase 3
        p[32] = t10 - t11;
        p[16] = t13 + z1; 		// phase 5
        p[48] = t13 - z1;

        // Odd part
        t10 = t4 + t5; 		// phase 2
        t11 = t5 + t6;
        t12 = t6 + t7;

        // The rotator is modified from fig 4-8 to avoid extra negations.
        z5 = MULTIPLY(t10 - t12, FIX_0_382683433); // c6
        z2 = MULTIPLY(t10, FIX_0_541196100) + z5; // 1.306562965f-c6
        z4 = MULTIPLY(t12, FIX_1_306562965) + z5; // 1.306562965f+c6
        z3 = MULTIPLY(t11, FIX_0_707106781); // c4
        z11 = t7 + z3;		// phase 5
        z13 = t7 - z3;

        p[40] = z13 + z2;// phase 6
        p[24] = z13 - z2;
        p[8] = z11 + z4;
        p[56] = z11 - z4;
    }

    // first non-zero element in reverse order
    int end0pos = 0;
    // Quantize/descale/zigzag the coefficients
    for(int i=0; i<64; ++i) {
		DUQ[s_jpeg_ZigZag[i]] = fast_roundf(DU[i]*fdtbl[i]);
        if (s_jpeg_ZigZag[i] > end0pos && DUQ[s_jpeg_ZigZag[i]]) {
            end0pos = s_jpeg_ZigZag[i];
        }
    }

    // Encode DC
    int diff = DUQ[0] - DC;
    if (diff == 0) {
        jpeg_writeBits(jpeg_buf, HTDC[0]);
    } else {
        uint16_t bits[2];
        jpeg_calcBits(diff, bits);
        jpeg_writeBits(jpeg_buf, HTDC[bits[1]]);
        jpeg_writeBits(jpeg_buf, bits);
    }

    // Encode ACs
    if(end0pos == 0) {
        jpeg_writeBits(jpeg_buf, EOB);
        return DUQ[0];
    }

    for(int i = 1; i <= end0pos; ++i) {
        int startpos = i;
        for (; DUQ[i]==0 && i<=end0pos ; ++i) {
        }
        int nrzeroes = i-startpos;
        if ( nrzeroes >= 16 ) {
            int lng = nrzeroes>>4;
            for (int nrmarker=1; nrmarker <= lng; ++nrmarker)
                jpeg_writeBits(jpeg_buf, M16zeroes);
            nrzeroes &= 15;
        }
        uint16_t bits[2];
        jpeg_calcBits(DUQ[i], bits);
        jpeg_writeBits(jpeg_buf, HTAC[(nrzeroes<<4)+bits[1]]);
        jpeg_writeBits(jpeg_buf, bits);
    }
    if(end0pos != 63) {
        jpeg_writeBits(jpeg_buf, EOB);
    }
    return DUQ[0];
}

static void jpeg_init(int quality)
{
    static int q =0;

    quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

    // If quality changed, update quantization matrix
    if (q != quality) {
        q = quality;
        for(int i = 0; i < 64; ++i) {
            int yti = (YQT[i]*quality+50)/100;
            YTable[s_jpeg_ZigZag[i]] = yti < 1 ? 1 : yti > 255 ? 255 : yti;
            int uvti  = (UVQT[i]*quality+50)/100;
            UVTable[s_jpeg_ZigZag[i]] = uvti < 1 ? 1 : uvti > 255 ? 255 : uvti;
        }

        for(int r = 0, k = 0; r < 8; ++r) {
            for(int c = 0; c < 8; ++c, ++k) {
                fdtbl_Y[k]  = 1.0f / (aasf[r] * aasf[c] * YTable [s_jpeg_ZigZag[k]] * 8.0f);
                fdtbl_UV[k] = 1.0f / (aasf[r] * aasf[c] * UVTable[s_jpeg_ZigZag[k]] * 8.0f);
            }
        }
    }
}

static void jpeg_write_headers(jpeg_buf_t *jpeg_buf, int w, int h, int bpp, jpeg_subsample_t jpeg_subsample)
{
    // Number of components (1 or 3)
    uint8_t nr_comp = (bpp == 1)? 1 : 3;

    // JPEG headers
    uint8_t m_soi[] = {
        0xFF, 0xD8          // SOI
    };

    uint8_t m_app0[] =  {
        0xFF, 0xE0,         // APP0
        0x00, 0x10,  'J',  'F',  'I',  'F', 0x00, 0x01,
        0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00
    };

    uint8_t m_dqt[] = {
        0xFF, 0xDB,         // DQT
        (bpp*65+2)>>8,      // Header length MSB
        (bpp*65+2)&0xFF,    // Header length LSB
    };

    uint8_t m_sof0[] = {
        0xFF, 0xC0,         // SOF0
        (nr_comp*3+8)>>8,   // Header length MSB
        (nr_comp*3+8)&0xFF, // Header length LSB
        0x08,               // Bits per sample
        h>>8, h&0xFF,       // Height
        w>>8, w&0xFF,       // Width
        nr_comp,            // Number of components
    };

    uint8_t m_dht[] = {
        0xFF, 0xC4,         // DHT
        (bpp*208+2)>>8,     // Header length MSB
        (bpp*208+2)&0xFF,   // Header length LSB
    };

    uint8_t m_sos[] = {
        0xFF, 0xDA,         // SOS
        (nr_comp*2+6)>>8,   // Header length MSB
        (nr_comp*2+6)&0xFF, // Header length LSB
        nr_comp,            // Number of components
    };

    // Write SOI marker
    jpeg_put_bytes(jpeg_buf, m_soi, sizeof(m_soi));
    // Write APP0 marker
    jpeg_put_bytes(jpeg_buf, m_app0, sizeof(m_app0));

    // Write DQT marker
    jpeg_put_bytes(jpeg_buf, m_dqt, sizeof(m_dqt));
    // Write Y quantization table (index, table)
    jpeg_put_char (jpeg_buf, 0);
    jpeg_put_bytes(jpeg_buf, YTable, sizeof(YTable));

    if (bpp > 1) {
        // Write UV quantization table (index, table)
        jpeg_put_char (jpeg_buf, 1);
        jpeg_put_bytes(jpeg_buf, UVTable, sizeof(UVTable));
    }

    // Write SOF0 marker
    jpeg_put_bytes(jpeg_buf, m_sof0, sizeof(m_sof0));
    for (int i=0; i<nr_comp; i++) {
        // Component ID, HV sampling, q table idx
        jpeg_put_bytes(jpeg_buf, (uint8_t [3]){i+1, (i==0 && bpp==2)? jpeg_subsample:0x11, (i>0)}, 3);

    }

    // Write DHT marker
    jpeg_put_bytes(jpeg_buf, m_dht, sizeof(m_dht));

    // Write DHT-YDC
    jpeg_put_char (jpeg_buf, 0x00);
    jpeg_put_bytes(jpeg_buf, std_dc_luminance_nrcodes+1, sizeof(std_dc_luminance_nrcodes)-1);
    jpeg_put_bytes(jpeg_buf, std_dc_luminance_values, sizeof(std_dc_luminance_values));

    // Write DHT-YAC
    jpeg_put_char (jpeg_buf, 0x10);
    jpeg_put_bytes(jpeg_buf, std_ac_luminance_nrcodes+1, sizeof(std_ac_luminance_nrcodes)-1);
    jpeg_put_bytes(jpeg_buf, std_ac_luminance_values, sizeof(std_ac_luminance_values));

    if (bpp > 1) {
        // Write DHT-UDC
        jpeg_put_char (jpeg_buf, 0x01);
        jpeg_put_bytes(jpeg_buf, std_dc_chrominance_nrcodes+1, sizeof(std_dc_chrominance_nrcodes)-1);
        jpeg_put_bytes(jpeg_buf, std_dc_chrominance_values, sizeof(std_dc_chrominance_values));

        // Write DHT-UAC
        jpeg_put_char (jpeg_buf, 0x11);
        jpeg_put_bytes(jpeg_buf, std_ac_chrominance_nrcodes+1, sizeof(std_ac_chrominance_nrcodes)-1);
        jpeg_put_bytes(jpeg_buf, std_ac_chrominance_values, sizeof(std_ac_chrominance_values));
    }

    // Write SOS marker
    jpeg_put_bytes(jpeg_buf, m_sos, sizeof(m_sos));
    for (int i=0; i<nr_comp; i++) {
        jpeg_put_bytes(jpeg_buf, (uint8_t [2]){i+1, (i==0)? 0x00:0x11}, 2);
    }

    // Spectral selection
    jpeg_put_bytes(jpeg_buf, (uint8_t [3]){0x00, 0x3F, 0x0}, 3);
}


void bayer_blk_to_rgb565(image_t *img, int w, int h, int xoffs, int yoffs, uint16_t *rgbbuf)
{
    int r, g, b;
    for (int y=yoffs; y<yoffs+h; y++) {
        for (int x=xoffs; x<xoffs+w; x++) {
            if ((y % 2) == 0) { // Even row
                if ((x % 2) == 0) { // Even col
                    r = (IM_GET_RAW_PIXEL(img, x-1, y-1)  +
                         IM_GET_RAW_PIXEL(img, x+1, y-1)  +
                         IM_GET_RAW_PIXEL(img, x-1, y+1)  +
                         IM_GET_RAW_PIXEL(img, x+1, y+1)) >> 2;

                    g = (IM_GET_RAW_PIXEL(img, x, y-1)  +
                         IM_GET_RAW_PIXEL(img, x, y+1)  +
                         IM_GET_RAW_PIXEL(img, x-1, y)  +
                         IM_GET_RAW_PIXEL(img, x+1, y)) >> 2;

                    b = IM_GET_RAW_PIXEL(img,  x, y);
                } else { // Odd col
                    r = (IM_GET_RAW_PIXEL(img, x, y-1)  +
                         IM_GET_RAW_PIXEL(img, x, y+1)) >> 1;

                    b = (IM_GET_RAW_PIXEL(img, x-1, y)  +
                         IM_GET_RAW_PIXEL(img, x+1, y)) >> 1;

                    g =  IM_GET_RAW_PIXEL(img, x, y);
                }
            } else { // Odd row
                if ((x % 2) == 0) { // Even Col
                    r = (IM_GET_RAW_PIXEL(img, x-1, y)  +
                         IM_GET_RAW_PIXEL(img, x+1, y)) >> 1;

                    g =  IM_GET_RAW_PIXEL(img, x, y);

                    b = (IM_GET_RAW_PIXEL(img, x, y-1)  +
                         IM_GET_RAW_PIXEL(img, x, y+1)) >> 1;
                } else { // Odd col
                    r = IM_GET_RAW_PIXEL(img,  x, y);

                    g = (IM_GET_RAW_PIXEL(img, x, y-1)  +
                         IM_GET_RAW_PIXEL(img, x, y+1)  +
                         IM_GET_RAW_PIXEL(img, x-1, y)  +
                         IM_GET_RAW_PIXEL(img, x+1, y)) >> 2;

                    b = (IM_GET_RAW_PIXEL(img, x-1, y-1)  +
                         IM_GET_RAW_PIXEL(img, x+1, y-1)  +
                         IM_GET_RAW_PIXEL(img, x-1, y+1)  +
                         IM_GET_RAW_PIXEL(img, x+1, y+1)) >> 2;
                }

            }
            r = IM_R825(r);
            g = IM_G826(g);
            b = IM_B825(b);
            *rgbbuf++ = IM_RGB565(r, g, b);
        }
    }
}


bool jpeg_compress(image_t *src, image_t *dst, int quality, bool realloc)
{
    int DCY=0, DCU=0, DCV=0;

    #if (TIME_JPEG==1)
    uint32_t start = HAL_GetTick();
    #endif

    // JPEG buffer
    jpeg_buf_t  jpeg_buf = {
        .idx =0,
        .buf = dst->pixels,
        .length = dst->bpp,
        .bitc = 0,
        .bitb = 0,
        .realloc = realloc,
        .overflow = false,
    };

    // Initialize quantization tables
    jpeg_init(quality);

    jpeg_subsample_t jpeg_subsample;

    if (quality >= 60) {
        jpeg_subsample = JPEG_SUBSAMPLE_1x1;
    } else if (quality > 35) {
        jpeg_subsample = JPEG_SUBSAMPLE_2x1;
    } else { // <= 35
        jpeg_subsample = JPEG_SUBSAMPLE_2x2;
    }

    // Write JPEG headers
    if (src->bpp == 3) { // BAYER
        // Will be converted to RGB565
        jpeg_write_headers(&jpeg_buf, src->w, src->h, 2, jpeg_subsample);
    } else {
        jpeg_write_headers(&jpeg_buf, src->w, src->h, src->bpp, jpeg_subsample);
    }

    // Encode 8x8 macroblocks
    if (src->bpp == 1) {
        int8_t YDU[64];
        uint8_t *pixels = (uint8_t *)src->pixels;

        // Copy 8x8 MCUs
        for (int y=0; y<src->h; y+=8) {
            for (int x=0; x<src->w; x+=8) {
                for (int r=y, idx=0; r<y+8; ++r, idx+=8) {
                    int ofs = r*src->w+x;
                    YDU[idx + 0] = pixels[ofs + 0] - 128;
                    YDU[idx + 1] = pixels[ofs + 1] - 128;
                    YDU[idx + 2] = pixels[ofs + 2] - 128;
                    YDU[idx + 3] = pixels[ofs + 3] - 128;
                    YDU[idx + 4] = pixels[ofs + 4] - 128;
                    YDU[idx + 5] = pixels[ofs + 5] - 128;
                    YDU[idx + 6] = pixels[ofs + 6] - 128;
                    YDU[idx + 7] = pixels[ofs + 7] - 128;
                }
                DCY = jpeg_processDU(&jpeg_buf, YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
            }
            if (jpeg_buf.overflow) {
                goto jpeg_overflow;
            }
        }
    } else if (src->bpp == 2) {// TODO assuming RGB565
        switch (jpeg_subsample) {
            case JPEG_SUBSAMPLE_1x1: {
                int8_t YDU[64], UDU[64], VDU[64];
                uint16_t *pixels = (uint16_t *)src->pixels;

                for (int y=0; y<src->h; y+=8) {
                    for (int x=0; x<src->w; x+=8) {
                        for (int r=y, pos=0; r<y+8; ++r, pos+=8) {
                            int ofs = r*src->w+x;
                            YDU[pos + 0] = yuv_table[pixels[ofs + 0] * 3 + 0];
                            UDU[pos + 0] = yuv_table[pixels[ofs + 0] * 3 + 1];
                            VDU[pos + 0] = yuv_table[pixels[ofs + 0] * 3 + 2];

                            YDU[pos + 1] = yuv_table[pixels[ofs + 1] * 3 + 0];
                            UDU[pos + 1] = yuv_table[pixels[ofs + 1] * 3 + 1];
                            VDU[pos + 1] = yuv_table[pixels[ofs + 1] * 3 + 2];

                            YDU[pos + 2] = yuv_table[pixels[ofs + 2] * 3 + 0];
                            UDU[pos + 2] = yuv_table[pixels[ofs + 2] * 3 + 1];
                            VDU[pos + 2] = yuv_table[pixels[ofs + 2] * 3 + 2];

                            YDU[pos + 3] = yuv_table[pixels[ofs + 3] * 3 + 0];
                            UDU[pos + 3] = yuv_table[pixels[ofs + 3] * 3 + 1];
                            VDU[pos + 3] = yuv_table[pixels[ofs + 3] * 3 + 2];

                            YDU[pos + 4] = yuv_table[pixels[ofs + 4] * 3 + 0];
                            UDU[pos + 4] = yuv_table[pixels[ofs + 4] * 3 + 1];
                            VDU[pos + 4] = yuv_table[pixels[ofs + 4] * 3 + 2];

                            YDU[pos + 5] = yuv_table[pixels[ofs + 5] * 3 + 0];
                            UDU[pos + 5] = yuv_table[pixels[ofs + 5] * 3 + 1];
                            VDU[pos + 5] = yuv_table[pixels[ofs + 5] * 3 + 2];

                            YDU[pos + 6] = yuv_table[pixels[ofs + 6] * 3 + 0];
                            UDU[pos + 6] = yuv_table[pixels[ofs + 6] * 3 + 1];
                            VDU[pos + 6] = yuv_table[pixels[ofs + 6] * 3 + 2];

                            YDU[pos + 7] = yuv_table[pixels[ofs + 7] * 3 + 0];
                            UDU[pos + 7] = yuv_table[pixels[ofs + 7] * 3 + 1];
                            VDU[pos + 7] = yuv_table[pixels[ofs + 7] * 3 + 2];
                        }

                        DCY = jpeg_processDU(&jpeg_buf, YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCU = jpeg_processDU(&jpeg_buf, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
                        DCV = jpeg_processDU(&jpeg_buf, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
                    }
                    if (jpeg_buf.overflow) {
                        goto jpeg_overflow;
                    }
                }
                break;
            }
            case JPEG_SUBSAMPLE_2x1: {
                int8_t YDU[128], UDU[64], VDU[64];
                uint16_t *pixels = (uint16_t *)src->pixels;

                for (int y=0; y<src->h; y+=8) {
                    for (int x=0; x<src->w; x+=16) {
                        for (int r=y, v_pos=0, uv_pos=0; r<y+8; r++, v_pos+=8, uv_pos+=8) {
                            int ofs = r*src->w+x;
                            YDU[v_pos + 0] = yuv_table[pixels[ofs + 0] * 3 + 0];
                            YDU[v_pos + 1] = yuv_table[pixels[ofs + 1] * 3 + 0];
                            YDU[v_pos + 2] = yuv_table[pixels[ofs + 2] * 3 + 0];
                            YDU[v_pos + 3] = yuv_table[pixels[ofs + 3] * 3 + 0];
                            YDU[v_pos + 4] = yuv_table[pixels[ofs + 4] * 3 + 0];
                            YDU[v_pos + 5] = yuv_table[pixels[ofs + 5] * 3 + 0];
                            YDU[v_pos + 6] = yuv_table[pixels[ofs + 6] * 3 + 0];
                            YDU[v_pos + 7] = yuv_table[pixels[ofs + 7] * 3 + 0];

                            YDU[v_pos + 0 + 64] = yuv_table[pixels[ofs + 8 + 0] * 3 + 0];
                            YDU[v_pos + 1 + 64] = yuv_table[pixels[ofs + 8 + 1] * 3 + 0];
                            YDU[v_pos + 2 + 64] = yuv_table[pixels[ofs + 8 + 2] * 3 + 0];
                            YDU[v_pos + 3 + 64] = yuv_table[pixels[ofs + 8 + 3] * 3 + 0];
                            YDU[v_pos + 4 + 64] = yuv_table[pixels[ofs + 8 + 4] * 3 + 0];
                            YDU[v_pos + 5 + 64] = yuv_table[pixels[ofs + 8 + 5] * 3 + 0];
                            YDU[v_pos + 6 + 64] = yuv_table[pixels[ofs + 8 + 6] * 3 + 0];
                            YDU[v_pos + 7 + 64] = yuv_table[pixels[ofs + 8 + 7] * 3 + 0];

                            // Just toss the old UV pixels (could average for better quality)
                            UDU[uv_pos + 0] = yuv_table[pixels[ofs + 0] * 3 + 1];
                            UDU[uv_pos + 1] = yuv_table[pixels[ofs + 2] * 3 + 1];
                            UDU[uv_pos + 2] = yuv_table[pixels[ofs + 4] * 3 + 1];
                            UDU[uv_pos + 3] = yuv_table[pixels[ofs + 6] * 3 + 1];
                            UDU[uv_pos + 4] = yuv_table[pixels[ofs + 8] * 3 + 1];
                            UDU[uv_pos + 5] = yuv_table[pixels[ofs +10] * 3 + 1];
                            UDU[uv_pos + 6] = yuv_table[pixels[ofs +12] * 3 + 1];
                            UDU[uv_pos + 7] = yuv_table[pixels[ofs +14] * 3 + 1];

                            VDU[uv_pos + 0] = yuv_table[pixels[ofs + 0] * 3 + 2];
                            VDU[uv_pos + 1] = yuv_table[pixels[ofs + 2] * 3 + 2];
                            VDU[uv_pos + 2] = yuv_table[pixels[ofs + 4] * 3 + 2];
                            VDU[uv_pos + 3] = yuv_table[pixels[ofs + 6] * 3 + 2];
                            VDU[uv_pos + 4] = yuv_table[pixels[ofs + 8] * 3 + 2];
                            VDU[uv_pos + 5] = yuv_table[pixels[ofs +10] * 3 + 2];
                            VDU[uv_pos + 6] = yuv_table[pixels[ofs +12] * 3 + 2];
                            VDU[uv_pos + 7] = yuv_table[pixels[ofs +14] * 3 + 2];
                        }

                        DCY = jpeg_processDU(&jpeg_buf, YDU,    fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+64, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCU = jpeg_processDU(&jpeg_buf, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
                        DCV = jpeg_processDU(&jpeg_buf, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
                    }
                    if (jpeg_buf.overflow) {
                        goto jpeg_overflow;
                    }
                }
                break;
            }
            case JPEG_SUBSAMPLE_2x2: {
                int8_t YDU[256], UDU[64], VDU[64];
                uint16_t *pixels = (uint16_t *)src->pixels;

                for (int y=0; y<src->h; y+=16) {
                    for (int x=0; x<src->w; x+=16) {
                        for (int i=0, r=y, idx=0; r<y+8; i++, r++, idx+=8) {
                            int ofs = r*src->w+x;
                            YDU[idx + 0]       = yuv_table[pixels[ofs + 0] * 3 + 0];
                            YDU[idx + 1]       = yuv_table[pixels[ofs + 1] * 3 + 0];
                            YDU[idx + 2]       = yuv_table[pixels[ofs + 2] * 3 + 0];
                            YDU[idx + 3]       = yuv_table[pixels[ofs + 3] * 3 + 0];
                            YDU[idx + 4]       = yuv_table[pixels[ofs + 4] * 3 + 0];
                            YDU[idx + 5]       = yuv_table[pixels[ofs + 5] * 3 + 0];
                            YDU[idx + 6]       = yuv_table[pixels[ofs + 6] * 3 + 0];
                            YDU[idx + 7]       = yuv_table[pixels[ofs + 7] * 3 + 0];

                            YDU[idx + 0 + 64]  = yuv_table[pixels[ofs + 0 + 8] * 3 + 0];
                            YDU[idx + 1 + 64]  = yuv_table[pixels[ofs + 1 + 8] * 3 + 0];
                            YDU[idx + 2 + 64]  = yuv_table[pixels[ofs + 2 + 8] * 3 + 0];
                            YDU[idx + 3 + 64]  = yuv_table[pixels[ofs + 3 + 8] * 3 + 0];
                            YDU[idx + 4 + 64]  = yuv_table[pixels[ofs + 4 + 8] * 3 + 0];
                            YDU[idx + 5 + 64]  = yuv_table[pixels[ofs + 5 + 8] * 3 + 0];
                            YDU[idx + 6 + 64]  = yuv_table[pixels[ofs + 6 + 8] * 3 + 0];
                            YDU[idx + 7 + 64]  = yuv_table[pixels[ofs + 7 + 8] * 3 + 0];

                            ofs = (r+8)*src->w+x;
                            YDU[idx + 0 + 128] = yuv_table[pixels[ofs + 0] * 3 + 0];
                            YDU[idx + 1 + 128] = yuv_table[pixels[ofs + 1] * 3 + 0];
                            YDU[idx + 2 + 128] = yuv_table[pixels[ofs + 2] * 3 + 0];
                            YDU[idx + 3 + 128] = yuv_table[pixels[ofs + 3] * 3 + 0];
                            YDU[idx + 4 + 128] = yuv_table[pixels[ofs + 4] * 3 + 0];
                            YDU[idx + 5 + 128] = yuv_table[pixels[ofs + 5] * 3 + 0];
                            YDU[idx + 6 + 128] = yuv_table[pixels[ofs + 6] * 3 + 0];
                            YDU[idx + 7 + 128] = yuv_table[pixels[ofs + 7] * 3 + 0];

                            YDU[idx + 0 + 192] = yuv_table[pixels[ofs + 0 + 8] * 3 + 0];
                            YDU[idx + 1 + 192] = yuv_table[pixels[ofs + 1 + 8] * 3 + 0];
                            YDU[idx + 2 + 192] = yuv_table[pixels[ofs + 2 + 8] * 3 + 0];
                            YDU[idx + 3 + 192] = yuv_table[pixels[ofs + 3 + 8] * 3 + 0];
                            YDU[idx + 4 + 192] = yuv_table[pixels[ofs + 4 + 8] * 3 + 0];
                            YDU[idx + 5 + 192] = yuv_table[pixels[ofs + 5 + 8] * 3 + 0];
                            YDU[idx + 6 + 192] = yuv_table[pixels[ofs + 6 + 8] * 3 + 0];
                            YDU[idx + 7 + 192] = yuv_table[pixels[ofs + 7 + 8] * 3 + 0];

                            ofs = (y+i*2)*src->w+x;
                            // Just toss the odd U/V pixels (could average for better quality)
                            UDU[idx + 0] = yuv_table[pixels[ofs + 0] * 3 + 1];
                            UDU[idx + 1] = yuv_table[pixels[ofs + 2] * 3 + 1];
                            UDU[idx + 2] = yuv_table[pixels[ofs + 4] * 3 + 1];
                            UDU[idx + 3] = yuv_table[pixels[ofs + 6] * 3 + 1];
                            UDU[idx + 4] = yuv_table[pixels[ofs + 8] * 3 + 1];
                            UDU[idx + 5] = yuv_table[pixels[ofs +10] * 3 + 1];
                            UDU[idx + 6] = yuv_table[pixels[ofs +12] * 3 + 1];
                            UDU[idx + 7] = yuv_table[pixels[ofs +14] * 3 + 1];

                            VDU[idx + 0] = yuv_table[pixels[ofs + 0] * 3 + 2];
                            VDU[idx + 1] = yuv_table[pixels[ofs + 2] * 3 + 2];
                            VDU[idx + 2] = yuv_table[pixels[ofs + 4] * 3 + 2];
                            VDU[idx + 3] = yuv_table[pixels[ofs + 6] * 3 + 2];
                            VDU[idx + 4] = yuv_table[pixels[ofs + 8] * 3 + 2];
                            VDU[idx + 5] = yuv_table[pixels[ofs +10] * 3 + 2];
                            VDU[idx + 6] = yuv_table[pixels[ofs +12] * 3 + 2];
                            VDU[idx + 7] = yuv_table[pixels[ofs +14] * 3 + 2];
                        }

                        DCY = jpeg_processDU(&jpeg_buf, YDU,     fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+64,  fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+128, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+192, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCU = jpeg_processDU(&jpeg_buf, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
                        DCV = jpeg_processDU(&jpeg_buf, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
                    }
                    if (jpeg_buf.overflow) {
                        goto jpeg_overflow;
                    }
                }
                break;
            }
        }
    } else if (src->bpp == 3) { //RAW/BAYER
        switch (jpeg_subsample) {
            case JPEG_SUBSAMPLE_1x1: {
                int8_t YDU[64], UDU[64], VDU[64];
                uint16_t rgbbuf[64];
                for (int y=1; y<src->h-1; y+=8) {
                    for (int x=1; x<src->w-1; x+=8) {
                        bayer_blk_to_rgb565(src, 8, 8, x, y, rgbbuf);
                        for (int r=0, idx=0; r<8; r++, idx+=8) {
                            YDU[idx + 0] = yuv_table[rgbbuf[idx + 0] * 3 + 0];
                            UDU[idx + 0] = yuv_table[rgbbuf[idx + 0] * 3 + 1];
                            VDU[idx + 0] = yuv_table[rgbbuf[idx + 0] * 3 + 2];

                            YDU[idx + 1] = yuv_table[rgbbuf[idx + 1] * 3 + 0];
                            UDU[idx + 1] = yuv_table[rgbbuf[idx + 1] * 3 + 1];
                            VDU[idx + 1] = yuv_table[rgbbuf[idx + 1] * 3 + 2];

                            YDU[idx + 2] = yuv_table[rgbbuf[idx + 2] * 3 + 0];
                            UDU[idx + 2] = yuv_table[rgbbuf[idx + 2] * 3 + 1];
                            VDU[idx + 2] = yuv_table[rgbbuf[idx + 2] * 3 + 2];

                            YDU[idx + 3] = yuv_table[rgbbuf[idx + 3] * 3 + 0];
                            UDU[idx + 3] = yuv_table[rgbbuf[idx + 3] * 3 + 1];
                            VDU[idx + 3] = yuv_table[rgbbuf[idx + 3] * 3 + 2];

                            YDU[idx + 4] = yuv_table[rgbbuf[idx + 4] * 3 + 0];
                            UDU[idx + 4] = yuv_table[rgbbuf[idx + 4] * 3 + 1];
                            VDU[idx + 4] = yuv_table[rgbbuf[idx + 4] * 3 + 2];

                            YDU[idx + 5] = yuv_table[rgbbuf[idx + 5] * 3 + 0];
                            UDU[idx + 5] = yuv_table[rgbbuf[idx + 5] * 3 + 1];
                            VDU[idx + 5] = yuv_table[rgbbuf[idx + 5] * 3 + 2];

                            YDU[idx + 6] = yuv_table[rgbbuf[idx + 6] * 3 + 0];
                            UDU[idx + 6] = yuv_table[rgbbuf[idx + 6] * 3 + 1];
                            VDU[idx + 6] = yuv_table[rgbbuf[idx + 6] * 3 + 2];

                            YDU[idx + 7] = yuv_table[rgbbuf[idx + 7] * 3 + 0];
                            UDU[idx + 7] = yuv_table[rgbbuf[idx + 7] * 3 + 1];
                            VDU[idx + 7] = yuv_table[rgbbuf[idx + 7] * 3 + 2];
                        }

                        DCY = jpeg_processDU(&jpeg_buf, YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCU = jpeg_processDU(&jpeg_buf, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
                        DCV = jpeg_processDU(&jpeg_buf, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
                    }
                    if (jpeg_buf.overflow) {
                        goto jpeg_overflow;
                    }
                }
                break;
            }
            case JPEG_SUBSAMPLE_2x1: {
                uint16_t rgbbuf[128];
                int8_t YDU[128], UDU[64], VDU[64];

                for (int y=1; y<src->h-1; y+=8) {
                    for (int x=1; x<src->w-1; x+=16) {
                        bayer_blk_to_rgb565(src, 16, 8, x, y, rgbbuf);
                        for (int r=0, idx=0, ofs=0; r<8; r++, idx+=8, ofs+=16) {
                            YDU[idx + 0]      = yuv_table[rgbbuf[ofs + 0] * 3 + 0];
                            YDU[idx + 1]      = yuv_table[rgbbuf[ofs + 1] * 3 + 0];
                            YDU[idx + 2]      = yuv_table[rgbbuf[ofs + 2] * 3 + 0];
                            YDU[idx + 3]      = yuv_table[rgbbuf[ofs + 3] * 3 + 0];
                            YDU[idx + 4]      = yuv_table[rgbbuf[ofs + 4] * 3 + 0];
                            YDU[idx + 5]      = yuv_table[rgbbuf[ofs + 5] * 3 + 0];
                            YDU[idx + 6]      = yuv_table[rgbbuf[ofs + 6] * 3 + 0];
                            YDU[idx + 7]      = yuv_table[rgbbuf[ofs + 7] * 3 + 0];

                            YDU[idx + 0 + 64] = yuv_table[rgbbuf[ofs + 8] * 3 + 0];
                            YDU[idx + 1 + 64] = yuv_table[rgbbuf[ofs + 9] * 3 + 0];
                            YDU[idx + 2 + 64] = yuv_table[rgbbuf[ofs +10] * 3 + 0];
                            YDU[idx + 3 + 64] = yuv_table[rgbbuf[ofs +11] * 3 + 0];
                            YDU[idx + 4 + 64] = yuv_table[rgbbuf[ofs +12] * 3 + 0];
                            YDU[idx + 5 + 64] = yuv_table[rgbbuf[ofs +13] * 3 + 0];
                            YDU[idx + 6 + 64] = yuv_table[rgbbuf[ofs +14] * 3 + 0];
                            YDU[idx + 7 + 64] = yuv_table[rgbbuf[ofs +15] * 3 + 0];

                            // Just toss the old UV pixels (could average for better quality)
                            UDU[idx + 0]      = yuv_table[rgbbuf[ofs + 0] * 3 + 1];
                            UDU[idx + 1]      = yuv_table[rgbbuf[ofs + 2] * 3 + 1];
                            UDU[idx + 2]      = yuv_table[rgbbuf[ofs + 4] * 3 + 1];
                            UDU[idx + 3]      = yuv_table[rgbbuf[ofs + 6] * 3 + 1];
                            UDU[idx + 4]      = yuv_table[rgbbuf[ofs + 8] * 3 + 1];
                            UDU[idx + 5]      = yuv_table[rgbbuf[ofs +10] * 3 + 1];
                            UDU[idx + 6]      = yuv_table[rgbbuf[ofs +12] * 3 + 1];
                            UDU[idx + 7]      = yuv_table[rgbbuf[ofs +14] * 3 + 1];

                            VDU[idx + 0]      = yuv_table[rgbbuf[ofs + 0] * 3 + 2];
                            VDU[idx + 1]      = yuv_table[rgbbuf[ofs + 2] * 3 + 2];
                            VDU[idx + 2]      = yuv_table[rgbbuf[ofs + 4] * 3 + 2];
                            VDU[idx + 3]      = yuv_table[rgbbuf[ofs + 6] * 3 + 2];
                            VDU[idx + 4]      = yuv_table[rgbbuf[ofs + 8] * 3 + 2];
                            VDU[idx + 5]      = yuv_table[rgbbuf[ofs +10] * 3 + 2];
                            VDU[idx + 6]      = yuv_table[rgbbuf[ofs +12] * 3 + 2];
                            VDU[idx + 7]      = yuv_table[rgbbuf[ofs +14] * 3 + 2];
                        }

                        DCY = jpeg_processDU(&jpeg_buf, YDU,    fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+64, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCU = jpeg_processDU(&jpeg_buf, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
                        DCV = jpeg_processDU(&jpeg_buf, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
                    }
                    if (jpeg_buf.overflow) {
                        goto jpeg_overflow;
                    }
                }
                break;
            }
            case JPEG_SUBSAMPLE_2x2: {
                uint16_t rgbbuf[256];
                int8_t YDU[256], UDU[64], VDU[64];

                for (int y=1; y<src->h-1; y+=16) {
                    for (int x=1; x<src->w-1; x+=16) {
                        bayer_blk_to_rgb565(src, 16, 16, x, y, rgbbuf);
                        for (int r=0, idx=0; r<8; r++, idx+=8) {
                            int ofs = r*16;
                            YDU[idx + 0]       = yuv_table[rgbbuf[ofs + 0] * 3 + 0];
                            YDU[idx + 1]       = yuv_table[rgbbuf[ofs + 1] * 3 + 0];
                            YDU[idx + 2]       = yuv_table[rgbbuf[ofs + 2] * 3 + 0];
                            YDU[idx + 3]       = yuv_table[rgbbuf[ofs + 3] * 3 + 0];
                            YDU[idx + 4]       = yuv_table[rgbbuf[ofs + 4] * 3 + 0];
                            YDU[idx + 5]       = yuv_table[rgbbuf[ofs + 5] * 3 + 0];
                            YDU[idx + 6]       = yuv_table[rgbbuf[ofs + 6] * 3 + 0];
                            YDU[idx + 7]       = yuv_table[rgbbuf[ofs + 7] * 3 + 0];

                            YDU[idx + 0 + 64]  = yuv_table[rgbbuf[ofs + 8] * 3 + 0];
                            YDU[idx + 1 + 64]  = yuv_table[rgbbuf[ofs + 9] * 3 + 0];
                            YDU[idx + 2 + 64]  = yuv_table[rgbbuf[ofs +10] * 3 + 0];
                            YDU[idx + 3 + 64]  = yuv_table[rgbbuf[ofs +11] * 3 + 0];
                            YDU[idx + 4 + 64]  = yuv_table[rgbbuf[ofs +12] * 3 + 0];
                            YDU[idx + 5 + 64]  = yuv_table[rgbbuf[ofs +13] * 3 + 0];
                            YDU[idx + 6 + 64]  = yuv_table[rgbbuf[ofs +14] * 3 + 0];
                            YDU[idx + 7 + 64]  = yuv_table[rgbbuf[ofs +15] * 3 + 0];

                            ofs = (r+8)*16;
                            YDU[idx + 0 + 128] = yuv_table[rgbbuf[ofs + 0] * 3 + 0];
                            YDU[idx + 1 + 128] = yuv_table[rgbbuf[ofs + 1] * 3 + 0];
                            YDU[idx + 2 + 128] = yuv_table[rgbbuf[ofs + 2] * 3 + 0];
                            YDU[idx + 3 + 128] = yuv_table[rgbbuf[ofs + 3] * 3 + 0];
                            YDU[idx + 4 + 128] = yuv_table[rgbbuf[ofs + 4] * 3 + 0];
                            YDU[idx + 5 + 128] = yuv_table[rgbbuf[ofs + 5] * 3 + 0];
                            YDU[idx + 6 + 128] = yuv_table[rgbbuf[ofs + 6] * 3 + 0];
                            YDU[idx + 7 + 128] = yuv_table[rgbbuf[ofs + 7] * 3 + 0];

                            YDU[idx + 0 + 192] = yuv_table[rgbbuf[ofs + 8] * 3 + 0];
                            YDU[idx + 1 + 192] = yuv_table[rgbbuf[ofs + 9] * 3 + 0];
                            YDU[idx + 2 + 192] = yuv_table[rgbbuf[ofs +10] * 3 + 0];
                            YDU[idx + 3 + 192] = yuv_table[rgbbuf[ofs +11] * 3 + 0];
                            YDU[idx + 4 + 192] = yuv_table[rgbbuf[ofs +12] * 3 + 0];
                            YDU[idx + 5 + 192] = yuv_table[rgbbuf[ofs +13] * 3 + 0];
                            YDU[idx + 6 + 192] = yuv_table[rgbbuf[ofs +14] * 3 + 0];
                            YDU[idx + 7 + 192] = yuv_table[rgbbuf[ofs +15] * 3 + 0];

                            ofs = (r*2)*16;
                            // Just toss the odd U/V pixels (could average for better quality)
                            UDU[idx + 0]       = yuv_table[rgbbuf[ofs + 0] * 3 + 1];
                            UDU[idx + 1]       = yuv_table[rgbbuf[ofs + 2] * 3 + 1];
                            UDU[idx + 2]       = yuv_table[rgbbuf[ofs + 4] * 3 + 1];
                            UDU[idx + 3]       = yuv_table[rgbbuf[ofs + 6] * 3 + 1];
                            UDU[idx + 4]       = yuv_table[rgbbuf[ofs + 8] * 3 + 1];
                            UDU[idx + 5]       = yuv_table[rgbbuf[ofs +10] * 3 + 1];
                            UDU[idx + 6]       = yuv_table[rgbbuf[ofs +12] * 3 + 1];
                            UDU[idx + 7]       = yuv_table[rgbbuf[ofs +14] * 3 + 1];

                            VDU[idx + 0]       = yuv_table[rgbbuf[ofs + 0] * 3 + 2];
                            VDU[idx + 1]       = yuv_table[rgbbuf[ofs + 2] * 3 + 2];
                            VDU[idx + 2]       = yuv_table[rgbbuf[ofs + 4] * 3 + 2];
                            VDU[idx + 3]       = yuv_table[rgbbuf[ofs + 6] * 3 + 2];
                            VDU[idx + 4]       = yuv_table[rgbbuf[ofs + 8] * 3 + 2];
                            VDU[idx + 5]       = yuv_table[rgbbuf[ofs +10] * 3 + 2];
                            VDU[idx + 6]       = yuv_table[rgbbuf[ofs +12] * 3 + 2];
                            VDU[idx + 7]       = yuv_table[rgbbuf[ofs +14] * 3 + 2];
                        }

                        DCY = jpeg_processDU(&jpeg_buf, YDU,     fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+64,  fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+128, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCY = jpeg_processDU(&jpeg_buf, YDU+192, fdtbl_Y, DCY, YDC_HT, YAC_HT);
                        DCU = jpeg_processDU(&jpeg_buf, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
                        DCV = jpeg_processDU(&jpeg_buf, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
                    }
                    if (jpeg_buf.overflow) {
                        goto jpeg_overflow;
                    }
                }
                break;
            }
        }
    }


    // Do the bit alignment of the EOI marker
    static const uint16_t fillBits[] = {0x7F, 7};
    jpeg_writeBits(&jpeg_buf, fillBits);

    // EOI
    jpeg_put_char(&jpeg_buf, 0xFF);
    jpeg_put_char(&jpeg_buf, 0xD9);

    dst->bpp = jpeg_buf.idx;
    dst->data = jpeg_buf.buf;

    #if (TIME_JPEG==1)
    printf("time: %lums\n", HAL_GetTick() - start);
    #endif

jpeg_overflow:
    return jpeg_buf.overflow;
}

// This function inits the geometry values of an image.
void jpeg_read_geometry(FIL *fp, image_t *img, const char *path)
{
    for (;;) {
        uint16_t header;
        read_word(fp, &header);
        header = IM_SWAP16(header);
        if ((0xFFD0 <= header) && (header <= 0xFFD9)) {
            continue;
        } else if (((0xFFC0 <= header) && (header <= 0xFFCF))
                || ((0xFFDA <= header) && (header <= 0xFFDF))
                || ((0xFFE0 <= header) && (header <= 0xFFEF))
                || ((0xFFF0 <= header) && (header <= 0xFFFE)))
        {
            uint16_t size;
            read_word(fp, &size);
            size = IM_SWAP16(size);
            if (((0xFFC0 <= header) && (header <= 0xFFC3))
             || ((0xFFC5 <= header) && (header <= 0xFFC7))
             || ((0xFFC9 <= header) && (header <= 0xFFCB))
             || ((0xFFCD <= header) && (header <= 0xFFCF)))
            {
                read_byte_ignore(fp);
                uint16_t width;
                read_word(fp, &width);
                width = IM_SWAP16(width);
                uint16_t height;
                read_word(fp, &height);
                height = IM_SWAP16(height);
                img->w = width;
                img->h = height;
                img->bpp = f_size(fp);
                return;
            } else {
                file_seek(fp, f_tell(fp) + size - 2);
            }
        } else {
            ff_file_corrupted(fp);
        }
    }
}

// This function reads the pixel values of an image.
void jpeg_read_pixels(FIL *fp, image_t *img)
{
    file_seek(fp, 0);
    read_data(fp, img->pixels, img->bpp);
}

void jpeg_read(image_t *img, const char *path)
{
    FIL fp;
    file_read_open(&fp, path);
    // Do not use file_buffer_on() here.
    jpeg_read_geometry(&fp, img, path);
    if (!img->pixels) img->pixels = xalloc(img->bpp);
    jpeg_read_pixels(&fp, img);
    // Do not use file_buffer_off() here.
    file_close(&fp);
}

void jpeg_write(image_t *img, const char *path, int quality)
{
    FIL fp;
    file_write_open(&fp, path);
    if (IM_IS_JPEG(img)) {
        write_data(&fp, img->pixels, img->bpp);
    } else {
        uint32_t size;
        uint8_t *buffer = fb_alloc_all(&size);
        image_t out = { .w=img->w, .h=img->h, .bpp=size, .pixels=buffer };
        // When jpeg_compress needs more memory than in currently allocated it
        // will try to realloc. MP will detect that the pointer is outside of
        // the heap and return NULL which will cause an out of memory error.
        jpeg_compress(img, &out, quality, false);
        write_data(&fp, out.pixels, out.bpp);
        fb_free();
    }
    file_close(&fp);
}
