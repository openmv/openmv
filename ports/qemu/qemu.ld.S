/* This file is part of the MicroPython project, http://micropython.org/
 * The MIT License (MIT)
 * Copyright (c) 2018 Damien P. George
 */

#include "omv_boardconfig.h"

MEMORY
{
  #if defined(OMV_ITCM_ORIGIN)
  ITCM (xrw)        : ORIGIN = OMV_ITCM_ORIGIN,      LENGTH = OMV_ITCM_LENGTH
  #endif
  #if defined(OMV_SRAM1_ORIGIN)
  SRAM1 (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,     LENGTH = OMV_SRAM1_LENGTH
  #endif
  #if defined(OMV_SRAM2_ORIGIN)
  SRAM2 (xrw)       : ORIGIN = OMV_SRAM2_ORIGIN,     LENGTH = OMV_SRAM2_LENGTH
  #endif
  FLASH_ISR (rx)    : ORIGIN = OMV_FLASH_ISR_ORIGIN, LENGTH = OMV_FLASH_ISR_LENGTH
  FLASH_TEXT (rx)   : ORIGIN = OMV_FLASH_TXT_ORIGIN, LENGTH = OMV_FLASH_TXT_LENGTH
}

/* Define output sections */
SECTIONS
{
  .isr_vector : ALIGN(4)
  {
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH_ISR

  /* The program code and other data goes into FLASH */
  .text : ALIGN(4)
  {
    *(.text*)           // .text* sections (code)
    . = ALIGN(4);
    *(.rodata*)         // .rodata* sections (constants, strings, etc.)
    . = ALIGN(4);
    *(.ARM.exidx*)
    . = ALIGN(4);
    _etext = .;
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
    __bss_start__ = .;
    . = ALIGN(4);
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;          // Define a global symbol at bss end
    __bss_end__ = .;
  } >OMV_MAIN_MEMORY

  #include "common.ld.S"
}
