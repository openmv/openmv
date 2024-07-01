/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python Machine Learning Module.
 */
#ifndef __PY_ML_H__
#define __PY_ML_H__
#include "imlib.h"

typedef enum {
    PY_ML_SCALE_NONE,
    PY_ML_SCALE_0_1,
    PY_ML_SCALE_S1_1,
    PY_ML_SCALE_S128_127
} py_ml_scale_t;

typedef enum py_ml_dtype {
    PY_ML_DTYPE_INT8,
    PY_ML_DTYPE_UINT8,
    PY_ML_DTYPE_INT16,
    PY_ML_DTYPE_FLOAT
} py_ml_dtype_t;

// TF Model Object.
typedef struct py_ml_model_obj {
    mp_obj_base_t base;
    unsigned int size;
    unsigned char *data;
    size_t memory_size;
    bool fb_alloc;
    mp_obj_tuple_t *input_shape;
    mp_obj_tuple_t *input_scale;
    mp_obj_tuple_t *input_zero_point;
    mp_obj_tuple_t *input_dtype;
    mp_obj_tuple_t *output_shape;
    mp_obj_tuple_t *output_scale;
    mp_obj_tuple_t *output_zero_point;
    mp_obj_tuple_t *output_dtype;
    void *state; // Private context for the backend.
} py_ml_model_obj_t;

typedef void (py_ml_input_callback_t) (void *self, py_ml_model_obj_t *model, size_t index);

// ML Image Arg Object.
typedef struct py_ml_image_arg_obj {
    mp_obj_base_t base;
    py_ml_input_callback_t *input_callback;
    image_t *image;
    rectangle_t roi;
    py_ml_scale_t scale;
    float mean[3];
    float stdev[3];
} py_image_arg_obj_t;

// Initialize a model.
int ml_backend_init_model(py_ml_model_obj_t *model);

// Return an input tensor by index.
void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index);

// Return an output tensor by index.
void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index);

// Run inference.
int ml_backend_run_inference(py_ml_model_obj_t *model);
#endif // __PY_ML_H__
