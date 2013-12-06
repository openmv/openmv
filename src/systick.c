#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_misc.h>
#include "systick.h"
static volatile uint32_t sys_ticks; 

void SysTick_Handler(void)
{
    ++sys_ticks;       
}

int systick_init()
{
    /* Configure systick to interrupt every 1ms */
    if (SysTick_Config(SystemCoreClock / 1000)) {
        return -1;
    }
 
    /* Set SysTick IRQ to the highest priority */
    NVIC_SetPriority(SysTick_IRQn, 0);
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
