/**
  ******************************************************************************
  * @file    layers_rnn.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of RNN layers
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef LAYERS_RNN_H
#define LAYERS_RNN_H

#include "layers_common.h"
#include "layers_nl.h"

AI_API_DECLARE_BEGIN

/*!
 * @struct ai_layer_lstm
 * @ingroup layers
 * @brief LSTM layer with generic nonlinearities and peephole connections
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_lstm_ {
  AI_LAYER_STATEFUL_FIELDS_DECLARE
  ai_size n_units;        /**< size of the hidden RNN state */
  func_nl activation_nl;  /**< activation nonlinearity (input to cell) */
  func_nl recurrent_nl;   /**< recurrent nonlinearity (hidden to cell) */
  func_nl out_nl;         /**< output nonlinearity (cell to hidden) */
  ai_bool go_backwards;   /**< process reversed input */
  ai_bool return_state;   /**< return state */
  ai_bool reverse_seq;    /**< reverse output sequence */
  ai_float cell_clip;     /**< cell clip value */
} ai_layer_lstm;

/*!
 * @struct ai_layer_gru
 * @ingroup layers
 * @brief Gated Recurrent Unit (GRU) layer with generic nonlinearities
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_gru_ {
  AI_LAYER_STATEFUL_FIELDS_DECLARE
  ai_size n_units;        /**< size of the hidden RNN state */
  func_nl activation_nl;  /**< activation nonlinearity (input to cell) */
  func_nl recurrent_nl;   /**< recurrent nonlinearity (hidden to cell) */
  ai_bool reset_after;
  ai_bool return_state;
  ai_bool go_backwards;   /**< process reversed input */
  ai_bool reverse_seq;    /**< reverse output sequence */
} ai_layer_gru;

/*!
 * @struct ai_layer_rnn
 * @ingroup layers
 * @brief Simple Recurrent Neural Network (RNN) layer
 */
typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_rnn_ {
  AI_LAYER_COMMON_FIELDS_DECLARE
  ai_size n_units;        /**< size of the hidden RNN state */
  func_nl activation_nl;  /**< activation nonlinearity (input to hidden) */
  ai_bool go_backwards;   /**< process reversed input */
  ai_bool reverse_seq;    /**< reverse output sequence */
  ai_bool return_state;
} ai_layer_rnn;


/*!
 * @brief Allocate states for a stateful network.
 * @ingroup layers
 *
 * Function used to allocate states of a stateful network.
 */
void _allocate_states(ai_float **states, ai_u32 size_in_bytes);


/*!
 * @brief Deallocate states for a stateful network.
 * @ingroup layers
 *
 * Function used to deallocate states of a stateful network.
 */
void _deallocate_states(ai_float **states);


/*!
 * @brief Initialize a Long-Short Term Memory (LSTM) layer.
 * @ingroup layers
 *
 * Function used to initialize lstm internal state
 */
AI_INTERNAL_API
void init_lstm(ai_layer * layer);


/*!
 * @brief Destroy a Long-Short Term Memory (LSTM) layer state.
 * @ingroup layers
 *
 * Function used to destroy lstm internal state
 */
AI_INTERNAL_API
void destroy_lstm(ai_layer * layer);


/*!
 * @brief Computes the activations of a Long-Short Term Memory (LSTM) layer.
 * @ingroup layers
 *
 * Implements a Long-Short Term Layer with peephole connections:
 * \f{eqnarray*}{
 *    i_t &=& \sigma_a(x_t W_{xi} + h_{t-1} W_{hi}
 *            + w_{ci} \odot c_{t-1} + b_i)\\
 *    f_t &=& \sigma_a(x_t W_{xf} + h_{t-1} W_{hf}
 *            + w_{cf} \odot c_{t-1} + b_f)\\
 *    c_t &=& f_t \odot c_{t - 1}
 *            + i_t \odot \sigma_r(x_t W_{xc} + h_{t-1} W_{hc} + b_c)\\
 *    o_t &=& \sigma_a(x_t W_{xo} + h_{t-1} W_{ho} + w_{co} \odot c_t + b_o)\\
 *    h_t &=& o_t \odot \sigma_o(c_t)
 * \f}
 * where \f$\sigma_a\f$ is the activation nonlinearity, \f$\sigma_r\f$ is the
 * recurrent nonlinearity and \f$\sigma_o\f$ is the out nonlinearity. The
 * \f$W_x\f$, \f$W_h\f$ and \f$W_c\f$ weights are sliced from the kernel,
 * recurrent and peephole weights.
 *
 * @param layer the LSTM layer
 */
AI_INTERNAL_API
void forward_lstm(ai_layer * layer);

AI_INTERNAL_API
void forward_lstm_is8os8ws8(ai_layer * layer);


/*!
 * @brief Initialize a Gated Recurrent Unit (GRU) layer.
 * @ingroup layers
 *
 * Function used to initialize gru internal state
 */
AI_INTERNAL_API
void init_gru(ai_layer * layer);


/*!
 * @brief Destroy a Gated Recurrent Unit (GRU) layer state.
 * @ingroup layers
 *
 * Function used to destroy gru internal state
 */
AI_INTERNAL_API
void destroy_gru(ai_layer * layer);

/*!
 * @brief Computes the activations of a Gated Recurrent Unit (GRU) layer.
 * @ingroup layers
 *
 * Implements a Gated Recurrent Unit with the formula:
 * \f{eqnarray*}{
 *    r_t &=& \sigma_a(x_t W_{xr} + h_{t - 1} W_{hr} + b_r) \\
 *    z_t &=& \sigma_a(x_t W_{xz} + h_{t - 1} W_{hz} + b_z) \\
 *    c_t &=& \sigma_r(x_t W_{xc} + r_t \odot (h_{t - 1} W_{hc} + b_{hc}) + b_c)
 *            \qquad \textnormal{when reset after is true} \\
 *    c_t &=& \sigma_r(x_t W_{xc} + (r_t \odot h_{t - 1}) W_{hc} + b_{hc} + b_c)
 *            \qquad \textnormal{when reset after is false (default)} \\
 *    h_t &=& (1 - z_t) \odot h_{t - 1} + z_t \odot c_t
 * \f}
 * where \f$\sigma_a\f$ is the activation nonlinearity and \f$\sigma_r\f$ is
 * the recurrent nonlinearity. The weights are sliced from the kernel and
 * recurrent weights.
 *
 * @param layer the GRU layer
 */
AI_INTERNAL_API
void forward_gru(ai_layer * layer);

/*!
 * @brief Computes the activations of a Recurrent Neural Network (RNN) layer.
 * @ingroup layers
 *
 * Implements a recurrent layer with the formula:
 * \f{eqnarray*}{
 *    h_t &=& \sigma_a(x_t W_{xr} + h_{t - 1} W_{hr} + b_r)
 * \f}
 * where \f$\sigma_a\f$ is the activation nonlinearity. The weights are sliced
 * from the kernel and recurrent weights.
 *
 * @param layer the RNN layer
 */
AI_INTERNAL_API
void forward_rnn(ai_layer * layer);

AI_API_DECLARE_END

#endif /* LAYERS_RNN_H */
