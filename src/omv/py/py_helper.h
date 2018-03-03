/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2018 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __PY_HELPER_H__
#define __PY_HELPER_H__
#include "py_assert.h"
#include "imlib.h"

image_t *py_helper_arg_to_image_mutable(const mp_obj_t arg);
image_t *py_helper_keyword_to_image_mutable(uint n_args, const mp_obj_t *args, uint arg_index,
                                            mp_map_t *kw_args, mp_obj_t kw, image_t *default_val);
image_t *py_helper_arg_to_image_grayscale(const mp_obj_t arg);
image_t *py_helper_arg_to_image_color(const mp_obj_t arg);
void py_helper_arg_to_thresholds(const mp_obj_t arg, list_t *thresholds);
void py_helper_keyword_thresholds(mp_map_t *kw_args, mp_obj_t kw, list_t *thresholds);
int py_helper_keyword_int(uint n_args, const mp_obj_t *args, uint arg_index,
                          mp_map_t *kw_args, mp_obj_t kw, int default_val);
float py_helper_keyword_float(uint n_args, const mp_obj_t *args, uint arg_index,
                              mp_map_t *kw_args, mp_obj_t kw, float default_val);
int py_helper_arg_to_ksize(const mp_obj_t arg);
int py_helper_ksize_to_n(int ksize);
uint py_helper_consume_array(uint n_args, const mp_obj_t *args, uint arg_index, size_t len, const mp_obj_t **items);
int py_helper_keyword_color(image_t *img, uint n_args, const mp_obj_t *args, uint arg_index,
                            mp_map_t *kw_args, int default_val);
int py_helper_lookup_int(mp_map_t *kw_args, mp_obj_t kw, int default_val);
float py_helper_lookup_float(mp_map_t *kw_args, mp_obj_t kw, float default_val);
void py_helper_lookup_int_array(mp_map_t *kw_args, mp_obj_t kw, int *x, int size);
void py_helper_lookup_float_array(mp_map_t *kw_args, mp_obj_t kw, float *x, int size);
int py_helper_lookup_color(mp_map_t *kw_args, int default_color);
void py_helper_lookup_offset(mp_map_t *kw_args, image_t *img, point_t *p);
void py_helper_lookup_rectangle(mp_map_t *kw_args, image_t *img, rectangle_t *r);
void py_helper_lookup_rectangle_2(mp_map_t *kw_args, mp_obj_t kw, image_t *img, rectangle_t *r);



#endif // __PY_HELPER__
