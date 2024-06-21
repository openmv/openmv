/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python Tensorflow library wrapper.
 */
#ifndef __PY_TF_H__
#define __PY_TF_H__
#include "libtf.h"
#include "imlib_config.h"

// TF Model Object.
typedef struct py_tf_model_obj {
    mp_obj_base_t base;
    unsigned int size;
    unsigned char *data;
    bool fb_alloc;
    mp_obj_t input_shape;
    mp_obj_t output_shape;
    mp_obj_t output_list;
    libtf_parameters_t params;
} py_tf_model_obj_t;

extern char *py_tf_log_buffer;
void py_tf_alloc_log_buffer();

// Functionality select
#if IMLIB_ENABLE_TF == IMLIB_TF_FULLOPS
#define libtf_get_parameters libtf_get_parameters_fullops
#define libtf_invoke libtf_invoke_fullops
#elif IMLIB_ENABLE_TF == IMLIB_TF_DEFAULT
#define libtf_get_parameters libtf_get_parameters_default
#define libtf_invoke libtf_invoke_default
#endif

#endif // __PY_TF_H__
