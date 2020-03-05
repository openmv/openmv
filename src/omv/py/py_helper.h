/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python helper functions.
 */
#ifndef __PY_HELPER_H__
#define __PY_HELPER_H__
#include "imlib.h"
#include "py_assert.h"
extern const mp_obj_fun_builtin_var_t py_func_unavailable_obj;
image_t *py_helper_arg_to_image_mutable(const mp_obj_t arg);
image_t *py_helper_arg_to_image_mutable_bayer(const mp_obj_t arg);
image_t *py_helper_arg_to_image_grayscale(const mp_obj_t arg);
image_t *py_helper_arg_to_image_color(const mp_obj_t arg);
image_t *py_helper_keyword_to_image_mutable(uint n_args, const mp_obj_t *args, uint arg_index,
                                            mp_map_t *kw_args, mp_obj_t kw, image_t *default_val);
image_t *py_helper_keyword_to_image_mutable_mask(uint n_args, const mp_obj_t *args, uint arg_index,
                                                 mp_map_t *kw_args);
void py_helper_keyword_rectangle(image_t *img, uint n_args, const mp_obj_t *args, uint arg_index,
                                 mp_map_t *kw_args, mp_obj_t kw, rectangle_t *r);
void py_helper_keyword_rectangle_roi(image_t *img, uint n_args, const mp_obj_t *args, uint arg_index,
                                     mp_map_t *kw_args, rectangle_t *r);
int py_helper_keyword_int(uint n_args, const mp_obj_t *args, uint arg_index,
                          mp_map_t *kw_args, mp_obj_t kw, int default_val);
float py_helper_keyword_float(uint n_args, const mp_obj_t *args, uint arg_index,
                              mp_map_t *kw_args, mp_obj_t kw, float default_val);
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
int py_helper_ksize_to_n(int ksize);
mp_obj_t py_helper_keyword_object(uint n_args, const mp_obj_t *args, uint arg_index, mp_map_t *kw_args, mp_obj_t kw);
#endif // __PY_HELPER__
