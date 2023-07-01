/*
 * Copyright (c) 2016 STMicroelectronics. All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 * USB3320 ULPI functions ported from stm32f7xx_lp_modes.c
 */
#ifndef __ULPI_H__
#define __ULPI_H__
void ulpi_enter_low_power(void);
void ulpi_leave_low_power(void);
#endif // __ULPI_H__
