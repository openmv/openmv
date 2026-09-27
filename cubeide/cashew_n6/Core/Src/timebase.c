/*
 * timebase.c -- microsecond clock from DWT CYCCNT (runs from ITCM, see the linker script).
 */
#include "main.h"
#include "timebase.h"

static uint32_t s_last_cyc;     // CYCCNT at the previous update
static uint32_t s_rem_cyc;      // cycles not yet converted to microseconds
static uint32_t s_us;           // microsecond counter
static uint32_t s_cyc_per_us;
static bool     s_dwt_ok;

// Fallback when the cycle counter does not run: HAL tick (1 ms) + SysTick down-counter.
static uint32_t systick_us(void) {
    uint32_t load = SysTick->LOAD + 1u;
    uint32_t ms = HAL_GetTick();
    uint32_t val = SysTick->VAL;
    if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) && val > load / 2) {
        ms++;                                   // wrapped, tick interrupt not taken yet
    }
    return ms * 1000u + (uint32_t) (((uint64_t) (load - 1u - val) * 1000u) / load);
}

void tb_init(void) {
    s_cyc_per_us = SystemCoreClock / 1000000u;
    if (s_cyc_per_us == 0) {
        s_cyc_per_us = 1;
    }
    // The firmware runs in Secure state: the cycle counter only counts there when secure
    // non-invasive debug is allowed, which may be off after a standalone boot from flash.
    DCB->DAUTHCTRL |= DCB_DAUTHCTRL_SPNIDENSEL_Msk | DCB_DAUTHCTRL_INTSPNIDEN_Msk;
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    uint32_t c0 = DWT->CYCCNT;
    for (volatile int i = 0; i < 100; i++) {
    }
    s_dwt_ok = (DWT->CYCCNT != c0);
    s_last_cyc = DWT->CYCCNT;
    s_rem_cyc = 0;
    s_us = 0;
}

bool tb_cycle_counter_ok(void) {
    return s_dwt_ok;
}

uint32_t tb_us(void) {
    if (!s_dwt_ok) {
        return systick_us();
    }
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t c = DWT->CYCCNT;
    s_rem_cyc += c - s_last_cyc;
    s_last_cyc = c;
    uint32_t q = s_rem_cyc / s_cyc_per_us;
    s_us += q;
    s_rem_cyc -= q * s_cyc_per_us;
    uint32_t us = s_us;
    __set_PRIMASK(primask);
    return us;
}

void tb_delay_us(uint32_t us) {
    uint32_t t0 = tb_us();
    while ((uint32_t) (tb_us() - t0) < us) {
    }
}
