/* This file was ported to work on Alif Semiconductor Ensemble family of devices. */

/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

__STACK_SIZE    = 0x00002000;
__HEAP_SIZE     = 0x00004000;
__APP_HEAP_SIZE = 0x00004000;
__ROM_BASE      = 0x80200000;
__ROM_SIZE      = 0x00200000;

TGU_BLOCK_SIZE   = 16384;

/*
 * Note:
 * The empty regions(eg SRAM1) are ON by default, but may have been released
 * by some example applications. So make sure your application requests
 * power for these regions before adding contents.
 */

MEMORY
{
  ITCM  (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00040000
  DTCM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00100000
  SRAM0 (rwx) : ORIGIN = 0x02000000, LENGTH = 0x00400000
  SRAM1 (rwx) : ORIGIN = 0x08000000, LENGTH = 0x00280000
  MRAM  (rx)  : ORIGIN = __ROM_BASE, LENGTH = __ROM_SIZE
}

ENTRY(Reset_Handler)

SECTIONS
{
  .startup.at_mram : ALIGN(16)
  {
    KEEP(*(.vectors))

    *startup_*(.text .rodata*)
    *system_*(.text .rodata*)
    *system_utils*(.text .rodata*)
    *pm*(.text .rodata*)
    *tgu_*(.text .rodata*)
    *mpu_*(.text .rodata*)
    *tcm_partition*(.text .rodata*)

    *lv_*(.text .rodata*)

    *(startup_ro_data)
    . = ALIGN(16);
  } > MRAM

  .copy.table : ALIGN(4)
  {
    __copy_table_start__ = .;
    LONG ( LOADADDR(.data.at_dtcm) )
    LONG ( ADDR(.data.at_dtcm) )
    LONG ( SIZEOF(.data.at_dtcm)/4 )
    LONG ( LOADADDR(.code.at_itcm) )
    LONG ( ADDR(.code.at_itcm) )
    LONG ( SIZEOF(.code.at_itcm)/4 )
    __copy_table_end__ = .;
    . = ALIGN(16);
  } > MRAM

  .zero.table : ALIGN(4)
  {
    __zero_table_start__ = .;
    LONG (ADDR(.bss))
    LONG (SIZEOF(.bss)/4)
    LONG (ADDR(.bss.at_sram0))
    LONG (SIZEOF(.bss.at_sram0)/4)
    __zero_table_end__ = .;
    . = ALIGN(16);
  } > MRAM

  .code.at_itcm : ALIGN(8)
  {
    *(.text*)
    . = ALIGN(16);
  } > ITCM AT > MRAM

  .data.at_dtcm : ALIGN(8)
  {
    *(vtable)
    *(.data)
    *(.data*)
    *arm_common_tables*(.data* .rodata*)

    KEEP(*(.jcr*))

    . = ALIGN(8);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(8);
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
    . = ALIGN(16);
  } > DTCM AT > MRAM

  .bss.at_sram0 (NOLOAD) : ALIGN(8)
  {
    *(.bss.lcd_crop_and_interpolate_buf)  /* LCD crop and intrepolate image processing buffer. */
    *(.bss.lcd_frame_buf)                 /* LCD frame Buffer. */
    *(.bss.camera_frame_buf)              /* Camer Frame Buffer */
    *(.bss.camera_frame_bayer_to_rgb_buf) /* (Optional) Camera Frame Buffer for Bayer to RGB Conversion. */
  } > SRAM0

  .bss (NOLOAD) : ALIGN(8)
  {
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)

    . = ALIGN(8);
    __bss_end__ = .;
  } > DTCM

  .nonsecure0_region (NOLOAD) :
  {
    . = ALIGN(TGU_BLOCK_SIZE);
    __ns_region_0_start = .;
    * (eth_buf)
    * (usb_dma_buf)
    * (sd_dma_buf)
    . = ALIGN(TGU_BLOCK_SIZE);
    __ns_region_0_end = .;
  } > DTCM

  .__app_heap (NOLOAD) : ALIGN(8)
  {
     . = ALIGN(8);
     __RAM_segment_used_end__ = .;
     . = . + __APP_HEAP_SIZE;
     . = ALIGN(8);
   } >DTCM

  .heap (NOLOAD) : ALIGN(8)
  {
    __end__ = .;
    PROVIDE(end = .);
    . = . + __HEAP_SIZE;
    . = ALIGN(8);
    __HeapLimit = .;
  } > DTCM

  .stack (ORIGIN(DTCM) + LENGTH(DTCM) - __STACK_SIZE) (NOLOAD) : ALIGN(8)
  {
    __StackLimit = .;
    . = . + __STACK_SIZE;
    . = ALIGN(8);
    __StackTop = .;
  } > DTCM
  PROVIDE(__stack = __StackTop);

  .readonly.at_mram : ALIGN(8)
  {
    /* Use wildcards to mop up any read-only not directed to TCM */
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

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)

    KEEP(*(.eh_frame*))
    . = ALIGN(16);
  } > MRAM

  /* Check if data + heap + stack exceeds RAM limit */
  ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
