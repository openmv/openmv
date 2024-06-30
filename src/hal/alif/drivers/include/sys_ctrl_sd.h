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
 * @file     sys_ctrl_sd.h
 * @author   Deepak Kumar
 * @email    deepak@alifsemi.com
 * @version  V0.0.1
 * @date     01-Sep-2023
 * @brief    SD Clock control
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef _SYS_CTRL_SD_H_
#define _SYS_CTRL_SD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "peripheral_types.h"

/**
  \fn     static inline void enable_sd_periph_clk(void)
  \brief  Enable SDMMC peripheral clock in CLKCTL_PER_MST PERIPH_CLK_ENA register.
  \param  none.
  \return none.
 */
static inline void enable_sd_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA |= PERIPH_CLK_ENA_SDC_CKEN;
}

/**
  \fn     static inline void disble_sd_periph_clk(void)
  \brief  Disable SDMMC peripheral clock in CLKCTL_PER_MST PERIPH_CLK_ENA register.
  \param  none.
  \return none.
 */
static inline void disable_sd_periph_clk(void)
{
    CLKCTL_PER_MST->PERIPH_CLK_ENA &= ~PERIPH_CLK_ENA_SDC_CKEN;
}

#ifdef __cplusplus
}
#endif

#endif
