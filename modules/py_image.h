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
 * Image Python module.
 */
#ifndef __PY_IMAGE_H__
#define __PY_IMAGE_H__
#include "imlib.h"
extern const mp_obj_type_t py_image_type;
mp_obj_t py_image(int width, int height, pixformat_t pixfmt, uint32_t size, void *pixels);
mp_obj_t py_image_from_struct(image_t *img);
void *py_image_cobj(mp_obj_t img_obj);
// image.Font (py_image_font.c)
extern const mp_obj_type_t py_font_type;
// Call on every soft reset, before any text is drawn: the built-in font lives on the heap
// that the reset has just thrown away.
void py_font_init0(void);
const font_t *py_font_default(void);
// Raises ValueError with the message for a font error code; returns if err is FONT_OK.
void py_font_check(font_err_t err);
const font_t *py_font_from_obj(mp_obj_t obj);
// Checks the layout arguments text takes and fills in the layout. Raises ValueError.
void py_font_layout(font_layout_t *layout, mp_obj_t scale, mp_int_t align, mp_int_t wrap_width, mp_int_t stroke_width);
int py_image_descriptor_from_roi(image_t *img, const char *path, rectangle_t *roi);
#endif // __PY_IMAGE_H__
