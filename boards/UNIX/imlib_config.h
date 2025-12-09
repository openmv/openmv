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
 * Image library configuration for Unix port.
 */

#ifndef __IMLIB_CONFIG_H__
#define __IMLIB_CONFIG_H__

// Enable all image processing features on Unix
// Performance is not as critical as on embedded systems

// Use pre-computed LAB lookup table (matches OPENMV4 hardware behavior)
// This ensures Unix port test results match flagship hardware expectations
#define IMLIB_ENABLE_LAB_LUT                   (1)

#define IMLIB_ENABLE_BINARY_OPS                (1)
#define IMLIB_ENABLE_MATH_OPS                  (1)
#define IMLIB_ENABLE_FLOOD_FILL                (1)
#define IMLIB_ENABLE_MEAN                      (1)
#define IMLIB_ENABLE_MEDIAN                    (1)
#define IMLIB_ENABLE_MODE                      (1)
#define IMLIB_ENABLE_MIDPOINT                  (1)
#define IMLIB_ENABLE_MORPH                     (1)
#define IMLIB_ENABLE_GAUSSIAN                  (1)
#define IMLIB_ENABLE_LAPLACIAN                 (1)
#define IMLIB_ENABLE_BILATERAL                 (1)
#define IMLIB_ENABLE_CARTOON                   (1)
#define IMLIB_ENABLE_LINPOLAR                  (1)
#define IMLIB_ENABLE_LOGPOLAR                  (1)
#define IMLIB_ENABLE_LENS_CORR                 (1)
#define IMLIB_ENABLE_ROTATION_CORR             (1)
#define IMLIB_ENABLE_BLOBS                     (1)
#define IMLIB_ENABLE_FIND_LINES                (1)
#define IMLIB_ENABLE_FIND_LINE_SEGMENTS        (1)
#define IMLIB_ENABLE_FIND_CIRCLES              (1)
#define IMLIB_ENABLE_FIND_RECTS                (1)
#define IMLIB_ENABLE_QRCODES                   (1)
#define IMLIB_ENABLE_APRILTAGS                 (1)
#define IMLIB_ENABLE_APRILTAGS_TAG36H11        (1)
#define IMLIB_ENABLE_DATAMATRICES              (1)
#define IMLIB_ENABLE_BARCODES                  (1)
#define IMLIB_ENABLE_FIND_DISPLACEMENT         (1)
#define IMLIB_FIND_TEMPLATE
#define IMLIB_ENABLE_FEATURES                  (1)
#define IMLIB_ENABLE_EYE                       (1)
#define IMLIB_ENABLE_FIND_LBP                  (1)
#define IMLIB_ENABLE_FIND_KEYPOINTS            (1)
#define IMLIB_ENABLE_DESCRIPTOR                (1)
#define IMLIB_ENABLE_SELECTIVE_SEARCH          (1)
#define IMLIB_ENABLE_HAAR                      (1)
#define IMLIB_ENABLE_HOG                       (1)
#define IMLIB_ENABLE_GET_REGRESSION            (1)
#define IMLIB_ENABLE_IMAGE_FILE_IO             (1)
#define IMLIB_ENABLE_IMAGE_IO                  (1)
#define IMLIB_ENABLE_GIF                       (1)
#define IMLIB_ENABLE_JPEG                      (1)
#define IMLIB_ENABLE_PNG                       (1)
#define IMLIB_ENABLE_PNG_ENCODER               (1)
#define IMLIB_ENABLE_PNG_DECODER               (1)

// Enable GPL code (AGAST corner detector for ORB keypoints)
// OMV_NO_GPL is intentionally NOT defined to allow GPL code
// #define OMV_NO_GPL                             (0)

// Enable AGAST corner detector for ORB keypoints (GPL code)
#define IMLIB_ENABLE_AGAST

#endif // __IMLIB_CONFIG_H__
