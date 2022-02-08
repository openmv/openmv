/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __LIBTF_H
#define __LIBTF_H

#define LIBTF_TENSOR_ARENA_ALIGNMENT 16

#ifdef __cplusplus
extern "C" {
#endif

typedef enum libtf_datatype {
    LIBTF_DATATYPE_UINT8,
    LIBTF_DATATYPE_INT8,
    LIBTF_DATATYPE_FLOAT
} libtf_datatype_t;

typedef struct libtf_parameters {
    size_t tensor_arena_size;
    size_t input_height, input_width, input_channels;
    libtf_datatype_t input_datatype;
    float input_scale;
    int input_zero_point;
    size_t output_height, output_width, output_channels;
    libtf_datatype_t output_datatype;
    float output_scale;
    int output_zero_point;
} libtf_parameters_t;

// Call this first to get the model parameters.
// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_get_parameters(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                         unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                         size_t tensor_arena_size, // Size of the above scratch buffer.
                         libtf_parameters_t *params); // Struct to hold model parameters.

// Callback to populate the model input data byte array (laid out in [height][width][channel] order).
typedef void (*libtf_input_data_callback_t)(void *callback_data,
                                            void *model_input,
                                            libtf_parameters_t *params);

// Callback to use the model output data byte array (laid out in [height][width][channel] order).
typedef void (*libtf_output_data_callback_t)(void *callback_data,
                                             void *model_output,
                                             libtf_parameters_t *params);

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_invoke(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                 unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                 libtf_parameters_t *params, // Struct with model parameters.
                 libtf_input_data_callback_t input_callback, // Callback to populate the model input data byte array.
                 void *input_callback_data, // User data structure passed to input callback.
                 libtf_output_data_callback_t output_callback, // Callback to use the model output data byte array.
                 void *output_callback_data); // User data structure passed to output callback.

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_initialize_micro_features();

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
// Converts audio sample data into a more compact form
// that's appropriate for feeding into a neural network.
int libtf_generate_micro_features(const int16_t *input, // Audio samples
                                  int input_size, // Audio sample size
                                  int output_size, // Slice data size
                                  int8_t *output, // Slice data
                                  size_t *num_samples_read); // Number of samples used

#ifdef __cplusplus
}
#endif

#endif // __LIBTF_H
