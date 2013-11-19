#include <stdint.h>
#include <stm32f4xx.h>
#include "systick.h"
static volatile uint32_t sys_ticks; 

void SysTick_Handler(void)
{
    ++sys_ticks;       
}

int systick_init()
{
    /* configure systick to interrupt every 1ms */
    if (SysTick_Config(SystemCoreClock / 1000)) {
        return -1;
    }
    return 0;
}

void systick_sleep(uint32_t ms)
{
    uint32_t curr_ticks = sys_ticks;
    while ((sys_ticks - curr_ticks) < ms);
}

uint32_t systick_current_millis()
{
    return sys_ticks;
}
