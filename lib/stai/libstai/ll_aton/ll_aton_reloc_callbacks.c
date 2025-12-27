/**
 ******************************************************************************
 * @file    ll_aton_reloc_callbacks.c
 * @author  GPM/AIS Team
 * @brief   Relocatable network support
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024,2025 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(LL_ATON_RT_RELOC)

#if defined(BUILD_AI_NETWORK_RELOC)

#include "ll_aton_caches_interface.h"

#undef BUILD_AI_NETWORK_RELOC
#include "ll_aton_reloc_network.h"

#include "ll_aton_lib.h"
#include "ll_aton_util.h"

extern struct ai_reloc_rt_ctx _network_rt_ctx;

void __assert_func(const char *filename, int line, const char *assert_func, const char *expr)
{
  volatile uint32_t _saved_r9;
  const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->assert_func)
  {
    __asm volatile("mov %0, r9\n\t" : "=r"(_saved_r9));
    cbs->assert_func(filename, line, assert_func, expr);
    __asm volatile("mov r9, %0\n\t" ::"r"(_saved_r9));
  }
  while (1)
    ;
}

void __ll_lib_error(int err_code, int line, const char *func)
{
  volatile uint32_t _saved_r9;
  const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->assert_func)
  {
    __asm volatile("mov %0, r9\n\t" : "=r"(_saved_r9));
    cbs->ll_lib_error(err_code, line, func);
    __asm volatile("mov r9, %0\n\t" ::"r"(_saved_r9));
  }
}

void LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->ll_aton_cache_mcu_clean_range)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    cbs->ll_aton_cache_mcu_clean_range(virtual_addr, size);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
}

void LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->ll_aton_cache_mcu_invalidate_range)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    cbs->ll_aton_cache_mcu_invalidate_range(virtual_addr, size);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
}

void LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->ll_aton_cache_mcu_clean_invalidate_range)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    cbs->ll_aton_cache_mcu_clean_invalidate_range(virtual_addr, size);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
}

void LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->ll_aton_cache_npu_clean_range)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    cbs->ll_aton_cache_npu_clean_range(virtual_addr, size);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
}

void LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  if (cbs && cbs->ll_aton_cache_npu_clean_invalidate_range)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    cbs->ll_aton_cache_npu_clean_invalidate_range(virtual_addr, size);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
}

/*
 * LL_ATON_LIB_ functions (see ll_lib_aton.h)
 */

int LL_ATON_LIB_Concat(const LL_Buffer_InfoTypeDef *inputs, unsigned int ninputs, const LL_Buffer_InfoTypeDef *output,
                       unsigned int axis, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_concat)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_concat(inputs, ninputs, output, axis, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_ImageToRow(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                               const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                               unsigned stride_h, unsigned stride_w, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_imagetorow)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_imagetorow(inputs, ninputs, output, blocksize_h, blocksize_w, stride_h, stride_w, dma_in,
                                          dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_SpaceToDepth(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                                 int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_spacetodepth)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_spacetodepth(inputs, ninputs, output, blocksize_h, blocksize_w, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_RowToImage(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                               const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                               unsigned stride_h, unsigned stride_w, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_rowtoimage)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_rowtoimage(inputs, ninputs, output, blocksize_h, blocksize_w, stride_h, stride_w, dma_in,
                                          dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_DepthToSpace(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                                 int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_depthtospace)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_depthtospace(inputs, ninputs, output, blocksize_h, blocksize_w, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Transpose(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                              const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                              const uint8_t *target_pos, const uint8_t *perm_to_use, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_transpose)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_transpose(input, input_axes_offsets, output, output_axes_offsets, target_pos,
                                         perm_to_use, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_Cast(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output, int dma_in,
                     int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_cast)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_cast(input, output, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_Softmax(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                        unsigned int axis, int legacy)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_softmax)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_softmax(input, output, axis, legacy);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Outputs_Flat_Copy(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *outputs,
                                      unsigned int nr_of_outputs, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_outputs_flat_copy)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_outputs_flat_copy(input, outputs, nr_of_outputs, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Outputs_Slice_SplitLike(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *output,
                                            int32_t tot_out_size, int32_t width_in_bytes, int32_t fheight,
                                            int32_t line_offset, int8_t n_bits, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_outputs_slice_splitlike)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_outputs_slice_splitlike(input, output, tot_out_size, width_in_bytes, fheight,
                                                       line_offset, n_bits, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *outputs,
                                               unsigned int nr_of_outputs, unsigned int leading_dims, int dma_in,
                                               int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_outputs_channel_split_aton)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_outputs_channel_split_aton(input, outputs, nr_of_outputs, leading_dims, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(const LL_Buffer_InfoTypeDef *input,
                                                  const LL_Buffer_InfoTypeDef *outputs, unsigned int nr_of_outputs,
                                                  int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_outputs_channel_split_batched)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_outputs_channel_split_batched(input, outputs, nr_of_outputs, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Pad_Memset(void *output, int32_t constant_value, size_t c, __ll_pad_sw_params_t *common_params,
                               bool deep_copy)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_pad_memset)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_pad_memset(output, constant_value, constant_value, common_params, deep_copy);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Pad_Filling(__ll_pad_sw_params_t *init_common_params)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_pad_filling)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_pad_filling(init_common_params);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_Async_Memcpy(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                             unsigned char *output_start, int dma_in, int dma_out)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_async_memcpy)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_async_memcpy(input_start, input_end, input_limit, output_start, dma_in, dma_out);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

int LL_ATON_LIB_DMA_Pad_4Loop_Filling(__ll_pad_sw_params_t *common_params)
{
  register uint32_t _saved_r9;
  register const struct ll_aton_reloc_callback *cbs = _network_rt_ctx.cbs;
  int res = -1;
  if (cbs && cbs->ll_aton_lib_dma_pad_4loop_filling)
  {
    __asm volatile("MOV %0, R9\n\t" : "=r"(_saved_r9));
    res = cbs->ll_aton_lib_dma_pad_4loop_filling(common_params);
    __asm volatile("MOV R9, %0\n\t" ::"r"(_saved_r9));
  };
  return res;
}

#else

#error "This file should be only used to build a relocatable model (BUILD_AI_NETWORK_RELOC)"

#endif

#endif /* LL_ATON_RT_RELOC */
