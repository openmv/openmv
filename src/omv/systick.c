#include <stm32f4xx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include "systick.h"
int systick_init()
{
    return 0;
}

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

bool sys_tick_has_passed(uint32_t stc, uint32_t delay_ms)
{
    systick_sleep(delay_ms);
    return true;
}
