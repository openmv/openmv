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
    SRAM (xrw)      : ORIGIN = OMV_SRAM_ORIGIN,    LENGTH = OMV_SRAM_LENGTH
    FLASH_TEXT (rx) : ORIGIN = OMV_TEXT_ORIGIN,    LENGTH = OMV_TEXT_LENGTH
}

_heap_size  = OMV_HEAP_SIZE;    /* required amount of heap */
_stack_size = OMV_STACK_SIZE;   /* minimum amount of stack */
_fs_start   = OMV_TEXT_ORIGIN + OMV_TEXT_LENGTH;
_fs_end     = OMV_TEXT_ORIGIN + OMV_TEXT_LENGTH + 64K;

/* define output sections */
SECTIONS
{
    /* The program code and other data goes into FLASH */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Startup code */
        *(.text)             /* .text sections (code) */
        *(.text*)            /* .text* sections (code) */
        *(.rodata)           /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)          /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
        _etext = .;          /* define a global symbol at end of code */
    } >FLASH_TEXT

    /* used by the startup to initialize data */
    _sidata = LOADADDR(.data);

    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;         /* create a global symbol at data start; used by startup code to initialise the .data section in RAM */
        _ram_start = .;     /* create a global symbol at ram start for garbage collector */
        *(.data)            /* .data sections */
        *(.data*)           /* .data* sections */

        . = ALIGN(4);
        _edata = .;         /* define a global symbol at data end; used by startup code to initialise the .data section in RAM */
    } >SRAM AT>FLASH_TEXT

    /* Uninitialized data section */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* define a global symbol at bss start; used by startup code */
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end; used by startup code and GC */
    } >SRAM

    /* Reserve heap memory */
    ._heap (NOLOAD) :
    {
      . = ALIGN(4);
      _heap_start = .;
      . = . + _heap_size;

      . = ALIGN(4);
      _heap_end  = .;

    } >OMV_MAIN_MEMORY

    /* Reserve stack memory */
    ._stack (NOLOAD) :
    {
      . = ALIGN(4);
      _sstack  = .;
      . = . + _stack_size;

      . = ALIGN(4);
      _estack  = .;
      _ram_end = .;
    } >OMV_STACK_MEMORY

    /* Main framebuffer memory */
    .fb_memory (NOLOAD) :
    {
      . = ALIGN(4);
      _fb_base = .;
      . = . + OMV_FB_SIZE + OMV_FB_ALLOC_SIZE;

      . = ALIGN(4);
      _fballoc = .;
      . = ALIGN(4);

      #if !defined(OMV_JPEG_MEMORY)
      _jpeg_buf = .;      // IDE JPEG buffer
      . = . + OMV_JPEG_BUF_SIZE;
      . = ALIGN(4);
      #endif
    } >OMV_FB_MEMORY

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
