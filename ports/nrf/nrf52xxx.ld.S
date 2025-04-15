/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2020-2024 OpenMV, LLC.
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
