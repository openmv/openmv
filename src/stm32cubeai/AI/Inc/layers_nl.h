/**
  ******************************************************************************
  * @file    layers_nl.h
  * @author  AST Embedded Analytics Research Platform
  * @date    18-Apr-2018
  * @brief   header file of AI platform nonlinearity layers datatypes
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __LAYERS_NL_H_
#define __LAYERS_NL_H_
#pragma once

#include "layers_common.h"

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
 * @typedef (*func_nl)
 * @ingroup layers_nl
 * @brief Fuction pointer for generic non linear transform
 * this function pointer abstracts a generic non linear layer.
 * see @ref nl_func_tanh_array_f32 and similar as examples.
 */
typedef void (*func_nl)(ai_handle out, const ai_handle in,
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
void nl_func_sm_channel_f32(ai_handle out, const ai_handle in,
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
void nl_func_sm_array_f32(ai_handle out, const ai_handle in,
                          const ai_size in_size,
                          const ai_size channel_size,
                          const ai_size in_channel_step,
                          const ai_size out_channel_step);

/*!
 * @brief Computes the tanh function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 * @param params opaque handler to optional nl parameters
 */
AI_INTERNAL_API
void nl_func_tanh_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_tanh_array_fixed(ai_handle out, const ai_handle in,
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
void nl_func_sigmoid_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_sigmoid_array_fixed(ai_handle out, const ai_handle in,
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
void nl_func_hard_sigmoid_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_exp_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_sqrt_array_f32(ai_handle out, const ai_handle in,
                            const ai_size size, const ai_handle params);

/*!
 * @brief Computes the soft plus function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 */
AI_INTERNAL_API
void nl_func_soft_plus_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_soft_sign_array_f32(ai_handle out, const ai_handle in,
                                 const ai_size size, const ai_handle params);

/*!
 * @brief Computes the sign function on a single float element.
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param size number of elements in the input buffer
 */
AI_INTERNAL_API
void nl_func_sign_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_clip_array_f32(ai_handle out, const ai_handle in,
                             const ai_size size, const ai_handle params);

/*!
 * @brief Computes the hardmax function on a float data array
 * @ingroup layers_nl
 * @param in opaque handler to float, size should be 1
 * @param out opaque handler to float output elem
 * @param axis direction of the max index to be searched
 */
AI_INTERNAL_API
void nl_func_hardmax_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_relu_generic_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_relu_thresholded_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_relu_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_relu_array_fixed(ai_handle out, const ai_handle in,
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
void nl_func_elu_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_relu_max_array_fixed(ai_handle out, const ai_handle in,
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
void nl_func_selu_array_f32(ai_handle out, const ai_handle in,
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
void nl_func_prelu_array_f32(ai_handle out, const ai_handle in,
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
 * @brief Computes the activations of a hyperbolic tangent (tanh) layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_tanh(ai_layer* layer);

/*!
 * @brief Computes the activations of a fixed point tanh nonlinear layer.
 * @ingroup layers_nl
 * @param layer the nonlinear (nl) layer
 */
AI_INTERNAL_API
void forward_tanh_fixed(ai_layer *pLayer);


/*!
 * @brief Computes the activations of a softmax nonlinear layer.
 * @ingroup layers_nl
 * @param layer the softmax (sm) layer
 */
AI_INTERNAL_API
void forward_sm(ai_layer* layer);


AI_API_DECLARE_END

#endif    /*__LAYERS_NL_H_*/
