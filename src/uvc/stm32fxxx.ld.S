/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Linker script for STM32 Devices.
 */

/* Entry Point */
ENTRY(Reset_Handler)

#include "omv_boardconfig.h"

/* Specify the memory areas */
MEMORY
{
  DTCM (xrw)        : ORIGIN = OMV_DTCM_ORIGIN,     LENGTH = OMV_DTCM_LENGTH
  #if defined(OMV_ITCM_ORIGIN)
  ITCM (xrw)        : ORIGIN = OMV_ITCM_ORIGIN,     LENGTH = OMV_ITCM_LENGTH
  #endif
  SRAM1 (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,    LENGTH = OMV_SRAM1_LENGTH
  #if defined(OMV_SRAM2_ORIGIN)
  SRAM2 (xrw)       : ORIGIN = OMV_SRAM2_ORIGIN,    LENGTH = OMV_SRAM2_LENGTH
  #endif
  #if defined(OMV_SRAM3_ORIGIN)
  SRAM3 (xrw)       : ORIGIN = OMV_SRAM3_ORIGIN,    LENGTH = OMV_SRAM3_LENGTH
  #endif
  #if defined(OMV_SRAM4_ORIGIN)
  SRAM4 (xrw)       : ORIGIN = OMV_SRAM4_ORIGIN,    LENGTH = OMV_SRAM4_LENGTH
  #endif
  #if defined(OMV_AXI_SRAM_ORIGIN)
  AXI_SRAM (xrw)    : ORIGIN = OMV_AXI_SRAM_ORIGIN, LENGTH = OMV_AXI_SRAM_LENGTH
  #endif
  #if defined(OMV_DRAM_ORIGIN)
  DRAM (xrw)        : ORIGIN = OMV_DRAM_ORIGIN,     LENGTH = OMV_DRAM_LENGTH
  #endif
  FLASH_TEXT (rx)   : ORIGIN = OMV_FLASH_TXT_ORIGIN, LENGTH = OMV_FLASH_TXT_LENGTH
}

_heap_size  = OMV_HEAP_SIZE;    /* required amount of heap */
_stack_size = OMV_STACK_SIZE;   /* minimum amount of stack */
_ram_end    = ORIGIN(OMV_MAIN_MEMORY) + LENGTH(OMV_MAIN_MEMORY);

#if defined(OMV_FFS_MEMORY)
#if !defined(OMV_FFS_MEMORY_OFFSET)
#define OMV_FFS_MEMORY_OFFSET           (0)
#endif
_ffs_cache          = ORIGIN(OMV_FFS_MEMORY) + OMV_FFS_MEMORY_OFFSET;
#endif

#if defined(OMV_JPEG_MEMORY)
#if !defined(OMV_JPEG_MEMORY_OFFSET)
#define OMV_JPEG_MEMORY_OFFSET          (0)
#endif
_jpeg_buf           = ORIGIN(OMV_JPEG_MEMORY) + OMV_JPEG_MEMORY_OFFSET;
#endif

#if defined(OMV_VOSPI_MEMORY)
#if !defined(OMV_VOSPI_MEMORY_OFFSET)
#define OMV_VOSPI_MEMORY_OFFSET         (0)
#endif
_vospi_buf          = ORIGIN(OMV_VOSPI_MEMORY) + OMV_VOSPI_MEMORY_OFFSET;
#endif

/* Define output sections */
SECTIONS
{
  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))// ISR table
    . = ALIGN(4);
    *(.text)            // .text sections (code)
    . = ALIGN(4);
    *(.text*)           // .text* sections (code)
    . = ALIGN(4);
    *(.rodata)          // .rodata sections (constants, strings, etc.)
    . = ALIGN(4);
    *(.rodata*)         // .rodata* sections (constants, strings, etc.)
    . = ALIGN(4);
    _etext = .;         // define a global symbols at end of code
  } >FLASH_TEXT

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Main framebuffer memory */
  .fb_memory (NOLOAD) :
  {
    . = ALIGN(4);
    _fb_base = .;
    . += OMV_FB_SIZE;

    _fb_end = .;
    . += OMV_FB_ALLOC_SIZE;

    . = ALIGN(4);
    _fballoc = .;
    . = ALIGN(4);
  } >OMV_FB_MEMORY

  #if defined(OMV_FB_OVERLAY_MEMORY)
  .fb_overlay_memory (NOLOAD) :
  {
    . = ALIGN(4);
    _fballoc_overlay_start = .;
    . = . + OMV_FB_OVERLAY_SIZE;
    _fballoc_overlay_end = .;
  } >OMV_FB_OVERLAY_MEMORY
  #endif

  /* Misc DMA buffers section */
  .dma_memory (NOLOAD) :
  {
    . = ALIGN(16);
    _line_buf = .;      // Image line buffer.
    . = . + OMV_LINE_BUF_SIZE;

    . = ALIGN(16);
    _msc_buf  = .;      // USB MSC bot data (2K)
    . = . + OMV_MSC_BUF_SIZE;

    . = ALIGN(16);
    _vfs_buf  = .;      // VFS sturct + FATFS file buffer  (around 624 bytes)
    . = . + OMV_VFS_BUF_SIZE;

    . = ALIGN(16);
    _fir_lepton_buf = .; // FIR Lepton Packet Double Buffer (328 bytes)
    . = . + OMV_FIR_LEPTON_BUF_SIZE;

    #if defined(OMV_FFS_BUF_SIZE)
    . = ALIGN(16);
    _ffs_cache = .;     // Flash filesystem cache
    . = . + OMV_FFS_BUF_SIZE;
    #endif

    #if !defined(OMV_JPEG_MEMORY)
    . = ALIGN(16);
    _jpeg_buf = .;      // IDE JPEG buffer
    . = . + OMV_JPEG_BUF_SIZE;
    #endif

   . = ALIGN(16);
    *(.dma_buffer)
  } >OMV_DMA_MEMORY

  /* Domain 1 DMA buffers. */
  #if defined(OMV_DMA_MEMORY_D1)
  .d1_dma_memory (NOLOAD) :
  {
   . = ALIGN(16);
    *(.d1_dma_buffer)
  } >OMV_DMA_MEMORY_D1
  #endif

  /* Domain 2 DMA buffers. */
  #if defined(OMV_DMA_MEMORY_D2)
  .d2_dma_memory (NOLOAD) :
  {
   . = ALIGN(16);
    *(.d2_dma_buffer)
  } >OMV_DMA_MEMORY_D2
  #endif

  /* Domain 3 DMA buffers. */
  #if defined(OMV_DMA_MEMORY_D3)
  .d3_dma_memory (NOLOAD) :
  {
   . = ALIGN(16);
    *(.d3_dma_buffer)
  } >OMV_DMA_MEMORY_D3
  #endif

  /* Initialized data sections */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;         // Create a global symbol at data start
    _ram_start = .;
    *(.data)            // .data sections

    . = ALIGN(4);
    *(.data*)           // .data* sections

    . = ALIGN(4);
    _edata = .;         // define a global symbol at data end
  } >OMV_MAIN_MEMORY AT> FLASH_TEXT

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;          // Used by the startup to initialize the .bss secion
    . = ALIGN(4);
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;          // define a global symbol at bss end
  } >OMV_MAIN_MEMORY

  ._heap (NOLOAD) :
  {
    . = ALIGN(4);
    _heap_start = .;
    . = . + _heap_size;

    . = ALIGN(4);
    _heap_end  = .;

  } >OMV_MAIN_MEMORY

  /* Make sure there is enough ram for the stack */
  ._stack (NOLOAD) :
  {
    . = ALIGN(8);
    _sstack  = .;
    . = . + _stack_size;

    . = ALIGN(8);
    _estack  = .;
  } >OMV_STACK_MEMORY

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
