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
 * FIR Python module.
 */
#ifndef __PY_FIR_LEPTON_H__
#define __PY_FIR_LEPTON_H__
void fir_lepton_deinit();
int fir_lepton_init(omv_i2c_t *bus, int *w, int *h, int *refresh, int *resolution);
void fir_lepton_register_vsync_cb(mp_obj_t cb);
mp_obj_t fir_lepton_get_radiometry();
void fir_lepton_register_frame_cb(mp_obj_t cb);
mp_obj_t fir_lepton_get_frame_available();
mp_obj_t fir_lepton_read_ta();
mp_obj_t fir_lepton_read_ir(int w, int h, bool mirror, bool flip, bool transpose, int timeout);
void fir_lepton_fill_image(image_t *img, int w, int h, bool auto_range, float min, float max,
                           bool mirror, bool flip, bool transpose, int timeout);
void fir_lepton_trigger_ffc(int timeout);
#endif // __PY_FIR_LEPTON_H__
