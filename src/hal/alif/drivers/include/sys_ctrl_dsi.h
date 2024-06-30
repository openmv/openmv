/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     sys_ctrl_dsi.h
 * @author   Prasanna Ravi
 * @email    prasanna.ravi@alifsemi.com
 * @version  V1.0.0
 * @date     19-April-2023
 * @brief    DSI system control Specific Header file.
 ******************************************************************************/
#ifndef SYS_CTRL_DSI_H_
#define SYS_CTRL_DSI_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
  \fn          static inline void enable_dsi_periph_clk(void)
  \brief       Enable clock supply for DSI.
  \return      none.
*/
static inline void enable_dsi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_DSI_CKEN;
}

/**
  \fn          static inline void disable_dsi_periph_clk(void)
  \brief       Disable clock supply for DSI.
  \return      none.
*/
static inline void disable_dsi_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_DSI_CKEN;
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_DSI_H_ */
