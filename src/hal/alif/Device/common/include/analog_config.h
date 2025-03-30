/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef ANALOG_CONFIG_H_
#define ANALOG_CONFIG_H_

#include "peripheral_types.h"

#define VBAT_ANA_REG2_VAL       0x00C00000 /* Enable analog peripheral LDO and precision bandgap */

/* Used to scale the input reference to the DAC6 with a step size of 1/16.
 * Ex: DAC6_VREF_SCALE has a value of 0x5, then
 * Scaling factor = Register value/16 => 0x5/16 => 0.3125
 * which means input reference to the DAC6 is scaled by a factor of 5/16 */
#define DAC6_VREF_SCALE         (0x1U << 27)

/* Output of DAC6 programmable reference voltage.
 * DAC6 Output = (DAC6_VREF_SCALE × DAC6_CONT) / 64 */
#define DAC6_CONT               (0x20U << 21)

#define DAC6_EN                 (0x1U << 20) /* Enable the DAC6 */

/* Used select different voltage reference values for a DAC12
 * 000: 0.500 (or 500 mv)
   001: 0.667 (or 667 mV)
   010: 0.667 (or 667 mV)
   011: 0.750 (or 750 mV)
   100: 0.750 (or 750 mV)
   101: 0.800 (or 800 mV)
   110: 0.800 (or 800 mV)
   111: 0.833 (or 833 mV)
 */
#define DAC12_VREF_CONT         (0x4U << 17)

/* 0x0: Vref = 1.8 V
   0x1: Vref = 1.5 V
*/
#define ADC_VREF_BUF_RDIV_EN    (0x0U << 16)

/* 0x0: ADC Vref is off
   0x1: ADC Vref is on
*/
#define ADC_VREF_BUF_EN         (0x1U << 15)

/* Control the ADC Vref: 1.8 V ± 100 mV, where:
   00000: -100 mV
   00001: -93 mV
   00010: -86 mV
   ...
   11110: 93 mV
   11111: 100 mV
   10000: 1.8 V = Default (0x10)
   Step size = 7 mV
*/
#define ADC_VREF_CONT           (0x10U << 10)

/* Analog peripheral LDO (LDO-5) output voltage (VDD_ANA):
   0000: 1.6 V
   0001: 1.62 V
   ...
   1010: 1.8 V
   1111: 1.9 V
   Step: 20 mV Default (0xA)
*/
#define ANA_PERIPH_LDO_CONT     (0xAU << 6)

/* Calibration for analog peripherals precision bandgap:
   0000: 1.140 V
   ...
   1010: 1.2 V
   1111: 1.233 V
   Step: 6 mV Default (0xA)
*/
#define ANA_PERIPH_BG_CONT      (0xAU << 1)

#define CMP_REG2_VAL            (DAC6_VREF_SCALE | DAC6_CONT | DAC6_EN  |  \
                                 DAC12_VREF_CONT | ADC_VREF_BUF_RDIV_EN |  \
                                 ADC_VREF_BUF_EN | ADC_VREF_CONT        |  \
                                 ANA_PERIPH_LDO_CONT | ANA_PERIPH_BG_CONT)

#define CMP_REG2_BASE           (CMP0_BASE + 0x00000004) /* CMP register2 base address */

/**
 @fn          void analog_config_vbat_reg2(void)
 @brief       Assigning Vbat registers values to the Vbat register2 base address
 @param[in]   none
 @return      none
 */
static inline void analog_config_vbat_reg2(void)
{
    /* Analog configuration Vbat register2 */
    ANA_REG->VBAT_ANA_REG2 |= VBAT_ANA_REG2_VAL;
}

/**
 @fn          void analog_config_cmp_reg2(void)
 @brief       Assigning comparator register2 values to the comparator
              register2 base address
 @param[in]   none
 @return      none
 */
static inline void analog_config_cmp_reg2(void)
{
    /* Analog configuration comparator register2 */
    *((volatile uint32_t *)CMP_REG2_BASE) = CMP_REG2_VAL;
}

/**
  \fn     static inline void enable_cmp_periph_clk(void)
  \brief  Enable CMP Control register.
  \param  none.
  \return none.
 */
static inline void enable_cmp_periph_clk(void)
{
    CLKCTL_PER_SLV->CMP_CTRL |= CMP_CTRL_CMP0_CLKEN;
}

/**
  \fn     static inline void disable_cmp_periph_clk(void)
  \brief  Disable CMP Control register.
  \param  none.
  \return none.
 */
static inline void disable_cmp_periph_clk(void)
{
    CLKCTL_PER_SLV->CMP_CTRL &= ~CMP_CTRL_CMP0_CLKEN;
}

#endif /* ANALOG_CONFIG_H_ */

