/**
  ******************************************************************************
  * @file    layers_iforest.h
  * @author  AIS
  * @brief   header file of AI platform iForest layers datatypes
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

#ifndef LAYERS_IFOREST_H
#define LAYERS_IFOREST_H

#include "layers_common.h"

/*!
 * @defgroup layers_ml Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN


/* Allowed tests branch in the iTrees */
typedef enum
{
  AI_IFOREST_BRANCH_LT_IDX = 0,
  AI_IFOREST_BRANCH_LEQ_IDX,
  AI_IFOREST_BRANCH_EQ_IDX,
  AI_IFOREST_BRANCH_END,
} ai_iforest_branch_e;


/*!
 * @struct ai_layer_iforest
 * @ingroup layers_iforest
 * @brief iForest layer
 *
 * The type of iforest function is handled by the specific forward function
 * @ref forward_iforest
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_iforest_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_float global_average_path_length;  /*!< global average path length used to normalized average path length*/
  ai_float score_threshold;             /*!< score threshold used to center the score around 0 */
} ai_layer_iforest;



/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Decodes the iforest ML algorithm.
 * @ingroup layers_iforest
 * @param layer iforest layer
 */
AI_INTERNAL_API
void forward_iforest(ai_layer *pLayer);



AI_API_DECLARE_END

#endif    /*LAYERS_IFOREST_H*/
