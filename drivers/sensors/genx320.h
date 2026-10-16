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
 * GENX320 driver.
 */
#ifndef __GENX320_H__
#define __GENX320_H__

typedef enum {
    OMV_CSI_GENX320_MODE_HISTO,
    OMV_CSI_GENX320_MODE_EVENT,
} genx320_mode_t;

typedef enum {
    OMV_CSI_GENX320_BIASES_DEFAULT,
    OMV_CSI_GENX320_BIASES_LOW_LIGHT,
    OMV_CSI_GENX320_BIASES_ACTIVE_MARKER,
    OMV_CSI_GENX320_BIASES_LOW_NOISE,
    OMV_CSI_GENX320_BIASES_HIGH_SPEED,
} genx320_biases_preset_t;

typedef enum {
    OMV_CSI_GENX320_BIAS_DIFF_OFF,
    OMV_CSI_GENX320_BIAS_DIFF_ON,
    OMV_CSI_GENX320_BIAS_FO,
    OMV_CSI_GENX320_BIAS_HPF,
    OMV_CSI_GENX320_BIAS_REFR,
} genx320_bias_t;

typedef enum {
    OMV_CSI_GENX320_STC_DISABLE,
    OMV_CSI_GENX320_STC_ONLY,
    OMV_CSI_GENX320_STC_TRAIL_ONLY,
    OMV_CSI_GENX320_STC_TRAIL,
} genx320_stc_modes_t;

// Argument for the OMV_CSI_IOCTL_GENX320_* requests.
typedef union {
    int32_t ivalue;                     // "i"
    struct {
        int32_t name;
        int32_t value;
    } bias;                             // "ii"
    struct {
        int32_t mode;
        int32_t freq_min;
        int32_t freq_max;
    } afk;                              // "iii"
    struct {
        int32_t mode;
        int32_t threshold1;             // STC_ONLY: STC, TRAIL_ONLY: trail, STC_TRAIL: STC
        int32_t threshold2;             // STC_TRAIL: trail
    } stc;                              // "iii"
    struct {
        int32_t mode;
        int32_t ndarray_size;           // -1 when not passed
    } mode;                             // "ii"
    struct {
        void *events;
        int32_t count;
    } events;                           // custom in, "_i" out
    struct {
        uint32_t event_count;
        float sigma;
        int32_t disabled;
    } calib;                            // "if" in, "__i" out
    image_t image;                      // custom out
} genx320_ioctl_arg_t;

#endif // __GENX320_H__
