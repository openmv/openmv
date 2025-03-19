/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef SYS_CTRL_UART_H_
#define SYS_CTRL_UART_H_

#include <stdint.h>
#include "peripheral_types.h"


static inline void enable_lpuart_clock(void)
{
	AON->RTSS_HE_LPUART_CKEN = (1 << 0);
}

static inline void disable_lpuart_clock(void)
{
	AON->RTSS_HE_LPUART_CKEN &= ( ~ (1 << 0) );
}

static inline void select_uart_dma1(uint32_t instance)
{
	/* select DMA1 only available for UART4-UART7 */
	if(instance == 4)
		CLKCTL_PER_SLV->UART_CTRL |= (1 << 24);
	else if(instance == 5)
		CLKCTL_PER_SLV->UART_CTRL |= (1 << 25);
	else if(instance == 6)
		CLKCTL_PER_SLV->UART_CTRL |= (1 << 26);
	else if(instance == 7)
		CLKCTL_PER_SLV->UART_CTRL |= (1 << 27);
}

static inline void enable_uart_clock(uint32_t instance)
{
	/* Enable UART clock for selected instance.
	 *  bits 0-7. (one bit for each instance.) */
	CLKCTL_PER_SLV->UART_CTRL |= (1 << instance);
}

static inline void disable_uart_clock(uint32_t instance)
{
	/* Disable UART clock for selected instance.
	 *  bits 0-7. (one bit for each instance.) */
	CLKCTL_PER_SLV->UART_CTRL &= ( ~ (1 << instance) );
}

static inline void select_uart_clock_hfosc_clk(uint32_t instance)
{
	/* UART clock select.
	 *  bits 8-15. (one bit for each instance.)
	 *       0: HFOSC_CLK */
	CLKCTL_PER_SLV->UART_CTRL &= ( ~ ( (1 << instance) << 8 ) );
}

static inline void select_uart_clock_syst_pclk(uint32_t instance)
{
	/* UART clock select.
	 *  bits 8-15. (one bit for each instance.)
	 *       1: SYST_PCLK clock */
	CLKCTL_PER_SLV->UART_CTRL |= ( (1 << instance) << 8 );
}

#endif /* SYS_CTRL_UART_H_ */
