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
 * @file     sys_ctrl_pdm.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     27-Apr-2023
 * @brief    System Control Device information for PDM.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef SYS_CTRL_PDM_H_
#define SYS_CTRL_PDM_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 @fn          void enable_pdm_periph_clk(void)
 @brief       Enable PDM input clock
 @param[in]   none
 @return      none
 */
static inline void enable_pdm_periph_clk(void)
{
    CLKCTL_PER_SLV->EXPMST0_CTRL |= EXPMST0_CTRL_PDM_CKEN;
}

/**
 @fn          void disable_pdm_periph_clk(void)
 @brief       Disable PDM input clock
 @param[in]   none
 @return      none
 */
static inline void disable_pdm_periph_clk(void)
{
    CLKCTL_PER_SLV->EXPMST0_CTRL &= ~EXPMST0_CTRL_PDM_CKEN;
}

/**
 @fn          void enable_lppdm_periph_clk(void)
 @brief       Enable LPPDM input clock
 @param[in]   none
 @return      none
 */
static inline void enable_lppdm_periph_clk(void)
{
    M55HE_CFG->HE_CLK_ENA |= HE_CLK_ENA_PDM_CKEN;
}

/**
 @fn          void disable_lppdm_periph_clk(void)
 @brief       Disable LPPDM input clock
 @param[in]   none
 @return      none
 */
static inline void disable_lppdm_periph_clk(void)
{
    M55HE_CFG->HE_CLK_ENA &= ~HE_CLK_ENA_PDM_CKEN;
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_PDM_H_ */
