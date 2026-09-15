/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2026 OpenMV, LLC.
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
 * Debug shim for the WINC1500 host driver.
 */
#ifndef __WDRV_WINC_DEBUG_H__
#define __WDRV_WINC_DEBUG_H__
#include <stdio.h>

#define WDRV_DBG_NONE       (0)
#define WDRV_DBG_ERROR      (1)
#define WDRV_DBG_INFORM     (2)
#define WDRV_DBG_VERBOSE    (3)

// Bring the chip's bring-up chatter back with -DOMV_WINC_DEBUG_LEVEL=3, which is
// the only way to see where a firmware/driver mismatch actually fails.
#ifndef OMV_WINC_DEBUG_LEVEL
#define OMV_WINC_DEBUG_LEVEL    WDRV_DBG_NONE
#endif

#if (OMV_WINC_DEBUG_LEVEL >= WDRV_DBG_ERROR)
#define WDRV_DBG_ERROR_PRINT(...)       printf(__VA_ARGS__)
#else
#define WDRV_DBG_ERROR_PRINT(...)
#endif

#if (OMV_WINC_DEBUG_LEVEL >= WDRV_DBG_INFORM)
#define WDRV_DBG_INFORM_PRINT(...)      printf(__VA_ARGS__)
#else
#define WDRV_DBG_INFORM_PRINT(...)
#endif

#if (OMV_WINC_DEBUG_LEVEL >= WDRV_DBG_VERBOSE)
#define WDRV_DBG_VERBOSE_PRINT(...)     printf(__VA_ARGS__)
#else
#define WDRV_DBG_VERBOSE_PRINT(...)
#endif

#endif // __WDRV_WINC_DEBUG_H__
