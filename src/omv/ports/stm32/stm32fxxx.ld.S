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
  DTCM (xrw)        : ORIGIN = OMV_DTCM_ORIGIN,      LENGTH = OMV_DTCM_LENGTH
  #if defined(OMV_ITCM_ORIGIN)
  ITCM (xrw)        : ORIGIN = OMV_ITCM_ORIGIN,      LENGTH = OMV_ITCM_LENGTH
  #endif
  SRAM1 (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,     LENGTH = OMV_SRAM1_LENGTH
  #if defined(OMV_SRAM2_ORIGIN)
  SRAM2 (xrw)       : ORIGIN = OMV_SRAM2_ORIGIN,     LENGTH = OMV_SRAM2_LENGTH
  #endif
  #if defined(OMV_SRAM3_ORIGIN)
  SRAM3 (xrw)       : ORIGIN = OMV_SRAM3_ORIGIN,     LENGTH = OMV_SRAM3_LENGTH
  #endif
  #if defined(OMV_SRAM4_ORIGIN)
  SRAM4 (xrw)       : ORIGIN = OMV_SRAM4_ORIGIN,     LENGTH = OMV_SRAM4_LENGTH
  #endif
  #if defined(OMV_AXI_SRAM_ORIGIN)
  AXI_SRAM (xrw)    : ORIGIN = OMV_AXI_SRAM_ORIGIN,  LENGTH = OMV_AXI_SRAM_LENGTH
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
  #if defined(OMV_CM4_RAM_ORIGIN)
  CM4_SRAM (xrw)    : ORIGIN = OMV_CM4_RAM_ORIGIN,     LENGTH = OMV_CM4_RAM_LENGTH
  #endif
}

_ram_start  = ORIGIN(OMV_MAIN_MEMORY);
_ram_end    = ORIGIN(OMV_MAIN_MEMORY) + LENGTH(OMV_MAIN_MEMORY);

#if defined(OMV_CM4_RAM_ORIGIN)
_cm4_ram_start = ORIGIN(CM4_SRAM);
#endif

// Location of filesystem flash storage
_micropy_hw_internal_flash_storage_start = ORIGIN(FLASH_FFS);
_micropy_hw_internal_flash_storage_end = ORIGIN(FLASH_FFS) + LENGTH(FLASH_FFS);

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

  #include "common.ld.S"

  .mp_etext :
  {
    _etext = .;         // This is for stm modmachine, and not used for anything else.
  } > FLASH_TEXT
}
