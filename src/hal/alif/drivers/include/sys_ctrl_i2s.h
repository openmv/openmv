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
 * @file     sys_ctrl_i2s.h
 * @author   Sudhir Sreedharan
 * @email    sudhir@alifsemi.com
 * @version  V1.0.0
 * @date     21-Apr-2023
 * @brief    System Control Device information for I2S
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#ifndef SYS_CTRL_I2S_H

#define SYS_CTRL_I2S_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "peripheral_types.h"

#ifdef  __cplusplus
extern "C"
{
#endif

/*!< I2S Input Clock Source */
#define I2S_CLK_SOURCE_76P8M_IN_HZ          76800000

/*!< Clock divisor max/min value  */
#define I2S_CLK_DIVISOR_MAX      0x3FF
#define I2S_CLK_DIVISOR_MIN      2

#define I2S_INTERNAL_CLOCK_SOURCE  0
#define I2S_EXTERNAL_CLOCK_SOURCE  1

/**
 * enum I2S_INSTANCE
 * I2S instances
 */
typedef enum _I2S_INSTANCE
{
    I2S_INSTANCE_0,                         /**< I2S instance - 0   */
    I2S_INSTANCE_1,                         /**< I2S instance - 1   */
    I2S_INSTANCE_2,                         /**< I2S instance - 2   */
    I2S_INSTANCE_3,                         /**< I2S instance - 3   */
    I2S_INSTANCE_LP                         /**< I2S instance - LP  */
} I2S_INSTANCE;

/**
  \fn          void enable_i2s_sclk_aon(I2S_INSTANCE instance)
  \brief       Enable the I2S SCLK out to external device(always on)
  \param[in]   instance  I2S controller instance
  \return      none
*/
static inline void enable_i2s_sclk_aon(I2S_INSTANCE instance)
{
    switch(instance)
    {
    case I2S_INSTANCE_0:
        CLKCTL_PER_SLV->I2S0_CTRL |= I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_1:
        CLKCTL_PER_SLV->I2S1_CTRL |= I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_2:
        CLKCTL_PER_SLV->I2S2_CTRL |= I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_3:
        CLKCTL_PER_SLV->I2S3_CTRL |= I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_LP:
        M55HE_CFG->HE_I2S_CTRL |= I2S_CTRL_SCLK_AON;
        break;
    }
}

/**
  \fn          void disable_i2s_sclk_aon(I2S_INSTANCE instance)
  \brief       Disable the I2S SCLK out to external device
  \param[in]   instance  I2S controller instance
  \return      none
*/
static inline void disable_i2s_sclk_aon(I2S_INSTANCE instance)
{
    switch(instance)
    {
    case I2S_INSTANCE_0:
        CLKCTL_PER_SLV->I2S0_CTRL &= ~I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_1:
        CLKCTL_PER_SLV->I2S1_CTRL &= ~I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_2:
        CLKCTL_PER_SLV->I2S2_CTRL &= ~I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_3:
        CLKCTL_PER_SLV->I2S3_CTRL &= ~I2S_CTRL_SCLK_AON;
        break;
    case I2S_INSTANCE_LP:
        M55HE_CFG->HE_I2S_CTRL &= ~I2S_CTRL_SCLK_AON;
        break;
    }
}

/**
  \fn          void bypass_i2s_clock_divider(I2S_INSTANCE instance)
  \brief       Bypass the clock divider
  \param[in]   instance  I2S controller instance
  \return      none
*/
static inline void bypass_i2s_clock_divider(I2S_INSTANCE instance)
{
    switch(instance)
    {
    case I2S_INSTANCE_0:
        CLKCTL_PER_SLV->I2S0_CTRL |= I2S_CTRL_DIV_BYPASS;
        break;
    case I2S_INSTANCE_1:
        CLKCTL_PER_SLV->I2S1_CTRL |= I2S_CTRL_DIV_BYPASS;
        break;
    case I2S_INSTANCE_2:
        CLKCTL_PER_SLV->I2S2_CTRL |= I2S_CTRL_DIV_BYPASS;
        break;
    case I2S_INSTANCE_3:
        CLKCTL_PER_SLV->I2S3_CTRL |= I2S_CTRL_DIV_BYPASS;
        break;
    case I2S_INSTANCE_LP:
        M55HE_CFG->HE_I2S_CTRL |= I2S_CTRL_DIV_BYPASS;
        break;
    }
}

/**
  \fn          void select_i2s_clock_source(I2S_INSTANCE instance, bool ext_clk_src_enable)
  \brief       Select the I2S clock source for SCLK
  \param[in]   instance  I2S controller instance
  \param[in]   ext_clk_src_enable  Enable external/internal clock source
  \return      none
*/
static inline void select_i2s_clock_source(I2S_INSTANCE instance, bool ext_clk_src_enable)
{
    uint32_t val = ext_clk_src_enable ? \
                          I2S_CTRL_CLK_SEL_EXT_CLK : I2S_CTRL_CLK_SEL_76M8_CLK;

    switch(instance)
    {
    case I2S_INSTANCE_0:
        CLKCTL_PER_SLV->I2S0_CTRL &= ~I2S_CTRL_CLK_SEL_Msk;
        CLKCTL_PER_SLV->I2S0_CTRL |= val;
        break;
    case I2S_INSTANCE_1:
        CLKCTL_PER_SLV->I2S1_CTRL &= ~I2S_CTRL_CLK_SEL_Msk;
        CLKCTL_PER_SLV->I2S1_CTRL |= val;
        break;
    case I2S_INSTANCE_2:
        CLKCTL_PER_SLV->I2S2_CTRL &= ~I2S_CTRL_CLK_SEL_Msk;
        CLKCTL_PER_SLV->I2S2_CTRL |= val;
        break;
    case I2S_INSTANCE_3:
        CLKCTL_PER_SLV->I2S3_CTRL &= ~I2S_CTRL_CLK_SEL_Msk;
        CLKCTL_PER_SLV->I2S3_CTRL |= val;
        break;
    case I2S_INSTANCE_LP:
        M55HE_CFG->HE_I2S_CTRL &= ~I2S_CTRL_CLK_SEL_Msk;
        M55HE_CFG->HE_I2S_CTRL |= val;
        break;
    }
}

/**
  \fn          void enable_i2s_clock(I2S_INSTANCE instance)
  \brief       Enable I2S controller clock
  \param[in]   instance  I2S controller instance
  \return      none
*/
static inline void enable_i2s_clock(I2S_INSTANCE instance)
{
    switch(instance)
    {
    case I2S_INSTANCE_0:
        CLKCTL_PER_SLV->I2S0_CTRL |= I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_1:
        CLKCTL_PER_SLV->I2S1_CTRL |= I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_2:
        CLKCTL_PER_SLV->I2S2_CTRL |= I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_3:
        CLKCTL_PER_SLV->I2S3_CTRL |= I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_LP:
        M55HE_CFG->HE_I2S_CTRL |= I2S_CTRL_CKEN;
        break;
    }
}

/**
  \fn          void disable_i2s_clock(I2S_INSTANCE instance)
  \brief       Disable I2S controller clock
  \param[in]   instance  I2S controller instance
  \return      none
*/
static inline void disable_i2s_clock(I2S_INSTANCE instance)
{
    switch(instance)
    {
    case I2S_INSTANCE_0:
        CLKCTL_PER_SLV->I2S0_CTRL &= ~I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_1:
        CLKCTL_PER_SLV->I2S1_CTRL &= ~I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_2:
        CLKCTL_PER_SLV->I2S2_CTRL &= ~I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_3:
        CLKCTL_PER_SLV->I2S3_CTRL &= ~I2S_CTRL_CKEN;
        break;
    case I2S_INSTANCE_LP:
        M55HE_CFG->HE_I2S_CTRL &= ~I2S_CTRL_CKEN;
        break;
    }
}

/**
  \fn          void set_i2s_clock_divisor(I2S_INSTANCE instance, uint16_t clk_div)
  \brief       Enable the clock divider unit and set the value
  \param[in]   instance  I2S controller instance
  \param[in]   clk_div  Clock divider value
  \return      none
*/
static inline void set_i2s_clock_divisor(I2S_INSTANCE instance, uint16_t clk_div)
{
    uint32_t ctrl = 0;

    switch(instance)
    {
    case I2S_INSTANCE_0:
        ctrl = CLKCTL_PER_SLV->I2S0_CTRL;
        ctrl &= ~(I2S_CTRL_CKDIV_Msk | I2S_CTRL_DIV_BYPASS);
        ctrl |= (clk_div & I2S_CTRL_CKDIV_Msk);
        CLKCTL_PER_SLV->I2S0_CTRL = ctrl;
        break;
    case I2S_INSTANCE_1:
        ctrl = CLKCTL_PER_SLV->I2S1_CTRL;
        ctrl &= ~(I2S_CTRL_CKDIV_Msk | I2S_CTRL_DIV_BYPASS);
        ctrl |= (clk_div & I2S_CTRL_CKDIV_Msk);
        CLKCTL_PER_SLV->I2S1_CTRL = ctrl;
        break;
    case I2S_INSTANCE_2:
        ctrl = CLKCTL_PER_SLV->I2S2_CTRL;
        ctrl &= ~(I2S_CTRL_CKDIV_Msk | I2S_CTRL_DIV_BYPASS);
        ctrl |= (clk_div & I2S_CTRL_CKDIV_Msk);
        CLKCTL_PER_SLV->I2S2_CTRL = ctrl;
        break;
    case I2S_INSTANCE_3:
        ctrl = CLKCTL_PER_SLV->I2S3_CTRL;
        ctrl &= ~(I2S_CTRL_CKDIV_Msk | I2S_CTRL_DIV_BYPASS);
        ctrl |= (clk_div & I2S_CTRL_CKDIV_Msk);
        CLKCTL_PER_SLV->I2S3_CTRL = ctrl;
        break;
    case I2S_INSTANCE_LP:
        ctrl = M55HE_CFG->HE_I2S_CTRL;
        ctrl &= ~(I2S_CTRL_CKDIV_Msk | I2S_CTRL_DIV_BYPASS);
        ctrl |= (clk_div & I2S_CTRL_CKDIV_Msk);
        M55HE_CFG->HE_I2S_CTRL = ctrl;
        break;
    }
}

/**
  \fn          void set_i2s_sampling_rate(I2S_INSTANCE instance,
                                          uint32_t sclk_freq,
                                          uint32_t clock_source)
  \brief       Set the Sampling rate
  \param[in]   instance  I2S controller instance
  \param[in]   sclk_freq  sclk frequency in Hz
  \param[in]   clock_source  Input clock source in Hz
  \return      \ret 0 for Success else Error
*/
static inline int32_t set_i2s_sampling_rate(I2S_INSTANCE instance,
                                            uint32_t sclk_freq,
                                            uint32_t clock_source)
{
    long int div;

    div = lroundf(((float) clock_source / sclk_freq));

    if(div > I2S_CLK_DIVISOR_MAX)
        return -1;

    if(div < I2S_CLK_DIVISOR_MIN)
        bypass_i2s_clock_divider(instance);
    else
        set_i2s_clock_divisor(instance, (uint16_t)div);

    return 0;
}

#ifdef  __cplusplus
}
#endif
#endif /* SYS_CTRL_I2S_H */
