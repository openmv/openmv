/**
 ******************************************************************************
 * @file    ll_aton_lib.c
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

#include "ll_aton_caches_interface.h"
#include "ll_aton_lib.h"
#include "ll_aton_runtime.h"

#if _LL_LIB_DEBUG
#include <stdio.h>

// if set it will perform CHECKDISTANCE with COSINE_SIMILARIY and not with MAXDISTANCE
#define USE_COSINE_SIMILARITY
// CHECKDISTANCE mode thresholds
#define MAXDISTANCE_TH       3
#define COSINE_SIMILARITY_TH 0.9

int LL_LIB_TENSOR_ELEMENTS(const LL_LIB_TensorInfo_TypeDef *t)
{
  int cont = 1;
  for (int i = 0; i < t->ndims; i++)
    cont *= t->shape[i];
  return cont;
}

void __ll_lib_error(int err_code, int line, const char *func) // for library internal use only
{
#ifndef NDEBUG
  char *errs;
  switch (err_code)
  {
  case _ERR_NINPUTS:
    errs = "Wrong number of inputs";
    break;
  case _ERR_NOUTPUTS:
    errs = "Wrong number of outputs";
    break;
  case _ERR_AXIS:
    errs = "Axis mismatch or not supported";
    break;
  case _ERR_FRACTIONAL:
    errs = "Fractional bits not supported";
    break;
  case _ERR_DATATYPE:
    errs = "Datatype not supported";
    break;
  case _ERR_NBITS:
    errs = "Number of bits unsupported or in/out mismatch";
    break;
  case _ERR_NBITS_IN:
    errs = "Number of bits unsupported or in(s) mismatch";
    break;
  case _ERR_NBITS_OUT:
    errs = "Number of bits unsupported or out(s) mismatch";
    break;
  case _ERR_SHAPE:
    errs = "Shape unsupported or in/out mismatch";
    break;
  case _ERR_SHAPE_IN:
    errs = "Shape unsupported or in(s) mismatch";
    break;
  case _ERR_SHAPE_OUT:
    errs = "Shape unsupported or out(s) mismatch";
    break;
  case _ERR_BUFFER:
    errs = "Buffer sizes in/out mismatch";
    break;
  case _ERR_BUFFER_IN:
    errs = "Buffer sizes in(s) mismatch";
    break;
  case _ERR_BUFFER_OUT:
    errs = "Buffer sizes out(s) mismatch";
    break;
  case _ERR_RANK:
    errs = "Inconsistent or unexpected rank value(s)";
    break;
  case _ERR_MODE:
    errs = "Unknown or not supported modality";
    break;
  case _ERR_UNKNOWN:
  default:
    errs = "Unknown";
    break;
  }
  LL_ATON_PRINTF("%s line %d LL_LIB Error: %s\n", func, line, errs);
#endif // NDEBUG
}
#endif // _LL_LIB_DEBUG

/*** Heap for hybrid operator implementations ***/
static uint32_t __ll_lib_heap[__LL_LIB_HEAP_SIZE]; // REMEMBER: static variables are not suited for
                                                   // multithreaded etc. environments
/** Static constants **/
static LL_Switch_InitTypeDef switch_init[] = {{LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0),
                                               LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0),
                                               LL_Switch_Init_Context(0) = 1, LL_Switch_Init_Frames(0) = 0}};
static int dma_unit_id[] = {1, 0}; /* {<dma_out>, <dma_in>} */
static LL_ATON_EnableUnits_InitTypeDef dma_units[] = {{{STRENG, 1}}, {{STRENG, 0}}};
static const LL_Streng_TensorInitTypeDef _static_const_dma_in = {
    .dir = 0, .raw = 1, .frame_tot_cnt = 1, .nbits_in = 24, .nbits_out = 24};
static const LL_Streng_TensorInitTypeDef _static_const_dma_out = {
    .dir = 1, .raw = 1, .frame_tot_cnt = 1, .nbits_in = 24, .nbits_out = 24};

/** Helper function(s) **/
static inline __ll_lib_params_t *__ll_lib_get_params(void)
{
  return (__ll_lib_params_t *)__ll_lib_heap;
}

static inline void *__ll_lib_get_lower_heap(void)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  params++;
  return (void *)params;
}

static inline void __ll_lib_dump_strswitch(int dma_in, int dma_out)
{
#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("===\n");
  LL_ATON_PRINTF("dma_in: %d, dma_out: %d\n", dma_in, dma_out);
  LL_ATON_PRINTF("---\n");
  LL_ATON_PRINTF("dest: %d\n", ATONN_DSTPORT_ID(switch_init[0].dest));
  LL_ATON_PRINTF("---\n");
  LL_ATON_PRINTF("source0: %d\n", ATONN_SRCPORT_ID(switch_init[0].source0));
  LL_ATON_PRINTF("frames0: %d\n", switch_init[0].frames0);
  LL_ATON_PRINTF("context0: %d\n", switch_init[0].context0);
  LL_ATON_PRINTF("---\n");
  LL_ATON_PRINTF("source1: %d\n", ATONN_SRCPORT_ID(switch_init[0].source1));
  LL_ATON_PRINTF("frames1: %d\n", switch_init[0].frames1);
  LL_ATON_PRINTF("context1: %d\n", switch_init[0].context1);
  LL_ATON_PRINTF("===\n");

#if (ATON_PLAT_HAS_FFLUSH)
  LL_ATON_FFLUSH(stdout);
#endif
#endif // !DUMP_DEBUG_SW_OPS
}

static void __ll_lib_strswitch_set_dmas(int dma_in, int dma_out, LL_ATON_RT_EpochBlockItem_t *epoch_block_array)
{
  __ll_lib_dump_strswitch(dma_in, dma_out);

  __ll_lib_params_t *params = __ll_lib_get_params();

  switch_init[0].source0 = __atonn_getSrcPortID(STRSWITCH, 0, STRENG, dma_in, 0);
  switch_init[0].dest = __atonn_getDstPortID(STRSWITCH, 0, STRENG, dma_out, 0);

  AccelUnits dma_in_streng = {STRENG, dma_in};
  AccelUnits dma_out_streng = {STRENG, dma_out};

  dma_units[1].unit = dma_in_streng;
  dma_unit_id[1] = dma_in;
  dma_units[0].unit = dma_out_streng;
  dma_unit_id[0] = dma_out;

  uint32_t wait_mask = (0x1 << dma_out);
  params->g_wait_mask = wait_mask;
  epoch_block_array->wait_mask = wait_mask;

  __ll_lib_dump_strswitch(dma_in, dma_out);
}

static inline void __ll_lib_start_transfer(__ll_lib_params_t *params)
{
  LL_Streng_TensorInit(dma_unit_id[1], &params->g_dma_in, 1);
  LL_Streng_TensorInit(dma_unit_id[0], &params->g_dma_out, 1);
  LL_Switch_Init_NoReset(switch_init, 1);
  LL_ATON_EnableUnits_Init(dma_units, 2);
}

static inline void __ll_lib_stop_transfer(void)
{
  LL_Switch_Deinit(switch_init, 1);
  LL_ATON_DisableUnits_Init(dma_units, 2);
}

static inline uint32_t __ll_lib_set_wait_mask(LL_ATON_RT_EpochBlockItem_t *eb, uint32_t wait_mask)
{
  uint32_t previous_value;
  previous_value = eb->wait_mask;
  eb->wait_mask = wait_mask;

  __LL_ATON_RT_SetWaitMask(eb->wait_mask);

  return previous_value;
}

static inline void __ll_lib_prepare_inputs_epoch(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                                 const LL_Streng_TensorInitTypeDef *dma_in,
                                                 const LL_Streng_TensorInitTypeDef *dma_out, unsigned char *out_start,
                                                 int nbytes_or_line_size)
{
  /* get pointers to static structures */
  __ll_lib_params_t *params = __ll_lib_get_params();
  void *inputs_copy = __ll_lib_get_lower_heap();

  /* fill `inputs_copy` */
  // NOTE: must treat the failing of the beyond `LL_ATON_ASSERT` statement as error!
  // TODO: must be changed in a way that allows both a return from this function
  //       and to return control back to the user's main loop
  //       e.g. by using an internal flag/variable to signal the error,
  //       then performing a `LL_ATON_RT_RuntimeDeInit()`,
  //       and returning with a respective (new) return value (of type `LL_ATON_RT_RetValues_t`),
  //       reporting about the error, from the latest call to `LL_ATON_RT_RunEpochBlock()`
  LL_ATON_ASSERT(ninputs <= __LL_MAX_TENSORS);
  memcpy(inputs_copy, inputs, sizeof(LL_LIB_TensorInfo_TypeDef) * ninputs);

  params->g_tensors = inputs_copy;
  params->g_num_tensors = ninputs;

  params->g_dma_in = *dma_in;
  params->g_dma_out = *dma_out;

  params->g_dst_o_src = out_start;
  params->g_not_continuous = 0; // signals that destination is not written linearly

  params->g_size = nbytes_or_line_size;
  params->g_idx = 0;

  params->g_offset_limit = 0;
}

static inline void __ll_lib_prepare_outputs_epoch(const LL_LIB_TensorShape_TypeDef *outputs, unsigned int noutputs,
                                                  const LL_Streng_TensorInitTypeDef *dma_in,
                                                  const LL_Streng_TensorInitTypeDef *dma_out,
                                                  const LL_LIB_TensorShape_TypeDef *input)
{
  /* get pointers to static structures */
  __ll_lib_params_t *params = __ll_lib_get_params();
  void *outputs_copy = __ll_lib_get_lower_heap();

  /* fill `outputs_copy` */
  // NOTE: must treat the failing of the beyond `LL_ATON_ASSERT` statement as error!
  // TODO: must be changed in a way that allows both a return from this function
  //       and to return control back to the user's main loop
  //       e.g. by using an internal flag/variable to signal the error,
  //       then performing a `LL_ATON_RT_RuntimeDeInit()`,
  //       and returning with a respective (new) return value (of type `LL_ATON_RT_RetValues_t`),
  //       reporting about the error, from the latest call to `LL_ATON_RT_RunEpochBlock()`
  LL_ATON_ASSERT(noutputs <= __LL_MAX_TENSORS);
  memcpy(outputs_copy, outputs, sizeof(LL_LIB_TensorShape_TypeDef) * noutputs);

  params->g_tensors = outputs_copy;
  params->g_num_tensors = noutputs;

  params->g_dma_in = *dma_in;
  params->g_dma_out = *dma_out;

  params->g_dst_o_src = LL_Buffer_addr_start(input);
  params->g_dst_o_src = LL_Buffer_addr_start(input);
  params->g_not_continuous = 0; // signals that source is not read linearly

  params->g_size = -1;
  params->g_idx = 0;

  params->g_offset_limit = input->offset_limit;
}

/* `memcpy` generic epoch blocks */
static inline size_t __ll_lib_memcpy_prolog(void **dst, void **src, size_t n)
{
  uint8_t *_dst_orig = *dst;

  int prolog_len = (n % 3);
  int i;
  uint8_t **_dst = (uint8_t **)dst;
  uint8_t **_src = (uint8_t **)src;

  if (n < __LL_DMA_MIN_BUFF_LEN)
    prolog_len = n; // not worth it ...

  for (i = 0; i < prolog_len; i++)
  {
    **_dst = **_src;
    (*_dst)++;
    (*_src)++;
  }
  n -= prolog_len;

  if (prolog_len > 0)
  {
    /* *** MCU cache clean & invalidate operation (SW) *** */
    LL_ATON_Cache_MCU_Clean_Invalidate_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)_dst_orig), prolog_len);
  }

  return n;
}

static void __ll_lib_inputs_memcpy_start(const void *epoch_block, uint8_t *_src)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  uint8_t *_dst = (uint8_t *)params->g_dst_o_src;
  size_t n;

  if (params->g_size < 0)
  {
    n = LL_Buffer_len(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx);
  }
  else
  {
    n = params->g_size;
  }

  if (params->g_not_continuous == 0)
    n = __ll_lib_memcpy_prolog((void **)&_dst, (void **)&_src, n);

  if (n > 0)
  {
    params->g_dma_in.addr_base.p = _src;
    params->g_dma_in.offset_start = 0;
    params->g_dma_in.offset_end = n; // not used for batched output version g_not_continuous == 1
    params->g_dma_in.offset_limit = ((LL_LIB_TensorInfo_TypeDef *)params->g_tensors)[params->g_idx].offset_limit;

    params->g_dma_out.addr_base.p = _dst;
    params->g_dma_out.offset_start = 0;
    params->g_dma_out.offset_end = n; // not used for batched input version g_not_continuous == 1

    __ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, params->g_wait_mask);

    __ll_lib_start_transfer(params);
  }
  else
  {
    /* do not start any transfer and wait, just proceed to end function */
    __ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0);
  }
}

static void __ll_lib_outputs_memcpy_start(const void *epoch_block, uint8_t *_dst)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  uint8_t *_src = (uint8_t *)params->g_dst_o_src;
  size_t n;

  if (params->g_size < 0)
  {
    n = LL_Buffer_len(((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx);
  }
  else
  {
    n = params->g_size;
  }

  if (params->g_not_continuous == 0)
    n = __ll_lib_memcpy_prolog((void **)&_dst, (void **)&_src, n);

  if (n > 0)
  {
    params->g_dma_out.addr_base.p = _dst;
    params->g_dma_out.offset_start = 0;
    params->g_dma_out.offset_end = n;

    if (params->g_not_continuous == 0)
    {
      params->g_dma_in.addr_base.p = _src;
      params->g_dma_in.offset_start = 0;
      params->g_dma_in.offset_end = n;
    }
    params->g_dma_in.offset_limit = params->g_offset_limit;

    __ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, params->g_wait_mask);
    __ll_lib_start_transfer(params);
  }
  else
  {
    /* do not start any transfer and wait, just proceed to end function */
    __ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0);
  }
}

/** Epoch start/end functions and epoch block arrays **/

static void __LL_LIB_Concat_Case3_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT((params->special.concat_case3.outer_idx < params->g_num_tensors) &&
                 (params->g_idx < params->special.concat_case3.in_fheight)); // must be checked before

  __ll_lib_inputs_memcpy_start(epoch_block, (uint8_t *)params->special.concat_case3.in_curr);
}

static void __LL_LIB_Concat_Case3_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  params->g_idx++;

  if (params->g_idx < params->special.concat_case3.in_fheight)
  {
    params->g_dst_o_src += params->special.concat_case3.out_line_size;
    params->special.concat_case3.in_curr += params->g_size;

    /* loop back one epoch block */
    LL_ATON_RT_DecCurrEpochBlock(1);
  }
  else
  {
    params->special.concat_case3.outer_idx++;

    if (params->special.concat_case3.outer_idx < params->g_num_tensors)
    {
      int in_ndims = ((LL_LIB_TensorInfo_TypeDef *)params->g_tensors)[params->special.concat_case3.outer_idx].ndims;
      unsigned int pix_size = params->special.concat_case3.nbytes *
                              ((LL_LIB_TensorInfo_TypeDef *)params->g_tensors)[params->special.concat_case3.outer_idx]
                                  .shape[(in_ndims - 4) + TDIM_NCHANNELS];
      params->g_size =
          pix_size * ((LL_LIB_TensorInfo_TypeDef *)params->g_tensors)[params->special.concat_case3.outer_idx]
                         .shape[(in_ndims - 4) + TDIM_FWIDTH];
      params->g_dst_o_src += params->g_size;
      params->special.concat_case3.in_curr = LL_Buffer_addr_start(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) +
                                                                  params->special.concat_case3.outer_idx);
      params->g_idx = 0;

      /* loop back one epoch block */
      LL_ATON_RT_DecCurrEpochBlock(1);
    }
    else
    {
      /* proceed to next epoch block */
    }
  }
}

static void __LL_LIB_Inputs_Memcpy_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before

  uint8_t *src = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_inputs_memcpy_start(epoch_block, src);
}

static void __LL_LIB_Inputs_Memcpy_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  if (params->g_size < 0)
  {
    params->g_dst_o_src += LL_Buffer_len(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx);
  }
  else
  {
    params->g_dst_o_src += (params->g_size);
  }
  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    /* loop back one epoch block */
    LL_ATON_RT_DecCurrEpochBlock(1);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Inputs_Batched_Memcpy_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before
  params->g_not_continuous =
      1; // disables code in __ll_lib_inputs_memcpy_start that assumes a flat copy operation e.g. prolog test

  uint8_t *src = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_inputs_memcpy_start(epoch_block, src);
}

static void __LL_LIB_Inputs_Batched_Memcpy_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  LL_LIB_TensorInfo_TypeDef *in = ((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx;
  int in_ndims = in->ndims;
  int in_nchannels_old = in->shape[(in_ndims - 4) + TDIM_NCHANNELS];

  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    in = ((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx;

    int nbits = in->nbits;
    int nbytes = (nbits + 7) >> 3;
    int in_batch = in->batch;
    int in_fheight = in->shape[(in_ndims - 4) + TDIM_FHEIGHT];
    int in_fwidth = in->shape[(in_ndims - 4) + TDIM_FWIDTH];
    int in_nchannels = in->shape[(in_ndims - 4) + TDIM_NCHANNELS];

    params->g_dst_o_src += in_nchannels_old * nbytes;

    // LL_ATON_PRINTF("nbytes=%d fw=%d fh=%d inb=%d inc=%d\n", nbytes, in_fwidth, in_fheight, in_batch, in_nchannels);

    params->g_dma_out.batch_depth = (nbytes == 4) ? (2 * in_batch) : in_batch;  // this must be updated on all inputs
    params->g_dma_out.frame_offset = in_batch * nbytes;                         // this must be updated on all inputs
    params->g_dma_out.frame_loop_cnt = in_nchannels / in_batch;                 // this must be updated on all inputs
    params->g_dma_out.frame_tot_cnt = in_nchannels / in_batch;                  // this must be updated on all inputs
    params->g_dma_out.loop_offset = in_fheight * in_fwidth * in_batch * nbytes; // this must be updated on all inputs

    /* loop back one epoch block */
    LL_ATON_RT_DecCurrEpochBlock(1);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Outputs_Channel_Split_Aton_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before
  params->g_not_continuous =
      1; // disables code in __ll_lib_outputs_memcpy_start that assumes a flat copy operation e.g. prolog test

  uint8_t *dst = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_outputs_memcpy_start(epoch_block, dst);
}

static void __LL_LIB_Outputs_Channel_Split_Aton_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  LL_LIB_TensorShape_TypeDef *out = ((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx;
  int out_rank_old = out->ndims;
  int out_nchannels_old = out->shape[(out_rank_old - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    out = ((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx;

    uint32_t out_ndims = out->ndims;
    LL_ATON_ASSERT(out_ndims >= 3);
    uint32_t nbytes = LL_LIB_NBYTES(out->nbits);
    uint32_t out_fwidth = out->shape[(out_ndims - 4) + TDIM_ONNX_FWIDTH];
    uint32_t out_fheight = out->shape[(out_ndims - 4) + TDIM_ONNX_FHEIGHT];
    uint32_t out_nchannels = out->shape[(out_ndims - 4) + TDIM_ONNX_NCHANNELS];

    // program output DMA
    params->g_dma_out.addr_base.p = out->addr_base.p;
    params->g_dma_out.offset_start = out->offset_start;
    params->g_dma_out.offset_end = out->offset_end;

    // LL_ATON_PRINTF("\ndma_out: addr_start=%p, addr_end=%p\n", LL_Streng_addr_start(&(params->g_dma_out)),
    // LL_Streng_addr_end(&(params->g_dma_out));

    unsigned batch_depth = (nbytes == 4) ? (2 * out_nchannels) : out_nchannels;
    unsigned frame_size = out_fwidth * out_fheight * batch_depth;

    // program input DMA
    if (frame_size != 1)
    {
      params->g_dma_in.raw = 0;                                    // this must be updated on all outputs
      params->g_dma_in.offset_start += out_nchannels_old * nbytes; // this must be updated on all outputs
      params->g_dma_in.batch_depth = batch_depth;                  // this must be updated on all outputs

      // LL_ATON_PRINTF("dma_in: frame_size=%d, nbytes=%u, out_nchannels_old=%d, out_nchannels=%u, src=%p,
      // out_fwidth=%u, out_fheight=%u, batch_depth=%u, batch_offset=%u, line_offset=%u, frame_offset=%u,
      // frame_tot_cnt=%u\n",
      //      frame_size, nbytes, out_nchannels_old, out_nchannels, LL_Streng_addr_start(&(params->g_dma_in)),
      //      params->g_dma_in.fwidth, params->g_dma_in.fheight, params->g_dma_in.batch_depth,
      //      params->g_dma_in.batch_offset, params->g_dma_in.line_offset, params->g_dma_in.frame_offset,
      //      params->g_dma_in.frame_tot_cnt);
      // LL_ATON_FFLUSH(stdout);
    }
    else
    { // frame_size == 1
#if 1
      LL_ATON_ASSERT(0); // should never happen!!!
#else
      params->g_dma_in.raw = 1;                                    // this must be updated on all outputs
      params->g_dma_in.offset_start += out_nchannels_old * nbytes; // this must be updated on all outputs
      params->g_dma_in.offset_end +=
          params->g_dma_in.offset_start +
          nbytes; // this must be updated on all outputs FIXME Francesco incorrect calculation
#endif
    }
    /* loop back one epoch block */
    LL_ATON_RT_DecCurrEpochBlock(1);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Outputs_Channel_Split_Batched_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before
  params->g_not_continuous =
      1; // disables code in __ll_lib_outputs_memcpy_start that assumes a flat copy operation e.g. prolog test

  uint8_t *dst = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_outputs_memcpy_start(epoch_block, dst);
}

static void __LL_LIB_Outputs_Channel_Split_Batched_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  LL_LIB_TensorShape_TypeDef *out = ((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx;
  int out_rank_old = out->ndims;
  int out_nchannels_old = out->shape[(out_rank_old - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    out = ((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx;

    uint32_t out_ndims = out->ndims;
    LL_ATON_ASSERT(out_ndims >= 3);
    uint32_t nbytes = LL_LIB_NBYTES(out->nbits);
    uint32_t out_fwidth = out->shape[(out_ndims - 4) + TDIM_ONNX_FWIDTH];
    uint32_t out_fheight = out->shape[(out_ndims - 4) + TDIM_ONNX_FHEIGHT];
    uint32_t out_nchannels = out->shape[(out_ndims - 4) + TDIM_ONNX_NCHANNELS];
    uint16_t out_batch = out->batch;
    LL_ATON_ASSERT((out_nchannels % out_batch) == 0);

    // program output DMA
    params->g_dma_out.addr_base.p = out->addr_base.p;
    params->g_dma_out.offset_start = out->offset_start;
    params->g_dma_out.offset_end = out->offset_end;

    // LL_ATON_PRINTF("\ndma_out: addr_start=%p, addr_end=%p\n", LL_Streng_addr_start(&(params->g_dma_out)),
    // LL_Streng_addr_end(&(params->g_dma_out)));

    unsigned batch_depth = (nbytes == 4) ? (2 * out_batch) : out_batch;
    unsigned frame_size = out_fwidth * out_fheight * batch_depth;

    // program input DMA
    if (frame_size != 1)
    {
      params->g_dma_in.raw = 0;                                    // this must be updated on all outputs
      params->g_dma_in.offset_start += out_nchannels_old * nbytes; // this must be updated on all outputs
      params->g_dma_in.batch_depth = batch_depth;                  // this must be updated on all outputs
      params->g_dma_in.frame_tot_cnt = out_nchannels / out_batch;  // this must be updated on all outputs
      params->g_dma_in.loop_offset = out_batch * nbytes;           // this must be updated on all outputs

      // LL_ATON_PRINTF("dma_in: frame_size=%d, nbytes=%u, out_nchannels_old=%d, out_nchannels=%u, src=%p,
      // out_fwidth=%u, out_fheight=%u, batch_depth=%u, batch_offset=%u, line_offset=%u, frame_offset=%u,
      // frame_tot_cnt=%u\n",
      //      frame_size, nbytes, out_nchannels_old, out_nchannels, LL_Streng_addr_start(&(params->g_dma_in)),
      //      params->g_dma_in.fwidth, params->g_dma_in.fheight, params->g_dma_in.batch_depth,
      //      params->g_dma_in.batch_offset, params->g_dma_in.line_offset, params->g_dma_in.frame_offset,
      //      params->g_dma_in.frame_tot_cnt);
      // LL_ATON_FFLUSH(stdout);
    }
    else
    { // frame_size == 1
#if 1
      LL_ATON_ASSERT(0); // should never happen!!!
#else
      // TODO (as not yet adapted for this case)
      params->g_dma_in.raw = 1;                                    // this must be updated on all outputs
      params->g_dma_in.offset_start += out_nchannels_old * nbytes; // this must be updated on all outputs
      params->g_dma_in.offset_end +=
          params->g_dma_in.offset_start +
          nbytes; // this must be updated on all outputs  FIXME Francesco incorrect calculation
#endif
    }
    /* loop back one epoch block */
    LL_ATON_RT_DecCurrEpochBlock(1);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Outputs_Memcpy_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before

  uint8_t *dst = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_outputs_memcpy_start(epoch_block, dst);
}

static void __LL_LIB_Outputs_Memcpy_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  if (params->g_size < 0)
  {
    params->g_dst_o_src += LL_Buffer_len(((LL_LIB_TensorShape_TypeDef *)params->g_tensors) + params->g_idx);
  }
  else
  {
    params->g_dst_o_src += (params->g_size);
  }
  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    /* loop back one epoch block */
    LL_ATON_RT_DecCurrEpochBlock(1);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

static void __LL_LIB_DMA_Pad_Memset_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  __ll_lib_stop_transfer();

  if (params->special.pad.callback_function != NULL)
  {
    /* return from current epoch block */
    __LL_ATON_RT_RetFromLibEpochBlockArray(false, NULL);
    /* call follow-up function */
    (*params->special.pad.callback_function)(&params->special.pad);
  }
}

static void __LL_LIB_DMA_Pad_Filling_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  __ll_pad_sw_params_t *common_params = &params->special.pad;

  /* set destination address */
  params->g_dst_o_src = (unsigned char *)common_params->out_target;

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): ASSIGN in=%lx, out=%lx, bytes=%u\n", __func__, __LINE__, (uintptr_t)common_params->in_target,
                 (uintptr_t)common_params->out_target, common_params->consecutive_bytes);
#if (ATON_PLAT_HAS_FFLUSH)
  LL_ATON_FFLUSH(stdout);
#endif
#endif

  /* start `memcpy` */
  {
    uint8_t *_dst = (uint8_t *)params->g_dst_o_src;
    uint8_t *_src = (uint8_t *)common_params->in_target;
    size_t n;

    n = params->g_size;
    n = __ll_lib_memcpy_prolog((void **)&_dst, (void **)&_src, n);

    if (n > 0)
    {
      params->g_dma_in.addr_base.p = (uint8_t *)_src;
      params->g_dma_in.offset_start = 0;
      params->g_dma_in.offset_end = n;
      params->g_dma_in.offset_limit = (uint8_t *)common_params->in_limit - (uint8_t *)_src; /* awful FIXME Francesco */

      params->g_dma_out.addr_base.p = _dst;
      params->g_dma_out.offset_start = 0;
      params->g_dma_out.offset_end = n;

      __ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, params->g_wait_mask);

      __ll_lib_start_transfer(params);
    }
    else
    {
      /* do not start any transfer and wait, just proceed to end function */
      __ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0);
    }
  }
}

static void __LL_LIB_DMA_Pad_Filling_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  __ll_pad_sw_params_t *common_params = &params->special.pad;

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  bool return_from_memcpy = true;

  do
  {
    if (common_params->pad_out_offsets_end[params->g_idx] > 0)
    {
      common_params->out_target += common_params->pad_out_offsets_end[params->g_idx];
    }

    if (common_params->pad_in_offsets_end[params->g_idx] < 0)
    {
      common_params->in_target -= common_params->pad_in_offsets_end[params->g_idx];
    }

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d) - end offsets for: curr_axis=%u, index=%u\n", __func__, __LINE__, params->g_idx,
                   common_params->indexes[params->g_idx]);
#endif

    /* returning from `memcpy` or coming from `end of do-while-loop` */
    if (return_from_memcpy)
    { // returning from `memcpy`
      LL_ATON_ASSERT(params->g_idx == common_params->consecutive_axis);
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d) - return from memcpy: curr_axis=%u, index=%u\n", __func__, __LINE__, params->g_idx,
                     common_params->indexes[params->g_idx]);
#endif
      common_params->in_target += common_params->consecutive_bytes;
      common_params->out_target += common_params->consecutive_bytes;

      if (params->g_idx == 0)
      {
        break;
      }
    }

    LL_ATON_ASSERT(params->g_idx > 0);
    params->g_idx--;
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d) - return to previous axis: curr_axis=%u, index=%u\n", __func__, __LINE__, params->g_idx,
                   common_params->indexes[params->g_idx]);
#endif

    common_params->indexes[params->g_idx] += 1;
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d) - inc axis index: curr_axis=%u, index=%u\n", __func__, __LINE__, params->g_idx,
                   common_params->indexes[params->g_idx]);
#endif
    if (common_params->indexes[params->g_idx] < common_params->min_shape[params->g_idx])
    {
    inner_loop:
      params->g_idx++;
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF(
          "%s(%d) - valid axis value, proceeded to next axis & apply start offsets: curr_axis=%u, index=%u\n", __func__,
          __LINE__, params->g_idx, common_params->indexes[params->g_idx]);
#endif

      if (common_params->pad_out_offsets_start[params->g_idx] > 0)
      {
        common_params->out_target += common_params->pad_out_offsets_start[params->g_idx];
      }

      if (common_params->pad_in_offsets_start[params->g_idx] < 0)
      {
        common_params->in_target -= common_params->pad_in_offsets_start[params->g_idx];
      }

      if (params->g_idx == common_params->consecutive_axis)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d) - reached consecutive axis -> memcpy: curr_axis=%u, index=%u\n", __func__, __LINE__,
                       params->g_idx, common_params->indexes[params->g_idx]);
#endif
        /* loop back one epoch block to start DMAs for filling next `consecutive bytes` */
        LL_ATON_RT_DecCurrEpochBlock(1);
        return;
      }
      else
      { // normal axis
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d) - repeat inner loop: curr_axis=%u, index=%u\n", __func__, __LINE__, params->g_idx,
                       common_params->indexes[params->g_idx]);
#endif
        goto inner_loop;
      }
    }
    else
    {
      common_params->indexes[params->g_idx] = 0;
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d) - axis overflow -> end loop & return to prev axis: curr_axis=%u, index=%u\n", __func__,
                     __LINE__, params->g_idx, common_params->indexes[params->g_idx]);
#endif
    }
    return_from_memcpy = false;
  } while (params->g_idx > 0);

  if (params->special.pad.callback_function != NULL)
  {
    /* return from current epoch block */
    __LL_ATON_RT_RetFromLibEpochBlockArray(false, NULL);
    /* call follow-up function */
    (*params->special.pad.callback_function)(&params->special.pad);
    return;
  }

#if defined(DUMP_RESULTS_PAD_OP)
  /* debug output print */
  switch (common_params->nbytes)
  {
  case 1:
  {
    int8_t *ptr = (int8_t *)common_params->saved_out_target;
    for (uint32_t i = 0; i < common_params->out_size; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 2:
  {
    int16_t *ptr = (int16_t *)(int8_t *)common_params->saved_out_target;
    for (uint32_t i = 0; i < common_params->out_size / 2; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 3: // NOTE: assuming no alignment
  {
    /* check endianess */
    const int32_t _const_val = 0x01020304;
    const int8_t *_const_val_ptr = (int8_t *)&_const_val;
    bool is_little_endian = (_const_val_ptr[0] == 0x04);

    int8_t *ptr = (int8_t *)common_params->saved_out_target;
    if (is_little_endian)
    {
      for (uint32_t i = 0; i < common_params->out_size / 3; i += 3)
      {
        int32_t value = 0;

        value |= ptr[i];
        value |= ptr[i + 1] << 8;
        value |= ptr[i + 2] << 16;

        LL_ATON_PRINTF("%d\n", value);
      }
    }
    else
    {
      for (uint32_t i = 0; i < common_params->out_size / 3; i += 3)
      {
        int32_t value = 0;

        value |= ptr[i] << 16;
        value |= ptr[i + 1] << 8;
        value |= ptr[i + 2];

        LL_ATON_PRINTF("%d\n", value);
      }
    }
  }
  break;
  case 4:
  {
    int32_t *ptr = (int32_t *)(int8_t *)common_params->saved_out_target;
    for (uint32_t i = 0; i < common_params->out_size / 4; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  default:
    LL_ATON_ASSERT(false);
    break;
  }
#endif // DUMP_RESULTS_PAD_OP
}

static void __LL_LIB_DMA_Transfer_Start_EpochBlock(const void *epoch_block)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  __ll_lib_start_transfer(params);
}

static void __LL_LIB_DMA_Transfer_End_EpochBlock(const void *epoch_block)
{
  __ll_lib_stop_transfer();

  /* proceed to next epoch block */
}

static LL_ATON_RT_EpochBlockItem_t _concat_case3_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_Concat_Case3_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_Concat_Case3_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -1,
        .last_epoch_num = -1,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _inputs_memcpy_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_Inputs_Memcpy_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_Inputs_Memcpy_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -2,
        .last_epoch_num = -2,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _inputs_batched_memcpy_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_Inputs_Batched_Memcpy_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_Inputs_Batched_Memcpy_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -3,
        .last_epoch_num = -3,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _outputs_memcpy_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_Outputs_Memcpy_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_Outputs_Memcpy_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -4,
        .last_epoch_num = -4,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _outputs_channel_split_aton_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_Outputs_Channel_Split_Aton_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_Outputs_Channel_Split_Aton_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -5,
        .last_epoch_num = -5,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _outputs_channel_split_batched_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_Outputs_Channel_Split_Batched_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_Outputs_Channel_Split_Batched_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -6,
        .last_epoch_num = -6,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _dma_ri2ir_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_DMA_Transfer_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_DMA_Transfer_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -7,
        .last_epoch_num = -7,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _dma_Pad_memset_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_DMA_Transfer_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_DMA_Pad_Memset_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -8,
        .last_epoch_num = -8,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _dma_Pad_filling_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_DMA_Pad_Filling_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_DMA_Pad_Filling_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -9,
        .last_epoch_num = -9,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _dma_transpose_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_DMA_Transfer_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_DMA_Transfer_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -10,
        .last_epoch_num = -10,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

static LL_ATON_RT_EpochBlockItem_t _slice_split_like_epoch_block_array[] = {
    // REMEMBER: static variables are not suited for multithreaded etc. environments
    {
        .start_epoch_block = __LL_LIB_DMA_Transfer_Start_EpochBlock,
        .end_epoch_block = __LL_LIB_DMA_Transfer_End_EpochBlock,
        .flags = EpochBlock_Flags_internal,
#ifdef LL_ATON_EB_DBG_INFO
        .epoch_num = -11,
        .last_epoch_num = -11,
#endif
    },
    {.flags = EpochBlock_Flags_last_eb},
};

/**
 * @brief  performs a memory copy operation from `ninputs` inputs to one output using stream engines `dma_in` and
 * `dma_out`
 * @param  inputs list of input tensor info structures
 * @param  ninputs number of inputs
 * @param  dst destination address
 * @param  nbytes number of bytes to copy (-1 means: derive from `inputs` structure)
 */
static void __LL_ATON_LIB_DMA_Inputs_Memcpy(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                            unsigned char *dst, int nbytes, int dma_in, int dma_out)
{
  /* start epoch block sequence */
  if (ninputs > 0)
  {
    /* prepare epoch */
    __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_static_const_dma_in, &_static_const_dma_out, dst, nbytes);

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, _inputs_memcpy_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(_inputs_memcpy_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

/**
 * @brief  performs a memory copy operation from `ninputs` inputs with a channel batch to one output (with a canonical
 * format batch == channels) using stream engines `dma_in` and `dma_out`
 * @param  inputs list of input tensor info structures
 * @param  ninputs number of inputs
 * @param  dst destination address
 */
static void __LL_ATON_LIB_DMA_Inputs_Batched_Memcpy(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                                    unsigned char *dst, int dma_in, int dma_out)
{
  uint32_t nbits = inputs[0].nbits;
  uint32_t in_ndims = inputs[0].ndims;
  uint32_t nbytes = (nbits + 7) >> 3;
  uint32_t in_fwidth = inputs[0].shape[(in_ndims - 4) + TDIM_FWIDTH];
  uint32_t in_fheight = inputs[0].shape[(in_ndims - 4) + TDIM_FHEIGHT];
  uint32_t in_batch = inputs[0].batch;
  uint32_t in_nchannels = inputs[0].shape[(in_ndims - 4) + TDIM_NCHANNELS];
  uint32_t in_nkernels = inputs[0].shape[(in_ndims - 4) + TDIM_NKERNELS];
  uint32_t out_nchannels = 0;
  uint32_t in_bytes_size = in_fwidth * in_fheight * in_nchannels * in_nkernels * nbytes;
  int i;

  for (i = 0; i < ninputs; i++)
    out_nchannels += inputs[i].shape[(in_ndims - 4) + TDIM_NCHANNELS];
  /* prepare epoch */

  // LL_ATON_PRINTF("nbytes=%d fw=%d fh=%d inb=%d inc=%d outc=%d\n", nbytes, in_fwidth, in_fheight, in_batch,
  // in_nchannels, out_nchannels);

  // memset(dst, 100, out_nchannels * nbytes * in_fwidth * in_fheight);

  LL_Streng_TensorInitTypeDef _dma_in = {
      .dir = 0, // input
      .raw = 1,
      .addr_base.p = inputs[0].addr_base.p,                 // this must be updated on all inputs
      .offset_start = inputs[0].offset_start,               // this must be updated on all inputs
      .offset_end = inputs[0].offset_start + in_bytes_size, // this must be updated on all inputs
      .frame_tot_cnt = 1,
      .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
  };

  LL_Streng_TensorInitTypeDef _dma_out = {
      .dir = 1, // output
      .raw = 0,
      .addr_base.p = dst, // this must be updated on all inputs
      .offset_start = 0,  // this must be updated on all inputs
      .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),

      .fwidth = in_fwidth,
      .fheight = in_fheight,
      .batch_depth = (nbytes == 4) ? (2 * in_batch) : in_batch, // this must be updated on all inputs
      .batch_offset = out_nchannels * nbytes,
      .frame_offset = in_batch * nbytes, // this must be updated on all inputs

      .frame_loop_cnt = in_nchannels / in_batch,                 // this must be updated on all inputs
      .frame_tot_cnt = in_nchannels / in_batch,                  // this must be updated on all inputs
      .loop_offset = in_fheight * in_fwidth * in_batch * nbytes, // this must be updated on all inputs
  };

  /* start epoch block sequence */
  if (ninputs > 0)
  {
    /* prepare epoch */
    __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_dma_in, &_dma_out, dst, -1);

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, _inputs_batched_memcpy_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(_inputs_batched_memcpy_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

/**
 * @brief  performs a memory copy operation from one input to `noutputs` outputs using stream engines `dma_in` and
 * `dma_out`
 * @param  src source address
 * @param  outputs list of output tensor shape structures
 * @param  noutputs number of outputs
 */
static void __LL_ATON_LIB_DMA_Outputs_Memcpy(const LL_LIB_TensorShape_TypeDef *input,
                                             const LL_LIB_TensorShape_TypeDef *outputs, unsigned int noutputs,
                                             int dma_in, int dma_out)
{
  /* start epoch block sequence */
  if (noutputs > 0)
  {
    /* prepare epoch */
    __ll_lib_prepare_outputs_epoch(outputs, noutputs, &_static_const_dma_in, &_static_const_dma_out, input);

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, _outputs_memcpy_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(_outputs_memcpy_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

/**
 * @brief  performs channel-split copy operation on an input and several outputs (both in ATON canonical format) using
 * DMA
 * @param  input tensor shape structure
 * @param  outputs tensor shape structures
 * @param  nr_of_outputs number of output tensors
 */
static void __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(const LL_LIB_TensorShape_TypeDef *input,
                                                         const LL_LIB_TensorShape_TypeDef *outputs,
                                                         unsigned int noutputs, unsigned int leading_dims, int dma_in,
                                                         int dma_out)
{
  uint32_t out_ndims = outputs[0].ndims;
  LL_ATON_ASSERT(out_ndims >= 3);
  uint32_t nbytes =
      LL_LIB_NBYTES(outputs[0].nbits); // assuming that value is the same for all output tensors and input tensor
  uint32_t out_fwidth = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FWIDTH]; // assuming that value is the same for all
                                                                              // output tensors and input tensor
  uint32_t out_fheight = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FHEIGHT]; // assuming that value is the same for
                                                                                // all output tensors and input tensor
  uint32_t out_nchannels = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_NCHANNELS];

  uint32_t in_nchannels = 0;
  int i;

  for (i = 0; i < noutputs; i++)
    in_nchannels += outputs[i].shape[(out_ndims - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  unsigned char nbits = (nbytes == 4) ? 16 : (nbytes * 8); // same for all output tensors and input tensor

  // LL_ATON_PRINTF("\ndma_out: addr_start=%p, addr_end=%p\n", LL_Buffer_addr_start(outputs + 0),
  // LL_Buffer_addr_end(outputs + 0));

  /* prepare epoch */
  LL_Streng_TensorInitTypeDef _dma_out = {
      .dir = 1, // output
      .raw = 1,
      .addr_base.p = outputs[0].addr_base.p,   // this must be updated on all outputs
      .offset_start = outputs[0].offset_start, // this must be updated on all outputs
      .offset_end = outputs[0].offset_end,     // this must be updated on all outputs
      .frame_tot_cnt = 1,
      .nbits_in = nbits,
      .nbits_out = nbits,
  };

  unsigned batch_depth = (nbytes == 4) ? (2 * out_nchannels) : out_nchannels;
  unsigned frame_size = out_fwidth * out_fheight * batch_depth;

  if (frame_size != 1)
  {
    LL_Streng_TensorInitTypeDef _dma_in = {
        .dir = 0,                            // input
        .raw = 0,                            // this must be updated on all outputs
        .addr_base.p = input->addr_base.p,   // this must be updated on all outputs
        .offset_start = input->offset_start, // this must be updated on all outputs
        .nbits_in = nbits,
        .nbits_out = nbits,

        .fwidth = out_fwidth,
        .fheight = out_fheight,
        .batch_depth = batch_depth, // this must be updated on all outputs
        .batch_offset = in_nchannels * nbytes,

        .line_offset = in_nchannels * out_fwidth * nbytes,

        .frame_offset = in_nchannels * out_fheight * out_fwidth * nbytes,

        .frame_loop_cnt = 0,
        .frame_tot_cnt = leading_dims,
        .loop_offset = 0,
    };

    // LL_ATON_PRINTF("dma_in: frame_size=%d, nbytes=%u, in_nchannels=%u, out_nchannels=%u, src=%p, out_fwidth=%u,
    // out_fheight=%u, batch_depth=%u, batch_offset=%u, line_offset=%u, frame_offset=%u, frame_tot_cnt=%u\n",
    //        frame_size, nbytes, in_nchannels, out_nchannels, LL_Streng_addr_start(&_dma_in),
    //        _dma_in.fwidth, _dma_in.fheight, _dma_in.batch_depth, _dma_in.batch_offset, _dma_in.line_offset,
    //        _dma_in.frame_offset, _dma_in.frame_tot_cnt);
    // LL_ATON_FFLUSH(stdout);

    __ll_lib_prepare_outputs_epoch(outputs, noutputs, &_dma_in, &_dma_out, input);
  }
  else // frame_size == 1
  {
#if 1
    LL_ATON_ASSERT(0); // should never happen!!!
#else
    LL_Streng_TensorInitTypeDef _dma_in = {
        .dir = 0,             // input
        .raw = 1,             // this must be updated on all outputs
        .addr_base.p = src,   // this must be updated on all outputs
        .offset_start = 0,    // this must be updated on all outputs
        .offset_end = nbytes, // this must be updated on all outputs
        .nbits_in = nbits,
        .nbits_out = nbits,

        .frame_offset = in_nchannels * out_fheight * out_fwidth * nbytes,

        .frame_loop_cnt = 0,
        .frame_tot_cnt = leading_dims,
        .loop_offset = 0,
    };

    __ll_lib_prepare_outputs_epoch(outputs, noutputs, &_dma_in, &_dma_out, src);
#endif
  }

  /* start epoch block sequence */
  if (noutputs > 0)
  {
    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, _outputs_channel_split_aton_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(_outputs_channel_split_aton_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

/**
 * @brief  performs a channel-split memory copy operation from one input (ATON canonical) to `noutputs`
 * non-ATON-canonical outputs using stream engines `dma_in` and `dma_out`
 * @param  src source address
 * @param  outputs list of output tensor shape structures
 * @param  noutputs number of outputs
 */
static void __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(const LL_LIB_TensorShape_TypeDef *input,
                                                            const LL_LIB_TensorShape_TypeDef *outputs,
                                                            unsigned int noutputs, int dma_in, int dma_out)
{
  uint32_t out_ndims = outputs[0].ndims;
  LL_ATON_ASSERT(out_ndims >= 3);
  uint32_t nbytes =
      LL_LIB_NBYTES(outputs[0].nbits); // assuming that value is the same for all output tensors and input tensor
  uint32_t out_fwidth = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FWIDTH]; // assuming that value is the same for all
                                                                              // output tensors and input tensor
  uint32_t out_fheight = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FHEIGHT]; // assuming that value is the same for
                                                                                // all output tensors and input tensor
  uint32_t out_nchannels = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_NCHANNELS];

  uint32_t in_nchannels = 0;
  int i;

  uint16_t out_batch = outputs[0].batch;

  for (i = 0; i < noutputs; i++)
    in_nchannels += outputs[i].shape[(out_ndims - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  LL_ATON_ASSERT((input->batch == 0) || (input->batch == in_nchannels));
  LL_ATON_ASSERT((out_nchannels % out_batch) == 0);

  unsigned char nbits = (nbytes == 4) ? 16 : (nbytes * 8); // same for all output tensors and input tensor

  // LL_ATON_PRINTF("\ndma_out: addr_start=%p, addr_end=%p\n", LL_Buffer_addr_start(outputs + 0),
  // LL_Buffer_addr_end(outputs + 0));

  /* prepare epoch */
  LL_Streng_TensorInitTypeDef _dma_out = {
      .dir = 1, // output
      .raw = 1,
      .addr_base.p = outputs[0].addr_base.p,   // this must be updated on all outputs
      .offset_start = outputs[0].offset_start, // this must be updated on all outputs
      .offset_end = outputs[0].offset_end,     // this must be updated on all outputs
      .frame_tot_cnt = 1,
      .nbits_in = nbits,
      .nbits_out = nbits,
  };

  unsigned batch_depth = (nbytes == 4) ? (2 * out_batch) : out_batch;
  unsigned frame_size = out_fwidth * out_fheight * batch_depth;

  if (frame_size != 1)
  {
    LL_Streng_TensorInitTypeDef _dma_in = {
        .dir = 0,                            // input
        .raw = 0,                            // this must be updated on all outputs
        .addr_base.p = input->addr_base.p,   // this must be updated on all outputs
        .offset_start = input->offset_start, // this must be updated on all outputs
        .nbits_in = nbits,
        .nbits_out = nbits,

        .fwidth = out_fwidth,
        .fheight = out_fheight,
        .batch_depth = batch_depth, // this must be updated on all outputs
        .batch_offset = in_nchannels * nbytes,

        .line_offset = in_nchannels * out_fwidth * nbytes,

        .frame_offset = in_nchannels * out_fheight * out_fwidth * nbytes,

        .frame_loop_cnt = 1,
        .frame_tot_cnt = out_nchannels / out_batch, // this must be updated on all outputs
        .loop_offset = out_batch * nbytes,          // this must be updated on all outputs
    };

    // LL_ATON_PRINTF("dma_in: frame_size=%d, nbytes=%u, in_nchannels=%u, out_nchannels=%u, src=%p, out_fwidth=%u,
    // out_fheight=%u, batch_depth=%u, batch_offset=%u, line_offset=%u, frame_offset=%u, frame_tot_cnt=%u\n",
    //        frame_size, nbytes, in_nchannels, out_nchannels, LL_Streng_addr_start(&_dma_in),
    //        _dma_in.fwidth, _dma_in.fheight, _dma_in.batch_depth, _dma_in.batch_offset, _dma_in.line_offset,
    //        _dma_in.frame_offset, _dma_in.frame_tot_cnt);
    // LL_ATON_FFLUSH(stdout);

    __ll_lib_prepare_outputs_epoch(outputs, noutputs, &_dma_in, &_dma_out, input);
  }
  else // frame_size == 1
  {
#if 1
    LL_ATON_ASSERT(0); // should never happen!!!
#else
    // TODO (as not yet adapted for this case)
    LL_Streng_TensorInitTypeDef _dma_in = {
        .dir = 0,             // input
        .raw = 1,             // this must be updated on all outputs
        .addr_base.p = src,   // this must be updated on all outputs
        .offset_start = 0,    // this must be updated on all outputs
        .offset_end = nbytes, // this must be updated on all outputs
        .nbits_in = nbits,
        .nbits_out = nbits,

        .frame_offset = in_nchannels * out_fheight * out_fwidth * nbytes,

        .frame_loop_cnt = 0,
        .frame_tot_cnt = leading_dims,
        .loop_offset = 0,
    };

    __ll_lib_prepare_outputs_epoch(outputs, noutputs, &_dma_in, &_dma_out, src);
#endif
  }

  /* start epoch block sequence */
  if (noutputs > 0)
  {
    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, _outputs_channel_split_aton_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(_outputs_channel_split_batched_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

/**
 * @brief  performs a tensor ImageToRow transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 * @param  stride_h vertical stride for the sliding window
 * @param  stride_w horizontal stride for the sliding window
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_ImageToRow(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                               const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                               unsigned stride_h, unsigned stride_w, int dma_in, int dma_out)
{
  uint32_t in_batches = inputs[0].shape[TDIM_NKERNELS];
  uint32_t in_fwidth = inputs[0].shape[TDIM_FWIDTH];
  uint32_t in_fheight = inputs[0].shape[TDIM_FHEIGHT];
  uint32_t in_nchannels = inputs[0].shape[TDIM_NCHANNELS];
  uint32_t out_batches = output->shape[TDIM_NKERNELS];
  uint32_t out_fwidth = output->shape[TDIM_FWIDTH];
  uint32_t out_fheight = output->shape[TDIM_FHEIGHT];
  uint32_t out_nchannels = output->shape[TDIM_NCHANNELS];
  uint32_t nbits = inputs[0].nbits;
  uint32_t nbits_unsigned = inputs[0].Qunsigned;
  uint32_t nbytes = (nbits + 7) >> 3;
  uint32_t in_bytes_size = in_fwidth * in_fheight * in_nchannels * in_batches * nbytes;

  /*
LL_ATON_PRINTF("in: b=%d w=%d g=%d c=%d\n",in_batches,in_fwidth,in_fheight,in_nchannels);
LL_ATON_PRINTF("out: b=%d w=%d g=%d c=%d ndims=%d\n",out_batches,out_fwidth,out_fheight,out_nchannels,output->ndims);
   */

  if (ninputs != 1)
    __LL_LIB_ERROR(_ERR_NINPUTS, LL_ATON_INVALID_PARAM);

  if (nbits != output->nbits)
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);

  if ((output->ndims < 1) || (output->ndims > 4))
    __LL_LIB_ERROR(_ERR_SHAPE_OUT, LL_ATON_INVALID_PARAM);

  if ((inputs[0].ndims < 1) || (inputs[0].ndims > 4))
    __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);

  if (in_batches != out_batches || out_nchannels != in_nchannels * (blocksize_h * blocksize_w) ||
      (in_fwidth < blocksize_w) || (in_fheight < blocksize_h) || ((in_fwidth - blocksize_w) % stride_w) ||
      ((in_fheight - blocksize_h) % stride_h) || (out_fwidth != (((in_fwidth - blocksize_w) / stride_w) + 1)) ||
      (out_fheight != (((in_fheight - blocksize_h) / stride_h) + 1)))
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);

  /* prepare epoch */
  /* the output DMA goes just sequential with non batched output */
  LL_Streng_TensorInitTypeDef _dma_out = {
      .dir = 1, // output
      .raw = 1,
      .addr_base.i = output->addr_base.i,
      .offset_start = output->offset_start,
      .offset_end = output->offset_start + in_bytes_size,
      .frame_tot_cnt = 1,
      .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_unsigned = nbits_unsigned,
  };

  unsigned batch_depth = (nbytes == 4) ? (2 * in_nchannels) : in_nchannels;

  /* this DMA scans the input one block at a time of size blocksize_h * blocksize_w * in_nchannels */
  if ((blocksize_w * blocksize_h * batch_depth) != 1)
  {
    LL_Streng_TensorInitTypeDef _dma_in = {
        .dir = 0, // input
        .addr_base.i = inputs[0].addr_base.i,
        .offset_start = inputs[0].offset_start,
        .offset_limit = inputs[0].offset_limit,
        .raw = 0,
        .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
        .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
        .nbits_unsigned = nbits_unsigned,

        .fwidth = blocksize_w,
        .batch_depth = batch_depth,
        .batch_offset = in_nchannels * nbytes,

        .fheight = blocksize_h,
        .line_offset = in_fwidth * in_nchannels * nbytes,

        .frame_loop_cnt = ((in_fwidth - blocksize_w) / stride_w) + 1, // nframes = frame_loop_cnt
        .frame_offset = stride_w * in_nchannels * nbytes,

        .frame_tot_cnt =
            in_batches * (((in_fheight - blocksize_h) / stride_h) + 1) * (((in_fwidth - blocksize_w) / stride_w) + 1),
        .loop_offset = stride_h * in_fwidth * in_nchannels * nbytes,
    };

    __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_dma_in, &_dma_out,
                                  /* all of the rest of parameters are irrelevant to this use */ 0, 0);
  }
  else // blocksize_w * blocksize_h * batch_depth) == 1
  {    // use raw mode
    LL_Streng_TensorInitTypeDef _dma_in = {
        .dir = 0, // input
        .addr_base.i = inputs[0].addr_base.i,
        .offset_start = inputs[0].offset_start,
        .offset_end = inputs[0].offset_start + nbytes,
        .offset_limit = inputs[0].offset_limit,
        .raw = 1,
        .nbits_in = (nbytes * 8),  // nbytes != 4
        .nbits_out = (nbytes * 8), // nbytes != 4
        .nbits_unsigned = nbits_unsigned,

        .frame_loop_cnt = ((in_fwidth - blocksize_w) / stride_w) + 1, // nframes = frame_loop_cnt
        .frame_offset = stride_w * in_nchannels * nbytes,

        .frame_tot_cnt =
            in_batches * (((in_fheight - blocksize_h) / stride_h) + 1) * (((in_fwidth - blocksize_w) / stride_w) + 1),
        .loop_offset = stride_h * in_fwidth * in_nchannels * nbytes,
    };

    __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_dma_in, &_dma_out,
                                  /* all of the rest of parameters are irrelevant to this use */ 0, 0);
  }

  /* configure stream switch */
  __ll_lib_strswitch_set_dmas(dma_in, dma_out, _dma_ri2ir_epoch_block_array);

  /* start epoch block sequence */
  LL_ATON_RT_Insert_LibEpochBlockArray(_dma_ri2ir_epoch_block_array);

  return LL_ATON_OK;
}

/**
 * @brief  performs a tensor SpaceToDepth transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_SpaceToDepth(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                                 int dma_in, int dma_out)
{
  return LL_ATON_LIB_DMA_ImageToRow(inputs, ninputs, output, blocksize_h, blocksize_w, blocksize_h, blocksize_w, dma_in,
                                    dma_out);
}

/**
 * @brief  performs a tensor RowToImage transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 * @param  stride_h vertical stride for the sliding window
 * @param  stride_w horizontal stride for the sliding window
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_RowToImage(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                               const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                               unsigned stride_h, unsigned stride_w, int dma_in, int dma_out)
{
  int in_batches = inputs[0].shape[TDIM_NKERNELS];
  int in_fwidth = inputs[0].shape[TDIM_FWIDTH];
  int in_fheight = inputs[0].shape[TDIM_FHEIGHT];
  int in_nchannels = inputs[0].shape[TDIM_NCHANNELS];
  int out_batches = output->shape[TDIM_NKERNELS];
  int out_fwidth = output->shape[TDIM_FWIDTH];
  int out_fheight = output->shape[TDIM_FHEIGHT];
  int out_nchannels = output->shape[TDIM_NCHANNELS];
  int nbits = inputs[0].nbits;
  unsigned nbits_unsigned = inputs[0].Qunsigned;
  int nbytes = (nbits + 7) >> 3;
  uint32_t in_bytes_size = in_fwidth * in_fheight * in_nchannels * in_batches * nbytes;

  /*
LL_ATON_PRINTF("in: b=%d w=%d g=%d c=%d\n",in_batches,in_fwidth,in_fheight,in_nchannels);
LL_ATON_PRINTF("out: b=%d w=%d g=%d c=%d ndims=%d\n",out_batches,out_fwidth,out_fheight,out_nchannels,output->ndims);
   */

  if (ninputs != 1)
    __LL_LIB_ERROR(_ERR_NINPUTS, LL_ATON_INVALID_PARAM);

  if (nbits != output->nbits)
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);

  if ((output->ndims < 1) || (output->ndims > 4))
    __LL_LIB_ERROR(_ERR_SHAPE_OUT, LL_ATON_INVALID_PARAM);

  if ((inputs[0].ndims < 1) || (inputs[0].ndims > 4))
    __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);

  if (in_batches != out_batches || in_nchannels != out_nchannels * (blocksize_h * blocksize_w) ||
      (out_fwidth < blocksize_w) || (out_fheight < blocksize_h) || ((out_fwidth - blocksize_w) % stride_w) ||
      ((out_fheight - blocksize_h) % stride_h) || (in_fwidth != (((out_fwidth - blocksize_w) / stride_w) + 1)) ||
      (in_fheight != (((out_fheight - blocksize_h) / stride_h) + 1)))
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);

  /* prepare epoch */
  /* the input DMA goes just sequential with non batched input */
  LL_Streng_TensorInitTypeDef _dma_in = {
      .dir = 0, // input
      .raw = 1,
      .addr_base.i = inputs[0].addr_base.i,
      .offset_start = inputs[0].offset_start,
      .offset_end = inputs[0].offset_start + in_bytes_size,
      .offset_limit = inputs[0].offset_limit,
      .frame_tot_cnt = 1,
      .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_unsigned = nbits_unsigned,
  };

  /* this DMA scans the output one block at a time of size blocksize_h * blocksize_w * out_nchannels */
  LL_Streng_TensorInitTypeDef _dma_out = {
      .dir = 1, // output
      .addr_base.i = output->addr_base.i,
      .offset_start = output->offset_start,
      .raw = 0,
      .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_unsigned = nbits_unsigned,

      .fwidth = blocksize_w,
      .batch_depth = (nbytes == 4) ? (2 * out_nchannels) : out_nchannels,
      .batch_offset = out_nchannels * nbytes,

      .fheight = blocksize_h,
      .line_offset = out_fwidth * out_nchannels * nbytes,

      .frame_loop_cnt = ((out_fwidth - blocksize_w) / stride_w) + 1, // nframes = frame_loop_cnt
      .frame_offset = stride_w * out_nchannels * nbytes,

      .frame_tot_cnt =
          out_batches * (((out_fheight - blocksize_h) / stride_h) + 1) * (((out_fwidth - blocksize_w) / stride_w) + 1),
      .loop_offset = stride_h * out_fwidth * out_nchannels * nbytes,
  };

  /* prepare epoch */
  __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_dma_in, &_dma_out,
                                /* all of the rest of parameters are irrelevant to this use */ 0, 0);

  /* configure stream switch */
  __ll_lib_strswitch_set_dmas(dma_in, dma_out, _dma_ri2ir_epoch_block_array);

  /* start epoch block sequence */
  LL_ATON_RT_Insert_LibEpochBlockArray(_dma_ri2ir_epoch_block_array);

  return LL_ATON_OK;
}

/**
 * @brief  performs a tensor DepthToSpace transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Supports only DCR (depth-column-row) order re-arrangement
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_DepthToSpace(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h, unsigned blocksize_w,
                                 int dma_in, int dma_out)
{
  return LL_ATON_LIB_DMA_RowToImage(inputs, ninputs, output, blocksize_h, blocksize_w, blocksize_h, blocksize_w, dma_in,
                                    dma_out);
}

int LL_ATON_LIB_DMA_Transpose(const LL_LIB_TensorShape_TypeDef *input, const uint32_t *input_axes_offsets,
                              const LL_LIB_TensorShape_TypeDef *output, const uint32_t *output_axes_offsets,
                              const uint8_t *target_pos, const uint8_t *perm_to_use, int dma_in, int dma_out)
{
  if (LL_Buffer_len(output) < __LL_DMA_MIN_BUFF_LEN)
  { // not worth doing it in HW
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("===> running pure SW version of `Transpose`\n");
#endif

    return LL_ATON_LIB_Transpose(input, input_axes_offsets, output, output_axes_offsets, perm_to_use);
  }

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("===> running DMA version of `Transpose`\n");
#endif

  // parameter checks
  if (input->nbits != output->nbits)
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM); // TODO: this restriction might be relaxed by a more
                                                       // accurate configuration of the DMAs?!?
  }

  if ((input->nbits < 8) || (input->nbits > 32))
  {
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  if (input->ndims != output->ndims)
  {
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }

  if ((input->ndims != 4) && (input->ndims != 3))
  {
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }

  uint8_t nbytes = LL_LIB_NBYTES(input->nbits);

  /* prepare epoch */
  /* the input DMA goes just sequential with non batched input */
  LL_Streng_TensorInitTypeDef _dma_in = {
      .dir = 0,
      .raw = 1,
      .addr_base.i = input->addr_base.i,
      .offset_start = input->offset_start,
      .offset_end = input->offset_end,
      .offset_limit = input->offset_limit,
      .frame_tot_cnt = 1,
      .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
      .nbits_unsigned = 0,
  };

  /* save input DMA configuration */
  __ll_lib_params_t *params = __ll_lib_get_params();
  params->g_dma_in = _dma_in;

  /* this DMA performs the optimized `Transpose` */
  switch (input->ndims)
  {
  case 4:
  {
    LL_Streng_TensorInitTypeDef _dma_out = {
        .dir = 1, // output
        .addr_base.i = output->addr_base.i,
        .offset_start = output->offset_start,
        .raw = 0,
        .batch_depth = (nbytes == 4) ? 2 : 1,
        .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
        .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
        .nbits_unsigned = 0,
        .fwidth = input->shape[3],
        .batch_offset = output_axes_offsets[target_pos[3]],
        .fheight = input->shape[2],
        .line_offset = output_axes_offsets[target_pos[2]],
        .frame_loop_cnt = input->shape[1],
        .frame_offset = output_axes_offsets[target_pos[1]],
        .loop_offset = output_axes_offsets[target_pos[0]],
        .frame_tot_cnt = input->shape[0] * input->shape[1],
    };

    /* save output DMA configuration */
    params->g_dma_out = _dma_out;
  }
  break;

  case 3:
  {
    LL_Streng_TensorInitTypeDef _dma_out = {
        .dir = 1, // output
        .addr_base.i = output->addr_base.i,
        .offset_start = output->offset_start,
        .raw = 0,
        .batch_depth = (nbytes == 4) ? 2 : 1,
        .nbits_in = (nbytes == 4) ? 16 : (nbytes * 8),
        .nbits_out = (nbytes == 4) ? 16 : (nbytes * 8),
        .nbits_unsigned = 0,
        .fwidth = input->shape[2],
        .batch_offset = output_axes_offsets[target_pos[2]],
        .fheight = input->shape[1],
        .line_offset = output_axes_offsets[target_pos[1]],
        .frame_loop_cnt = input->shape[0],
        .frame_offset = output_axes_offsets[target_pos[0]],
        .loop_offset = 0,
        .frame_tot_cnt = input->shape[0],
    };

    /* save output DMA configuration */
    params->g_dma_out = _dma_out;
  }
  break;

  default:
    LL_ATON_ASSERT(0); // can never happen
  }

  /* configure stream switch */
  __ll_lib_strswitch_set_dmas(dma_in, dma_out, _dma_transpose_epoch_block_array);

  /* schedule epoch block */
  LL_ATON_RT_Insert_LibEpochBlockArray(_dma_transpose_epoch_block_array);

  return LL_ATON_OK;
}

#ifndef _LL_LIB_Concat_Cast_USE_ATON_HW
#define _LL_LIB_Concat_Cast_USE_ATON_HW 1
#endif

/**
 * @brief  performs a concat operation according to ONNX semantics
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structure
 * @param  axis for concatenation
 * @retval Error code
 */
int LL_ATON_LIB_Concat(const LL_Buffer_InfoTypeDef *inputs, unsigned int ninputs, const LL_Buffer_InfoTypeDef *output,
                       unsigned int axis, int dma_in, int dma_out)
{
  int i, k;

  // LL_ATON_PRINTF("Concat ------ axis=%d\n", axis);
  if (ninputs == 0)
    __LL_LIB_ERROR(_ERR_NINPUTS, LL_ATON_INVALID_PARAM);

  int in_ndims = inputs[0].ndims;

  if (in_ndims < 4)
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);

  int in_batch = inputs[0].batch;
  // int in_fwidth = inputs[0].shape[(in_ndims - 4) + TDIM_FWIDTH];
  int in_fheight = inputs[0].shape[(in_ndims - 4) + TDIM_FHEIGHT];
  int in_nchannels = inputs[0].shape[(in_ndims - 4) + TDIM_NCHANNELS];
  int out_batch = output->batch;
  int out_fwidth = output->shape[(in_ndims - 4) + TDIM_FWIDTH];
  // int out_fheight = output->shape[(in_ndims - 4) + TDIM_FHEIGHT];
  int out_nchannels = output->shape[(in_ndims - 4) + TDIM_NCHANNELS];

  int in_canonical = (in_batch == in_nchannels);
  int out_canonical = (out_batch == out_nchannels);

  // convert axis from ...CHW -> ...HWC
  int axis_lut[] = {TDIM_NKERNELS, TDIM_NCHANNELS, TDIM_FHEIGHT, TDIM_FWIDTH}; // 0, 3, 1, 2
#define LUT_AXIS(x) ((x >= (in_ndims - 4)) ? (in_ndims - 4) + axis_lut[x - (in_ndims - 4)] : x)

  /*
LL_ATON_PRINTF("axis: %d\n",axis);
LL_ATON_PRINTF("in: b=%d w=%d g=%d c=%d\n",in_batches,in_fwidth,in_fheight,in_nchannels);
LL_ATON_PRINTF("out: b=%d w=%d g=%d c=%d ndims=%d\n",out_batches,out_fwidth,out_fheight,out_nchannels,output->ndims);
   */

  if (output->ndims != in_ndims)
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);

  /* patch up axis to 4 dimensions */
  axis = in_ndims < 4 ? (axis + (4 - in_ndims)) : axis;
  /* we can move left axis if dimensions are == 1  */
  /* this should be more efficient (larger chunks) */
  /* except if formats are batched                 */

  int nbits = inputs[0].nbits;
  int nbytes = (inputs[0].nbits + 7) >> 3;

  if (nbits & 0x7)
    __LL_LIB_ERROR(_ERR_FRACTIONAL, LL_ATON_INVALID_PARAM); // for now can't handle fractional bytes

  int tot_size = 0;
  int tot_axis_dim = 0;
  int atonn_axis = LUT_AXIS(axis);

  for (i = 0; i < ninputs; i++)
  {
    tot_size += LL_Buffer_len(inputs + i);
    if (!(((inputs[i].shape[(in_ndims - 4) + TDIM_NCHANNELS] == inputs[i].batch) && in_canonical) ||
          (in_batch == inputs[i].batch)))
    {
      // LL_ATON_PRINTF("name=%s inb=%d inb(i)=%d chan(i)=%d\n", inputs[i].name, in_batch, inputs[i].batch,
      // inputs[i].shape[TDIM_NCHANNELS]);
      __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);
    }
    if (inputs[i].ndims != in_ndims)
      __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);
    if (inputs[i].nbits != nbits)
      __LL_LIB_ERROR(_ERR_NBITS_IN, LL_ATON_INVALID_PARAM);

    tot_axis_dim += inputs[i].shape[atonn_axis];

    for (k = 0; k < in_ndims; k++)
    {
      if (k != atonn_axis && inputs[0].shape[k] != inputs[i].shape[k])
        __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);
    }
  }

  for (k = 0; k < in_ndims; k++)
  {
    if (k != atonn_axis && output->shape[k] != inputs[0].shape[k])
    {
      // LL_ATON_PRINTF("k=%d axis=%d %d != %d\n", k, atonn_axis, output->shape[k], inputs[0].shape[k]);
      __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
    }
  }

  if (output->shape[atonn_axis] != tot_axis_dim)
    __LL_LIB_ERROR(_ERR_AXIS, LL_ATON_INVALID_PARAM);

  // LL_ATON_PRINTF("tot: size=%d b=%d w=%d g=%d c=%d\n",tot_size,tot_batches,tot_fwidth,tot_fheight,tot_nchannels);

  if (nbits != output->nbits) // perhaps this could be relaxed later on FIXME !!!
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);

  if (tot_size > LL_Buffer_len(output))
  {
    // LL_ATON_PRINTF("tot_size=%d out size=%ld\n", tot_size, LL_Buffer_len(output));
    __LL_LIB_ERROR(_ERR_BUFFER, LL_ATON_INVALID_PARAM);
  }

#if 1 // 0/1
  int axis_is_leftmost = 1;

  for (i = 0; i < axis; i++)
    axis_is_leftmost &= (output->shape[i] == 1);

  // 0-> >= 3 anything
  // 2-> >= 3 H = 1
  // 1->2 C
  // 3 -> 0 W

  if (nbits > 24) // assumes that inputs & output have the same number of bits (`nbits`), see above `__LL_LIB_ERROR`s
    axis_is_leftmost = 0;

  if (axis_is_leftmost)
  {
    switch ((in_ndims - 1) - axis) // count from right CHW, W=0,H=1,C=2, anything else >= 3
    {
    default: // any dimension left of onnx channels (they should all be == 1)
      if (in_batch != out_batch)
      { // it's unclear for now what it means to concatenate if different batches or non canonical
        __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
      }
#if _LL_LIB_Concat_Cast_USE_ATON_HW
      __LL_ATON_LIB_DMA_Inputs_Memcpy(inputs, ninputs, LL_Buffer_addr_start(output), -1, dma_in, dma_out);
#else  // !_LL_LIB_Concat_Cast_USE_ATON_HW
      {
        /* case when concatenation is on ONNX dim before channels or height */
        unsigned char *start = LL_Buffer_addr_start(output);
        for (i = 0; i < ninputs; i++)
        {
          // LL_ATON_PRINTF("in[%d]\n",i);
          memcpy((void *)start, (void *)LL_Buffer_addr_start(inputs + i), LL_Buffer_len(inputs + i));
          start += LL_Buffer_len(inputs + i);
        }
      }
#endif // !_LL_LIB_Concat_Cast_USE_ATON_HW
      return LL_ATON_OK;
    case 2: // Channels
    {
      if (in_batch == out_batch)
      {
        __LL_ATON_LIB_DMA_Inputs_Memcpy(inputs, ninputs, LL_Buffer_addr_start(output), -1, dma_in, dma_out);
      }
      else
      {
        __LL_ATON_LIB_DMA_Inputs_Batched_Memcpy(inputs, ninputs, LL_Buffer_addr_start(output), dma_in, dma_out);
      }
    }
      return LL_ATON_OK;
    case 0: // Width
    {
      if (in_batch != out_batch)
      { // it's unclear for now what it means to concatenate if different batches or non canonical
        __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
      }
      // we need to scan raster scan input tensors and copy each line data onto output
      unsigned int out_pix_size = nbytes * out_nchannels;
      unsigned int out_line_size = out_pix_size * out_fwidth;
      unsigned char *out_start = LL_Buffer_addr_start(output);

#if _LL_LIB_Concat_Cast_USE_ATON_HW
      {
        unsigned int pix_size = nbytes * inputs[0].shape[(in_ndims - 4) + TDIM_NCHANNELS];
        unsigned int line_size = pix_size * inputs[0].shape[(in_ndims - 4) + TDIM_FWIDTH];

        __ll_lib_params_t *params = __ll_lib_get_params();
        params->special.concat_case3.outer_idx = 0;
        params->special.concat_case3.in_fheight = in_fheight;
        params->special.concat_case3.nbytes = nbytes;
        params->special.concat_case3.out_line_size = out_line_size;
        params->special.concat_case3.in_curr = LL_Buffer_addr_start((LL_LIB_TensorInfo_TypeDef *)params->g_tensors);

        /* start epoch block sequence */
        if ((ninputs > 0) && (in_fheight > 0))
        {
          /* prepare epoch */
          __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_static_const_dma_in, &_static_const_dma_out,
                                        (void *)out_start, line_size);

          /* configure stream switch */
          __ll_lib_strswitch_set_dmas(dma_in, dma_out, _concat_case3_epoch_block_array);

          LL_ATON_RT_Insert_LibEpochBlockArray(_concat_case3_epoch_block_array);
        }
        else
        {
          /* proceed to next epoch block */
        }
      }
#else  // !_LL_LIB_Concat_Cast_USE_ATON_HW
      {
        for (i = 0; i < ninputs; i++)
        {
          // LL_ATON_PRINTF("in[%d]\n",i);
          unsigned int pix_size = nbytes * inputs[i].nchannels;
          unsigned int line_size = pix_size * inputs[i].fwidth;
          unsigned char *out_curr = out_start;
          unsigned char *in_curr = LL_Buffer_addr_start(inputs + i);
          unsigned int row;
          for (row = 0; row < in_fheight; row++)
          {
            memcpy((void *)out_curr, (void *)(in_curr), line_size);
            out_curr += out_line_size;
            in_curr += line_size;
          }
          out_start += line_size;
        }
      }
#endif //_!LL_LIB_USE_ATON_HW
    }
      return LL_ATON_OK;
    }
  }
#endif // 0/1
  /* python example
  stop = Z.size
  jump = 1
  for i in range(axis+1,len(X.shape)):
      jump = jump*X.shape[i]
  jump = jump*Z1.shape[axis]

  start = 0
  for i in range(0,len(Ins)):
      W = Ins[i]
      if (verbose): print("W:",W)
      copy_val = 1
      for k in range(axis+1,len(Ins_orig[i].shape)):
          copy_val = copy_val*Ins_orig[i].shape[k]
      copy_val = copy_val*Ins_orig[i].shape[axis]
      src = 0
      for dst in range(start,stop,jump):
          if (verbose): print("start:",start)
          if (verbose): print("Z tmp:",Z[dst:dst+copy_val])
          if (verbose): print(Z[dst:dst+copy_val].shape)
          if (verbose): print("W tmp:",W[src:src+copy_val])
          if (verbose): print(W[src:src+copy_val].shape)
          Z[dst:dst+copy_val] = W[src:src+copy_val]
          src = src + copy_val
      start = start + copy_val
  */
  if (in_canonical == 0)
    __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);
  if (out_canonical == 0)
    __LL_LIB_ERROR(_ERR_SHAPE_OUT, LL_ATON_INVALID_PARAM);
#if 0
  // it's unclear for now what it means to concatenate if different batches or non canonical
  if (in_batch != out_batch)
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
#endif

  uint32_t start = 0;
  // convert axis from ...CHW -> ...HWC
  // LL_ATON_PRINTF("axis =%d\n", axis);
  // LL_ATON_PRINTF("atonn axis=%d\n", atonn_axis);

  uint32_t stop = tot_size;
  uint32_t jump_base = 1;
  for (i = atonn_axis + 1; i < output->ndims; i++)
    jump_base *= output->shape[i];
  jump_base *= nbytes;
  uint32_t jump = jump_base * output->shape[atonn_axis];
  // LL_ATON_PRINTF("jump_base=%d, jump=%d\n", jump_base, jump);

  for (i = 0; i < ninputs; i++)
  {
    uint32_t copy_val = inputs[i].shape[atonn_axis] * jump_base;
    // LL_ATON_PRINTF("i=%d copy_val=%d\n", i, copy_val);

    int dst;
    int src = 0;
    for (dst = start; dst < stop; dst += jump, src += copy_val)
    {
      // LL_ATON_PRINTF("i =%d dst = %d src = %d\n", i, dst, src);
      memcpy(LL_Buffer_addr_start(output) + dst, LL_Buffer_addr_start(inputs + i) + src, copy_val);
    }
    start += copy_val;
  }

  return LL_ATON_OK;
}

/* replace these with ARM ISA instructions and assembly intrinsics */

static int floating_to_Q(float f, int Qm, int Qn)
{
  float tmp;
  if (Qn >= 0)
    tmp = (f * ((int)1 << Qn) + (f > (float)0 ? (float)0.5 : (float)-0.5));
  if (Qn < 0)
    tmp = (f * (float)1 / ((int)1 << -Qn));
  if (tmp > (float)(((int)1 << (Qm + Qn)) - 1))
    tmp = (float)(((int)1 << (Qm + Qn)) - 1);
  if (tmp < -(float)((int)1 << (Qm + Qn)))
    tmp = -(float)((int)1 << (Qm + Qn));
  return (int)tmp;
}

static float Q_to_floating(int i, int Qm, int Qn)
{
  if (Qn >= 0)
    return ((float)i / (float)((int)1 << Qn));
  if (Qn < 0)
    return ((float)i * (float)((int)1 << -Qn));
  return 0.f;
}

static int floating_to_scale_offset(float f, int Qm, int Qn, float scale, int offset)
{
  float fval = ((f / scale) + offset);
  return floating_to_Q(fval, Qm, Qn);
}

static float scale_offset_to_floating(int f, int Qm, int Qn, float scale, int offset)
{
  float fval = Q_to_floating(f, Qm, Qn);
  float val = (fval - offset) * scale;
  return val;
}

static int Q_to_scale_offset(int f, int Qm_in, int Qn_in, int Qm_out, int Qn_out, float scale, int offset)
{
  float fval = Q_to_floating(f, Qm_in, Qn_in);
  return floating_to_scale_offset(fval, Qm_out, Qn_out, scale, offset);
}

static int scale_offset_to_Q(int f, int Qm_in, int Qn_in, float scale, int offset, int Qm_out, int Qn_out)
{
  float fval = scale_offset_to_floating(f, Qm_in, Qn_in, scale, offset);
  return floating_to_Q(fval, Qm_out, Qn_out);
}

static void dtype_convert_to_QMN(int *dtype, int *Qm, int *Qn, int nbits)
{
  switch (*dtype)
  {
  // case  TENSORINFO_DATATYPE_FLOAT: already taken care of above
  case DataType_UINT8:
  case DataType_INT8:
  case DataType_UINT16:
  case DataType_INT16:
  case DataType_BOOL:
  {
    *dtype = DataType_FXP;
    *Qm = nbits;
    *Qn = 0;
    break;
  }
  // for the following we don't support casting yet FIXME !!!
  case DataType_INT32:
  case DataType_DOUBLE:
  case DataType_UINT32:
  case DataType_FLOAT16:
  case DataType_BFLOAT16:
  case DataType_INT64:
  case DataType_UINT64:
  case DataType_COMPLEX64:
  case DataType_COMPLEX128:
  case DataType_UNDEFINED:
  case DataType_STRING:
    // case  TENSORINFO_DATATYPE_QMN=100,  // ATONN specific
  default:;
  }
}

/**
 * @brief  performs a cast operation to/from Qmn and float
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @retval Error code
 */
int LL_ATON_LIB_Cast(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output, int dma_in,
                     int dma_out)
{
  int Qm_in = input->Qm;
  int Qm_out = output->Qm;
  int Qn_in = input->Qn;
  int Qn_out = output->Qn;
  int Qunsigned_in = input->Qunsigned;
  int Qunsigned_out = output->Qunsigned;
  int dtype_in = input->type;
  int dtype_out = output->type;
  int nbits_in = input->nbits;
  int nbits_out = output->nbits;
  int in_elements = LL_LIB_TENSOR_ELEMENTS(input);
  int out_elements = LL_LIB_TENSOR_ELEMENTS(output);
  int in_bit_size = (input->nbits == 0 ? sizeof(float) * 8 : input->nbits);
  int out_bit_size = (output->nbits == 0 ? sizeof(float) * 8 : output->nbits);
  int in_byte_size = (in_bit_size * in_elements + 7) >> 3;
  int out_byte_size = (out_bit_size * out_elements + 7) >> 3;
  int in_scaleoffset = (input->scale != NULL);
  int out_scaleoffset = (output->scale != NULL);
  float in_scale = in_scaleoffset ? input->scale[0] : 0;
  int8_t in_offset = in_scaleoffset ? input->offset[0] : 0;
  float out_scale = out_scaleoffset ? output->scale[0] : 0;
  int8_t out_offset = out_scaleoffset ? output->offset[0] : 0;

  // LL_ATON_PRINTF("in: type=%d Qm=%d Qn=%d nb=%d\n",dtype_in,Qm_in,Qn_in,nbits_in);
  // LL_ATON_PRINTF("out: type=%d Qm=%d Qn=%d nb=%d\n",dtype_out,Qm_out,Qn_out,nbits_out);
  // we convert integer types to QMN to use the same (inefficient) code
  dtype_convert_to_QMN(&dtype_in, &Qm_in, &Qn_in, nbits_in);
  dtype_convert_to_QMN(&dtype_out, &Qm_out, &Qn_out, nbits_out);
  // LL_ATON_PRINTF("after:\n");
  // LL_ATON_PRINTF("in: type=%d Qm=%d Qn=%d nb=%d\n",dtype_in,Qm_in,Qn_in,nbits_in);
  // LL_ATON_PRINTF("out: type=%d Qm=%d Qn=%d nb=%d\n",dtype_out,Qm_out,Qn_out,nbits_out);

  if (in_elements != out_elements)
    __LL_LIB_ERROR(_ERR_BUFFER, LL_ATON_INVALID_PARAM);

  if (in_byte_size > LL_Buffer_len(input))
    __LL_LIB_ERROR(_ERR_BUFFER_IN, LL_ATON_INVALID_PARAM);

  if (out_byte_size > LL_Buffer_len(output))
    __LL_LIB_ERROR(_ERR_BUFFER_OUT, LL_ATON_INVALID_PARAM);

  if (input->per_channel || output->per_channel)
    __LL_LIB_ERROR(_ERR_BUFFER_OUT, LL_ATON_INVALID_PARAM);

  if (dtype_in == dtype_out &&
      (dtype_in != DataType_FXP ||
       ((Qm_in == Qm_out) && (Qn_in == Qn_out) &&
        (Qunsigned_in == Qunsigned_out)))) // nothing to do here except perhaps copying the input into the output
  {
    if (LL_Buffer_addr_start(input) != LL_Buffer_addr_start(output))
    {
      // LL_ATON_PRINTF("Cast: Just a memcpy\n");
#if _LL_LIB_Concat_Cast_USE_ATON_HW
      __LL_ATON_LIB_DMA_Inputs_Memcpy(input, 1, (void *)LL_Buffer_addr_start(output), in_byte_size, dma_in, dma_out);
#else  // !_LL_LIB_Concat_Cast_USE_ATON_HW
      memcpy((void *)LL_Buffer_addr_start(output), (void *)LL_Buffer_addr_start(input), in_byte_size);
#endif // !_LL_LIB_Concat_Cast_USE_ATON_HW
    }
    // else LL_ATON_PRINTF("Cast: nothing to do\n");
    return LL_ATON_OK;
  }

  if (dtype_in == DataType_FXP && dtype_out == DataType_FLOAT)
  { // from to Qmn and/or scale/offset to float
    int i;
    /* going backward to prevent input clobbering if input buffer=output buffer */
    switch (input->nbits)
    {
    case 8:
    {
      float *out = (float *)LL_Buffer_addr_end(output) - 1;
      int8_t *in = (int8_t *)LL_Buffer_addr_end(input) - 1;
      // LL_ATON_PRINTF("q2f nbits=%d 8: out=%p in=%p in_el=%d\n", input->nbits, out, in, in_elements);
      for (i = 0; i < in_elements; i++)
      {
        int t = (int)*in;
        float f = in_scaleoffset ? scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset)
                                 : Q_to_floating(t, Qm_in, Qn_in);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d out=%p in=%p\n", i, f, t, out, in);
        *out-- = f;
        --in;
      }
    }
    break;
    case 16:
    {
      float *out = (float *)LL_Buffer_addr_end(output) - 1;
      int16_t *in = (int16_t *)LL_Buffer_addr_end(input) - 1;
      // LL_ATON_PRINTF("q2f nbits=%d 16: out=%p in=%p in_el=%d\n", input->nbits, out, in, in_elements);
      for (i = 0; i < in_elements; i++)
      {
        int t = (int)*in;
        float f = in_scaleoffset ? scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset)
                                 : Q_to_floating(t, Qm_in, Qn_in);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        *out-- = f;
        --in;
      }
    }
    break;
    default:
    {
      int nbits = input->nbits;
      int bitcnt = in_bit_size * (in_elements - 1);
      float *out = (float *)LL_Buffer_addr_end(output) - 1;
      uint32_t *in = (uint32_t *)LL_Buffer_addr_start(input);
      // LL_ATON_PRINTF("q2f nbits=%d def: out=%p in=%p in_el=%d\n", input->nbits, out, in, in_elements);
      for (i = 0; i < in_elements; i++)
      {
        int t = LL_ATON_getbits(in, bitcnt, nbits);
        float f = in_scaleoffset ? scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset)
                                 : Q_to_floating(t, Qm_in, Qn_in);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        *out-- = f;
        bitcnt -= nbits;
      }
    }
    }
  }
  else if (dtype_in == DataType_FLOAT && dtype_out == DataType_FXP)
  { // from to float to Qmn and/or scale offset
    int i;
    /* going forward to prevent input clobbering if input buffer=output buffer */
    switch (output->nbits)
    {
    case 8:
    {
      int8_t *out = (int8_t *)LL_Buffer_addr_start(output);
      float *in = (float *)LL_Buffer_addr_start(input);
      // LL_ATON_PRINTF("f2q 8: nbits=%d out=%p in=%p in_el=%d\n", output->nbits, out, in, in_elements);
      for (i = 0; i < in_elements; i++)
      {
        float f = *in;
        int t = out_scaleoffset ? floating_to_scale_offset(f, Qm_out, Qn_out, out_scale, out_offset)
                                : floating_to_Q(f, Qm_out, Qn_out);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        *out++ = (int8_t)t;
        ++in;
      }
    }
    break;
    case 16:
    {
      int16_t *out = (int16_t *)LL_Buffer_addr_start(output);
      float *in = (float *)LL_Buffer_addr_start(input);
      // LL_ATON_PRINTF("f2q 16: nbits=%d out=%p in=%p in_el=%d\n", output->nbits, out, in, in_elements);
      for (i = 0; i < in_elements; i++)
      {
        float f = *in;
        int t = out_scaleoffset ? floating_to_scale_offset(f, Qm_out, Qn_out, out_scale, out_offset)
                                : floating_to_Q(f, Qm_out, Qn_out);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        *out++ = (int16_t)t;
        ++in;
      }
    }
    break;
    default:
    {
      int nbits = input->nbits;
      int bitcnt = 0;
      uint32_t *out = (uint32_t *)LL_Buffer_addr_start(output);
      float *in = (float *)LL_Buffer_addr_start(input);
      // LL_ATON_PRINTF("f2q def: nbits=%d out=%p in=%p in_el=%d\n", output->nbits, out, in, in_elements);
      for (i = 0; i < in_elements; i++)
      {
        float f = *in;
        int t = out_scaleoffset ? floating_to_scale_offset(f, Qm_out, Qn_out, out_scale, out_offset)
                                : floating_to_Q(f, Qm_out, Qn_out);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        LL_ATON_setbits(out, bitcnt, nbits, t);
        bitcnt += nbits;
      }
    }
    }
  }
  else
  {
    // the following code is very inefficient for integer types, specific code should be implemented for those FIXME !!!

    if (dtype_in == DataType_FXP && dtype_out == DataType_FXP)
    {                                    // from to Qmn to Qmn assumes max in/out bits = 16
      int fwd = (nbits_in >= nbits_out); // forward
      int in_bitsinc = fwd ? nbits_in : -nbits_in;
      int out_bitsinc = fwd ? nbits_out : -nbits_out;
      int in_bitcnt = fwd ? 0 : in_bit_size * (in_elements - 1);
      int out_bitcnt = fwd ? 0 : out_bit_size * (out_elements - 1);
      uint32_t *in = (uint32_t *)LL_Buffer_addr_start(input);
      uint32_t *out = (uint32_t *)LL_Buffer_addr_start(output);
      uint32_t tmask = (~(-1 << nbits_out)); //  create a mask with output precision
      int i;
      for (i = 0; i < in_elements; i++)
      {
        int t = LL_ATON_getbits(in, in_bitcnt, nbits_in); // note t is sign extended to int
        int tM = 0;
        if (!in_scaleoffset && !out_scaleoffset)
          tM = (Qn_out >= Qn_in ? (t << (Qn_out - Qn_in)) : (t << (Qn_in - Qn_out))); // align to output mantissa
        if (in_scaleoffset && !out_scaleoffset)
          tM = scale_offset_to_Q(t, Qm_in, Qn_in, in_scale, in_offset, Qm_out, Qn_out);
        if (!in_scaleoffset && out_scaleoffset)
          tM = Q_to_scale_offset(t, Qm_in, Qn_in, Qm_out, Qn_out, out_scale, out_offset);
        if (in_scaleoffset && out_scaleoffset)
        {
          // very inefficient, FIXME
          float fval = in_scaleoffset ? scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset) : t;
          tM = out_scaleoffset ? floating_to_scale_offset(fval, Qm_out, Qn_out, out_scale, out_offset) : (int)fval;
        }
        // extract bits least significant guard bits (if Qm_out < Qm_in) and most significant mantissa
        int tout = (tM & tmask);
        LL_ATON_setbits(out, out_bitcnt, nbits_out, tout);
        in_bitcnt += in_bitsinc;
        out_bitcnt += out_bitsinc;
      }
    }
    else
      __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs a float Softmax (oonx opset >=13) operation on float inputs and output operands according to ONNX
 * semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1)
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int LL_ATON_LIB_Softmax_float(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                                     unsigned int axis)
{
  // int in_batches = input->shape[TDIM_NKERNELS];
  // int in_fwidth = input->shape[TDIM_FWIDTH];
  // int in_fheight = input->shape[TDIM_FHEIGHT];
  // int in_nchannels = input->shape[TDIM_NCHANNELS];
  float *exps = (float *)LL_Buffer_addr_start(output + 1);
  LL_ATON_ASSERT(LL_Buffer_len(output + 1) >= input->shape[axis] * 4);

  int b, o, hw;
  int outer_elem = 1, inner_elem = 1;
  int axis_elem = input->shape[axis];

  for (int i = 0; i < axis; i++)
    outer_elem *= input->shape[i];
  for (int i = axis + 1; i < input->ndims; i++)
    inner_elem *= input->shape[i];

  // LL_ATON_PRINTF("outer_elem=%d inner_eleme=%d axis_elem=%d\n", outer_elem, inner_elem, axis_elem);

  for (b = 0; b < outer_elem; b++)
  {
    int stride = b * inner_elem * axis_elem;
    float *in = (float *)LL_Buffer_addr_start(input) + stride;
    float *out = (float *)LL_Buffer_addr_start(output) + stride;

    for (hw = 0; hw < inner_elem; hw++)
    {
      float exp_sum = 0.f;
      // compute max
      float maxf = in[0];
      for (o = 0; o < axis_elem * inner_elem; o += inner_elem)
        maxf = (maxf < in[o] ? in[o] : maxf);
      // compute sum of exps
      int oi = 0;
      for (o = 0; o < axis_elem * inner_elem; o += inner_elem, oi++)
      {
        float f = expf(in[o] - maxf);
        exps[oi] = f;
        exp_sum += f;
      }
      exp_sum = 1.0f / exp_sum;
      // exp_sum = maxf + log(exp_sum);
      // normalize
      oi = 0;
      for (o = 0; o < axis_elem * inner_elem; o += inner_elem, oi++)
      {
        // out[o] = exp(in[o] - exp_sum);
        out[o] = exps[oi] * exp_sum;
        // LL_ATON_PRINTF("%g %x", out[o],out+o);
      }
      // in += axis_elem * inner_elem;
      // out += axis_elem * inner_elem;
      in++;
      out++;
    }
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs an INT8 (scale/offset) Softmax (oonx opset >=13) operation inputs and output operands according to
 * ONNX semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1)
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int LL_ATON_LIB_Softmax_INT8(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                                    unsigned int axis)
{
  int b, o, hw;

  int outer_elem = 1, inner_elem = 1;
  int axis_elem = input->shape[axis];

  for (int i = 0; i < axis; i++)
    outer_elem *= input->shape[i];
  for (int i = axis + 1; i < input->ndims; i++)
    inner_elem *= input->shape[i];

  double scalein = (double)input->scale[0];
  float scaleout = output->scale[0];
  int off = output->offset[0];
  float *exps = (float *)LL_Buffer_addr_start(output + 1);
  LL_ATON_ASSERT(LL_Buffer_len(output + 1) >= 512 * 4);

  for (b = -256; b <= 255; b++)
  {
    float f;
    f = exp(b * scalein);
#if 0 // is this necessary ?
    if (isnanf(f) || isinff(f))  {
         f = b < 0 ? 0 : (b > 0 ? FLT_MAX : b);
    }
#endif
    exps[b + 256] = f; // encoding 0:127 -> 0:127 and -128:-1 -> 128-255 to save one addition later on
    // LL_ATON_PRINTF("b=%d f=%g\n", b + 256, f);
  }

  for (b = 0; b < outer_elem; b++)
  {
    int stride = b * inner_elem * axis_elem;
    int8_t *in = (int8_t *)LL_Buffer_addr_start(input) + stride;
    int8_t *out = (int8_t *)LL_Buffer_addr_start(output) + stride;

    for (hw = 0; hw < inner_elem; hw++)
    {
      float exp_sum = 0.f;

      int maxb = -128;
      for (o = 0; o < axis_elem * inner_elem; o += inner_elem)
        maxb = (maxb < in[o] ? in[o] : maxb);
      maxb -= 256;
      // LL_ATON_PRINTF("maxb = %d\n", maxb);

      for (o = 0; o < axis_elem * inner_elem; o += inner_elem)
      {
        exp_sum += exps[in[o] - maxb];
        // LL_ATON_PRINTF("in[o]=%d idx=%d val=%g exp_sum=%g\n", in[o], in[o] - maxb + 256, exps[in[o] - maxb + 256],
        // exp_sum);
      }
      // LL_ATON_PRINTF("exp_sum=%g\n", exp_sum);

      exp_sum *= scaleout;
      float inv_exp_sum = 1.0f / exp_sum;

      for (o = 0; o < axis_elem * inner_elem; o += inner_elem)
      {
        float t = exps[in[o] - maxb];
        t = (t * inv_exp_sum + off);
        int ti = (t > 0 ? (int)(t + 0.5f) : (int)(t - 0.5f));
        ti = (t > 127 ? 127 : (t < -128 ? -128 : ti));
        out[o] = (int8_t)ti;
        // LL_ATON_PRINTF("%g %x", out[o],out+o);
      }
      in++;
      out++;
    }
  }

  return LL_ATON_OK;
}
/**
 * @brief  performs a float Softmax (oonx opset < 13) operation on float inputs and output operands according to ONNX
 * semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1) with input tensor coerced to
 * 2D by collapsing dimensions before and after axis
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int LL_ATON_LIB_Softmax_float_legacy(const LL_LIB_TensorInfo_TypeDef *input,
                                            const LL_LIB_TensorInfo_TypeDef *output, unsigned int axis)
{
  // this function must assume shape to be described as an BCHW for the purpose of computing the softmax
  // while actual memory storage is BHWC
  // note that ndim MUST be always >= 4 when invoking the function (for dims < 4 must be adding extra dimensions = 1)
  int start_dim = input->ndims - 4;
  // int in_batches = input->shape[start_dim + TDIM_NKERNELS];
  int in_fwidth = input->shape[start_dim + TDIM_FWIDTH];
  int in_fheight = input->shape[start_dim + TDIM_FHEIGHT];
  int in_nchannels = input->shape[start_dim + TDIM_NCHANNELS];

  int b, o, left;
  int outer_elem = 1, inner_elem = 1, left_elem = 1;
  int dim_lut[3] = {1, 2, 0}; // HWC -> CHW (1,2,0)

  int alternate_axis = -1;
  if (axis > start_dim)
    alternate_axis = 1 + dim_lut[(axis - start_dim - 1)];
  // alternate_axis = 1 C inn = H*W*C, left = 1
  // alternate_axis = 2 H inn = H*W, left = C
  // alternate_axis = 3 W inn = W, left = C
  switch (alternate_axis)
  {
  case 1:
    inner_elem = in_nchannels * in_fheight * in_fwidth;
    left_elem = 1;
    break;
  case 2:
    inner_elem = in_fheight * in_fwidth;
    left_elem = in_nchannels;
    break;
  case 3:
    inner_elem = in_fwidth;
    left_elem = in_nchannels;
    break;
  default:
    for (int i = axis; i < input->ndims; i++)
      inner_elem *= input->shape[i];
  }

  // LL_ATON_PRINTF("start_dim=%d axis=%d altern_axis=%d\n", start_dim, axis, alternate_axis);

  for (int i = 0; i < input->ndims; i++)
    outer_elem *= input->shape[i];
  outer_elem /= inner_elem * left_elem;

  // LL_ATON_PRINTF("inner elem=%d outer_elem=%d left_elem=%d\n", inner_elem, outer_elem, left_elem);

  for (left = 0; left < left_elem; left++)
    for (b = 0; b < outer_elem; b++)
    {
      int stride = b * inner_elem * left_elem + left;
      float *in = (float *)LL_Buffer_addr_start(input) + stride;
      float *out = (float *)LL_Buffer_addr_start(output) + stride;

      float exp_sum = 0.f;
      // compute max
      float maxf = in[0];
      for (o = 0; o < left_elem * inner_elem; o += left_elem)
      {
        // LL_ATON_PRINTF("in: %g %p\n", in[o], (in + o));
        maxf = (maxf < in[o] ? in[o] : maxf);
      }
      // LL_ATON_PRINTF("maxf=%g\n", maxf);
      //  compute sum of exps
      for (o = 0; o < left_elem * inner_elem; o += left_elem)
      {
        float f = expf(in[o] - maxf);
        exp_sum += f;
      }
      // LL_ATON_PRINTF("exp_sum=%g\n", exp_sum);
      exp_sum = maxf + logf(exp_sum);
      // LL_ATON_PRINTF("exp_sum=%g\n", exp_sum);
      //  normalize
      for (o = 0; o < left_elem * inner_elem; o += left_elem)
      {
        out[o] = expf(in[o] - exp_sum);
        // LL_ATON_PRINTF("out:%g %g %p\n", in[o], out[o], (out + o));
      }
      in += left_elem;
      out += left_elem;
    }

  return LL_ATON_OK;
}

/**
 * @brief  performs an INT8 (scale/offset) Softmax (oonx opset >=13) operation inputs and output operands according to
 * ONNX semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1) with input tensor coerced to
 * 2D by collapsing dimensions before and after axis
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int LL_ATON_LIB_Softmax_INT8_legacy(const LL_LIB_TensorInfo_TypeDef *input,
                                           const LL_LIB_TensorInfo_TypeDef *output, unsigned int axis)
{
  int start_dim = input->ndims - 4;
  int in_fwidth = input->shape[start_dim + TDIM_FWIDTH];
  int in_fheight = input->shape[start_dim + TDIM_FHEIGHT];
  int in_nchannels = input->shape[start_dim + TDIM_NCHANNELS];

  int b, o, left;
  int outer_elem = 1, inner_elem = 1, left_elem = 1;
  int dim_lut[3] = {1, 2, 0};

  int alternate_axis = -1;
  if (axis > start_dim)
    alternate_axis = 1 + dim_lut[(axis - start_dim - 1)];
  // alternate_axis = 1 C inn = H*W*C, left = 1
  // alternate_axis = 2 H inn = H*W, left = C
  // alternate_axis = 3 W inn = W, left = C
  switch (alternate_axis)
  {
  case 1:
    inner_elem = in_nchannels * in_fheight * in_fwidth;
    left_elem = 1;
    break;
  case 2:
    inner_elem = in_fheight * in_fwidth;
    left_elem = in_nchannels;
    break;
  case 3:
    inner_elem = in_fwidth;
    left_elem = in_nchannels;
    break;
  default:
    for (int i = axis; i < input->ndims; i++)
      inner_elem *= input->shape[i];
  }

  // LL_ATON_PRINTF("start_dim=%d axis=%d altern_axis=%d\n", start_dim, axis, alternate_axis);

  for (int i = 0; i < input->ndims; i++)
    outer_elem *= input->shape[i];
  outer_elem /= inner_elem * left_elem;

  // LL_ATON_PRINTF("inner elem=%d outer_elem=%d left_elem=%d\n", inner_elem, outer_elem, left_elem);

  double scalein = (double)input->scale[0];
  float scaleout = output->scale[0];
  int off = output->offset[0];
  float *exps = (float *)LL_Buffer_addr_start(output + 1);
  LL_ATON_ASSERT(LL_Buffer_len(output + 1) >= 512 * 4);

  for (b = -256; b <= 255; b++)
  {
    float f;
    f = exp(b * scalein);
#if 0 // is this necessary ?
    if (isnanf(f) || isinff(f))  {
         f = b < 0 ? 0 : (b > 0 ? FLT_MAX : b);
    }
#endif
    exps[b + 256] = f; // encoding 0:127 -> 0:127 and -128:-1 -> 128-255 to save one addition later on
    // LL_ATON_PRINTF("b=%d f=%g\n", b + 256, f);
  }

  for (left = 0; left < left_elem; left++)
    for (b = 0; b < outer_elem; b++)
    {
      int stride = b * inner_elem * left_elem + left;
      int8_t *in = (int8_t *)LL_Buffer_addr_start(input) + stride;
      int8_t *out = (int8_t *)LL_Buffer_addr_start(output) + stride;

      float exp_sum = 0.f;

      int maxb = -128;
      for (o = 0; o < left_elem * inner_elem; o += left_elem)
        maxb = (maxb < in[o] ? in[o] : maxb);
      maxb -= 256;
      // LL_ATON_PRINTF("maxb = %d\n", maxb);

      for (o = 0; o < left_elem * inner_elem; o += left_elem)
      {
        exp_sum += exps[in[o] - maxb];
        // LL_ATON_PRINTF("in[o]=%d idx=%d val=%g exp_sum=%g\n", in[o], in[o] - maxb + 256, exps[in[o] - maxb + 256],
        // exp_sum);
      }
      // LL_ATON_PRINTF("exp_sum=%g\n", exp_sum);

      exp_sum *= scaleout;
      float inv_exp_sum = 1.0f / exp_sum;

      for (o = 0; o < left_elem * inner_elem; o += left_elem)
      {
        float t = exps[in[o] - maxb];
        t = (t * inv_exp_sum + off);
        int ti = (t > 0 ? (int)(t + 0.5f) : (int)(t - 0.5f));
        ti = (t > 127 ? 127 : (t < -128 ? -128 : ti));
        out[o] = (int8_t)ti;
        // LL_ATON_PRINTF("out:%d %d %p\n", in[o], out[o], (out + o));
        //  LL_ATON_PRINTF("%g %x", out[o],out+o);
      }
      in += left_elem;
      out += left_elem;
    }

  return LL_ATON_OK;
}

int LL_ATON_LIB_Softmax(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                        unsigned int axis, int legacy)
{
  int in_elements = LL_LIB_TENSOR_ELEMENTS(input);
  int out_elements = LL_LIB_TENSOR_ELEMENTS(output);
  int el_size = input->type == DataType_FLOAT ? 4 : 1;
  int in_byte_size = (in_elements * el_size * 8) >> 3;
  int out_byte_size = (out_elements * el_size * 8) >> 3;

  // if (axis != 1) // for now we only support axis = 1 FIXME !!!
  //   __LL_LIB_ERROR(_ERR_AXIS, LL_ATON_INVALID_PARAM);

  if (in_elements != out_elements)
    __LL_LIB_ERROR(_ERR_BUFFER, LL_ATON_INVALID_PARAM);

#if 0
  if ((input->Qm + input->Qn) != 0 || (output->Qm + output->Qn) != 0) // must be float
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
#else
  if (input->type != output->type ||
      (input->type != DataType_FLOAT && input->type != DataType_INT8)) // must be float or INT8
    __LL_LIB_ERROR(_ERR_DATATYPE, LL_ATON_INVALID_PARAM);
#endif

  if (in_byte_size > LL_Buffer_len(input))
    __LL_LIB_ERROR(_ERR_BUFFER_IN, LL_ATON_INVALID_PARAM);

  if (out_byte_size > LL_Buffer_len(output))
    __LL_LIB_ERROR(_ERR_BUFFER_OUT, LL_ATON_INVALID_PARAM);

  if (input->ndims < 4)
    __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);

  if (output->ndims < 4)
    __LL_LIB_ERROR(_ERR_SHAPE_OUT, LL_ATON_INVALID_PARAM);

  if (input->type == DataType_INT8)
  {
    if (input->per_channel)
      __LL_LIB_ERROR(_ERR_DATATYPE, LL_ATON_INVALID_PARAM);
    return legacy ? LL_ATON_LIB_Softmax_INT8_legacy(input, output, axis)
                  : LL_ATON_LIB_Softmax_INT8(input, output, axis);
  }
  if (input->type == DataType_FLOAT)
  {
    return legacy ? LL_ATON_LIB_Softmax_float_legacy(input, output, axis)
                  : LL_ATON_LIB_Softmax_float(input, output, axis);
  }

  return LL_ATON_INVALID_PARAM;
}

/**
 * @brief  performs flat copy operation on an input and several outputs using DMA
 * @param  input tensor shape structure
 * @param  outputs tensor shape structures
 * @param  nr_of_outputs number of output tensors
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_in DMA number of DMA writing to memory
 * @retval Error code
 */
int LL_ATON_LIB_DMA_Outputs_Flat_Copy(const LL_LIB_TensorShape_TypeDef *input,
                                      const LL_LIB_TensorShape_TypeDef *outputs, unsigned int nr_of_outputs, int dma_in,
                                      int dma_out)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned i = 0; i < nr_of_outputs; i++)
  {
    output_size += LL_Buffer_len(outputs + i);
  }

  if (input_size < output_size)
  { // should never happen
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }
#endif // !NDEBUG

  if (nr_of_outputs <= 0)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  if (nr_of_outputs > __LL_MAX_TENSORS)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  __LL_ATON_LIB_DMA_Outputs_Memcpy(input, outputs, nr_of_outputs, dma_in, dma_out);

  return LL_ATON_OK;
}

/**
 * @brief  perform split-like slice operation using DMAs
 * @param  input tensor shape structure
 * @param  outputs tensor shape structures
 * @param  tot_out_size size of output buffer
 * @param  width_in_bytes number of bytes per `memcpy`
 * @param  fheight DMA `fheight` field
 * @param  line_offset DMA `line_offset` field
 * @param  n_bits DMA channel size
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_in DMA number of DMA writing to memory
 * @return Error code
 */
int LL_ATON_LIB_DMA_Outputs_Slice_SplitLike(const LL_LIB_TensorShape_TypeDef *input,
                                            const LL_LIB_TensorShape_TypeDef *output, int32_t tot_out_size,
                                            int32_t width_in_bytes, int32_t fheight, int32_t line_offset, int8_t n_bits,
                                            int dma_in, int dma_out)
{
  // Do actual copy
  if (tot_out_size < __LL_DMA_MIN_BUFF_LEN)
  {
    for (unsigned source = 0, dest = 0; dest < tot_out_size; source += line_offset, dest += width_in_bytes)
    {
      // LL_ATON_PRINTF("dest=%d, source=%d\n", dest, source);
      memcpy(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start(output) + dest),
             ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR(LL_Buffer_addr_start(input) + source), width_in_bytes);
    }
  }
  else
  {
    /* prepare epoch */
    LL_ATON_ASSERT((tot_out_size % width_in_bytes) == 0);

    LL_Streng_TensorInitTypeDef _dma_in = {.addr_base.i = input->addr_base.i,
                                           .offset_start = input->offset_start,
                                           .offset_end = input->offset_start + width_in_bytes,
                                           .offset_limit = input->offset_limit,

                                           .dir = 0,
                                           .raw = 1,

                                           .nbits_in = n_bits,
                                           .nbits_out = n_bits,

                                           .frame_offset = line_offset,

                                           .frame_tot_cnt = fheight,
                                           .frame_loop_cnt = 0};

    LL_Streng_TensorInitTypeDef _dma_out = {.addr_base.i = output->addr_base.i,
                                            .offset_start = output->offset_start,
                                            .offset_end = output->offset_end,

                                            .dir = 1,
                                            .raw = 1,

                                            .nbits_in = n_bits,
                                            .nbits_out = n_bits,

                                            .frame_tot_cnt = 1,
                                            .frame_loop_cnt = 0};

    /* save DMA configurations */
    __ll_lib_params_t *params = __ll_lib_get_params();
    params->g_dma_in = _dma_in;
    params->g_dma_out = _dma_out;

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, _slice_split_like_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(_slice_split_like_epoch_block_array);
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs channel-split copy operation on an input and several outputs (both in ATON canonical format) using
 * DMA
 * @param  input tensor shape structure
 * @param  outputs tensor shape structures
 * @param  nr_of_outputs number of output tensors
 * @retval Error code
 */
int LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(const LL_LIB_TensorShape_TypeDef *input,
                                               const LL_LIB_TensorShape_TypeDef *outputs, unsigned int nr_of_outputs,
                                               unsigned int leading_dims, int dma_in, int dma_out)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned i = 0; i < nr_of_outputs; i++)
  {
    output_size += LL_Buffer_len(outputs + i);
  }

  if (input_size != output_size)
  { // should never happen
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }
#endif // !NDEBUG

  if (nr_of_outputs <= 0)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  if (nr_of_outputs > __LL_MAX_TENSORS)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(input, outputs, nr_of_outputs, leading_dims, dma_in, dma_out);

  return LL_ATON_OK;
}

/**
 * @brief  performs a channel-split memory copy operation from one input (ATON canonical) to `noutputs`
 * non-ATON-canonical outputs using stream engines `dma_in` and `dma_out`
 * @param  src source address
 * @param  outputs list of output tensor shape structures
 * @param  noutputs number of outputs
 * @retval Error code
 */
int LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(const LL_LIB_TensorShape_TypeDef *input,
                                                  const LL_LIB_TensorShape_TypeDef *outputs, unsigned int nr_of_outputs,
                                                  int dma_in, int dma_out)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned i = 0; i < nr_of_outputs; i++)
  {
    output_size += LL_Buffer_len(outputs + i);
  }

  if (input_size != output_size)
  { // should never happen
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
  }
#endif // !NDEBUG

  if (nr_of_outputs <= 0)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  if (nr_of_outputs > __LL_MAX_TENSORS)
  { // should never happen
    __LL_LIB_ERROR(_ERR_NOUTPUTS, LL_ATON_INVALID_PARAM);
  }

  __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(input, outputs, nr_of_outputs, dma_in, dma_out);

  return LL_ATON_OK;
}

/* `memset` helper functions */
#define __LL_DMA_INTERNAL_BUSPORT_WIDTH 8 // MUST correspond to value of `BUSPORT_DATA_W` in Verilog file `ipu_def.vpp`

static inline uint32_t __ll_lib_match_preload_with_busport(size_t size, uint8_t nbytes, uint32_t *min_bytes_to_preload)
{
  uint32_t _min_bytes_to_preload = *min_bytes_to_preload;
  uint32_t max_nr_preloads_in_busport = (__LL_DMA_INTERNAL_BUSPORT_WIDTH / _min_bytes_to_preload);

  unsigned nr_of_samples = 2;
  uint32_t last_aligned_size = size % _min_bytes_to_preload;
  for (; nr_of_samples <= max_nr_preloads_in_busport; nr_of_samples++)
  {
    uint32_t copy_samples_in_bytes = (nr_of_samples * _min_bytes_to_preload);
    uint32_t aligned_size = size % copy_samples_in_bytes;
    LL_ATON_ASSERT((aligned_size % nbytes) == 0);

    if ((aligned_size + copy_samples_in_bytes) > __LL_DMA_INTERNAL_BUSPORT_WIDTH)
    {
      break;
    }

    last_aligned_size = aligned_size;
  }
  nr_of_samples--;

  uint32_t samples_to_copy_from_in_bytes = (nr_of_samples * _min_bytes_to_preload);
  uint32_t bytes_to_preload = (last_aligned_size + samples_to_copy_from_in_bytes);

  LL_ATON_ASSERT(bytes_to_preload <= __LL_DMA_INTERNAL_BUSPORT_WIDTH);
  /* LL_ATON_ASSERT(bytes_to_preload <= size); */ // always guaranteed as `size >= __LL_DMA_INTERNAL_BUSPORT_WIDTH`)
  LL_ATON_ASSERT((bytes_to_preload % nbytes) == 0);

  *min_bytes_to_preload = samples_to_copy_from_in_bytes;

  // return number of bytes to pre-load
  return bytes_to_preload;
}

static inline void __ll_lib_load_const_val(void **dst, int32_t constant_value, size_t size, uint8_t nbytes)
{
  uint8_t *_dst_orig = *dst;

  switch (nbytes)
  {
  case 1:
  { // 8-bit
    int8_t **_dst = (int8_t **)dst;
    int i = 0;
    for (; i < size; i++)
    {
      **_dst = (int8_t)constant_value;
      (*_dst)++;
    }
  }
  break;
  case 2:
  { // 16-bit
    int16_t **_dst = (int16_t **)dst;
    int i = 0;
    for (; i < size; i += 2)
    {
      **_dst = (int16_t)constant_value;
      (*_dst)++;
    }
  }
  break;
  case 3:
  { // 24-bit
    int8_t **_dst = (int8_t **)dst;
    for (int i = 0; i < size; i += 3)
    {
      **_dst = constant_value & 0xFF;
      (*_dst)++;
      **_dst = (constant_value >> 8) & 0xFF;
      (*_dst)++;
      **_dst = (constant_value >> 16) & 0xFF;
      (*_dst)++;
    }
  }
  break;
  case 4:
  { // 32-bit
    int32_t **_dst = (int32_t **)dst;
    int i = 0;
    for (; i < size; i += 4)
    {
      **_dst = (int32_t)constant_value;
      (*_dst)++;
    }
  }
  break;
  default:
    LL_ATON_ASSERT(0); // should never happen!!!
    return;
  }

  if (size > 0)
  {
    /* *** MCU cache clean & invalidate operation (SW) *** */
    LL_ATON_Cache_MCU_Clean_Invalidate_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)_dst_orig), size);
  }
}

/* NOTE: function assumes that `size`, `*dst`, & `min_bytes_to_preload` are correctly aligned */
static inline size_t __ll_lib_memset_prolog(void **dst, int32_t constant_value, size_t size, uint8_t nbytes,
                                            uint32_t *min_bytes_to_preload)
{
#if (__LL_DMA_INTERNAL_BUSPORT_WIDTH < 6)
  /* `6` comes from the worst case scenario of 16bits samples with minimum of 4 bytes preload (`4 bytes +  2 bytes` of
   * max rest) */
#error "`__LL_DMA_INTERNAL_BUSPORT_WIDTH` is too small for current (slightly optimized) version of DMA-based `memset`!"
#endif

  if (size < (__LL_DMA_MIN_BUFF_LEN + __LL_DMA_INTERNAL_BUSPORT_WIDTH))
  {
    /* it's not worth it ... */
    __ll_lib_load_const_val(dst, constant_value, size, nbytes);

    return 0;
  }
  else
  {
    uint32_t nr_of_bytes_to_pre_load = __ll_lib_match_preload_with_busport(size, nbytes, min_bytes_to_preload);
    /* pre-load first samples */
    __ll_lib_load_const_val(dst, constant_value, nr_of_bytes_to_pre_load, nbytes);
    size -= nr_of_bytes_to_pre_load;

    return size;
  }
}

static bool __ll_lib_memset(void *dst, void *dst_limit, int32_t constant_value, uint8_t nbytes, size_t size)
{
  unsigned char *dst_orig = (unsigned char *)dst;
  uint32_t samples_in_bytes_preloaded = 0;
  int nbits = 0;

  LL_ATON_ASSERT((size % nbytes) == 0);

  switch (nbytes)
  {
  case 1:
  case 3:
    samples_in_bytes_preloaded = 3; // max `nbytes` & equal to nr of channels used (`#channels_used == 3`
    nbits = 24;                     // `#channels_used * 8`
    break;
  case 2:
  case 4:
    LL_ATON_ASSERT((((intptr_t)dst) % nbytes) == 0);
    samples_in_bytes_preloaded = 4; // max `nbytes` & multiple of nr of channels used (`#channels_used == 2`)
    nbits = 16;                     // `#channels_used * 8`
    break;
  default:
    LL_ATON_ASSERT(0); // should never happen!!!
    return false;
  }

  /* pre-load destination & align `dst` to value of `samples_in_bytes_preloaded` */
  size = __ll_lib_memset_prolog(&dst, constant_value, size, nbytes, &samples_in_bytes_preloaded);
  if (size == 0)
  {
    return false; // we are done
  }

  /* setup configuration for DMAs */
  LL_ATON_ASSERT((size % samples_in_bytes_preloaded) == 0);
  int frame_tot_cnt = (size / samples_in_bytes_preloaded);

  LL_Streng_TensorInitTypeDef _dma_in = {.dir = 0,
                                         .addr_base = {dst_orig},
                                         .offset_start = 0,
                                         .offset_end = samples_in_bytes_preloaded,
                                         .offset_limit =
                                             (unsigned char *)dst_limit - dst_orig, /* awful FIXME Francesco */
                                         .raw = 1,
                                         .noinc = 1,
                                         .frame_tot_cnt = frame_tot_cnt,
                                         .nbits_in = nbits,
                                         .nbits_out = nbits,
                                         .nbits_unsigned = 0};

  LL_Streng_TensorInitTypeDef _dma_out = {.dir = 1,
                                          .addr_base = {dst},
                                          .offset_start = 0,
                                          .offset_end = size,
                                          .raw = 1,
                                          .frame_tot_cnt = 1,
                                          .nbits_in = nbits,
                                          .nbits_out = nbits,
                                          .nbits_unsigned = 0};

  /* save DMA configurations */
  __ll_lib_params_t *params = __ll_lib_get_params();
  params->g_dma_in = _dma_in;
  params->g_dma_out = _dma_out;

  return true;
}

static inline void __ll_lib_pad_save_params(__ll_pad_sw_params_t *common_params)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  void *lower_heap = __ll_lib_get_lower_heap();

  /* flat copy of params */
  params->special.pad = *common_params;

  /* prepare for deep copy of vectors */
  uint32_t *min_shape = (uint32_t *)lower_heap;
  int32_t *pad_in_offsets_start = (int32_t *)(min_shape + common_params->tensor_rank);
  int32_t *pad_in_offsets_end = (int32_t *)(pad_in_offsets_start + common_params->tensor_rank);
  int32_t *pad_out_offsets_start = (int32_t *)(pad_in_offsets_end + common_params->tensor_rank);
  int32_t *pad_out_offsets_end = (int32_t *)(pad_out_offsets_start + common_params->tensor_rank);
  int32_t *out_shape = (int32_t *)(pad_out_offsets_end + common_params->tensor_rank);
  int32_t *out_offsets = (int32_t *)(out_shape + common_params->tensor_rank);
  uint32_t *indexes = (uint32_t *)(out_offsets + common_params->tensor_rank);

  /* copy vectors to lower heap & overwrite pointers */
  for (uint32_t i = 0; i < common_params->tensor_rank; i++)
  {
    min_shape[i] = common_params->min_shape[i];
    pad_in_offsets_start[i] = common_params->pad_in_offsets_start[i];
    pad_in_offsets_end[i] = common_params->pad_in_offsets_end[i];
    pad_out_offsets_start[i] = common_params->pad_out_offsets_start[i];
    pad_out_offsets_end[i] = common_params->pad_out_offsets_end[i];
    out_shape[i] = common_params->out_shape[i];
    out_offsets[i] = common_params->out_offsets[i];
    indexes[i] = 0;
  }
  params->special.pad.min_shape = min_shape;
  params->special.pad.pad_in_offsets_start = pad_in_offsets_start;
  params->special.pad.pad_in_offsets_end = pad_in_offsets_end;
  params->special.pad.pad_out_offsets_start = pad_out_offsets_start;
  params->special.pad.pad_out_offsets_end = pad_out_offsets_end;
  params->special.pad.out_shape = out_shape;
  params->special.pad.out_offsets = out_offsets;
  params->special.pad.indexes = indexes;
}

/**
 * @brief  performs an optimized `memset` for the `Pad` operator using DMA (aka Framing)
 * @param  output destination address of `memset` operation
 * @param  constant_value constant value to be set
 * @param  out_size number of bytes to output
 * @param  common_params parameters needed to setup DMAs and to forward to eventual callback function
 * @retval Error code
 */
int LL_ATON_LIB_DMA_Pad_Memset(void *output, int32_t constant_value, size_t out_size,
                               __ll_pad_sw_params_t *common_params)
{
  /* save common parameters */
  __ll_lib_pad_save_params(common_params);

  /* start operation */
  bool ret = __ll_lib_memset(output, common_params->out_limit, constant_value, common_params->nbytes, out_size);

  if (ret)
  {
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): performing DMA based `memset`\n", __func__, __LINE__);
#endif

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(common_params->dma_in, common_params->dma_out, _dma_Pad_memset_epoch_block_array);

    /* start DMAs for `memset` & run `LL_ATON_LIB_Pad_Filling()` */
    LL_ATON_RT_Insert_LibEpochBlockArray(_dma_Pad_memset_epoch_block_array);
  }
  else
  {
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): performing pure SW `memset`\n", __func__, __LINE__);
#endif

    __ll_lib_params_t *params = __ll_lib_get_params();

    /* `memset` already done => run callback function (if any) */
    if (params->special.pad.callback_function != NULL)
    {
      /* call follow-up function */
      return (*params->special.pad.callback_function)(&params->special.pad);
    }
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs HW accelerated filling operation for `Pad` operator (aka Filling)
 * @retval Error code
 */
int LL_ATON_LIB_DMA_Pad_Filling(__ll_pad_sw_params_t *init_common_params)
{
  /* save common parameters */
  if (init_common_params != NULL)
  {
    __ll_lib_pad_save_params(init_common_params);
  }

  /* get common parameters */
  __ll_lib_params_t *params = __ll_lib_get_params();
  __ll_pad_sw_params_t *common_params = &params->special.pad;

  /* prepare epoch */
  params->g_dma_in = _static_const_dma_in;
  params->g_dma_out = _static_const_dma_out;

  /* `__ll_lib_inputs_memcpy_start()` requires use of generic `size` parameter */
  params->g_size = common_params->consecutive_bytes;

  for (params->g_idx = 0; params->g_idx <= common_params->consecutive_axis; params->g_idx++)
  {
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): in=%lx, out=%lx, curr_axis=%u, min_dim=%u, in_start=%d, out_start=%d\n", __func__, __LINE__,
                   (uintptr_t)common_params->in_target, (uintptr_t)common_params->out_target, params->g_idx,
                   common_params->min_shape[params->g_idx], common_params->pad_in_offsets_start[params->g_idx],
                   common_params->pad_out_offsets_start[params->g_idx]);
#endif

    if (common_params->pad_out_offsets_start[params->g_idx] > 0)
    {
      common_params->out_target += common_params->pad_out_offsets_start[params->g_idx];
    }

    if (common_params->pad_in_offsets_start[params->g_idx] < 0)
    {
      common_params->in_target -= common_params->pad_in_offsets_start[params->g_idx];
    }
  }

  params->g_idx = common_params->consecutive_axis;

  /* configure stream switch */
  __ll_lib_strswitch_set_dmas(common_params->dma_in, common_params->dma_out, _dma_Pad_filling_epoch_block_array);

  /* start DMAs for filling `consecutive bytes` */
  LL_ATON_RT_Insert_LibEpochBlockArray(_dma_Pad_filling_epoch_block_array);

  return LL_ATON_OK;
}
