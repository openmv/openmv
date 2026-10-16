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
 * Boson driver.
 */
#ifndef __BOSON_H__
#define __BOSON_H__

// Values match FLR_BOSON_GAINMODE_E.
typedef enum {
    OMV_CSI_BOSON_GAIN_HIGH,
    OMV_CSI_BOSON_GAIN_LOW,
    OMV_CSI_BOSON_GAIN_AUTO,
    OMV_CSI_BOSON_GAIN_DUAL,
    OMV_CSI_BOSON_GAIN_MANUAL,
} boson_gain_mode_t;

// Values match FLR_BOSON_FFCMODE_E.
typedef enum {
    OMV_CSI_BOSON_FFC_MANUAL,
    OMV_CSI_BOSON_FFC_AUTO,
    OMV_CSI_BOSON_FFC_EXTERNAL,
} boson_ffc_mode_t;

// Values match FLR_BOSON_FFCSTATUS_E.
typedef enum {
    OMV_CSI_BOSON_FFC_STATUS_NONE,
    OMV_CSI_BOSON_FFC_STATUS_IMMINENT,
    OMV_CSI_BOSON_FFC_STATUS_RUNNING,
    OMV_CSI_BOSON_FFC_STATUS_COMPLETE,
} boson_ffc_status_t;

// Values match FLR_RADIOMETRY_RBFO_TYPE_E.
typedef enum {
    OMV_CSI_BOSON_RBFO_DEFAULT,
    OMV_CSI_BOSON_RBFO_FACTORY,
} boson_rbfo_type_t;

// Values match FLR_SPOTMETER_STATS_TEMP_MODE_E.
typedef enum {
    OMV_CSI_BOSON_SPOT_METER_CELSIUS,
    OMV_CSI_BOSON_SPOT_METER_FAHRENHEIT,
    OMV_CSI_BOSON_SPOT_METER_KELVIN,
} boson_spot_meter_mode_t;

// Argument for the OMV_CSI_IOCTL_BOSON_* requests.
typedef union {
    int32_t ivalue;                     // "i"
    float fvalue;                       // "f"
    omv_csi_window_t roi;               // unpack_window in, "iiii" out
    struct {
        uint32_t major;
        uint32_t minor;
        uint32_t patch;
    } rev;                              // "iii"
    struct {
        int32_t w;
        int32_t h;
    } roi_max;                          // "ii"
    struct {
        int32_t mean;
        int32_t deviation;
        int32_t min_value;
        int32_t min_x;
        int32_t min_y;
        int32_t max_value;
        int32_t max_x;
        int32_t max_y;
    } stats;                            // "iiiiiiii"
    struct {
        float mean;
        float deviation;
        float min_value;
        int32_t min_x;
        int32_t min_y;
        float max_value;
        int32_t max_x;
        int32_t max_y;
    } temp_stats;                       // "fffiifii"
    struct {
        int32_t rbfo_type;
        int32_t counts;
        float temp;
    } temp_from_counts;                 // "ii" in, "__f" out
    struct {
        int32_t rbfo_type;
        int32_t low_gain;
        float r;
        float b;
        float f;
        float o;
    } rbfo;                             // "ii" in, "__ffff" out
} boson_ioctl_arg_t;

#endif // __BOSON_H__
