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
 * Board configuration.
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

// Architecture info
#define OMV_BOARD_ARCH                        "Unix host"
#define OMV_BOARD_TYPE                        "Unix"

// JPEG configuration.
#define OMV_JPEG_QUALITY_LOW                  (50)
#define OMV_JPEG_QUALITY_HIGH                 (90)
#define OMV_JPEG_QUALITY_THRESHOLD            (320 * 240 * 2)

// Host-side UMA pool size. Override at compile time for memory-hungry
// workloads: `make TARGET=UNIX CFLAGS_USERMOD=-DOMV_UNIX_UMA_POOL_SIZE=...`.
#ifndef OMV_UNIX_UMA_POOL_SIZE
#define OMV_UNIX_UMA_POOL_SIZE                (64 * 1024 * 1024)
#endif

#endif // __BOARD_CONFIG_H__
