/**
 *  Linker script for STM32F4xx Devices with 1MB FLASH, 192KB RAM (64KB CCM)
 */

/* Entry Point */
ENTRY(Reset_Handler)

#ifdef OPENMV1
#define HEAP_MEMORY     CCM
#define FLASH_ORIGIN    0x08000000
#define FLASH_LENGTH    512K

#define ISR_ORIGIN      0x08000000
#define ISR_LENGTH      16K

#define TEXT_ORIGIN     0x0800C000
#define TEXT_LENGTH     464K

#define RAM_ORIGIN      0x20000000
#define RAM_LENGTH      128K

#define CCM_ORIGIN      0x10000000
#define CCM_LENGTH      64K

#else
#define HEAP_MEMORY     SDRAM
#define FLASH_ORIGIN    0x08000000
#define FLASH_LENGTH    2048K

#define ISR_ORIGIN      0x08000000
#define ISR_LENGTH      16K

#define TEXT_ORIGIN     0x08040000
#define TEXT_LENGTH     1984K

#define RAM_ORIGIN      0x20000000
#define RAM_LENGTH      192K

#define CCM_ORIGIN      0x10000000
#define CCM_LENGTH      64K

#define SDRAM_ORIGIN    0xC0000000
#define SDRAM_LENGTH    15M
#endif //OPENMV1

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = FLASH_ORIGIN,  LENGTH = FLASH_LENGTH
  FLASH_ISR (rx)  : ORIGIN = ISR_ORIGIN,    LENGTH = ISR_LENGTH
  FLASH_TEXT (rx) : ORIGIN = TEXT_ORIGIN,   LENGTH = TEXT_LENGTH
  RAM (xrw)       : ORIGIN = RAM_ORIGIN,    LENGTH = RAM_LENGTH
  CCM (xrw)       : ORIGIN = CCM_ORIGIN,    LENGTH = CCM_LENGTH
#ifdef OPENMV2
  SDRAM (xrw)     : ORIGIN = SDRAM_ORIGIN,  LENGTH = SDRAM_LENGTH
#endif
}

_estack     = 0x10010000;   /* Stack is allocated on CCM block */
_ram_end    = 0x10010000;   /* 64KB CCM */

#ifdef OPENMV1

_fb_base    = 0x20000000;
_stack_size = 0x1300;       /* required amount of stack */
_heap_size  = 0x7300;       /* required amount of heap */
_cache_size = 0x4000;       /* required amount of cache */

#else

_fb_base    = 0xC0F04000;   /* sdram_start + head_end + cache */
_stack_size = 0x4000;       /* required amount of stack */
_heap_size  = 0x00F00000;   /* required amount of heap */
_cache_size = 0x4000;       /* required amount of cache */
_heap_end   = ORIGIN(SDRAM) + _heap_size;

#endif

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    */ff.o(.text*)
    */stm32f4xx_hal_sd.o(.text*)
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

  ._heap :
  {
    . = ALIGN(4);
    _heap_start = .;
#ifdef OPENMV1
    . = . + _heap_size;
    . = ALIGN(4);
    _heap_end  = .;
#endif
  } >HEAP_MEMORY

  /* Make sure there is enough RAM the stack and FS cache */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    . = . + _stack_size;
#ifdef OPENMV1
    . = . + _cache_size;
#endif
    . = ALIGN(4);
  } >CCM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
