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

#ifdef OPENMV1
#define HEAP_MEMORY     CCM
#define FLASH_ORIGIN    0x08000000
#define FLASH_LENGTH    512K

#define TEXT_ORIGIN     0x08000000
#define TEXT_LENGTH     16K

#define RAM_ORIGIN      0x20000000
#define RAM_LENGTH      128K

#define CCM_ORIGIN      0x10000000
#define CCM_LENGTH      64K

#else
#define HEAP_MEMORY     CCM
#define FLASH_ORIGIN    0x08000000
#define FLASH_LENGTH    1024K

#define TEXT_ORIGIN     0x08000000
#define TEXT_LENGTH     16K

#define RAM_ORIGIN      0x20000000
#define RAM_LENGTH      192K

#define CCM_ORIGIN      0x10000000
#define CCM_LENGTH      64K
#endif //OPENMV1

/* Specify the memory areas */
MEMORY
{
  FLASH_TEXT (rx) : ORIGIN = TEXT_ORIGIN,   LENGTH = TEXT_LENGTH
  RAM (xrw)       : ORIGIN = RAM_ORIGIN,    LENGTH = RAM_LENGTH
  CCM (xrw)       : ORIGIN = CCM_ORIGIN,    LENGTH = CCM_LENGTH
}

_estack     = 0x10010000;   /* Stack is allocated on CCM block */
_ram_end    = 0x10010000;   /* 64KB CCM */
_stack_size = 0xC00;        /* minimum amount of stack */

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
  ._user_heap_stack :
  {
    . = ALIGN(4);
    . = . + _stack_size;
    . = ALIGN(4);
  } >CCM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
