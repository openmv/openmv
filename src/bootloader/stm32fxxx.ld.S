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
  CCM (xrw)       : ORIGIN = OMV_CCM_ORIGIN,    LENGTH = OMV_CCM_LENGTH
  RAM (xrw)       : ORIGIN = OMV_SRAM1_ORIGIN,  LENGTH = OMV_SRAM1_LENGTH
  FLASH_TEXT (rx) : ORIGIN = OMV_BOOT_ORIGIN,   LENGTH = OMV_BOOT_LENGTH
}

_estack     = ORIGIN(CCM) + LENGTH(CCM);
_ram_end    = ORIGIN(CCM) + LENGTH(CCM);
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
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    _ram_start = .;
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >CCM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
    _bss_end = _ebss; /* for gccollect */
  } >CCM

  /* Make sure there is enough RAM the stack and FS cache */
  .stack :
  {
    . = ALIGN(4);
    . = . + _stack_size;
    . = ALIGN(4);
  } >CCM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
