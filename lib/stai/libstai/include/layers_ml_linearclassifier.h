/**
  ******************************************************************************
  * @file    layers_ml_linearclassifier.h
  * @author  SRA
  * @brief   header file of AI platform LinearClassifier datatypes
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
#ifndef LAYERS_LINEARCLASSIFIER_H
#define LAYERS_LINEARCLASSIFIER_H

#include "layers_common.h"
#include "layers_nl.h"

/*!
 * @defgroup layers_linearclassifier Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN


/*!
 * @struct ai_layer_linearclassifier
 * @ingroup layers_linearclassifier
 * @brief Linearclassifier layer
 *
 * The type of svmreg function is handled by the specific forward function
 * @ref forward_linearclassifier
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_linearclassifier_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  func_nl     nl_func;           /*!< function pointer to non linear transform */ \
  ai_bool multi_class;       /*!< Indicates whether to do OvR or multinomial */
  ai_bool has_classlabels_int;      /*!< if True, LinearClassifier returns classlabels int, else classlabels string */

} ai_layer_linearclassifier;


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Decodes the LinearClassifier ML operator.
 * @ingroup layers_linaerclassifier
 * @param layer linear classifier layer
 */
AI_INTERNAL_API
void forward_linearclassifier(ai_layer *pLayer);



AI_API_DECLARE_END

#endif    /*LAYERS_LINEARCLASSIFIER_H*/
