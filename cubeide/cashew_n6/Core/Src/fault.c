/*
 * fault.c -- fault handlers: print the faulting PC and fault status over the UART (polled),
 * light the red LED and stop. Kept in AXISRAM (.text, not ITCM) so that a fault before the
 * ITCM code is copied can still be reported.
 */
#include "main.h"

static void fault_putc(char c) {
    if (!(RCC->APB2ENR & RCC_APB2ENR_USART1EN)) {
        return;                             // fault before the UART was set up
    }
    for (uint32_t n = 0; n < 1000000u && !(LOG_UART->ISR & USART_ISR_TXE_TXFNF); n++) {
    }
    LOG_UART->TDR = (uint8_t) c;
}

static void fault_puts(const char *s) {
    while (*s) {
        fault_putc(*s++);
    }
}

static void fault_hex(uint32_t v) {
    static const char h[] = "0123456789abcdef";
    fault_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        fault_putc(h[(v >> i) & 15]);
    }
}

__attribute__((used, noreturn)) void fault_report(uint32_t *sp, const char *what) {
    __disable_irq();
    fault_puts("\r\n*** ");
    fault_puts(what);
    fault_puts(" PC ");
    fault_hex(sp[6]);
    fault_puts(" LR ");
    fault_hex(sp[5]);
    fault_puts(" CFSR ");
    fault_hex(SCB->CFSR);
    fault_puts(" HFSR ");
    fault_hex(SCB->HFSR);
    fault_puts(" BFAR ");
    fault_hex(SCB->BFAR);
    fault_puts(" MMFAR ");
    fault_hex(SCB->MMFAR);
    fault_puts(" SFSR ");
    fault_hex(SAU->SFSR);
    fault_puts("\r\n");
    if (RCC->AHB4ENR & RCC_AHB4ENR_GPIOGEN) {
        LED_RED_PORT->BSRR = (uint32_t) LED_RED_PIN << 16;     // red LED on (active low)
    }
    for (;;) {
    }
}

// r0 = stacked exception frame (MSP or PSP per EXC_RETURN bit 2), r1 = name.
// MSPLIM is cleared first: after a stack overflow the report itself needs stack below the limit.
#define FAULT_HANDLER(name, text)                                   \
    __attribute__((naked)) void name(void) {                        \
        __asm volatile (                                            \
            "movs r2, #0     \n"                                    \
            "msr msplim, r2  \n"                                    \
            "tst lr, #4      \n"                                    \
            "ite eq          \n"                                    \
            "mrseq r0, msp   \n"                                    \
            "mrsne r0, psp   \n"                                    \
            "ldr r1, =1f     \n"                                    \
            "b fault_report  \n"                                    \
            ".ltorg          \n"                                    \
            "1: .asciz \"" text "\"\n"                              \
            ".align 2        \n");                                  \
    }

FAULT_HANDLER(HardFault_Handler, "HardFault")
FAULT_HANDLER(MemManage_Handler, "MemManage")
FAULT_HANDLER(BusFault_Handler, "BusFault")
FAULT_HANDLER(UsageFault_Handler, "UsageFault")
FAULT_HANDLER(SecureFault_Handler, "SecureFault")
