/**
  ******************************************************************************
  * @file    ai_lite_inspect.h
  * @author  STMicroelectronics
  * @brief   Definitions and implementations of runtime-lite inspection routines
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
#ifndef AI_LITE_INSPECT_H
#define AI_LITE_INSPECT_H
#include "ai_platform.h"

// #define HAS_LITE_INSPECT

#ifdef HAS_LITE_INSPECT
#include "stai_debug.h"

#define LITE_INSPECT_CB(flags, node_id, data_ptr, data_size, data_fmt, data_id, data_pos) { \
  if (graph->cb) { \
    graph->cb((const void*)(graph->cb_cookie), \
              (const stai_flags)(flags), \
              (const int32_t)(node_id), (const void*)(data_ptr), (const int32_t)(data_size), \
              (const int32_t)(data_fmt), (const int32_t)(data_id), (const int32_t)(data_pos)); \
  } \
}

#else

#define LITE_INSPECT_CB(flags, node_id, data_ptr, data_size, data_fmt, data_id, data_pos) { \
  do { /* LITE_INSPECT_CB() */ } while (0); \
}

#endif    /* HAS_LITE_INSPECT */

#endif    /* AI_LITE_INSPECT_H */
