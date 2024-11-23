/**
  ******************************************************************************
  * @file    layers_pad_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform DQNN padding datatypes
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
#ifndef LAYERS_PADDING_DQNN_H
#define LAYERS_PADDING_DQNN_H

#include "layers_common.h"
#include "layers_generic.h"

/*!
 * @defgroup layers_generic_dqnn Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN


/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/


/*!
 * @brief Handles padding with binary input and binary output
 * @ingroup layers_generic_dqnn
 * @param layer pad layer
 */
AI_INTERNAL_API
void forward_pad_is1os1(ai_layer *pLayer);


AI_API_DECLARE_END

#endif    /*LAYERS_PADDING_DQNN_H*/
