// SPDX-License-Identifier: MIT
//
// Copyright (C) 2026 OpenMV, LLC.
//
// Default image library configuration.
//
// A board's imlib_config.h includes this file and then turns features off with
// #undef IMLIB_ENABLE_X, or turns optional ones on with #define IMLIB_ENABLE_X.
// The result is checked by imlib_config_check.h, which imlib.h includes.
#ifndef __IMLIB_CONFIG_DEFAULT_H__
#define __IMLIB_CONFIG_DEFAULT_H__

// Enable Image I/O
#define IMLIB_ENABLE_IMAGE_IO

// Enable Image File I/O
#define IMLIB_ENABLE_IMAGE_FILE_IO

// Enable LAB LUT
#define IMLIB_ENABLE_LAB_LUT

// Enable the gamma LUT (software ISP)
// #define IMLIB_ENABLE_GAMMA_LUT

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

// Enable find_displacement(), requires IMLIB_ENABLE_ROTATION_CORR
#define IMLIB_ENABLE_FIND_DISPLACEMENT

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

// AprilTag families, find_apriltags() needs at least one
// #define IMLIB_ENABLE_APRILTAGS_TAG16H5
// #define IMLIB_ENABLE_APRILTAGS_TAG25H9
// #define IMLIB_ENABLE_APRILTAGS_TAG36H10
#define IMLIB_ENABLE_APRILTAGS_TAG36H11
// #define IMLIB_ENABLE_APRILTAGS_TAGCIRCLE21H7
// #define IMLIB_ENABLE_APRILTAGS_TAGCIRCLE49H12
// #define IMLIB_ENABLE_APRILTAGS_TAGCUSTOM48H12
// #define IMLIB_ENABLE_APRILTAGS_TAGSTANDARD41H12
// #define IMLIB_ENABLE_APRILTAGS_TAGSTANDARD52H13

// Enable fine find_apriltags() - (8-way connectivity versus 4-way connectivity)
// #define IMLIB_ENABLE_FINE_APRILTAGS

// Enable high res find_apriltags() - uses more RAM
#define IMLIB_ENABLE_HIGH_RES_APRILTAGS

// Enable find_datamatrices() (26 KB)
#define IMLIB_ENABLE_DATAMATRICES

// Enable find_barcodes() (42 KB)
#define IMLIB_ENABLE_BARCODES

// Enable find_features() and Haar cascades.
#define IMLIB_ENABLE_FEATURES

// Enable FAST.
// #define IMLIB_ENABLE_FAST

// Enable AGAST.
#define IMLIB_ENABLE_AGAST

// Enable find_template()
#define IMLIB_FIND_TEMPLATE

// Enable find_lbp()
#define IMLIB_ENABLE_FIND_LBP

// Enable find_keypoints(), uses IMLIB_ENABLE_FAST or IMLIB_ENABLE_AGAST
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

// Enable the fast debayer
// #define IMLIB_ENABLE_DEBAYER_OPTIMIZATION

#endif // __IMLIB_CONFIG_DEFAULT_H__
