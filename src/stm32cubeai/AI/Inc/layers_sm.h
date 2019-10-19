/**
  ******************************************************************************
  * @file    layers_sm.h
  * @author  AST Embedded Analytics Research Platform
  * @date    18-Apr-2018
  * @brief   header file of AI platform non softmax layer datatype
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright
  *      notice, this list of conditions and the following disclaimer in the
  *      documentation and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its
  *      contributors may be used to endorse or promote products derived from
  *      this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  * POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#ifndef __LAYERS_SM_H_
#define __LAYERS_SM_H_
#pragma once

#include "layers_common.h"

/*!
 * @defgroup layers SoftMax Layer Definitions
 * @brief definition 
 *
 */

AI_API_DECLARE_BEGIN

/*!
 * @brief Softmax normalization computed on an array of fixed point channels
 * @ingroup layers_sm
 * @param out opaque handler to output channel array
 * @param in  opaque handler to input channel array
 * @param in_size  total size (number of elements) to process on the input
 * @param channel_size number of elements of the input channel
 * @param in_channel_step number of elements to move to next input element
 * @param out_channel_step number of elements to move to next output element
 */
AI_INTERNAL_API
void sm_func_sm_array_fixed(ai_handle out, const ai_handle in,
                            const ai_size in_size,
                            const ai_size channel_size,
                            const ai_size in_channel_step,
                            const ai_size out_channel_step);

/*!
 * @brief Computes the activations of a fixed point softmax nonlinear layer.
 * @ingroup layers_sm
 * @param layer the softmax (sm) layer
 */
AI_INTERNAL_API
void forward_sm_fixed(ai_layer *pLayer);

AI_API_DECLARE_END

#endif    /*__LAYERS_SM_H_*/

