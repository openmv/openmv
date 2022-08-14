/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Linker script for STM32F4xx Devices.
 */

/* Entry Point */
ENTRY(Reset_Handler)

#include "omv_boardconfig.h"
#include "omv_bootconfig.h"

/* Specify the memory areas */
MEMORY
{
  DTCM (xrw)      : ORIGIN = OMV_DTCM_ORIGIN,       LENGTH = OMV_DTCM_LENGTH
  RAM (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,      LENGTH = OMV_SRAM1_LENGTH
  FLASH_TEXT (rx) : ORIGIN = OMV_BOOT_FLASH_ORIGIN, LENGTH = OMV_BOOT_FLASH_LENGTH
}

_estack     = ORIGIN(DTCM) + LENGTH(DTCM);
_ram_end    = ORIGIN(DTCM) + LENGTH(DTCM);
_stack_size = (4K);       /* minimum amount of stack */

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
    _exit = .;
  } >FLASH_TEXT

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    _ram_start = .;
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCM AT> FLASH_TEXT

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;          // Used by the startup to initialize the .bss secion
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;          // define a global symbol at bss end
  } >DTCM

  /* Make sure there is enough RAM the stack and FS cache */
  .stack :
  {
    . = ALIGN(4);
    . = . + _stack_size;
    . = ALIGN(4);
  } >DTCM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
