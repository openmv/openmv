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

#if defined(OPENMV1)
    #define FLASH_ORIGIN    0x08000000
    #define FLASH_LENGTH    512K
    
    #define TEXT_ORIGIN     0x08010000
    #define TEXT_LENGTH     448K
    
    #define SRAM1_ORIGIN    0x20000000
    #define SRAM1_LENGTH    128K
    
    #define CCM_ORIGIN      0x10000000
    #define CCM_LENGTH      64K
#elif defined(OPENMV2)
    #define FB_MEMORY       SRAM1   // Framebuffer, fb_alloc
    #define MAIN_MEMORY     CCM     // data, bss, stack and heap
    #define DMA_MEMORY      SRAM2   // Misc DMA buffers
    
    // Located in main memory region
    #define STACK_SIZE      (3K)
    #define HEAP_SIZE       (54K)

    // Located in FB memory 
    #define FB_SIZE         (151K)  // FB memory: header + QVGA/GS image

    // fb alloc memory is dynamic = FB_ALLOC_SIZE + FB_SIZE - (w*h*bpp)
    // Note this overwrites the 2KBs line buf when not reading a frame.
    #define FB_ALLOC_SIZE   (14K)   // minimum fb alloc size

    // Located in DMA memory 
    #define LINE_BUF_SIZE   (2K)    // Image line buffer round(320 * 2BPP * 2 buffers).
    #define MSC_BUF_SIZE    (2K)    // USB MSC bot data
    #define VFS_BUF_SIZE    (1K)    // VFS sturct + FATFS file buffer (624 bytes)
    #define FFS_BUF_SIZE    (16K)   // Flash filesystem cache
    // TODO Set OMV_JPEG_BUF_SIZE if you change this
    #define JPEG_BUF_SIZE   (8K)    // IDE JPEG buffer

    #define FLASH_ORIGIN    0x08000000
    #define FLASH_LENGTH    1024K
    
    #define TEXT_ORIGIN     0x08010000
    #define TEXT_LENGTH     960K
       
    #define CCM_ORIGIN      0x10000000
    #define CCM_LENGTH      64K

    #define SRAM1_ORIGIN    0x20000000
    #define SRAM1_LENGTH    163K

    #define SRAM2_ORIGIN    0x20028C00
    #define SRAM2_LENGTH    29K

#elif defined(OPENMV3)
    #define FB_MEMORY       SRAM1   // Framebuffer, fb_alloc
    #define MAIN_MEMORY     CCM     // data, bss, stack and heap
    #define DMA_MEMORY      CCM     // Misc DMA buffers

    #define FB_SIZE         (301K)  // FB memory: header + VGA/GS image
    #define FB_ALLOC_SIZE   (83K)   // minimum fb alloc size

    #define STACK_SIZE      (4K)
    #define HEAP_SIZE       (55K)

    #define LINE_BUF_SIZE   (3K)    // Image line buffer round(640 * 2BPP * 2 buffers).
    #define MSC_BUF_SIZE    (2K)    // USB MSC bot data
    #define VFS_BUF_SIZE    (1K)    // VFS sturct + FATFS file buffer (624 bytes)
    #define FFS_BUF_SIZE    (32K)   // Flash filesystem cache
    // TODO Set OMV_JPEG_BUF_SIZE if you change this
    #define JPEG_BUF_SIZE   (23K)   // IDE JPEG buffer

    #define FLASH_ORIGIN    0x08000000
    #define FLASH_LENGTH    2048K
    
    #define TEXT_ORIGIN     0x08020000
    #define TEXT_LENGTH     1920K
    
    // Note DTCM/ITCM memory is not cacheable on M7
    #define CCM_ORIGIN      0x20000000
    #define CCM_LENGTH      128K

    #define SRAM1_ORIGIN    0x20020000
    #define SRAM1_LENGTH    368K

    #define SRAM2_ORIGIN    0x2007C000
    #define SRAM2_LENGTH    16K
#elif defined(OPENMV4)
    #define FB_MEMORY       SRAM1   // Framebuffer, fb_alloc
    #define MAIN_MEMORY     CCM     // data, bss, stack and heap
    #define DMA_MEMORY      CCM     // Misc DMA buffers

    #define FB_SIZE         (301K)  // FB memory: header + VGA/GS image
    #define FB_ALLOC_SIZE   (83K)   // minimum fb alloc size

    #define STACK_SIZE      (4K)
    #define HEAP_SIZE       (55K)

    #define LINE_BUF_SIZE   (3K)    // Image line buffer round(640 * 2BPP * 2 buffers).
    #define MSC_BUF_SIZE    (2K)    // USB MSC bot data
    #define VFS_BUF_SIZE    (1K)    // VFS sturct + FATFS file buffer (624 bytes)
    #define FFS_BUF_SIZE    (32K)   // Flash filesystem cache
    // TODO Set OMV_JPEG_BUF_SIZE if you change this
    #define JPEG_BUF_SIZE   (23K)   // IDE JPEG buffer

    #define FLASH_ORIGIN    0x08000000
    #define FLASH_LENGTH    2048K

    #define TEXT_ORIGIN     0x08020000
    #define TEXT_LENGTH     1920K

    // Note DTCM/ITCM memory is not cacheable on M7
    #define CCM_ORIGIN      0x20000000
    #define CCM_LENGTH      128K

    #define SRAM1_ORIGIN    0x20020000
    #define SRAM1_LENGTH    368K

    #define SRAM2_ORIGIN    0x2007C000
    #define SRAM2_LENGTH    16K
#endif

/* Specify the memory areas */
MEMORY
{
  CCM (xrw)         : ORIGIN = CCM_ORIGIN,      LENGTH = CCM_LENGTH
  SRAM1 (xrw)       : ORIGIN = SRAM1_ORIGIN,    LENGTH = SRAM1_LENGTH
  SRAM2 (xrw)       : ORIGIN = SRAM2_ORIGIN,    LENGTH = SRAM2_LENGTH
  FLASH_TEXT (rx)   : ORIGIN = TEXT_ORIGIN,     LENGTH = TEXT_LENGTH
}

_fb_base    = ORIGIN(FB_MEMORY);
_fballoc    = ORIGIN(FB_MEMORY) + FB_SIZE + FB_ALLOC_SIZE;

_estack     = ORIGIN(MAIN_MEMORY) + LENGTH(MAIN_MEMORY);
_ram_end    = ORIGIN(MAIN_MEMORY) + LENGTH(MAIN_MEMORY);

_heap_size  = HEAP_SIZE;    /* required amount of heap */
_stack_size = STACK_SIZE;   /* minimum amount of stack */

/* Define output sections */
SECTIONS
{
  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    . = ALIGN(4);
    *(.text*)          /* .text* sections (code) */
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    . = ALIGN(4);
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH_TEXT

  /* used by the startup to initialize data */
  _sidata = .;

  /* Non-cacheable DMA buffers */
  .dma_buffers (NOLOAD) :
  {
    . = ALIGN(4);
    _line_buf = .; // Image line buffer.
    . = . + LINE_BUF_SIZE;

    . = ALIGN(4);
    _msc_buf  = .; // USB MSC bot data (2K)
    . = . + MSC_BUF_SIZE;

    . = ALIGN(4);
    _vfs_buf  = .; // VFS sturct + FATFS file buffer  (around 624 bytes)
    . = . + VFS_BUF_SIZE;

    . = ALIGN(4);
    _ffs_cache = .; // Flash filesystem cache
    . = . + FFS_BUF_SIZE;

    . = ALIGN(4);
    _jpeg_buf = .; // IDE JPEG buffer
    . = . + JPEG_BUF_SIZE;

    . = ALIGN(4);
    *(.dma_buffer)

  } >DMA_MEMORY

  /* Initialized data sections goes into ram, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    _ram_start = .;
    *(.data)           /* .data sections */
    . = ALIGN(4);
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >MAIN_MEMORY

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    . = ALIGN(4);
    *(.bss*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
    _bss_end = _ebss; /* for gccollect */
  } >MAIN_MEMORY

  ._heap (NOLOAD) :
  {
    . = ALIGN(4);
    _heap_start = .;
    . = . + _heap_size;
    . = ALIGN(4);
    _heap_end  = .;
  } >MAIN_MEMORY

  /* Make sure there is enough ram for the stack */
  ._stack (NOLOAD) :
  {
    . = ALIGN(4);
    . = . + _stack_size;
    . = ALIGN(4);
  } >MAIN_MEMORY

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
