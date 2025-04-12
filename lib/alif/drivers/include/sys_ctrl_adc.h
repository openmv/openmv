/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef SYS_CTRL_ADC_H_
#define SYS_CTRL_ADC_H_

#include "peripheral_types.h"

/* ADC control */
#define ADC_CTRL_BASE                      ADC120_BASE

/* ADC reg1 position macro */
#define ADC120_DIFFERENTIAL_EN_Pos               (1)
#define ADC120_COMPARATOR_EN_Pos                 (2)
#define ADC120_COMPARATOR_BIAS_Pos               (3)
#define ADC120_VCM_DIV_Pos                       (5)

#define ADC121_DIFFERENTIAL_EN_Pos               (7)
#define ADC121_COMPARATOR_EN_Pos                 (8)
#define ADC121_COMPARATOR_BIAS_Pos               (9)
#define ADC121_VCM_DIV_Pos                       (11)

#define ADC122_DIFFERENTIAL_EN_Pos               (13)
#define ADC122_COMPARATOR_EN_Pos                 (14)
#define ADC122_COMPARATOR_BIAS_Pos               (15)
#define ADC122_VCM_DIV_Pos                       (17)

#define ADC_CTRL_ADC_CKEN_Msk                  (0x110)

/**
 * enum ADC_INSTANCE.
 * ADC instances.
 */
typedef enum _ADC_INSTANCE
{
    ADC_INSTANCE_ADC12_0,                         /* ADC12 instance - 0 */
    ADC_INSTANCE_ADC12_1,                         /* ADC12 instance - 1 */
    ADC_INSTANCE_ADC12_2,                         /* ADC12 instance - 2 */
    ADC_INSTANCE_ADC24_0,                         /* ADC24 instance - 0 */
} ADC_INSTANCE;

/* ADC120 */
static inline void enable_adc0_periph_clk(void)
{
    CLKCTL_PER_SLV->ADC_CTRL |= ADC_CTRL_ADC0_CKEN;
}

static inline void disable_adc0_periph_clk(void)
{
    /* ADC0 clock is common for all ADC instances
     * check for other instances are enable or not
     * if all instances are disable then only disable ADC0 CKEN */
    if (!(CLKCTL_PER_SLV->ADC_CTRL & ADC_CTRL_ADC_CKEN_Msk))
    {
        CLKCTL_PER_SLV->ADC_CTRL &= ~ADC_CTRL_ADC0_CKEN;
    }
}

/* ADC121 */
static inline void enable_adc1_periph_clk(void)
{
    /* ADC0 clock is common for all ADC instances */
    CLKCTL_PER_SLV->ADC_CTRL |= (ADC_CTRL_ADC1_CKEN | ADC_CTRL_ADC0_CKEN);
}

static inline void disable_adc1_periph_clk(void)
{
    CLKCTL_PER_SLV->ADC_CTRL &= ~ADC_CTRL_ADC1_CKEN;

    /* ADC0 clock is common for all ADC instances
     * check for other instances are enable or not
     * if all instances are disable then only disable ADC0 CKEN */
    if (!(CLKCTL_PER_SLV->ADC_CTRL & ADC_CTRL_ADC_CKEN_Msk))
    {
        CLKCTL_PER_SLV->ADC_CTRL &= ~ADC_CTRL_ADC0_CKEN;
    }
}

/* ADC122 */
static inline void enable_adc2_periph_clk(void)
{
    /* ADC0 clock is common for all ADC instances */
    CLKCTL_PER_SLV->ADC_CTRL |= (ADC_CTRL_ADC2_CKEN | ADC_CTRL_ADC0_CKEN);
}

static inline void disable_adc2_periph_clk(void)
{
    CLKCTL_PER_SLV->ADC_CTRL &= ~ADC_CTRL_ADC2_CKEN;

    /* ADC0 clock is common for all ADC instances
     * check for other instances are enable or not
     * if all instances are disable then only disable ADC0 CKEN */
    if (!(CLKCTL_PER_SLV->ADC_CTRL & ADC_CTRL_ADC_CKEN_Msk))
    {
        CLKCTL_PER_SLV->ADC_CTRL &= ~ADC_CTRL_ADC0_CKEN;
    }
}

/* ADC24 */
static inline void enable_adc24_periph_clk(void)
{
    CLKCTL_PER_SLV->ADC_CTRL |= ADC_CTRL_ADC24_CKEN;
}

static inline void disable_adc24_periph_clk(void)
{
    CLKCTL_PER_SLV->ADC_CTRL &= ~ADC_CTRL_ADC24_CKEN;
}

/*
 * @func         : void adc_set_differential_ctrl(uint8_t instance,
                                                  bool vcm_en,
                                                  bool differential_en)
 * @brief        : control differential setting
 * @parameter[1] : instance        : adc instances
 * @parameter[3] : differential_en : differential enable
 * @return       : NONE
 */
static inline void adc_set_differential_ctrl(uint8_t instance,
                                             bool differential_en)
{
    uint32_t value = 0U;

    volatile ADC_Type *adc_ctrl = (volatile ADC_Type *)ADC_CTRL_BASE;

    switch (instance)
    {
        case ADC_INSTANCE_ADC12_0:
        {
            value |= (differential_en << ADC120_DIFFERENTIAL_EN_Pos |
                     (1 << ADC120_VCM_DIV_Pos));
            break;
        }
        case ADC_INSTANCE_ADC12_1:
        {
            value |= (differential_en << ADC121_DIFFERENTIAL_EN_Pos |
                     (1 << ADC121_VCM_DIV_Pos));
            break;
        }
        case ADC_INSTANCE_ADC12_2:
        {
            value |= (differential_en << ADC122_DIFFERENTIAL_EN_Pos |
                     (1 << ADC122_VCM_DIV_Pos));
            break;
        }
        case ADC_INSTANCE_ADC24_0:
            break;
    }
    adc_ctrl->ADC_REG1 |= value;
}

/*
 * @func         : void adc_set_comparator_ctrl(uint8_t instance,
                             bool comparator_en,
                             uint8_t comparator_bias)
 * @brief        :control differential setting
 * @parameter[1] : instance        : adc instances
 * @parameter[2] : comparator_en   : comparator enable
 * @parameter[3] : comparator_bias : comparator bias value
 * @return       : NONE
 */
void adc_set_comparator_ctrl(uint8_t instance,
                             bool comparator_en,
                             uint8_t comparator_bias)
{
    uint32_t value = 0U;

    volatile ADC_Type *adc_ctrl = (volatile ADC_Type *)ADC_CTRL_BASE;

    switch (instance)
    {
        case ADC_INSTANCE_ADC12_0:
        {
            value |= (comparator_en << ADC120_COMPARATOR_EN_Pos |
                      comparator_bias << ADC120_COMPARATOR_BIAS_Pos);
            break;
        }
        case ADC_INSTANCE_ADC12_1:
        {
            value |= (comparator_en << ADC121_COMPARATOR_EN_Pos |
                      comparator_bias << ADC121_COMPARATOR_BIAS_Pos);
            break;
        }
        case ADC_INSTANCE_ADC12_2:
        {
            value |= (comparator_en << ADC122_COMPARATOR_EN_Pos |
                      comparator_bias << ADC122_COMPARATOR_BIAS_Pos);
            break;
        }
        case ADC_INSTANCE_ADC24_0:
            break;
    }

    adc_ctrl->ADC_REG1 |= value;
}

/**
 * @fn       : void adc_set_clk_control(const ADC_RESOURCES *ADC, bool enable)
 * @brief    : Enable/Disable ADC input clock
 * @param[1] : instance        : adc instances
 * @param[2] : enable : Enable/Disable control
 * @return   : NONE
*/
static inline void adc_set_clk_control(uint8_t instance, bool enable)
{
    switch (instance)
    {
        case ADC_INSTANCE_ADC12_0:
        {
            if (enable)
            {
                enable_adc0_periph_clk();
            }
            else
            {
                disable_adc0_periph_clk();
            }
            break;
        }
        case ADC_INSTANCE_ADC12_1:
        {
            if (enable)
            {
                enable_adc1_periph_clk();
            }
            else
            {
                disable_adc1_periph_clk();
            }
            break;
        }
        case ADC_INSTANCE_ADC12_2:
        {
            if (enable)
            {
                enable_adc2_periph_clk();
            }
            else
            {
                disable_adc2_periph_clk();
            }
        break;
        }
        case ADC_INSTANCE_ADC24_0:
        {
            if (enable)
            {
                enable_adc24_periph_clk();
            }
            else
            {
                disable_adc24_periph_clk();
            }
        break;
        }
    }
}

/**
  \fn     static inline void enable_adc24(void)
  \brief  Enable ADC24 from control register
  \param  none.
  \return none.
 */
static inline void enable_adc24(void)
{
    __disable_irq();

    AON->PMU_PERIPH |= PMU_PERIPH_ADC24_EN;

    __enable_irq();
}

/**
  \fn     static inline void disable_adc24(void)
  \brief  Disable ADC24 from control register
  \param  none.
  \return none.
 */
static inline void disable_adc24(void)
{
    __disable_irq();

    AON->PMU_PERIPH &= ~PMU_PERIPH_ADC24_EN;

    __enable_irq();
}

/*
 * @func           : void set_adc24_output_rate(uint32_t rate)
 * @brief          : set output rate for adc24
 * @parameter[1]   : bias : value for setting bias
 * @return         : NONE
 */
static inline void set_adc24_bias(uint32_t bias)
{
    __disable_irq();

    AON->PMU_PERIPH |= ((bias << PMU_PERIPH_ADC24_BIAS_Pos) & PMU_PERIPH_ADC24_BIAS_Msk);

    __enable_irq();
}

/*
 * @func           : void set_adc24_output_rate(uint32_t rate)
 * @brief          : set output rate for adc24
 * @parameter[1]   : rate : value for setting output rate
 * @return         : NONE
 */
static inline void set_adc24_output_rate(uint32_t rate)
{
    __disable_irq();

    AON->PMU_PERIPH |= ((rate << PMU_PERIPH_ADC24_OUTPUT_RATE_Pos) & PMU_PERIPH_ADC24_OUTPUT_RATE_Msk);

    __enable_irq();
}

/*
 * @func           : void enable_adc_pga_gain(uint32_t gain_setting)
 * @brief          : Enable the pga gain for adc instances
 * @parameter[1]   : instance : adc controller instance
 * @parameter[2]   : gain     : setting to configure
 * @return         : NONE
 */
static inline void enable_adc_pga_gain(uint8_t instance, uint32_t gain)
{
    __disable_irq();

    switch (instance)
    {
        case ADC_INSTANCE_ADC12_0:
        {
            AON->PMU_PERIPH |= (PMU_PERIPH_ADC1_PGA_EN |
                               ((gain << PMU_PERIPH_ADC1_PGA_GAIN_Pos) & PMU_PERIPH_ADC1_PGA_GAIN_Msk));
            break;
        }
        case ADC_INSTANCE_ADC12_1:
        {
            AON->PMU_PERIPH |= (PMU_PERIPH_ADC2_PGA_EN |
                               ((gain << PMU_PERIPH_ADC2_PGA_GAIN_Pos) & PMU_PERIPH_ADC2_PGA_GAIN_Msk));
            break;
        }
        case ADC_INSTANCE_ADC12_2:
        {
            AON->PMU_PERIPH |= (PMU_PERIPH_ADC3_PGA_EN |
                               ((gain << PMU_PERIPH_ADC3_PGA_GAIN_Pos) & PMU_PERIPH_ADC3_PGA_GAIN_Msk));
            break;
        }
        case ADC_INSTANCE_ADC24_0:
        {
            AON->PMU_PERIPH |= (PMU_PERIPH_ADC24_PGA_EN |
                               ((gain << PMU_PERIPH_ADC24_PGA_GAIN_Pos) & PMU_PERIPH_ADC24_PGA_GAIN_Msk));
            break;
        }
    }
    __enable_irq();
}

/*
 * @func           : void disable_adc_Pga_gain(uint32_t gain_setting)
 * @brief          : Disable the pga gain for adc instances
 * @parameter[1]   : instance : adc controller instance
 * @return         : NONE
 */
static inline void disable_adc_pga_gain(uint8_t instance)
{
    __disable_irq();

    switch (instance)
    {
        case ADC_INSTANCE_ADC12_0:
        {
            AON->PMU_PERIPH &= ~(PMU_PERIPH_ADC1_PGA_EN | PMU_PERIPH_ADC1_PGA_GAIN_Msk);
            break;
        }
        case ADC_INSTANCE_ADC12_1:
        {
            AON->PMU_PERIPH &= ~(PMU_PERIPH_ADC2_PGA_EN | PMU_PERIPH_ADC2_PGA_GAIN_Msk);
            break;
        }
        case ADC_INSTANCE_ADC12_2:
        {
            AON->PMU_PERIPH &= ~(PMU_PERIPH_ADC3_PGA_EN | PMU_PERIPH_ADC3_PGA_GAIN_Msk);
            break;
        }
        case ADC_INSTANCE_ADC24_0:
        {
            AON->PMU_PERIPH &= ~(PMU_PERIPH_ADC24_PGA_EN | PMU_PERIPH_ADC24_PGA_GAIN_Msk);
            break;
        }
    }

    __enable_irq();
}

#endif /* SYS_CTRL_ADC_H_ */
