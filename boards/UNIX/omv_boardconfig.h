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
 * Unix board configuration.
 */

#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

// Unix port - software-only platform for development and testing
#define OMV_BOARD_NAME "Unix"
#define OMV_BOARD_ARCH "Unix x86_64"
#define OMV_BOARD_TYPE "Unix"

// Unix has no hardware UID, use dummy values
#define OMV_BOARD_UID_ADDR 0
#define OMV_BOARD_UID_SIZE 3
#define OMV_BOARD_UID_OFFSET 0

// umm_malloc block size for Unix (use reasonable default for desktop)
#define OMV_UMM_BLOCK_SIZE 128

// JPEG quality settings for framebuffer streaming
#define OMV_JPEG_QUALITY_LOW                  (50)
#define OMV_JPEG_QUALITY_HIGH                 (90)
#define OMV_JPEG_QUALITY_THRESHOLD            (320 * 240 * 2)

// No hardware-specific defines needed for Unix port
// All hardware features are disabled

#endif // __OMV_BOARDCONFIG_H__
