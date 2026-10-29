// SPDX-License-Identifier: MIT
//
// Copyright (C) 2013-2024 OpenMV, LLC.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// NOTE this linker script is pre-processed using the CPP first before passing it
// to LD. It has access to the board config file, can define and use CPP macros,
// and it uses a lot of magic 🪄.

#include "umalloc.h"

/* Adds a section to a table */
#define OMV_ADD_SECTION(s) \
  LONG (ADDR(s)) \
  LONG (SIZEOF(s))

/* Aligns a section to the next power of 2. */
#define OMV_ALIGN_DMA_SECTION(section) \
    ALIGN(., 1 << LOG2CEIL(SIZEOF(section)))

/* The ARMv7-M MPU ignores the base address bits below the region size, so a DMA
   region that is not aligned to its (power of 2) size is silently misplaced and
   part of it stays cacheable. ARMv8-M regions have 32-byte granularity. */
#if (__ARM_ARCH == 7)
#define OMV_ASSERT_DMA_SECTION(section) \
    ASSERT((SIZEOF(section) == 0) || ((ADDR(section) & (SIZEOF(section) - 1)) == 0), \
           "DMA region not aligned to its size, see OMV_ASSERT_DMA_SECTION")
#else
#define OMV_ASSERT_DMA_SECTION(section)
#endif

/* Define a new DMA buffer region. */
#define OMV_DMA_MEMORY_NEW(X) \
    .dma.memory##X (NOLOAD) : ALIGN(32) \
    { \
        *(.d##X##_dma_buffer) \
        . = OMV_ALIGN_DMA_SECTION(.dma.memory##X); \
    } >OMV_DMA_MEMORY_D##X

/* Main GC heap for MicroPython */
#if defined(OMV_GC_BLOCK0_MEMORY)
.gc.block0 (NOLOAD) : ALIGN(4)
{
  _heap_start = .;
  . = . + OMV_GC_BLOCK0_SIZE;
  . = ALIGN(4);
  _heap_end  = .;
} >OMV_GC_BLOCK0_MEMORY
#endif

/* Heap memory for libc malloc/sbrk */
#if defined(OMV_HEAP_MEMORY)
.heap (NOLOAD) : ALIGN(4)
{
  __end__ = .;
  PROVIDE(end = .);
  . = . + OMV_HEAP_SIZE;
  . = ALIGN(4);
  _heap_limit = .;
} > OMV_HEAP_MEMORY
#else
/* No dedicated heap region; provide zero-size fallback so _sbrk links. */
PROVIDE(__end__ = .);
PROVIDE(_heap_limit = .);
#endif

/* Stack memory */
#if defined(OMV_STACK_MEMORY)
.stack (NOLOAD) : ALIGN(8)
{
  _sstack  = .;
  __StackLimit = .;
  . = . + OMV_STACK_SIZE;
  . = ALIGN(8);
  _estack  = .;
  __StackTop = .;
} > OMV_STACK_MEMORY
PROVIDE(__stack = __StackTop);
#endif

/* Streaming buffer memory */
#if defined(OMV_SB_MEMORY)
.sb_memory (NOLOAD) :
{
  . = ALIGN(4);
  _sb_memory_start = .;
  . = . + OMV_SB_SIZE;
  . = ALIGN(4);
  _sb_memory_end = .;
} >OMV_SB_MEMORY
#endif

/* GPU memory */
#if defined(OMV_GPU_MEMORY)
.gpu_memory (NOLOAD) :
{
  . = ALIGN(16);
  _gpu_memory_start = .;
  . = . + OMV_GPU_SIZE;
  . = ALIGN(16);
  _gpu_memory_end = .;
} >OMV_GPU_MEMORY
#endif

/* Open-AMP Shared Memory Region */
#if defined(OMV_OPENAMP_MEMORY)
.openamp_memory (NOLOAD) : ALIGN(32)
{
  _openamp_shm_region_start = .;
  . = . + OMV_OPENAMP_SIZE;
  _openamp_shm_region_end = .;
  . = OMV_ALIGN_DMA_SECTION(.openamp_memory);
} >OMV_OPENAMP_MEMORY
#endif

/* Memory for the second core */
#if defined(OMV_CORE1_MEMORY)
.core1_memory (NOLOAD) : ALIGN(32)
{
  _core1_memory_start = .;
  . = . + OMV_CORE1_SIZE;
  _core1_memory_end = .;
  . = OMV_ALIGN_DMA_SECTION(.core1_memory);
} >OMV_CORE1_MEMORY
#endif

/* lwIP memory pools */
#if defined(OMV_LWIP_MEMORY)
.lwip_memory (NOLOAD) : ALIGN(4)
{
  *(.lwip)
  *(.lwip*)
  . = ALIGN(4);
} >OMV_LWIP_MEMORY
#endif

/* Misc DMA buffers section */
.dma.memory0 (NOLOAD) : ALIGN(32)
{
  /* Image line buffer. */
  #if defined(OMV_LINE_BUF_SIZE)
  . = ALIGN(32);
  _line_buf = .;
  . = . + OMV_LINE_BUF_SIZE;
  #endif

  /* USB MSC bot data (2K) */
  #if defined(OMV_MSC_BUF_SIZE)
  . = ALIGN(32);
  _msc_buf  = .;
  . = . + OMV_MSC_BUF_SIZE;
  #endif

  #if defined(OMV_SB_SIZE) && !defined(OMV_SB_MEMORY)
  . = ALIGN(32);
  _sb_memory_start = .;
  . = . + OMV_SB_SIZE;
  _sb_memory_end = .;
  #endif

  /* FFS (internal) filesystem buffer */
  #if defined(OMV_FFS_BUF_SIZE)
  . = ALIGN(32);
  _micropy_hw_internal_flash_storage_ram_cache_start = .;
  . = . + OMV_FFS_BUF_SIZE;
  _micropy_hw_internal_flash_storage_ram_cache_end = .;
  #endif

 . = ALIGN(32);
  *(.dma_buffer)

 . = ALIGN(32);
 *(NonCacheable)

  . = OMV_ALIGN_DMA_SECTION(.dma.memory0);
#if defined(OMV_DMA_MEMORY)
} >OMV_DMA_MEMORY
#else
} >OMV_MAIN_MEMORY
#endif


#if defined(OMV_ROMFS_PART0_ORIGIN)
_micropy_hw_romfs_part0_start = OMV_ROMFS_PART0_ORIGIN;
_micropy_hw_romfs_part0_size = OMV_ROMFS_PART0_LENGTH;
#endif

#if defined(OMV_ROMFS_PART1_ORIGIN)
_micropy_hw_romfs_part1_start = OMV_ROMFS_PART1_ORIGIN;
_micropy_hw_romfs_part1_size = OMV_ROMFS_PART1_LENGTH;
#endif

.ARM.attributes 0 : { *(.ARM.attributes) }
