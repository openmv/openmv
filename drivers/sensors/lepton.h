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
 * Lepton driver.
 */
#ifndef __LEPTON_H__
#define __LEPTON_H__

// Argument for the OMV_CSI_IOCTL_LEPTON_* requests.
typedef union {
    int32_t ivalue;                     // "i"
    float fvalue;                       // "f"
    struct {
        int32_t measurement;
        int32_t high_temp;
    } mode;                             // "ii" in, "bb" out
    struct {
        float min;
        float max;
    } range;                            // "ff"
    struct {
        int32_t command;
        uint16_t *data;
        uint32_t len;
    } attr;                             // custom
} lepton_ioctl_arg_t;

#endif // __LEPTON_H__
