/**
  ******************************************************************************
  * @file    layers.h
  * @author  AST Embedded Analytics Research Platform
  * @date    01-May-2017
  * @brief   header file of AI platform layers datatypes
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef __LAYERS_H_
#define __LAYERS_H_
#pragma once

#include "layers_common.h"
#include "layers_conv2d.h"
#include "layers_generic.h"
#include "layers_nl.h"
#include "layers_norm.h"
#include "layers_pool.h"
#include "layers_rnn.h"
#include "layers_dense.h"
#include "layers_sm.h"

#ifdef USE_OPERATORS
  #include "layers_lambda.h"
#endif /* USE_OPERATORS */


AI_API_DECLARE_BEGIN

/*!
 * @defgroup layers Layers
 * @brief Definition of the forward functions for the layers and the general
 * ai_layer datastructure used to abstract specific layer implementation in the
 * generic forward function definition
 *
 * The forward function for a layer computes the layer activations given the
 * activations of the previous layer. They are added to the layer as function
 * pointer and called implicitly by the @ref ai_layers_forward_all function.
 * The input activations are read from layer &rarr; in and the computed
 * activations stored in layer &rarr; out. The layer type needs to be compatible
 * with the forward function, but layers with the same layout (e.g. `mp` and
 * `ap`) can share the same structure.
 */

/******************************************************************************/
/* Forward Functions Section                                                  */
/******************************************************************************/

/*!
 * @brief Executes a single layer in the network.
 * @ingroup layers
 * @param layer the layer to process
 * @return pointer to the next layer
 */
AI_INTERNAL_API
ai_layer* ai_layers_forward_layer(ai_layer* layer);


/*!
 * @brief Computes the ouptut of the network given the input.
 * @ingroup layers
 *
 * Given a network with the input pre-loaded in the net &rarr; in tensor,
 * computes the output by calling the forward functions of each layer and
 * selecting the next layer. When the layer has no successor or it's in a
 * loop-back configuration (layer &rarr; next is again layer), the function
 * stops. The result is stored in net &rarr; out.
 *
 * @param net the network to evaluate
 */
AI_INTERNAL_API
void ai_layers_forward_all(ai_network* net);

AI_API_DECLARE_END

#endif /* __LAYERS_H_ */
