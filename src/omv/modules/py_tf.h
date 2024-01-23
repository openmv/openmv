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

typedef struct py_tf_model_obj {
    mp_obj_base_t base;
    unsigned char *model_data;
    unsigned int model_data_len;
    libtf_parameters_t params;
} py_tf_model_obj_t;

// Log buffer
#define PY_TF_PUTCHAR_BUFFER_LEN    1023
extern char *py_tf_putchar_buffer;
extern size_t py_tf_putchar_buffer_index;
extern size_t py_tf_putchar_buffer_len;
void py_tf_alloc_putchar_buffer();

// Functionality select
#if defined(IMLIB_ENABLE_TF_ALL_OPS)
#define libtf_get_parameters libtf_all_ops_get_parameters
#define libtf_invoke libtf_all_ops_invoke
#elif defined(IMLIB_ENABLE_TF_MINIMUM_OPS)
#define libtf_get_parameters libtf_minimum_ops_get_parameters
#define libtf_invoke libtf_minimum_ops_invoke
#else
#define libtf_get_parameters libtf_reduced_ops_get_parameters
#define libtf_invoke libtf_reduced_ops_invoke
#endif
#endif // __PY_TF_H__
