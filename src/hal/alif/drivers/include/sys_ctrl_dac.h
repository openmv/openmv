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
 * @file     sys_ctrl_dac.h
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     27-Apr-2023
 * @brief    System Control Device information for DAC.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#ifndef SYS_CTRL_DAC_H_
#define SYS_CTRL_DAC_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
 * enum DAC_INSTANCE.
 * DAC instances.
 */
typedef enum _DAC_INSTANCE
{
    DAC_INSTANCE_0,    /* DAC instance - 0 */
    DAC_INSTANCE_1     /* DAC instance - 1 */
}DAC_INSTANCE;

/**
  \fn     static inline void enable_dac_periph_clk(DAC_INSTANCE instance)
  \brief  Enable DAC0 and DAC1 peripheral clock register.
  \param  none.
  \return none.
 */
static inline void enable_dac_periph_clk(DAC_INSTANCE instance)
{
    if (instance == DAC_INSTANCE_0)
    {
        CLKCTL_PER_SLV->DAC_CTRL |= DAC_CTRL_DAC0_CKEN;
    }
    else
    {
        CLKCTL_PER_SLV->DAC_CTRL |= DAC_CTRL_DAC1_CKEN;
    }
}

/**
  \fn     static inline void disable_dac_periph_clk(DAC_INSTANCE instance)
  \brief  Disable DAC0 and DAC1 peripheral clock register.
  \param  none.
  \return none.
 */
static inline void disable_dac_periph_clk(DAC_INSTANCE instance)
{
    if (instance == DAC_INSTANCE_0)
    {
        CLKCTL_PER_SLV->DAC_CTRL &= ~(DAC_CTRL_DAC0_CKEN);
    }
    else
    {
        CLKCTL_PER_SLV->DAC_CTRL &= ~(DAC_CTRL_DAC1_CKEN);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* SYS_CTRL_DAC_H_ */
