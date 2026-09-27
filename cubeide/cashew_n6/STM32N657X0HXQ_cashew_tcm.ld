/*
 * STM32N657X0HXQ_cashew_tcm.ld -- linker script for the cashew camera FSBL image
 * (NUCLEO-N657X0-Q, Cortex-M55 secure, 600 MHz).
 *
 * Based on the STM32N657XX_AXISRAM2_fsbl.ld template from STMicroelectronics'
 * cmsis-device-n6 (Apache-2.0), with the tightly coupled memories added:
 *
 *   ITCM  0x10000000   64 KB  detector code (cashew_core.o), interrupt handlers, capture
 *                             and main loop, UART driver, libm and memset/memcpy.
 *   DTCM  0x30000000  128 KB  all .data/.bss (detector hot work buffers ~93 KB, frame
 *                             queue, parameters, UART rings), detector constants, and the
 *                             16 KB main stack at the top.
 *   ROM   0x34180400  255 KB  the FSBL image the boot ROM copies from flash (AXISRAM2):
 *                             vector table, init code, constants, and the load images of
 *                             the ITCM code and DTCM data.
 *   RAM   0x341C0000  256 KB  larger, rarely used buffers (.sram_bss): the detector's
 *                             warm work buffers (candidate hull / neck test).
 *   FRAMES 0x34000000   1 MB  AXISRAM1: DCMIPP frame buffers (the camera DMA cannot
 *                             write to the TCMs).
 *
 * The ITCM code is copied from its load address by tcm_init() (a .preinit_array hook,
 * before main). .data (in DTCM) is copied and .bss zeroed by the ST startup code;
 * .sram_bss is zeroed by tcm_init().
 */

ENTRY(Reset_Handler)

_Min_Stack_Size = 16K;  /* main stack, top of DTCM */
_Min_Heap_Size  = 0;    /* no malloc in this firmware */

MEMORY
{
  ITCM   (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
  DTCM   (rw)  : ORIGIN = 0x30000000, LENGTH = 128K
  FRAMES (rw)  : ORIGIN = 0x34000000, LENGTH = 1024K
  ROM    (xrw) : ORIGIN = 0x34180400, LENGTH = 255K
  RAM    (xrw) : ORIGIN = 0x341C0000, LENGTH = 256K
}

_estack = ORIGIN(DTCM) + LENGTH(DTCM);
_sstack = _estack - _Min_Stack_Size;

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >ROM

  /* ---- ITCM: code that runs every frame ------------------------------------------ */
  .itcm_text :
  {
    . = ALIGN(8);
    _sitcm = .;
    *cashew_core.o(.text .text*)
    *cashew_app.o(.text .text*)
    *capture.o(.text .text*)
    *uart_log.o(.text .text*)
    *timebase.o(.text .text*)
    *stm32n6xx_it.o(.text .text*)
    *(.itcm_text .itcm_text*)
    *stm32n6xx_hal_dcmipp.o(.text.HAL_DCMIPP_IRQHandler .text.HAL_DCMIPP_CSI_IRQHandler)
    *stm32n6xx_hal_dcmipp.o(.text.HAL_DCMIPP_PIPE_SetMemoryAddress)
    *stm32n6xx_hal.o(.text.HAL_IncTick .text.HAL_GetTick)
    *stm32n6xx_hal_gpio.o(.text.HAL_GPIO_WritePin .text.HAL_GPIO_TogglePin .text.HAL_GPIO_ReadPin)
    *stm32n6xx_hal_cortex.o(.text.HAL_NVIC_EnableIRQ .text.HAL_NVIC_DisableIRQ)
    *libm.a:*(.text .text*)
    *libc.a:*memset*(.text .text*)
    *libc.a:*memcpy*(.text .text*)
    *libc_nano.a:*memset*(.text .text*)
    *libc_nano.a:*memcpy*(.text .text*)
    *libgcc.a:*(.text .text*)
    . = ALIGN(8);
    _eitcm = .;
  } >ITCM AT> ROM
  _siitcm = LOADADDR(.itcm_text);

  /* ---- everything else runs from AXISRAM2 (cached) ------------------------------- */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    *(.RamFunc)
    *(.RamFunc*)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >ROM

  .rodata :
  {
    . = ALIGN(4);
    *(EXCLUDE_FILE(*cashew_core.o) .rodata)
    *(EXCLUDE_FILE(*cashew_core.o) .rodata*)
    . = ALIGN(4);
  } >ROM

  .ARM.extab (READONLY) :
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >ROM

  .ARM (READONLY) :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >ROM

  .preinit_array (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >ROM

  .init_array (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >ROM

  .fini_array (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >ROM

  .gnu.sgstubs :
  {
    _sNSCVeneer = .;
    . = ALIGN(4);
    *(.gnu.sgstubs*)
    . = ALIGN(4);
  } >ROM
  _eNSCVeneer = .;

  /* ---- DTCM: all initialised and zeroed variables ---------------------------------- */
  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(8);
    _sdata = .;
    *cashew_core.o(.rodata .rodata*)   /* detector lookup tables: read from DTCM */
    *(.dtcm_data .dtcm_data*)
    *(.data)
    *(.data*)
    . = ALIGN(8);
    _edata = .;
  } >DTCM AT> ROM

  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.dtcm_bss .dtcm_bss*)
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(8);
    _ebss = .;
    __bss_end__ = _ebss;
  } >DTCM

  ASSERT(_ebss <= _sstack, "DTCM overflow: .data/.bss run into the 16 KB stack")

  /* ---- AXISRAM2: larger buffers that are not touched on every pixel ---------------- */
  .sram_bss (NOLOAD) :
  {
    . = ALIGN(32);
    _ssram_bss = .;
    *(.sram_bss .sram_bss*)
    . = ALIGN(32);
    _esram_bss = .;
  } >RAM

  ._user_heap :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = ALIGN(8);
  } >RAM

  /* ---- AXISRAM1: camera frame buffers (DMA target, never initialised) ------------- */
  .frames (NOLOAD) :
  {
    . = ALIGN(32);
    *(.frame_buffers .frame_buffers*)
    . = ALIGN(32);
  } >FRAMES

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
