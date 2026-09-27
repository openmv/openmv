/*
 * timebase.h -- microsecond clock from the Cortex-M55 cycle counter (DWT CYCCNT).
 */
#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>
#include <stdbool.h>

void     tb_init(void);
// Free-running microseconds (wraps after ~71 minutes; use unsigned differences).
// Safe to call from interrupts. Must be called at least every 7 s (SysTick does it).
uint32_t tb_us(void);
void     tb_delay_us(uint32_t us);
// False if the DWT cycle counter does not run (then tb_us() uses SysTick, ~1 us resolution).
bool     tb_cycle_counter_ok(void);

#endif
