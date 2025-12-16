/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// Entry Point
ENTRY(Reset_Handler)

#include "omv_boardconfig.h"

MEMORY
{
    FLASH_TEXT  (rx)    : ORIGIN = OMV_FLASH_BOOT_ORIGIN, LENGTH = OMV_FLASH_BOOT_LENGTH
    ITCM        (rx)    : ORIGIN = OMV_ITCM_ORIGIN,     LENGTH = OMV_ITCM_LENGTH
    DTCM        (rw)    : ORIGIN = OMV_DTCM_ORIGIN,     LENGTH = OMV_DTCM_LENGTH
    #if defined(OMV_SRAM0_ORIGIN)
    SRAM0       (rw)    : ORIGIN = OMV_SRAM0_ORIGIN,    LENGTH = OMV_SRAM0_LENGTH
    #endif
    #if defined(OMV_SRAM1_ORIGIN)
    SRAM1       (rw)    : ORIGIN = OMV_SRAM1_ORIGIN,    LENGTH = OMV_SRAM1_LENGTH
    #endif
    #if defined(OMV_SRAM6_A_ORIGIN)
    SRAM6_A     (rw)    : ORIGIN = OMV_SRAM6_A_ORIGIN,  LENGTH = OMV_SRAM6_A_LENGTH
    #endif
    #if defined(OMV_SRAM6_B_ORIGIN)
    SRAM6_B     (rw)    : ORIGIN = OMV_SRAM6_B_ORIGIN,  LENGTH = OMV_SRAM6_B_LENGTH
    #endif
    #if defined(OMV_SRAM7_ORIGIN)
    SRAM7       (rw)    : ORIGIN = OMV_SRAM7_ORIGIN,    LENGTH = OMV_SRAM7_LENGTH
    #endif
    #if defined(OMV_SRAM8_ORIGIN)
    SRAM8       (rw)    : ORIGIN = OMV_SRAM8_ORIGIN,    LENGTH = OMV_SRAM8_LENGTH
    #endif
    #if defined(OMV_SRAM9_A_ORIGIN)
    SRAM9_A     (rw)    : ORIGIN = OMV_SRAM9_A_ORIGIN,  LENGTH = OMV_SRAM9_A_LENGTH
    #endif
    #if defined(OMV_SRAM9_B_ORIGIN)
    SRAM9_B     (rw)    : ORIGIN = OMV_SRAM9_B_ORIGIN,  LENGTH = OMV_SRAM9_B_LENGTH
    #endif
}

SECTIONS
{
  .text : ALIGN(16)
  {
    KEEP(*(.vectors))
    . = ALIGN(4);
    *(.text*)
    . = ALIGN(4);
    *(.rodata*)
    . = ALIGN(16);
  } > FLASH_TEXT

  .copy.table : ALIGN(4)
  {
    __copy_table_start__ = .;
    LONG ( LOADADDR(.data) )
    LONG ( ADDR(.data) )
    LONG ( SIZEOF(.data)/4 )
    __copy_table_end__ = .;
    . = ALIGN(16);
  } > FLASH_TEXT

  .zero.table : ALIGN(4)
  {
    __zero_table_start__ = .;
    LONG (ADDR(.bss))
    LONG (SIZEOF(.bss)/4)
    LONG (ADDR(.bss.sram))
    LONG (SIZEOF(.bss.sram)/4)
    __zero_table_end__ = .;
    . = ALIGN(16);
  } > FLASH_TEXT

  .data : ALIGN(8)
  {
    *(.data)
    . = ALIGN(8);
    *(.data.*)
    . = ALIGN(8);
    . = ALIGN(16);
  } > OMV_MAIN_MEMORY AT > FLASH_TEXT

  /* Peripherals in expansion master 0 (such as USB, Ethernet, SD/MMC)
     are by default configured as non-secure, so they don't have access
     to DTCMs. This can be fixed in the ToC by allowing access to DTCMs
     to all bus masters, for now these peripherals should place buffers
     in regular SRAM */
  .bss.sram (NOLOAD) : ALIGN(16)
  {
    *(.bss.sram*)
    . = ALIGN(16);
  } > SRAM1

  .bss : ALIGN(4)
  {
    __bss_start__ = .;
    *(.bss)
    . = ALIGN(4);
    *(.bss.*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > OMV_MAIN_MEMORY

  .init_fini_arrays : ALIGN(16)
  {
    KEEP(*(.init))
    KEEP(*(.fini))

    . = ALIGN(4);
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.eh_frame*))
    . = ALIGN(16);
  } > FLASH_TEXT

  #include "common.ld.S"
}
