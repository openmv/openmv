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
#include "header.h"

/* Specify the memory areas */
MEMORY
{
  RAM (rwx): ORIGIN = OMV_DTCM_ORIGIN,       LENGTH = OMV_DTCM_LENGTH
  ROM (rx) : ORIGIN = OMV_FLASH_BOOT_ORIGIN, LENGTH = OMV_FLASH_BOOT_LENGTH
}

_ram_end    = ORIGIN(RAM) + LENGTH(RAM);
_stack_size = 16K;

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */

    /* Boot header at fixed offset */
    . = OMV_BOOT_HEADER_OFFSET;
    KEEP(*(.boot_header))

    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
    _exit = .;
  } >ROM

  .gnu.sgstubs :
  {
    . = ALIGN(4);
    _start_sg = .;
    *(.gnu*)
    . = ALIGN(4);
    _end_sg = .;
  } >ROM

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
  } >RAM AT> ROM

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
  } >RAM

  /* Make sure there is enough RAM the stack and FS cache */
  .stack :
  {
    . = ALIGN(8);
    _sstack  = .;
    . = . + _stack_size;
    . = ALIGN(8);
    _estack = .;
  } >RAM

  .ram_function : ALIGN(4)
  {
    _ram_function_start = .;
    *(.ram_function*)
    . = ALIGN(4);
    _ram_function_end = .;
  } > RAM AT> ROM

  _ram_function_flash = LOADADDR(.ram_function);

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
