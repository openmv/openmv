/*
 * stm32n6xx_it.c -- interrupt handlers (placed in ITCM by the linker script).
 */
#include "main.h"
#include "timebase.h"
#include "uart_log.h"

extern DCMIPP_HandleTypeDef hdcmipp;

// Fault handlers are in fault.c (kept out of ITCM so early faults can be reported).

void NMI_Handler(void) {
    for (;;) {
    }
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();
    (void) tb_us();     // keeps the microsecond clock's cycle-counter extension current
}

// ---- peripherals ----------------------------------------------------------------------
void DCMIPP_IRQHandler(void) {
    HAL_DCMIPP_IRQHandler(&hdcmipp);
}

void CSI_IRQHandler(void) {
    HAL_DCMIPP_CSI_IRQHandler(&hdcmipp);
}

void USART1_IRQHandler(void) {
    ulog_irq_handler();
}
