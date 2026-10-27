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
 * VL53L9CX driver.
 */
#ifndef __VL53L9CX_H__
#define __VL53L9CX_H__
#include <stdint.h>

typedef enum {
    OMV_CSI_VL53L9CX_MODE_PRECISION,
    OMV_CSI_VL53L9CX_MODE_RANGE,
} vl53l9cx_mode_t;

// Planes written by OMV_CSI_IOCTL_VL53L9CX_READ_PLANES. The planes are read
// up to the last one enabled, so fewer planes allow higher frame rates.
typedef enum {
    OMV_CSI_VL53L9CX_PLANE_DEPTH     = (1 << 0),
    OMV_CSI_VL53L9CX_PLANE_AMPLITUDE = (1 << 1),
    OMV_CSI_VL53L9CX_PLANE_AMBIENT   = (1 << 2),
} vl53l9cx_plane_t;

// Number of planes of the READ_PLANES array.
#define OMV_CSI_VL53L9CX_PLANE_COUNT    (3)

// Argument for the OMV_CSI_IOCTL_VL53L9CX_* requests.
typedef union {
    int32_t ivalue;                     // "i"
    struct {
        int32_t min;
        int32_t max;
    } range;                            // "ii"
    uint16_t *planes;                   // custom in
} vl53l9cx_ioctl_arg_t;

#endif // __VL53L9CX_H__
