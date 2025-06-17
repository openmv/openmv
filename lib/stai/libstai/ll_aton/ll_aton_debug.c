/**
 ******************************************************************************
 * @file    ll_aton_debug.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON LL library support for debugging (buffer dumpers, ...).
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#if defined(DUMP_DEBUG_API)
#warning "deprecated use of DUMP_DEBUG_API (define LL_ATON_DUMP_DEBUG_API instead)"
#define LL_ATON_DUMP_DEBUG_API
#endif

#if defined(LL_ATON_DUMP_DEBUG_API)

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ll_aton_util.h" // Leave blank line after the include

#include "ll_aton_NN_interface.h"
#include "ll_aton_debug.h"

LL_ATON_WEAK void *LL_ATON_physical_to_virtual(uintptr_t addr)
{
  return (void *)(uintptr_t)addr;
}

static const LL_Buffer_InfoTypeDef *__get_buffer(const char *bufname, int in, const NN_Interface_TypeDef *nn_interface)
{
  const LL_Buffer_InfoTypeDef *bufs =
      (in == BUFF_IN ? nn_interface->input_buffers_info()
                     : (in == BUFF_OUT ? nn_interface->output_buffers_info() : nn_interface->internal_buffers_info()));

  while (bufs != NULL && bufs->name != NULL)
  {
    if (strcmp(bufs->name, bufname) == 0)
    {
      return bufs;
    }
    bufs++;
  }
  return NULL;
}

static void __set_buffer(const LL_Buffer_InfoTypeDef *buf, unsigned val)
{
  uint32_t len = LL_Buffer_len(buf);
  int t;
  unsigned char *p = (unsigned char *)LL_Buffer_addr_start(buf);
  for (t = 0; t < len; t++)
    p[t] = (unsigned char)val;
}

static void __set_all_buffers(const LL_Buffer_InfoTypeDef *bufs, unsigned val)
{
  while (bufs != NULL && bufs->name != NULL)
  {
    __set_buffer(bufs, val);
    bufs++;
  }
}

#ifndef NDEBUG
static float Q_to_floating(int i, int Qm, int Qn)
{
  if (Qn >= 0)
    return ((float)i / (float)((int)1 << Qn));
  if (Qn < 0)
    return ((float)i * (float)((int)1 << -Qn));
  return 0.f;
}

static void __dump_buffer_info(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_PRINTF("dumping [%s] len = %d phy: %p\n", buf->name, (int)len, (void *)LL_Buffer_addr_start(buf));

  LL_ATON_ASSERT(buf->ndims >= 3);
  int32_t dim_b_c = buf->batch;
  int32_t dim_c = buf->shape[buf->ndims - 1];
  dim_c = (dim_c == 0) ? 1 : dim_c;
  LL_ATON_ASSERT(dim_b_c != 0);
  int32_t num_batch = dim_c / dim_b_c;

  /* output onnx shape */
  LL_ATON_PRINTF("  name = %s, onnx shape(", buf->name);
  int32_t channel_onnx_idx = buf->ndims - 3;
  uint32_t i;
  for (i = 0; i < channel_onnx_idx; i++)
  {
    int32_t tmp = (int32_t)buf->shape[i];
    tmp = (tmp == 0) ? 1 : tmp;
    if (tmp > 1)
      LL_ATON_PRINTF("%" PRId32 ",", tmp);
  }
  LL_ATON_PRINTF("%" PRId32 ",", dim_c);
  for (; i < (buf->ndims - 1); i++)
  {
    int32_t tmp = (int32_t)buf->shape[i];
    tmp = (tmp == 0) ? 1 : tmp;
    if (tmp > 1)
    {
      LL_ATON_PRINTF("%" PRId32, tmp);
      if (i != (buf->ndims - 2))
        LL_ATON_PRINTF(",");
    }
  }

  /* output batch & num_batch */
  LL_ATON_PRINTF("), num_batch=%" PRId32 ", batch=%" PRId32 ", ", num_batch, dim_b_c);

  /* output memory layout */
  LL_ATON_PRINTF("memory layout(");
  bool leading = true;
  for (i = 0; i < channel_onnx_idx; i++)
  {
    int32_t tmp = (int32_t)buf->shape[i];
    tmp = (tmp == 0) ? 1 : tmp;
    if (!leading || (tmp > 1))
    {
      LL_ATON_PRINTF("%" PRId32 ",", tmp);
      leading = false;
    }
  }
  if (num_batch != 1)
  {
    LL_ATON_PRINTF("%" PRId32 ",", num_batch);
    leading = false;
  }
  for (; i < (buf->ndims - 1); i++)
  {
    int32_t tmp = (int32_t)buf->shape[i];
    tmp = (tmp == 0) ? 1 : tmp;
    if (!leading || (tmp > 1))
    {
      leading = false;
      LL_ATON_PRINTF("%" PRId32 "", tmp);
      if (i != (buf->ndims - 2))
        LL_ATON_PRINTF(",");
    }
  }
  if (dim_b_c > 1)
  {
    LL_ATON_PRINTF(",%" PRId32, dim_b_c);
  }

  /* output format information */
  LL_ATON_PRINTF("), Qmn=(%d,%d,%s)", buf->Qm, buf->Qn, buf->Qunsigned ? "unsigned" : "signed");
  if (buf->scale != 0)
  {
    LL_ATON_PRINTF(", \"has scale");
    if (buf->offset != 0)
    {
      LL_ATON_PRINTF("/has offset\"");
    }
    else
    {
      LL_ATON_PRINTF("\"");
    }
  }
  else
  { // no scale
    if (buf->offset != 0)
    {
      LL_ATON_PRINTF(", \"has offset\"");
    }
  }

  int is_scale_off = buf->scale != 0 || buf->offset != 0;
  LL_ATON_PRINTF(" %s", is_scale_off ? (buf->per_channel ? "per channel" : "per tensor") : "");
  if (is_scale_off)
  {
    if (buf->per_channel == 0)
      LL_ATON_PRINTF(" (%f, %d)", buf->scale[0], (int)buf->offset[0]);
    else
    {
      for (uint32_t c = 0; c < dim_c; c++)
      {
        LL_ATON_PRINTF(" (%g, %d)", buf->scale[c], (int)buf->offset[c]);
        if (c < (dim_c - 1))
          LL_ATON_PRINTF(",");
      }
    }
  }
  LL_ATON_PUTS("");
}

static void __dump_buffer_Qmn_8bit(const LL_Buffer_InfoTypeDef *buf)
{
  // uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_ASSERT(LL_Buffer_bits(buf) == 8);

  int32_t ndims = (int32_t)buf->ndims;
  LL_ATON_ASSERT(ndims >= 4);

  int32_t dim_m = 1;
  int32_t dim_h = (int32_t)buf->shape[(ndims - 4) + 1];
  int32_t dim_w = (int32_t)buf->shape[(ndims - 4) + 2];
  int32_t dim_c = (int32_t)buf->shape[(ndims - 4) + 3];

  for (int32_t i = 0; i <= ndims - 4; i++)
  {
    uint32_t tmp = (int32_t)buf->shape[i];
    dim_m *= (tmp == 0 ? 1 : tmp);
  }

  dim_m = dim_m == 0 ? 1 : dim_m;
  dim_h = dim_h == 0 ? 1 : dim_h;
  dim_w = dim_w == 0 ? 1 : dim_w;
  dim_c = dim_c == 0 ? 1 : dim_c;

  int32_t Qm = buf->Qm;

  int32_t Qn = buf->Qn;
  // int32_t Qsign = buf->Qunsigned == 1 ? 0 : 1;

  int32_t dim_b_c = buf->batch;
  // int32_t num_batch = dim_c / dim_b_c;

  // int32_t dim_k_m = 1; // buf->kbatch;
  // int32_t  = dim_m / dim_k_m;

  LL_ATON_PRINTF("uint8 [");
  uint8_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int is_unsigned = buf->Qunsigned;
  for (int32_t m = 0; m < dim_m; ++m)
  { // M
    if (m)
      LL_ATON_PRINTF("\n");
    LL_ATON_PRINTF("[");
    for (int32_t c = 0; c < dim_c; ++c)
    { // C
      if (c)
        LL_ATON_PRINTF("\n  ");
      LL_ATON_PRINTF("[");
      for (int32_t h = 0; h < dim_h; ++h)
      { // H
        if (h)
          LL_ATON_PRINTF("\n   ");
        LL_ATON_PRINTF("[ ");
        for (int32_t w = 0; w < dim_w; ++w)
        { // W
          uint32_t off_orlando = /* m * dim_c * dim_h * dim_w + */ (c / dim_b_c) * dim_b_c * dim_h * dim_w +
                                 h * dim_w * dim_b_c + w * dim_b_c + (c % dim_b_c);
          int32_t data = p[off_orlando];
          data = is_unsigned ? data : (int8_t)(data);
          if (Qn == 0)
            LL_ATON_PRINTF("%-.0f. ", Q_to_floating(data, Qm, Qn));
          else
            LL_ATON_PRINTF("%0.6f ", Q_to_floating(data, Qm, Qn));
        }
        LL_ATON_PRINTF("]");
      }
      LL_ATON_PRINTF("]");
      if (c < (dim_c - 1))
        LL_ATON_PRINTF("\n");
    }
    LL_ATON_PRINTF("]\n");
    p += dim_c * dim_h * dim_w;
  }
  LL_ATON_PRINTF("]\n");
}

static void __dump_buffer_Qmn_16bit(const LL_Buffer_InfoTypeDef *buf)
{
  // uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_ASSERT(LL_Buffer_bits(buf) == 16);

  int32_t ndims = (int32_t)buf->ndims;
  LL_ATON_ASSERT(ndims >= 4);

  int32_t dim_m = 1;
  int32_t dim_h = (int32_t)buf->shape[(ndims - 4) + 1];
  int32_t dim_w = (int32_t)buf->shape[(ndims - 4) + 2];
  int32_t dim_c = (int32_t)buf->shape[(ndims - 4) + 3];

  for (int32_t i = 0; i <= ndims - 4; i++)
  {
    uint32_t tmp = (int32_t)buf->shape[i];
    dim_m *= (tmp == 0 ? 1 : tmp);
  }

  LL_ATON_PRINTF("dim_m=%" PRId32 " off=%" PRId32, dim_m, dim_c * dim_h * dim_w);

  int32_t Qm = buf->Qm;
  int32_t Qn = buf->Qn;
  // int32_t Qsign = buf->Qunsigned == 1 ? 0 : 1;

  int32_t dim_b_c = buf->batch;
  // int32_t num_batch = dim_c / dim_b_c;

  // int32_t dim_k_m = 1; // buf->kbatch;
  // int32_t  = dim_m / dim_k_m;

  LL_ATON_PRINTF("uint16 [");
  uint16_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int is_unsigned = buf->Qunsigned;
  for (int32_t m = 0; m < dim_m; ++m)
  { // M
    if (m)
      LL_ATON_PRINTF("\n");
    LL_ATON_PRINTF("[");
    for (int32_t c = 0; c < dim_c; ++c)
    { // C
      if (c)
        LL_ATON_PRINTF("\n  ");
      LL_ATON_PRINTF("[");
      for (int32_t h = 0; h < dim_h; ++h)
      { // H
        if (h)
          LL_ATON_PRINTF("\n   ");
        LL_ATON_PRINTF("[");
        for (int32_t w = 0; w < dim_w; ++w)
        { // W
          uint32_t off_orlando = /* m * dim_c * dim_h * dim_w +*/ (c / dim_b_c) * dim_b_c * dim_h * dim_w +
                                 h * dim_w * dim_b_c + w * dim_b_c + (c % dim_b_c);
          // LL_ATON_PRINTF("m=%d c=%d c/dim_b_c=%d h=%d w=%d c %% dim_b_c=%d off=%d\n",m,c,c/dim_b_c, h,w,c %
          // dim_b_c,off_orlando);
          int32_t data = p[off_orlando];
          data = is_unsigned ? data : (int16_t)(data);
#if 0
          if (Qn == 0)
            LL_ATON_PRINTF("%-.0f. ", Q_to_floating(data, Qm, Qn));
          else
#endif
          if (data == ((1 << (Qm)) - 1))
            LL_ATON_PRINTF("+sat ");
          else if (data == -(1 << (Qm)))
            LL_ATON_PRINTF("-sat ");
          else if (Qn == 0)
            LL_ATON_PRINTF("%-.0f ", Q_to_floating(data, Qm, Qn));
          else
            LL_ATON_PRINTF("%0.6f ", Q_to_floating(data, Qm, Qn));
        }
        LL_ATON_PRINTF("]");
      }
      LL_ATON_PRINTF("]");
      if (c < (dim_c - 1))
        LL_ATON_PRINTF("\n");
    }
    LL_ATON_PRINTF("]\n");
    p += dim_c * dim_h * dim_w;
  }
  LL_ATON_PRINTF("]\n");
}

static void __dump_buffer_Qmn_32bit(const LL_Buffer_InfoTypeDef *buf)
{
  // uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_ASSERT(LL_Buffer_bits(buf) == 32);

  int32_t ndims = (int32_t)buf->ndims;
  LL_ATON_ASSERT(ndims >= 4);

  int32_t dim_m = 1;
  int32_t dim_h = (int32_t)buf->shape[(ndims - 4) + 1];
  int32_t dim_w = (int32_t)buf->shape[(ndims - 4) + 2];
  int32_t dim_c = (int32_t)buf->shape[(ndims - 4) + 3];

  for (int32_t i = 0; i <= ndims - 4; i++)
  {
    uint32_t tmp = (int32_t)buf->shape[i];
    dim_m *= (tmp == 0 ? 1 : tmp);
  }

  LL_ATON_PRINTF("dim_m=%" PRId32 " off=%" PRId32, dim_m, dim_c * dim_h * dim_w);

  int32_t Qm = buf->Qm;
  int32_t Qn = buf->Qn;
  // int32_t Qsign = buf->Qunsigned == 1 ? 0 : 1;

  int32_t dim_b_c = buf->batch;

  // int32_t num_batch = dim_c / dim_b_c;

  // int32_t dim_k_m = 1; // buf->kbatch;
  // int32_t  = dim_m / dim_k_m;

  LL_ATON_PRINTF("int32 [");
  int32_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  for (int32_t m = 0; m < dim_m; ++m)
  { // M
    if (m)
      LL_ATON_PRINTF("\n");
    LL_ATON_PRINTF("[");
    for (int32_t c = 0; c < dim_c; ++c)
    { // C
      if (c)
        LL_ATON_PRINTF("\n  ");
      LL_ATON_PRINTF("[");
      for (int32_t h = 0; h < dim_h; ++h)
      { // H
        if (h)
          LL_ATON_PRINTF("\n   ");
        LL_ATON_PRINTF("[");
        for (int32_t w = 0; w < dim_w; ++w)
        { // W
          uint32_t off_orlando = /* m * dim_c * dim_h * dim_w +*/ (c / dim_b_c) * dim_b_c * dim_h * dim_w +
                                 h * dim_w * dim_b_c + w * dim_b_c + (c % dim_b_c);
          // LL_ATON_PRINTF("m=%d c=%d c/dim_b_c=%d h=%d w=%d c %% dim_b_c=%d off=%d\n",m,c,c/dim_b_c, h,w,c %
          // dim_b_c,off_orlando);
          int32_t data = p[off_orlando];

          if (Qn == 0)
            LL_ATON_PRINTF("%-.0f. ", Q_to_floating(data, Qm, Qn));
          else
            LL_ATON_PRINTF("%0.6f ", Q_to_floating(data, Qm, Qn));
        }
        LL_ATON_PRINTF("]");
      }
      LL_ATON_PRINTF("]");
      if (c < (dim_c - 1))
        LL_ATON_PRINTF("\n");
    }
    LL_ATON_PRINTF("]\n");
    p += dim_c * dim_h * dim_w;
  }
  LL_ATON_PRINTF("]\n");
}

static void __dump_buffer_float(const LL_Buffer_InfoTypeDef *buf)
{
  LL_ATON_ASSERT(buf->type == DataType_FLOAT);

  int32_t ndims = (int32_t)buf->ndims;
  LL_ATON_ASSERT(ndims >= 4);

  int32_t dim_m = 1;
  int32_t dim_h = (int32_t)buf->shape[(ndims - 4) + 1];
  int32_t dim_w = (int32_t)buf->shape[(ndims - 4) + 2];
  int32_t dim_c = (int32_t)buf->shape[(ndims - 4) + 3];

  for (int32_t i = 0; i <= ndims - 4; i++)
  {
    uint32_t tmp = (int32_t)buf->shape[i];
    dim_m *= (tmp == 0 ? 1 : tmp);
  }

  dim_m = dim_m == 0 ? 1 : dim_m;
  dim_h = dim_h == 0 ? 1 : dim_h;
  dim_w = dim_w == 0 ? 1 : dim_w;
  dim_c = dim_c == 0 ? 1 : dim_c;

  int32_t dim_b_c = buf->batch;

  LL_ATON_PRINTF("[");
  float *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  for (int32_t m = 0; m < dim_m; ++m)
  { // M
    if (m)
      LL_ATON_PRINTF("\n");
    LL_ATON_PRINTF("[");
    for (int32_t c = 0; c < dim_c; ++c)
    { // C
      if (c)
        LL_ATON_PRINTF("\n  ");
      LL_ATON_PRINTF("[");
      for (int32_t h = 0; h < dim_h; ++h)
      { // H
        if (h)
          LL_ATON_PRINTF("\n   ");
        LL_ATON_PRINTF("[");
        for (int32_t w = 0; w < dim_w; ++w)
        { // W
          uint32_t off_orlando = /* m * dim_c * dim_h * dim_w +*/ (c / dim_b_c) * dim_b_c * dim_h * dim_w +
                                 h * dim_w * dim_b_c + w * dim_b_c + (c % dim_b_c);
          float data = p[off_orlando];
          LL_ATON_PRINTF("%f. ", data);
        }
        LL_ATON_PRINTF("]");
      }
      LL_ATON_PRINTF("]");
      if (c < (dim_c - 1))
        LL_ATON_PRINTF("\n");
    }
    LL_ATON_PRINTF("]\n");
    p += dim_c * dim_h * dim_w;
  }
  LL_ATON_PRINTF("]\n");
}

static void __dump_buffer_raw_Qmn_8bit(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_ASSERT(LL_Buffer_bits(buf) == 8);
  int32_t Qm = buf->Qm;
  int32_t Qn = buf->Qn;
  // int32_t Qsign = buf->Qunsigned == 1 ? 0 : 1;
  const char *pformat = (Qn == 0) ? "%-.0f. " : "%0.3f ";

  uint8_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int is_unsigned = buf->Qunsigned;
  int cont = 0;
  for (unsigned i = 0; i < len; i++)
  {
    int32_t data = p[i];
    data = is_unsigned ? data : (int8_t)(data);
    LL_ATON_PRINTF(pformat, Q_to_floating(data, Qm, Qn));
    if (cont++ == 30)
    {
      cont = 0;
      LL_ATON_PUTS("");
    }
  }
  LL_ATON_PRINTF("\n");
}

static void __dump_buffer_raw_Qmn_16bit(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_ASSERT(LL_Buffer_bits(buf) == 16);
  int32_t Qm = buf->Qm;
  int32_t Qn = buf->Qn;
  // int32_t Qsign = buf->Qunsigned == 1 ? 0 : 1;
  const char *pformat = (Qn == 0) ? "%-.0f. " : "%0.3g ";

  uint16_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int is_unsigned = buf->Qunsigned;
  int cont = 0;
  for (unsigned i = 0; i < len / 2; i++)
  {
    int32_t data = p[i];
    data = is_unsigned ? data : (int16_t)(data);
    LL_ATON_PRINTF(pformat, Q_to_floating(data, Qm, Qn));
    if (cont++ == 30)
    {
      cont = 0;
      LL_ATON_PUTS("");
    }
  }
  LL_ATON_PRINTF("\n");
}

static void __dump_buffer_raw_Qmn_32bit(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  LL_ATON_ASSERT(LL_Buffer_bits(buf) == 32);

  int32_t Qm = buf->Qm;
  int32_t Qn = buf->Qn;
  // int32_t Qsign = buf->Qunsigned == 1 ? 0 : 1;
  const char *pformat = (Qn == 0) ? "%-.0f. " : "%0.3g ";

  int32_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int cont = 0;
  for (unsigned i = 0; i < len / 4; i++)
  {
    int32_t data = p[i];
    LL_ATON_PRINTF(pformat, Q_to_floating(data, Qm, Qn));
    if (cont++ == 30)
    {
      cont = 0;
      LL_ATON_PUTS("");
    }
  }
  LL_ATON_PRINTF("\n");
}

static void __dump_buffer_raw_16bit(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  uint16_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int is_unsigned = buf->Qunsigned;

  int cont = 0;
  for (unsigned i = 0; i < len / 2; i++)
  {
    int32_t data = p[i];
    data = is_unsigned ? data : (int16_t)(data);
    LL_ATON_PRINTF("%" PRId32 " ", data);
    if (cont++ == 30)
    {
      cont = 0;
      LL_ATON_PUTS("");
    }
  }
  LL_ATON_PRINTF("\n");
}

static void __dump_buffer_raw_32bit(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  uint32_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));

  int cont = 0;
  int is_unsigned = buf->Qunsigned;
  for (unsigned i = 0; i < len / 4; i++)
  {
    uint32_t data = p[i];
    if (is_unsigned)
      LL_ATON_PRINTF("%" PRIu32 " ", data);
    else
      LL_ATON_PRINTF("%" PRId32 " ", (int32_t)data);
    if (cont++ == 30)
    {
      cont = 0;
      LL_ATON_PUTS("");
    }
  }
  LL_ATON_PRINTF("\n");
}

static void __dump_buffer_raw_8bit(const LL_Buffer_InfoTypeDef *buf)
{
  uintptr_t len = LL_Buffer_len(buf);
  uint8_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
  int cont = 0;
  int is_unsigned = buf->Qunsigned;
  for (unsigned i = 0; i < len; i++)
  {
    int32_t data = p[i];
    data = is_unsigned ? data : (int8_t)(data);
    LL_ATON_PRINTF("%" PRId32 " ", data);
    if (cont++ == 30)
    {
      cont = 0;
      LL_ATON_PUTS("");
    }
  }
  LL_ATON_PRINTF("\n");
}

static void __dump_buffer(int mode, const LL_Buffer_InfoTypeDef *buf)
{
  if (buf != NULL)
  {
    __dump_buffer_info(buf);
    switch (mode)
    {
    case MODE_RAW_INBITS:
      if (LL_Buffer_bits(buf) == 8)
      {
        __dump_buffer_raw_8bit(buf);
        break;
      }
      if (LL_Buffer_bits(buf) == 16)
      {
        __dump_buffer_raw_16bit(buf);
        break;
      }
      if (LL_Buffer_bits(buf) == 32)
      {
        __dump_buffer_raw_32bit(buf);
        break;
      }
      // intentional fall-thru
    case MODE_RAW_INFLOAT:
      if (buf->type == DataType_FLOAT)
      {
        for (float *lfd = (float *)LL_Buffer_addr_start(buf); lfd < (float *)LL_Buffer_addr_end(buf); lfd++)
        {
          LL_ATON_PRINTF("%f\n", *lfd);
        }

#if (ATON_PLAT_HAS_FFLUSH)
        LL_ATON_FFLUSH(stdout);
#endif
        return;
      }
      if (LL_Buffer_bits(buf) == 8)
        __dump_buffer_raw_Qmn_8bit(buf);
      if (LL_Buffer_bits(buf) == 16)
        __dump_buffer_raw_Qmn_16bit(buf);
      if (LL_Buffer_bits(buf) == 32)
        __dump_buffer_raw_Qmn_32bit(buf);
      break;
    case MODE_ONNX_INFLOAT:
      switch (buf->type)
      {
      case DataType_FLOAT:
        __dump_buffer_float(buf);
        break;
      default:
        if (LL_Buffer_bits(buf) == 8)
          __dump_buffer_Qmn_8bit(buf);
        if (LL_Buffer_bits(buf) == 16)
          __dump_buffer_Qmn_16bit(buf);
        if (LL_Buffer_bits(buf) == 32)
          __dump_buffer_Qmn_32bit(buf);
        break;
      }
      break;
    default:
      LL_ATON_PRINTF("Buffer Dump ignored: unrecognized dump mode\n");
      break;
    }
  }
}

static void __dump_epoch_buffers(const LL_Buffer_InfoTypeDef *bufs, int mode, int epoch)
{
  while (bufs != NULL && bufs[0].name != NULL)
  {
    if (epoch == -1 || bufs[0].epoch == epoch)
    {
      __dump_buffer(mode, bufs);
    }
    bufs++;
  }
}
#endif // NDEBUG

void *get_buffer(const char *bufname, int in, unsigned *_len, unsigned *_bits, const NN_Interface_TypeDef *nn_interface)
{
  const LL_Buffer_InfoTypeDef *buf = __get_buffer(bufname, in, nn_interface);
  if (buf != NULL)
  {
    uint8_t *p = LL_ATON_physical_to_virtual((uintptr_t)LL_Buffer_addr_start(buf));
    *_len = LL_Buffer_len(buf);
    *_bits = LL_Buffer_bits(buf);
    return (void *)p;
  }
  return NULL;
}

int get_buffer_info(const char *bufname, int in, LL_Buffer_InfoTypeDef *ret, const NN_Interface_TypeDef *nn_interface)
{
  const LL_Buffer_InfoTypeDef *buf = __get_buffer(bufname, in, nn_interface);
  if (buf != NULL)
  {
    *ret = *buf;
    ret->addr_base.i = (uintptr_t)LL_ATON_physical_to_virtual((uintptr_t)buf->addr_base.i);
    ret->offset_start = buf->offset_start;
    ret->offset_end = buf->offset_end;
    return 1;
  }
  return 0;
}

void set_all_buffers(int in, unsigned val, const NN_Interface_TypeDef *nn_interface)
{
  if ((in & BUFF_IN) != 0)
    __set_all_buffers(nn_interface->input_buffers_info(), val);
  if ((in & BUFF_OUT) != 0)
    __set_all_buffers(nn_interface->output_buffers_info(), val);
  if ((in & BUFF_INT) != 0)
    __set_all_buffers(nn_interface->internal_buffers_info(), val);
}

void dump_buffer(int mode, const char *bufname, int in, const NN_Interface_TypeDef *nn_interface)
{
#ifndef NDEBUG
  __dump_buffer(mode, __get_buffer(bufname, in, nn_interface));
#endif // NDEBUG
}

void dump_all_buffers(int mode, int in, const NN_Interface_TypeDef *nn_interface)
{
#ifndef NDEBUG
  if ((in & BUFF_IN) != 0)
    __dump_epoch_buffers(nn_interface->input_buffers_info(), mode, -1);
  if ((in & BUFF_OUT) != 0)
    __dump_epoch_buffers(nn_interface->output_buffers_info(), mode, -1);
  if ((in & BUFF_INT) != 0)
    __dump_epoch_buffers(nn_interface->internal_buffers_info(), mode, -1);
#endif // NDEBUG
}

void dump_epoch_buffers(int mode, int epoch, const NN_Interface_TypeDef *nn_interface)
{
#ifndef NDEBUG
  __dump_epoch_buffers(nn_interface->internal_buffers_info(), mode, epoch);
#endif // NDEBUG
}

/** @brief Return the total length of network parameters
 */
unsigned int get_params_len(const NN_Interface_TypeDef *nn_interface)
{
  LL_Buffer_InfoTypeDef *bufdesc = (LL_Buffer_InfoTypeDef *)nn_interface->input_buffers_info();
  unsigned int plen = 0;

  while (bufdesc->name)
  {
    if (bufdesc->is_param)
      plen += LL_Buffer_len(bufdesc);
    bufdesc++;
  }

  return plen;
}

#endif
