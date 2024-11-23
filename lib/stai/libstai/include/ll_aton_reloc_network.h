/**
 ******************************************************************************
 * @file    ll_aton_reloc_network.h
 * @author  MCD/AIS Team
 * @brief   Header file of ATON LL module for relocatable model support
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

#ifndef __LL_ATON_RELOC_NETWORK_H__
#define __LL_ATON_RELOC_NETWORK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "ll_aton_lib.h"

#if !defined(LL_ATON_RT_RELOC)
#error "LL_ATON_RT_RELOC should be defined to use the relocatable models"
#endif

/* -----------------------------------------------------------------------------
 * AI RELOC definition
 * -----------------------------------------------------------------------------
 */

/* version of the AI RELOC runtime (bootstrap) */
#define AI_RELOC_RT_VERSION_MAJOR 8 /* for NPU/ATON env. */
#define AI_RELOC_RT_VERSION_MINOR 0

/* AI RT executing mode definitions */
#define AI_RELOC_RT_LOAD_MODE_XIP                                                                                      \
  (1 << 0)                                   /* (default) only the data/bss section are                                \
                                            copied in RAM, code is executed in-place */
#define AI_RELOC_RT_LOAD_MODE_COPY  (1 << 1) /* code and data sections are copied in RAM */
#define AI_RELOC_RT_LOAD_MODE_CLEAR (1 << 2) /* clear the acts memory pools */

/* AI RT error definitions */
#define AI_RELOC_RT_ERR_NONE          (0)
#define AI_RELOC_RT_ERR_INVALID_BIN   (-1) /* Invalid binary object */
#define AI_RELOC_RT_ERR_MEMORY        (-2) /* RAM size is insufficient */
#define AI_RELOC_RT_ERR_NOT_SUPPORTED (-3) /* feature/option not supported */
#define AI_RELOC_RT_ERR_ARG           (-4) /* argument not valid */
#define AI_RELOC_RT_ERR_INVALID_HDL   (-5) /* handle not valid */
#define AI_RELOC_RT_ERR_INSTALL       (-6) /* installation failed */
#define AI_RELOC_RT_ERR_PARAM_ADDR    (-7) /* invalid param addr */
#define AI_RELOC_RT_ERR_PARAM_DESC    (-8) /* invalid param descriptor */

/*
 * AI RELOC flags (32b) - part of the binary header
 *
 * 	 b31..b24 : 8b  - RELOC API version major.minor (4b+4b)
 *
 *   b23..b20 : 4b  - fields reserved for post-process script
 *        b20 : SECURE mode
 *        b21 : LL_ATON_EB_DBG_INFO
 *        b22 : LL_ATON_RT_MODE == LL_ATON_RT_ASYNC
 *        b23 : reserved
 *
 *  Variant fields
 *
 * 	 b19..b12 : 8b  - compilation options: ARM tool-chain, FPU, FLOAT-ABI
 * 	                  b19..b16: 4b - ARM tool-chain - 1000b: GNU Arm Embedded Tool-chain
 * 	                  b15:      1b - reserved
 * 	                  b14.b13:  2b - Floating-point EABI used - '00b':soft, '01b':softfp, '10b':hard
 * 	                  b12:      1b - FPU is used
 * 	 b11..b0  : 12b - CPUID (Part Number fields of the @0xE000ED00 CPUID register)
 */

/* CPUID/Cortex-mM definition */
#define AI_RELOC_ARM_CORTEX_M0P (0xC60UL)
#define AI_RELOC_ARM_CORTEX_M3  (0xC23UL)
#define AI_RELOC_ARM_CORTEX_M4  (0xC24UL)
#define AI_RELOC_ARM_CORTEX_M7  (0xC27UL)
#define AI_RELOC_ARM_CORTEX_M33 (0xD21UL)
#define AI_RELOC_ARM_CORTEX_M55 (0xD22UL)

/* Tool-chain definition (ONLY this tool-chain is currently supported)*/
#define AI_RELOC_TOOLCHAIN_ARM_EMBEDDED (0x8UL)

/* Floating-point ABI definition (in relation with the tool-chain) */
#define AI_RELOC_TOOLCHAIN_FP_ABI_SOFT   (0x0UL)
#define AI_RELOC_TOOLCHAIN_FP_ABI_SOFTFP (0x1UL)
#define AI_RELOC_TOOLCHAIN_FP_ABI_HARD   (0x2UL)

/* Getter/setter macros to read/write the flags */
#define AI_RELOC_RT_SET_FLAGS(_var, _extra)                                                                            \
  (((AI_RELOC_RT_VERSION_MAJOR << 4 | AI_RELOC_RT_VERSION_MINOR << 0) << 24) | (((_extra)&0xF) << 20) |                \
   ((_var)&0xFFFFF))

#define AI_RELOC_RT_GET_MAJOR(_flags) (int)(((_flags) >> 28) & 0xF)
#define AI_RELOC_RT_GET_MINOR(_flags) (int)(((_flags) >> 24) & 0xF)

#define AI_RELOC_RT_GET_VARIANT(_flags) ((_flags)&0xFFFFF)
#define AI_RELOC_RT_GET_EXTRA(_flags)   (((_flags) >> 20) & 0xF)

/* VARIANT */
#define AI_RELOC_RT_GET_CPUID(_flags)    (((_flags) >> 0) & 0xFFF)
#define AI_RELOC_RT_GET_COMPILER(_flags) (((_flags) >> 16) & 0xF)
#define AI_RELOC_RT_GET_FPEABI(_flags)   (((_flags) >> 13) & 0x3)
#define AI_RELOC_RT_FPU_USED(_flags)     (((_flags) >> 12) & 1)

#define AI_RELOC_RT_SET_TOOLS(_val)    ((_val & 0xF) << 16)
#define AI_RELOC_RT_SET_ABI(_val)      ((_val & 3) << 13)
#define AI_RELOC_RT_SET_FPU(_val)      ((_val & 1) << 12)
#define AI_RELOC_RT_SET_MCU_CONF(_val) (_val & 0xFFF)

/* EXTRA flags */
#define AI_RELOC_RT_SECURE(_flags)     (((_flags) >> 20) & 1)
#define AI_RELOC_RT_DBG_INFO(_flags)   (((_flags) >> 21) & 1)
#define AI_RELOC_RT_ASYNC_MODE(_flags) (((_flags) >> 22) & 1)

#define AI_RELOC_MAGIC (0x4E49424EUL)

  /* Callback functions */
  struct ll_aton_reloc_callback
  {
    /* services */
    void (*assert_func)(const char *filename, int line, const char *assert_func, const char *expr);
    void (*ll_lib_error)(int err_code, int line, const char *func);

    /* cache operations */
    void (*ll_aton_cache_mcu_clean_range)(uintptr_t virtual_addr, uint32_t size);
    void (*ll_aton_cache_mcu_invalidate_range)(uintptr_t virtual_addr, uint32_t size);
    void (*ll_aton_cache_mcu_clean_invalidate_range)(uintptr_t virtual_addr, uint32_t size);
    void (*ll_aton_cache_npu_clean_range)(uintptr_t virtual_addr, uint32_t size);
    void (*ll_aton_cache_npu_clean_invalidate_range)(uintptr_t virtual_addr, uint32_t size);

    /* ll aton lib */
    int (*ll_aton_lib_concat)(const LL_Buffer_InfoTypeDef *inputs, unsigned int ninputs,
                              const LL_Buffer_InfoTypeDef *output, unsigned int axis, int dma_in, int dma_out);
    int (*ll_aton_lib_cast)(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output, int dma_in,
                            int dma_out);
    int (*ll_aton_lib_softmax)(const LL_LIB_TensorInfo_TypeDef *input, const LL_LIB_TensorInfo_TypeDef *output,
                               unsigned int axis, int legacy);
    int (*ll_aton_lib_dma_imagetorow)(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                      const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h,
                                      unsigned blocksize_w, unsigned stride_h, unsigned stride_w, int dma_in,
                                      int dma_out);
    int (*ll_aton_lib_dma_spacetodepth)(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                        const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h,
                                        unsigned blocksize_w, int dma_in, int dma_out);
    int (*ll_aton_lib_dma_rowtoimage)(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                      const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h,
                                      unsigned blocksize_w, unsigned stride_h, unsigned stride_w, int dma_in,
                                      int dma_out);
    int (*ll_aton_lib_dma_depthtospace)(const LL_LIB_TensorInfo_TypeDef *inputs, unsigned int ninputs,
                                        const LL_LIB_TensorInfo_TypeDef *output, unsigned blocksize_h,
                                        unsigned blocksize_w, int dma_in, int dma_out);
    int (*ll_aton_lib_dma_outputs_flat_copy)(const LL_LIB_TensorShape_TypeDef *input,
                                             const LL_LIB_TensorShape_TypeDef *outputs, unsigned int nr_of_outputs,
                                             int dma_in, int dma_out);
    int (*ll_aton_lib_dma_outputs_slice_splitlike)(const LL_LIB_TensorShape_TypeDef *input,
                                                   const LL_LIB_TensorShape_TypeDef *output, int32_t tot_out_size,
                                                   int32_t width_in_bytes, int32_t fheight, int32_t line_offset,
                                                   int8_t n_bits, int dma_in, int dma_out);
    int (*ll_aton_lib_dma_outputs_channel_split_aton)(const LL_LIB_TensorShape_TypeDef *input,
                                                      const LL_LIB_TensorShape_TypeDef *outputs,
                                                      unsigned int nr_of_outputs, unsigned int leading_dims, int dma_in,
                                                      int dma_out);
    int (*ll_aton_lib_dma_outputs_channel_split_batched)(const LL_LIB_TensorShape_TypeDef *input,
                                                         const LL_LIB_TensorShape_TypeDef *outputs,
                                                         unsigned int nr_of_outputs, int dma_in, int dma_out);
    int (*ll_aton_lib_dma_pad_memset)(void *output, int32_t constant_value, size_t c,
                                      __ll_pad_sw_params_t *common_params);
    int (*ll_aton_lib_dma_pad_filling)(__ll_pad_sw_params_t *init_common_params);
    int (*ll_aton_lib_dma_transpose)(const LL_LIB_TensorShape_TypeDef *input, const uint32_t *input_axes_offsets,
                                     const LL_LIB_TensorShape_TypeDef *output, const uint32_t *output_axes_offsets,
                                     const uint8_t *target_pos, const uint8_t *perm_to_use, int dma_in, int dma_out);
  };

  /* AI RELOC RT context definition */
  struct ai_reloc_rt_ctx
  {
    volatile uint32_t state;            /* current state */
    uint32_t file_addr;                 /* file address of the binary (flashed location) */
    uint32_t ram_addr;                  /* loaded base address for the RAM segment */
    uint32_t rom_addr;                  /* loaded base address for the ROM segment */
    struct ll_aton_reloc_callback *cbs; /* callback functions */
    const char *c_name;                 /* c-name of model */
    const uint32_t acts_sz;             /* size for the activations */
    const uint32_t params_sz;           /* size for the params/weights */
    const uint32_t ext_ram_sz;          /* requested external ram size for the activations (and params) */
    const char *rt_version_desc;        /* rt description */
    const uint32_t rt_version;          /* rt version */
    const uint32_t rt_version_extra;    /* rt version extra */
    const uint32_t params_bin_sz;       /* size of the binary blob to handle the params/weights */
    const uint32_t params_bin_crc32;    /* crc32 value of the binary blob */
    NN_Instance_TypeDef *ll_instance;   /* associated user NN Instance for LL ATON code */
    const NN_Interface_TypeDef *itf_network;
  };

#define AI_RELOC_RT_STATE_NOT_INITIALIZED (0)
#define AI_RELOC_RT_STATE_INITIALIZED     (1 << 0)
#define AI_RELOC_RT_STATE_XIP_MODE        (1 << 1)

  typedef struct _ll_aton_reloc_info
  {
    const char *c_name;          /* c-name of the model */
    uint32_t variant;            /* 32-b word to handle the reloc rt version,
                              the used ARM Embedded compiler,
                              Cortex-Mx (CPUID) and if the FPU is requested */
    uint32_t code_sz;            /* size of the code (header + txt + rodata + data + got + rel sections) */
    uint32_t params_off;         /* offset (in bytes) of the weights */
    uint32_t params_sz;          /* size (in bytes) of the weights */
    uint32_t acts_sz;            /* minimum requested RAM size (in bytes) for the activations buffer */
    uint32_t ext_ram_sz;         /* requested external ram size for the activations (and params) */
    uint32_t rt_ram_xip;         /* minimum requested RAM size to install it, XIP mode */
    uint32_t rt_ram_copy;        /* minimum requested RAM size to install it, COPY mode */
    const char *rt_version_desc; /* rt description */
    uint32_t rt_version;         /* rt version */
    uint32_t rt_version_extra;   /* rt version extra */
  } ll_aton_reloc_info;

  typedef struct _ll_aton_reloc_config
  {
    uintptr_t exec_ram_addr;  /* base@ of the exec memory region to place the relocatable code/data (8-Bytes aligned) */
    uint32_t exec_ram_size;   /* max size in byte of the exec memory region */
    uintptr_t ext_ram_addr;   /* base@ of the external memory region to place the external pool (if requested) */
    size_t ext_ram_size;      /* max size in byte of the external memory region */
    uintptr_t ext_param_addr; /* base@ of the param memory region (if requested) */
    uint32_t mode;
  } ll_aton_reloc_config;

  typedef struct _ll_aton_reloc_mem_pool_desc
  {
    const char *name; /* name */
    uint32_t flags;   /* type definition: 32b:4x8b <type><data_type><reserved><id> */
    uint32_t foff;    /* offset in the binary file */
    uint32_t dst;     /* dst @ */
    uint32_t size;    /* real size */
  } ll_aton_reloc_mem_pool_desc;

#define AI_RELOC_MPOOL_TYPE_RELOC 1
#define AI_RELOC_MPOOL_TYPE_COPY  2
#define AI_RELOC_MPOOL_TYPE_RESET 3

#define AI_RELOC_MPOOL_GET_TYPE(_flags) (((_flags) >> 24) & 0xFF)

#define AI_RELOC_MPOOL_IS_RELOC(_flags) (AI_RELOC_MPOOL_GET_TYPE(_flags) == AI_RELOC_MPOOL_TYPE_RELOC)
#define AI_RELOC_MPOOL_IS_COPY(_flags)  (AI_RELOC_MPOOL_GET_TYPE(_flags) == AI_RELOC_MPOOL_TYPE_COPY)
#define AI_RELOC_MPOOL_IS_RESET(_flags) (AI_RELOC_MPOOL_GET_TYPE(_flags) == AI_RELOC_MPOOL_TYPE_RESET)

#define AI_RELOC_MPOOL_DTYPE_PARAM 1
#define AI_RELOC_MPOOL_DTYPE_ACTIV 2
#define AI_RELOC_MPOOL_DTYPE_MIXED 3

#define AI_RELOC_MPOOL_GET_DTYPE(_flags) (((_flags) >> 16) & 0xFF)

#define AI_RELOC_MPOOL_IS_PARAM(_flags) (AI_RELOC_MPOOL_GET_DTYPE(_flags) == AI_RELOC_MPOOL_DTYPE_PARAM)
#define AI_RELOC_MPOOL_IS_ACTIV(_flags) (AI_RELOC_MPOOL_GET_DTYPE(_flags) == AI_RELOC_MPOOL_DTYPE_ACTIV)
#define AI_RELOC_MPOOL_IS_MIXED(_flags) (AI_RELOC_MPOOL_GET_DTYPE(_flags) == AI_RELOC_MPOOL_DTYPE_MIXED)

#define AI_RELOC_MPOOL_DATTR_READ      1
#define AI_RELOC_MPOOL_DATTR_WRITE     2
#define AI_RELOC_MPOOL_DATTR_CACHEABLE 4

#define AI_RELOC_MPOOL_GET_ATTR(_flags) (((_flags) >> 8) & 0xFF)

#define AI_RELOC_MPOOL_IS_CACHEABLE(_flags) (AI_RELOC_MPOOL_GET_ATTR(_flags) & AI_RELOC_MPOOL_DATTR_CACHEABLE)
#define AI_RELOC_MPOOL_IS_READ(_flags)      (AI_RELOC_MPOOL_GET_ATTR(_flags) & AI_RELOC_MPOOL_DATTR_READ)
#define AI_RELOC_MPOOL_IS_WRITE(_flags)     (AI_RELOC_MPOOL_GET_ATTR(_flags) & AI_RELOC_MPOOL_DATTR_WRITE)

#define AI_RELOC_MPOOL_GET_ID(_flags) ((_flags)&0xFF)

  /* -----------------------------------------------------------------------------
   * Public API declaration
   * ----------------------------------------------------------------------------- */

  void ll_aton_reloc_log_info(const uintptr_t file_ptr);

  int ll_aton_reloc_get_info(const uintptr_t file_ptr, ll_aton_reloc_info *rt);

  int ll_aton_reloc_install(const uintptr_t file_ptr, const ll_aton_reloc_config *config,
                            NN_Instance_TypeDef *nn_instance);

  int ll_aton_reloc_is_valid(const NN_Instance_TypeDef *nn_instance);
  int ll_aton_reloc_get_file_ptr(const NN_Instance_TypeDef *nn_instance, uintptr_t *file_ptr);

  const LL_Buffer_InfoTypeDef *ll_aton_reloc_get_input_buffers_info(const NN_Instance_TypeDef *nn_instance,
                                                                    int32_t num);
  const LL_Buffer_InfoTypeDef *ll_aton_reloc_get_output_buffers_info(const NN_Instance_TypeDef *nn_instance,
                                                                     int32_t num);

  LL_ATON_User_IO_Result_t ll_aton_reloc_set_input(const NN_Instance_TypeDef *nn_instance, uint32_t num, void *buffer,
                                                   uint32_t size);
  LL_ATON_User_IO_Result_t ll_aton_reloc_set_output(const NN_Instance_TypeDef *nn_instance, uint32_t num, void *buffer,
                                                    uint32_t size);
  void *ll_aton_reloc_get_input(const NN_Instance_TypeDef *nn_instance, uint32_t num);
  void *ll_aton_reloc_get_output(const NN_Instance_TypeDef *nn_instance, uint32_t num);

  const LL_Buffer_InfoTypeDef *ll_aton_reloc_get_internal_buffers_info(const NN_Instance_TypeDef *nn_instance);

  ll_aton_reloc_mem_pool_desc *ll_aton_reloc_get_mem_pool_desc(const uintptr_t file_ptr, int index);
  int ll_aton_reloc_set_callbacks(const NN_Instance_TypeDef *nn_instance, const struct ll_aton_reloc_callback *cbs);

  /* -----------------------------------------------------------------------------
   * Internal entry points of the relocatable model
   * ----------------------------------------------------------------------------- */

  bool ai_rel_network_ec_network_init(uintptr_t inst);
  bool ai_rel_network_ec_inference_init(uintptr_t inst);

  LL_ATON_User_IO_Result_t ai_rel_network_set_input(uintptr_t inst, uint32_t num, void *buffer, uint32_t size);
  void *ai_rel_network_get_input(uintptr_t inst, uint32_t num);

  LL_ATON_User_IO_Result_t ai_rel_network_set_output(uintptr_t inst, uint32_t num, void *buffer, uint32_t size);
  void *ai_rel_network_get_output(uintptr_t inst, uint32_t num);

  const EpochBlock_ItemTypeDef *ai_rel_network_get_epoch_items(uintptr_t inst);
  const LL_Buffer_InfoTypeDef *ai_rel_network_get_output_buffers_info(uintptr_t inst);
  const LL_Buffer_InfoTypeDef *ai_rel_network_get_input_buffers_info(uintptr_t inst);
  const LL_Buffer_InfoTypeDef *ai_rel_network_get_internal_buffers_info(uintptr_t inst);

  typedef void (*volatile start_end_func_ptr)(const void *epoch_block);

  void ai_rel_call_start_end_function(uintptr_t inst, start_end_func_ptr fct, const void *epoch_block);

#if defined(BUILD_AI_NETWORK_RELOC)

  /* -----------------------------------------------------------------------------
   * This part is only used during the compilation of the network.c to
   * generate the entry points (see linker and relocatable_pp.py files).
   * -----------------------------------------------------------------------------
   */

#if !defined(MODEL_CONF)
#include "network_reloc_conf.h"
#endif

#if !defined(C_NAME)
#define C_NAME "network"
#endif

#if !defined(ACTS_SZ)
#define ACTS_SZ (2222)
#endif

#if !defined(PARAMS_SZ)
#define PARAMS_SZ (3333)
#endif

#if !defined(EXT_RAM_SZ)
#error "EXT_RAM_SZ should be defined"
#endif

#if !defined(RUNTIME_DESC)
#define RUNTIME_DESC "RUNTIME undefined"
#endif

#if !defined(RUNTIME_VERSION)
#define RUNTIME_VERSION (0)
#endif

#if !defined(RUNTIME_VERSION_DEV)
#define RUNTIME_VERSION_DEV (0)
#endif

#if !defined(ARM_MCU_CONF)
#define ARM_MCU_CONF AI_RELOC_ARM_CORTEX_M55
#endif

#if !defined(ARM_TOOLS_CONF)
#define ARM_TOOLS_CONF AI_RELOC_TOOLCHAIN_ARM_EMBEDDED
#endif

#if !defined(SECURE_CONF)
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#define SECURE_FLAG 1
#else
#define SECURE_FLAG 0
#endif
#endif

#if defined(LL_ATON_EB_DBG_INFO)
#define DBG_INFO_FLAG 1
#else
#define DBG_INFO_FLAG 0
#endif

#if defined(LL_ATON_RT_MODE) && LL_ATON_RT_MODE == LL_ATON_RT_ASYNC
#define ASYNC_MODE_FLAG 1
#else
#define ASYNC_MODE_FLAG 0
#endif

/* build EXTRA flags defintion */
#define EXTRA (SECURE_FLAG | (DBG_INFO_FLAG << 1) | (ASYNC_MODE_FLAG << 2))

  /* https://developer.arm.com/documentation/dui0774/l/Other-Compiler-specific-Features/Predefined-macros?lang=en#a183-predefined-macros__when-the-softfp-predefined-macro-is-defined
   */

#if __ARM_FP /* Set if hardware floating-point is available */
#define _FPU 1UL
#if __SOFTFP__
#define _EABI AI_RELOC_TOOLCHAIN_FP_ABI_SOFT /* -mfloat-abi=soft */
#else
#define _EABI AI_RELOC_TOOLCHAIN_FP_ABI_HARD /* -mfloat-abi=hard */
#endif
#else /* __ARM_FP */
#define _FPU 0UL
#if __SOFTFP__
#define _EABI AI_RELOC_TOOLCHAIN_FP_ABI_SOFT /* -mfloat-abi=soft */
#else
#define _EABI AI_RELOC_TOOLCHAIN_FP_ABI_HARD /* -mfloat-abi=hard */
#endif
#endif /* !__ARM_FP */

#if _EABI != AI_RELOC_TOOLCHAIN_FP_ABI_HARD
#error "Only -mfloat-abi=hard is supported"
#endif

#if !defined(VARIANT)
/* Default variant definition */
#define VARIANT                                                                                                        \
  (AI_RELOC_RT_SET_TOOLS(ARM_TOOLS_CONF) | AI_RELOC_RT_SET_ABI(_EABI) | AI_RELOC_RT_SET_FPU(_FPU) |                    \
   AI_RELOC_RT_SET_MCU_CONF(ARM_MCU_CONF))
#endif

#if !defined(__GNUC__)
#error "AI_NETWORK_RELOC code generation is only supported with a GCC-based tool-chain"
#endif

#if !defined(INCLUDE_INTERNAL_BUFFERS)
  const LL_Buffer_InfoTypeDef *LL_ATON_Internal_Buffers_Info_Default_Empty(void)
  {
    return NULL;
  };

#define _LL_ATON_INTERNAL_BUFFERS LL_ATON_Internal_Buffers_Info_Default_Empty
#else
#define _LL_ATON_INTERNAL_BUFFERS LL_ATON_Internal_Buffers_Info_Default
#endif

  /*
   *  Entry table to handle the offset of network entry point and
   *  the RT context.
   */
  struct ai_reloc_network_entries
  {
    bool (*ec_network_init)(void);
    bool (*ec_inference_init)(void);
    LL_ATON_User_IO_Result_t (*input_setter)(uint32_t num, void *buffer, uint32_t size);
    void *(*input_getter)(uint32_t num);
    LL_ATON_User_IO_Result_t (*output_setter)(uint32_t num, void *buffer, uint32_t size);
    void *(*output_getter)(uint32_t num);
    const EpochBlock_ItemTypeDef *(*get_epoch_items)(void);
    const LL_Buffer_InfoTypeDef *(*get_output_buffers_info)(void);
    const LL_Buffer_InfoTypeDef *(*get_input_buffers_info)(void);
    const LL_Buffer_InfoTypeDef *(*get_internal_buffers_info)(void);
    struct ai_reloc_rt_ctx *rt_ctx;
  };

  const NN_Interface_TypeDef _itf_network = {.network_name = C_NAME};

#define AI_RELOC_NETWORK()                                                                                             \
  struct ai_reloc_rt_ctx __attribute__((used, section(".network_rt_ctx"), )) _network_rt_ctx = {0,                     \
                                                                                                0,                     \
                                                                                                0,                     \
                                                                                                0,                     \
                                                                                                NULL,                  \
                                                                                                C_NAME,                \
                                                                                                ACTS_SZ,               \
                                                                                                PARAMS_SZ,             \
                                                                                                EXT_RAM_SZ,            \
                                                                                                RUNTIME_DESC,          \
                                                                                                RUNTIME_VERSION,       \
                                                                                                RUNTIME_VERSION_DEV,   \
                                                                                                PARAMS_BIN_SZ,         \
                                                                                                PARAMS_BIN_CRC32,      \
                                                                                                NULL,                  \
                                                                                                &_itf_network};        \
  const struct ai_reloc_network_entries __attribute__((used, section(".network_entries"), visibility("default")))      \
  _network_entries = {                                                                                                 \
      .ec_network_init = &LL_ATON_EC_Network_Init_Default,                                                             \
      .ec_inference_init = &LL_ATON_EC_Inference_Init_Default,                                                         \
      .input_setter = &LL_ATON_Set_User_Input_Buffer_Default,                                                          \
      .input_getter = &LL_ATON_Get_User_Input_Buffer_Default,                                                          \
      .output_setter = &LL_ATON_Set_User_Output_Buffer_Default,                                                        \
      .output_getter = &LL_ATON_Get_User_Output_Buffer_Default,                                                        \
      .get_epoch_items = &LL_ATON_EpochBlockItems_Default,                                                             \
      .get_output_buffers_info = &LL_ATON_Output_Buffers_Info_Default,                                                 \
      .get_input_buffers_info = &LL_ATON_Input_Buffers_Info_Default,                                                   \
      .get_internal_buffers_info = &_LL_ATON_INTERNAL_BUFFERS,                                                         \
      .rt_ctx = &_network_rt_ctx,                                                                                      \
  };                                                                                                                   \
  const uint32_t __attribute__((used, section(".network_flags"), visibility("default"))) _network_flags =              \
      AI_RELOC_RT_SET_FLAGS(VARIANT, EXTRA);

#else

#define AI_RELOC_NETWORK()

#endif

#ifdef __cplusplus
}
#endif

#endif /* __LL_ATON_RELOC_NETWORK_H__ */
