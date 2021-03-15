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
  RAM (xrw)    : ORIGIN = OMV_CM4_RAM_ORIGIN,     LENGTH = OMV_CM4_RAM_LENGTH
  FLASH (rx)   : ORIGIN = OMV_CM4_FLASH_ORIGIN,   LENGTH = OMV_CM4_FLASH_LENGTH
}

_heap_size  = (1 * 1024);   /* heap size  */
_stack_size = (4 * 1024);   /* stack size */

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
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

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
  } >RAM AT> FLASH

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
  } >RAM

  ._heap (NOLOAD) :
  {
    . = ALIGN(4);
    _heap_start = .;
    . = . + _heap_size;

    . = ALIGN(4);
    _heap_end  = .;

  } >RAM

  /* Make sure there is enough ram for the stack */
  ._stack (NOLOAD) :
  {
    . = ALIGN(8);
    _sstack  = .;
    . = . + _stack_size;

    . = ALIGN(8);
    _estack  = .;
  } >RAM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
