/**
  ******************************************************************************
  * @file    layers_svc.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform SVM Classifier (SVC) datatypes
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

#ifndef LAYERS_SVC_H
#define LAYERS_SVC_H

#include "layers_common.h"

/*!
 * @defgroup layers_svc Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN


/* SVM classifier (SVC) kernel types */
typedef enum ai_svc_kernel_e_ {
  AI_SVC_KERNEL_LINEAR = 0,
  AI_SVC_KERNEL_POLYNOMIAL,
  AI_SVC_KERNEL_RBF,
  AI_SVC_KERNEL_SIGMOID,
  AI_SVC_KERNEL_UNSUPPORTED
} ai_svc_kernel_e;


/*!
 * @struct ai_layer_svc
 * @ingroup layers_svc
 * @brief SVM Classifier (SVC) layer
 *
 * The type of svc function is handled by the specific forward function
 * @ref forward_svc
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_svc_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_float gamma;                        /*!< kernel coefficient for rbf, polynomial and sigmoid functions */
  ai_float coef0;                        /*!< term in polynomial and sigmoid functions */
  ai_u32 degree;                         /*!< polynomial function degree */
  ai_svc_kernel_e kernel_type;           /*!< kernel type : see ai_svm_kernel_e */
  ai_bool proba_support;                 /*!< whether or not use the parameters learned in Platt scaling */
  ai_bool has_classlabels_int;           /*!< if True, SVC returns classlabels int, else classlabels string */
} ai_layer_svc;


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Decodes the SVM Classifier ML operator.
 * @ingroup layers_svc
 * @param layer svm classifier layer
 */
AI_INTERNAL_API
void forward_svc(ai_layer *pLayer);


AI_API_DECLARE_END

#endif    /*LAYERS_SVC_H*/
