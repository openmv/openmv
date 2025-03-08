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
 * @file     sys_ctrl_lptimer.h
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     24-04-2023
 * @brief    LPTIMER system control Specific Header file.
 ******************************************************************************/

#ifndef SYS_CTRL_LPTIMER_H_
#define SYS_CTRL_LPTIMER_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum LPTIMER_CLK_SRC.
 * lptimer clock source selection.
 */
typedef enum _LPTIMER_CLK_SRC {
    LPTIMER_CLK_SOURCE_32K     = 0x0U,                   /**< LPTIMER 32KHz Clock Source>*/
    LPTIMER_CLK_SOURCE_128K    = 0x1U,                   /**< LPTIMER 128KHz Clock Source>*/
    LPTIMER_CLK_EXT_SOURCE     = 0x2U,                   /**< LPTIMER External Clock Source>*/
    LPTIMER_CLK_SOURCE_CASCADE = 0x3U                    /**< LPTIMER Cascade Timer Clock Source>*/
} LPTIMER_CLK_SRC;

/**
  \fn          static inline void select_lptimer_clk (LPTIMER_RESOURCES LPTIMER_RES, uint8_t channel)
  \brief       LPTIMER input clock selection
  \param[in]   clk       clock
  \param[in]   channel   used LPTIMER channel
  \return      none
*/
static inline void select_lptimer_clk (LPTIMER_CLK_SRC clk, uint8_t channel)
{
    /* VBAT TIMER_CLKSEL is used to select LPTIMER clock source
     * Bit 0-1  : used for LPTIMER channel 0
     * Bit 4-5  : used for LPTIMER channel 1
     * Bit 8-9  : used for LPTIMER channel 2
     * Bit 12-13: used for LPTIMER channel 3
     * */

    /* clear appropriate channel bits */
    VBAT->TIMER_CLKSEL &= ~(TIMER_CLKSEL_Msk << (channel << 2U));

    /* assign value to the appropriate channel bits */
    VBAT->TIMER_CLKSEL |= (clk << (channel << 2U));
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_LPTIMER_H_ */
