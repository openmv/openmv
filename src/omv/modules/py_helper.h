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
 * Python helper functions.
 */
#ifndef __PY_HELPER_H__
#define __PY_HELPER_H__
#include "imlib.h"

typedef enum py_helper_arg_image_flags {
    ARG_IMAGE_ANY          = (0 << 0),
    ARG_IMAGE_MUTABLE      = (1 << 0),
    ARG_IMAGE_UNCOMPRESSED = (1 << 1),
    ARG_IMAGE_GRAYSCALE    = (1 << 2),
    ARG_IMAGE_ALLOC        = (1 << 3)
} py_helper_arg_image_flags_t;

extern const mp_obj_fun_builtin_var_t py_func_unavailable_obj;
image_t *py_helper_arg_to_image(const mp_obj_t arg, uint32_t flags);
const void *py_helper_arg_to_palette(const mp_obj_t arg, uint32_t pixfmt);
rectangle_t py_helper_arg_to_roi(const mp_obj_t arg, const image_t *img);
void py_helper_arg_to_scale(const mp_obj_t arg_x_scale, const mp_obj_t arg_y_scale,
                            float *x_scale, float *y_scale);
void py_helper_arg_to_minmax(const mp_obj_t minmax, float *min, float *max,
                             const mp_obj_t *array, size_t array_size);
float py_helper_arg_to_float(const mp_obj_t arg, float default_value);
void py_helper_arg_to_float_array(const mp_obj_t arg, float *array, size_t size);

image_t *py_helper_keyword_to_image(uint n_args, const mp_obj_t *args, uint arg_index,
                                    mp_map_t *kw_args, mp_obj_t kw, image_t *default_val);
void py_helper_keyword_rectangle(image_t *img, uint n_args, const mp_obj_t *args, uint arg_index,
                                 mp_map_t *kw_args, mp_obj_t kw, rectangle_t *r);
void py_helper_keyword_rectangle_roi(image_t *img, uint n_args, const mp_obj_t *args, uint arg_index,
                                     mp_map_t *kw_args, rectangle_t *r);
int py_helper_keyword_int(uint n_args, const mp_obj_t *args, uint arg_index,
                          mp_map_t *kw_args, mp_obj_t kw, int default_val);
bool py_helper_keyword_int_maybe(uint n_args, const mp_obj_t *args, uint arg_index,
                                 mp_map_t *kw_args, mp_obj_t kw, int *value);
float py_helper_keyword_float(uint n_args, const mp_obj_t *args, uint arg_index,
                              mp_map_t *kw_args, mp_obj_t kw, float default_val);
bool py_helper_keyword_float_maybe(uint n_args, const mp_obj_t *args, uint arg_index,
                                   mp_map_t *kw_args, mp_obj_t kw, float *value);
void py_helper_keyword_int_array(uint n_args, const mp_obj_t *args, uint arg_index,
                                 mp_map_t *kw_args, mp_obj_t kw, int *x, int size);
void py_helper_keyword_float_array(uint n_args, const mp_obj_t *args, uint arg_index,
                                   mp_map_t *kw_args, mp_obj_t kw, float *x, int size);
float *py_helper_keyword_corner_array(uint n_args, const mp_obj_t *args, uint arg_index,
                                      mp_map_t *kw_args, mp_obj_t kw);
uint py_helper_consume_array(uint n_args, const mp_obj_t *args, uint arg_index, size_t len, const mp_obj_t **items);
int py_helper_keyword_color(image_t *img, uint n_args, const mp_obj_t *args, uint arg_index,
                            mp_map_t *kw_args, int default_val);
void py_helper_arg_to_thresholds(const mp_obj_t arg, list_t *thresholds);
void py_helper_keyword_thresholds(uint n_args, const mp_obj_t *args, uint arg_index,
                                  mp_map_t *kw_args, list_t *thresholds);
int py_helper_arg_to_ksize(const mp_obj_t arg);
mp_obj_t py_helper_keyword_object(uint n_args, const mp_obj_t *args,
                                  uint arg_index, mp_map_t *kw_args, mp_obj_t kw, mp_obj_t default_val);
const uint16_t *py_helper_keyword_color_palette(uint n_args, const mp_obj_t *args,
                                                uint arg_index, mp_map_t *kw_args, const uint16_t *default_color_palette);
const uint8_t *py_helper_keyword_alpha_palette(uint n_args, const mp_obj_t *args,
                                               uint arg_index, mp_map_t *kw_args, const uint8_t *default_alpha_palette);
bool py_helper_is_equal_to_framebuffer(image_t *img);
void py_helper_update_framebuffer(image_t *img);
void py_helper_set_to_framebuffer(image_t *img);
#endif // __PY_HELPER__
