/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/******************************************************************************
 * @file     sys_ctrl_canfd.h
 * @author   Shreehari H K
 * @email    shreehari.hk@alifsemi.com
 * @version  V1.0.0
 * @date     05-July-2023
 * @brief    System control header file for canfd
 * @bug      None.
 * @Note     None.
 ******************************************************************************/

#ifndef SYS_CTRL_CANFD_H_
#define SYS_CTRL_CANFD_H_

#include <stdbool.h>
#include "peripheral_types.h"

#define CANFD_CLK_SRC_38P4MHZ_CLK           38400000U                        /* 38.4 MHz */
#define CANFD_CLK_SRC_160MHZ_CLK            160000000U                       /* 160 MHz  */
#define CANFD_MAX_CLK_SPEED                 (CANFD_CLK_SRC_160MHZ_CLK / 2U)  /* 80 MHz   */

/* Macros for Clock control register */
#define CANFD_CTRL_FD_ENA                   (1U << 20U)
#define CANFD_CTRL_CLK_SEL_Pos              (16U)
#define CANFD_CTRL_CKEN                     (1U << 12U)
#define CANFD_CKDIV_Pos                     (0U)

/**
  \fn          static inline void canfd_clock_enable(const bool clk_sel,
  \                                                  const uint8_t clk_div)
  \brief       Enables CANFD clock
  \param[in]   clk_sel : Clock selection (160 MHz / 38.4 MHz)
  \param[in]   clk_div : clock divider value
  \return      none
*/
static inline void canfd_clock_enable(const bool clk_sel,
                                      const uint8_t clk_div)
{
    /* Enables clock for CANFD module */
    CLKCTL_PER_SLV->CANFD_CTRL = (CANFD_CTRL_CKEN                       |
                                  (clk_sel << CANFD_CTRL_CLK_SEL_Pos)   |
                                  (clk_div << CANFD_CKDIV_Pos));
}

/**
  \fn          static inline void canfd_clock_disable(void)
  \brief       Disables CANFD clock
  \param       none
  \return      none
*/
static inline void canfd_clock_disable(void)
{
    /* Disables clock for CANFD module */
    CLKCTL_PER_SLV->CANFD_CTRL &= ~CANFD_CTRL_CKEN;
}

/**
  \fn          static inline void canfd_setup_fd_mode (const bool enable)
  \brief       enable/disables CANFD Fast data mode
  \param[in]   enable : Command to enable/disable for Fast data mode
  \return      none
*/
static inline void canfd_setup_fd_mode(const bool enable)
{
    if(enable)
    {
        CLKCTL_PER_SLV->CANFD_CTRL |= CANFD_CTRL_FD_ENA;
    }
    else
    {
        CLKCTL_PER_SLV->CANFD_CTRL &= ~CANFD_CTRL_FD_ENA;
    }
}

/**
  \fn          static inline bool canfd_in_fd_mode (void)
  \brief       returns canfd mode (FD / Classic 2.0)
  \param[in]   none
  \return      true - In FD mode/ false - Classic 2.0 mode)
*/
static inline bool canfd_in_fd_mode(void)
{
    return (CLKCTL_PER_SLV->CANFD_CTRL & (CANFD_CTRL_FD_ENA));
}

#endif /* SYS_CTRL_CANFD_H_ */
