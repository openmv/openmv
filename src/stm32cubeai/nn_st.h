/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2019 STMicroelectronics
 *
 * This work is licensed under the MIT license, see the file LICENSE for
 * details.
 */

#ifndef _NN_ST_H
#define _NN_ST_H
#include "imlib.h"
#include <stdint.h>
#include <string.h>

/* AI header files */
#include "ai_datatypes_defines.h"
#include "ai_platform.h"
#include "core_datatypes.h" /* AI_PLATFORM_RUNTIME_xxx definition */
#include "network.h"
#include "network_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  ai_handle network;
  ai_network_report report;
} ai_network_exec_ctx;

typedef struct {
  ai_network_exec_ctx *nn_exec_ctx_ptr;
} stnn_t;

void aiLogErr(const ai_error err, const char *fct);
ai_u32 aiBufferSize(const ai_buffer *buffer);
void aiPrintNetworkInfo(const ai_network_report *report);

void aiInit(const char *nn_name, stnn_t *net);
int aiRun(stnn_t *net, image_t *img, rectangle_t *roi);
void ai_transform_input(ai_buffer *input_net, image_t *img, ai_u8 *input_data,
                        rectangle_t *roi);
void aiDeInit(void);

#ifdef __cplusplus
}
#endif

#endif // _NN_ST_H
