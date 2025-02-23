/**
  ******************************************************************************
  * @file    layers_lite_graph.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform lite graph layers wrapper interface
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LAYERS_LITE_GRAPH_H
#define LAYERS_LITE_GRAPH_H

#include "core_common.h"

/*!
 * @defgroup layers_lite_graph Lite Graph Wrapper Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_lite_graph
 * @ingroup layers_lite_graph
 * @brief Generic Lite Graph Layer Wrapper
 *
 * The type of lite graph is handled by the specific forward lite graph function.
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_lite_graph_ {
  AI_NODE_COMMON_FIELDS_DECLARE
  ai_handle* activations_map;         /*!< array of pointers to shared activations memory pools */
  ai_handle* weights_map;             /*!< array of pointers to shared weights memory pools */
} ai_layer_lite_graph;


AI_API_DECLARE_END

#endif    /*LAYERS_LITE_GRAPH_H*/
