/**
  ******************************************************************************
  * @file    lite_operators.h
  * @author  AIS
  * @brief   main header file of AI platform lite operators list
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
#ifndef LITE_OPERATORS_H
#define LITE_OPERATORS_H

#include "lite_internal_apis.h"

#include "lite_bn_f32.h"
#include "lite_bn_integer.h"
#include "lite_conv2d.h"
#include "lite_conv2d_dqnn.h"
#include "lite_conv2d_is16.h"
#include "lite_convert_dqnn.h"
#include "lite_dense_if32.h"
#include "lite_dense_is1.h"
#include "lite_dense_is16.h"
#include "lite_dense_is1ws1.h"
#include "lite_dense_ws1.h"
#include "lite_gru_f32.h"
#include "lite_dw_dqnn.h"
#include "lite_pw_dqnn.h"
#include "lite_conv2d_sssa8_ch.h"

#include "lite_dense_is8os8ws8.h"
#include "lite_dense_is8os1ws1.h"
#include "lite_generic_float.h"
#include "lite_pool_f32.h"
#include "lite_maxpool_dqnn.h"
#include "lite_nl_generic_integer.h"
#include "lite_pad_generic.h"
#include "lite_pad_dqnn.h"
#include "lite_upsample_generic.h"
#include "lite_resize.h"
#include "lite_lstm.h"
#include "lite_argminmax.h"
#include "lite_pool_is8os8.h"

#endif /* LITE_OPERATORS_H */
