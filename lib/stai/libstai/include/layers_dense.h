#ifndef LAYERS_DENSE_H
#define LAYERS_DENSE_H
/**
  ******************************************************************************
  * @file    layers_dense.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform dense layers datatypes
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

#include "layers_common.h"


/*!
 * @defgroup layers Normalization Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @brief Computes the activations of a fixed point dense (fully connected) layer.
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_fixed(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer.
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSSA per layer quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSSA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSSA per channel quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSSA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSUA per layer quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for SSUA per channel quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_SSUA_ch(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for UAUA per layer quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_UAUA(ai_layer *pLayer);

/*!
 * @brief Computes the activations of a integer dense (fully connected) layer
 *        for UAUA per channel quantized scheme
 * @ingroup layers_dense
 * @param layer the dense layer
 */
AI_INTERNAL_API
void forward_dense_integer_UAUA_ch(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*LAYERS_DENSE_H*/

