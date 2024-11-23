/**
  ******************************************************************************
  * @file    layers_conv2d_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform DQNN pool datatypes
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

#ifndef LAYERS_POOL_DQNN_H
#define LAYERS_POOL_DQNN_H

#include "layers_common.h"
#include "layers_pool.h"

/*!
 * @defgroup layers_pool_dqnn Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN



/*!
 * @struct ai_layer_pool_dqnn
 * @ingroup layers_pool_dqnn
 * @brief pool_dqnn layer
 *
 * @ref forward_maxpool_is1os1
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_pool_dqnn_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_shape_2d pool_size;            /*!< pooling size */
  ai_shape_2d pool_stride;          /*!< pooling stride */
  ai_shape    pool_pad;             /*!< pooling pad, y,x border sizes */
//  ai_u32      pad_value;            /*!< pooling pad value */
} ai_layer_pool_dqnn;



/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles max pooling with binary input and binary output
 * @ingroup layers_pool_dqnn
 * @param layer conv2d_pool layer
 */
AI_INTERNAL_API
void forward_maxpool_is1os1(ai_layer *pLayer);



AI_API_DECLARE_END

#endif    /*LAYERS_POOL_DQNN_H*/
