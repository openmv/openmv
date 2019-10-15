/**
  ******************************************************************************
  * @file    core_utils.h
  * @author  AST Embedded Analytics Research Platform
  * @date    16-Aug-2018
  * @brief   header file of core utils routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#ifndef __CORE_CONVERT_H_
#define __CORE_CONVERT_H_
#pragma once

#include "ai_platform.h"
#include "ai_platform_interface.h"

#include "core_common.h"

AI_API_DECLARE_BEGIN

/*!
 * @defgroup core_convert Core Convert Routines
 * @brief Implementation of core node format convertion routines (Q7 to float, ... etc.)
 */


/*!
 * @brief Convert input tensor array from input format to output format
 * @ingroup core_convert
 * @param[in] pNode in a handler to node (layer or operators) with tensor informations
 */
AI_INTERNAL_API
void    node_convert(ai_node *pNode);

/*!
 * @brief Convert a shape struct into a stride struct
 * @ingroup core_convert
 * @param[in] in a pointer to a shape to convert
 * @return a condverted stride datastruct
 */
AI_INTERNAL_API
ai_stride core_shape_to_stride(const ai_shape* in);

/*!
 * @brief Convert a shape 2D struct into a stride struct
 * @ingroup core_convert
 * @param[in] in a pointer to a shape to convert
 * @return a condverted stride datastruct
 */
AI_INTERNAL_API
ai_stride core_shape_2d_to_stride(const ai_shape_2d* in);

/*!
 * @brief Convert a shape struct into a ND stride struct (multi dimensional)
 * @ingroup core_convert
 * @param[in] in a pointer to a shape to convert
 * @return a condverted ND stride datastruct
 */
AI_INTERNAL_API
ai_stride_nd core_shape_to_stride_nd(const ai_shape* in);

/*!
 * @brief Convert a shape 2D struct into a ND stride struct (multi dimensional)
 * @ingroup core_convert
 * @param[in] in a pointer to a shape 2D to convert
 * @return a condverted ND stride datastruct
 */
AI_INTERNAL_API
ai_stride_nd core_shape_2d_to_stride_nd(const ai_shape_2d* in);

#endif    /*__CORE_CONVERT_H_*/
