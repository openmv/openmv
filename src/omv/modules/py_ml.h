/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Python Machine Learning Module.
 */
#ifndef __PY_ML_H__
#define __PY_ML_H__
// TF Model Object.
typedef struct py_ml_model_obj {
    mp_obj_base_t base;
    unsigned int size;
    unsigned char *data;
    size_t memory_size;
    bool fb_alloc;
    size_t inputs_size;
    mp_obj_tuple_t *input_shape;
    float input_scale;
    int input_zero_point;
    char input_dtype;
    size_t outputs_size;
    mp_obj_tuple_t *output_shape;
    float output_scale;
    int output_zero_point;
    char output_dtype;
    void *state; // Private context for the backend.
} py_ml_model_obj_t;

// Initialize a model.
int ml_backend_init_model(py_ml_model_obj_t *model);

// Run inference.
int ml_backend_run_inference(py_ml_model_obj_t *model);

// Return an input tensor by index.
void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index);

// Return an output tensor by index.
void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index);
#endif // __PY_ML_H__
