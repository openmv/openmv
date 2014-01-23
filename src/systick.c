#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_misc.h>
#include <stdlib.h>
#include "systick.h"
#include "array.h"
static volatile uint32_t sys_ticks; 
static struct array *task_list;

struct systick_task {
    task_cb cb;
    uint32_t period;
};

void SysTick_Handler(void)
{
    int i;
    struct systick_task *task;

    ++sys_ticks;       

    for (i=0; i<array_length(task_list); i++) {
        task = array_at(task_list, i);
        if ((sys_ticks % task->period)==0) {
            task->cb();
        }
    }
}

int systick_init()
{
    /* Allocate task_list array */
    array_alloc(&task_list, free);

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

void systick_sched_task(task_cb cb, uint32_t period)
{
    struct systick_task *task;

    task = malloc(sizeof(struct systick_task));
    task->cb = cb;
    task->period = period;
    array_push_back(task_list, task);
}
