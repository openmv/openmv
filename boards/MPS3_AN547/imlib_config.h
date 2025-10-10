/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
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
 * Image library configuration.
 */
#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__

// Enable Image I/O
#define IMLIB_ENABLE_IMAGE_IO

// Enable Image File I/O
#define IMLIB_ENABLE_IMAGE_FILE_IO

// Enable LAB LUT
#define IMLIB_ENABLE_LAB_LUT

// Enable YUV LUT
//#define IMLIB_ENABLE_YUV_LUT

// Enable ISP ops
#define IMLIB_ENABLE_ISP_OPS

// Enable binary ops
#define IMLIB_ENABLE_BINARY_OPS

// Enable math ops
#define IMLIB_ENABLE_MATH_OPS

// Enable flood_fill()
#define IMLIB_ENABLE_FLOOD_FILL

// Enable mean()
#define IMLIB_ENABLE_MEAN

// Enable median()
#define IMLIB_ENABLE_MEDIAN

// Enable mode()
#define IMLIB_ENABLE_MODE

// Enable midpoint()
#define IMLIB_ENABLE_MIDPOINT

// Enable morph()
#define IMLIB_ENABLE_MORPH

// Enable Gaussian
#define IMLIB_ENABLE_GAUSSIAN

// Enable Laplacian
#define IMLIB_ENABLE_LAPLACIAN

// Enable bilateral()
#define IMLIB_ENABLE_BILATERAL

// Enable linpolar()
#define IMLIB_ENABLE_LINPOLAR

// Enable logpolar()
#define IMLIB_ENABLE_LOGPOLAR

// Enable lens_corr()
#define IMLIB_ENABLE_LENS_CORR

// Enable rotation_corr()
#define IMLIB_ENABLE_ROTATION_CORR

// Enable phasecorrelate()
#if defined(IMLIB_ENABLE_ROTATION_CORR)
#define IMLIB_ENABLE_FIND_DISPLACEMENT
#endif

// Enable get_similarity()
#define IMLIB_ENABLE_GET_SIMILARITY

// Enable find_lines()
#define IMLIB_ENABLE_FIND_LINES

// Enable find_line_segments()
#define IMLIB_ENABLE_FIND_LINE_SEGMENTS

// Enable find_circles()
#define IMLIB_ENABLE_FIND_CIRCLES

// Enable find_rects()
#define IMLIB_ENABLE_FIND_RECTS

// Enable find_qrcodes() (14 KB)
#define IMLIB_ENABLE_QRCODES

// Enable find_apriltags() (64 KB)
#define IMLIB_ENABLE_APRILTAGS
#define IMLIB_ENABLE_APRILTAGS_TAG36H11

// Enable fine find_apriltags() - (8-way connectivity versus 4-way connectivity)
// #define IMLIB_ENABLE_FINE_APRILTAGS

// Enable high res find_apriltags() - uses more RAM
// #define IMLIB_ENABLE_HIGH_RES_APRILTAGS

// Enable find_datamatrices() (26 KB)
#define IMLIB_ENABLE_DATAMATRICES

// Enable find_barcodes() (42 KB)
#define IMLIB_ENABLE_BARCODES

// Enable find_features() and Haar cascades.
#define IMLIB_ENABLE_FEATURES

// Enable Tensor Flow
#define IMLIB_ENABLE_STAI

// Enable AGAST.
#define IMLIB_ENABLE_AGAST

// Enable find_template()
#define IMLIB_FIND_TEMPLATE

// Enable find_lbp()
#define IMLIB_ENABLE_FIND_LBP

// Enable find_keypoints()
#if defined(IMLIB_ENABLE_FAST) || defined(IMLIB_ENABLE_AGAST)
#define IMLIB_ENABLE_FIND_KEYPOINTS
#endif

// Enable load, save and match descriptor
#define IMLIB_ENABLE_DESCRIPTOR

// Enable find_hog()
#define IMLIB_ENABLE_HOG

// Enable selective_search()
#define IMLIB_ENABLE_SELECTIVE_SEARCH

// Enable PNG encoder/decoder
#define IMLIB_ENABLE_PNG_ENCODER
#define IMLIB_ENABLE_PNG_DECODER

// Stereo Imaging
#define IMLIB_ENABLE_STEREO_DISPARITY

#endif //__IMLIB_CONFIG_H__
