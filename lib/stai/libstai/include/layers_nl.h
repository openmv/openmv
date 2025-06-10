/**
  ******************************************************************************
  * @file    layers_nl.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform nonlinearity layers datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_NL_H
#define LAYERS_NL_H

#include "layers_common.h"
#include "lite_internal_apis.h"

/*!
 * @defgroup layers_nl Normalization Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_nl
 * @ingroup layers_nl
 * @brief Generic Nonlinearity layer
 *
 * The type of nonlinearity is handled by the specific forward function.
 * It is a sequential layer. see @ref ai_layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_nl_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* nl_params;  /*!< associated parameters array */
} ai_layer_nl;

/*!
 * @struct ai_layer_sm
 * @ingroup layers_nl
 * @brief Softmax Nonlinearity layer
 *
 * It is a sequential layer. see @ref ai_layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_sm_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  AI_CONST ai_array* nl_params;  /*!< associated parameters array */
  ai_i16   axis;
} ai_layer_sm;

/*!
 * @typedef (*func_nl)
 * @ingroup layers_nl
 * @brief Fuction pointer for generic non linear transform
 * this function pointer abstracts a generic non linear layer.
 * see @ref nl_func_tanh_array_f32 and similar as examples.
 */
//typedef void (*func_nl)(ai_array *out, const ai_array *in,
//                        const ai_size size, const ai_handle params);
typedef void (*func_nl)(ai_tensor *out, const ai_tensor *in,
                        const ai_size size, const ai_handle params);

/*!
 * @brief Softmax pooling computed on a single float channel
 * @ingroup layers_nl
 * @param out opaque handler to float output channel
 * @param in  opaque handler to float input channel
 * @param channel_size number of elements of the input channel
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sm_channel_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size channel_size, const ai_handle params);

/*!
 * @brief Softmax normalization computed on an array of float channels
 * @ingroup layers_nl
 * @param out opaque handler to float output channel array
 * @param in  opaque handler to float input channel array
 * @param in_size  total size (number of elements) to process on the input
 * @param channel_size number of elements of the input channel
 * @param in_channel_step number of elements to move to next input element
 * @param out_channel_step number of elements to move to next output element
 */
AI_INTERNAL_API
void nl_func_sm_array_f32(ai_tensor *out, ai_tensor *in,
                          const ai_size in_size,
                          const ai_size channel_size,
                          const ai_size in_channel_step,
                          const ai_size out_channel_step);

/*!
 * @brief Softmax zero pooling computed on a single float channel
 * @ingroup layers_nl
 * @param out opaque handler to float output channel
 * @param in  opaque handler to float input channel
 * @param channel_size number of elements of the input channel
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sm_zero_channel_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size channel_size, const ai_handle params);

/*!
 * @brief Probit non linearity
 * @ingroup layers_nl
 * @param out opaque handler to float output channel
 * @param in  opaque handler to float input channel
 * @param channel_size number of elements of the input channel
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_probit_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size channel_size, const ai_handle params);


/*!
 * @brief Computes the tanh function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_tanh_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the tanh function on a fixed point data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_tanh_array_fixed(ai_tensor *out, const ai_tensor *in,
                              const ai_size size, const ai_handle params);


/*!
 * @brief Computes the sigmoid function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sigmoid_array_f32(ai_tensor *out, const ai_tensor *in,
                               const ai_size size, const ai_handle params);

/*!
 * @brief Computes the sigmoid function on a fixed point data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sigmoid_array_fixed(ai_tensor *out, const ai_tensor *in,
                                 const ai_size size, const ai_handle params);


/*!
 * @brief Computes the hard sigmoid function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_hard_sigmoid_array_f32(ai_tensor *out, const ai_tensor *in,
                                    const ai_size size, const ai_handle params);
/*!
 * @brief Computes the logistic function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_logistic_array_f32(ai_tensor *out, const ai_tensor *in,
                               const ai_size size, const ai_handle params);
/*!
 * @brief Computes the swish function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_swish_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the hard swish function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_hard_swish_array_f32(ai_tensor *out, const ai_tensor *in,
                                  const ai_size size, const ai_handle params);

/*!
 * @brief Computes the gelu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_gelu_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the absolute value function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_abs_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the cosine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_cos_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the inverse cosine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_acos_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the hyperbolic cosine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_cosh_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the inverse hyperbolic cosine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_acosh_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the sine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sin_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the inverse sine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_asin_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the hyperbolic sine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sinh_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the inverse hyperbolic sine function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_asinh_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the tangent function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_tan_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the inverse tangent function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_atan_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the inverse hyperbolic tangent function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_atanh_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the error function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_erf_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the natural logarithm function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_log_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the reciprocal square root function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_rsqrt_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the squarefunction on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_square_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the floor function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_floor_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the ceil function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_ceil_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the rounding function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_round_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the exponential function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_exp_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the sign negation function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_neg_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the sign negation function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_not_array_bool(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);


/*!
 * @brief Computes the reciprocal function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_reciprocal_array_f32(ai_tensor *out, const ai_tensor *in,
                                  const ai_size size, const ai_handle params);

/*!
 * @brief Computes the square root function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_sqrt_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the soft plus function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 */
AI_INTERNAL_API
void nl_func_soft_plus_array_f32(ai_tensor *out, const ai_tensor *in,
                                 const ai_size size, const ai_handle params);

/*!
 * @brief Computes the soft sign function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_soft_sign_array_f32(ai_tensor *out, const ai_tensor *in,
                                 const ai_size size, const ai_handle params);

/*!
 * @brief Computes the sign function on a single float element.
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 */
AI_INTERNAL_API
void nl_func_sign_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the clip function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_clip_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the hardmax function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param axis direction of the max index to be searched
 */
AI_INTERNAL_API
void nl_func_hardmax_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_shape *shape, const ai_handle params);

/*!
 * @brief Computes the generic relu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_relu_generic_array_f32(ai_tensor *out, const ai_tensor *in,
                                    const ai_size size, const ai_handle params);

/*!
 * @brief Computes the thresholded relu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_relu_thresholded_array_f32(ai_tensor *out, const ai_tensor *in,
                                        const ai_size size, const ai_handle params);

/*!
 * @brief Computes the relu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_relu_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the relu function on a fixed point data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_relu_array_fixed(ai_tensor *out, const ai_tensor *in,
                              const ai_size size, const ai_handle params);

/*!
 * @brief Computes the relu function on an integer-quantized data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
void nl_func_relu_array_integer(ai_tensor *out, const ai_tensor *in,
                                const ai_size size, const ai_handle params);

/*!
 * @brief Computes the clip function on an integer-quantized data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
void nl_func_clip_array_integer(ai_tensor *out, const ai_tensor *in,
                                const ai_size size, const ai_handle params);

/*!
 * @brief Computes the activation function on an integer-quantized data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to generated and used LUT
 */
void nl_func_array_integer(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the elu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_elu_array_f32(ai_tensor *out, const ai_tensor *in,
                           const ai_size size, const ai_handle params);

/*!
 * @brief Computes the max relu function on a fixed point data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_relu_max_array_fixed(ai_tensor *out, const ai_tensor *in,
                                  const ai_size size, const ai_handle params);

/*!
 * @brief Computes the selu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_selu_array_f32(ai_tensor *out, const ai_tensor *in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the prelu function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param slope opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size size of the input data in bytes
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_prelu_array_f32(ai_tensor *out, const ai_tensor *in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the prelu function on an integer-quantized data array
 * @ingroup layers_nl
 * @param in opaque handler to input elements to process
 * @param out opaque handler to output elements
 * @param size total size (number of elements) to process on the input
 * @param params opaque handler to optional nl parameters
 */
void nl_func_prelu_array_integer(ai_tensor *out, const ai_tensor *in,
                                 const ai_size size, const ai_handle params);

/******************************************************************************/
/** Forward Functions Section                                                **/
/******************************************************************************/

/*!
 * @brief Computes the activations of a ReLU nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_relu(ai_layer* layer);

/*!
 * @brief Computes the activations of a fixed point ReLU nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_relu_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer-quantized ReLU nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_relu_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a clip integer-quantized nonlinear layer.
 * @ingroup layers_nl
 * @param pLayer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_clip_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a ReLU6 nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_relu_thresholded(ai_layer* layer);

/*!
 * @brief Computes the activations of a fixed point max ReLU layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_relu_max_fixed(ai_layer *pLayer);


/*!
 * @brief Computes the activations of a ELU nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_elu(ai_layer* layer);

/*!
 * @brief Computes the activations of a SELU nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_selu(ai_layer* layer);

/*!
 * @brief Computes the activations of a PRELU nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_prelu(ai_layer* layer);

/*!
 * @brief Computes the activations of a binary tanh (sign) nonlinear layer.
 * @ingroup layers
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_sign(ai_layer* layer);

/*!
 * @brief Computes the activations of a clip nonlinear layer.
 * @ingroup layers
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_clip(ai_layer* layer);

/*!
 * @brief Computes the activations of a sigmoid nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_sigmoid(ai_layer* layer);

/*!
 * @brief Computes the activations of a fixed point sigmoid nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_sigmoid_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a hard sigmoid nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_hard_sigmoid(ai_layer* layer);

/*!
 * @brief Computes the activations of a swish nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_swish(ai_layer* layer);

/*!
 * @brief Computes the activations of a hard swish nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_hard_swish(ai_layer* layer);

/*!
 * @brief Computes the activations of a gelu nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_gelu(ai_layer* layer);

/*!
 * @brief Computes the activations of an exponential nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_exp(ai_layer* layer);

/*!
 * @brief Computes the activations of an square root nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_sqrt(ai_layer* layer);

/*!
 * @brief Computes the activations of a soft plus nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_soft_plus(ai_layer* layer);

/*!
 * @brief Computes the activations of a soft sign nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_soft_sign(ai_layer* layer);

/*!
 * @brief Computes the activations of a cosine (cos) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_cos(ai_layer* layer);

/*!
 * @brief Computes the activations of a inverse cosine (acos) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_acos(ai_layer* layer);

/*!
 * @brief Computes the activations of a hyperbolic cosine (cosh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_cosh(ai_layer* layer);

/*!
 * @brief Computes the activations of a inverse hyperbolic cosine (acosh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_acosh(ai_layer* layer);

/*!
 * @brief Computes the activations of a sine (sin) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_sin(ai_layer* layer);

/*!
 * @brief Computes the activations of a inverse sine (asin) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_asin(ai_layer* layer);

/*!
 * @brief Computes the activations of a hyperbolic sine (sinh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_sinh(ai_layer* layer);

/*!
 * @brief Computes the activations of a inverse hyperbolic sine (asinh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_asinh(ai_layer* layer);

/*!
 * @brief Computes the activations of a tangent (tan) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_tan(ai_layer* layer);

/*!
 * @brief Computes the activations of a inverse tangent (atan) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_atan(ai_layer* layer);

/*!
 * @brief Computes the activations of a hyperbolic tangent (tanh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_tanh(ai_layer* layer);

/*!
 * @brief Computes the activations of a inverse hyperbolic tangent (atanh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_atanh(ai_layer* layer);

/*!
 * @brief Computes the activations of a fixed point tanh nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_tanh_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a error function (erf) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_erf(ai_layer* layer);

/*!
 * @brief Computes the activations of a natural logarithm (log) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_log(ai_layer* layer);

/*!
 * @brief Computes the activations of a reciprocal square root (rsqrt) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_rsqrt(ai_layer* layer);

/*!
 * @brief Computes the activations of a square layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_square(ai_layer* layer);

/*!
 * @brief Computes the activations of an absolute value (abs) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_abs(ai_layer* layer);

/*!
 * @brief Computes the activations of a ceil layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_ceil(ai_layer* layer);

/*!
 * @brief Computes the activations of a floor layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_floor(ai_layer* layer);

/*!
 * @brief Computes the activations of a rounding layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_round(ai_layer* layer);

/*!
 * @brief Computes the activations of a sign negation (neg) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_neg(ai_layer* layer);
/*!
 * @brief Computes the activations of a sign negation (not) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_not(ai_layer* layer);


/*!
 * @brief Computes the activations of a reciprocal layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_reciprocal(ai_layer* layer);

/*!
 * @brief Hardmax on an input tensors
 * @ingroup layers_generic
 * @param layer the hardmax layer
 */
AI_INTERNAL_API
void forward_hardmax(ai_layer* layer);

/*!
 * @brief Computes the activations of a softmax nonlinear layer.
 * @ingroup layers_nl
 * @param layer the softmax (sm) layer
 */
AI_INTERNAL_API
void forward_sm(ai_layer* layer);

/*!
 * @brief Computes the activations of a softmax nonlinear layer (integer version).
 * @ingroup layers_nl
 * @param layer the softmax (sm) layer
 */
AI_INTERNAL_API
void forward_sm_integer(ai_layer* layer);

/*!
 * @brief Computes the activations of an integer quantized nonlinear layer.
 *        Non linear operation is function of used LUT defined through
 *        (pLayer->nl_params->data)
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_nl_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of an integer quantized PReLu.
 *        Slope params are located like weights, not params because they are
 *        quantized
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_prelu_integer(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*LAYERS_NL_H*/
