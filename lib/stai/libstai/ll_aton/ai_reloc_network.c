/**
 ******************************************************************************
 * @file    ai_reloc_network.h
 * @author  MCD/AIS Team
 * @brief   Relocatable network support
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019,2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(STM32F7)
#include "stm32f7xx_hal.h"
#endif

#if defined(STM32H7)
#include "stm32h7xx_hal.h"
#endif

#include <ai_reloc_network.h>

/* -----------------------------------------------------------------------------
 * APP definitions
 * -----------------------------------------------------------------------------
 */

#if !defined(APP_DEBUG)
#define APP_DEBUG                1   /* 1: enable debug trace (printf-based) */
#endif

#if !defined(AI_RELOC_RT_MCU_CHECKING)
#define AI_RELOC_RT_MCU_CHECKING 1   /* 1: enable RT MCU checking */
#endif

#ifndef AI_RELOC_MALLOC
#define AI_RELOC_MALLOC(_size) malloc(_size)
#endif

#ifndef AI_RELOC_FREE
#define AI_RELOC_FREE(_ptr)    free(_ptr)
#endif


/* -----------------------------------------------------------------------------
 * AI RELOC definitions to manage the binary image
 * -----------------------------------------------------------------------------
 */

#define AI_RELOC_MAGIC       (0x4E49424E)

#define AI_RELOC_FLASH_BASE  (0x20000000)
#define AI_RELOC_RAM_BASE    (0x80000000)

#define AI_RELOC_MASK_OFFSET (0x0FFFFFFF)

#define AI_RELOC_GET_OFFSET(laddr) (uintptr_t)((laddr) & AI_RELOC_MASK_OFFSET)

#define AI_RELOC_GET_ADDR(_base, _off) ((uintptr_t)_base + AI_RELOC_GET_OFFSET(_off))
#define AI_RELOC_GET_VAL(_base, _off) ((uintptr_t)_base + AI_RELOC_GET_OFFSET(_off))

#define AI_RELOC_IN_RAM(val) \
    ((val & 0xF0000000) == AI_RELOC_RAM_BASE)

#define AI_RELOC_IN_FLASH(val) \
    ((val & 0xF0000000) == AI_RELOC_FLASH_BASE)

#define AI_RELOC_ROUND_UP(_v) (((_v) + 3) & ~3)

#define AI_RELOC_IS_ALIGNED(_v) (((_v) & 0x3) == 0)

struct bin_hdr {
  uint32_t magic;
  uint32_t flags;
};

struct sec_info {
  uint32_t data_start;
  uint32_t data_end;
  uint32_t data_data;

  uint32_t bss_start;
  uint32_t bss_end;

  uint32_t got_start;
  uint32_t got_end;
  uint32_t rel_start;
  uint32_t rel_end;
  uint32_t weights_start;
  uint32_t weights_end;
};

struct net_entries {
  uint32_t create;
  uint32_t init;
  uint32_t init_v2;
  uint32_t run;
  uint32_t report;
  uint32_t error;
  uint32_t destroy;
  uint32_t forward;
  uint32_t plt_obs_register;
  uint32_t plt_obs_unregister;
  uint32_t plt_obs_node_info;
  uint32_t ctx;
};

struct ai_reloc_bin_hdr {
  struct bin_hdr      hdr;
  struct sec_info     sect;
  struct net_entries  vec;
};

#define APP_FLASH_RELOC(laddr, offset)\
    (laddr + (offset & AI_RELOC_MASK_OFFSET))

/*
 * Naked function to set the R9 value and to call the entry point
 * without a "prolog" and "epilog".
 *
 * 	r0 = ROM base address
 * 	r1 = offset of the function
 * 	r2 = RAM base address (used for R9)
 * 	r3      = arg1		 -> r0
 * 	sp[0]   = arg2       -> r1
 * 	sp[0+4] = arg3       -> r2
 *
 */

#if defined(__GNUC__) && !defined(__ARMCC_VERSION) /* GNU compiler */

static uintptr_t __attribute__((naked)) call_with_r9(const void *base,
		uint32_t offset, void *data,
		uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    asm volatile (
        "add  r12, r0, r1      \n"
        "mov  r0,  r3          \n"
        "ldr  r1,  [sp]        \n"
        "push {r9, lr}         \n"
        "mov  r9, r2           \n"
        "ldr  r2,  [sp, #12]   \n"
        "blx  r12              \n"
        "pop  {r9, pc}         \n"
    );

    return 0; // dummy to fool gcc
}

#elif defined(__ICCARM__) /* IAR compiler */

__task __irq  uintptr_t call_with_r9(const void *base,
		uint32_t offset, void *data,
		uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
__task __irq uintptr_t call_with_r9(const void *base,
		uint32_t offset, void *data,
		uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    asm volatile (
		"add  r12, r0, r1      \n"
        "mov  r0,  r3          \n"
        "ldr  r1,  [sp]        \n"
        "push {r9, lr}         \n"
        "mov  r9, r2           \n"
        "ldr  r2,  [sp, #12]   \n"
        "blx  r12              \n"
        "pop  {r9, pc}         \n"
    );

    return 0; // dummy to fool gcc
}

#elif defined(__CC_ARM) /* Arm compiler 4/5 */

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

#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) /* Arm Compiler 6 */

static uintptr_t __attribute__((naked)) call_with_r9(const void *base,
		uint32_t offset, void *data,
		uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    __asm (
        "add  r12, r0, r1      \n"
        "mov  r0,  r3          \n"
        "ldr  r1,  [sp]        \n"
        "push {r9, lr}         \n"
        "mov  r9, r2           \n"
        "ldr  r2,  [sp, #12]   \n"
        "blx  r12              \n"
        "pop  {r9, pc}         \n"
    );

}

#else

#error Unknown compiler.

#endif



AI_DECLARE_STATIC
ai_handle _ai_reloc_network_data_weights_get(const void* obj)
{
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)obj;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0))
    return AI_HANDLE_NULL;

  if ((bin->sect.weights_start == bin->sect.weights_end) ||
      (bin->sect.weights_start == 0))
    return AI_HANDLE_NULL;

  const uint32_t off =  AI_RELOC_GET_OFFSET(bin->sect.weights_start);

  return (ai_handle)((uintptr_t)obj + off);
}

AI_DECLARE_STATIC
uint32_t _ai_reloc_code_size(const void* obj)
{
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)obj;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0))
    return 0;

  const ai_size ro_sz =  AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(bin->sect.data_data));

  return ro_sz;
}

AI_DECLARE_STATIC
ai_size _ai_reloc_requested_ram_size(const void* obj, uint32_t mode)
{
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)obj;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0))
    return 0;

  const ai_size rw_sz =  AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(bin->sect.bss_end));
  const ai_size ro_sz =  AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(bin->sect.data_data));

  if ((mode & AI_RELOC_RT_LOAD_MODE_XIP) == AI_RELOC_RT_LOAD_MODE_XIP)
    return rw_sz;
  else
    return rw_sz + ro_sz;
}


#if defined(APP_DEBUG) && APP_DEBUG == 1
AI_DECLARE_STATIC
char* _magic_to_str(uint32_t val) {
	static char res[5];
	res[3] = val >> 24;
	res[2] = val >> 16;
	res[1] = val >> 8;
	res[0] = val >> 0;
	return res;
}
#endif

#define _CPUID  *(volatile uint32_t *)(0xE000ED00)
#define _CPACR  *(volatile uint32_t *)(0xE000ED88)

#define _CPUID_PART_NUMBER (0xFFF << 4)   /* Part Number */
#define _CPACR_CPx         (0xF << 20)    /* CP1 & CP0 bits */

#define _GET_PART_NUMBER() (int)((_CPUID & _CPUID_PART_NUMBER) >> 4)
#define _GET_FPU_CPX()     (int)((_CPACR & _CPACR_CPx) >> 20)

AI_DECLARE_STATIC
int ai_reloc_rt_mcu_checking(const struct ai_reloc_bin_hdr *bin)
{
#if defined(AI_RELOC_RT_MCU_CHECKING) && AI_RELOC_RT_MCU_CHECKING == 1
  const uint32_t flags = bin->hdr.flags;
  const uint32_t cpuid = _GET_PART_NUMBER();

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0)) {
#if defined(APP_DEBUG) && APP_DEBUG == 1
    printf("AI RELOC ERROR: Binary is invalid\r\n");
#endif
      return -1;
  }

  if (cpuid != AI_RELOC_RT_GET_CPUID(flags)) {
#if defined(APP_DEBUG) && APP_DEBUG == 1
    printf("AI RELOC ERROR: CPUID is invalid 0x%03X (expected 0x%03X)\r\n", (int)cpuid,
        (int)AI_RELOC_RT_GET_CPUID(flags));
#endif
    return -2;
  }
  if (AI_RELOC_RT_FPU_USED(flags)) {
    if (!_GET_FPU_CPX()) {
#if defined(APP_DEBUG) && APP_DEBUG == 1
      printf("AI RELOC ERROR: FPU should be initialized\r\n");
#endif
      return -3;
    }
  }
#endif /* AI_RELOC_RT_MCU_CHECKING == 1 */
  return 0;
}

AI_DECLARE_STATIC
void ai_reloc_log_hdr(const struct ai_reloc_bin_hdr *bin, uint32_t mode)
{
#if defined(APP_DEBUG) && APP_DEBUG == 1
  ai_rel_network_info rt_info;

  ai_rel_network_rt_get_info(bin, &rt_info);

  printf("\r\nAI binary network image (0x%08x)\r\n", (int)bin);
  printf("  c-name        : \"%s\"\r\n", rt_info.c_name);
  printf("  activations   : %d\r\n", (int)rt_info.acts_sz);
  printf("  weights       : %d\r\n", (int)rt_info.weights_sz);
  printf("  ram size      : %d for XIP mode (%d for COPY mode)\r\n",
      (int)rt_info.rt_ram_xip,
      (int)rt_info.rt_ram_copy);
  printf("                  requested mode : %s\r\n",
      mode == AI_RELOC_RT_LOAD_MODE_XIP?"XIP":"COPY");

  printf("\r\n Binary header\r\n");
  printf("  magic         : 0x%08X (%s)\r\n", (int)bin->hdr.magic,
      _magic_to_str(bin->hdr.magic));
  printf("  flags         : v%d.%d (0x%08X)\r\n",
      AI_RELOC_RT_GET_MAJOR(rt_info.variant),
      AI_RELOC_RT_GET_MINOR(rt_info.variant),
      (int)rt_info.variant);
  printf("  size          : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.rel_end));
  printf("  .txt/.rodata  : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.data_data));
  printf("  .data         : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.data_end) -
      (int)AI_RELOC_GET_OFFSET(bin->sect.data_start));
  printf("  .got          : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.got_end) -
      (int)AI_RELOC_GET_OFFSET(bin->sect.got_start));
  printf("  .rel          : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.rel_end) -
      (int)AI_RELOC_GET_OFFSET(bin->sect.rel_start));
  printf("  .bss          : %d\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.bss_end) -
      (int)AI_RELOC_GET_OFFSET(bin->sect.bss_start));
  printf("  .weights      : %d (0x%08x)\r\n", (int)AI_RELOC_GET_OFFSET(bin->sect.weights_end) -
      (int)AI_RELOC_GET_OFFSET(bin->sect.weights_start), (int)rt_info.weights);

  printf("\r\n Runtime\r\n");
  int fpu_is_enabled = _GET_FPU_CPX();
  printf("  CPUID         : 0x%03x (FPU is %s)\r\n", _GET_PART_NUMBER(),
      fpu_is_enabled?"enabled":"disabled");

  printf("\r\n");
#endif /* APP_DEBUG == 1 */
}

/*
 * Low level function to update the GOT section in RAM.
 */
AI_DECLARE_STATIC
int _ai_reloc_got_update(const struct ai_reloc_bin_hdr *bin, void* ram_addr)
{
  uint32_t *got_start = (uint32_t *)AI_RELOC_GET_ADDR(ram_addr, bin->sect.got_start);
  uint32_t *got_end = (uint32_t *)AI_RELOC_GET_ADDR(ram_addr, bin->sect.got_end);

  for (uint32_t *p = got_start; p < got_end; p++) {
    uint32_t val = *p;
    if AI_RELOC_IN_RAM(val) {
      val = (uint32_t)AI_RELOC_GET_VAL(ram_addr, val);
    } else if AI_RELOC_IN_FLASH(val) {
      val = (uint32_t)AI_RELOC_GET_VAL(bin, val);
    } else if (val != 0) {
#if defined(APP_DEBUG) && APP_DEBUG == 1
      printf("AI RELOC ERROR: _ai_reloc_got_update - val is invalid %08x\r\n", (int)val);
#endif
      return -1;
    }
    *p = val;
  }
  return 0;
}

/*
 * Low level function to update the DATA section in RAM.
 */
AI_DECLARE_STATIC
int _ai_reloc_ram_update(const struct ai_reloc_bin_hdr *bin, void* ram_addr, const void* obj)
{
  uint32_t *rel_start = (uint32_t *)AI_RELOC_GET_ADDR(obj, bin->sect.rel_start);
  uint32_t *rel_end = (uint32_t *)AI_RELOC_GET_ADDR(obj, bin->sect.rel_end);

  for (uint32_t *p = rel_start; p < rel_end; p++) {
    uint32_t add = *p;
    uint32_t val = *(uint32_t*)AI_RELOC_GET_VAL(ram_addr, add);
    if AI_RELOC_IN_RAM(val) {
      val = (uint32_t)AI_RELOC_GET_VAL(ram_addr, val);
    } else if AI_RELOC_IN_FLASH(val) {
      val = (uint32_t)AI_RELOC_GET_VAL(bin, val);
    } else if (val != 0) {
#if defined(APP_DEBUG) && APP_DEBUG == 1
      printf("AI RELOC ERROR: _ai_reloc_ram_update - val is invalid %08x\r\n", (int)val);
#endif
      return -1;
    }
    uint32_t *dest = (uint32_t *)AI_RELOC_GET_ADDR(ram_addr, add);
    *dest = val;
  }
  return 0;
}

/*
 * Low level function to install the relocatable code.
 *
 * - 'obj' address of the memory-mapped binary object.
 * - ram_addr/ram_size indicates the location (and the size)
 *   of the buffer destination to install and to update the data/got
 *   and bss sections for XIP mode or including the hdr/text and
 *   rodata sections for COPY mode. rel section is only used
 *   at init time. Note: if ram_addr and/or ram_size are NULL,
 *   requested RAM size is dynamically allocated in the system
 *   heap (AI_RELOC_MALLOC/AI_RELOC_FREE macros).
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
 * obj@                               ram_addr@
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
 *                                      [ hd      ]
 *                 -- COPY mode -->     [ text    ]
 *                                      [ rodata  ]
 *                                      ----------- new ram_addr@
 *                                      [ data    ]
 *                                      [ got     ]
 *                                      [ bss     ]
 *                                      -----------
 *
 */
AI_DECLARE_STATIC
int _ai_reloc_install(const void* obj, void* ram_addr, size_t ram_size,
    ai_handle* hdl, uint32_t mode)
{
  uint32_t state = AI_RELOC_RT_STATE_NOT_INITIALIZED;
  struct ai_reloc_bin_hdr *rom_addr = (struct ai_reloc_bin_hdr *)obj;
  const uint32_t req_ram_size = _ai_reloc_requested_ram_size(obj, mode);
  void *ram_alloc_addr = NULL;

  /* RT checking */
  if (ai_reloc_rt_mcu_checking(rom_addr)) {
    return AI_RELOC_RT_ERR_INVALID_BIN;
  }

  /* Parameter check */
  if (!req_ram_size)
    return AI_RELOC_RT_ERR_INVALID_BIN;

  if (((mode != AI_RELOC_RT_LOAD_MODE_XIP) &&
      (mode != AI_RELOC_RT_LOAD_MODE_COPY)) || (!hdl))
    return AI_RELOC_RT_ERR_PARAM;

  if (ram_addr && ram_size && !AI_RELOC_IS_ALIGNED((uintptr_t)ram_addr))
    return AI_RELOC_RT_ERR_MEMORY;

  /* Allocate memory if necessary */
  if (!ram_addr || !ram_size) {
    ram_size = req_ram_size;
    ram_alloc_addr = AI_RELOC_MALLOC(ram_size + 4);
    if (ram_alloc_addr) {
      ram_addr = (void *)AI_RELOC_ROUND_UP((uintptr_t)ram_alloc_addr);
    }
    else
      return AI_RELOC_RT_ERR_MEMORY;
  }
  else if (req_ram_size > ram_size)
    return AI_RELOC_RT_ERR_MEMORY;

  if (mode & AI_RELOC_RT_LOAD_MODE_COPY) {
    /* Copy hrd, txt and rodata sections in RAM */
    const uint32_t ro_sz =  AI_RELOC_ROUND_UP(AI_RELOC_GET_OFFSET(rom_addr->sect.data_data));
    memcpy(ram_addr, obj, ro_sz);

#if defined(STM32F7) || defined(STM32H7)
    SCB_CleanDCache();
#endif

    /* Update the rom_addr/ram_addr pointers */
    rom_addr = (struct ai_reloc_bin_hdr *)(ram_addr);
    ram_addr = (void *)((uintptr_t)ram_addr + ro_sz);
  }
  else {
    state |= AI_RELOC_RT_STATE_XIP_MODE;
  }

  ai_reloc_log_hdr(rom_addr, mode);

  const uintptr_t bss_start = AI_RELOC_GET_ADDR(ram_addr, rom_addr->sect.bss_start);
  const uint32_t  bss_size = rom_addr->sect.bss_end - rom_addr->sect.bss_start;
  const uintptr_t src_data = AI_RELOC_GET_ADDR(obj, rom_addr->sect.data_data);
  const uint32_t rw_sz =  AI_RELOC_GET_OFFSET(rom_addr->sect.bss_end);

  /* Copy the data section, including the got section */
  memcpy(ram_addr, (const void*)src_data, rw_sz - bss_size);

  /* Clear the bss section */
  memset((void *)bss_start, 0, bss_size);

  /* Update the relocation table and data */
  if (_ai_reloc_got_update(rom_addr, ram_addr))
    return AI_RELOC_RT_ERR_INVALID_BIN;

  if (_ai_reloc_ram_update(rom_addr, ram_addr, obj))
    return AI_RELOC_RT_ERR_INVALID_BIN;

  /* Update the RT context */
  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)AI_RELOC_GET_ADDR(ram_addr,
      rom_addr->vec.ctx);

  rt_ctx->rom_addr = (uint32_t)rom_addr;
  rt_ctx->ram_addr = (uint32_t)ram_addr;
  rt_ctx->ram_alloc_addr = (uint32_t)ram_alloc_addr;
  rt_ctx->state = (state | AI_RELOC_RT_STATE_INITIALIZED);

  *hdl = (ai_handle)(rt_ctx);

  return 0;
}

AI_DECLARE_STATIC
int _ai_rel_check_handler(ai_handle hdl)
{
  if (!hdl)
    return -1;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || !(rt_ctx->state & AI_RELOC_RT_STATE_INITIALIZED))
    return -1;

  return 0;
}

AI_DECLARE_STATIC
ai_error _ai_rel_create(ai_handle* hdl, const ai_buffer* network_config)
{
  if (!hdl || _ai_rel_check_handler(*hdl)) {
    ai_error err = {AI_ERROR_INVALID_HANDLE, AI_ERROR_CODE_INVALID_PTR};
    return err;
  }

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)*hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.create),
      (void *)rt_ctx->ram_addr, (uintptr_t)&rt_ctx->network, (uintptr_t)network_config, 0);

  const ai_error err = { .type = (res & 0xFF), .code = (res & 0xFFFFFF00) >> 8 };
  return err;
}

/* -----------------------------------------------------------------------------
 * Public API implementation
 * -----------------------------------------------------------------------------
 */

AI_API_ENTRY
ai_error ai_rel_network_rt_get_info(const void* obj, ai_rel_network_info* rt)
{
  struct ai_reloc_bin_hdr *bin = (struct ai_reloc_bin_hdr *)obj;

  if (!bin || (bin->hdr.magic != AI_RELOC_MAGIC) || (((uintptr_t)bin & 0x3) != 0) || (!rt)) {
#if defined(APP_DEBUG) && APP_DEBUG == 1
    printf("AI RELOC ERROR: Binary is invalid\r\n");
#endif
    ai_error err = {AI_ERROR_INVALID_HANDLE, AI_ERROR_CODE_INVALID_PTR};
    return err;
  }

  struct ai_reloc_rt_ctx *rt_ctx =
      (struct ai_reloc_rt_ctx *)AI_RELOC_GET_ADDR(bin +
          AI_RELOC_GET_OFFSET(bin->sect.data_data), bin->vec.ctx);

  const char *c_name = (const char *)AI_RELOC_GET_ADDR(bin,
      AI_RELOC_GET_OFFSET((int)rt_ctx->c_name));

  rt->c_name = c_name;
  rt->variant = (ai_u32)bin->hdr.flags;
  rt->weights = _ai_reloc_network_data_weights_get(obj);
  rt->weights_sz = (ai_size)rt_ctx->weights_size;
  rt->acts_sz = (ai_size)rt_ctx->act_size;
  rt->rt_ram_xip = _ai_reloc_requested_ram_size(obj, AI_RELOC_RT_LOAD_MODE_XIP);
  rt->rt_ram_copy = _ai_reloc_requested_ram_size(obj, AI_RELOC_RT_LOAD_MODE_COPY);
  rt->code_sz = _ai_reloc_code_size(obj);

  ai_error err = {AI_ERROR_NONE, AI_ERROR_CODE_NONE};
  return err;
}

AI_API_ENTRY
ai_error ai_rel_network_load_and_create(const void* obj, ai_handle ram_addr,
    ai_size ram_size, uint32_t mode,
    ai_handle* hdl)
{
  if (!hdl || !obj) {
    ai_error err = {AI_ERROR_INVALID_HANDLE, AI_ERROR_CODE_INVALID_PTR};
    return err;
  }

  int res = _ai_reloc_install(obj, ram_addr, ram_size, hdl, mode);

  if (!res) {
    return _ai_rel_create(hdl, NULL);
  }

  ai_error err = {AI_ERROR_CREATE_FAILED, AI_ERROR_CODE_NETWORK};

  return err;
}

AI_API_ENTRY
ai_bool ai_rel_network_init(ai_handle hdl, const ai_handle *weights, const ai_handle *act)
{
  if (_ai_rel_check_handler(hdl))
    return false;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.init_v2),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, (uintptr_t)weights, (uintptr_t)act);

  return res?true:false;
}

AI_API_ENTRY
ai_bool ai_rel_network_get_report(ai_handle hdl, ai_network_report* report)
{
  if (_ai_rel_check_handler(hdl))
    return false;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.report),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, (uintptr_t)report, 0);

  return res?true:false;
}

AI_API_ENTRY
ai_error ai_rel_network_get_error(ai_handle hdl)
{
  if (_ai_rel_check_handler(hdl)) {
    ai_error err = {AI_ERROR_INVALID_HANDLE, AI_ERROR_CODE_NETWORK};
    return err;
  }
  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.error),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, 0, 0);

  const ai_error err = { .type = (res & 0xFF), .code = (res & 0xFFFFFF00) >> 8 } ;
  return err;
}

AI_API_ENTRY
ai_i32 ai_rel_network_run(ai_handle hdl, const ai_buffer* input, ai_buffer* output)
{
  if (_ai_rel_check_handler(hdl))
    return 0;

  const struct ai_reloc_rt_ctx *rt_ctx = (const struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.run),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network,
      (uintptr_t)input, (uintptr_t)output);

  return (ai_i32)res;
}

AI_API_ENTRY
ai_handle ai_rel_network_destroy(ai_handle hdl)
{
  if (!_ai_rel_check_handler(hdl))
    return 0;

  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr, AI_RELOC_GET_OFFSET(bin->vec.destroy),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, 0, 0);

  rt_ctx->network = (ai_handle)res;
  if (rt_ctx->ram_alloc_addr) {
    AI_RELOC_FREE((void *)rt_ctx->ram_alloc_addr);
  }
  rt_ctx->state = AI_RELOC_RT_STATE_INITIALIZED;

  return rt_ctx->network;
}

AI_API_ENTRY
ai_bool ai_rel_platform_observer_register(ai_handle hdl,
    ai_observer_node_cb cb, ai_handle cookie, ai_u32 flags)
{
  if (_ai_rel_check_handler(hdl))
    return false;

  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  rt_ctx->obs_ctx.on_node = cb;
  rt_ctx->obs_ctx.cookie = (ai_handle)cookie;
  rt_ctx->obs_ctx.flags = flags;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr,
      AI_RELOC_GET_OFFSET(bin->vec.plt_obs_register),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, (uintptr_t)&rt_ctx->obs_ctx, 0);

  return res?true:false;
}

AI_API_ENTRY
ai_bool ai_rel_platform_observer_unregister(ai_handle hdl,
    ai_observer_node_cb cb, ai_handle cookie)
{
  if (_ai_rel_check_handler(hdl))
    return false;

  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr,
      AI_RELOC_GET_OFFSET(bin->vec.plt_obs_unregister),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, (uintptr_t)&rt_ctx->obs_ctx, 0);

  return res?true:false;
}

AI_API_ENTRY
ai_bool ai_rel_platform_observer_node_info(ai_handle hdl,
    ai_observer_node *node_info)
{
  if (_ai_rel_check_handler(hdl))
    return false;

  struct ai_reloc_rt_ctx *rt_ctx = (struct ai_reloc_rt_ctx *)hdl;
  const struct ai_reloc_bin_hdr *bin = (const struct ai_reloc_bin_hdr *)rt_ctx->rom_addr;

  uintptr_t res = call_with_r9((void *)rt_ctx->rom_addr,
      AI_RELOC_GET_OFFSET(bin->vec.plt_obs_node_info),
      (void *)rt_ctx->ram_addr, (uintptr_t)rt_ctx->network, (uintptr_t)node_info, 0);

  return res?true:false;
}

