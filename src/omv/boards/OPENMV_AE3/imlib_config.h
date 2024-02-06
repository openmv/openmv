/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Image library configuration.
 */
#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__
#if CORE_M55_HE
// Enable ISP ops
#define IMLIB_ENABLE_ISP_OPS

// Enable binary ops
#define IMLIB_ENABLE_BINARY_OPS

// Enable math ops
#define IMLIB_ENABLE_MATH_OPS

#else
// Enable Image I/O
#define IMLIB_ENABLE_IMAGE_IO

// Enable Image File I/O
#define IMLIB_ENABLE_IMAGE_FILE_IO

// Enable LAB LUT
#define IMLIB_ENABLE_LAB_LUT

// Enable YUV LUT
//#define IMLIB_ENABLE_YUV_LUT

// Enable mean pooling
#define IMLIB_ENABLE_MEAN_POOLING

// Enable midpoint pooling
#define IMLIB_ENABLE_MIDPOINT_POOLING

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

// Enable cartoon()
// #define IMLIB_ENABLE_CARTOON

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
#define IMLIB_ENABLE_HIGH_RES_APRILTAGS

// Enable find_datamatrices() (26 KB)
#define IMLIB_ENABLE_DATAMATRICES

// Enable find_barcodes() (42 KB)
#define IMLIB_ENABLE_BARCODES

// Enable find_features()
#define IMLIB_ENABLE_FEATURES

// Enable FAST (20+ KBs).
// #define IMLIB_ENABLE_FAST

// Enable find_template()
#define IMLIB_FIND_TEMPLATE

// Enable find_lbp()
#define IMLIB_ENABLE_FIND_LBP

// Enable find_keypoints()
#define IMLIB_ENABLE_FIND_KEYPOINTS

// Enable load, save and match descriptor
#define IMLIB_ENABLE_DESCRIPTOR

// Enable find_hog()
// #define IMLIB_ENABLE_HOG

// Enable selective_search()
// #define IMLIB_ENABLE_SELECTIVE_SEARCH

// Enable PNG encoder/decoder
#define IMLIB_ENABLE_PNG_ENCODER
#define IMLIB_ENABLE_PNG_DECODER

// Stereo Imaging
// #define IMLIB_ENABLE_STEREO_DISPARITY

// Fast debayer
#define IMLIB_ENABLE_DEBAYER_OPTIMIZATION
#endif
#endif //__IMLIB_CONFIG_H__
