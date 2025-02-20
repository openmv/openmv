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
 * MicroPython board config.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// // JPEG configuration.
#define OMV_JPEG_CODEC_ENABLE           (0)
#define OMV_JPEG_QUALITY_LOW            (50)
#define OMV_JPEG_QUALITY_HIGH           (90)
#define OMV_JPEG_QUALITY_THRESHOLD      (320 * 240 * 2)

// // UMM heap block size
#define OMV_UMM_BLOCK_SIZE              256

#define OMV_FB_MEMORY                   DRAM   // Framebuffer, fb_alloc
#define OMV_FB_SIZE                     (64 * 1024 * 1024)        // FB memory: header + VGA/GS image
#define OMV_FB_ALLOC_SIZE               (64 * 1024 * 1024)         // minimum fb alloc size
#define OMV_JPEG_SIZE                   (2 * 1024 * 1024)  // IDE JPEG buffer (header + data).

#endif //__OMV_BOARDCONFIG_H__
