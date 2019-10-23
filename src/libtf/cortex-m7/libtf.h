/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

#ifndef __LIBTF_H
#define __LIBTF_H

#ifdef __cplusplus
extern "C" {
#endif

// Call this first to get the shape of the model input.
// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_get_input_data_hwc(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                             unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                             const unsigned int tensor_arena_size, // Size of the above scratch buffer.
                             unsigned int *input_height, // Height for the model.
                             unsigned int *input_width, // Width for the model.
                             unsigned int *input_channels); // Channels for the model (1 for grayscale8 and 3 for rgb888).

// Call this second to get the shape of the model output.
// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_get_output_data_hwc(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                              unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                              const unsigned int tensor_arena_size, // Size of the above scratch buffer.
                              unsigned int *output_height, // Height for the model.
                              unsigned int *output_width, // Width for the model.
                              unsigned int *output_channels); // Channels for the model (1 for grayscale8 and 3 for rgb888).

// Callback to populate the model input data byte array (laid out in [height][width][channel] order).
typedef void (*libtf_input_data_callback_t)(void *callback_data,
                                            unsigned char *model_input,
                                            const unsigned int input_height,
                                            const unsigned int input_width,
                                            const unsigned int input_channels);

// Callback to use the model output data byte array (laid out in [height][width][channel] order).
typedef void (*libtf_output_data_callback_t)(void *callback_data,
                                             unsigned char *model_output,
                                             const unsigned int output_height,
                                             const unsigned int output_width,
                                             const unsigned int output_channels);

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_invoke(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                 unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                 const unsigned int tensor_arena_size, // Size of the above scratch buffer.
                 libtf_input_data_callback_t input_callback, // Callback to populate the model input data byte array.
                 void *input_callback_data, // User data structure passed to input callback.
                 libtf_output_data_callback_t output_callback, // Callback to use the model output data byte array.
                 void *output_callback_data); // User data structure passed to output callback.

#ifdef __cplusplus
}
#endif

#endif // __LIBTF_H
