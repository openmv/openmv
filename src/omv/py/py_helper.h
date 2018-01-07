/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MicroPython helper functions.
 *
 */
#ifndef __PY_HELPER_H__
#define __PY_HELPER_H__
#include <mp.h>
#include "imlib.h"
int py_helper_lookup_int(mp_map_t *kw_args, mp_obj_t kw, int default_val);
float py_helper_lookup_float(mp_map_t *kw_args, mp_obj_t kw, float default_val);
void py_helper_lookup_int_array(mp_map_t *kw_args, mp_obj_t kw, int *x, int size);
void py_helper_lookup_float_array(mp_map_t *kw_args, mp_obj_t kw, float *x, int size);
int py_helper_lookup_color(mp_map_t *kw_args, int default_color);
void py_helper_lookup_offset(mp_map_t *kw_args, image_t *img, point_t *p);
void py_helper_lookup_rectangle(mp_map_t *kw_args, image_t *img, rectangle_t *r);
void py_helper_lookup_rectangle_2(mp_map_t *kw_args, mp_obj_t kw, image_t *img, rectangle_t *r);
#endif // __PY_HELPER__
