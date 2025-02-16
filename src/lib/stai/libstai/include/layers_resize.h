/**
  ******************************************************************************
  * @file    layers_resize.h
  * @author  STMicroelectronics
  * @brief   header file of AI platform padding generic datatypes
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
#ifndef LAYERS_RESIZE_H
#define LAYERS_RESIZE_H

#include "layers_generic.h"

/*!
 * @defgroup layers_pad_generic Layers Definitions
 * @brief definition
 *
 */
AI_API_DECLARE_BEGIN


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles generic resizing in nearest mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_if32of32(ai_layer* layer);

/*!
 * @brief Handles generic resizing in bilinear mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_bilinear_if32of32(ai_layer *pLayer);

/*!
 * @brief Handles generic resizing in bilinear mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_nearest_if32of32(ai_layer *pLayer);

/*!
 * @brief Handles generic resizing in bilinear mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_bilinear_is16os16(ai_layer *pLayer);

/*!
 * @brief Handles generic resizing in bilinear mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_nearest_is16os16(ai_layer *pLayer);

/*!
 * @brief Handles generic resizing in bilinear mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_bilinear_is8os8(ai_layer *pLayer);

/*!
 * @brief Handles generic resizing in bilinear mode
 * @ingroup layers_generic
 * @param layer resize layer
 */
AI_INTERNAL_API
void forward_resize_nearest_is8os8(ai_layer *pLayer);


AI_API_DECLARE_END

#endif    /*LAYERS_PAD_GENERIC_H*/
