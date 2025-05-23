/**
 ******************************************************************************
 * @file    ll_aton_reloc_network.c
 * @author  MCD/AIS Team
 * @brief   Implementation of the ATON LL module for relocatable model support
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024, 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(LL_ATON_RT_RELOC)

#include "ll_aton_lib.h"
#include "ll_aton_reloc_network.h"
#include "ll_aton_version.h"

/* -----------------------------------------------------------------------------
 * GlOBAL definitions
 * -----------------------------------------------------------------------------
 */

#if !defined(AI_RELOC_LOG_ENABLE)
#define AI_RELOC_LOG_ENABLE 1 /* 1: enable debug trace support (printf-based) */
#endif

#ifndef NDEBUG
#if defined(AI_RELOC_LOG_ENABLE) && AI_RELOC_LOG_ENABLE == 1
#define AI_RELOC_LOG(...) printf(__VA_ARGS__)
#else
#define AI_RELOC_LOG(...)                                                                                              \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#endif
#else
#define AI_RELOC_LOG(...)                                                                                              \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#endif

#if !defined(LL_ATON_PLATFORM) || (LL_ATON_PLATFORM != LL_ATON_PLAT_STM32N6)
#error "Model Relocatable mode is only supported for LL_ATON_PLAT_STM32N6 platform"
#if !defined(STM32N6)
#error "STM32N6 should be defined"
#endif
#endif

#define AI_RELOC_NPU_EXTERNAL_ADDR (0x60000000UL)

#if defined(STM32F7) || defined(STM32H7) || defined(STM32N6)
#define RELOC_MCU_CLEAN_INVALIDATE(_addr, _size)                                                                       \
  LL_ATON_Cache_MCU_Clean_Invalidate_Range((uintptr_t)(_addr), (uint32_t)(_size))
#else
#define RELOC_MCU_CLEAN_INVALIDATE(_addr, _size)                                                                       \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#endif

#if defined(STM32N6)
#define RELOC_NPU_INVALIDATE() LL_ATON_Cache_NPU_Invalidate() // npu_cache_invalidate()
#else
#define RELOC_NPU_INVALIDATE()                                                                                         \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#endif

/*
 *  Implementation of the call-backs fcts
 */
static void _assert_func(const char *filename, int line, const char *assert_func, const char *expr)
{
  AI_RELOC_LOG("-> assert_func called from reloc code : %d %s : %s %s\n", line, filename, assert_func, expr);
  assert(1 != 1);
}

static void _ll_lib_error(int err_code, int line, const char *func)
{
  AI_RELOC_LOG("-> ll_lib_error called from reloc code : %d %d : %s\n", line, err_code, func);
  assert(1 != 1);
}

static void _LL_ATON_Cache_MCU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
{
  LL_ATON_Cache_MCU_Clean_Range(virtual_addr, size);
}

static void _LL_ATON_Cache_MCU_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
  LL_ATON_Cache_MCU_Invalidate_Range(virtual_addr, size);
}

static void _LL_ATON_Cache_MCU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
  LL_ATON_Cache_MCU_Clean_Invalidate_Range(virtual_addr, size);
}

static void _LL_ATON_Cache_NPU_Clean_Range(uintptr_t virtual_addr, uint32_t size)
{
  LL_ATON_Cache_NPU_Clean_Range(virtual_addr, size);
}

static void _LL_ATON_Cache_NPU_Clean_Invalidate_Range(uintptr_t virtual_addr, uint32_t size)
{
  LL_ATON_Cache_NPU_Clean_Invalidate_Range(virtual_addr, size);
}

/* -----------------------------------------------------------------------------
 * Callbacks structure
 * -----------------------------------------------------------------------------
 */
static struct ll_aton_reloc_callback _network_reloc_callback = {
    .assert_func = &_assert_func,
    .ll_lib_error = &_ll_lib_error,

    .ll_aton_cache_mcu_clean_range = &_LL_ATON_Cache_MCU_Clean_Range,
    .ll_aton_cache_mcu_invalidate_range = &_LL_ATON_Cache_MCU_Invalidate_Range,
    .ll_aton_cache_mcu_clean_invalidate_range = &_LL_ATON_Cache_MCU_Clean_Invalidate_Range,
    .ll_aton_cache_npu_clean_range = &_LL_ATON_Cache_NPU_Clean_Range,
    .ll_aton_cache_npu_clean_invalidate_range = &_LL_ATON_Cache_NPU_Clean_Invalidate_Range,

    .ll_aton_lib_concat = &LL_ATON_LIB_Concat,
    .ll_aton_lib_cast = &LL_ATON_LIB_Cast,
    .ll_aton_lib_softmax = &LL_ATON_LIB_Softmax,
    .ll_aton_lib_dma_imagetorow = &LL_ATON_LIB_DMA_ImageToRow,
    .ll_aton_lib_dma_spacetodepth = &LL_ATON_LIB_DMA_SpaceToDepth,
    .ll_aton_lib_dma_rowtoimage = &LL_ATON_LIB_DMA_RowToImage,
    .ll_aton_lib_dma_depthtospace = &LL_ATON_LIB_DMA_DepthToSpace,
    .ll_aton_lib_dma_outputs_flat_copy = &LL_ATON_LIB_DMA_Outputs_Flat_Copy,
    .ll_aton_lib_dma_outputs_slice_splitlike = &LL_ATON_LIB_DMA_Outputs_Slice_SplitLike,
    .ll_aton_lib_dma_outputs_channel_split_aton = &LL_ATON_LIB_DMA_Outputs_Channel_Split_Aton,
    .ll_aton_lib_dma_outputs_channel_split_batched = &LL_ATON_LIB_DMA_Outputs_Channel_Split_Batched,
    .ll_aton_lib_dma_pad_memset = &LL_ATON_LIB_DMA_Pad_Memset,
    .ll_aton_lib_dma_pad_filling = &LL_ATON_LIB_DMA_Pad_Filling,
    .ll_aton_lib_dma_transpose = &LL_ATON_LIB_DMA_Transpose,
};

/* -----------------------------------------------------------------------------
 * AI RELOC definitions to manage the relocatable binary image
 * -----------------------------------------------------------------------------
 */

#define AI_RELOC_FLASH_BASE   (0x20000000UL)
#define AI_RELOC_RAM_BASE     (0x40000000UL)
#define AI_RELOC_PARAM_0_BASE (0x80000000UL)
#define AI_RELOC_PARAM_1_BASE (0x90000000UL)

#define AI_RELOC_MASK_ID     (0xF0000000UL)
#define AI_RELOC_MASK_OFFSET (0x0FFFFFFFUL)

static inline uint32_t _ai_reloc_get_offset(const uint32_t laddr)
{
  return (uint32_t)((laddr)&AI_RELOC_MASK_OFFSET);
}

static inline uint32_t _ai_reloc_get_addr(uint32_t base, uint32_t offset)
{
  return base + _ai_reloc_get_offset(offset);
}

static inline uint32_t _ai_reloc_get_val(uint32_t base, uint32_t offset)
{
  return base + _ai_reloc_get_offset(offset);
}

#define AI_RELOC_GET_OFFSET(_laddr)    (uintptr_t)(_ai_reloc_get_offset(_laddr))
#define AI_RELOC_GET_ADDR(_base, _off) (uintptr_t)(_ai_reloc_get_addr((uint32_t)_base, _off))
#define AI_RELOC_GET_VAL(_base, _off)  (uintptr_t)(_ai_reloc_get_val((uint32_t)_base, _off))

#define AI_RELOC_IN_RAM(_val) (((_val)&0xF0000000) == AI_RELOC_RAM_BASE)

#define AI_RELOC_IN_FLASH(_val) (((_val)&0xF0000000) == AI_RELOC_FLASH_BASE)

#define AI_RELOC_IN_PARAM_0(_val) (((_val)&0xF0000000) == AI_RELOC_PARAM_0_BASE)

#define AI_RELOC_IN_PARAM_1(_val) (((_val)&0xF0000000) == AI_RELOC_PARAM_1_BASE)

#define AI_RELOC_ROUND_UP(_v) (((_v) + 7) & ~7) /* 8-Bytes aligned */

#define AI_RELOC_IS_ALIGNED(_v) (((_v)&0x3) == 0) /* 8-Bytes aligned */

/* ! should be aligned with definition in linker script (see reloc_network.lkr) */
struct bin_hdr
{
  uint32_t magic; /* magic number of the RELOC binary object */
  uint32_t flags; /* configuration  (see (see ll_aton_reloc_network.h) */
};

/* ! should be aligned with definition in linker script (see reloc_network.lkr) */
struct sec_info
{
  uint32_t data_start; /* start of the data section */
  uint32_t data_end;   /* end of the data section */
  uint32_t data_data;  /* .. */
  uint32_t bss_start;
  uint32_t bss_end;
  uint32_t got_start;
  uint32_t got_end;
  uint32_t rel_start;
  uint32_t rel_end;
  uint32_t params_start;
  uint32_t params_offset;
};

/* ! should be aligned with struct ai_reloc_network_entries (see ll_aton_reloc_network.h) */
struct net_entries
{
  uint32_t ec_network_init;
  uint32_t ec_inference_init;
  uint32_t input_setter;
  uint32_t input_getter;
  uint32_t output_setter;
  uint32_t output_getter;
  uint32_t get_epoch_items;
  uint32_t get_output_buffers;
  uint32_t get_input_buffers;
  uint32_t get_internal_buffers;
  uint32_t ctx;
};

/* Header of the binary file */
struct ai_reloc_bin_hdr
{
  struct bin_hdr hdr;
  struct sec_info sect;
  struct net_entries vec;
};

/*
 * Naked function to set the R9 value and to call the entry point
 * without a "prolog" and "epilog".
 *
 * 	r0 = ROM base address
 * 	r1 = offset of the function
 * 	r2 = RAM base address (used for R9)
 * 	r3      = arg1		   -> r0
 * 	sp[0]   = arg2       -> r1
 * 	sp[0+4] = arg3       -> r2
 *
 */

#if defined(__GNUC__) && !defined(__ARMCC_VERSION) /* GNU compiler */

static uintptr_t __attribute__((naked))
call_with_r9(const void *base, uint32_t offset, void *data, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
  __asm__ volatile("add  r12, r0, r1      \n"
                   "mov  r0,  r3          \n"
                   "ldr  r1,  [sp]        \n"
                   "push {r9, lr}         \n"
                   "mov  r9, r2           \n"
                   "ldr  r2,  [sp, #12]   \n"
                   "blx  r12              \n"
                   "pop  {r9, pc}         \n");

  return 0; // dummy to fool gcc
}

#elif defined(__ICCARM__) /* IAR compiler */

__task __irq uintptr_t call_with_r9(const void *base, uint32_t offset, void *data, uintptr_t arg1, uintptr_t arg2,
                                    uintptr_t arg3);
__task __irq uintptr_t call_with_r9(const void *base, uint32_t offset, void *data, uintptr_t arg1, uintptr_t arg2,
                                    uintptr_t arg3)
{
  asm volatile("add  r12, r0, r1      \n"
               "mov  r0,  r3          \n"
               "ldr  r1,  [sp]        \n"
               "push {r9, lr}         \n"
               "mov  r9, r2           \n"
               "ldr  r2,  [sp, #12]   \n"
               "blx  r12              \n"
               "pop  {r9, pc}         \n");

  return 0; // dummy to fool gcc
}

#elif defined(__CC_ARM) /* Arm compiler 4/5 */

// clang-format off
__asm uintptr_t call_with_r9(const void *base,
		uint32_t offset, void *data,
		uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
	add  r12, r0, r1
	mov  r0,  r3
	ldr  r1,  [sp]
			   push {r9, lr}
	mov  r9, r2
	ldr  r2,  [sp, #12]
			   blx  r12
			   pop  {r9, pc}
}
// clang-format on

#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) /* Arm Compiler 6 */

static uintptr_t __attribute__((naked))
call_with_r9(const void *base, uint32_t offset, void *data, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
  __asm("add  r12, r0, r1      \n"
        "mov  r0,  r3          \n"
        "ldr  r1,  [sp]        \n"
        "push {r9, lr}         \n"
        "mov  r9, r2           \n"
        "ldr  r2,  [sp, #12]   \n"
        "blx  r12              \n"
        "pop  {r9, pc}         \n");
}

#else

#error Unknown compiler.

#endif

/**
 * Return the size of the ro region (hdr, text & rodata sections) - 8-Bytes aligned
 */
static uint32_t _npu_reloc_code_size(const uintptr_t file_ptr)
{
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)file_ptr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0))
    return 0;

  uint32_t ro_sz = AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(bin->sect.data_data));

  return ro_sz;
}

/**
 * Return the minimum requested size (in bytes) of the exec ram region - 8-Bytes aligned
 * according the expected mode - COPY or XIP
 */
static uint32_t _npu_reloc_requested_ram_size(const uintptr_t file_ptr, uint32_t mode)
{
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)file_ptr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0))
    return 0;

  const uint32_t rw_sz = AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(bin->sect.bss_end));
  const uint32_t ro_sz = AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(bin->sect.data_data));

  if ((mode & AI_RELOC_RT_LOAD_MODE_XIP) == AI_RELOC_RT_LOAD_MODE_XIP)
    return rw_sz;
  else
    return rw_sz + ro_sz;
}

#define _CPUID *(volatile uint32_t *)(0xE000ED00)
#define _CPACR *(volatile uint32_t *)(0xE000ED88)

#define _CPUID_PART_NUMBER (0xFFF << 4) /* Part Number */
#define _CPACR_CPx         (0xF << 20)  /* CP1 & CP0 bits */

#define _GET_PART_NUMBER() (int)((_CPUID & _CPUID_PART_NUMBER) >> 4)
#define _GET_FPU_CPX()     (int)((_CPACR & _CPACR_CPx) >> 20)

static int _ai_reloc_rt_checking(const struct ai_reloc_bin_hdr *bin)
{
  const uint32_t flags = bin->hdr.flags;
  const uint32_t cpuid = _GET_PART_NUMBER();

  /* Binary header/context */
  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (!AI_RELOC_IS_ALIGNED((uintptr_t)bin)))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Invalid binary header\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  if ((AI_RELOC_RT_GET_MAJOR(flags) != AI_RELOC_RT_VERSION_MAJOR) &&
      (AI_RELOC_RT_GET_MINOR(flags) != AI_RELOC_RT_VERSION_MINOR))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Binary header - invalid version\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  if (cpuid != AI_RELOC_RT_GET_CPUID(flags))
  {
    AI_RELOC_LOG("AI RELOC ERROR: CPUID is invalid 0x%03X (expected 0x%03X)\r\n", (int)cpuid,
                 (int)AI_RELOC_RT_GET_CPUID(flags));
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  if (AI_RELOC_RT_FPU_USED(flags))
  {
    if (!_GET_FPU_CPX())
    {
      AI_RELOC_LOG("AI RELOC ERROR: FPU is not enabled\r\n");
      return AI_RELOC_RT_ERR_INVALID_BIN;
    }
  }

  /* Extra flags */
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
  if (!AI_RELOC_RT_SECURE(flags))
  {
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
#else
  if (AI_RELOC_RT_SECURE(flags))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Not compiled with secure options\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
#endif

#if defined(LL_ATON_EB_DBG_INFO)
  if (!AI_RELOC_RT_DBG_INFO(flags))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Not compiled with LL_ATON_EB_DBG_INFO\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
#else
  if (AI_RELOC_RT_DBG_INFO(flags))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Compiled with LL_ATON_EB_DBG_INFO\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
#endif

#if defined(LL_ATON_RT_MODE) && (LL_ATON_RT_MODE == LL_ATON_RT_ASYNC)
  if (!AI_RELOC_RT_ASYNC_MODE(flags))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Not compiled with LL_ATON_RT_ASYNC\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
#else
  if (AI_RELOC_RT_ASYNC_MODE(flags))
  {
    AI_RELOC_LOG("AI RELOC ERROR: Compiled with LL_ATON_RT_ASYNC\r\n");
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
#endif

  /* Runtime version */
  struct ai_reloc_rt_ctx *rt_ctx =
      (struct ai_reloc_rt_ctx *)AI_RELOC_GET_ADDR(bin + AI_RELOC_GET_OFFSET(bin->sect.data_data), bin->vec.ctx);

  uint32_t rt_vers_ = LL_ATON_VERSION_MAJOR << 24 | LL_ATON_VERSION_MINOR << 16 | LL_ATON_VERSION_MICRO << 8;

  if (rt_vers_ != (rt_ctx->rt_version & 0xFFFFFF00UL))
  {
    AI_RELOC_LOG("AI RELOC ERROR: RT version is invalid, firmware version = 0x%03X (expected 0x%03X)\r\n",
                 (int)rt_vers_, (int)rt_ctx->rt_version);
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  return AI_RELOC_RT_ERR_NONE;
}

/*
 * Low level functions to install/create a relocatable binary model
 */

struct id_mpool_mapping
{
  uintptr_t addr_0;
  uint32_t sz_0;
  uintptr_t addr_1;
  uint32_t sz_1;
};

static int _ai_reloc_prepare_mpools(const uintptr_t file_ptr, struct id_mpool_mapping *id_map, uint32_t mode)
{
  const struct ai_reloc_bin_hdr *header = (struct ai_reloc_bin_hdr *)file_ptr;
  uint32_t params_start = AI_RELOC_GET_OFFSET(header->sect.params_start);
  params_start += AI_RELOC_GET_OFFSET(header->sect.data_data);
  uintptr_t addr_0 = 0;
  bool invalidate_npu_cache = false;

  ll_aton_reloc_mem_pool_desc *cur_mem_c_desc;

  /* Set/check base param addr - user addr is used in priority */
  if ((id_map->addr_0 == 0) && (header->sect.params_offset == 0))
    return AI_RELOC_RT_ERR_PARAM_ADDR;

  if (id_map->addr_0 == 0)
    id_map->addr_0 = file_ptr + AI_RELOC_GET_OFFSET(header->sect.params_offset);

  if (!AI_RELOC_IS_ALIGNED(id_map->addr_0))
    return AI_RELOC_RT_ERR_PARAM_ADDR;

  int cur_index = 0;
  cur_mem_c_desc = (ll_aton_reloc_mem_pool_desc *)AI_RELOC_GET_ADDR(header, params_start);

  while ((cur_index < 10) && (cur_mem_c_desc->flags) && (cur_mem_c_desc->name))
  {
    const uint32_t flags = cur_mem_c_desc->flags;
    const uintptr_t dst = cur_mem_c_desc->dst;
    const uint32_t sz = AI_RELOC_ROUND_UP(cur_mem_c_desc->size);
    const uint32_t foff = cur_mem_c_desc->foff;
    const uintptr_t src = id_map->addr_0 + foff;

    if (AI_RELOC_MPOOL_IS_CACHEABLE(flags))
    {
      invalidate_npu_cache = true;
    }

    if (AI_RELOC_MPOOL_IS_RELOC(flags)) /* Relocated mempool */
    {
      const uint32_t id = AI_RELOC_MPOOL_GET_ID(flags);
      if (id == 0) /* Parameters/weights section */
      {
        if ((AI_RELOC_MPOOL_IS_CACHEABLE(flags) && id_map->addr_0 <= AI_RELOC_NPU_EXTERNAL_ADDR))
        {
          return AI_RELOC_RT_ERR_PARAM_ADDR;
        }
        if (AI_RELOC_MPOOL_IS_MIXED(flags) || AI_RELOC_MPOOL_IS_WRITE(flags))
        {
          return AI_RELOC_RT_ERR_PARAM_DESC;
        }
        if ((AI_RELOC_MPOOL_IS_PARAM(flags)) && (!AI_RELOC_MPOOL_IS_ACTIV(flags)))
        {
          addr_0 = src;
        }
      }
      else if (id == 1) /* external RAM section */
      {
        if ((AI_RELOC_MPOOL_IS_CACHEABLE(flags) && id_map->addr_1 <= AI_RELOC_NPU_EXTERNAL_ADDR))
        {
          return AI_RELOC_RT_ERR_PARAM_ADDR;
        }
        if (((id_map->addr_1 == 0) || (id_map->sz_1 < sz)))
        {
          return AI_RELOC_RT_ERR_PARAM_ADDR;
        }
        if (AI_RELOC_MPOOL_IS_MIXED(flags))
        {
          memcpy((void *)id_map->addr_1, (void const *)(src), sz);
          RELOC_MCU_CLEAN_INVALIDATE(id_map->addr_1, sz);
        }
        else if ((AI_RELOC_MPOOL_IS_ACTIV(flags)) && (mode & AI_RELOC_RT_LOAD_MODE_CLEAR))
        {
          memset((void *)id_map->addr_1, 0, sz);
          RELOC_MCU_CLEAN_INVALIDATE(id_map->addr_1, sz);
        }
      }
      else
      {
        return AI_RELOC_RT_ERR_PARAM_DESC;
      }
    }
    else /* !AI_RELOC_MPOOL_IS_RELOC */
    {
      if (AI_RELOC_MPOOL_IS_COPY(flags))
      {
        memcpy((void *)dst, (void const *)(src), sz);
        RELOC_MCU_CLEAN_INVALIDATE(dst, sz);
      }
      if (AI_RELOC_MPOOL_IS_RESET(flags) && (mode & AI_RELOC_RT_LOAD_MODE_CLEAR))
      {
        memset((void *)dst, 0, sz);
        RELOC_MCU_CLEAN_INVALIDATE(dst, sz);
      }
    }

    cur_mem_c_desc++;
    cur_index++;
  }

  if (invalidate_npu_cache)
    RELOC_NPU_INVALIDATE();

  if (addr_0) /* Update the param_0 base address */
    id_map->addr_0 = addr_0;

  if (cur_index == 10)
    return AI_RELOC_RT_ERR_INVALID_BIN;

  return AI_RELOC_RT_ERR_NONE;
}

static int _ai_reloc_got_update(const struct ai_reloc_bin_hdr *bin, uintptr_t ram_addr, uintptr_t param_0_addr,
                                uintptr_t param_1_addr)
{
  uint32_t *got_start = (uint32_t *)AI_RELOC_GET_ADDR(ram_addr, bin->sect.got_start);
  uint32_t *got_end = (uint32_t *)AI_RELOC_GET_ADDR(ram_addr, bin->sect.got_end);

  for (uint32_t *p = got_start; p < got_end; p++)
  {
    uint32_t val = *p;
    if AI_RELOC_IN_RAM (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(ram_addr, val);
    }
    else if AI_RELOC_IN_FLASH (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(bin, val);
    }
    else if AI_RELOC_IN_PARAM_0 (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(param_0_addr, val);
    }
    else if AI_RELOC_IN_PARAM_1 (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(param_1_addr, val);
    }
    else if (val != 0)
    {
      AI_RELOC_LOG("AI RELOC ERROR: GOT update - unsupported value: %08x\r\n", (int)val);
      return AI_RELOC_RT_ERR_INVALID_BIN;
    }
    *p = val;
  }
  return AI_RELOC_RT_ERR_NONE;
}

/*
 * Low level function to update the DATA section in RAM.
 */
int _ai_reloc_ram_update(const struct ai_reloc_bin_hdr *bin, uintptr_t ram_addr, uintptr_t param_0_addr,
                         uintptr_t param_1_addr, const uintptr_t obj)
{
  uint32_t *rel_start = (uint32_t *)AI_RELOC_GET_ADDR(obj, bin->sect.rel_start);
  uint32_t *rel_end = (uint32_t *)AI_RELOC_GET_ADDR(obj, bin->sect.rel_end);

  for (uint32_t *p = rel_start; p < rel_end; p++)
  {
    uint32_t add = *p;
    uint32_t val = *(uint32_t *)AI_RELOC_GET_VAL(ram_addr, add);
    if AI_RELOC_IN_RAM (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(ram_addr, val);
    }
    else if AI_RELOC_IN_FLASH (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(bin, val);
    }
    else if AI_RELOC_IN_PARAM_0 (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(param_0_addr, val);
    }
    else if AI_RELOC_IN_PARAM_1 (val)
    {
      val = (uint32_t)AI_RELOC_GET_VAL(param_1_addr, val);
    }
    else if (val != 0)
    {
      AI_RELOC_LOG("AI RELOC ERROR: REL update - unsupported value: %08x\r\n", (int)val);
      return AI_RELOC_RT_ERR_INVALID_BIN;
    }
    uint32_t *dest = (uint32_t *)AI_RELOC_GET_ADDR(ram_addr, add);
    *dest = val;
  }
  return AI_RELOC_RT_ERR_NONE;
}

/*
 * Low level function to install the relocatable code.
 *
 * - 'obj' address of the memory-mapped binary object.
 * - ram_addr/ram_size indicates the location (and the size)
 *   of the buffer destination to install and to update the data/got
 *   and bss sections for XIP mode or including the hdr/text and
 *   rodata sections for COPY mode. rel section is only used
 *   at init time.
 * - 'mode' indicates the load mode: XIP or COPY.
 * - if successful, 0 is returned and the 'hdl' parameter is
 *   updated with the address of an internal opaque structure. This
 *   handle should be used for the other function ai_reloc_XX.
 *
 * Loading mode:
 *
 * 	XIP mode -  only the RW and got sections are copied in RAM
 * 	            data/got section is updated according the ram/rom@
 * 	             and the info from the rel section.
 * 	            code (text/rodata section) is executed-in-place
 * 	COPY mode - code is also copied in RAM
 *
 *
 * obj@                               ram_addr@       -> should be aligned 8-bytes
 *   ----------- rom_addr@              -----------
 * 	 [  hdr    ]                        [ data    ]
 *   [  text   ]                        [ got     ]
 *   [  rodata ]   -- XIP  mode  ->     [ bss     ]
 *   -----------                        -----------
 *   [  data   ]
 *   [  got    ]
 *   [  rel    ]
 *   -----------
 *  					              ram_addr@ -> rom_addr@
 *  					                -----------
 *                                      [ hdr     ]
 *                 -- COPY mode -->     [ text    ]
 *                                      [ rodata  ]
 *                                       pad..       if requested to align (8-bytes) the new ram addr
 *                                      ----------- new ram_addr@
 *                                      [ data    ]
 *                                      [ got     ]
 *                                      [ bss     ]
 *                                      -----------
 *
 */
static int _ai_reloc_install(const uintptr_t file_ptr, uintptr_t ram_addr, size_t ram_size, uint32_t ext_ram_addr,
                             size_t ext_ram_size, uintptr_t param_addr, uint32_t mode, NN_Instance_TypeDef *nn_instance)
{
  int res;
  uint32_t state = AI_RELOC_RT_STATE_NOT_INITIALIZED;
  struct ai_reloc_bin_hdr *rom_addr = (struct ai_reloc_bin_hdr *)file_ptr;
  const uint32_t req_ram_size = _npu_reloc_requested_ram_size(file_ptr, mode);
  struct id_mpool_mapping id_map = {param_addr, 0, ext_ram_addr, ext_ram_size};

  /* Parameter check */
  if (!req_ram_size)
    return AI_RELOC_RT_ERR_INVALID_BIN;

  if (((!(mode & AI_RELOC_RT_LOAD_MODE_XIP)) && (!(mode & AI_RELOC_RT_LOAD_MODE_COPY))) || (!nn_instance))
    return AI_RELOC_RT_ERR_ARG;

  if (!ram_addr || !ram_size || (req_ram_size > ram_size))
    return AI_RELOC_RT_ERR_MEMORY;

  if (ext_ram_addr && !AI_RELOC_IS_ALIGNED(ext_ram_addr))
    return AI_RELOC_RT_ERR_PARAM_ADDR;

  if (ram_addr && ram_size && !AI_RELOC_IS_ALIGNED(ram_addr))
    return AI_RELOC_RT_ERR_MEMORY;

  res = _ai_reloc_prepare_mpools(file_ptr, &id_map, mode);
  if (res)
    return res;

  /* Copy hrd, txt and rodata sections in RAM */
  if (mode & AI_RELOC_RT_LOAD_MODE_COPY)
  {
    const uint32_t ro_sz = AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(rom_addr->sect.data_data));
    memcpy((void *)ram_addr, (void const *)file_ptr, ro_sz);

    RELOC_MCU_CLEAN_INVALIDATE(ram_addr, ro_sz);

    /* Update the rom_addr/ram_addr pointers */
    rom_addr = (struct ai_reloc_bin_hdr *)(ram_addr);
    ram_addr = (uintptr_t)(ram_addr + ro_sz);
  }
  else
  {
    state |= AI_RELOC_RT_STATE_XIP_MODE;
  }

  const uintptr_t bss_start = AI_RELOC_GET_ADDR(ram_addr, rom_addr->sect.bss_start);
  const uint32_t bss_size = rom_addr->sect.bss_end - rom_addr->sect.bss_start;
  const uintptr_t src_data = AI_RELOC_GET_ADDR(file_ptr, rom_addr->sect.data_data);
  const uint32_t rw_sz = AI_RELOC_GET_OFFSET(rom_addr->sect.bss_end);

  /* Copy the data section, including the got section */
  memcpy((void *)ram_addr, (const void *)src_data, rw_sz - bss_size);

  /* Clear the bss section */
  memset((void *)bss_start, 0, bss_size);

  /* Update the relocation table and data */
  /* R_ARM_GOT_BREL type */
  if (_ai_reloc_got_update(rom_addr, ram_addr, id_map.addr_0, id_map.addr_1))
    return AI_RELOC_RT_ERR_INVALID_BIN;

  /* R_ARM_ABS32 type */
  if (_ai_reloc_ram_update(rom_addr, ram_addr, id_map.addr_0, id_map.addr_1, file_ptr))
    return AI_RELOC_RT_ERR_INVALID_BIN;

  RELOC_MCU_CLEAN_INVALIDATE(ram_addr, rw_sz);

  /* Update the RT context */
  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)AI_RELOC_GET_ADDR(ram_addr, rom_addr->vec.ctx);

  rt_ctx->rom_addr = (uint32_t)rom_addr;
  rt_ctx->ram_addr = (uint32_t)ram_addr;
  rt_ctx->file_addr = (uint32_t)file_ptr;
  rt_ctx->state = (state | AI_RELOC_RT_STATE_INITIALIZED);
  rt_ctx->ll_instance = nn_instance;

  /* fill the handler */
  nn_instance->network = rt_ctx->itf_network;
  memset(&nn_instance->exec_state, 0, sizeof(NN_Execution_State_TypeDef));
  nn_instance->exec_state.inst_reloc = (uint32_t)rt_ctx;

  return AI_RELOC_RT_ERR_NONE;
}

static int _ai_rel_check_handler(uintptr_t hdl)
{
  if (!hdl)
    return AI_RELOC_RT_ERR_INVALID_HDL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || !(rt_ctx->state & AI_RELOC_RT_STATE_INITIALIZED))
    return AI_RELOC_RT_ERR_INVALID_BIN;

  return AI_RELOC_RT_ERR_NONE;
}

/* -----------------------------------------------------------------------------
 * Public API implementation
 * -----------------------------------------------------------------------------
 */

ll_aton_reloc_mem_pool_desc *ll_aton_reloc_get_mem_pool_desc(const uintptr_t file_ptr, int index)
{
  const struct ai_reloc_bin_hdr *bin = (struct ai_reloc_bin_hdr *)file_ptr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (!AI_RELOC_IN_RAM(bin->sect.params_start)))
  {
    return NULL;
  }

  ll_aton_reloc_mem_pool_desc *cur_mem_c_desc;
  int cur_index = 0;

  uint32_t off_params = AI_RELOC_GET_OFFSET(bin->sect.params_start);
  off_params += AI_RELOC_GET_OFFSET(bin->sect.data_data);
  cur_mem_c_desc = (ll_aton_reloc_mem_pool_desc *)AI_RELOC_GET_ADDR(bin, off_params);

  while ((cur_index < 10) && (cur_mem_c_desc->flags) && (cur_mem_c_desc->name))
  {
    if (cur_index == index)
      return cur_mem_c_desc;
    cur_mem_c_desc++;
    cur_index++;
  }

  return NULL;
}

int ll_aton_reloc_get_info(const uintptr_t file_ptr, ll_aton_reloc_info *rt)
{
  const struct ai_reloc_bin_hdr *bin = (struct ai_reloc_bin_hdr *)file_ptr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (!AI_RELOC_IS_ALIGNED((uintptr_t)bin)) || (!rt))
  {
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  struct ai_reloc_rt_ctx *rt_ctx =
      (struct ai_reloc_rt_ctx *)AI_RELOC_GET_ADDR(bin + AI_RELOC_GET_OFFSET(bin->sect.data_data), bin->vec.ctx);

  rt->c_name = (const char *)AI_RELOC_GET_ADDR(bin, AI_RELOC_GET_OFFSET((int)rt_ctx->c_name));
  rt->variant = (uint32_t)bin->hdr.flags;
  rt->params_off = (uint32_t)AI_RELOC_GET_OFFSET(bin->sect.params_offset);
  rt->params_sz = (uint32_t)rt_ctx->params_sz;
  rt->acts_sz = (uint32_t)rt_ctx->acts_sz;
  rt->ext_ram_sz = (uint32_t)AI_RELOC_ROUND_UP(rt_ctx->ext_ram_sz);
  rt->rt_ram_xip = _npu_reloc_requested_ram_size(file_ptr, AI_RELOC_RT_LOAD_MODE_XIP);
  rt->rt_ram_copy = _npu_reloc_requested_ram_size(file_ptr, AI_RELOC_RT_LOAD_MODE_COPY);
  rt->code_sz = _npu_reloc_code_size(file_ptr);
  rt->rt_version = rt_ctx->rt_version;
  rt->rt_version_extra = rt_ctx->rt_version_extra;
  rt->rt_version_desc = (const char *)AI_RELOC_GET_ADDR(bin, AI_RELOC_GET_OFFSET((int)rt_ctx->rt_version_desc));

  return AI_RELOC_RT_ERR_NONE;
}

#if defined(AI_RELOC_LOG_ENABLE) && (AI_RELOC_LOG_ENABLE == 1) && !defined(NDEBUG)
static char *_magic_to_str(uint32_t val)
{
  static char res[5];
  res[3] = val >> 24;
  res[2] = val >> 16;
  res[1] = val >> 8;
  res[0] = val >> 0;
  return res;
}
#endif

void ll_aton_reloc_log_info(const uintptr_t file_ptr)
{
#if defined(AI_RELOC_LOG_ENABLE) && AI_RELOC_LOG_ENABLE == 1
  ll_aton_reloc_info rt_info;

  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)file_ptr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (!AI_RELOC_IS_ALIGNED((uintptr_t)bin)))
  {
    AI_RELOC_LOG("\r\nai_rel_log_header: ERR invalid file_ptr: %x\r\n", (unsigned int)file_ptr);
    return;
  }

  ll_aton_reloc_get_info(file_ptr, &rt_info);

  AI_RELOC_LOG("\r\nBinary model image (@0x%08x)\r\n", (int)bin);
  AI_RELOC_LOG("----------------------------------------------------------------\n");
  AI_RELOC_LOG("  c-name        : \"%s\"\r\n", rt_info.c_name);
  AI_RELOC_LOG("  act sz        : %d\r\n", (int)rt_info.acts_sz);
  AI_RELOC_LOG("  params sz     : %d\r\n", (int)rt_info.params_sz);
  AI_RELOC_LOG("  params off    : %d\r\n", (int)rt_info.params_off);
  AI_RELOC_LOG("  ext ram sz    : %d\r\n", (int)rt_info.ext_ram_sz);
  AI_RELOC_LOG("  exec ram xip  : %d for XIP mode\r\n", (int)rt_info.rt_ram_xip);
  AI_RELOC_LOG("  exec ram copy : %d for COPY mode\r\n", (int)rt_info.rt_ram_copy);
  AI_RELOC_LOG("  rt_desc       : \"%s\"\r\n", rt_info.rt_version_desc);
  AI_RELOC_LOG("  rt_version    : %d.%d.%d-%d\r\n", (int)(rt_info.rt_version >> 24 & 0xFF),
               (int)(rt_info.rt_version >> 16 & 0xFF), (int)(rt_info.rt_version >> 8 & 0xFF),
               (int)(rt_info.rt_version_extra));

  AI_RELOC_LOG("\n C-mempool descriptors\n");
  AI_RELOC_LOG(" --------------------------------------------------------------\n");

  ll_aton_reloc_mem_pool_desc *mem_c_desc;
  int index = 0;

  while ((mem_c_desc = ll_aton_reloc_get_mem_pool_desc((uintptr_t)bin, index)))
  {
    AI_RELOC_LOG(" %d: flags=%x foff=%d dst=%x s=%d %s\n", index, mem_c_desc->flags, mem_c_desc->foff, mem_c_desc->dst,
                 mem_c_desc->size, (char *)AI_RELOC_GET_ADDR(bin, (uint32_t)mem_c_desc->name));
    index++;
  }

#ifndef NDEBUG
  int fpu_is_enabled = _GET_FPU_CPX();
#endif

  AI_RELOC_LOG("\r\n Binary header\r\n");
  AI_RELOC_LOG("  magic         : 0x%08X (%s)\r\n", (int)bin->hdr.magic, _magic_to_str(bin->hdr.magic));
  AI_RELOC_LOG("  flags         : v%d.%d compil=%d feabi=%d (0x%08X)\r\n", (int)AI_RELOC_RT_GET_MAJOR(rt_info.variant),
               (int)AI_RELOC_RT_GET_MINOR(rt_info.variant), (int)AI_RELOC_RT_GET_COMPILER(rt_info.variant),
               (int)AI_RELOC_RT_GET_FPEABI(rt_info.variant), (int)rt_info.variant);
  AI_RELOC_LOG("                  fpu=%d (is_enabled:%d)\r\n", (int)AI_RELOC_RT_FPU_USED(rt_info.variant),
               fpu_is_enabled);
  AI_RELOC_LOG("                  cpuid=0x%03x (0x%03x)\r\n", AI_RELOC_RT_GET_CPUID(rt_info.variant),
               _GET_PART_NUMBER());
  AI_RELOC_LOG("  extra         : dbg=%d async=%d secure=%d\r\n", (int)AI_RELOC_RT_DBG_INFO(rt_info.variant),
               (int)AI_RELOC_RT_ASYNC_MODE(rt_info.variant), (int)AI_RELOC_RT_SECURE(rt_info.variant));
  AI_RELOC_LOG("  size          : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.rel_end));
  AI_RELOC_LOG("  .txt/.rodata  : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.data_data));
  AI_RELOC_LOG("  .data         : %d\r\n",
               (int)AI_RELOC_GET_OFFSET(bin->sect.data_end) - (int)AI_RELOC_GET_OFFSET(bin->sect.data_start));
  AI_RELOC_LOG("  .got          : %d\r\n",
               (int)AI_RELOC_GET_OFFSET(bin->sect.got_end) - (int)AI_RELOC_GET_OFFSET(bin->sect.got_start));
  AI_RELOC_LOG("  .rel          : %d\r\n",
               (int)AI_RELOC_GET_OFFSET(bin->sect.rel_end) - (int)AI_RELOC_GET_OFFSET(bin->sect.rel_start));
  AI_RELOC_LOG("  .bss          : %d\r\n",
               (int)AI_RELOC_GET_OFFSET(bin->sect.bss_end) - (int)AI_RELOC_GET_OFFSET(bin->sect.bss_start));

  AI_RELOC_LOG("\r\n");
#endif /* AI_RELOC_LOG_ENABLE == 1 */
}

int ll_aton_reloc_install(const uintptr_t file_ptr, const ll_aton_reloc_config *config,
                          NN_Instance_TypeDef *nn_instance)
{
  if (!nn_instance || !file_ptr)
  {
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  const struct ai_reloc_bin_hdr *rom_addr = (struct ai_reloc_bin_hdr *)file_ptr;

  /* Binary/header & RT environment checking */
  if (_ai_reloc_rt_checking(rom_addr))
    return AI_RELOC_RT_ERR_INVALID_BIN;

  if (!config)
    return AI_RELOC_RT_ERR_ARG;

  int res;
  res = _ai_reloc_install(file_ptr, config->exec_ram_addr, config->exec_ram_size, config->ext_ram_addr,
                          config->ext_ram_size, config->ext_param_addr, config->mode, nn_instance);

  if (!res)
    res = ll_aton_reloc_set_callbacks(nn_instance, &_network_reloc_callback);

  return res;
}

int ll_aton_reloc_set_callbacks(const NN_Instance_TypeDef *nn_instance, const struct ll_aton_reloc_callback *cbs)
{
  if (!nn_instance || !cbs || !nn_instance->exec_state.inst_reloc)
  {
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }
  if (_ai_rel_check_handler(nn_instance->exec_state.inst_reloc))
    return 0;

  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)nn_instance->exec_state.inst_reloc;
  rt_ctx->cbs = (void *)cbs;

  return AI_RELOC_RT_ERR_NONE;
}

/*
 * Indicate if the associated model is a relocatable installed model
 */
int ll_aton_reloc_is_valid(const NN_Instance_TypeDef *nn_inst)
{
  if (!nn_inst)
  {
    return AI_RELOC_RT_ERR_INVALID_HDL;
  }

  int res = _ai_rel_check_handler(nn_inst->exec_state.inst_reloc);

  if (res == AI_RELOC_RT_ERR_NONE)
    return 1;

  return res;
}

/* -----------------------------------------------------------------------------
 * Entry points of the relocatable model
 * ----------------------------------------------------------------------------- */

bool ai_rel_network_ec_network_init(uintptr_t hdl)
{
  if (_ai_rel_check_handler(hdl))
    return 0;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.ec_network_init),
                               (void *)rt_ctx->ram_addr, 0, 0, 0);

  return (bool)res;
}

bool ai_rel_network_ec_inference_init(uintptr_t hdl)
{
  if (_ai_rel_check_handler(hdl))
    return 0;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.ec_inference_init),
                               (void *)rt_ctx->ram_addr, 0, 0, 0);

  return (bool)res;
}

LL_ATON_User_IO_Result_t ai_rel_network_set_input(uintptr_t inst, uint32_t num, void *buffer, uint32_t size)
{
  if (_ai_rel_check_handler(inst))
    return LL_ATON_User_IO_WRONG_INDEX;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.input_setter),
                               (void *)rt_ctx->ram_addr, num, (uintptr_t)buffer, size);

  return (LL_ATON_User_IO_Result_t)res;
}

void *ai_rel_network_get_input(uintptr_t inst, uint32_t num)
{
  if (_ai_rel_check_handler(inst))
    return NULL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.input_getter),
                               (void *)rt_ctx->ram_addr, num, 0, 0);

  return (void *)res;
}

LL_ATON_User_IO_Result_t ai_rel_network_set_output(uintptr_t inst, uint32_t num, void *buffer, uint32_t size)
{
  if (_ai_rel_check_handler(inst))
    return LL_ATON_User_IO_WRONG_INDEX;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.output_setter),
                               (void *)rt_ctx->ram_addr, num, (uintptr_t)buffer, size);

  return (LL_ATON_User_IO_Result_t)res;
}

void *ai_rel_network_get_output(uintptr_t inst, uint32_t num)
{
  if (_ai_rel_check_handler(inst))
    return NULL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.output_getter),
                               (void *)rt_ctx->ram_addr, num, 0, 0);

  return (void *)res;
}

const EpochBlock_ItemTypeDef *ai_rel_network_get_epoch_items(uintptr_t inst)
{
  if (_ai_rel_check_handler(inst))
    return NULL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;
  const EpochBlock_ItemTypeDef *blocks;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.get_epoch_items),
                               (void *)rt_ctx->ram_addr, 0, 0, 0);

  blocks = (const EpochBlock_ItemTypeDef *)res;

  return blocks;
}

const LL_Buffer_InfoTypeDef *ai_rel_network_get_output_buffers_info(uintptr_t inst)
{
  if (_ai_rel_check_handler(inst))
    return NULL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;
  const LL_Buffer_InfoTypeDef *buff_infos;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.get_output_buffers),
                               (void *)rt_ctx->ram_addr, 0, 0, 0);

  buff_infos = (const LL_Buffer_InfoTypeDef *)res;

  return buff_infos;
}

const LL_Buffer_InfoTypeDef *ai_rel_network_get_input_buffers_info(uintptr_t inst)
{
  if (_ai_rel_check_handler(inst))
    return NULL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;
  const LL_Buffer_InfoTypeDef *buff_infos;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.get_input_buffers),
                               (void *)rt_ctx->ram_addr, 0, 0, 0);

  buff_infos = (const LL_Buffer_InfoTypeDef *)res;

  return buff_infos;
}

const LL_Buffer_InfoTypeDef *ai_rel_network_get_internal_buffers_info(uintptr_t inst)
{
  if (_ai_rel_check_handler(inst))
    return NULL;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)inst;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;
  const LL_Buffer_InfoTypeDef *buff_infos;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.get_internal_buffers),
                               (void *)rt_ctx->ram_addr, 0, 0, 0);

  buff_infos = (const LL_Buffer_InfoTypeDef *)res;

  return buff_infos;
}

void ai_rel_call_start_end_function(uintptr_t inst, start_end_func_ptr fct, const void *epoch_block)
{
  register uint32_t _saved_r9;
  register uint32_t _r9 = ((struct ai_reloc_rt_ctx *)inst)->ram_addr;
  __asm volatile("mov %0, r9\n\t" : "=r"(_saved_r9));
  __asm volatile("mov r9, %0\n\t" ::"r"(_r9));
  fct(epoch_block);
  __asm volatile("mov r9, %0\n\t" ::"r"(_saved_r9));
}

int ll_aton_reloc_get_file_ptr(const NN_Instance_TypeDef *nn_inst, uintptr_t *file_ptr)
{
  if (!nn_inst || !file_ptr || !nn_inst->exec_state.inst_reloc)
  {
    return AI_RELOC_RT_ERR_INVALID_HDL;
  }

  int res = _ai_rel_check_handler(nn_inst->exec_state.inst_reloc);
  if (res)
    return res;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)nn_inst->exec_state.inst_reloc;

  *file_ptr = (uintptr_t)rt_ctx->file_addr;

  return AI_RELOC_RT_ERR_NONE;
}

const EpochBlock_ItemTypeDef *ll_aton_reloc_get_epoch_items(const NN_Instance_TypeDef *nn_inst)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return NULL;
  }

  const EpochBlock_ItemTypeDef *epochs = ai_rel_network_get_epoch_items(nn_inst->exec_state.inst_reloc);
  return epochs;
}

const LL_Buffer_InfoTypeDef *ll_aton_reloc_get_input_buffers_info(const NN_Instance_TypeDef *nn_inst, int32_t num)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return NULL;
  }

  const LL_Buffer_InfoTypeDef *buffs = ai_rel_network_get_input_buffers_info(nn_inst->exec_state.inst_reloc);
  if ((num < 0) || (!buffs))
    return buffs;

  int32_t idx = 0;
  while (buffs)
  {
    if (!buffs->is_param)
    {
      if (idx == num)
        return buffs;
    }
    else
    {
      return NULL;
    }
    buffs++;
    idx++;
  }

  return NULL;
}

const LL_Buffer_InfoTypeDef *ll_aton_reloc_get_output_buffers_info(const NN_Instance_TypeDef *nn_inst, int32_t num)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return NULL;
  }

  const LL_Buffer_InfoTypeDef *buffs = ai_rel_network_get_output_buffers_info(nn_inst->exec_state.inst_reloc);
  if ((num < 0) || (!buffs))
    return buffs;

  int32_t idx = 0;
  while (buffs)
  {
    if (!buffs->is_param)
    {
      if (idx == num)
        return buffs;
    }
    else
    {
      return NULL;
    }
    buffs++;
    idx++;
  }

  return NULL;
}

const LL_Buffer_InfoTypeDef *ll_aton_reloc_get_internal_buffers_info(const NN_Instance_TypeDef *nn_inst)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return NULL;
  }

  const LL_Buffer_InfoTypeDef *buffs = ai_rel_network_get_internal_buffers_info(nn_inst->exec_state.inst_reloc);
  return buffs;
}

LL_ATON_User_IO_Result_t ll_aton_reloc_set_input(const NN_Instance_TypeDef *nn_inst, uint32_t num, void *buffer,
                                                 uint32_t size)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return LL_ATON_User_IO_WRONG_INDEX;
  }

  return ai_rel_network_set_input(nn_inst->exec_state.inst_reloc, num, buffer, size);
}

void *ll_aton_reloc_get_input(const NN_Instance_TypeDef *nn_inst, uint32_t num)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return NULL;
  }

  return ai_rel_network_get_input(nn_inst->exec_state.inst_reloc, num);
}

LL_ATON_User_IO_Result_t ll_aton_reloc_set_output(const NN_Instance_TypeDef *nn_inst, uint32_t num, void *buffer,
                                                  uint32_t size)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return LL_ATON_User_IO_WRONG_INDEX;
  }

  return ai_rel_network_set_output(nn_inst->exec_state.inst_reloc, num, buffer, size);
}

void *ll_aton_reloc_get_output(const NN_Instance_TypeDef *nn_inst, uint32_t num)
{
  if (!nn_inst || !nn_inst->exec_state.inst_reloc)
  {
    return NULL;
  }

  return ai_rel_network_get_output(nn_inst->exec_state.inst_reloc, num);
}

#endif /* defined(LL_ATON_RT_RELOC) */
