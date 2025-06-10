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

#include <assert.h>
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
                            const LL_LIB_TensorInfo_TypeDef *output)
{
  int in_elements = LL_LIB_TENSOR_ELEMENTS(&inputs[0]);
  int kern_elements = LL_LIB_TENSOR_ELEMENTS(&inputs[1]);
  // int bias_elements = ninputs > 2 ? LL_LIB_TENSOR_ELEMENTS(&inputs[2]) : 0;
  int out_elements = LL_LIB_TENSOR_ELEMENTS(output);
  int el_size = inputs[0].type == DataType_INT8 || inputs[0].type == DataType_UINT8 ? 1 : -1;
  int el_out_size = output[0].type == DataType_INT32 ? 4 : -1;
  int in_byte_size = (in_elements * el_size * 8) >> 3;
  int out_byte_size = (out_elements * el_out_size * 8) >> 3;
  int kern_byte_size = (kern_elements * el_size * 8) >> 3;

  // if (axis != 1) // for now we only support axis = 1 FIXME !!!
  //   __LL_LIB_ERROR(_ERR_AXIS, LL_ATON_INVALID_PARAM);

  // if (in_elements != out_elements)
  //   __LL_LIB_ERROR(_ERR_BUFFER, LL_ATON_INVALID_PARAM);

  if ((el_out_size != 4 || output->type != DataType_INT32))
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
  const LL_LIB_TensorInfo_TypeDef *out = &output[0];
  int in_ndims = feat->ndims;
  int k_ndims = kern->ndims;

  // int in_batch = feat->batch;

  int N = feat->shape[(in_ndims - 4) + TDIM_NKERNELS];
  int C = feat->shape[(in_ndims - 4) + TDIM_NCHANNELS];
  int H = feat->shape[(in_ndims - 4) + TDIM_FHEIGHT];
  int W = feat->shape[(in_ndims - 4) + TDIM_FWIDTH];

  int K = kern->shape[(k_ndims - 4) + TDIM_NKERNELS];
  int R = kern->shape[(k_ndims - 4) + TDIM_FHEIGHT];
  int S = kern->shape[(k_ndims - 4) + TDIM_FWIDTH];

  int pad_top = 2;
  int pad_bottom = 2;
  int pad_left = 2;
  int pad_right = 2;
  int stride_h = 1;
  int stride_w = 1;
  int8_t pad_value = -128;

  int8_t *in_data = (int8_t *)LL_Buffer_addr_start(feat);
  int32_t *out_data = (int32_t *)LL_Buffer_addr_start(out);
  int8_t *w_data = (int8_t *)LL_Buffer_addr_start(kern);

  int out_H = (H + pad_top + pad_bottom - R) / stride_h + 1;
  int out_W = (W + pad_left + pad_right - S) / stride_w + 1;

  int32_t maxmax = 0;
  for (int n = 0; n < N; ++n)
  {
    for (int k = 0; k < K; ++k)
    {
      for (int oh = 0; oh < out_H; ++oh)
      {
        for (int ow = 0; ow < out_W; ++ow)
        {
          int32_t sum = 0;
          int32_t max = 0;
          for (int r = 0; r < R; ++r)
          {
            for (int s = 0; s < S; ++s)
            {
              for (int c = 0; c < C; ++c)
              {
                int ih = oh * stride_h - pad_top + r;
                int iw = ow * stride_w - pad_left + s;
                int32_t input_value;
                if (ih >= 0 && ih < H && iw >= 0 && iw < W)
                {
                  int input_idx = ((n * H + ih) * W + iw) * C + c; // HWC
                  // int input_idx = ((n * C + c) * H + ih) * W + iw;A // CHW
                  input_value = in_data[input_idx];
                }
                else
                {
                  input_value = pad_value;
                }
                // LL_ATON_PROFILER_PRINTF("%d %d %d=%d\n", c, r, s, input_value);
                int weight_idx = ((k * R + r) * S + s) * C + c; // HWC
                // int weight_idx = ((k * C + c) * R + r) * S + s;
                sum += input_value * w_data[weight_idx];
                max = sum > max ? sum : max;
                // LL_ATON_PROFILER_PRINTF("%d %d %d=%d %d %d\n", c, r, s, input_value, w_data[weight_idx], sum);
              }
            }
          }
          int output_idx = ((n * out_H + oh) * out_W + ow) * K + k; // HWC
          // int output_idx = ((n * K + k) * out_H + oh) * out_W + ow;
          out_data[output_idx] = sum;
          LL_ATON_PROFILER_PRINTF("oidx=%d = %d max=%d\n", output_idx, sum, max);
          maxmax = max > maxmax ? max : maxmax;
        }
      }
    }
  }
  LL_ATON_PROFILER_PRINTF("tot max=%d %f", maxmax, log((float)maxmax) / log(2.0));

  if (inputs[0].type == DataType_UINT8)
  {
  }

  return LL_ATON_INVALID_PARAM;
}
