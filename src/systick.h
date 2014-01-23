#ifndef __SYSTICK_H__
#define __SYSTICK_H__
typedef void (*task_cb) ();
int systick_init();
void systick_sleep(uint32_t ms);
uint32_t systick_current_millis();
void systick_sched_task(task_cb cb, uint32_t period);
#endif /* __SYSTICK_H__ */
