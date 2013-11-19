#ifndef __SYSTICK_H__
#define __SYSTICK_H__
int systick_init();
void systick_sleep(uint32_t ms);
uint32_t systick_current_millis();
#endif /* __SYSTICK_H__ */
