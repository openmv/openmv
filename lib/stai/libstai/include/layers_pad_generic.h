/**
  ******************************************************************************
  * @file    layers_pad_generic.h
  * @author  AIS
  * @brief   header file of AI platform padding generic datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_PADDING_DQNN_H
#define LAYERS_PADDING_DQNN_H

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
 * @brief Handles generic padding in constant mode
 * @ingroup layers_generic_dqnn
 * @param layer pad layer
 */
AI_INTERNAL_API
void forward_pad_constant(ai_layer *pLayer);

/*!
 * @brief Handles generic padding in edge mode
 * @ingroup layers_generic_dqnn
 * @param layer pad layer
 */
AI_INTERNAL_API
void forward_pad_edge(ai_layer *pLayer);

/*!
 * @brief Handles generic padding in reflect mode
 * @ingroup layers_generic_dqnn
 * @param layer pad layer
 */
AI_INTERNAL_API
void forward_pad_reflect(ai_layer *pLayer);


/*!
 * @brief Handles generic padding in constant mode Channel 1st 8bit
 * @ingroup layers_generic_dqnn
 * @param layer pad layer
 */
AI_INTERNAL_API
void forward_pad_8bit_ch1st_3x3_constant(ai_layer* pLayer);


AI_API_DECLARE_END

#endif    /*LAYERS_PAD_GENERIC_H*/
