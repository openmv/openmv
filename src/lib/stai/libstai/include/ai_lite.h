/**
  ******************************************************************************
  * @file    ai_lite.h
  * @author  STMicroelectronics
  * @brief   Definitions and implementations of runtime-lite public APIs
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
#ifndef AI_LITE_H
#define AI_LITE_H

#include "ai_platform.h"
#include "stai.h"

#define LITE_API_ENTRY \
  /* LITE_API_ENTRY */

#define LITE_GRAPH_INIT(_inputs, _outputs, _activations, _weights, _cb, _cb_cookie) { \
  .inputs = (stai_ptr*)(_inputs), \
  .outputs = (stai_ptr*)(_outputs), \
  .activations = (stai_ptr*)(_activations), \
  .weights = (const stai_ptr*)(_weights), \
  .cb = (_cb), \
  .cb_cookie = (_cb_cookie), \
}


STAI_API_DECLARE_BEGIN

typedef enum {
  LITE_OK               = 0x0,
  LITE_KO_INPUTS        = (0x1 << 0),
  LITE_KO_OUTPUTS       = (0x1 << 1),
  LITE_KO_WEIGHTS       = (0x1 << 2),
  LITE_KO_ACTIVATIONS   = (0x1 << 3),
  LITE_KO_GRAPH         = (0x1 << 4),
  LITE_KO_API           = (0x1 << 5),
} lite_result;


typedef struct {
  stai_ptr*                 inputs;
  stai_ptr*                 outputs;
  stai_ptr*                 activations;
  const stai_ptr*           weights;
  const stai_event_cb       cb;
  void*                     cb_cookie;
} lite_graph;


STAI_API_DECLARE_END

#endif    /* AI_LITE_H */
