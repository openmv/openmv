#include STM32_HAL_H
#include "stdint.h"
#include "stdbool.h"

void systick_sleep(volatile uint32_t ms)
{
    volatile uint32_t curr_ticks = HAL_GetTick();
    while ((HAL_GetTick() - curr_ticks) < ms) {
        __WFI();
    }
}

uint32_t systick_current_millis()
{
    return HAL_GetTick();
}

bool sys_tick_has_passed(uint32_t start_tick, uint32_t delay_ms) {
    return HAL_GetTick() - start_tick >= delay_ms;
}
