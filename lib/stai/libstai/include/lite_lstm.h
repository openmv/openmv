/**
  ******************************************************************************
  * @file    lite_lstm.h
  * @author  AIS
  * @brief   header file of AI platform lite lstm kernel
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
#ifndef LITE_LSTM_H
#define LITE_LSTM_H

#include "ai_lite_interface.h"
#include "ai_math_helpers.h"

#include "lite_internal_apis.h"

enum {
    AI_LITE_LSTM_INPUT = 0,
    AI_LITE_LSTM_FORGET = 1,
    AI_LITE_LSTM_CELL = 2,
    AI_LITE_LSTM_OUTPUT = 3,
    AI_LITE_LSTM_MAX
};

void forward_lite_lstm_if32of32wf32( AI_CONST ai_float* kernel,
                                     AI_CONST ai_float* recurrent,
                                     AI_CONST ai_float* bias,
                                     func_nl_lite activation_nl,  /**< activation nonlinearity (input to cell) */
                                     AI_CONST ai_float* activation_param,   /**< activation NL parameters */
                                     func_nl_lite recurrent_nl,   /**< recurrent nonlinearity (hidden to cell) */
                                     AI_CONST ai_float* recurrent_param,    /**< activation NL parameters */
                                     ai_float* hidden,
                                     AI_CONST ai_float* initial_hidden,
                                     ai_float* out_hidden,
                                     ai_u16 n_features,
                                     ai_u16 n_cell,
                                     AI_CONST ai_float* peepholes,
                                     ai_handle state,
                                     ai_float* cell,
                                     AI_CONST ai_float* initial_cell,
                                     func_nl_lite out_nl,
                                     AI_CONST ai_float* out_param,
                                     ai_float cell_clip,
                                     ai_float* data_in,
                                     ai_float** data_out,
                                     ai_ptr_offset *out_offset,
                                     ai_float* gates,
                                     ai_i32 timesteps,
                                     ai_i32 nb_t_out,
                                     ai_bool go_backwards,
                                     ai_bool reverse_seq,
                                     ai_bool stateful,
                                     ai_bool return_state);


void forward_lite_lstm_is8os8ws8(  AI_CONST ai_i8* kernel[AI_LITE_LSTM_MAX],
                                   AI_CONST ai_i8* recurrent[AI_LITE_LSTM_MAX],
                                   AI_CONST ai_i32* bias[AI_LITE_LSTM_MAX],
                                   AI_CONST ai_i8* initial_hidden,
                                   ai_i8* out_hidden,
                                   ai_u16 batch_size,
                                   ai_u16 n_features,
                                   ai_u16 n_cell,
                                   AI_CONST ai_i16* initial_cell,
                                   ai_i16* out_cell,
                                   ai_i8* data_in,
                                   const ai_i8 in_zeropoint,
                                   const ai_float in_scale,
                                   ai_i8* data_out,
                                   const ai_i8 out_zeropoint,
                                   const ai_float out_scale,
                                   const ai_float kernel_scale[AI_LITE_LSTM_MAX],
                                   const ai_float recurrent_scale[AI_LITE_LSTM_MAX],
                                   ai_i32 timesteps,
                                   ai_i32 nb_t_out,
                                   ai_bool go_backwards,
                                   ai_bool reverse_seq,
                                   ai_bool stateful,
                                   ai_bool time_major,
                                   ai_bool return_state,
                                   ai_i32 scratch_size,
                                   ai_i8 *p_scratch_data);

#endif /* LITE_LSTM_H */

