/**
 ******************************************************************************
 * @file    ll_aton_profiler.c
 * @author  SRA Artificial Intelligence & Embedded Architectures
 * @brief   ATON LL library for basic kernels making use of HW blocks driver.
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

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ll_aton_util.h" // Leave blank line after the include

#include "ll_aton_profiler.h"
#include "ll_aton_runtime.h"

#if _LL_LIB_DEBUG
#include <stdio.h>
#endif

int LL_ATON_LIB_ConvInteger(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                            const LL_LIB_TensorInfo_TypeDef *output, int pad_top, int pad_left, int pad_bottom,
                            int pad_right, int stride_h, int stride_w, int dilation_h, int dilation_w, int pad_value,
                            int groups, char *conv_name)
{
  int in_elements = LL_LIB_TENSOR_ELEMENTS(&inputs[0]);
  int kern_elements = LL_LIB_TENSOR_ELEMENTS(&inputs[1]);
  // int bias_elements = ninputs > 2 ? LL_LIB_TENSOR_ELEMENTS(&inputs[2]) : 0;
  int out_elements = LL_LIB_TENSOR_ELEMENTS(output);
  int el_size_0 = inputs[0].nbits / 8; // inputs[0].type == DataType_INT8 || inputs[0].type == DataType_UINT8 ? 1 : -1;
  int el_size_1 = inputs[1].nbits / 8; // inputs[0].type == DataType_INT8 || inputs[0].type == DataType_UINT8 ? 1 : -1;
  int el_out_size = output[0].nbits / 8; // output[0].type == DataType_INT32 ? 4 : -1;
  int in_byte_size = (in_elements * el_size_0 * 8) >> 3;
  int out_byte_size = (out_elements * el_out_size * 8) >> 3;
  int kern_byte_size = (kern_elements * el_size_1 * 8) >> 3;

  // if (axis != 1) // for now we only support axis = 1 FIXME !!!
  //   __LL_LIB_ERROR(_ERR_AXIS, LL_ATON_INVALID_PARAM);

  // if (in_elements != out_elements)
  //   __LL_LIB_ERROR(_ERR_BUFFER, LL_ATON_INVALID_PARAM);

  if ((el_size_0 != 1 && el_size_0 != 2) || (inputs[0].type != DataType_INT8 && inputs[0].type != DataType_UINT8 &&
                                             inputs[0].type != DataType_INT16 && inputs[0].type != DataType_UINT16))
    __LL_LIB_ERROR(_ERR_DATATYPE, LL_ATON_INVALID_PARAM);

  if ((el_size_1 != 1 && el_size_1 != 2) || (inputs[0].type != DataType_INT8 && inputs[0].type != DataType_UINT8 &&
                                             inputs[0].type != DataType_INT16 && inputs[0].type != DataType_UINT16))
    __LL_LIB_ERROR(_ERR_DATATYPE, LL_ATON_INVALID_PARAM);

  if (el_out_size != 4 || (output->type != DataType_INT32 && output->type != DataType_FXP))
    __LL_LIB_ERROR(_ERR_DATATYPE, LL_ATON_INVALID_PARAM);

  if (in_byte_size > LL_Buffer_len(inputs + 0))
    __LL_LIB_ERROR(_ERR_BUFFER_IN, LL_ATON_INVALID_PARAM);

  if (out_byte_size > LL_Buffer_len(output + 0))
    __LL_LIB_ERROR(_ERR_BUFFER_OUT, LL_ATON_INVALID_PARAM);

  if (kern_byte_size > LL_Buffer_len(inputs + 1))
    __LL_LIB_ERROR(_ERR_BUFFER_IN, LL_ATON_INVALID_PARAM);

  if (inputs[0].ndims < 4 || inputs[1].ndims < 4)
    __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);

  if (output->ndims < 4)
    __LL_LIB_ERROR(_ERR_SHAPE_OUT, LL_ATON_INVALID_PARAM);

  if (inputs[0].per_channel)
    __LL_LIB_ERROR(_ERR_DATATYPE, LL_ATON_INVALID_PARAM);

  // if (inputs[0].type == DataType_INT8)

  const LL_LIB_TensorInfo_TypeDef *feat = &inputs[0];
  const LL_LIB_TensorInfo_TypeDef *kern = &inputs[1];
  // const LL_LIB_TensorInfo_TypeDef *ref = ninputs > 2 ? &inputs[2] : NULL;
  const LL_LIB_TensorInfo_TypeDef *out = &output[0];
  int in_ndims = feat->ndims;
  int k_ndims = kern->ndims;

  // int in_batch = feat->batch;

  int N = feat->shape[(in_ndims - 4) + TDIM_NKERNELS];
  int C = feat->shape[(in_ndims - 4) + TDIM_NCHANNELS];
  int H = feat->shape[(in_ndims - 4) + TDIM_FHEIGHT];
  int W = feat->shape[(in_ndims - 4) + TDIM_FWIDTH];
  int B = feat->batch;

  int K = kern->shape[(k_ndims - 4) + TDIM_NKERNELS];
  // int KC = kern->shape[(in_ndims - 4) + TDIM_NCHANNELS];
  int R = kern->shape[(k_ndims - 4) + TDIM_FHEIGHT];
  int S = kern->shape[(k_ndims - 4) + TDIM_FWIDTH];
  // int KB = kern->batch;

  if (groups > 1) // not yet supported
    return 0;
#if 0
  if (KB != B)
  {
    LL_ATON_PROFILER_PRINTF("ERROR k batch != channels bacth %d %d\n", KB, B);
    __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);
  }
#endif

  int NB = C / B; // num channels batches
  int kcount = 0;
#if 0
  int pad_top = 2;
  int pad_bottom = 2;
  int pad_left = 2;
  int pad_right = 2;
  int stride_h = 1;
  int stride_w = 1;
  int8_t pad_value = -128;
#endif

  int8_t *in_data = (int8_t *)LL_Buffer_addr_start(feat);   // feat->addr_start.p;
  int32_t *out_data = (int32_t *)LL_Buffer_addr_start(out); // out->addr_start.p;
  int8_t *w_data = (int8_t *)LL_Buffer_addr_start(kern);    // kern->addr_start.p;
  int in_signed = feat->Qunsigned ? 0 : 1;
  int w_signed = kern->Qunsigned ? 0 : 1;

  uint8_t *in_data_u = (uint8_t *)LL_Buffer_addr_start(feat); // feat->addr_start.p;
  uint8_t *w_data_u = (uint8_t *)LL_Buffer_addr_start(kern);  // kern->addr_start.p;

  int out_H = (H + pad_top + pad_bottom - dilation_h * (R - 1) - 1) / stride_h + 1;
  int out_W = (W + pad_left + pad_right - dilation_w * (S - 1) - 1) / stride_w + 1;
  LL_ATON_ASSERT(out_H == out->shape[(out->ndims - 4) + TDIM_FHEIGHT]);
  LL_ATON_ASSERT(out_W == out->shape[(out->ndims - 4) + TDIM_FWIDTH]);

  int32_t maxmax = 0;
  for (int n = 0; n < N; ++n) // input batch
  {
    for (int k = 0; k < K; ++k) // num kernels per group
    {
      int32_t maxmax_k = 0;
      for (int oh = 0; oh < out_H; ++oh) // out height
      {
        for (int ow = 0; ow < out_W; ++ow) // out width
        {
          int32_t sum = 0;
          int32_t max = 0;
          for (int b = 0; b < NB; ++b) // num batches
          {
            for (int c = 0; c < B; ++c) // num channels in a batch
            {
              for (int r = 0; r < R; ++r) // kernel height
              {
                for (int s = 0; s < S; ++s) // kernel width
                {
                  int ih = oh * stride_h - pad_top + r * dilation_h;
                  int iw = ow * stride_w - pad_left + s * dilation_w;
                  int32_t input_value;
                  if (ih >= 0 && ih < H && iw >= 0 && iw < W)
                  {
                    int input_idx = ((n * C + c) * H + ih) * W + iw; // N[C/B]HWB
                    LL_ATON_ASSERT(LL_Buffer_addr_end(feat) /*feat->addr_end.p*/ >
                                   (unsigned char *)(in_data + input_idx));
                    if (el_size_0 == 1)
                      input_value = in_signed ? in_data[input_idx] : in_data_u[input_idx];
                    else
                      input_value = in_signed ? ((int16_t *)in_data)[input_idx] : ((uint16_t *)in_data)[input_idx];
                  }
                  else
                  {
                    input_value = pad_value;
                  }
                  // LL_ATON_PROFILER_PRINTF("%d %d %d=%d\n", c, r, s, input_value);
                  int weight_idx = ((k * C + c) * R + r) * S + s;
                  LL_ATON_ASSERT(LL_Buffer_addr_end(kern) /*kern->addr_end.p*/ >
                                 (unsigned char *)(w_data + weight_idx));
                  int32_t w_value;
                  if (el_size_0 == 1)
                    w_value = w_signed ? w_data[weight_idx] : w_data_u[weight_idx];
                  else
                    w_value = w_signed ? ((int16_t *)w_data)[weight_idx] : ((uint16_t *)w_data)[weight_idx];
                  sum += input_value * w_value;
                  max = abs(sum) > abs(max) ? abs(sum) : abs(max);
                  kcount++;
                  // LL_ATON_PROFILER_PRINTF("%d %d %d=%d %d %d\n", c, r, s, input_value, w_data[weight_idx], sum);
                }
              }
            }
          }
          int output_idx = ((n * K + k) * out_H + oh) * out_W + ow;
          LL_ATON_ASSERT(LL_Buffer_addr_end(out) /*out->addr_end.p*/ > (unsigned char *)(out_data + output_idx));
          out_data[output_idx] = sum;
          // LL_ATON_PROFILER_PRINTF("oidx=%d = %d max=%d\n", output_idx, sum, max);
          maxmax = max > maxmax ? max : maxmax;
          maxmax_k = max > maxmax_k ? max : maxmax_k;
        }
      }
      LL_ATON_PROFILER_PRINTF("%s k=%d %d scale=%g\n", conv_name, k, maxmax_k, (double)kern->scale[k]);
    }
  }
  LL_ATON_ASSERT(kcount == K * S * R * C * N * out_H * out_W);

#define DIFFTH 1
#if 0
  if (ref != NULL)
  {
    int RN = ref->shape[(in_ndims - 4) + TDIM_NKERNELS];
    int RC = ref->shape[(in_ndims - 4) + TDIM_NCHANNELS];
    int RH = ref->shape[(in_ndims - 4) + TDIM_FHEIGHT];
    int RW = ref->shape[(in_ndims - 4) + TDIM_FWIDTH];
    int RB = ref->batch;
    int RNB = RC / RB;                                        // num channels batches
    int16_t *ref_data = (int16_t *)LL_Buffer_addr_start(ref); // ref->addr_start.p;
    int32_t max_diff = 0;
    int max_n = 0;
    int max_c = 0;
    int max_h = 0;
    int max_w = 0;

    if (RN == N && out_H == RH && out_W == RW && K == RC && ref->nbits == 16 && ref->Qunsigned == 0)
    {
      for (int n = 0; n < RN; ++n) // input batch
      {
        LL_ATON_PROFILER_PRINTF("n=%d [[[\n", n);
        for (int b = 0; b < RNB; ++b) // num batches
        {
          for (int c = 0; c < RB; ++c) // num channels in a batch
          {
            LL_ATON_PROFILER_PRINTF("c=%d [[\n", b * RB + c);
            for (int oh = 0; oh < RH; ++oh) // out height
            {
              LL_ATON_PROFILER_PRINTF("h=%d [\n", oh);
              for (int ow = 0; ow < RW; ++ow) // out width
              {
                int ref_idx = n * RH * RW * RC + (b * RB * RH * RW) + (oh * RW + ow) * RB + c; // HWC
                int out_idx = n * RH * RW * RC + (oh * RW + ow) * RC + b * RB + c;             // HWC
                LL_ATON_ASSERT(LL_Buffer_addr_end(ref) /*ref->addr_end.p */ > (unsigned char *)(ref_data + ref_idx));
                int32_t ref_val = ref_data[ref_idx];
                ref_val = ref->Qn >= 0 ? ref_val >> ref->Qn : ref_val << -ref->Qn;
                LL_ATON_ASSERT(LL_Buffer_addr_end(out) /*out->addr_end.p*/ > (unsigned char *)(out_data + out_idx));
                int32_t out_val = out_data[out_idx];
                int diff = abs(ref_val - out_val);
                if (diff > DIFFTH)
                  LL_ATON_PROFILER_PRINTF("(%d,%d,%d,%d)=(%d %d) ", n, b * RB + c, oh, ow, out_val, ref_val);
                if (diff > max_diff)
                {
                  max_diff = diff;
                  max_n = n;
                  max_c = b * RB + c;
                  max_h = oh;
                  max_w = ow;
                }
              }
              LL_ATON_PROFILER_PRINTF("\n]\n");
            }
            LL_ATON_PROFILER_PRINTF("\n]]\n");
          }
        }
        LL_ATON_PROFILER_PRINTF("\n]]]\n");
      }
    }
    else
      LL_ATON_PROFILER_PRINTF("max_diff=%d (%d,%d,%d,%d)\n", max_diff, max_n, max_c, max_h, max_w);
  }
#endif

  // LL_ATON_PROFILER_PRINTF("tot max=%d %f", maxmax, log((float)maxmax) / log(2.0));
#if 0  
  LL_ATON_PROFILER_PRINTF("%s %d\n", conv_name, maxmax); // log((float)maxmax) / log(2.0));
#else
  FILE *f = LL_ATON_FOPEN("atonn.prof", "a+");
  if (f)
  {
    LL_ATON_PROFILER_FPRINTF(f, "%s %d\n", conv_name, maxmax); // log((float)maxmax) / log(2.0));
    LL_ATON_FCLOSE(f);
  }
#endif

  if (inputs[0].type == DataType_UINT8)
  {
  }

  return LL_ATON_INVALID_PARAM;
}
