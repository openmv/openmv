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
  SRAM (xrw)      : ORIGIN = OMV_SRAM_ORIGIN, LENGTH = OMV_SRAM_LENGTH
  FLASH_TEXT (rx) : ORIGIN = OMV_TEXT_ORIGIN, LENGTH = OMV_TEXT_LENGTH
}

_ram_start  = ORIGIN(SRAM);
_ram_end    = ORIGIN(SRAM) + LENGTH(SRAM);

_fs_start   = ORIGIN(FLASH_TEXT) + LENGTH(FLASH_TEXT);
_fs_end     = ORIGIN(FLASH_TEXT) + LENGTH(FLASH_TEXT) + OMV_FFS_LENGTH;

/* define output sections */
SECTIONS
{
    /* The program code and other data goes into FLASH */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) // ISR table
        . = ALIGN(4);
        *(.text)             // .text sections (code)
        . = ALIGN(4);
        *(.text*)            // .text* sections (code)
        . = ALIGN(4);
        *(.rodata)           // .rodata sections (constants, strings, etc.)
        . = ALIGN(4);
        *(.rodata*)          // .rodata sections (constants, strings, etc.)
        . = ALIGN(4);
        _etext = .;
        _unused_flash_start = .;
    } >FLASH_TEXT

    /* The address used as the source for copying the initialized data section. */
    _sidata = LOADADDR(.data);

    /* Initialized data sections */
    .data :
    {
        . = ALIGN(4);       // Used by the startup to initialize the data section
        _sdata = .;
        *(.data)            // .data sections
        . = ALIGN(4);
        *(.data*)           // .data sections
        . = ALIGN(4);
        _edata = .;
    } >SRAM AT>FLASH_TEXT

    /* Uninitialized data section */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;          // Used by the startup to initialize the .bss section
        *(.bss)
        . = ALIGN(4);
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;          // define a global symbol at bss end
    } >SRAM

    #include "common.ld.S"
}

_unused_flash_len = (ORIGIN(FLASH_TEXT) + LENGTH(FLASH_TEXT)) - _unused_flash_start;
