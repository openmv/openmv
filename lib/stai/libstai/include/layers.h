/**
  ******************************************************************************
  * @file    layers.h
  * @author  STMicroelectronics
  * @brief   header file of AI platform layers datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LAYERS_H
#define LAYERS_H

#include "layers_common.h"
#include "layers_conv2d.h"
#include "layers_custom.h"
#include "layers_dense.h"
#include "layers_formats_converters.h"
#include "layers_generic.h"
#include "layers_lite_graph.h"
#include "layers_nl.h"
#include "layers_norm.h"
#include "layers_pad_dqnn.h"
#include "layers_pad_generic.h"
#include "layers_pool.h"
#include "layers_rnn.h"
#include "layers_sm.h"
#include "layers_ml.h"
#include "layers_ml_iforest.h"
#include "layers_ml_svc.h"
#include "layers_ml.h"
#include "layers_ml_linearclassifier.h"
#include "layers_ml_treeensembleclassifier.h"
#include "layers_ml_treeensembleregressor.h"
#include "layers_ml_svmregressor.h"

#include "layers_conv2d_dqnn.h"
#include "layers_dense_dqnn.h"
#include "layers_pool_dqnn.h"
#include "layers_generic_dqnn.h"
#include "layers_upsample_generic.h"
#include "layers_upsample.h"
#include "layers_resize.h"
#include "layers_argminmax.h"
#include "layers_wrappers.h"
#include "ai_math_helpers.h"

#include "layers_loss_softmax_crossentropy.h"
#include "layers_loss_softmax_crossentropy_grad.h"
#include "layers_in_place_accumulator.h"
#include "layers_relu_grad.h"

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_any_layer_ptr
 * @ingroup layers
 * @brief Generic union for typed layers pointers
 */
typedef struct {
  ai_layer_type type;              /*!< layer type id (see @ref ai_layer_type) */
  union {
#define LAYER_ENTRY(type_, id_, struct_, forward_func_, init_func_, destroy_func_) \
   AI_CONCAT(ai_layer_, struct_)* struct_;
#include "layers_list.h"
  };
} ai_any_layer_ptr;


AI_API_DECLARE_END

#endif /*LAYERS_H*/
