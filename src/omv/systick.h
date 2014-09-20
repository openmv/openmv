/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Systick driver.
 *
 */
#ifndef __SYSTICK_H__
#define __SYSTICK_H__
#include <stdint.h>
int systick_init();
void systick_sleep(uint32_t ms);
uint32_t systick_current_millis();
bool sys_tick_has_passed(uint32_t stc, uint32_t delay_ms);
#endif // __SYSTICK_H__
