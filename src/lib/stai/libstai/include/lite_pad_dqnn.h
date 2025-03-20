/**
  ******************************************************************************
  * @file    lite_pad_dqnn.h
  * @author  AIS
  * @brief   header file of AI platform lite padding kernel datatypes
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
#ifndef LITE_PADDING_DQNN_H
#define LITE_PADDING_DQNN_H


#include "ai_lite_interface.h"

/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Handles padding with binary input and binary output - Lite I/F
 * @ingroup lite_padding_dqnn
 */
LITE_API_ENTRY
void forward_lite_pad_is1os1(const ai_u32 *pDataIn_init,
                                 ai_u32 *pDataOut_init,
                                 const ai_i32 width_in,
                                 const ai_i32 width_out,
                                 const ai_i32 height_in,
                                 const ai_i32 height_out,
                                 const ai_u32 n_channel_out,
                                 const ai_i32 mode,
                                 const ai_u16 pads_x,
                                 const ai_u16 pads_y,
                                 const ai_u16 pads_x_r,
                                 const ai_u16 pads_y_b,
                                 const ai_u32 pad_value);


#endif    /*LITE_PADDING_DQNN_H*/
