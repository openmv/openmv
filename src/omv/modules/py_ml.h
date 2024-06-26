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
    size_t inputs_size;
    mp_obj_tuple_t *input_shape;
    float input_scale;
    int input_zero_point;
    py_ml_dtype_t input_dtype;
    size_t outputs_size;
    mp_obj_tuple_t *output_shape;
    float output_scale;
    int output_zero_point;
    py_ml_dtype_t output_dtype;
    void *state; // Private context for the backend.
} py_ml_model_obj_t;

// Initialize a model.
int ml_backend_init_model(py_ml_model_obj_t *model);

// Callback to populate the model input data.
typedef void (*ml_backend_input_callback_t) (py_ml_model_obj_t *model, void *arg);

// Callback to get the model output data.
typedef void (*ml_backend_output_callback_t) (py_ml_model_obj_t *model, void *arg);

// Return an input tensor by index.
void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index);

// Return an output tensor by index.
void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index);

// Run inference.
int ml_backend_run_inference(py_ml_model_obj_t *model,
                             ml_backend_input_callback_t input_callback, // Callback to populate the model input data.
                             void *input_data, // User data structure passed to input callback.
                             ml_backend_output_callback_t output_callback, // Callback to use the model output data.
                             void *output_data); // User data structure passed to output callback.
#endif // __PY_ML_H__
