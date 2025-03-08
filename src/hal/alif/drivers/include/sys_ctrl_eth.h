/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#ifndef SYS_CTRL_ETH_H_
#define SYS_CTRL_ETH_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include <peripheral_types.h>

/**
  \fn     static inline void enable_eth_periph_clk(void)
  \brief  Enable Ethernet peripheral clock in CLKCTL_PER_MST PERIPH_CLK_ENA register.
  \param  none.
  \return none.
 */
static inline void enable_eth_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_ETH_CKEN;
}

/**
  \fn     static inline void disble_eth_periph_clk(void)
  \brief  Disable Ethernet peripheral clock in CLKCTL_PER_MST PERIPH_CLK_ENA register.
  \param  none.
  \return none.
 */
static inline void disable_eth_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_ETH_CKEN;
}

#ifdef  __cplusplus
}
#endif

#endif /* SYS_CTRL_ETH_H_ */
