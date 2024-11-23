
/**
  ******************************************************************************
  * @file    layers_arminmax.h
  * @author  AIS
  * @brief   header file of AI platform generic layers datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_ARGMINMAX_H
#define LAYERS_ARGMINMAX_H

#include "layers_generic.h"


AI_API_DECLARE_BEGIN

/******************************************************************************/
/* Forward Functions Section                                                  */
/******************************************************************************/

/*!
 * @brief Compute the indices of the max elements of the input tensor's element along the provided axis.
 * @ingroup layers_generic
 * @param layer argminmax layer
 */
AI_INTERNAL_API
void forward_argmax_is8(ai_layer* layer);

/*!
 * @brief Compute the indices of the max elements of the input tensor's element along the provided axis.
 * @ingroup layers_generic
 * @param layer argminmax layer
 */
AI_INTERNAL_API
void forward_argmin_is8(ai_layer* layer);

AI_API_DECLARE_END

#endif    /*LAYERS_ARGMINMAX_H*/
