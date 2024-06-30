/*
 * 1394-Based Digital Camera Control Library
 *
 * Bayer pattern decoding functions
 *
 * Written by Damien Douxchamps and Frederic Devernay
 * The original VNG and AHD Bayer decoding are from Dave Coffin's DCRAW.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**************************************************************************//**
 * @file     bayer.c
 * @author   Tanay Rami
 * @email    tanay@alifsemi.com
 * @version  V1.0.0
 * @date     18-Nov-2021
 * @brief    Original "Open-Source" Linux Command-line Script code is
 *            available here: https://github.com/jdthomas/bayer2rgb
 *
 *            Modified this file in order to fix memory issues:
 *             - Selected Bit Version:
 *               - 8-bit Version (16-bit version is not tested.)
 *             - Selected Bayer Method:
 *               - dc1394 Bayer HQLinear Method
 *             - Commented out all other unused Bayer methods.
 ******************************************************************************/

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include "conversions.h"
#include "bayer.h"

/* At the time of writing, GCC produces incorrect assembly */
#define ENABLE_MVE_BAYER2RGB (__ARMCC_VERSION >= 6180002 && (__ARM_FEATURE_MVE & 1))

#if __ARM_FEATURE_MVE & 1
#include <arm_mve.h>
#endif

#define ENABLE_8_BIT_VERSION      1   /* Enable 8-bit Version. */
#define ENABLE_16_BIT_VERSION     0   /* Enable 16-bit Version.(Not Tested.) */

#if ENABLE_8_BIT_VERSION
/* Only enabled dc1394 Bayer HQLinear Method for Bayer to RGB Conversion.
 * remaining all methods are disable to fix memory issues.
 */
#define ENABLE_DC1394_BAYER_METHOD_HQLINEAR        1
#define ENABLE_DC1394_BAYER_METHOD_NEAREST         0
#define ENABLE_DC1394_BAYER_METHOD_BILINEAR        0
#define ENABLE_DC1394_BAYER_METHOD_EDGESENSE       0
#define ENABLE_DC1394_BAYER_METHOD_DOWNSAMPLE      0
#define ENABLE_DC1394_BAYER_METHOD_SIMPLE          0
#define ENABLE_DC1394_BAYER_METHOD_VNG             0
#define ENABLE_DC1394_BAYER_METHOD_AHD             0
#endif

#define CLIP(in, out)\
   in = in < 0 ? 0 : in;\
   in = in > 255 ? 255 : in;\
   out=in;

#define CLIP16(in, out, bits)\
   in = in < 0 ? 0 : in;\
   in = in > ((1<<bits)-1) ? ((1<<bits)-1) : in;\
   out=in;

void
ClearBorders(uint8_t *rgb, int sx, int sy, int w)
{
    int i, j;
    // black edges are added with a width w:
    i = 3 * sx * w - 1;
    j = 3 * sx * sy - 1;
    while (i >= 0) {
        rgb[i--] = 0;
        rgb[j--] = 0;
    }

    int low = sx * (w - 1) * 3 - 1 + w * 3;
    i = low + sx * (sy - w * 2 + 1) * 3;
    while (i > low) {
        j = 6 * w;
        while (j > 0) {
            rgb[i--] = 0;
            j--;
        }
        i -= (sx - 2 * w) * 3;
    }
}

void
ClearBorders_uint16(uint16_t * rgb, int sx, int sy, int w)
{
    int i, j;

    // black edges:
    i = 3 * sx * w - 1;
    j = 3 * sx * sy - 1;
    while (i >= 0) {
        rgb[i--] = 0;
        rgb[j--] = 0;
    }

    int low = sx * (w - 1) * 3 - 1 + w * 3;
    i = low + sx * (sy - w * 2 + 1) * 3;
    while (i > low) {
        j = 6 * w;
        while (j > 0) {
            rgb[i--] = 0;
            j--;
        }
        i -= (sx - 2 * w) * 3;
    }

}

/**************************************************************
 *     Color conversion functions for cameras that can        *
 * output raw-Bayer pattern images, such as some Basler and   *
 * Point Grey camera. Most of the algos presented here come   *
 * from http://www-ise.stanford.edu/~tingchen/ and have been  *
 * converted from Matlab to C and extended to all elementary  *
 * patterns.                                                  *
 **************************************************************/

/* 8-bits versions */
/* insprired by OpenCV's Bayer decoding */

#if ENABLE_8_BIT_VERSION

#if ENABLE_DC1394_BAYER_METHOD_NEAREST
dc1394error_t
dc1394_bayer_NearestNeighbor(const uint8_t *restrict bayer, uint8_t *restrict rgb, int sx, int sy, int tile)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;
    int i, imax, iinc;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
      return DC1394_INVALID_COLOR_FILTER;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
        rgb[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
        rgb[i++] = 0;
        rgb[i++] = 0;
        rgb[i++] = 0;
    }

    rgb += 1;
    width -= 1;
    height -= 1;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
      //int t0, t1;
        const uint8_t *bayerEnd = bayer + width;

        if (start_with_green) {
            rgb[-blue] = bayer[1];
            rgb[0] = bayer[bayerStep + 1];
            rgb[blue] = bayer[bayerStep];
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[-1] = bayer[0];
                rgb[0] = bayer[1];
                rgb[1] = bayer[bayerStep + 1];

                rgb[2] = bayer[2];
                rgb[3] = bayer[bayerStep + 2];
                rgb[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[1] = bayer[0];
                rgb[0] = bayer[1];
                rgb[-1] = bayer[bayerStep + 1];

                rgb[4] = bayer[2];
                rgb[3] = bayer[bayerStep + 2];
                rgb[2] = bayer[bayerStep + 1];
            }
        }

        if (bayer < bayerEnd) {
            rgb[-blue] = bayer[0];
            rgb[0] = bayer[1];
            rgb[blue] = bayer[bayerStep + 1];
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return DC1394_SUCCESS;
}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_NEAREST */

/* OpenCV's Bayer decoding */
#if ENABLE_DC1394_BAYER_METHOD_BILINEAR
dc1394error_t
dc1394_bayer_Bilinear(const uint8_t *restrict bayer, uint8_t *restrict rgb, int sx, int sy, int tile)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    /*
       the two letters  of the OpenCV name are respectively
       the 4th and 3rd letters from the blinky name,
       and we also have to switch R and B (OpenCV is BGR)

       CV_BayerBG2BGR <-> DC1394_COLOR_FILTER_BGGR
       CV_BayerGB2BGR <-> DC1394_COLOR_FILTER_GBRG
       CV_BayerGR2BGR <-> DC1394_COLOR_FILTER_GRBG

       int blue = tile == CV_BayerBG2BGR || tile == CV_BayerGB2BGR ? -1 : 1;
       int start_with_green = tile == CV_BayerGB2BGR || tile == CV_BayerGR2BGR;
     */
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
        return DC1394_INVALID_COLOR_FILTER;

    ClearBorders(rgb, sx, sy, 1);
    rgb += rgbStep + 3 + 1;
    height -= 2;
    width -= 2;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        int t0, t1;
        const uint8_t *bayerEnd = bayer + width;

        if (start_with_green) {
            /* OpenCV has a bug in the next line, which was
               t0 = (bayer[0] + bayer[bayerStep * 2] + 1) >> 1; */
            t0 = (bayer[1] + bayer[bayerStep * 2 + 1] + 1) >> 1;
            t1 = (bayer[bayerStep] + bayer[bayerStep + 2] + 1) >> 1;
            rgb[-blue] = (uint8_t) t0;
            rgb[0] = bayer[bayerStep + 1];
            rgb[blue] = (uint8_t) t1;
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
                      bayer[bayerStep * 2 + 2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
                      2) >> 2;
                rgb[-1] = (uint8_t) t0;
                rgb[0] = (uint8_t) t1;
                rgb[1] = bayer[bayerStep + 1];

                t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
                t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                      1) >> 1;
                rgb[2] = (uint8_t) t0;
                rgb[3] = bayer[bayerStep + 2];
                rgb[4] = (uint8_t) t1;
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
                      bayer[bayerStep * 2 + 2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
                      2) >> 2;
                rgb[1] = (uint8_t) t0;
                rgb[0] = (uint8_t) t1;
                rgb[-1] = bayer[bayerStep + 1];

                t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
                t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                      1) >> 1;
                rgb[4] = (uint8_t) t0;
                rgb[3] = bayer[bayerStep + 2];
                rgb[2] = (uint8_t) t1;
            }
        }

        if (bayer < bayerEnd) {
            t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
                  bayer[bayerStep * 2 + 2] + 2) >> 2;
            t1 = (bayer[1] + bayer[bayerStep] +
                  bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
                  2) >> 2;
            rgb[-blue] = (uint8_t) t0;
            rgb[0] = (uint8_t) t1;
            rgb[blue] = bayer[bayerStep + 1];
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }
    return DC1394_SUCCESS;
}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_BILINEAR */

/* High-Quality Linear Interpolation For Demosaicing Of
   Bayer-Patterned Color Images, by Henrique S. Malvar, Li-wei He, and
   Ross Cutler, in ICASSP'04 */
#if ENABLE_DC1394_BAYER_METHOD_HQLINEAR
dc1394error_t
dc1394_bayer_HQLinear(const uint8_t *restrict bayer, uint8_t *restrict rgb, int sx, int sy, int tile)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
      return DC1394_INVALID_COLOR_FILTER;

    ClearBorders(rgb, sx, sy, 2);
    rgb += 2 * rgbStep + 6 + 1;
    height -= 4;
    width -= 4;

    /* We begin with a (+1 line,+1 column) offset with respect to bilinear decoding, so start_with_green is the same, but blue is opposite */
    blue = -blue;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        int t0, t1;
        const uint8_t *bayerEnd = bayer + width;
        const int bayerStep2 = bayerStep * 2;
        const int bayerStep3 = bayerStep * 3;
        const int bayerStep4 = bayerStep * 4;

        if (start_with_green) {
            /* at green pixel */
            rgb[0] = bayer[bayerStep2 + 2];
            t0 = rgb[0] * 5
                + ((bayer[bayerStep + 2] + bayer[bayerStep3 + 2]) << 2)
                - bayer[2]
                - bayer[bayerStep + 1]
                - bayer[bayerStep + 3]
                - bayer[bayerStep3 + 1]
                - bayer[bayerStep3 + 3]
                - bayer[bayerStep4 + 2]
                + ((bayer[bayerStep2] + bayer[bayerStep2 + 4] + 1) >> 1);
            t1 = rgb[0] * 5 +
                ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 3]) << 2)
                - bayer[bayerStep2]
                - bayer[bayerStep + 1]
                - bayer[bayerStep + 3]
                - bayer[bayerStep3 + 1]
                - bayer[bayerStep3 + 3]
                - bayer[bayerStep2 + 4]
                + ((bayer[2] + bayer[bayerStep4 + 2] + 1) >> 1);
            t0 = (t0 + 4) >> 3;
            CLIP(t0, rgb[-blue]);
            t1 = (t1 + 4) >> 3;
            CLIP(t1, rgb[blue]);
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                /* B at B */
                rgb[1] = bayer[bayerStep2 + 2];
                /* R at B */
                t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                       bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                    -
                    (((bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                                                     2]) * 3 + 1) >> 1)
                    + rgb[1] * 6;
                /* G at B */
                t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                       bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2]) << 1)
                    - (bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                    + (rgb[1] << 2);
                t0 = (t0 + 4) >> 3;
                CLIP(t0, rgb[-1]);
                t1 = (t1 + 4) >> 3;
                CLIP(t1, rgb[0]);
                /* at green pixel */
                rgb[3] = bayer[bayerStep2 + 3];
                t0 = rgb[3] * 5
                    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
                    - bayer[3]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep4 + 3]
                    +
                    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
                      1) >> 1);
                t1 = rgb[3] * 5 +
                    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
                    - bayer[bayerStep2 + 1]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep2 + 5]
                    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
                t0 = (t0 + 4) >> 3;
                CLIP(t0, rgb[2]);
                t1 = (t1 + 4) >> 3;
                CLIP(t1, rgb[4]);
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                /* R at R */
                rgb[-1] = bayer[bayerStep2 + 2];
                /* B at R */
                t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                       bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                    -
                    (((bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                                                     2]) * 3 + 1) >> 1)
                    + rgb[-1] * 6;
                /* G at R */
                t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                       bayer[bayerStep2 + 3] + bayer[bayerStep * 3 +
                                                     2]) << 1)
                    - (bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                    + (rgb[-1] << 2);
                t0 = (t0 + 4) >> 3;
                CLIP(t0, rgb[1]);
                t1 = (t1 + 4) >> 3;
                CLIP(t1, rgb[0]);

                /* at green pixel */
                rgb[3] = bayer[bayerStep2 + 3];
                t0 = rgb[3] * 5
                    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
                    - bayer[3]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep4 + 3]
                    +
                    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
                      1) >> 1);
                t1 = rgb[3] * 5 +
                    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
                    - bayer[bayerStep2 + 1]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep2 + 5]
                    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
                t0 = (t0 + 4) >> 3;
                CLIP(t0, rgb[4]);
                t1 = (t1 + 4) >> 3;
                CLIP(t1, rgb[2]);
            }
        }

        if (bayer < bayerEnd) {
            /* B at B */
            rgb[blue] = bayer[bayerStep2 + 2];
            /* R at B */
            t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                   bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                -
                (((bayer[2] + bayer[bayerStep2] +
                   bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                                                 2]) * 3 + 1) >> 1)
                + rgb[blue] * 6;
            /* G at B */
            t1 = (((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                    bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2])) << 1)
                - (bayer[2] + bayer[bayerStep2] +
                   bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                + (rgb[blue] << 2);
            t0 = (t0 + 4) >> 3;
            CLIP(t0, rgb[-blue]);
            t1 = (t1 + 4) >> 3;
            CLIP(t1, rgb[0]);
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return DC1394_SUCCESS;

}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_HQLINEAR */

/* coriander's Bayer decoding */
/* Edge Sensing Interpolation II from http://www-ise.stanford.edu/~tingchen/ */
/*   (Laroche,Claude A.  "Apparatus and method for adaptively
     interpolating a full color image utilizing chrominance gradients"
     U.S. Patent 5,373,322) */
#if ENABLE_DC1394_BAYER_METHOD_EDGESENSE
dc1394error_t
dc1394_bayer_EdgeSense(const uint8_t *restrict bayer, uint8_t *restrict rgb, int sx, int sy, int tile)
{
    /* Removed due to patent concerns */
    return DC1394_FUNCTION_NOT_SUPPORTED;
}
#endif /*end of ENABLE_DC1394_BAYER_METHOD_EDGESENSE */

/* coriander's Bayer decoding */
#if ENABLE_DC1394_BAYER_METHOD_DOWNSAMPLE
dc1394error_t
dc1394_bayer_Downsample(const uint8_t *restrict bayer, uint8_t *restrict rgb, int sx, int sy, int tile)
{
    uint8_t *outR, *outG, *outB;
    register int i, j;
    int tmp;

    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:
    case DC1394_COLOR_FILTER_BGGR:
        outR = &rgb[0];
        outG = &rgb[1];
        outB = &rgb[2];
        break;
    case DC1394_COLOR_FILTER_GBRG:
    case DC1394_COLOR_FILTER_RGGB:
        outR = &rgb[2];
        outG = &rgb[1];
        outB = &rgb[0];
        break;
    default:
      return DC1394_INVALID_COLOR_FILTER;
    }

    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:        //---------------------------------------------------------
    case DC1394_COLOR_FILTER_GBRG:
        for (i = 0; i < sy*sx; i += (sx<<1)) {
            for (j = 0; j < sx; j += 2) {
                tmp = ((bayer[i + j] + bayer[i + sx + j + 1]) >> 1);
                CLIP(tmp, outG[((i >> 2) + (j >> 1)) * 3]);
                tmp = bayer[i + sx + j + 1];
                CLIP(tmp, outR[((i >> 2) + (j >> 1)) * 3]);
                tmp = bayer[i + sx + j];
                CLIP(tmp, outB[((i >> 2) + (j >> 1)) * 3]);
            }
        }
        break;
    case DC1394_COLOR_FILTER_BGGR:        //---------------------------------------------------------
    case DC1394_COLOR_FILTER_RGGB:
        for (i = 0; i < sy*sx; i += (sx<<1)) {
            for (j = 0; j < sx; j += 2) {
                tmp = ((bayer[i + sx + j] + bayer[i + j + 1]) >> 1);
                CLIP(tmp, outG[((i >> 2) + (j >> 1)) * 3]);
                tmp = bayer[i + sx + j + 1];
                CLIP(tmp, outR[((i >> 2) + (j >> 1)) * 3]);
                tmp = bayer[i + j];
                CLIP(tmp, outB[((i >> 2) + (j >> 1)) * 3]);
            }
        }
        break;
    }

    return DC1394_SUCCESS;

}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_DOWNSAMPLE */

/* this is the method used inside AVT cameras. See AVT docs. */
#if ENABLE_DC1394_BAYER_METHOD_SIMPLE
dc1394error_t
dc1394_bayer_Simple(const uint8_t *restrict bayer, uint8_t *restrict rgb, int sx, int sy, int tile)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;
    int i, imax, iinc;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
      return DC1394_INVALID_COLOR_FILTER;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
        rgb[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
        rgb[i++] = 0;
        rgb[i++] = 0;
        rgb[i++] = 0;
    }

    rgb += 1;
    width -= 1;
    height -= 1;

#if (__ARM_FEATURE_MVE & 1) //ENABLE_MVE_BAYER2RGB 1
	// Index table into 16 RGB pairs for scatter stores: { 0, 6, 12, .. }
	const uint8x16_t inc6 = vmulq(vidupq_n_u8(0, 1), 6);
#endif

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        const uint8_t *bayerEnd = bayer + width;

        if (start_with_green) {
            rgb[-blue] = bayer[1];
            rgb[0] = (bayer[0] + bayer[bayerStep + 1] + 1) >> 1;
            rgb[blue] = bayer[bayerStep];
            bayer++;
            rgb += 3;
        }

#if (__ARM_FEATURE_MVE & 1)
    // Helium lets us process 16 at a time (8 per beat on Cortex-M55)
    int pairs_to_go = (bayerEnd - bayer) / 2;
    while (pairs_to_go > 0) {
        mve_pred16_t p = vctp8q(pairs_to_go);
        uint8x16x2_t rg = vld2q(bayer);
        uint8x16x2_t gb = vld2q(bayer + bayerStep);
        __builtin_prefetch(bayer + bayerStep + 16 * 2 * 2);
        uint8x16_t r0 = rg.val[0];
        uint8x16_t g0 = vrhaddq_x(rg.val[1], gb.val[0], p);
        uint8x16_t b0 = gb.val[1];
        vstrbq_scatter_offset_p(rgb - blue, inc6, r0, p);
        vstrbq_scatter_offset_p(rgb, inc6, g0, p);
        vstrbq_scatter_offset_p(rgb + blue, inc6, b0, p);

        uint8x16x2_t gr = vld2q(bayer + 1);
        uint8x16x2_t bg = vld2q(bayer + bayerStep + 1);
        uint8x16_t r1 = gr.val[1];
        uint8x16_t g1 = vrhaddq_x(gr.val[0], bg.val[1], p);
        uint8x16_t b1 = bg.val[0];
        vstrbq_scatter_offset_p(rgb + 3 - blue, inc6, r1, p);
        vstrbq_scatter_offset_p(rgb + 3, inc6, g1, p);
        vstrbq_scatter_offset_p(rgb + 3 + blue, inc6, b1, p);
        bayer += 16 * 2;
        rgb += 16 * 6;
        pairs_to_go -= 16;
    }

    bayer += pairs_to_go * 2;
    rgb += pairs_to_go * 6;
#else
        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[-1] = bayer[0];
                rgb[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
                rgb[1] = bayer[bayerStep + 1];

                rgb[2] = bayer[2];
                rgb[3] = (bayer[1] + bayer[bayerStep + 2] + 1) >> 1;
                rgb[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[1] = bayer[0];
                rgb[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
                rgb[-1] = bayer[bayerStep + 1];

                rgb[4] = bayer[2];
                rgb[3] = (bayer[1] + bayer[bayerStep + 2] + 1) >> 1;
                rgb[2] = bayer[bayerStep + 1];
            }
        }
#endif
        if (bayer < bayerEnd) {
            rgb[-blue] = bayer[0];
            rgb[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
            rgb[blue] = bayer[bayerStep + 1];
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return DC1394_SUCCESS;

}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_SIMPLE */

#endif /* end of ENABLE_8_BIT_VERSION */

/* 16-bits versions */

#if ENABLE_16_BIT_VERSION
/* insprired by OpenCV's Bayer decoding */
dc1394error_t
dc1394_bayer_NearestNeighbor_uint16(const uint16_t *restrict bayer, uint16_t *restrict rgb, int sx, int sy, int tile, int bits)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;
    int i, iinc, imax;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
      return DC1394_INVALID_COLOR_FILTER;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
        rgb[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
        rgb[i++] = 0;
        rgb[i++] = 0;
        rgb[i++] = 0;
    }

    rgb += 1;
    height -= 1;
    width -= 1;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
      //int t0, t1;
        const uint16_t *bayerEnd = bayer + width;

        if (start_with_green) {
            rgb[-blue] = bayer[1];
            rgb[0] = bayer[bayerStep + 1];
            rgb[blue] = bayer[bayerStep];
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[-1] = bayer[0];
                rgb[0] = bayer[1];
                rgb[1] = bayer[bayerStep + 1];

                rgb[2] = bayer[2];
                rgb[3] = bayer[bayerStep + 2];
                rgb[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                rgb[1] = bayer[0];
                rgb[0] = bayer[1];
                rgb[-1] = bayer[bayerStep + 1];

                rgb[4] = bayer[2];
                rgb[3] = bayer[bayerStep + 2];
                rgb[2] = bayer[bayerStep + 1];
            }
        }

        if (bayer < bayerEnd) {
            rgb[-blue] = bayer[0];
            rgb[0] = bayer[1];
            rgb[blue] = bayer[bayerStep + 1];
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return DC1394_SUCCESS;

}
/* OpenCV's Bayer decoding */
dc1394error_t
dc1394_bayer_Bilinear_uint16(const uint16_t *restrict bayer, uint16_t *restrict rgb, int sx, int sy, int tile, int bits)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
      return DC1394_INVALID_COLOR_FILTER;

    rgb += rgbStep + 3 + 1;
    height -= 2;
    width -= 2;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        int t0, t1;
        const uint16_t *bayerEnd = bayer + width;

        if (start_with_green) {
            /* OpenCV has a bug in the next line, which was
               t0 = (bayer[0] + bayer[bayerStep * 2] + 1) >> 1; */
            t0 = (bayer[1] + bayer[bayerStep * 2 + 1] + 1) >> 1;
            t1 = (bayer[bayerStep] + bayer[bayerStep + 2] + 1) >> 1;
            rgb[-blue] = (uint16_t) t0;
            rgb[0] = bayer[bayerStep + 1];
            rgb[blue] = (uint16_t) t1;
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
                      bayer[bayerStep * 2 + 2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
                      2) >> 2;
                rgb[-1] = (uint16_t) t0;
                rgb[0] = (uint16_t) t1;
                rgb[1] = bayer[bayerStep + 1];

                t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
                t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                      1) >> 1;
                rgb[2] = (uint16_t) t0;
                rgb[3] = bayer[bayerStep + 2];
                rgb[4] = (uint16_t) t1;
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
                      bayer[bayerStep * 2 + 2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
                      2) >> 2;
                rgb[1] = (uint16_t) t0;
                rgb[0] = (uint16_t) t1;
                rgb[-1] = bayer[bayerStep + 1];

                t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
                t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                      1) >> 1;
                rgb[4] = (uint16_t) t0;
                rgb[3] = bayer[bayerStep + 2];
                rgb[2] = (uint16_t) t1;
            }
        }

        if (bayer < bayerEnd) {
            t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
                  bayer[bayerStep * 2 + 2] + 2) >> 2;
            t1 = (bayer[1] + bayer[bayerStep] +
                  bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
                  2) >> 2;
            rgb[-blue] = (uint16_t) t0;
            rgb[0] = (uint16_t) t1;
            rgb[blue] = bayer[bayerStep + 1];
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return DC1394_SUCCESS;

}

/* High-Quality Linear Interpolation For Demosaicing Of
   Bayer-Patterned Color Images, by Henrique S. Malvar, Li-wei He, and
   Ross Cutler, in ICASSP'04 */
dc1394error_t
dc1394_bayer_HQLinear_uint16(const uint16_t *restrict bayer, uint16_t *restrict rgb, int sx, int sy, int tile, int bits)
{
    const int bayerStep = sx;
    const int rgbStep = 3 * sx;
    int width = sx;
    int height = sy;
    /*
       the two letters  of the OpenCV name are respectively
       the 4th and 3rd letters from the blinky name,
       and we also have to switch R and B (OpenCV is BGR)

       CV_BayerBG2BGR <-> DC1394_COLOR_FILTER_BGGR
       CV_BayerGB2BGR <-> DC1394_COLOR_FILTER_GBRG
       CV_BayerGR2BGR <-> DC1394_COLOR_FILTER_GRBG

       int blue = tile == CV_BayerBG2BGR || tile == CV_BayerGB2BGR ? -1 : 1;
       int start_with_green = tile == CV_BayerGB2BGR || tile == CV_BayerGR2BGR;
     */
    int blue = tile == DC1394_COLOR_FILTER_BGGR
        || tile == DC1394_COLOR_FILTER_GBRG ? -1 : 1;
    int start_with_green = tile == DC1394_COLOR_FILTER_GBRG
        || tile == DC1394_COLOR_FILTER_GRBG;

    if ((tile>DC1394_COLOR_FILTER_MAX)||(tile<DC1394_COLOR_FILTER_MIN))
      return DC1394_INVALID_COLOR_FILTER;

    ClearBorders_uint16(rgb, sx, sy, 2);
    rgb += 2 * rgbStep + 6 + 1;
    height -= 4;
    width -= 4;

    /* We begin with a (+1 line,+1 column) offset with respect to bilinear decoding, so start_with_green is the same, but blue is opposite */
    blue = -blue;

    for (; height--; bayer += bayerStep, rgb += rgbStep) {
        int t0, t1;
        const uint16_t *bayerEnd = bayer + width;
        const int bayerStep2 = bayerStep * 2;
        const int bayerStep3 = bayerStep * 3;
        const int bayerStep4 = bayerStep * 4;

        if (start_with_green) {
            /* at green pixel */
            rgb[0] = bayer[bayerStep2 + 2];
            t0 = rgb[0] * 5
                + ((bayer[bayerStep + 2] + bayer[bayerStep3 + 2]) << 2)
                - bayer[2]
                - bayer[bayerStep + 1]
                - bayer[bayerStep + 3]
                - bayer[bayerStep3 + 1]
                - bayer[bayerStep3 + 3]
                - bayer[bayerStep4 + 2]
                + ((bayer[bayerStep2] + bayer[bayerStep2 + 4] + 1) >> 1);
            t1 = rgb[0] * 5 +
                ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 3]) << 2)
                - bayer[bayerStep2]
                - bayer[bayerStep + 1]
                - bayer[bayerStep + 3]
                - bayer[bayerStep3 + 1]
                - bayer[bayerStep3 + 3]
                - bayer[bayerStep2 + 4]
                + ((bayer[2] + bayer[bayerStep4 + 2] + 1) >> 1);
            t0 = (t0 + 4) >> 3;
            CLIP16(t0, rgb[-blue], bits);
            t1 = (t1 + 4) >> 3;
            CLIP16(t1, rgb[blue], bits);
            bayer++;
            rgb += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                /* B at B */
                rgb[1] = bayer[bayerStep2 + 2];
                /* R at B */
                t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                       bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                    -
                    (((bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                                                     2]) * 3 + 1) >> 1)
                    + rgb[1] * 6;
                /* G at B */
                t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                       bayer[bayerStep2 + 3] + bayer[bayerStep * 3 +
                                                     2]) << 1)
                    - (bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                    + (rgb[1] << 2);
                t0 = (t0 + 4) >> 3;
                CLIP16(t0, rgb[-1], bits);
                t1 = (t1 + 4) >> 3;
                CLIP16(t1, rgb[0], bits);
                /* at green pixel */
                rgb[3] = bayer[bayerStep2 + 3];
                t0 = rgb[3] * 5
                    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
                    - bayer[3]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep4 + 3]
                    +
                    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
                      1) >> 1);
                t1 = rgb[3] * 5 +
                    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
                    - bayer[bayerStep2 + 1]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep2 + 5]
                    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
                t0 = (t0 + 4) >> 3;
                CLIP16(t0, rgb[2], bits);
                t1 = (t1 + 4) >> 3;
                CLIP16(t1, rgb[4], bits);
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, rgb += 6) {
                /* R at R */
                rgb[-1] = bayer[bayerStep2 + 2];
                /* B at R */
                t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                       bayer[bayerStep * 3 + 1] + bayer[bayerStep3 +
                                                        3]) << 1)
                    -
                    (((bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                                                     2]) * 3 + 1) >> 1)
                    + rgb[-1] * 6;
                /* G at R */
                t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                       bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2]) << 1)
                    - (bayer[2] + bayer[bayerStep2] +
                       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                    + (rgb[-1] << 2);
                t0 = (t0 + 4) >> 3;
                CLIP16(t0, rgb[1], bits);
                t1 = (t1 + 4) >> 3;
                CLIP16(t1, rgb[0], bits);

                /* at green pixel */
                rgb[3] = bayer[bayerStep2 + 3];
                t0 = rgb[3] * 5
                    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
                    - bayer[3]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep4 + 3]
                    +
                    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
                      1) >> 1);
                t1 = rgb[3] * 5 +
                    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
                    - bayer[bayerStep2 + 1]
                    - bayer[bayerStep + 2]
                    - bayer[bayerStep + 4]
                    - bayer[bayerStep3 + 2]
                    - bayer[bayerStep3 + 4]
                    - bayer[bayerStep2 + 5]
                    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
                t0 = (t0 + 4) >> 3;
                CLIP16(t0, rgb[4], bits);
                t1 = (t1 + 4) >> 3;
                CLIP16(t1, rgb[2], bits);
            }
        }

        if (bayer < bayerEnd) {
            /* B at B */
            rgb[blue] = bayer[bayerStep2 + 2];
            /* R at B */
            t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
                   bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
                -
                (((bayer[2] + bayer[bayerStep2] +
                   bayer[bayerStep2 + 4] + bayer[bayerStep4 +
                                                 2]) * 3 + 1) >> 1)
                + rgb[blue] * 6;
            /* G at B */
            t1 = (((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
                    bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2])) << 1)
                - (bayer[2] + bayer[bayerStep2] +
                   bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
                + (rgb[blue] << 2);
            t0 = (t0 + 4) >> 3;
            CLIP16(t0, rgb[-blue], bits);
            t1 = (t1 + 4) >> 3;
            CLIP16(t1, rgb[0], bits);
            bayer++;
            rgb += 3;
        }

        bayer -= width;
        rgb -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return DC1394_SUCCESS;
}

/* coriander's Bayer decoding */
dc1394error_t
dc1394_bayer_EdgeSense_uint16(const uint16_t *restrict bayer, uint16_t *restrict rgb, int sx, int sy, int tile, int bits)
{
    /* Removed due to patent concerns */
    return DC1394_FUNCTION_NOT_SUPPORTED;
}

/* coriander's Bayer decoding */
dc1394error_t
dc1394_bayer_Downsample_uint16(const uint16_t *restrict bayer, uint16_t *restrict rgb, int sx, int sy, int tile, int bits)
{
    uint16_t *outR, *outG, *outB;
    register int i, j;
    int tmp;

    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:
    case DC1394_COLOR_FILTER_BGGR:
        outR = &rgb[0];
        outG = &rgb[1];
        outB = &rgb[2];
        break;
    case DC1394_COLOR_FILTER_GBRG:
    case DC1394_COLOR_FILTER_RGGB:
        outR = &rgb[2];
        outG = &rgb[1];
        outB = &rgb[0];
        break;
    default:
      return DC1394_INVALID_COLOR_FILTER;
    }

    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:        //---------------------------------------------------------
    case DC1394_COLOR_FILTER_GBRG:
        for (i = 0; i < sy*sx; i += (sx<<1)) {
            for (j = 0; j < sx; j += 2) {
                tmp =
                    ((bayer[i + j] + bayer[i + sx + j + 1]) >> 1);
                CLIP16(tmp, outG[((i >> 2) + (j >> 1)) * 3], bits);
                tmp = bayer[i + sx + j + 1];
                CLIP16(tmp, outR[((i >> 2) + (j >> 1)) * 3], bits);
                tmp = bayer[i + sx + j];
                CLIP16(tmp, outB[((i >> 2) + (j >> 1)) * 3], bits);
            }
        }
        break;
    case DC1394_COLOR_FILTER_BGGR:        //---------------------------------------------------------
    case DC1394_COLOR_FILTER_RGGB:
        for (i = 0; i < sy*sx; i += (sx<<1)) {
            for (j = 0; j < sx; j += 2) {
                tmp =
                    ((bayer[i + sx + j] + bayer[i + j + 1]) >> 1);
                CLIP16(tmp, outG[((i >> 2) + (j >> 1)) * 3], bits);
                tmp = bayer[i + sx + j + 1];
                CLIP16(tmp, outR[((i >> 2) + (j >> 1)) * 3], bits);
                tmp = bayer[i + j];
                CLIP16(tmp, outB[((i >> 2) + (j >> 1)) * 3], bits);
            }
        }
        break;
    }

    return DC1394_SUCCESS;

}

/* coriander's Bayer decoding */
dc1394error_t
dc1394_bayer_Simple_uint16(const uint16_t *restrict bayer, uint16_t *restrict rgb, int sx, int sy, int tile, int bits)
{
    uint16_t *outR, *outG, *outB;
    register int i, j;
    int tmp, base;

    // sx and sy should be even
    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:
    case DC1394_COLOR_FILTER_BGGR:
        outR = &rgb[0];
        outG = &rgb[1];
        outB = &rgb[2];
        break;
    case DC1394_COLOR_FILTER_GBRG:
    case DC1394_COLOR_FILTER_RGGB:
        outR = &rgb[2];
        outG = &rgb[1];
        outB = &rgb[0];
        break;
    default:
      return DC1394_INVALID_COLOR_FILTER;
    }

    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:
    case DC1394_COLOR_FILTER_BGGR:
        outR = &rgb[0];
        outG = &rgb[1];
        outB = &rgb[2];
        break;
    case DC1394_COLOR_FILTER_GBRG:
    case DC1394_COLOR_FILTER_RGGB:
        outR = &rgb[2];
        outG = &rgb[1];
        outB = &rgb[0];
        break;
    default:
        outR = NULL;
        outG = NULL;
        outB = NULL;
        break;
    }

    switch (tile) {
    case DC1394_COLOR_FILTER_GRBG:        //---------------------------------------------------------
    case DC1394_COLOR_FILTER_GBRG:
        for (i = 0; i < sy - 1; i += 2) {
            for (j = 0; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base] + bayer[base + sx + 1]) >> 1);
                CLIP16(tmp, outG[base * 3], bits);
                tmp = bayer[base + 1];
                CLIP16(tmp, outR[base * 3], bits);
                tmp = bayer[base + sx];
                CLIP16(tmp, outB[base * 3], bits);
            }
        }
        for (i = 0; i < sy - 1; i += 2) {
            for (j = 1; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base + 1] + bayer[base + sx]) >> 1);
                CLIP16(tmp, outG[(base) * 3], bits);
                tmp = bayer[base];
                CLIP16(tmp, outR[(base) * 3], bits);
                tmp = bayer[base + 1 + sx];
                CLIP16(tmp, outB[(base) * 3], bits);
            }
        }
        for (i = 1; i < sy - 1; i += 2) {
            for (j = 0; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base + sx] + bayer[base + 1]) >> 1);
                CLIP16(tmp, outG[base * 3], bits);
                tmp = bayer[base + sx + 1];
                CLIP16(tmp, outR[base * 3], bits);
                tmp = bayer[base];
                CLIP16(tmp, outB[base * 3], bits);
            }
        }
        for (i = 1; i < sy - 1; i += 2) {
            for (j = 1; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base] + bayer[base + 1 + sx]) >> 1);
                CLIP16(tmp, outG[(base) * 3], bits);
                tmp = bayer[base + sx];
                CLIP16(tmp, outR[(base) * 3], bits);
                tmp = bayer[base + 1];
                CLIP16(tmp, outB[(base) * 3], bits);
            }
        }
        break;
    case DC1394_COLOR_FILTER_BGGR:        //---------------------------------------------------------
    case DC1394_COLOR_FILTER_RGGB:
        for (i = 0; i < sy - 1; i += 2) {
            for (j = 0; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base + sx] + bayer[base + 1]) >> 1);
                CLIP16(tmp, outG[base * 3], bits);
                tmp = bayer[base + sx + 1];
                CLIP16(tmp, outR[base * 3], bits);
                tmp = bayer[base];
                CLIP16(tmp, outB[base * 3], bits);
            }
        }
        for (i = 1; i < sy - 1; i += 2) {
            for (j = 0; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base] + bayer[base + 1 + sx]) >> 1);
                CLIP16(tmp, outG[(base) * 3], bits);
                tmp = bayer[base + 1];
                CLIP16(tmp, outR[(base) * 3], bits);
                tmp = bayer[base + sx];
                CLIP16(tmp, outB[(base) * 3], bits);
            }
        }
        for (i = 0; i < sy - 1; i += 2) {
            for (j = 1; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base] + bayer[base + sx + 1]) >> 1);
                CLIP16(tmp, outG[base * 3], bits);
                tmp = bayer[base + sx];
                CLIP16(tmp, outR[base * 3], bits);
                tmp = bayer[base + 1];
                CLIP16(tmp, outB[base * 3], bits);
            }
        }
        for (i = 1; i < sy - 1; i += 2) {
            for (j = 1; j < sx - 1; j += 2) {
                base = i * sx + j;
                tmp = ((bayer[base + 1] + bayer[base + sx]) >> 1);
                CLIP16(tmp, outG[(base) * 3], bits);
                tmp = bayer[base];
                CLIP16(tmp, outR[(base) * 3], bits);
                tmp = bayer[base + 1 + sx];
                CLIP16(tmp, outB[(base) * 3], bits);
            }
        }
        break;
    }

    /* add black border */
    for (i = sx * (sy - 1) * 3; i < sx * sy * 3; i++) {
        rgb[i] = 0;
    }
    for (i = (sx - 1) * 3; i < sx * sy * 3; i += (sx - 1) * 3) {
        rgb[i++] = 0;
        rgb[i++] = 0;
        rgb[i++] = 0;
    }

    return DC1394_SUCCESS;

}
#endif /* end of ENABLE_16_BIT_VERSION */

/* Variable Number of Gradients, from dcraw <http://www.cybercom.net/~dcoffin/dcraw/> */
/* Ported to libdc1394 by Frederic Devernay */

#if ENABLE_DC1394_BAYER_METHOD_VNG
#define FORC3 for (c=0; c < 3; c++)

#define SQR(x) ((x)*(x))
#define ABS(x) (((int)(x) ^ ((int)(x) >> 31)) - ((int)(x) >> 31))
#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
  #define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#define LIM(x,min,max) MAX(min,MIN(x,max))
#define ULIM(x,y,z) ((y) < (z) ? LIM(x,y,z) : LIM(x,z,y))
/*
   In order to inline this calculation, I make the risky
   assumption that all filter patterns can be described
   by a repeating pattern of eight rows and two columns

   Return values are either 0/1/2/3 = G/M/C/Y or 0/1/2/3 = R/G1/B/G2
 */
#define FC(row,col) \
        (filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

/*
   This algorithm is officially called:

   "Interpolation using a Threshold-based variable number of gradients"

   described in http://www-ise.stanford.edu/~tingchen/algodep/vargra.html

   I've extended the basic idea to work with non-Bayer filter arrays.
   Gradients are numbered clockwise from NW=0 to W=7.
 */
static const signed char bayervng_terms[] = {
    -2,-2,+0,-1,0,0x01, -2,-2,+0,+0,1,0x01, -2,-1,-1,+0,0,0x01,
    -2,-1,+0,-1,0,0x02, -2,-1,+0,+0,0,0x03, -2,-1,+0,+1,1,0x01,
    -2,+0,+0,-1,0,0x06, -2,+0,+0,+0,1,0x02, -2,+0,+0,+1,0,0x03,
    -2,+1,-1,+0,0,0x04, -2,+1,+0,-1,1,0x04, -2,+1,+0,+0,0,0x06,
    -2,+1,+0,+1,0,0x02, -2,+2,+0,+0,1,0x04, -2,+2,+0,+1,0,0x04,
    -1,-2,-1,+0,0,0x80, -1,-2,+0,-1,0,0x01, -1,-2,+1,-1,0,0x01,
    -1,-2,+1,+0,1,0x01, -1,-1,-1,+1,0,0x88, -1,-1,+1,-2,0,0x40,
    -1,-1,+1,-1,0,0x22, -1,-1,+1,+0,0,0x33, -1,-1,+1,+1,1,0x11,
    -1,+0,-1,+2,0,0x08, -1,+0,+0,-1,0,0x44, -1,+0,+0,+1,0,0x11,
    -1,+0,+1,-2,1,0x40, -1,+0,+1,-1,0,0x66, -1,+0,+1,+0,1,0x22,
    -1,+0,+1,+1,0,0x33, -1,+0,+1,+2,1,0x10, -1,+1,+1,-1,1,0x44,
    -1,+1,+1,+0,0,0x66, -1,+1,+1,+1,0,0x22, -1,+1,+1,+2,0,0x10,
    -1,+2,+0,+1,0,0x04, -1,+2,+1,+0,1,0x04, -1,+2,+1,+1,0,0x04,
    +0,-2,+0,+0,1,0x80, +0,-1,+0,+1,1,0x88, +0,-1,+1,-2,0,0x40,
    +0,-1,+1,+0,0,0x11, +0,-1,+2,-2,0,0x40, +0,-1,+2,-1,0,0x20,
    +0,-1,+2,+0,0,0x30, +0,-1,+2,+1,1,0x10, +0,+0,+0,+2,1,0x08,
    +0,+0,+2,-2,1,0x40, +0,+0,+2,-1,0,0x60, +0,+0,+2,+0,1,0x20,
    +0,+0,+2,+1,0,0x30, +0,+0,+2,+2,1,0x10, +0,+1,+1,+0,0,0x44,
    +0,+1,+1,+2,0,0x10, +0,+1,+2,-1,1,0x40, +0,+1,+2,+0,0,0x60,
    +0,+1,+2,+1,0,0x20, +0,+1,+2,+2,0,0x10, +1,-2,+1,+0,0,0x80,
    +1,-1,+1,+1,0,0x88, +1,+0,+1,+2,0,0x08, +1,+0,+2,-1,0,0x40,
    +1,+0,+2,+1,0,0x10
}, bayervng_chood[] = { -1,-1, -1,0, -1,+1, 0,+1, +1,+1, +1,0, +1,-1, 0,-1 };

dc1394error_t
dc1394_bayer_VNG(const uint8_t *restrict bayer,
                 uint8_t *restrict dst, int sx, int sy,
                 dc1394color_filter_t pattern)
{
    const int height = sy, width = sx;
    static const signed char *cp;
    /* the following has the same type as the image */
    uint8_t (*brow[5])[3], *pix;          /* [FD] */
    int code[8][2][320], *ip, gval[8], gmin, gmax, sum[4];
    int row, col, x, y, x1, x2, y1, y2, t, weight, grads, color, diag;
    int g, diff, thold, num, c;
    uint32_t filters;                     /* [FD] */

    /* first, use bilinear bayer decoding */
    dc1394_bayer_Bilinear(bayer, dst, sx, sy, pattern);

    switch(pattern) {
    case DC1394_COLOR_FILTER_BGGR:
        filters = 0x16161616;
        break;
    case DC1394_COLOR_FILTER_GRBG:
        filters = 0x61616161;
        break;
    case DC1394_COLOR_FILTER_RGGB:
        filters = 0x94949494;
        break;
    case DC1394_COLOR_FILTER_GBRG:
        filters = 0x49494949;
        break;
    default:
        return DC1394_INVALID_COLOR_FILTER;
    }

    for (row=0; row < 8; row++) {                /* Precalculate for VNG */
        for (col=0; col < 2; col++) {
            ip = code[row][col];
            for (cp=bayervng_terms, t=0; t < 64; t++) {
                y1 = *cp++;  x1 = *cp++;
                y2 = *cp++;  x2 = *cp++;
                weight = *cp++;
                grads = *cp++;
                color = FC(row+y1,col+x1);
                if (FC(row+y2,col+x2) != color) continue;
                diag = (FC(row,col+1) == color && FC(row+1,col) == color) ? 2:1;
                if (abs(y1-y2) == diag && abs(x1-x2) == diag) continue;
                *ip++ = (y1*width + x1)*3 + color; /* [FD] */
                *ip++ = (y2*width + x2)*3 + color; /* [FD] */
                *ip++ = weight;
                for (g=0; g < 8; g++)
                    if (grads & 1<<g) *ip++ = g;
                *ip++ = -1;
            }
            *ip++ = INT_MAX;
            for (cp=bayervng_chood, g=0; g < 8; g++) {
                y = *cp++;  x = *cp++;
                *ip++ = (y*width + x) * 3;      /* [FD] */
                color = FC(row,col);
                if (FC(row+y,col+x) != color && FC(row+y*2,col+x*2) == color)
                    *ip++ = (y*width + x) * 6 + color; /* [FD] */
                else
                    *ip++ = 0;
            }
        }
    }
    brow[4] = calloc (width*3, sizeof **brow);
    //merror (brow[4], "vng_interpolate()");
    for (row=0; row < 3; row++)
        brow[row] = brow[4] + row*width;
    for (row=2; row < height-2; row++) {                /* Do VNG interpolation */
        for (col=2; col < width-2; col++) {
            pix = dst + (row*width+col)*3;        /* [FD] */
            ip = code[row & 7][col & 1];
            memset (gval, 0, sizeof gval);
            while ((g = ip[0]) != INT_MAX) {                /* Calculate gradients */
                diff = ABS(pix[g] - pix[ip[1]]) << ip[2];
                gval[ip[3]] += diff;
                ip += 5;
                if ((g = ip[-1]) == -1) continue;
                gval[g] += diff;
                while ((g = *ip++) != -1)
                    gval[g] += diff;
            }
            ip++;
            gmin = gmax = gval[0];                        /* Choose a threshold */
            for (g=1; g < 8; g++) {
                if (gmin > gval[g]) gmin = gval[g];
                if (gmax < gval[g]) gmax = gval[g];
            }
            if (gmax == 0) {
                memcpy (brow[2][col], pix, 3 * sizeof *dst); /* [FD] */
                continue;
            }
            thold = gmin + (gmax >> 1);
            memset (sum, 0, sizeof sum);
            color = FC(row,col);
            for (num=g=0; g < 8; g++,ip+=2) {                /* Average the neighbors */
                if (gval[g] <= thold) {
                    for (c=0; c < 3; c++)         /* [FD] */
                        if (c == color && ip[1])
                            sum[c] += (pix[c] + pix[ip[1]]) >> 1;
                        else
                            sum[c] += pix[ip[0] + c];
                    num++;
                }
            }
            for (c=0; c < 3; c++) {               /* [FD] Save to buffer */
                t = pix[color];
                if (c != color)
                    t += (sum[c] - sum[color]) / num;
                CLIP(t,brow[2][col][c]);          /* [FD] */
            }
        }
        if (row > 3)                                /* Write buffer to image */
            memcpy (dst + 3*((row-2)*width+2), brow[0]+2, (width-4)*3*sizeof *dst); /* [FD] */
        for (g=0; g < 4; g++)
            brow[(g-1) & 3] = brow[g];
    }
    memcpy (dst + 3*((row-2)*width+2), brow[0]+2, (width-4)*3*sizeof *dst);
    memcpy (dst + 3*((row-1)*width+2), brow[1]+2, (width-4)*3*sizeof *dst);
    free (brow[4]);

    return DC1394_SUCCESS;
}


dc1394error_t
dc1394_bayer_VNG_uint16(const uint16_t *restrict bayer,
                        uint16_t *restrict dst, int sx, int sy,
                        dc1394color_filter_t pattern, int bits)
{
    const int height = sy, width = sx;
    static const signed char *cp;
    /* the following has the same type as the image */
    uint16_t (*brow[5])[3], *pix;          /* [FD] */
    int code[8][2][320], *ip, gval[8], gmin, gmax, sum[4];
    int row, col, x, y, x1, x2, y1, y2, t, weight, grads, color, diag;
    int g, diff, thold, num, c;
    uint32_t filters;                     /* [FD] */

    /* first, use bilinear bayer decoding */

    dc1394_bayer_Bilinear_uint16(bayer, dst, sx, sy, pattern, bits);

    switch(pattern) {
    case DC1394_COLOR_FILTER_BGGR:
        filters = 0x16161616;
        break;
    case DC1394_COLOR_FILTER_GRBG:
        filters = 0x61616161;
        break;
    case DC1394_COLOR_FILTER_RGGB:
        filters = 0x94949494;
        break;
    case DC1394_COLOR_FILTER_GBRG:
        filters = 0x49494949;
        break;
    default:
        return DC1394_INVALID_COLOR_FILTER;
    }

    for (row=0; row < 8; row++) {                /* Precalculate for VNG */
        for (col=0; col < 2; col++) {
            ip = code[row][col];
            for (cp=bayervng_terms, t=0; t < 64; t++) {
                y1 = *cp++;  x1 = *cp++;
                y2 = *cp++;  x2 = *cp++;
                weight = *cp++;
                grads = *cp++;
                color = FC(row+y1,col+x1);
                if (FC(row+y2,col+x2) != color) continue;
                diag = (FC(row,col+1) == color && FC(row+1,col) == color) ? 2:1;
                if (abs(y1-y2) == diag && abs(x1-x2) == diag) continue;
                *ip++ = (y1*width + x1)*3 + color; /* [FD] */
                *ip++ = (y2*width + x2)*3 + color; /* [FD] */
                *ip++ = weight;
                for (g=0; g < 8; g++)
                    if (grads & 1<<g) *ip++ = g;
                *ip++ = -1;
            }
            *ip++ = INT_MAX;
            for (cp=bayervng_chood, g=0; g < 8; g++) {
                y = *cp++;  x = *cp++;
                *ip++ = (y*width + x) * 3;      /* [FD] */
                color = FC(row,col);
                if (FC(row+y,col+x) != color && FC(row+y*2,col+x*2) == color)
                    *ip++ = (y*width + x) * 6 + color; /* [FD] */
                else
                    *ip++ = 0;
            }
        }
    }
    brow[4] = calloc (width*3, sizeof **brow);
    //merror (brow[4], "vng_interpolate()");
    for (row=0; row < 3; row++)
        brow[row] = brow[4] + row*width;
    for (row=2; row < height-2; row++) {                /* Do VNG interpolation */
        for (col=2; col < width-2; col++) {
            pix = dst + (row*width+col)*3;  /* [FD] */
            ip = code[row & 7][col & 1];
            memset (gval, 0, sizeof gval);
            while ((g = ip[0]) != INT_MAX) {                /* Calculate gradients */
                diff = ABS(pix[g] - pix[ip[1]]) << ip[2];
                gval[ip[3]] += diff;
                ip += 5;
                if ((g = ip[-1]) == -1) continue;
                gval[g] += diff;
                while ((g = *ip++) != -1)
                    gval[g] += diff;
            }
            ip++;
            gmin = gmax = gval[0];                        /* Choose a threshold */
            for (g=1; g < 8; g++) {
                if (gmin > gval[g]) gmin = gval[g];
                if (gmax < gval[g]) gmax = gval[g];
            }
            if (gmax == 0) {
                memcpy (brow[2][col], pix, 3 * sizeof *dst); /* [FD] */
                continue;
            }
            thold = gmin + (gmax >> 1);
            memset (sum, 0, sizeof sum);
            color = FC(row,col);
            for (num=g=0; g < 8; g++,ip+=2) {                /* Average the neighbors */
                if (gval[g] <= thold) {
                    for (c=0; c < 3; c++)         /* [FD] */
                        if (c == color && ip[1])
                            sum[c] += (pix[c] + pix[ip[1]]) >> 1;
                        else
                            sum[c] += pix[ip[0] + c];
                    num++;
                }
            }
            for (c=0; c < 3; c++) {           /* [FD] Save to buffer */
                t = pix[color];
                if (c != color)
                    t += (sum[c] - sum[color]) / num;
                CLIP16(t,brow[2][col][c],bits);        /* [FD] */
            }
        }
        if (row > 3)                                /* Write buffer to image */
            memcpy (dst + 3*((row-2)*width+2), brow[0]+2, (width-4)*3*sizeof *dst); /* [FD] */
        for (g=0; g < 4; g++)
            brow[(g-1) & 3] = brow[g];
    }
    memcpy (dst + 3*((row-2)*width+2), brow[0]+2, (width-4)*3*sizeof *dst);
    memcpy (dst + 3*((row-1)*width+2), brow[1]+2, (width-4)*3*sizeof *dst);
    free (brow[4]);

    return DC1394_SUCCESS;
}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_VNG */



/* AHD interpolation ported from dcraw to libdc1394 by Samuel Audet */
#if ENABLE_DC1394_BAYER_METHOD_AHD
static dc1394bool_t ahd_inited = DC1394_FALSE; /* WARNING: not multi-processor safe */

#define CLIPOUT(x)        LIM(x,0,255)
#define CLIPOUT16(x,bits) LIM(x,0,((1<<bits)-1))

static const double xyz_rgb[3][3] = {                        /* XYZ from RGB */
  { 0.412453, 0.357580, 0.180423 },
  { 0.212671, 0.715160, 0.072169 },
  { 0.019334, 0.119193, 0.950227 } };
static const float d65_white[3] = { 0.950456, 1, 1.088754 };

static void cam_to_cielab (uint16_t cam[3], float lab[3]) /* [SA] */
{
    int c, i, j;
    float r, xyz[3];
    static float cbrt[0x10000], xyz_cam[3][4];

    if (cam == NULL) {
        for (i=0; i < 0x10000; i++) {
            r = i / 65535.0;
            cbrt[i] = r > 0.008856 ? pow(r,1/3.0) : 7.787*r + 16/116.0;
        }
        for (i=0; i < 3; i++)
            for (j=0; j < 3; j++)                           /* [SA] */
                xyz_cam[i][j] = xyz_rgb[i][j] / d65_white[i]; /* [SA] */
    } else {
        xyz[0] = xyz[1] = xyz[2] = 0.5;
        FORC3 { /* [SA] */
            xyz[0] += xyz_cam[0][c] * cam[c];
            xyz[1] += xyz_cam[1][c] * cam[c];
            xyz[2] += xyz_cam[2][c] * cam[c];
        }
        xyz[0] = cbrt[CLIPOUT16((int) xyz[0],16)];        /* [SA] */
        xyz[1] = cbrt[CLIPOUT16((int) xyz[1],16)];        /* [SA] */
        xyz[2] = cbrt[CLIPOUT16((int) xyz[2],16)];        /* [SA] */
        lab[0] = 116 * xyz[1] - 16;
        lab[1] = 500 * (xyz[0] - xyz[1]);
        lab[2] = 200 * (xyz[1] - xyz[2]);
    }
}

/*
   Adaptive Homogeneity-Directed interpolation is based on
   the work of Keigo Hirakawa, Thomas Parks, and Paul Lee.
 */
#define TS 256                /* Tile Size */

dc1394error_t
dc1394_bayer_AHD(const uint8_t *restrict bayer,
                 uint8_t *restrict dst, int sx, int sy,
                 dc1394color_filter_t pattern)
{
    int i, j, top, left, row, col, tr, tc, fc, c, d, val, hm[2];
    /* the following has the same type as the image */
    uint8_t (*pix)[3], (*rix)[3];      /* [SA] */
    uint16_t rix16[3];                 /* [SA] */
    static const int dir[4] = { -1, 1, -TS, TS };
    unsigned ldiff[2][4], abdiff[2][4], leps, abeps;
    float flab[3];                     /* [SA] */
    uint8_t (*rgb)[TS][TS][3];
    short (*lab)[TS][TS][3];
    char (*homo)[TS][TS], *buffer;

    /* start - new code for libdc1394 */
    uint32_t filters;
    const int height = sy, width = sx;
    int x, y;

    if (ahd_inited==DC1394_FALSE) {
        /* WARNING: this might not be multi-processor safe */
        cam_to_cielab (NULL,NULL);
        ahd_inited = DC1394_TRUE;
    }

    switch(pattern) {
    case DC1394_COLOR_FILTER_BGGR:
        filters = 0x16161616;
        break;
    case DC1394_COLOR_FILTER_GRBG:
        filters = 0x61616161;
        break;
    case DC1394_COLOR_FILTER_RGGB:
        filters = 0x94949494;
        break;
    case DC1394_COLOR_FILTER_GBRG:
        filters = 0x49494949;
        break;
    default:
        return DC1394_INVALID_COLOR_FILTER;
    }

    /* fill-in destination with known exact values */
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int channel = FC(y,x);
            dst[(y*width+x)*3 + channel] = bayer[y*width+x];
        }
    }
    /* end - new code for libdc1394 */

    /* start - code from border_interpolate (int border) */
    {
        int border = 3;
        unsigned row, col, y, x, f, c, sum[8];

        for (row=0; row < height; row++)
            for (col=0; col < width; col++) {
                if (col==border && row >= border && row < height-border)
                    col = width-border;
                memset (sum, 0, sizeof sum);
                for (y=row-1; y != row+2; y++)
                    for (x=col-1; x != col+2; x++)
                        if (y < height && x < width) {
                            f = FC(y,x);
                            sum[f] += dst[(y*width+x)*3 + f];           /* [SA] */
                            sum[f+4]++;
                        }
                f = FC(row,col);
                FORC3 if (c != f && sum[c+4])                     /* [SA] */
                    dst[(row*width+col)*3 + c] = sum[c] / sum[c+4]; /* [SA] */
            }
    }
    /* end - code from border_interpolate (int border) */


    buffer = (char *) malloc (26*TS*TS);                /* 1664 kB */
    /* merror (buffer, "ahd_interpolate()"); */
    rgb  = (uint8_t(*)[TS][TS][3]) buffer;                /* [SA] */
    lab  = (short (*)[TS][TS][3])(buffer + 12*TS*TS);
    homo = (char  (*)[TS][TS])   (buffer + 24*TS*TS);

    for (top=0; top < height; top += TS-6)
        for (left=0; left < width; left += TS-6) {
            memset (rgb, 0, 12*TS*TS);

            /*  Interpolate green horizontally and vertically:                */
            for (row = top < 2 ? 2:top; row < top+TS && row < height-2; row++) {
                col = left + (FC(row,left) == 1);
                if (col < 2) col += 2;
                for (fc = FC(row,col); col < left+TS && col < width-2; col+=2) {
                    pix = (uint8_t (*)[3])dst + (row*width+col);          /* [SA] */
                    val = ((pix[-1][1] + pix[0][fc] + pix[1][1]) * 2
                           - pix[-2][fc] - pix[2][fc]) >> 2;
                    rgb[0][row-top][col-left][1] = ULIM(val,pix[-1][1],pix[1][1]);
                    val = ((pix[-width][1] + pix[0][fc] + pix[width][1]) * 2
                           - pix[-2*width][fc] - pix[2*width][fc]) >> 2;
                    rgb[1][row-top][col-left][1] = ULIM(val,pix[-width][1],pix[width][1]);
                }
            }
            /*  Interpolate red and blue, and convert to CIELab:                */
            for (d=0; d < 2; d++)
                for (row=top+1; row < top+TS-1 && row < height-1; row++)
                    for (col=left+1; col < left+TS-1 && col < width-1; col++) {
                        pix = (uint8_t (*)[3])dst + (row*width+col);        /* [SA] */
                        rix = &rgb[d][row-top][col-left];
                        if ((c = 2 - FC(row,col)) == 1) {
                            c = FC(row+1,col);
                            val = pix[0][1] + (( pix[-1][2-c] + pix[1][2-c]
                                                 - rix[-1][1] - rix[1][1] ) >> 1);
                            rix[0][2-c] = CLIPOUT(val);         /* [SA] */
                            val = pix[0][1] + (( pix[-width][c] + pix[width][c]
                                                 - rix[-TS][1] - rix[TS][1] ) >> 1);
                        } else
                            val = rix[0][1] + (( pix[-width-1][c] + pix[-width+1][c]
                                                 + pix[+width-1][c] + pix[+width+1][c]
                                                 - rix[-TS-1][1] - rix[-TS+1][1]
                                                 - rix[+TS-1][1] - rix[+TS+1][1] + 1) >> 2);
                        rix[0][c] = CLIPOUT(val);             /* [SA] */
                        c = FC(row,col);
                        rix[0][c] = pix[0][c];
                        rix16[0] = rix[0][0];                 /* [SA] */
                        rix16[1] = rix[0][1];                 /* [SA] */
                        rix16[2] = rix[0][2];                 /* [SA] */
                        cam_to_cielab (rix16, flab);          /* [SA] */
                        FORC3 lab[d][row-top][col-left][c] = 64*flab[c];
                    }
            /*  Build homogeneity maps from the CIELab images:                */
            memset (homo, 0, 2*TS*TS);
            for (row=top+2; row < top+TS-2 && row < height; row++) {
                tr = row-top;
                for (col=left+2; col < left+TS-2 && col < width; col++) {
                    tc = col-left;
                    for (d=0; d < 2; d++)
                        for (i=0; i < 4; i++)
                            ldiff[d][i] = ABS(lab[d][tr][tc][0]-lab[d][tr][tc+dir[i]][0]);
                    leps = MIN(MAX(ldiff[0][0],ldiff[0][1]),
                               MAX(ldiff[1][2],ldiff[1][3]));
                    for (d=0; d < 2; d++)
                        for (i=0; i < 4; i++)
                            if (i >> 1 == d || ldiff[d][i] <= leps)
                                abdiff[d][i] = SQR(lab[d][tr][tc][1]-lab[d][tr][tc+dir[i]][1])
                                    + SQR(lab[d][tr][tc][2]-lab[d][tr][tc+dir[i]][2]);
                    abeps = MIN(MAX(abdiff[0][0],abdiff[0][1]),
                                MAX(abdiff[1][2],abdiff[1][3]));
                    for (d=0; d < 2; d++)
                        for (i=0; i < 4; i++)
                            if (ldiff[d][i] <= leps && abdiff[d][i] <= abeps)
                                homo[d][tr][tc]++;
                }
            }
            /*  Combine the most homogenous pixels for the final result:        */
            for (row=top+3; row < top+TS-3 && row < height-3; row++) {
                tr = row-top;
                for (col=left+3; col < left+TS-3 && col < width-3; col++) {
                    tc = col-left;
                    for (d=0; d < 2; d++)
                        for (hm[d]=0, i=tr-1; i <= tr+1; i++)
                            for (j=tc-1; j <= tc+1; j++)
                                hm[d] += homo[d][i][j];
                    if (hm[0] != hm[1])
                        FORC3 dst[(row*width+col)*3 + c] = CLIPOUT(rgb[hm[1] > hm[0]][tr][tc][c]); /* [SA] */
                    else
                        FORC3 dst[(row*width+col)*3 + c] =
                            CLIPOUT((rgb[0][tr][tc][c] + rgb[1][tr][tc][c]) >> 1);      /* [SA] */
                }
            }
        }
    free (buffer);

    return DC1394_SUCCESS;
}

dc1394error_t
dc1394_bayer_AHD_uint16(const uint16_t *restrict bayer,
                        uint16_t *restrict dst, int sx, int sy,
                        dc1394color_filter_t pattern, int bits)
{
    int i, j, top, left, row, col, tr, tc, fc, c, d, val, hm[2];
    /* the following has the same type as the image */
    uint16_t (*pix)[3], (*rix)[3];      /* [SA] */
    static const int dir[4] = { -1, 1, -TS, TS };
    unsigned ldiff[2][4], abdiff[2][4], leps, abeps;
    float flab[3];
    uint16_t (*rgb)[TS][TS][3];         /* [SA] */
    short (*lab)[TS][TS][3];
    char (*homo)[TS][TS], *buffer;

    /* start - new code for libdc1394 */
    uint32_t filters;
    const int height = sy, width = sx;
    int x, y;

    if (ahd_inited==DC1394_FALSE) {
        /* WARNING: this might not be multi-processor safe */
        cam_to_cielab (NULL,NULL);
        ahd_inited = DC1394_TRUE;
    }

    switch(pattern) {
    case DC1394_COLOR_FILTER_BGGR:
        filters = 0x16161616;
        break;
    case DC1394_COLOR_FILTER_GRBG:
        filters = 0x61616161;
        break;
    case DC1394_COLOR_FILTER_RGGB:
        filters = 0x94949494;
        break;
    case DC1394_COLOR_FILTER_GBRG:
        filters = 0x49494949;
        break;
    default:
        return DC1394_INVALID_COLOR_FILTER;
    }

    /* fill-in destination with known exact values */
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int channel = FC(y,x);
            dst[(y*width+x)*3 + channel] = bayer[y*width+x];
        }
    }
    /* end - new code for libdc1394 */

    /* start - code from border_interpolate(int border) */
    {
        int border = 3;
        unsigned row, col, y, x, f, c, sum[8];

        for (row=0; row < height; row++)
            for (col=0; col < width; col++) {
                if (col==border && row >= border && row < height-border)
        col = width-border;
                memset (sum, 0, sizeof sum);
                for (y=row-1; y != row+2; y++)
                    for (x=col-1; x != col+2; x++)
                        if (y < height && x < width) {
                            f = FC(y,x);
                            sum[f] += dst[(y*width+x)*3 + f];           /* [SA] */
                            sum[f+4]++;
                        }
                f = FC(row,col);
                FORC3 if (c != f && sum[c+4])                     /* [SA] */
                    dst[(row*width+col)*3 + c] = sum[c] / sum[c+4]; /* [SA] */
            }
    }
    /* end - code from border_interpolate(int border) */


    buffer = (char *) malloc (26*TS*TS);                /* 1664 kB */
    /* merror (buffer, "ahd_interpolate()"); */
    rgb  = (uint16_t(*)[TS][TS][3]) buffer;               /* [SA] */
    lab  = (short (*)[TS][TS][3])(buffer + 12*TS*TS);
    homo = (char  (*)[TS][TS])   (buffer + 24*TS*TS);

    for (top=0; top < height; top += TS-6)
        for (left=0; left < width; left += TS-6) {
            memset (rgb, 0, 12*TS*TS);

            /*  Interpolate green horizontally and vertically:                */
            for (row = top < 2 ? 2:top; row < top+TS && row < height-2; row++) {
                col = left + (FC(row,left) == 1);
                if (col < 2) col += 2;
                for (fc = FC(row,col); col < left+TS && col < width-2; col+=2) {
                    pix = (uint16_t (*)[3])dst + (row*width+col);          /* [SA] */
                    val = ((pix[-1][1] + pix[0][fc] + pix[1][1]) * 2
                           - pix[-2][fc] - pix[2][fc]) >> 2;
                    rgb[0][row-top][col-left][1] = ULIM(val,pix[-1][1],pix[1][1]);
                    val = ((pix[-width][1] + pix[0][fc] + pix[width][1]) * 2
                           - pix[-2*width][fc] - pix[2*width][fc]) >> 2;
                    rgb[1][row-top][col-left][1] = ULIM(val,pix[-width][1],pix[width][1]);
                }
            }
            /*  Interpolate red and blue, and convert to CIELab:                */
            for (d=0; d < 2; d++)
                for (row=top+1; row < top+TS-1 && row < height-1; row++)
                    for (col=left+1; col < left+TS-1 && col < width-1; col++) {
                        pix = (uint16_t (*)[3])dst + (row*width+col);        /* [SA] */
                        rix = &rgb[d][row-top][col-left];
                        if ((c = 2 - FC(row,col)) == 1) {
                            c = FC(row+1,col);
                            val = pix[0][1] + (( pix[-1][2-c] + pix[1][2-c]
                                                 - rix[-1][1] - rix[1][1] ) >> 1);
                            rix[0][2-c] = CLIPOUT16(val, bits); /* [SA] */
                            val = pix[0][1] + (( pix[-width][c] + pix[width][c]
                                                 - rix[-TS][1] - rix[TS][1] ) >> 1);
                        } else
                            val = rix[0][1] + (( pix[-width-1][c] + pix[-width+1][c]
                                                 + pix[+width-1][c] + pix[+width+1][c]
                                                 - rix[-TS-1][1] - rix[-TS+1][1]
                                                 - rix[+TS-1][1] - rix[+TS+1][1] + 1) >> 2);
                        rix[0][c] = CLIPOUT16(val, bits);     /* [SA] */
                        c = FC(row,col);
                        rix[0][c] = pix[0][c];
                        cam_to_cielab (rix[0], flab);
                        FORC3 lab[d][row-top][col-left][c] = 64*flab[c];
                    }
            /*  Build homogeneity maps from the CIELab images:                */
            memset (homo, 0, 2*TS*TS);
            for (row=top+2; row < top+TS-2 && row < height; row++) {
                tr = row-top;
                for (col=left+2; col < left+TS-2 && col < width; col++) {
                    tc = col-left;
                    for (d=0; d < 2; d++)
                        for (i=0; i < 4; i++)
                            ldiff[d][i] = ABS(lab[d][tr][tc][0]-lab[d][tr][tc+dir[i]][0]);
                    leps = MIN(MAX(ldiff[0][0],ldiff[0][1]),
                               MAX(ldiff[1][2],ldiff[1][3]));
                    for (d=0; d < 2; d++)
                        for (i=0; i < 4; i++)
                            if (i >> 1 == d || ldiff[d][i] <= leps)
                                abdiff[d][i] = SQR(lab[d][tr][tc][1]-lab[d][tr][tc+dir[i]][1])
                                    + SQR(lab[d][tr][tc][2]-lab[d][tr][tc+dir[i]][2]);
                    abeps = MIN(MAX(abdiff[0][0],abdiff[0][1]),
                                MAX(abdiff[1][2],abdiff[1][3]));
                    for (d=0; d < 2; d++)
                        for (i=0; i < 4; i++)
                            if (ldiff[d][i] <= leps && abdiff[d][i] <= abeps)
                                homo[d][tr][tc]++;
                }
            }
            /*  Combine the most homogenous pixels for the final result:        */
            for (row=top+3; row < top+TS-3 && row < height-3; row++) {
                tr = row-top;
                for (col=left+3; col < left+TS-3 && col < width-3; col++) {
                    tc = col-left;
                    for (d=0; d < 2; d++)
                        for (hm[d]=0, i=tr-1; i <= tr+1; i++)
                            for (j=tc-1; j <= tc+1; j++)
                                hm[d] += homo[d][i][j];
                    if (hm[0] != hm[1])
                        FORC3 dst[(row*width+col)*3 + c] = CLIPOUT16(rgb[hm[1] > hm[0]][tr][tc][c], bits); /* [SA] */
                    else
                        FORC3 dst[(row*width+col)*3 + c] =
                            CLIPOUT16((rgb[0][tr][tc][c] + rgb[1][tr][tc][c]) >> 1, bits); /* [SA] */
                }
            }
        }
    free (buffer);

    return DC1394_SUCCESS;
}
#endif /* end of ENABLE_DC1394_BAYER_METHOD_AHD.*/

#if ENABLE_8_BIT_VERSION
dc1394error_t
dc1394_bayer_decoding_8bit(const uint8_t *restrict bayer, uint8_t *restrict rgb, uint32_t sx, uint32_t sy, dc1394color_filter_t tile, dc1394bayer_method_t method)
{
    switch (method) {
#if ENABLE_DC1394_BAYER_METHOD_NEAREST
    case DC1394_BAYER_METHOD_NEAREST:
        return dc1394_bayer_NearestNeighbor(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_SIMPLE
    case DC1394_BAYER_METHOD_SIMPLE:
        return dc1394_bayer_Simple(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_BILINEAR
    case DC1394_BAYER_METHOD_BILINEAR:
        return dc1394_bayer_Bilinear(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_HQLINEAR
    case DC1394_BAYER_METHOD_HQLINEAR:
        return dc1394_bayer_HQLinear(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_DOWNSAMPLE
    case DC1394_BAYER_METHOD_DOWNSAMPLE:
        return dc1394_bayer_Downsample(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_EDGESENSE
    case DC1394_BAYER_METHOD_EDGESENSE:
        return dc1394_bayer_EdgeSense(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_VNG
    case DC1394_BAYER_METHOD_VNG:
        return dc1394_bayer_VNG(bayer, rgb, sx, sy, tile);
#endif

#if ENABLE_DC1394_BAYER_METHOD_AHD
    case DC1394_BAYER_METHOD_AHD:
        return dc1394_bayer_AHD(bayer, rgb, sx, sy, tile);
#endif

    default:
        return DC1394_INVALID_BAYER_METHOD;
  }

}
#endif /* end of ENABLE_8_BIT_VERSION */

#if ENABLE_16_BIT_VERSION
dc1394error_t
dc1394_bayer_decoding_16bit(const uint16_t *restrict bayer, uint16_t *restrict rgb, uint32_t sx, uint32_t sy, dc1394color_filter_t tile, dc1394bayer_method_t method, uint32_t bits)
{
    switch (method) {
    case DC1394_BAYER_METHOD_NEAREST:
        return dc1394_bayer_NearestNeighbor_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_SIMPLE:
        return dc1394_bayer_Simple_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_BILINEAR:
        return dc1394_bayer_Bilinear_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_HQLINEAR:
        return dc1394_bayer_HQLinear_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_DOWNSAMPLE:
        return dc1394_bayer_Downsample_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_EDGESENSE:
        return dc1394_bayer_EdgeSense_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_VNG:
        return dc1394_bayer_VNG_uint16(bayer, rgb, sx, sy, tile, bits);
    case DC1394_BAYER_METHOD_AHD:
        return dc1394_bayer_AHD_uint16(bayer, rgb, sx, sy, tile, bits);
    default:
        return DC1394_INVALID_BAYER_METHOD;
    }

}
#endif /* end of ENABLE_16_BIT_VERSION */


#if 0
dc1394error_t
Adapt_buffer_bayer(dc1394video_frame_t *in, dc1394video_frame_t *out, dc1394bayer_method_t method)
{
    uint32_t bpp;

    // conversions will halve the buffer size if the method is DOWNSAMPLE:
    out->size[0]=in->size[0];
    out->size[1]=in->size[1];
    if (method == DC1394_BAYER_METHOD_DOWNSAMPLE) {
        out->size[0]/=2; // ODD SIZE CASES NOT TAKEN INTO ACCOUNT
        out->size[1]/=2;
    }

    // as a convention we divide the image position by two in the case of a DOWNSAMPLE:
    out->position[0]=in->position[0];
    out->position[1]=in->position[1];
    if (method == DC1394_BAYER_METHOD_DOWNSAMPLE) {
        out->position[0]/=2;
        out->position[1]/=2;
    }

    // the destination color coding is ALWAYS RGB. Set this.
    if ( (in->color_coding==DC1394_COLOR_CODING_RAW16) ||
	 (in->color_coding==DC1394_COLOR_CODING_MONO16) )
        out->color_coding=DC1394_COLOR_CODING_RGB16;
    else
        out->color_coding=DC1394_COLOR_CODING_RGB8;

    // keep the color filter value in all cases. If the format is not raw it will not be further used anyway
    out->color_filter=in->color_filter;

    // The output is never YUV, hence nothing to do about YUV byte order

    // bit depth is conserved for 16 bit and set to 8bit for 8bit:
    if ( (in->color_coding==DC1394_COLOR_CODING_RAW16) ||
	 (in->color_coding==DC1394_COLOR_CODING_MONO16) )
        out->data_depth=in->data_depth;
    else
        out->data_depth=8;

    // don't know what to do with stride... >>>> TODO: STRIDE SHOULD BE TAKEN INTO ACCOUNT... <<<<
    // out->stride=??

    // the video mode should not change. Color coding and other stuff can be accessed in specific fields of this struct
    out->video_mode = in->video_mode;

    // padding is kept:
    out->padding_bytes = in->padding_bytes;

    // image bytes changes:    >>>> TODO: STRIDE SHOULD BE TAKEN INTO ACCOUNT... <<<<
    dc1394_get_color_coding_bit_size(out->color_coding, &bpp);
    out->image_bytes=(out->size[0]*out->size[1]*bpp)/8;

    // total is image_bytes + padding_bytes
    out->total_bytes = out->image_bytes + out->padding_bytes;

    // bytes-per-packet and packets_per_frame are internal data that can be kept as is.
    out->packet_size  = in->packet_size;
    out->packets_per_frame = in->packets_per_frame;

    // timestamp, frame_behind, id and camera are copied too:
    out->timestamp = in->timestamp;
    out->frames_behind = in->frames_behind;
    out->camera = in->camera;
    out->id = in->id;

    // verify memory allocation:
    if (out->total_bytes>out->allocated_image_bytes) {
        free(out->image);
        out->image=(uint8_t*)malloc(out->total_bytes*sizeof(uint8_t));
        if (out->image)
            out->allocated_image_bytes = out->total_bytes*sizeof(uint8_t);
        else
            out->allocated_image_bytes = 0;
    }

    // Copy padding bytes:
    if(out->image)
        memcpy(&(out->image[out->image_bytes]),&(in->image[in->image_bytes]),out->padding_bytes);

    out->little_endian=0; // not used before 1.32 is out.
    out->data_in_padding=0; // not used before 1.32 is out.

    if(out->image)
        return DC1394_SUCCESS;

    return DC1394_MEMORY_ALLOCATION_FAILURE;
}

dc1394error_t
dc1394_debayer_frames(dc1394video_frame_t *in, dc1394video_frame_t *out, dc1394bayer_method_t method)
{
    if ((method<DC1394_BAYER_METHOD_MIN)||(method>DC1394_BAYER_METHOD_MAX))
        return DC1394_INVALID_BAYER_METHOD;

    switch (in->color_coding) {
    case DC1394_COLOR_CODING_RAW8:
    case DC1394_COLOR_CODING_MONO8:

        if(DC1394_SUCCESS != Adapt_buffer_bayer(in,out,method))
            return DC1394_MEMORY_ALLOCATION_FAILURE;

        switch (method) {
        case DC1394_BAYER_METHOD_NEAREST:
            return dc1394_bayer_NearestNeighbor(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_SIMPLE:
            return dc1394_bayer_Simple(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_BILINEAR:
            return dc1394_bayer_Bilinear(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_HQLINEAR:
            return dc1394_bayer_HQLinear(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_DOWNSAMPLE:
            return dc1394_bayer_Downsample(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_EDGESENSE:
            return dc1394_bayer_EdgeSense(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_VNG:
            return dc1394_bayer_VNG(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        case DC1394_BAYER_METHOD_AHD:
            return dc1394_bayer_AHD(in->image, out->image, in->size[0], in->size[1], in->color_filter);
        }
        break;
    case DC1394_COLOR_CODING_MONO16:
    case DC1394_COLOR_CODING_RAW16:

        if(DC1394_SUCCESS != Adapt_buffer_bayer(in,out,method))
            return DC1394_MEMORY_ALLOCATION_FAILURE;

        switch (method) {
        case DC1394_BAYER_METHOD_NEAREST:
            return dc1394_bayer_NearestNeighbor_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_SIMPLE:
            return dc1394_bayer_Simple_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_BILINEAR:
            return dc1394_bayer_Bilinear_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_HQLINEAR:
            return dc1394_bayer_HQLinear_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_DOWNSAMPLE:
            return dc1394_bayer_Downsample_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_EDGESENSE:
            return dc1394_bayer_EdgeSense_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_VNG:
            return dc1394_bayer_VNG_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        case DC1394_BAYER_METHOD_AHD:
            return dc1394_bayer_AHD_uint16((uint16_t*)in->image, (uint16_t*)out->image, in->size[0], in->size[1], in->color_filter, in->data_depth);
        }
        break;
    default:
        return DC1394_FUNCTION_NOT_SUPPORTED;
    }

    return DC1394_SUCCESS;
}
#endif
