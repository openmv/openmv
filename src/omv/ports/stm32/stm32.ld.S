/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Linker script for STM32 Devices.
 */

/* Entry Point */
ENTRY(Reset_Handler)

#include "omv_boardconfig.h"

/* Specify the memory areas */
MEMORY
{
  DTCM (xrw)        : ORIGIN = OMV_DTCM_ORIGIN,      LENGTH = OMV_DTCM_LENGTH
  #if defined(OMV_ITCM_ORIGIN)
  ITCM (xrw)        : ORIGIN = OMV_ITCM_ORIGIN,      LENGTH = OMV_ITCM_LENGTH
  #endif
  #if defined(OMV_SRAM0_ORIGIN)
  SRAM0 (xrw)       : ORIGIN = OMV_SRAM0_ORIGIN,     LENGTH = OMV_SRAM0_LENGTH
  #endif
  #if defined(OMV_SRAM1_ORIGIN)
  SRAM1 (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,     LENGTH = OMV_SRAM1_LENGTH
  #endif
  #if defined(OMV_SRAM2_ORIGIN)
  SRAM2 (xrw)       : ORIGIN = OMV_SRAM2_ORIGIN,     LENGTH = OMV_SRAM2_LENGTH
  #endif
  #if defined(OMV_SRAM3_ORIGIN)
  SRAM3 (xrw)       : ORIGIN = OMV_SRAM3_ORIGIN,     LENGTH = OMV_SRAM3_LENGTH
  #endif
  #if defined(OMV_SRAM4_ORIGIN)
  SRAM4 (xrw)       : ORIGIN = OMV_SRAM4_ORIGIN,     LENGTH = OMV_SRAM4_LENGTH
  #endif
  #if defined(OMV_SRAM5_ORIGIN)
  SRAM5 (xrw)       : ORIGIN = OMV_SRAM5_ORIGIN,     LENGTH = OMV_SRAM5_LENGTH
  #endif
  #if defined(OMV_SRAM6_ORIGIN)
  SRAM6 (xrw)       : ORIGIN = OMV_SRAM6_ORIGIN,     LENGTH = OMV_SRAM6_LENGTH
  #endif
  #if defined(OMV_SRAM7_ORIGIN)
  SRAM7 (xrw)       : ORIGIN = OMV_SRAM7_ORIGIN,     LENGTH = OMV_SRAM7_LENGTH
  #endif
  #if defined(OMV_SRAM8_ORIGIN)
  SRAM8 (xrw)       : ORIGIN = OMV_SRAM8_ORIGIN,     LENGTH = OMV_SRAM8_LENGTH
  #endif
  #if defined(OMV_DRAM_ORIGIN)
  DRAM (xrw)        : ORIGIN = OMV_DRAM_ORIGIN,      LENGTH = OMV_DRAM_LENGTH
  #endif
  #if defined(OMV_FLASH_FFS_ORIGIN)
  FLASH_FFS (rx)    : ORIGIN = OMV_FLASH_FFS_ORIGIN, LENGTH = OMV_FLASH_FFS_LENGTH
  #endif
  FLASH_TEXT (rx)   : ORIGIN = OMV_FLASH_TXT_ORIGIN, LENGTH = OMV_FLASH_TXT_LENGTH
  #if defined(OMV_FLASH_EXT_ORIGIN)
  FLASH_EXT (rx)    : ORIGIN = OMV_FLASH_EXT_ORIGIN, LENGTH = OMV_FLASH_EXT_LENGTH
  #endif
}

_ram_start  = ORIGIN(OMV_MAIN_MEMORY);
_ram_end    = ORIGIN(OMV_MAIN_MEMORY) + LENGTH(OMV_MAIN_MEMORY);

#if defined(OMV_FLASH_FFS_ORIGIN)
// Location of filesystem flash storage
_micropy_hw_internal_flash_storage_start = ORIGIN(FLASH_FFS);
_micropy_hw_internal_flash_storage_end = ORIGIN(FLASH_FFS) + LENGTH(FLASH_FFS);
#endif

#if defined(OMV_FFS_MEMORY)
#if !defined(OMV_FFS_MEMORY_OFFSET)
#define OMV_FFS_MEMORY_OFFSET           (0)
#endif
// Location of filesystem RAM cache
_micropy_hw_internal_flash_storage_ram_cache_start = ORIGIN(OMV_FFS_MEMORY) + OMV_FFS_MEMORY_OFFSET;
_micropy_hw_internal_flash_storage_ram_cache_end = LENGTH(OMV_FFS_MEMORY);
#endif

/* Define output sections */
SECTIONS
{
  /* The program code and other data goes into FLASH */
  .text : ALIGN(4)
  {
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
  } >FLASH_TEXT

  /* The address used as the source for copying the initialized data section. */
  _sidata = LOADADDR(.data);

  /* Initialized data sections */
  .data : ALIGN(4)
  {
    _sdata = .;         // Used by the startup to initialize the data section
    *(.data)            // .data sections
    . = ALIGN(4);
    *(.data*)           // .data* sections
    . = ALIGN(4);
    _edata = .;         // Define a global symbol at data end
  } >OMV_MAIN_MEMORY AT> FLASH_TEXT

  /* Uninitialized data section */
  .bss (NOLOAD) : ALIGN(4)
  {
    _sbss = .;          // Used by the startup to initialize the .bss section
    . = ALIGN(4);
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;          // Define a global symbol at bss end
  } >OMV_MAIN_MEMORY

  /* CM4 boot memory */
  #if defined(OMV_CM4_BOOT_MEMORY)
  .cm4_boot_memory (NOLOAD) : ALIGN(32)
  {
    _cm4_ram_start = .;
    . = . + OMV_CM4_BOOT_SIZE;
    . = ALIGN(32);
    _cm4_ram_end = .;
  } >OMV_CM4_BOOT_MEMORY
  #endif

  #include "common.ld.S"

  .mp_etext :
  {
    _etext = .;         // This is for stm modmachine, and not used for anything else.
  } > FLASH_TEXT
}
