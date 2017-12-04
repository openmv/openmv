/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Linker script for STM32F4xx Devices.
 *
 */

/* Entry Point */
ENTRY(Reset_Handler)

#include "omv_boardconfig.h"

/* Specify the memory areas */
MEMORY
{
  CCM (xrw)         : ORIGIN = OMV_CCM_ORIGIN,      LENGTH = OMV_CCM_LENGTH
  SRAM1 (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,    LENGTH = OMV_SRAM1_LENGTH
  SRAM2 (xrw)       : ORIGIN = OMV_SRAM2_ORIGIN,    LENGTH = OMV_SRAM2_LENGTH
  FLASH_TEXT (rx)   : ORIGIN = OMV_TEXT_ORIGIN,     LENGTH = OMV_TEXT_LENGTH
}

_fb_base    = ORIGIN(OMV_FB_MEMORY);
_fballoc    = ORIGIN(OMV_FB_MEMORY) + OMV_FB_SIZE + OMV_FB_ALLOC_SIZE;

_estack     = ORIGIN(OMV_MAIN_MEMORY) + LENGTH(OMV_MAIN_MEMORY);
_ram_end    = ORIGIN(OMV_MAIN_MEMORY) + LENGTH(OMV_MAIN_MEMORY);

_heap_size  = OMV_HEAP_SIZE;    /* required amount of heap */
_stack_size = OMV_STACK_SIZE;   /* minimum amount of stack */

/* Define output sections */
SECTIONS
{
  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    . = ALIGN(4);
    *(.text*)          /* .text* sections (code) */
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    . = ALIGN(4);
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH_TEXT

  /* used by the startup to initialize data */
  _sidata = .;

  /* Non-cacheable DMA buffers */
  .dma_buffers (NOLOAD) :
  {
    . = ALIGN(4);
    _line_buf = .; // Image line buffer.
    . = . + OMV_LINE_BUF_SIZE;

    . = ALIGN(4);
    _msc_buf  = .; // USB MSC bot data (2K)
    . = . + OMV_MSC_BUF_SIZE;

    . = ALIGN(4);
    _vfs_buf  = .; // VFS sturct + FATFS file buffer  (around 624 bytes)
    . = . + OMV_VFS_BUF_SIZE;

    . = ALIGN(4);
    _ffs_cache = .; // Flash filesystem cache
    . = . + OMV_FFS_BUF_SIZE;

    . = ALIGN(4);
    _jpeg_buf = .; // IDE JPEG buffer
    . = . + OMV_JPEG_BUF_SIZE;

    . = ALIGN(4);
    *(.dma_buffer)

  } >OMV_DMA_MEMORY

  /* Initialized data sections goes into ram, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    _ram_start = .;
    *(.data)           /* .data sections */
    . = ALIGN(4);
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >OMV_MAIN_MEMORY

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    . = ALIGN(4);
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
    _bss_end = _ebss; /* for gccollect */
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
    . = ALIGN(4);
    . = . + _stack_size;
    . = ALIGN(4);
  } >OMV_MAIN_MEMORY

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
