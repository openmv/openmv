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

// if set it will perform CHECKDISTANCE with COSINE_SIMILARITY and not with MAXDISTANCE
#define USE_COSINE_SIMILARITY
// CHECKDISTANCE mode thresholds
#define MAXDISTANCE_TH       3
#define COSINE_SIMILARITY_TH 0.9

/* replace these with ARM ISA instructions and assembly intrinsics */
static int __floating_to_Q(float f, int Qm, int Qn)
{
  float tmp;
  if (Qn >= 0)
    tmp = (f * ((long long)1 << Qn) + (f > (float)0 ? (float)0.5 : (float)-0.5));
  if (Qn < 0)
    tmp = (f * (float)1 / ((long long)1 << -Qn));
  if (tmp > (float)(((long long)1 << (Qm + Qn)) - 1))
    tmp = (float)(((long long)1 << (Qm + Qn)) - 1);
  if (tmp < -(float)((long long)1 << (Qm + Qn)))
    tmp = -(float)((long long)1 << (Qm + Qn));
  return (int)tmp;
}

static float __Q_to_floating(int i, int Qm, int Qn)
{
  LL_ATON_LIB_UNUSED(Qm);

  if (Qn >= 0)
    return ((float)i / (float)((long long)1 << Qn));
  if (Qn < 0)
    return ((float)i * (float)((long long)1 << -Qn));
  return 0.f;
}

static int __floating_to_scale_offset(float f, int Qm, int Qn, float scale, int offset)
{
  float fval = ((f / scale) + offset);
  return __floating_to_Q(fval, Qm, Qn);
}

static float __scale_offset_to_floating(int f, int Qm, int Qn, float scale, int offset)
{
  float fval = __Q_to_floating(f, Qm, Qn);
  float val = (fval - offset) * scale;
  return val;
}

static int __Q_to_scale_offset(int f, int Qm_in, int Qn_in, int Qm_out, int Qn_out, float scale, int offset)
{
  float fval = __Q_to_floating(f, Qm_in, Qn_in);
  return __floating_to_scale_offset(fval, Qm_out, Qn_out, scale, offset);
}

static int __scale_offset_to_Q(int f, int Qm_in, int Qn_in, float scale, int offset, int Qm_out, int Qn_out)
{
  float fval = __scale_offset_to_floating(f, Qm_in, Qn_in, scale, offset);
  return __floating_to_Q(fval, Qm_out, Qn_out);
}

static void __dtype_convert_to_QMN(int *dtype, unsigned int *Qm, unsigned int *Qn, unsigned int nbits)
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

/*** Heap for hybrid operator implementations ***/
static uint32_t __ll_lib_heap[__LL_LIB_HEAP_SIZE]; // REMEMBER: static variables are not suited for
                                                   // multithreaded etc. environments
/** Static constants **/
static LL_Switch_InitTypeDef __ll_switch_init[] = {
    {LL_Switch_Init_Dest() = ATONN_DSTPORT(STRSWITCH, 0, STRENG, 1, 0),
     LL_Switch_Init_Source(0) = ATONN_SRCPORT(STRSWITCH, 0, STRENG, 0, 0), LL_Switch_Init_Context(0) = 1,
     LL_Switch_Init_Frames(0) = 0}};
static int __ll_lib_dma_unit_id[] = {1, 0}; /* {<dma_out>, <dma_in>} */
static LL_ATON_EnableUnits_InitTypeDef __ll_lib_dma_units[] = {{{STRENG, 1}}, {{STRENG, 0}}};
static const LL_Streng_TensorInitTypeDef __ll_lib_static_const_dma_in = {
    .dir = 0, .raw = 1, .frame_tot_cnt = 1, .nbits_in = 24, .nbits_out = 24};
static const LL_Streng_TensorInitTypeDef __ll_lib_static_const_dma_out = {
    .dir = 1, .raw = 1, .frame_tot_cnt = 1, .nbits_in = 24, .nbits_out = 24};

/** Helper function(s) **/
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
#endif // !NDEBUG
}
#endif // _LL_LIB_DEBUG

static inline __ll_lib_params_t *__ll_lib_get_params(void)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
#endif // !NDEBUG

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
  LL_ATON_LIB_UNUSED(dma_in);
  LL_ATON_LIB_UNUSED(dma_out);

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("===\n");
  LL_ATON_PRINTF("dma_in: %d, dma_out: %d\n", dma_in, dma_out);
  LL_ATON_PRINTF("---\n");
  LL_ATON_PRINTF("dest: %d\n", ATONN_DSTPORT_ID(__ll_switch_init[0].dest));
  LL_ATON_PRINTF("---\n");
  LL_ATON_PRINTF("source0: %d\n", ATONN_SRCPORT_ID(__ll_switch_init[0].source0));
  LL_ATON_PRINTF("frames0: %d\n", __ll_switch_init[0].frames0);
  LL_ATON_PRINTF("context0: %d\n", __ll_switch_init[0].context0);
  LL_ATON_PRINTF("---\n");
  LL_ATON_PRINTF("source1: %d\n", ATONN_SRCPORT_ID(__ll_switch_init[0].source1));
  LL_ATON_PRINTF("frames1: %d\n", __ll_switch_init[0].frames1);
  LL_ATON_PRINTF("context1: %d\n", __ll_switch_init[0].context1);
  LL_ATON_PRINTF("===\n");

#if (ATON_PLAT_HAS_FFLUSH)
  LL_ATON_FFLUSH(stdout);
#endif
#endif // DUMP_DEBUG_SW_OPS
}

static void __ll_lib_strswitch_set_dmas(int dma_in, int dma_out, LL_ATON_RT_EpochBlockItem_t *epoch_block_array)
{
  __ll_lib_dump_strswitch(dma_in, dma_out); // just for debug purposes

  __ll_lib_params_t *params = __ll_lib_get_params();

  __ll_switch_init[0].source0 = __atonn_getSrcPortID(STRSWITCH, 0, STRENG, dma_in, 0);
  __ll_switch_init[0].dest = __atonn_getDstPortID(STRSWITCH, 0, STRENG, dma_out, 0);

  AccelUnits dma_in_streng = {STRENG, dma_in};
  AccelUnits dma_out_streng = {STRENG, dma_out};

  __ll_lib_dma_units[1].unit = dma_in_streng;
  __ll_lib_dma_unit_id[1] = dma_in;
  __ll_lib_dma_units[0].unit = dma_out_streng;
  __ll_lib_dma_unit_id[0] = dma_out;

  uint32_t wait_mask = (0x1 << dma_out);
  params->g_wait_mask = wait_mask;
  epoch_block_array->wait_mask = wait_mask;

  __ll_lib_dump_strswitch(dma_in, dma_out); // just for debug purposes
}

static inline void __ll_lib_start_transfer(__ll_lib_params_t *params)
{
  LL_Streng_TensorInit(__ll_lib_dma_unit_id[1], &params->g_dma_in, 1);
  LL_Streng_TensorInit(__ll_lib_dma_unit_id[0], &params->g_dma_out, 1);
  LL_Switch_Init(__ll_switch_init, 1);
  LL_ATON_EnableUnits_Init(__ll_lib_dma_units, 2);
}

static inline void __ll_lib_stop_transfer(void)
{
  LL_Switch_Deinit(__ll_switch_init, 1);
  LL_ATON_DisableUnits_Init(__ll_lib_dma_units, 2);
  __LL_ATON_RT_SetWaitMask(0);
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

static inline void __ll_lib_prepare_outputs_epoch(const LL_Buffer_InfoTypeDef *outputs, unsigned int noutputs,
                                                  const LL_Streng_TensorInitTypeDef *dma_in,
                                                  const LL_Streng_TensorInitTypeDef *dma_out,
                                                  const LL_Buffer_InfoTypeDef *input)
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
  memcpy(outputs_copy, outputs, sizeof(LL_Buffer_InfoTypeDef) * noutputs);

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

static void __ll_lib_inputs_memcpy_start(const LL_ATON_RT_EpochBlockItem_t *epoch_block, uint8_t *_src)
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

static void __ll_lib_outputs_memcpy_start(const LL_ATON_RT_EpochBlockItem_t *epoch_block, uint8_t *_dst)
{
  __ll_lib_params_t *params = __ll_lib_get_params();

  uint8_t *_src = (uint8_t *)params->g_dst_o_src;
  size_t n;

  if (params->g_size < 0)
  {
    n = LL_Buffer_len(((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx);
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

/* `memset` helper functions */
#define __LL_DMA_INTERNAL_BUSPORT_WIDTH 8 // MUST correspond to value of `BUSPORT_DATA_W` in Verilog file `ipu_def.vpp`

static inline uint32_t __ll_lib_match_preload_with_busport(size_t size, uint8_t nbytes, uint32_t *min_bytes_to_preload)
{
  uint32_t _min_bytes_to_preload = *min_bytes_to_preload;
  uint32_t max_nr_preloads_in_busport = (__LL_DMA_INTERNAL_BUSPORT_WIDTH / _min_bytes_to_preload);

  unsigned int nr_of_samples = 2;
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
    size_t i = 0;
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
    size_t i = 0;
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
    for (size_t i = 0; i < size; i += 3)
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
    size_t i = 0;
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
                                         .offset_limit = (unsigned char *)dst_limit - dst_orig,
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

static inline void __ll_lib_pad_save_params(__ll_pad_sw_params_t *common_params, bool deep_copy)
{
  __ll_lib_params_t *params = __ll_lib_get_params();
  void *lower_heap = __ll_lib_get_lower_heap();

  /* flat copy of params */
  params->special.pad = *common_params;

  if (deep_copy == false)
  {
    return;
  }

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

/** Epoch start/end functions and epoch block arrays **/

#ifndef __LL_LIB_Concat_Cast_USE_ATON_HW
#define __LL_LIB_Concat_Cast_USE_ATON_HW 1
#endif

int __LL_LIB_TENSOR_ELEMENTS(const LL_LIB_TensorInfo_TypeDef *t)
{
  int cont = 1;
  for (int i = 0; i < t->ndims; i++)
    cont *= t->shape[i];
  return cont;
}

static void __LL_LIB_Concat_Case3_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                   const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT((params->special.concat_case3.outer_idx < params->g_num_tensors) &&
                 (params->g_idx < params->special.concat_case3.in_fheight)); // must be checked before

  __ll_lib_inputs_memcpy_start(epoch_block, (uint8_t *)params->special.concat_case3.in_curr);
}

static void __LL_LIB_Concat_Case3_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                 const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

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
      /* Release ATON lock */
      __ll_clear_aton_owner(nn_instance, false);

      /* proceed to next epoch block */
    }
  }
}

static void __LL_LIB_Inputs_Memcpy_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                    const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before

  uint8_t *src = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_inputs_memcpy_start(epoch_block, src);
}

static void __LL_LIB_Inputs_Memcpy_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                  const NN_Instance_TypeDef *nn_instance)
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
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Inputs_Batched_Memcpy_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                            const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before
  params->g_not_continuous =
      1; // disables code in __ll_lib_inputs_memcpy_start that assumes a flat copy operation e.g. prolog test

  uint8_t *src = (uint8_t *)LL_Buffer_addr_start(((LL_LIB_TensorInfo_TypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_inputs_memcpy_start(epoch_block, src);
}

static void __LL_LIB_Inputs_Batched_Memcpy_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                          const NN_Instance_TypeDef *nn_instance)
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
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Outputs_Channel_Split_Aton_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                                 const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before
  params->g_not_continuous =
      1; // disables code in __ll_lib_outputs_memcpy_start that assumes a flat copy operation e.g. prolog test

  uint8_t *dst = (uint8_t *)LL_Buffer_addr_start(((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_outputs_memcpy_start(epoch_block, dst);
}

static void __LL_LIB_Outputs_Channel_Split_Aton_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                               const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  LL_Buffer_InfoTypeDef *out = ((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx;
  int out_rank_old = out->ndims;
  int out_nchannels_old = out->shape[(out_rank_old - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    out = ((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx;

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

    unsigned int batch_depth = (nbytes == 4) ? (2 * out_nchannels) : out_nchannels;
    unsigned int frame_size = out_fwidth * out_fheight * batch_depth;

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
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Outputs_Channel_Split_Batched_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                                    const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before
  params->g_not_continuous =
      1; // disables code in __ll_lib_outputs_memcpy_start that assumes a flat copy operation e.g. prolog test

  uint8_t *dst = (uint8_t *)LL_Buffer_addr_start(((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_outputs_memcpy_start(epoch_block, dst);
}

static void __LL_LIB_Outputs_Channel_Split_Batched_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                                  const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  LL_Buffer_InfoTypeDef *out = ((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx;
  int out_rank_old = out->ndims;
  int out_nchannels_old = out->shape[(out_rank_old - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  params->g_idx++;

  if (params->g_idx < params->g_num_tensors)
  {
    out = ((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx;

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

    unsigned int batch_depth = (nbytes == 4) ? (2 * out_batch) : out_batch;
    unsigned int frame_size = out_fwidth * out_fheight * batch_depth;

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
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_Outputs_Memcpy_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                     const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  LL_ATON_ASSERT(params->g_idx < params->g_num_tensors); // must be checked before

  uint8_t *dst = (uint8_t *)LL_Buffer_addr_start(((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx);
  __ll_lib_outputs_memcpy_start(epoch_block, dst);
}

static void __LL_LIB_Outputs_Memcpy_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                   const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();

  if (__ll_lib_set_wait_mask((LL_ATON_RT_EpochBlockItem_t *)epoch_block, 0))
  {
    __ll_lib_stop_transfer();
  }

  if (params->g_size < 0)
  {
    params->g_dst_o_src += LL_Buffer_len(((LL_Buffer_InfoTypeDef *)params->g_tensors) + params->g_idx);
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
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_DMA_Pad_Memset_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                   const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(epoch_block);

  __ll_lib_params_t *params = __ll_lib_get_params();

  __ll_lib_stop_transfer();

  if (params->special.pad.callback_function != NULL)
  {
#ifndef NDEBUG
    extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(__ll_current_aton_ip_owner == nn_instance);
#endif // !NDEBUG

    /* return from current epoch block */
    __LL_ATON_RT_RetFromLibEpochBlockArray(nn_instance); // Note: we are still the ATON owner
    /* call follow-up function */
    (*params->special.pad.callback_function)(&params->special.pad, nn_instance);
  }
  else
  {
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_DMA_Pad_Filling_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                      const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

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
      params->g_dma_in.offset_limit = (uint8_t *)common_params->in_limit - (uint8_t *)common_params->in_target;

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

static void __LL_LIB_DMA_Pad_Filling_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                    const NN_Instance_TypeDef *nn_instance)
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
#ifndef NDEBUG
    extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
    LL_ATON_ASSERT(__ll_current_aton_ip_owner == nn_instance);
#endif // !NDEBUG

    /* return from current epoch block */
    __LL_ATON_RT_RetFromLibEpochBlockArray(nn_instance); // Note: we are still the ATON owner
    /* call follow-up function */
    (*params->special.pad.callback_function)(&params->special.pad, nn_instance);
    return;
  }
  else
  {
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
      /* check endianness */
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

    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* proceed to next epoch block */
  }
}

static void __LL_LIB_DMA_Transfer_Start_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                   const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(epoch_block);
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_params_t *params = __ll_lib_get_params();
  __ll_lib_start_transfer(params);
}

static void __LL_LIB_DMA_Transfer_End_EpochBlock(const LL_ATON_RT_EpochBlockItem_t *epoch_block,
                                                 const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(epoch_block);
  LL_ATON_LIB_UNUSED(nn_instance);

  __ll_lib_stop_transfer();

  /* Release ATON lock */
  __ll_clear_aton_owner(nn_instance, false);

  /* proceed to next epoch block */
}

static LL_ATON_RT_EpochBlockItem_t __ll_internal_concat_case3_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_inputs_memcpy_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_inputs_batched_memcpy_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_outputs_memcpy_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_outputs_channel_split_aton_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_outputs_channel_split_batched_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_simple_oneshot_transfer[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_dma_Pad_memset_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

static LL_ATON_RT_EpochBlockItem_t __ll_internal_dma_Pad_filling_epoch_block_array[] = {
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
    {.flags = EpochBlock_Flags_last_eb | EpochBlock_Flags_internal},
};

/**
 * @brief Performs an asynchronous memory copy operation using DMAs
 * @param  input_start Start address of input tensor
 * @param  input_end End address of input tensor
 * @param  input_limit Input tensor limit address
 * @param  output_start Start address of output tensor
 * @param  dma_in Input DMA / Streaming Engine to be used
 * @param  dma_out Output DMA / Streaming Engine to be used
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @return Error code
 *
 * @note   Must only be called by ATON owner
 */
static int __LL_ATON_LIB_Async_Memcpy(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                                      unsigned char *output_start, int dma_in, int dma_out,
                                      const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner == nn_instance);
#endif // !NDEBUG

  /* get parameters location */
  __ll_lib_params_t *params = __ll_lib_get_params();

  /* use standard settings for DMAs*/
  params->g_dma_in = __ll_lib_static_const_dma_in;
  params->g_dma_out = __ll_lib_static_const_dma_out;

  uint8_t *_dst = (uint8_t *)output_start;
  uint8_t *_src = (uint8_t *)input_start;
  uint32_t n = input_end - input_start;
  n = __ll_lib_memcpy_prolog((void **)&_dst, (void **)&_src, n);

  if (n > 0)
  {
    /* output dma */
    params->g_dma_out.addr_base.p = (uint8_t *)_dst;
    params->g_dma_out.offset_start = 0;
    params->g_dma_out.offset_end = n;

    /* input dma*/
    params->g_dma_in.addr_base.p = (uint8_t *)_src;
    params->g_dma_in.offset_start = 0;
    params->g_dma_in.offset_end = n;

    params->g_dma_in.offset_limit = input_limit - input_start;

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_simple_oneshot_transfer);

    /* start asynchronous memcpy */
    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_simple_oneshot_transfer);
  }
  else
  {
    /* Release ATON lock */
    __ll_clear_aton_owner(nn_instance, false);

    /* do not start any transfer and wait, just proceed to end function */
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs a memory copy operation from `ninputs` inputs to one output using stream engines `dma_in` and
 * `dma_out`
 * @param  inputs list of input tensor info structures
 * @param  ninputs number of inputs
 * @param  dst destination address
 * @param  nbytes number of bytes to copy (-1 means: derive from `inputs` structure)
 */
static void __LL_ATON_LIB_DMA_Inputs_Memcpy(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                            unsigned char *dst, int nbytes, int dma_in, int dma_out,
                                            const NN_Instance_TypeDef *nn_instance)
{
  /* start epoch block sequence */
  if (ninputs > 0)
  {
    /* become ATON owner */
    __ll_set_aton_owner(nn_instance);

    /* prepare epoch */
    __ll_lib_prepare_inputs_epoch(inputs, ninputs, &__ll_lib_static_const_dma_in, &__ll_lib_static_const_dma_out, dst,
                                  nbytes);

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_inputs_memcpy_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_inputs_memcpy_epoch_block_array);
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
                                                    unsigned char *dst, int dma_in, int dma_out,
                                                    const NN_Instance_TypeDef *nn_instance)
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
  unsigned int i;

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
    /* become ATON owner */
    __ll_set_aton_owner(nn_instance);

    /* prepare epoch */
    __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_dma_in, &_dma_out, dst, -1);

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_inputs_batched_memcpy_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_inputs_batched_memcpy_epoch_block_array);
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
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 */
static void __LL_ATON_LIB_DMA_Outputs_Memcpy_Helper(const LL_Buffer_InfoTypeDef *input,
                                                    const LL_Buffer_InfoTypeDef *outputs, unsigned int noutputs,
                                                    int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance)
{
  /* start epoch block sequence */
  if (noutputs > 0)
  {
    /* become ATON owner */
    __ll_set_aton_owner(nn_instance);

    /* prepare epoch */
    __ll_lib_prepare_outputs_epoch(outputs, noutputs, &__ll_lib_static_const_dma_in, &__ll_lib_static_const_dma_out,
                                   input);

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_outputs_memcpy_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_outputs_memcpy_epoch_block_array);
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
 * @param  noutputs number of output tensors
 * @param  leading_dims dimensions before split axis
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 */
static void __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton_Helper(const LL_Buffer_InfoTypeDef *input,
                                                                const LL_Buffer_InfoTypeDef *outputs,
                                                                unsigned int noutputs, unsigned int leading_dims,
                                                                int dma_in, int dma_out,
                                                                const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_ASSERT(noutputs > 0);

  uint32_t out_ndims = outputs[0].ndims;
  LL_ATON_ASSERT(out_ndims >= 3);
  uint32_t nbytes =
      LL_LIB_NBYTES(outputs[0].nbits); // assuming that value is the same for all output tensors and input tensor
  uint32_t out_fwidth = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FWIDTH];   // assuming that value is the same for
                                                                                // all output tensors and input tensor
  uint32_t out_fheight = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FHEIGHT]; // assuming that value is the same for
                                                                                // all output tensors and input tensor
  uint32_t out_nchannels = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_NCHANNELS];

  uint32_t in_nchannels = 0;
  unsigned int i;

  for (i = 0; i < noutputs; i++)
    in_nchannels += outputs[i].shape[(out_ndims - 4) + 1 /* ONNX_CHANNEL_OFFSET */];

  unsigned char nbits = (nbytes == 4) ? 16 : (nbytes * 8); // same for all output tensors and input tensor

  // LL_ATON_PRINTF("\ndma_out: addr_start=%p, addr_end=%p\n", LL_Buffer_addr_start(outputs + 0),
  // LL_Buffer_addr_end(outputs + 0));

  if (noutputs > 0)
  {
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

    unsigned int batch_depth = (nbytes == 4) ? (2 * out_nchannels) : out_nchannels;
    unsigned int frame_size = out_fwidth * out_fheight * batch_depth;

    /* become ATON owner */
    __ll_set_aton_owner(nn_instance);

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
    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_outputs_channel_split_aton_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_outputs_channel_split_aton_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
}

/**
 * @brief  performs an INT8 (scale/offset) Softmax (oonx opset >=13) operation inputs and output operands according to
 * ONNX semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1) with input tensor coerced
 * to 2D by collapsing dimensions before and after axis
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int __LL_ATON_LIB_Softmax_INT8_legacy(const LL_LIB_TensorInfo_TypeDef *input,
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
  if ((int)axis > start_dim)
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

/**
 * @brief  performs a channel-split memory copy operation from one input (ATON canonical) to `noutputs`
 * non-ATON-canonical outputs using stream engines `dma_in` and `dma_out`
 * @param  src source address
 * @param  outputs list of output tensor shape structures
 * @param  noutputs number of outputs
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 */
static void __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched_Helper(const LL_Buffer_InfoTypeDef *input,
                                                                   const LL_Buffer_InfoTypeDef *outputs,
                                                                   unsigned int noutputs, int dma_in, int dma_out,
                                                                   const NN_Instance_TypeDef *nn_instance)
{
  uint32_t out_ndims = outputs[0].ndims;
  LL_ATON_ASSERT(out_ndims >= 3);
  uint32_t nbytes =
      LL_LIB_NBYTES(outputs[0].nbits); // assuming that value is the same for all output tensors and input tensor
  uint32_t out_fwidth = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FWIDTH];   // assuming that value is the same for
                                                                                // all output tensors and input tensor
  uint32_t out_fheight = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_FHEIGHT]; // assuming that value is the same for
                                                                                // all output tensors and input tensor
  uint32_t out_nchannels = outputs[0].shape[(out_ndims - 4) + TDIM_ONNX_NCHANNELS];

  uint32_t in_nchannels = 0;
  unsigned int i;

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

  unsigned int batch_depth = (nbytes == 4) ? (2 * out_batch) : out_batch;
  unsigned int frame_size = out_fwidth * out_fheight * batch_depth;

  /* become ATON owner */
  if (noutputs > 0)
  {
    __ll_set_aton_owner(nn_instance);
  }

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
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_outputs_channel_split_aton_epoch_block_array);

    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_outputs_channel_split_batched_epoch_block_array);
  }
  else
  {
    /* proceed to next epoch block */
  }
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
static int __LL_ATON_LIB_Softmax_INT8(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                                      unsigned int axis)
{
  int b, o, hw;

  int outer_elem = 1, inner_elem = 1;
  int axis_elem = input->shape[axis];

  for (unsigned int i = 0; i < axis; i++)
    outer_elem *= input->shape[i];
  for (unsigned int i = axis + 1; i < input->ndims; i++)
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
 * @brief  performs a float Softmax (oonx opset >=13) operation on float inputs and output operands according to ONNX
 * semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1)
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int __LL_ATON_LIB_Softmax_float(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
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

  for (unsigned int i = 0; i < axis; i++)
    outer_elem *= input->shape[i];
  for (unsigned int i = axis + 1; i < input->ndims; i++)
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
 * @brief  performs a float Softmax (oonx opset < 13) operation on float inputs and output operands according to ONNX
 * semantics
 * @brief Softmax(input, axis) = Exp(input) / ReduceSum(Exp(input), axis=axis, keepdims=1) with input tensor coerced
 * to 2D by collapsing dimensions before and after axis
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  axis for coalescing of shape into a 2D matrix
 * @retval Error code
 */
static int __LL_ATON_LIB_Softmax_float_legacy(const LL_LIB_TensorInfo_TypeDef *input,
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
  if ((int)axis > start_dim)
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
 * @brief  performs flat copy operation on an input and several outputs using DMA
 * @param  input tensor shape structure
 * @param  outputs tensor shape structures
 * @param  nr_of_outputs number of output tensors
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_in DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @retval Error code
 */
int __LL_ATON_LIB_DMA_Outputs_Flat_Copy(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *outputs,
                                        unsigned int nr_of_outputs, int dma_in, int dma_out,
                                        const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned int i = 0; i < nr_of_outputs; i++)
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

  __LL_ATON_LIB_DMA_Outputs_Memcpy_Helper(input, outputs, nr_of_outputs, dma_in, dma_out, nn_instance);

  return LL_ATON_OK;
}

/**
 * @brief  performs channel-split copy operation on an input and several outputs (both in ATON canonical format) using
 * DMA
 * @param  input tensor shape structure
 * @param  outputs tensor shape structures
 * @param  nr_of_outputs number of output tensors
 * @param  leading_dims dimensions before split axis
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @retval Error code
 */
int __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton(const LL_Buffer_InfoTypeDef *input,
                                                 const LL_Buffer_InfoTypeDef *outputs, unsigned int nr_of_outputs,
                                                 unsigned int leading_dims, int dma_in, int dma_out,
                                                 const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned int i = 0; i < nr_of_outputs; i++)
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

  __LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton_Helper(input, outputs, nr_of_outputs, leading_dims, dma_in, dma_out,
                                                      nn_instance);

  return LL_ATON_OK;
}

/**
 * @brief  performs a channel-split memory copy operation from one input (ATON canonical) to `noutputs`
 * non-ATON-canonical outputs using stream engines `dma_in` and `dma_out`
 * @param  src source address
 * @param  outputs list of output tensor shape structures
 * @param  nr_of_outputs number of outputs
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @retval Error code
 */
int __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched(const LL_Buffer_InfoTypeDef *input,
                                                    const LL_Buffer_InfoTypeDef *outputs, unsigned int nr_of_outputs,
                                                    int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  // LL_ATON_PRINTF("%s() line %d\n", __func__, __LINE__);

  int input_size = LL_Buffer_len(input);
  int output_size = 0;

  for (unsigned int i = 0; i < nr_of_outputs; i++)
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

  __LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched_Helper(input, outputs, nr_of_outputs, dma_in, dma_out, nn_instance);

  return LL_ATON_OK;
}

/**
 * @brief  performs an optimized `memset` for the `Pad` operator using DMA (aka Framing)
 * @param  output destination address of `memset` operation
 * @param  constant_value constant value to be set
 * @param  out_size number of bytes to output
 * @param  common_params parameters needed to setup DMAs and to forward to eventual callback function
 * @param  deep_copy if true, common_params vectors are to be copied to lower heap
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @retval Error code
 */
static int __LL_ATON_LIB_DMA_Pad_Memset(void *output, int32_t constant_value, size_t out_size,
                                        __ll_pad_sw_params_t *common_params, bool deep_copy,
                                        const NN_Instance_TypeDef *nn_instance)
{
  /* become ATON owner */
  __ll_set_aton_owner(nn_instance);

  /* save common parameters */
  __ll_lib_pad_save_params(common_params, deep_copy);

  /* start operation */
  bool ret = __ll_lib_memset(output, common_params->out_limit, constant_value, common_params->nbytes, out_size);

  if (ret)
  {
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): performing DMA based `memset`\n", __func__, __LINE__);
#endif

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(common_params->dma_in, common_params->dma_out,
                                __ll_internal_dma_Pad_memset_epoch_block_array);

    /* start DMAs for `memset` & run `LL_ATON_LIB_Pad_Filling()` */
    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_dma_Pad_memset_epoch_block_array);
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
      return (*params->special.pad.callback_function)(&params->special.pad, nn_instance);
    }
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs HW accelerated filling operation for `Pad` operator
 * @retval Error code
 */
static int __LL_ATON_LIB_DMA_Pad_Filling(__ll_pad_sw_params_t *init_common_params,
                                         const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
#endif // !NDEBUG

  /* save common parameters */
  if (init_common_params != NULL)
  {
    /* become ATON owner */
    __ll_set_aton_owner(nn_instance);

    __ll_lib_pad_save_params(init_common_params, true);
  }
  else
  {
#ifndef NDEBUG
    LL_ATON_ASSERT(__ll_current_aton_ip_owner == nn_instance);
#endif // !NDEBUG
  }

  /* get common parameters */
  __ll_lib_params_t *params = __ll_lib_get_params();
  __ll_pad_sw_params_t *common_params = &params->special.pad;

  /* prepare epoch */
  params->g_dma_in = __ll_lib_static_const_dma_in;
  params->g_dma_out = __ll_lib_static_const_dma_out;

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
  __ll_lib_strswitch_set_dmas(common_params->dma_in, common_params->dma_out,
                              __ll_internal_dma_Pad_filling_epoch_block_array);

  /* start DMAs for filling `consecutive bytes` */
  LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_dma_Pad_filling_epoch_block_array);

  return LL_ATON_OK;
}

/**
 * @brief  performs HW accelerated filling operation for `Pad` operator where 4-Loop optimization is possible
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @retval Error code
 *
 * @note   Has never a callback to be called
 * @note   Must only be called by ATON owner
 */
static int __LL_ATON_LIB_DMA_Pad_4Loop_Filling(__ll_pad_sw_params_t *common_params,
                                               const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner == nn_instance);
#endif // !NDEBUG

  /* get common parameters */
  __ll_lib_params_t *params = __ll_lib_get_params();
  if (common_params != NULL)
  {
    params->special.pad = *common_params;
  }

  struct four_axes *negative_4loop = (struct four_axes *)&params->special.pad.negative_4loop;
  LL_ATON_ASSERT((negative_4loop->inner_bytes_to_copy > 0) || (negative_4loop->nr_of_stream_eng_loops == 0));
  LL_ATON_ASSERT(negative_4loop->nr_of_stream_eng_loops <= NR_OF_STREAM_ENG_LOOPS);

  unsigned char *in_start =
      (unsigned char *)params->special.pad.in_target + negative_4loop->initial_cumulative_start_offset_in_bytes;
  unsigned char *in_end = in_start + negative_4loop->total_bytes_to_copy;

  struct four_axes *positive_4loop = (struct four_axes *)&params->special.pad.positive_4loop;
  LL_ATON_ASSERT((positive_4loop->inner_bytes_to_copy > 0) || (positive_4loop->nr_of_stream_eng_loops == 0));
  LL_ATON_ASSERT(positive_4loop->nr_of_stream_eng_loops <= NR_OF_STREAM_ENG_LOOPS);

  unsigned char *out_start =
      (unsigned char *)params->special.pad.out_target + positive_4loop->initial_cumulative_start_offset_in_bytes;

  /* both negative and positive padding result to be a memory copy */
  if ((positive_4loop->nr_of_stream_eng_loops == 0) && (negative_4loop->nr_of_stream_eng_loops == 0))
  { // just perform a memcpy
    return __LL_ATON_LIB_Async_Memcpy(in_start, in_end, (unsigned char *)params->special.pad.in_limit, out_start,
                                      params->special.pad.dma_in, params->special.pad.dma_out, nn_instance);
  }

  /* calculate number of bits to use in input streaming engine */
  uint32_t n_bits_to_use_negative;
  uint32_t consecutive_bytes_negative = negative_4loop->inner_bytes_to_copy;
  uint32_t batch_depth_negative;
  if ((consecutive_bytes_negative % 3) == 0)
  {
    n_bits_to_use_negative = 24;
    batch_depth_negative = (consecutive_bytes_negative / 3);
  }
  else if ((consecutive_bytes_negative % 2) == 0)
  {
    n_bits_to_use_negative = 16;
    batch_depth_negative = (consecutive_bytes_negative / 2);
  }
  else
  {
    n_bits_to_use_negative = 8;
    batch_depth_negative = consecutive_bytes_negative;
  }

  /* calculate number of bits to use in output streaming engine */
  uint32_t n_bits_to_use_positive;
  uint32_t consecutive_bytes_positive = positive_4loop->inner_bytes_to_copy;
  uint32_t batch_depth_positive;
  if ((consecutive_bytes_positive % 3) == 0)
  {
    n_bits_to_use_positive = 24;
    batch_depth_positive = (consecutive_bytes_positive / 3);
  }
  else if ((consecutive_bytes_positive % 2) == 0)
  {
    n_bits_to_use_positive = 16;
    batch_depth_positive = (consecutive_bytes_positive / 2);
  }
  else
  {
    n_bits_to_use_positive = 8;
    batch_depth_positive = consecutive_bytes_positive;
  }

  /* calculate lowest common denominator */
  uint32_t n_bits_to_use;
  if (n_bits_to_use_negative == n_bits_to_use_positive)
  {
    n_bits_to_use = n_bits_to_use_negative;
  }
  else
  {
    n_bits_to_use = 8;
    batch_depth_negative = consecutive_bytes_negative;
    batch_depth_positive = consecutive_bytes_positive;
  }

  /* Reset in/out DMA values*/
  params->g_dma_in = __ll_lib_static_const_dma_in;
  params->g_dma_out = __ll_lib_static_const_dma_out;

  /*** Configure input DMA ***/
  /* common values */
  params->g_dma_in.dir = 0;
  params->g_dma_in.nbits_in = n_bits_to_use;
  params->g_dma_in.nbits_out = n_bits_to_use;
  params->g_dma_in.addr_base.p = in_start;
  params->g_dma_in.offset_start = 0;
  params->g_dma_in.offset_end = negative_4loop->total_bytes_to_copy;
  params->g_dma_in.offset_limit = params->special.pad.in_limit - (int8_t *)in_start;

  /* not common configuration for raw & non-raw mode */
  if (negative_4loop->nr_of_stream_eng_loops == 0)
  { // read in raw mode
    /* configure raw input DMA */
    params->g_dma_in.raw = 1;
    params->g_dma_in.frame_tot_cnt = 1;
  }
  else
  { // cannot read in raw mode
    /* configure non-raw mode */
    params->g_dma_in.raw = 0;

    /* reset all loops */
    params->g_dma_in.fwidth = 1;
    params->g_dma_in.fheight = 1;
    params->g_dma_in.frame_tot_cnt = 1;
    params->g_dma_in.frame_loop_cnt = 0;

    params->g_dma_in.batch_offset = 0;
    params->g_dma_in.line_offset = 0;
    params->g_dma_in.frame_offset = 0;
    params->g_dma_in.loop_offset = 0;

    /* start programming batch loop */
    params->g_dma_in.batch_depth = batch_depth_negative;

    params->g_dma_in.fwidth = negative_4loop->four_items_array[0].nr_of_loops;
    params->g_dma_in.batch_offset = negative_4loop->four_items_array[0].offset_in_bytes;

    /* start programming line loop */
    if (negative_4loop->nr_of_stream_eng_loops > 1)
    {
      params->g_dma_in.fheight = negative_4loop->four_items_array[1].nr_of_loops;
      params->g_dma_in.line_offset = negative_4loop->four_items_array[1].offset_in_bytes;

      if (negative_4loop->nr_of_stream_eng_loops > 2)
      {
        /* prepare for two outer loops */
        if (negative_4loop->nr_of_stream_eng_loops < NR_OF_STREAM_ENG_LOOPS) // i.e. == 3
        {                                                                    // repetition loop
          params->g_dma_in.frame_tot_cnt = negative_4loop->four_items_array[2].nr_of_loops;
        }
        else
        { // full 4-loops; main loop
          params->g_dma_in.frame_loop_cnt = negative_4loop->four_items_array[2].nr_of_loops;
          params->g_dma_in.frame_tot_cnt =
              params->g_dma_in.frame_loop_cnt * negative_4loop->four_items_array[3].nr_of_loops;
          params->g_dma_in.loop_offset = negative_4loop->four_items_array[3].offset_in_bytes;
        }
        params->g_dma_in.frame_offset = negative_4loop->four_items_array[2].offset_in_bytes;
      }
    }
  }

  /*** Configure output DMA ***/
  /* common values */
  params->g_dma_out.dir = 1;
  params->g_dma_out.nbits_in = n_bits_to_use;
  params->g_dma_out.nbits_out = n_bits_to_use;
  params->g_dma_out.addr_base.p = out_start;
  params->g_dma_out.offset_start = 0;
  params->g_dma_out.offset_end = positive_4loop->total_bytes_to_copy;

  /* not common configuration for raw & non-raw mode */
  if (positive_4loop->nr_of_stream_eng_loops == 0)
  { // read in raw mode
    /* configure raw input DMA */
    params->g_dma_out.raw = 1;
    params->g_dma_out.frame_tot_cnt = 1;
  }
  else
  {
    /* configure non-raw mode */
    params->g_dma_out.raw = 0;

    /* reset all loops */
    params->g_dma_out.fwidth = 1;
    params->g_dma_out.fheight = 1;
    params->g_dma_out.frame_tot_cnt = 1;
    params->g_dma_out.frame_loop_cnt = 0;

    params->g_dma_out.batch_offset = 0;
    params->g_dma_out.line_offset = 0;
    params->g_dma_out.frame_offset = 0;
    params->g_dma_out.loop_offset = 0;

    /* start programming batch loop */
    params->g_dma_out.batch_depth = batch_depth_positive;

    params->g_dma_out.fwidth = positive_4loop->four_items_array[0].nr_of_loops;
    params->g_dma_out.batch_offset = positive_4loop->four_items_array[0].offset_in_bytes;

    /* start programming line loop */
    if (positive_4loop->nr_of_stream_eng_loops > 1)
    {
      params->g_dma_out.fheight = positive_4loop->four_items_array[1].nr_of_loops;
      params->g_dma_out.line_offset = positive_4loop->four_items_array[1].offset_in_bytes;

      if (positive_4loop->nr_of_stream_eng_loops > 2)
      {
        /* prepare for two outer loops */
        if (positive_4loop->nr_of_stream_eng_loops < NR_OF_STREAM_ENG_LOOPS) // i.e. == 3
        {                                                                    // repetition loop
          params->g_dma_out.frame_tot_cnt = positive_4loop->four_items_array[2].nr_of_loops;
        }
        else
        { // full 4-loops; main loop
          params->g_dma_out.frame_loop_cnt = positive_4loop->four_items_array[2].nr_of_loops;
          params->g_dma_out.frame_tot_cnt =
              params->g_dma_out.frame_loop_cnt * positive_4loop->four_items_array[3].nr_of_loops;
          params->g_dma_out.loop_offset = positive_4loop->four_items_array[3].offset_in_bytes;
        }
        params->g_dma_out.frame_offset = positive_4loop->four_items_array[2].offset_in_bytes;
      }
    }
  }

  /* configure stream switch */
  __ll_lib_strswitch_set_dmas(params->special.pad.dma_in, params->special.pad.dma_out,
                              __ll_internal_simple_oneshot_transfer);

  /* start asynchronous memcpy */
  LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_simple_oneshot_transfer);

  return LL_ATON_OK;
}

/*** Compiler generated code callable functions ***/

/**
 * @brief  performs a tensor ImageToRow transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 * @param  stride_h vertical stride for the sliding window
 * @param  stride_w horizontal stride for the sliding window
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_ImageToRow(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                               const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                               unsigned int blocksize_w, unsigned int stride_h, unsigned int stride_w, int dma_in,
                               int dma_out, const NN_Instance_TypeDef *nn_instance)
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

  unsigned int batch_depth = (nbytes == 4) ? (2 * in_nchannels) : in_nchannels;

  /* become ATON owner */
  __ll_set_aton_owner(nn_instance);

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
  __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_simple_oneshot_transfer);

  /* start epoch block sequence */
  LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_simple_oneshot_transfer);

  return LL_ATON_OK;
}

/**
 * @brief  performs a tensor SpaceToDepth transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_SpaceToDepth(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                                 unsigned int blocksize_w, int dma_in, int dma_out,
                                 const NN_Instance_TypeDef *nn_instance)
{
  return LL_ATON_LIB_DMA_ImageToRow(inputs, ninputs, output, blocksize_h, blocksize_w, blocksize_h, blocksize_w, dma_in,
                                    dma_out, nn_instance);
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
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_RowToImage(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                               const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                               unsigned int blocksize_w, unsigned int stride_h, unsigned int stride_w, int dma_in,
                               int dma_out, const NN_Instance_TypeDef *nn_instance)
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
  unsigned int nbits_unsigned = inputs[0].Qunsigned;
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

  if ((in_batches != out_batches) || (in_nchannels != (out_nchannels * (int)(blocksize_h * blocksize_w))) ||
      ((unsigned int)out_fwidth < blocksize_w) || ((unsigned int)out_fheight < blocksize_h) ||
      ((out_fwidth - blocksize_w) % stride_w) || ((out_fheight - blocksize_h) % stride_h) ||
      (in_fwidth != (((out_fwidth - (int)blocksize_w) / (int)stride_w) + 1)) ||
      (in_fheight != (((out_fheight - (int)blocksize_h) / (int)stride_h) + 1)))
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

  /* become ATON owner */
  __ll_set_aton_owner(nn_instance);

  /* prepare epoch */
  __ll_lib_prepare_inputs_epoch(inputs, ninputs, &_dma_in, &_dma_out,
                                /* all of the rest of parameters are irrelevant to this use */ 0, 0);

  /* configure stream switch */
  __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_simple_oneshot_transfer);

  /* start epoch block sequence */
  LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_simple_oneshot_transfer);

  return LL_ATON_OK;
}

/**
 * @brief  performs a tensor DepthToSpace transfer operation using stream engines `dma_in` and `dma_out`
 * @param  list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structures
 * @param  blocksize_h vertical dimension for the blocksize
 * @param  blocksize_w horizontal dimension for the blocksize
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 *
 * @note   Supports only input and output tensors in ATON canonical format
 *
 * @note   Supports only DCR (depth-column-row) order re-arrangement
 *
 * @note   Bit-sizes are rounded up to multiples of 8-bits
 *
 */
int LL_ATON_LIB_DMA_DepthToSpace(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                 const LL_LIB_TensorInfo_TypeDef *output, unsigned int blocksize_h,
                                 unsigned int blocksize_w, int dma_in, int dma_out,
                                 const NN_Instance_TypeDef *nn_instance)
{
  return LL_ATON_LIB_DMA_RowToImage(inputs, ninputs, output, blocksize_h, blocksize_w, blocksize_h, blocksize_w, dma_in,
                                    dma_out, nn_instance);
}

int LL_ATON_LIB_DMA_Transpose(const LL_Buffer_InfoTypeDef *input, const uint32_t *input_axes_offsets,
                              const LL_Buffer_InfoTypeDef *output, const uint32_t *output_axes_offsets,
                              const uint8_t *target_pos, const uint8_t *perm_to_use, int dma_in, int dma_out,
                              const NN_Instance_TypeDef *nn_instance)
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

  /* become ATON owner */
  __ll_set_aton_owner(nn_instance);

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
  __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_simple_oneshot_transfer);

  /* schedule epoch block */
  LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_simple_oneshot_transfer);

  return LL_ATON_OK;
}

/**
 * @brief  performs a concat operation according to ONNX semantics
 * @param  inputs list of input tensor info structures
 * @param  number of inputs
 * @param  output tensor info structure
 * @param  axis for concatenation
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @retval Error code
 */
int LL_ATON_LIB_Concat(const LL_Buffer_InfoTypeDef *inputs, unsigned int ninputs, const LL_Buffer_InfoTypeDef *output,
                       unsigned int axis, int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance)
{
  unsigned int i;
  int k;

  // LL_ATON_PRINTF("Concat ------ axis=%d\n", axis);
  if (ninputs == 0)
    __LL_LIB_ERROR(_ERR_NINPUTS, LL_ATON_INVALID_PARAM);

  unsigned int in_ndims = inputs[0].ndims;

  if (in_ndims < 4)
    __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);

  unsigned int in_batch = inputs[0].batch;
  // int in_fwidth = inputs[0].shape[(in_ndims - 4) + TDIM_FWIDTH];
  unsigned int in_fheight = inputs[0].shape[(in_ndims - 4) + TDIM_FHEIGHT];
  unsigned int in_nchannels = inputs[0].shape[(in_ndims - 4) + TDIM_NCHANNELS];
  unsigned int out_batch = output->batch;
  unsigned int out_fwidth = output->shape[(in_ndims - 4) + TDIM_FWIDTH];
  // int out_fheight = output->shape[(in_ndims - 4) + TDIM_FHEIGHT];
  unsigned int out_nchannels = output->shape[(in_ndims - 4) + TDIM_NCHANNELS];

  bool in_canonical = (in_batch == in_nchannels);
  bool out_canonical = (out_batch == out_nchannels);

  // convert axis from ...CHW -> ...HWC
  int axis_lut[] = {TDIM_NKERNELS, TDIM_NCHANNELS, TDIM_FHEIGHT, TDIM_FWIDTH}; // 0, 3, 1, 2
#define __LL_ATON_LIB_LUT_AXIS(x) ((x >= (in_ndims - 4)) ? (in_ndims - 4) + axis_lut[x - (in_ndims - 4)] : x)

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
  int atonn_axis = __LL_ATON_LIB_LUT_AXIS(axis);

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

    for (k = 0; k < (int)in_ndims; k++)
    {
      if ((k != atonn_axis) && (inputs[0].shape[k] != inputs[i].shape[k]))
        __LL_LIB_ERROR(_ERR_SHAPE_IN, LL_ATON_INVALID_PARAM);
    }
  }

  for (k = 0; k < (int)in_ndims; k++)
  {
    if (k != atonn_axis && output->shape[k] != inputs[0].shape[k])
    {
      // LL_ATON_PRINTF("k=%d axis=%d %d != %d\n", k, atonn_axis, output->shape[k], inputs[0].shape[k]);
      __LL_LIB_ERROR(_ERR_SHAPE, LL_ATON_INVALID_PARAM);
    }
  }

  if (output->shape[atonn_axis] != (unsigned int)tot_axis_dim)
    __LL_LIB_ERROR(_ERR_AXIS, LL_ATON_INVALID_PARAM);

  // LL_ATON_PRINTF("tot: size=%d b=%d w=%d g=%d c=%d\n",tot_size,tot_batches,tot_fwidth,tot_fheight,tot_nchannels);

  if (nbits != output->nbits) // perhaps this could be relaxed later on FIXME !!!
    __LL_LIB_ERROR(_ERR_NBITS, LL_ATON_INVALID_PARAM);

  if ((unsigned int)tot_size > LL_Buffer_len(output))
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
#if __LL_LIB_Concat_Cast_USE_ATON_HW
      __LL_ATON_LIB_DMA_Inputs_Memcpy(inputs, ninputs, LL_Buffer_addr_start(output), -1, dma_in, dma_out, nn_instance);
#else  // !__LL_LIB_Concat_Cast_USE_ATON_HW
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
#endif // !__LL_LIB_Concat_Cast_USE_ATON_HW
      return LL_ATON_OK;
    case 2: // Channels
    {
      if (in_batch == out_batch)
      {
        __LL_ATON_LIB_DMA_Inputs_Memcpy(inputs, ninputs, LL_Buffer_addr_start(output), -1, dma_in, dma_out,
                                        nn_instance);
      }
      else
      {
        __LL_ATON_LIB_DMA_Inputs_Batched_Memcpy(inputs, ninputs, LL_Buffer_addr_start(output), dma_in, dma_out,
                                                nn_instance);
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

#if __LL_LIB_Concat_Cast_USE_ATON_HW
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
          __ll_lib_prepare_inputs_epoch(inputs, ninputs, &__ll_lib_static_const_dma_in, &__ll_lib_static_const_dma_out,
                                        (void *)out_start, line_size);

          /* configure stream switch */
          __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_concat_case3_epoch_block_array);

          LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_concat_case3_epoch_block_array);
        }
        else
        {
          /* proceed to next epoch block */
        }
      }
#else  // !__LL_LIB_Concat_Cast_USE_ATON_HW
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

    unsigned int dst;
    unsigned int src = 0;
    for (dst = start; dst < stop; dst += jump, src += copy_val)
    {
      // LL_ATON_PRINTF("i =%d dst = %d src = %d\n", i, dst, src);
      memcpy(LL_Buffer_addr_start(output) + dst, LL_Buffer_addr_start(inputs + i) + src, copy_val);
    }
    start += copy_val;
  }

  return LL_ATON_OK;
}

/**
 * @brief  performs a cast operation to/from Qmn and float
 * @param  input tensor info structure
 * @param  output tensor info structure
 * @param  dma_in DMA number of DMA reading from memory
 * @param  dma_out DMA number of DMA writing to memory
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @retval Error code
 */
int LL_ATON_LIB_Cast(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output, int dma_in,
                     int dma_out, const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_LIB_UNUSED(nn_instance);

  unsigned int Qm_in = input->Qm;
  unsigned int Qm_out = output->Qm;
  unsigned int Qn_in = input->Qn;
  unsigned int Qn_out = output->Qn;
  unsigned int Qunsigned_in = input->Qunsigned;
  unsigned int Qunsigned_out = output->Qunsigned;
  int dtype_in = input->type;
  int dtype_out = output->type;
  unsigned int nbits_in = input->nbits;
  unsigned int nbits_out = output->nbits;
  unsigned int in_elements = __LL_LIB_TENSOR_ELEMENTS(input);
  unsigned int out_elements = __LL_LIB_TENSOR_ELEMENTS(output);
  unsigned int in_bit_size = (input->nbits == 0 ? sizeof(float) * 8 : input->nbits);
  unsigned int out_bit_size = (output->nbits == 0 ? sizeof(float) * 8 : output->nbits);
  unsigned int in_byte_size = (in_bit_size * in_elements + 7) >> 3;
  unsigned int out_byte_size = (out_bit_size * out_elements + 7) >> 3;
  bool in_scaleoffset = (input->scale != NULL);
  bool out_scaleoffset = (output->scale != NULL);
  float in_scale = in_scaleoffset ? input->scale[0] : 0;
  int8_t in_offset = in_scaleoffset ? input->offset[0] : 0;
  float out_scale = out_scaleoffset ? output->scale[0] : 0;
  int8_t out_offset = out_scaleoffset ? output->offset[0] : 0;

  // LL_ATON_PRINTF("in: type=%d Qm=%d Qn=%d nb=%d\n",dtype_in,Qm_in,Qn_in,nbits_in);
  // LL_ATON_PRINTF("out: type=%d Qm=%d Qn=%d nb=%d\n",dtype_out,Qm_out,Qn_out,nbits_out);
  // we convert integer types to QMN to use the same (inefficient) code
  __dtype_convert_to_QMN(&dtype_in, &Qm_in, &Qn_in, nbits_in);
  __dtype_convert_to_QMN(&dtype_out, &Qm_out, &Qn_out, nbits_out);
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
#if __LL_LIB_Concat_Cast_USE_ATON_HW
      __LL_ATON_LIB_DMA_Inputs_Memcpy(input, 1, (void *)LL_Buffer_addr_start(output), in_byte_size, dma_in, dma_out,
                                      nn_instance);
#else  // !__LL_LIB_Concat_Cast_USE_ATON_HW
      memcpy((void *)LL_Buffer_addr_start(output), (void *)LL_Buffer_addr_start(input), in_byte_size);
#endif // !__LL_LIB_Concat_Cast_USE_ATON_HW
    }
    // else LL_ATON_PRINTF("Cast: nothing to do\n");
    return LL_ATON_OK;
  }

  if (dtype_in == DataType_FXP && dtype_out == DataType_FLOAT)
  { // from to Qmn and/or scale/offset to float
    unsigned int i;
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
        float f = in_scaleoffset ? __scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset)
                                 : __Q_to_floating(t, Qm_in, Qn_in);
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
        float f = in_scaleoffset ? __scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset)
                                 : __Q_to_floating(t, Qm_in, Qn_in);
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
        float f = in_scaleoffset ? __scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset)
                                 : __Q_to_floating(t, Qm_in, Qn_in);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        *out-- = f;
        bitcnt -= nbits;
      }
    }
    }
  }
  else if (dtype_in == DataType_FLOAT && dtype_out == DataType_FXP)
  { // from to float to Qmn and/or scale offset
    unsigned int i;
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
        int t = out_scaleoffset ? __floating_to_scale_offset(f, Qm_out, Qn_out, out_scale, out_offset)
                                : __floating_to_Q(f, Qm_out, Qn_out);
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
        int t = out_scaleoffset ? __floating_to_scale_offset(f, Qm_out, Qn_out, out_scale, out_offset)
                                : __floating_to_Q(f, Qm_out, Qn_out);
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
        int t = out_scaleoffset ? __floating_to_scale_offset(f, Qm_out, Qn_out, out_scale, out_offset)
                                : __floating_to_Q(f, Qm_out, Qn_out);
        // LL_ATON_PRINTF("i=%d f=%0.2f t=%d \n", i, f, t);
        LL_ATON_setbits(out, bitcnt, nbits, t);
        bitcnt += nbits;
      }
    }
    }
  }
  else
  {
    // the following code is very inefficient for integer types, specific code should be implemented for those FIXME
    // !!!

    if (dtype_in == DataType_FXP && dtype_out == DataType_FXP)
    {                                    // from to Qmn to Qmn assumes max in/out bits = 16
      int fwd = (nbits_in >= nbits_out); // forward
      int in_bitsinc = fwd ? nbits_in : -nbits_in;
      int out_bitsinc = fwd ? nbits_out : -nbits_out;
      int in_bitcnt = fwd ? 0 : in_bit_size * (in_elements - 1);
      int out_bitcnt = fwd ? 0 : out_bit_size * (out_elements - 1);
      uint32_t *in = (uint32_t *)LL_Buffer_addr_start(input);
      uint32_t *out = (uint32_t *)LL_Buffer_addr_start(output);
      uint32_t tmask = (~(0xFFFFFFFFU << nbits_out)); //  create a mask with output precision
      unsigned int i;
      for (i = 0; i < in_elements; i++)
      {
        int t = LL_ATON_getbits(in, in_bitcnt, nbits_in); // note t is sign extended to int
        int tM = 0;
        if (!in_scaleoffset && !out_scaleoffset)
          tM = (Qn_out >= Qn_in ? (t << (Qn_out - Qn_in)) : (t << (Qn_in - Qn_out))); // align to output mantissa
        if (in_scaleoffset && !out_scaleoffset)
          tM = __scale_offset_to_Q(t, Qm_in, Qn_in, in_scale, in_offset, Qm_out, Qn_out);
        if (!in_scaleoffset && out_scaleoffset)
          tM = __Q_to_scale_offset(t, Qm_in, Qn_in, Qm_out, Qn_out, out_scale, out_offset);
        if (in_scaleoffset && out_scaleoffset)
        {
          // very inefficient, FIXME
          float fval = in_scaleoffset ? __scale_offset_to_floating(t, Qm_in, Qn_in, in_scale, in_offset) : t;
          tM = out_scaleoffset ? __floating_to_scale_offset(fval, Qm_out, Qn_out, out_scale, out_offset) : (int)fval;
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

int LL_ATON_LIB_Softmax(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                        unsigned int axis, int legacy)
{
  unsigned int in_elements = __LL_LIB_TENSOR_ELEMENTS(input);
  unsigned int out_elements = __LL_LIB_TENSOR_ELEMENTS(output);
  unsigned int el_size = input->type == DataType_FLOAT ? 4 : 1;
  unsigned int in_byte_size = (in_elements * el_size * 8) >> 3;
  unsigned int out_byte_size = (out_elements * el_size * 8) >> 3;

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
    return legacy ? __LL_ATON_LIB_Softmax_INT8_legacy(input, output, axis)
                  : __LL_ATON_LIB_Softmax_INT8(input, output, axis);
  }
  if (input->type == DataType_FLOAT)
  {
    return legacy ? __LL_ATON_LIB_Softmax_float_legacy(input, output, axis)
                  : __LL_ATON_LIB_Softmax_float(input, output, axis);
  }

  return LL_ATON_INVALID_PARAM;
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
 * @param  nn_instance pointer to network instance (may not be `NULL`)
 * @return Error code
 */
int LL_ATON_LIB_DMA_Outputs_Slice_SplitLike(const LL_Buffer_InfoTypeDef *input, const LL_Buffer_InfoTypeDef *output,
                                            uint32_t tot_out_size, uint32_t width_in_bytes, uint32_t fheight,
                                            uint32_t line_offset, uint8_t n_bits, int dma_in, int dma_out,
                                            const NN_Instance_TypeDef *nn_instance)
{
  // Do actual copy
  if (tot_out_size < __LL_DMA_MIN_BUFF_LEN)
  {
    for (unsigned int source = 0, dest = 0; dest < tot_out_size; source += line_offset, dest += width_in_bytes)
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

    /* become ATON owner */
    __ll_set_aton_owner(nn_instance);

    /* save DMA configurations */
    __ll_lib_params_t *params = __ll_lib_get_params();
    params->g_dma_in = _dma_in;
    params->g_dma_out = _dma_out;

    /* configure stream switch */
    __ll_lib_strswitch_set_dmas(dma_in, dma_out, __ll_internal_simple_oneshot_transfer);

    LL_ATON_RT_Insert_LibEpochBlockArray(__ll_internal_simple_oneshot_transfer);
  }

  return LL_ATON_OK;
}

/*
 * `Pad` operator related functions.
 * (Moved gere from `ll_aton_sw_operators.[ch]` to avoid issues related to relocatable)
 */

/**
 * @brief  performs a `Pad` operation on a (multi-dimensional) matrix
 * @param  input start address of input tensor
 * @param  output start address of output tensor
 * @param  min_shape shape obtained by performing element-wise `min` on each axis of reduced input vs output tensor
 * @param  mode where 0 == `constant`, 1 == `reflect`, 2 ==`edge`
 * @param  nbytes 8-bit or 16-bit mode
 * @param  out_elems number of output elements
 * @param  constant_value constant value to be `memset`
 * @param  consecutive_axis last input axis with all zero start/end paddings and whose following axes also have all
 * zero paddings (or last axis)
 * @param  consecutive_elems number of consecutive elements to copy in each repetition
 * @param  pad_in_offsets_start vector containing amount (in bytes) of input padding data added at beginning of each
 * axis
 * @param  pad_in_offsets_end vector containing amount (in bytes) of input padding data added at and of each axis
 * @param  pad_out_offsets_start vector containing amount (in bytes) of output padding data added at beginning of each
 * axis
 * @param  pad_out_offsets_end vector containing amount (in bytes) of output padding data added at and of each axis
 * @retval Error code
 */
static void *__ll_aton_lib_memset16(void *dst, int16_t c, size_t n)
{
  uintptr_t rest_addr = ((uintptr_t)dst) % 4;
  int8_t *_dst = (int8_t *)dst;

  LL_ATON_ASSERT((rest_addr == 0) || (rest_addr == 2));
  if (rest_addr != 0)
  {
    *((int16_t *)dst) = (int16_t)c;
    n -= 2;
    _dst += 2;
  }

  uint32_t rest_dim = n % 4;
  LL_ATON_ASSERT((rest_dim == 0) || (rest_dim == 2));
  if (rest_dim != 0)
  {
    *((int16_t *)(_dst + n - 2)) = (int16_t)c;
    n -= 2;
  }

  LL_ATON_ASSERT((n % 4) == 0);
  LL_ATON_ASSERT((n == 0) || ((((uintptr_t)_dst) % 4) == 0));

  if (n > 0)
  {
    int32_t repetition_constant = (c << 16) | (0xFFFF & c);
    for (unsigned int i = 0; i < n; i += 4)
    {
      *((int32_t *)(_dst + i)) = (int32_t)repetition_constant;
    }
  }

  return dst;
}

static void *__ll_aton_lib_memset24(void *dst, int32_t c, size_t n) // NOTE: assuming no alignment
{
  int8_t *_dst = (int8_t *)dst;
  int32_t num_elems = (n / 3);
  LL_ATON_ASSERT((n % 3) == 0);

  for (int i = 0; i < num_elems; i++)
  {
    *_dst++ = c & 0xFF;
    *_dst++ = (c >> 8) & 0xFF;
    *_dst++ = (c >> 16) & 0xFF;
  }

  return dst;
}

static void *__ll_aton_lib_memset32(void *dst, int32_t c, size_t n)
{
  int32_t *_dst = (int32_t *)dst;
  int32_t num_elems = (n / 4);

  LL_ATON_ASSERT((((uintptr_t)dst) % 4) == 0);
  LL_ATON_ASSERT((n % 4) == 0);

  for (int i = 0; i < num_elems; i++)
  {
    *_dst++ = c;
  }

  return dst;
}

static inline void *__ll_aton_lib_memset(uint8_t nbytes, void *dst, int32_t c, size_t n)
{
  if (n > 0)
  {
    switch (nbytes)
    {
    case 1:
      return memset(dst, c, n);
    case 2:
      return __ll_aton_lib_memset16(dst, c, n);
    case 3:
      return __ll_aton_lib_memset24(dst, c, n);
    case 4:
      return __ll_aton_lib_memset32(dst, c, n);
    default:
      LL_ATON_ASSERT(false);
      return dst;
    }
  }
  else
  {
    return dst;
  }
}

static inline void __ll_aton_lib_reflect_inner_framing_sw(uint32_t curr_axis, uint8_t nbytes, int8_t *dst, int8_t *src,
                                                          int32_t n_elems, int32_t length, bool start)
{
  LL_ATON_LIB_UNUSED(curr_axis);
  LL_ATON_ASSERT(n_elems > 0);
  if (nbytes == 2)
  {
    LL_ATON_ASSERT((((uintptr_t)src) % 2) == 0);
  }
  if (nbytes == 4)
  {
    LL_ATON_ASSERT((((uintptr_t)src) % 4) == 0);
  }

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): reflect, curr_axis=%d, dst=%p, src=%p, min_shape_elems=%d, bytes=%d\n", __func__, __LINE__,
                 curr_axis, dst, src, n_elems, length);
#if (ATON_PLAT_HAS_FFLUSH)
  LL_ATON_FFLUSH(stdout);
#endif
#endif // DUMP_DEBUG_SW_OPS

  bool forward = start;

  if (start)
  {
    int8_t *dst_ptr = dst + length - nbytes;
    int32_t src_idx = (n_elems > 1) ? 1 : 0;

    for (; length > 0; length -= nbytes)
    {
      LL_ATON_ASSERT(dst_ptr >= dst);
      LL_ATON_ASSERT(src_idx < n_elems);
      LL_ATON_ASSERT(src_idx >= 0);

#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): reflect, curr_axis=%d, dst=%p, src=%p, src_idx=%d, nbytes=%d\n", __func__, __LINE__,
                     curr_axis, dst_ptr, src, src_idx, nbytes);
#endif

      int8_t *src_ptr = (int8_t *)(&((int8_t *)src)[src_idx * nbytes]);
      __ll_aton_lib_copy_element(nbytes, src_idx, dst_ptr, src_ptr);

      dst_ptr -= nbytes;
      if (forward)
      {
        if (src_idx >= (n_elems - 1))
        {
          forward = false;
          src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
        }
      }
      else
      {
        if (src_idx <= 0)
        {
          forward = true;
          src_idx = (n_elems > 1) ? 1 : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
        }
      }
    }
  }
  else
  {
    int8_t *dst_ptr = dst;
#ifndef NDEBUG
    int8_t *last_ptr = dst + length;
#endif

    int32_t src_idx = (n_elems > 1) ? -1 : 0;
    for (; length > 0; length -= nbytes)
    {
      LL_ATON_ASSERT(dst_ptr < last_ptr);
      LL_ATON_ASSERT((-src_idx) < n_elems);
      LL_ATON_ASSERT(src_idx <= 0);

      int8_t *src_ptr = (int8_t *)(&((int8_t *)src)[src_idx * nbytes]);
      __ll_aton_lib_copy_element(nbytes, src_idx, dst_ptr, src_ptr);

      dst_ptr += nbytes;
      if (forward)
      {
        if (src_idx >= 0)
        {
          forward = false;
          src_idx = (n_elems > 1) ? -1 : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
        }
      }
      else
      {
        if ((-src_idx) >= (n_elems - 1))
        {
          forward = true;
          src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
        }
        else
        {
          src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
        }
      }
    }
  }
}

static inline void __ll_aton_lib_edge_framing_sw(uint32_t curr_axis, uint8_t nbytes, void *dst, void *src,
                                                 int32_t length)
{
  LL_ATON_LIB_UNUSED(curr_axis);

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): memset, curr_axis=%d, dst=%p, src=%p, bytes=%d\n", __func__, __LINE__, curr_axis, dst, src,
                 length);
#endif
  __ll_aton_lib_memset(nbytes, dst, *((int32_t *)src), length);
}

static void __ll_aton_lib_pad_filling_sw(uint32_t curr_axis, __ll_pad_sw_params_t *common_params)
{
  LL_ATON_ASSERT(curr_axis <= common_params->consecutive_axis);

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): in=%lx, out=%lx, curr_axis=%u, min_dim=%u, in_start=%d, out_start=%d\n", __func__, __LINE__,
                 (uintptr_t)common_params->in_target, (uintptr_t)common_params->out_target, curr_axis,
                 common_params->min_shape[curr_axis], common_params->pad_in_offsets_start[curr_axis],
                 common_params->pad_out_offsets_start[curr_axis]);
#endif

  if (common_params->pad_in_offsets_start[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_start[curr_axis];
  }

  if (common_params->pad_out_offsets_start[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_start[curr_axis];
  }

  if (curr_axis == common_params->consecutive_axis)
  {
#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): ASSIGN in=%lx, out=%lx, bytes=%u\n", __func__, __LINE__,
                   (uintptr_t)common_params->in_target, (uintptr_t)common_params->out_target,
                   common_params->consecutive_bytes);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif

    memcpy(common_params->out_target, common_params->in_target, common_params->consecutive_bytes);
    common_params->in_target += common_params->consecutive_bytes;
    common_params->out_target += common_params->consecutive_bytes;

    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }
  else
  {
    for (uint32_t i = 0; i < common_params->min_shape[curr_axis]; i++)
    {
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): i=%" PRIu32 "\n", __func__, __LINE__, i);
#endif

      __ll_aton_lib_pad_filling_sw(curr_axis + 1, common_params);
    }
  }

  if (common_params->pad_out_offsets_end[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_end[curr_axis];
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }

  if (common_params->pad_in_offsets_end[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_end[curr_axis];
  }

#if defined(DUMP_DEBUG_SW_OPS)
  LL_ATON_PRINTF("%s(%d): curr_axis=%u, in_end=%d, out_end=%d\n", __func__, __LINE__, curr_axis,
                 common_params->pad_in_offsets_end[curr_axis], common_params->pad_out_offsets_end[curr_axis]);
#endif
}

static void __ll_aton_lib_pad_reflect_sw(uint32_t curr_axis, __ll_pad_sw_params_t *common_params, bool fill)
{
  LL_ATON_ASSERT(curr_axis <= (common_params->tensor_rank - 1));
  LL_ATON_ASSERT(common_params->pad_in_offsets_start[curr_axis] >= 0);

  int8_t *curr_out_ptr = common_params->out_target;

  if (common_params->pad_out_offsets_start[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_start[curr_axis];
  }

  if (curr_axis == (common_params->tensor_rank - 1))
  {
    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      __ll_aton_lib_reflect_inner_framing_sw(curr_axis, common_params->nbytes, curr_out_ptr, common_params->in_target,
                                             common_params->min_shape[curr_axis],
                                             common_params->pad_out_offsets_start[curr_axis], true);
    }

    int32_t filling_bytes = common_params->min_shape[curr_axis] * common_params->nbytes;

    if (fill)
    {
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d\n", __func__, __LINE__, curr_axis,
                     (int8_t *)common_params->out_target, (int8_t *)common_params->in_target, filling_bytes);
#endif

      memcpy(common_params->out_target, common_params->in_target, filling_bytes);
    }

    common_params->in_target += filling_bytes;
    common_params->out_target += filling_bytes;

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }
  else
  {
    for (unsigned int i = 0; i < common_params->min_shape[curr_axis]; i++)
    {
      __ll_aton_lib_pad_reflect_sw(curr_axis + 1, common_params, fill);
    }

    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      int32_t pad_start = common_params->pad_out_offsets_start[curr_axis] /
                          common_params->out_offsets[curr_axis]; // i.e.: pads_start[curr_axis];
      int32_t pad_end = common_params->pad_out_offsets_end[curr_axis] /
                        common_params->out_offsets[curr_axis]; // i.e.: pads_end[curr_axis];

      LL_ATON_ASSERT(pad_end >= 0);

      int32_t nr_loops = pad_start;
      int32_t n_elems = common_params->out_shape[curr_axis] - pad_start - pad_end;
      int32_t padding_bytes = common_params->out_offsets[curr_axis];

      int8_t *dst_ptr = curr_out_ptr + common_params->pad_out_offsets_start[curr_axis] - padding_bytes;
      int8_t *src_ptr = curr_out_ptr + common_params->pad_out_offsets_start[curr_axis];

      int32_t src_idx = (n_elems > 1) ? 1 : 0;

      bool forward = true;
      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, src_idx=%d, bytes=%d, n_elems=%d, times=%d/%d\n",
                       __func__, __LINE__, curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr + (src_idx * padding_bytes),
                       src_idx, padding_bytes, n_elems, i + 1, nr_loops);
#if (ATON_PLAT_HAS_FFLUSH)
        LL_ATON_FFLUSH(stdout);
#endif
#endif
        LL_ATON_ASSERT(dst_ptr >= curr_out_ptr);
        LL_ATON_ASSERT(src_idx < n_elems);
        LL_ATON_ASSERT(src_idx >= 0);

        memcpy(dst_ptr, src_ptr + (src_idx * padding_bytes), padding_bytes);

        dst_ptr -= padding_bytes;

        if (forward)
        {
          if (src_idx >= (n_elems - 1))
          {
            forward = false;
            src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
          }
        }
        else
        {
          if (src_idx <= 0)
          {
            forward = true;
            src_idx = (n_elems > 1) ? 1 : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
          }
        }
      }
    }
  }

  if (common_params->pad_out_offsets_end[curr_axis] > 0)
  {
    if (curr_axis == (common_params->tensor_rank - 1))
    {
      int8_t *src_ptr = common_params->out_target - common_params->nbytes;
      __ll_aton_lib_reflect_inner_framing_sw(curr_axis, common_params->nbytes, common_params->out_target, src_ptr,
                                             common_params->min_shape[curr_axis],
                                             common_params->pad_out_offsets_end[curr_axis], false);
    }
    else
    {
      int32_t pad_start = common_params->pad_out_offsets_start[curr_axis] /
                          common_params->out_offsets[curr_axis]; // i.e.: pads_start[curr_axis];
      int32_t pad_end = common_params->pad_out_offsets_end[curr_axis] /
                        common_params->out_offsets[curr_axis]; // i.e.: pads_end[curr_axis];

      int32_t nr_loops = pad_end;

      int32_t padding_bytes = common_params->out_offsets[curr_axis];
      int32_t n_elems = common_params->out_shape[curr_axis] - __ll_zero_neg(pad_start) - __ll_zero_neg(pad_end);

      int8_t *dst_ptr = common_params->out_target;
      int8_t *src_ptr = common_params->out_target - padding_bytes;

      int32_t src_idx = (n_elems > 1) ? -1 : 0;

      bool forward = false;
      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, src_idx=%d, bytes=%d, n_elems=%d, times=%d/%d\n",
                       __func__, __LINE__, curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr + (src_idx * padding_bytes),
                       src_idx, padding_bytes, n_elems, i + 1, nr_loops);
#if (ATON_PLAT_HAS_FFLUSH)
        LL_ATON_FFLUSH(stdout);
#endif
#endif
        LL_ATON_ASSERT(dst_ptr < (dst_ptr + common_params->pad_out_offsets_end[curr_axis]));
        LL_ATON_ASSERT((-src_idx) < n_elems);
        LL_ATON_ASSERT(src_idx <= 0);

        memcpy(dst_ptr, src_ptr + (src_idx * padding_bytes), padding_bytes);

        dst_ptr += padding_bytes;
        if (forward)
        {
          if (src_idx >= 0)
          {
            forward = false;
            src_idx = (n_elems > 1) ? -1 : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
          }
        }
        else
        {
          if ((-src_idx) >= (n_elems - 1))
          {
            forward = true;
            src_idx = (n_elems > 1) ? (src_idx + 1) : 0;
          }
          else
          {
            src_idx = (n_elems > 1) ? (src_idx - 1) : 0;
          }
        }
      }
    }

    common_params->out_target += common_params->pad_out_offsets_end[curr_axis];

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }

  LL_ATON_ASSERT(common_params->pad_in_offsets_end[curr_axis] >= 0);
}

static void __ll_aton_lib_pad_edge_sw(uint32_t curr_axis, __ll_pad_sw_params_t *common_params, bool fill)
{
  LL_ATON_ASSERT(curr_axis <= (common_params->tensor_rank - 1));

  if (common_params->pad_in_offsets_start[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_start[curr_axis];
  }

  int8_t *curr_out_ptr = common_params->out_target;

  if (common_params->pad_out_offsets_start[curr_axis] > 0)
  {
    common_params->out_target += common_params->pad_out_offsets_start[curr_axis];
  }

  if (curr_axis == (common_params->tensor_rank - 1))
  {
    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      __ll_aton_lib_edge_framing_sw(curr_axis, common_params->nbytes, curr_out_ptr, common_params->in_target,
                                    common_params->pad_out_offsets_start[curr_axis]);
    }

    int32_t filling_bytes = common_params->min_shape[curr_axis] * common_params->nbytes;

    if (fill)
    {
#if defined(DUMP_DEBUG_SW_OPS)
      LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d\n", __func__, __LINE__, curr_axis,
                     (int8_t *)common_params->out_target, (int8_t *)common_params->in_target, filling_bytes);
#endif

      memcpy(common_params->out_target, common_params->in_target, filling_bytes);
    }

    common_params->in_target += filling_bytes;
    common_params->out_target += filling_bytes;

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }
  else
  {
    for (unsigned int i = 0; i < common_params->min_shape[curr_axis]; i++)
    {
      __ll_aton_lib_pad_edge_sw(curr_axis + 1, common_params, fill);
    }

    if (common_params->pad_out_offsets_start[curr_axis] > 0)
    {
      int32_t nr_loops = common_params->pad_out_offsets_start[curr_axis] /
                         common_params->out_offsets[curr_axis]; // i.e.: pads_start[curr_axis];
      int32_t padding_bytes = common_params->pad_out_offsets_start[curr_axis] / nr_loops;

      int8_t *dst_ptr = curr_out_ptr;
      int8_t *src_ptr = curr_out_ptr + common_params->pad_out_offsets_start[curr_axis];

      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d, times=%d/%d\n", __func__, __LINE__,
                       curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr, padding_bytes, i + 1, nr_loops);
#endif
        memcpy(dst_ptr, src_ptr, padding_bytes);
        dst_ptr += padding_bytes;
      }
    }
  }

  if (common_params->pad_out_offsets_end[curr_axis] > 0)
  {
    if (curr_axis == (common_params->tensor_rank - 1))
    {
      __ll_aton_lib_edge_framing_sw(curr_axis, common_params->nbytes, common_params->out_target,
                                    (common_params->out_target - common_params->nbytes),
                                    common_params->pad_out_offsets_end[curr_axis]);
    }
    else
    {
      int32_t nr_loops = common_params->pad_out_offsets_end[curr_axis] /
                         common_params->out_offsets[curr_axis]; // i.e. pads_end[curr_axis];
      int32_t padding_bytes = common_params->pad_out_offsets_end[curr_axis] / nr_loops;

      int8_t *dst_ptr = common_params->out_target;
      int8_t *src_ptr = common_params->out_target - (common_params->pad_out_offsets_end[curr_axis] / nr_loops);

      for (int i = 0; i < nr_loops; i++)
      {
#if defined(DUMP_DEBUG_SW_OPS)
        LL_ATON_PRINTF("%s(%d): memcpy, curr_axis=%d, dst=%p, src=%p, bytes=%d, times=%d/%d\n", __func__, __LINE__,
                       curr_axis, (int8_t *)dst_ptr, (int8_t *)src_ptr, padding_bytes, i + 1, nr_loops);
#endif
        memcpy(dst_ptr, src_ptr, padding_bytes);
        dst_ptr += padding_bytes;
      }
    }

    common_params->out_target += common_params->pad_out_offsets_end[curr_axis];

#if defined(DUMP_DEBUG_SW_OPS)
    LL_ATON_PRINTF("%s(%d): out=%p, end=%p\n", __func__, __LINE__, (int8_t *)common_params->out_target,
                   (int8_t *)common_params->end_out_target);
#if (ATON_PLAT_HAS_FFLUSH)
    LL_ATON_FFLUSH(stdout);
#endif
#endif
    LL_ATON_ASSERT(common_params->out_target <= common_params->end_out_target);
  }

  if (common_params->pad_in_offsets_end[curr_axis] < 0)
  {
    common_params->in_target -= common_params->pad_in_offsets_end[curr_axis];
  }
}

static int __ll_aton_lib_pad_filling(__ll_pad_sw_params_t *common_params, const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
#endif // !NDEBUG

#if defined(DUMP_RESULTS_PAD_OP)
  int8_t *out_target = common_params->out_target;
#endif // DUMP_RESULTS_PAD_OP

  /* check if there is something to do */
  if (common_params->consecutive_bytes == 0)
  {
    goto dump_results; // nothing needs to be copied/filled in (just `memset`), so we are done!
  }

  /* fill with content */
  if ((common_params->consecutive_bytes < __LL_PAD_FILLING_DMA_MIN_BUFF_LEN) ||
      (common_params->tensor_rank > __LL_DMA_PAD_MAX_DIMS))
  {                                               // do it without HW support
    if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
    {
      __ll_pad_sw_params_t copied_params = *common_params;

      /* Release ATON lock */
      __ll_clear_aton_owner(nn_instance, false);

      __ll_aton_lib_pad_filling_sw(0, &copied_params);
    }
    else
    {
      __ll_aton_lib_pad_filling_sw(0, common_params);
    }

    if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
    {
      if ((common_params->end_out_target - common_params->saved_out_target) > 0)
      {
        /* *** MCU cache clean operation (SW) *** */
        uint32_t size = (uintptr_t)(common_params->end_out_target) - (uintptr_t)(common_params->saved_out_target);
        LL_ATON_Cache_MCU_Clean_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->saved_out_target),
                                      size);
      }
    }

  dump_results:
#if defined(DUMP_RESULTS_PAD_OP)
    /* debug output print */
    switch (common_params->nbytes)
    {
    case 1:
    {
      int8_t *ptr = (int8_t *)out_target;
      for (uint32_t i = 0; i < common_params->out_size; i++)
      {
        LL_ATON_PRINTF("%d\n", ptr[i]);
      }
    }
    break;
    case 2:
    {
      int16_t *ptr = (int16_t *)(int8_t *)out_target;
      for (uint32_t i = 0; i < common_params->out_size / 2; i++)
      {
        LL_ATON_PRINTF("%d\n", ptr[i]);
      }
    }
    break;
    case 3: // NOTE: assuming no alignment
    {
      /* check endianness */
      const int32_t _const_val = 0x01020304;
      const int8_t *_const_val_ptr = (int8_t *)&_const_val;
      bool is_little_endian = (_const_val_ptr[0] == 0x04);

      int8_t *ptr = (int8_t *)out_target;
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
      int32_t *ptr = (int32_t *)(int8_t *)out_target;
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

    return LL_ATON_OK;
  }
  else
  {                                               // perform second phase in HW
    if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
    {
      common_params->callback_function = NULL;
      return __LL_ATON_LIB_DMA_Pad_Filling(NULL, nn_instance);
    }
    else
    { // not a callback => `memset` (i.e. `framing`) has been executed in SW
      if ((common_params->end_out_target - common_params->saved_out_target) > 0)
      {
        /* *** MCU cache clean & invalidate operation (SW) *** */
        uint32_t size = (uintptr_t)(common_params->end_out_target) - (uintptr_t)(common_params->saved_out_target);
        LL_ATON_Cache_MCU_Clean_Invalidate_Range(
            ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->saved_out_target), size);
      }

      return __LL_ATON_LIB_DMA_Pad_Filling(common_params, nn_instance);
    }
  }
}

static int __ll_aton_lib_pad_framing_sw(__ll_pad_sw_params_t *common_params, const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
  LL_ATON_ASSERT(__ll_current_aton_ip_owner != NULL);
  LL_ATON_ASSERT((common_params != NULL) &&
                 (common_params->callback_function != NULL)); /* always "called as callback" */
#endif                                                        // !NDEBUG

  /* reset input/output target pointers */
  common_params->in_target = common_params->saved_in_target;
  common_params->out_target = common_params->saved_out_target;

#if defined(DUMP_RESULTS_PAD_OP)
  int8_t *out_target = common_params->out_target;
#endif // DUMP_RESULTS_PAD_OP

  /* copy params */
  __ll_pad_sw_params_t copied_params = *common_params;

  /* Release ATON lock */
  __ll_clear_aton_owner(nn_instance, false);

  switch (common_params->mode)
  {
  case 1: // `reflect` mode
    __ll_aton_lib_pad_reflect_sw(0, &copied_params, false);
    break;

  case 2: // `edge` mode
    __ll_aton_lib_pad_edge_sw(0, &copied_params, false);
    break;

  default:
    __LL_LIB_ERROR(_ERR_MODE, LL_ATON_INVALID_PARAM)
  }

  if ((copied_params.end_out_target - copied_params.saved_out_target) > 0)
  {
    /* *** MCU cache clean operation (SW) *** */
    uint32_t size = (uintptr_t)(copied_params.end_out_target) - (uintptr_t)(copied_params.saved_out_target);
    LL_ATON_Cache_MCU_Clean_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)copied_params.saved_out_target), size);
  }

#if defined(DUMP_RESULTS_PAD_OP)
  /* debug output print */
  switch (copied_params.nbytes)
  {
  case 1:
  {
    int8_t *ptr = (int8_t *)out_target;
    for (uint32_t i = 0; i < copied_params.out_size; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 2:
  {
    int16_t *ptr = (int16_t *)(int8_t *)out_target;
    for (uint32_t i = 0; i < copied_params.out_size / 2; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 3: // NOTE: assuming no alignment
  {
    /* check endianness */
    const int32_t _const_val = 0x01020304;
    const int8_t *_const_val_ptr = (int8_t *)&_const_val;
    bool is_little_endian = (_const_val_ptr[0] == 0x04);

    int8_t *ptr = (int8_t *)out_target;
    if (is_little_endian)
    {
      for (uint32_t i = 0; i < copied_params.out_size / 3; i += 3)
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
      for (uint32_t i = 0; i < copied_params.out_size / 3; i += 3)
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
    int32_t *ptr = (int32_t *)(int8_t *)out_target;
    for (uint32_t i = 0; i < copied_params.out_size / 4; i++)
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

  return LL_ATON_OK;
}

static int __ll_aton_lib_pad_4loop_filling(__ll_pad_sw_params_t *common_params, const NN_Instance_TypeDef *nn_instance)
{
#ifndef NDEBUG
  extern const NN_Instance_TypeDef *volatile __ll_current_aton_ip_owner;
#endif // !NDEBUG

  if (common_params->callback_function != NULL) /* take this as indication for "called as callback" */
  {
    LL_ATON_ASSERT(__ll_current_aton_ip_owner == nn_instance);
    return __LL_ATON_LIB_DMA_Pad_4Loop_Filling(NULL, nn_instance);
  }
  else // not a callback => `memset` (i.e. `framing`) has been executed in SW
  {
    if ((common_params->out_end - common_params->out_target) > 0)
    {
      /* *** MCU cache clean & invalidate operation (SW) *** */
      uint32_t size = (uintptr_t)(common_params->out_end) - (uintptr_t)(common_params->out_target);
      LL_ATON_Cache_MCU_Clean_Invalidate_Range(ATON_LIB_PHYSICAL_TO_VIRTUAL_ADDR((uintptr_t)common_params->out_target),
                                               size);
    }

    /* Become ATON owner */
    __ll_set_aton_owner(nn_instance);

    return __LL_ATON_LIB_DMA_Pad_4Loop_Filling(common_params, nn_instance);
  }
}

int LL_ATON_LIB_Pad_4Loop(unsigned char *input_start, unsigned char *input_end, unsigned char *input_limit,
                          unsigned char *output_start, unsigned char *output_end, unsigned char *output_limit,
                          int32_t constant_value, uint8_t nbytes, uint32_t *negative_4loop, uint32_t *positive_4loop,
                          int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_ASSERT(dma_in >= 0);
  LL_ATON_ASSERT(dma_out >= 0);
  LL_ATON_ASSERT(positive_4loop != NULL);

  __ll_pad_sw_params_t common_params = {
      .in_target = (int8_t *)input_start,
      .in_end = (int8_t *)input_end,
      .in_limit = (int8_t *)input_limit,
      .out_target = (int8_t *)output_start,
      .out_end = (int8_t *)output_end,
      .out_limit = (int8_t *)output_limit,
      .nbytes = nbytes,
      .callback_function = NULL,
      .dma_in = dma_in,
      .dma_out = dma_out,
      .negative_4loop = *((struct four_axes *)negative_4loop),
      .positive_4loop = *((struct four_axes *)positive_4loop),
  };

  /* fill output with constant value
     (TODO: trade-off between one shot and multiple single short shots - without overwriting non constant value
     ranges - still to be evaluated) */
  size_t out_size = (size_t)(output_end - output_start);
  if (out_size < __LL_PAD_FRAMING_DMA_MIN_BUFF_LEN)
  {
    __ll_aton_lib_memset(nbytes, (int8_t *)output_start, constant_value, out_size);
    return __ll_aton_lib_pad_4loop_filling(&common_params, nn_instance);
  }
  else
  {
    common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_4loop_filling;
    return __LL_ATON_LIB_DMA_Pad_Memset((int8_t *)output_start, constant_value, out_size, &common_params, false,
                                        nn_instance); // first phase (in this case `framing`) is made in HW
  }
}

int LL_ATON_LIB_Pad_Standard(unsigned char *input, unsigned char *output, unsigned char *input_limit,
                             unsigned char *output_limit, const uint32_t *min_shape, uint8_t mode, uint8_t nbytes,
                             uint32_t out_elems, int32_t constant_value, uint32_t consecutive_axis,
                             uint32_t consecutive_elems, const int32_t *pad_in_offsets_start,
                             const int32_t *pad_in_offsets_end, const int32_t *pad_out_offsets_start,
                             const int32_t *pad_out_offsets_end, const int32_t *out_shape, const int32_t *out_offsets,
                             size_t tensor_rank, int dma_in, int dma_out, const NN_Instance_TypeDef *nn_instance)
{
  LL_ATON_ASSERT(dma_in >= 0);
  LL_ATON_ASSERT(dma_out >= 0);

  size_t out_size = out_elems * nbytes;
  uint32_t consecutive_bytes = consecutive_elems * nbytes;

  __ll_pad_sw_params_t common_params = {
    .in_target = (int8_t *)input,
    .out_target = (int8_t *)output,
    .in_limit = (int8_t *)input_limit,
    .out_limit = (int8_t *)output_limit,
    .nbytes = nbytes,
    .mode = mode,
    .tensor_rank = tensor_rank,
    .saved_in_target = (int8_t *)input,
    .saved_out_target = (int8_t *)output,
    .end_out_target = (int8_t *)output + out_size,
    .callback_function = NULL,
    .consecutive_axis = consecutive_axis,
    .consecutive_bytes = consecutive_bytes,
    .dma_in = dma_in,
    .dma_out = dma_out,
#if defined(DUMP_RESULTS_PAD_OP)
    .out_size = out_size,
#endif

    .min_shape = min_shape,
    .pad_in_offsets_start = pad_in_offsets_start,
    .pad_in_offsets_end = pad_in_offsets_end,
    .pad_out_offsets_start = pad_out_offsets_start,
    .pad_out_offsets_end = pad_out_offsets_end,
    .out_shape = out_shape,
    .out_offsets = out_offsets,
  };

  if (consecutive_bytes == 0)
  {
    // corner case: nothing needs to be copied/filled in (just `memset`), align with `onnxruntime (v1.2.0)`
    // reference implementation
    constant_value = 0;
  }

  switch (mode)
  {
  case 0: // `constant` mode
    /* fill output with constant value
       (TODO: trade-off between one shot and multiple single short shots - without overwriting non constant value
       ranges - still to be evaluated) */
    if ((out_size < __LL_PAD_FRAMING_DMA_MIN_BUFF_LEN) || (tensor_rank > __LL_DMA_PAD_MAX_DIMS))
    {
      __ll_aton_lib_memset(nbytes, (int8_t *)output, constant_value, out_size);
      return __ll_aton_lib_pad_filling(&common_params, nn_instance);
    }
    else
    {
      common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_filling;
      return __LL_ATON_LIB_DMA_Pad_Memset((int8_t *)output, constant_value, out_size, &common_params, true,
                                          nn_instance); // first phase (in this case `framing`) is made in HW
    }

  case 1: // `reflect` mode
    if ((consecutive_bytes < __LL_PAD_FILLING_DMA_MIN_BUFF_LEN) || (tensor_rank > __LL_DMA_PAD_MAX_DIMS))
    {                                                        // do it without HW support
      __ll_aton_lib_pad_reflect_sw(0, &common_params, true); // all (i.e. `filling` & `framing`) is done in SW
    }
    else
    {
      common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_framing_sw;
      return __LL_ATON_LIB_DMA_Pad_Filling(&common_params,
                                           nn_instance); // first phase (in this case `filling`) is made in HW
    }
    break;

  case 2: // `edge` mode
    if ((consecutive_bytes < __LL_PAD_FILLING_DMA_MIN_BUFF_LEN) || (tensor_rank > __LL_DMA_PAD_MAX_DIMS))
    {                                                     // do it without HW support
      __ll_aton_lib_pad_edge_sw(0, &common_params, true); // all is done in SW
    }
    else
    {
      common_params.callback_function = (pad_callback_func_t)&__ll_aton_lib_pad_framing_sw;
      return __LL_ATON_LIB_DMA_Pad_Filling(&common_params,
                                           nn_instance); // first phase (in this case `filling`) is made in HW
    }
    break;

  default:
    __LL_LIB_ERROR(_ERR_MODE, LL_ATON_INVALID_PARAM)
  }

#if defined(DUMP_RESULTS_PAD_OP)
  /* debug output print */
  switch (common_params.nbytes)
  {
  case 1:
  {
    int8_t *ptr = (int8_t *)output;
    for (uint32_t i = 0; i < common_params.out_size; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 2:
  {
    int16_t *ptr = (int16_t *)(int8_t *)output;
    for (uint32_t i = 0; i < common_params.out_size / 2; i++)
    {
      LL_ATON_PRINTF("%d\n", ptr[i]);
    }
  }
  break;
  case 3: // NOTE: assuming no alignment
  {
    /* check endianness */
    const int32_t _const_val = 0x01020304;
    const int8_t *_const_val_ptr = (int8_t *)&_const_val;
    bool is_little_endian = (_const_val_ptr[0] == 0x04);

    int8_t *ptr = (int8_t *)output;
    if (is_little_endian)
    {
      for (uint32_t i = 0; i < common_params.out_size / 3; i += 3)
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
      for (uint32_t i = 0; i < common_params.out_size / 3; i += 3)
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
    int32_t *ptr = (int32_t *)(int8_t *)output;
    for (uint32_t i = 0; i < common_params.out_size / 4; i++)
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

  return LL_ATON_OK;
}
