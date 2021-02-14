/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
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
                             unsigned int tensor_arena_size, // Size of the above scratch buffer.
                             unsigned int *input_height, // Height for the model.
                             unsigned int *input_width, // Width for the model.
                             unsigned int *input_channels, // Channels for the model (1 for grayscale8 and 3 for rgb888).
                             bool *signed_or_unsigned, // True if input is int8_t ([0:255]->[-128:127]), False if input is uint8_t ([0:255]->[0:255]).
                             bool *is_float); // Actual is float32 (not optimal - network should be fixed). Input should be ([0:255]->[0.0f:+1.0f]).

// Call this second to get the shape of the model output.
// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_get_output_data_hwc(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                              unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                              unsigned int tensor_arena_size, // Size of the above scratch buffer.
                              unsigned int *output_height, // Height for the model.
                              unsigned int *output_width, // Width for the model.
                              unsigned int *output_channels, // Channels for the model (1 for grayscale8 and 3 for rgb888).
                              bool *signed_or_unsigned, // True if output is int8_t ([-128:127]->[0:255]->[0.0f:1.0f]), False if output is uint8_t ([0:255]->[0:255]->[0.0f:1.0f]).
                              bool *is_float); // Actual is float32 (not optimal - network should be fixed). Output is [0.0f:+1.0f].

// Callback to populate the model input data byte array (laid out in [height][width][channel] order).
typedef void (*libtf_input_data_callback_t)(void *callback_data,
                                            void *model_input,
                                            const unsigned int input_height,
                                            const unsigned int input_width,
                                            const unsigned int input_channels,
                                            const bool signed_or_unsigned, // True if input is int8_t ([0:255]->[-128:127]), False if input is uint8_t ([0:255]->[0:255]).
                                            const bool is_float); // Actual is float32 (not optimal - network should be fixed). Input should be ([0:255]->[0.0f:+1.0f]).

// Callback to use the model output data byte array (laid out in [height][width][channel] order).
typedef void (*libtf_output_data_callback_t)(void *callback_data,
                                             void *model_output,
                                             const unsigned int output_height,
                                             const unsigned int output_width,
                                             const unsigned int output_channels,
                                             const bool signed_or_unsigned, // True if output is int8_t ([-128:127]->[0:255]->[0.0f:1.0f]), False if output is uint8_t ([0:255]->[0:255]->[0.0f:1.0f]).
                                             const bool is_float); // Actual is float32 (not optimal - network should be fixed). Output is [0.0f:+1.0f].

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_invoke(const unsigned char *model_data, // TensorFlow Lite binary model (8-bit quant).
                 unsigned char *tensor_arena, // As big as you can make it scratch buffer.
                 unsigned int tensor_arena_size, // Size of the above scratch buffer.
                 libtf_input_data_callback_t input_callback, // Callback to populate the model input data byte array.
                 void *input_callback_data, // User data structure passed to input callback.
                 libtf_output_data_callback_t output_callback, // Callback to use the model output data byte array.
                 void *output_callback_data); // User data structure passed to output callback.

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
int libtf_initialize_micro_features();

// Returns 0 on success and 1 on failure.
// Errors are printed to stdout.
// Converts audio sample data into a more compact form that's
// appropriate for feeding into a neural network.
int libtf_generate_micro_features(const int16_t* input, // Audio samples
                                  int input_size, // Audio samples size
                                  int output_size, // Slice size
                                  int8_t* output, // Slice data
                                  size_t* num_samples_read); // Number of samples used.

#ifdef __cplusplus
}
#endif

#endif // __LIBTF_H
