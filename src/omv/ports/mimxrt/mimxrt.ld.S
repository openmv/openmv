/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Linker script for MIMXRT Devices.
 */

/* Entry Point */
ENTRY(Reset_Handler)

#include "omv_boardconfig.h"

/* Specify the memory areas */
MEMORY
{
  FLASH         (rx):    ORIGIN = OMV_FLASH_ORIGIN,                 LENGTH = OMV_FLASH_LENGTH
  FLASH_FCB     (RX):    ORIGIN = OMV_FLASH_APP_ORIGIN + 0x0000,    LENGTH = 0x00001000
  FLASH_IVT     (RX):    ORIGIN = OMV_FLASH_APP_ORIGIN + 0x1000,    LENGTH = 0x00001000
  FLASH_ISR     (RX):    ORIGIN = OMV_FLASH_APP_ORIGIN + 0x2000,    LENGTH = 0x00001000
  FLASH_TEXT    (RX):    ORIGIN = OMV_FLASH_TXT_ORIGIN,             LENGTH = OMV_FLASH_TXT_LENGTH
  FLASH_VFS     (RX):    ORIGIN = OMV_FLASH_FFS_ORIGIN,             LENGTH = OMV_FLASH_FFS_LENGTH
  #if defined(OMV_ITCM1_ORIGIN)
  ITCM1         (RWX):   ORIGIN = OMV_ITCM1_ORIGIN,                 LENGTH = OMV_ITCM1_LENGTH
  #endif
  #if defined(OMV_ITCM2_ORIGIN)
  ITCM2         (RWX):   ORIGIN = OMV_ITCM2_ORIGIN,                 LENGTH = OMV_ITCM2_LENGTH
  #endif
  #if defined(OMV_DTCM_ORIGIN)
  DTCM          (RWX):   ORIGIN = OMV_DTCM_ORIGIN,                  LENGTH = OMV_DTCM_LENGTH
  #endif
  #if defined(OMV_OCRM1_ORIGIN)
  OCRM1         (RWX):   ORIGIN = OMV_OCRM1_ORIGIN,                 LENGTH = OMV_OCRM1_LENGTH
  #endif
  #if defined(OMV_OCRM2_ORIGIN)
  OCRM2         (RWX):   ORIGIN = OMV_OCRM2_ORIGIN,                 LENGTH = OMV_OCRM2_LENGTH
  #endif
  #if defined(OMV_DRAM_ORIGIN)
  DRAM          (RWX):   ORIGIN = OMV_DRAM_ORIGIN,                  LENGTH = OMV_DRAM_LENGTH
  #endif
}

_start = main;
__flash_start = ORIGIN(FLASH);
#if defined(OMV_DRAM_ORIGIN)
__sdram_start = ORIGIN(DRAM);
#endif
__vfs_start = ORIGIN(FLASH_VFS);
__vfs_end = ORIGIN(FLASH_VFS) + LENGTH(FLASH_VFS);  

// FlexRAM configuration addresses and settings
__iomux_gpr14_adr = 0x400AC038;
__iomux_gpr16_adr = 0x400AC040;
__iomux_gpr17_adr = 0x400AC044;
__iomux_gpr17_value = OMV_FLEXRAM_CONFIG;

/* Define output sections */
SECTIONS
{
  .fcb :
  {
    . = ALIGN(4);
    __FLASH_BASE = .;
    KEEP(* (.boot_hdr.conf))     /* flash config section */
    . = ALIGN(4);
  } > FLASH_FCB

  .ivt :
  {
    . = ALIGN(4);
    KEEP(* (.boot_hdr.ivt))           /* ivt section */
    KEEP(* (.boot_hdr.boot_data))     /* boot section */
    KEEP(* (.boot_hdr.dcd_data))      /* dcd section */
    . = ALIGN(4);
  } > FLASH_IVT

  /* The startup code goes first into internal RAM */
  .isr :
  {
    __VECTOR_TABLE = .;
    __Vectors = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector))     /* Startup code */
    . = ALIGN(4);
  } > FLASH_ISR

  /* The program code and other data goes into internal RAM */
  .text :
  {
    . = ALIGN(4);
    *(EXCLUDE_FILE(*fsl_flexspi.o *gc.o *vm.o *parse*.o *runtime*.o *map.o *mpirq.o ) .text*)
    *(.rodata)
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
  } > FLASH_TEXT

  /* The address used as the source for copying the initialized data section. */
  __etext = LOADADDR(.data);

  /* Initialized data sections */
  .data : ALIGN(4)
  {
    _ram_start = .;
    __data_start__ = .;      // Used by the startup to initialize the data section
    *(m_usb_dma_init_data)
    . = ALIGN(4);
    *(.data)                 // .data sections */
    . = ALIGN(4);
    *(.data*)                // .data* sections */
    KEEP(*(.jcr*))
    . = ALIGN(4);
    __data_end__ = .;        // Define a global symbol at data end
  } > OMV_MAIN_MEMORY AT> FLASH_TEXT

  /* Uninitialized data section */
  .bss (NOLOAD) : ALIGN(4)
  {
    __bss_start__ = .;       // Used by the startup to initialize the .bss section
    *(m_usb_dma_noninit_data)
    . = ALIGN(4);
    *(.bss)
    . = ALIGN(4);
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = .;
  } > OMV_MAIN_MEMORY

  .ram_function : ALIGN(4)
  {
    __ram_function_start__ = .;
    *(.ram_functions*)
    *(.text*)
    . = ALIGN(4);
    __ram_function_end__ = .;
  } > OMV_RAMFUNC_MEMORY AT> FLASH_TEXT

  #include "common.ld.S"
}
