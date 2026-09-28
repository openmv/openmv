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

#endif // __BOSON_H__
