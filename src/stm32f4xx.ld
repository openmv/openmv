/**
 *  Linker script for STM32F4xx Devices with 1MB FLASH, 192KB RAM (64KB CCM)
 */

/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
  FLASH_ISR (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
  FLASH_TEXT (rx) : ORIGIN = 0x08020000, LENGTH = 512K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
  CCM (w!rx)      : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Highest address of the user mode stack */
_estack     = 0x10010000; /* Stack is allocated on CCM block */

_ram_end   = 0x10010000; /* 64KB CCM */

/* Generate a link error if heap and stack don't fit into RAM */
_stack_size = 0x1000; /* required amount of stack */
_heap_size  = 0x8000; /* required amount of heap */
_cache_size = 0x4000; /* required amount of cache */

_main_ram_start = 0x20000000;
/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH_ISR

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
    _exit = .;
  } >FLASH_TEXT

  .ARM.extab   : {
      *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH_TEXT

  .ARM : {
  __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH_TEXT

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH_TEXT

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH_TEXT

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
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

  ._heap :
  {
    . = ALIGN(4);
    _heap_start = .;
    . = . + _heap_size;
    . = ALIGN(4);
    _heap_end  = .;
  } >CCM

  /* Make sure there is enough RAM left for the stack */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    . = . + _cache_size;
    . = . + _stack_size;
    . = ALIGN(4);
  } >CCM

  /* Remove information from the standard libraries */
  /*
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
  */

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
