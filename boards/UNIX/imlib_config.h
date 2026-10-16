/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2026 OpenMV, LLC.
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
 * Image library configuration for Unix port.
 */

#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__

// Enable all image processing features on Unix; the host has plenty of
// memory and performance is not critical. Tested via #ifdef everywhere
// in the codebase, so to disable a feature comment out the line.

#define IMLIB_ENABLE_LAB_LUT
#define IMLIB_ENABLE_BINARY_OPS
#define IMLIB_ENABLE_MATH_OPS
#define IMLIB_ENABLE_FLOOD_FILL
#define IMLIB_ENABLE_MEAN
#define IMLIB_ENABLE_MEDIAN
#define IMLIB_ENABLE_MODE
#define IMLIB_ENABLE_MIDPOINT
#define IMLIB_ENABLE_MORPH
#define IMLIB_ENABLE_GAUSSIAN
#define IMLIB_ENABLE_LAPLACIAN
#define IMLIB_ENABLE_BILATERAL
#define IMLIB_ENABLE_CARTOON
#define IMLIB_ENABLE_LINPOLAR
#define IMLIB_ENABLE_LOGPOLAR
#define IMLIB_ENABLE_LENS_CORR
#define IMLIB_ENABLE_ROTATION_CORR
#define IMLIB_ENABLE_BLOBS
#define IMLIB_ENABLE_FIND_LINES
#define IMLIB_ENABLE_FIND_LINE_SEGMENTS
#define IMLIB_ENABLE_FIND_CIRCLES
#define IMLIB_ENABLE_FIND_RECTS
#define IMLIB_ENABLE_QRCODES
#define IMLIB_ENABLE_APRILTAGS
#define IMLIB_ENABLE_APRILTAGS_TAG36H11
// Host has plenty of RAM; allow apriltag detection on full-res images
// (the test data set uses 640x426 frames which exceed the 64K-pixel
// embedded default).
#define IMLIB_ENABLE_HIGH_RES_APRILTAGS
#define IMLIB_ENABLE_DATAMATRICES
#define IMLIB_ENABLE_BARCODES
#define IMLIB_ENABLE_FIND_DISPLACEMENT
#define IMLIB_FIND_TEMPLATE
#define IMLIB_ENABLE_FEATURES
#define IMLIB_ENABLE_EYE
#define IMLIB_ENABLE_FIND_LBP
#define IMLIB_ENABLE_FIND_KEYPOINTS
#define IMLIB_ENABLE_DESCRIPTOR
#define IMLIB_ENABLE_SELECTIVE_SEARCH
#define IMLIB_ENABLE_HAAR
#define IMLIB_ENABLE_HOG
#define IMLIB_ENABLE_GET_REGRESSION
#define IMLIB_ENABLE_IMAGE_FILE_IO
#define IMLIB_ENABLE_IMAGE_IO
#define IMLIB_ENABLE_GIF
#define IMLIB_ENABLE_JPEG
#define IMLIB_ENABLE_PNG
#define IMLIB_ENABLE_PNG_ENCODER
#define IMLIB_ENABLE_PNG_DECODER

// AGAST corner detector for ORB keypoints (GPL).
// OMV_NO_GPL is intentionally not defined.
#define IMLIB_ENABLE_AGAST

#endif // __IMLIB_CONFIG_H__
