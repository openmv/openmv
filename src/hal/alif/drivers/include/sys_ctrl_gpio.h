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
 * @file     sys_ctrl_gpio.h
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     24-04-2023
 * @brief    GPIO system control Specific Header file.
 ******************************************************************************/

#ifndef SYS_CTRL_GPIO_H_
#define SYS_CTRL_GPIO_H_

#ifdef  __cplusplus
extern "C"
{
#endif

#include "peripheral_types.h"

/**
  \fn          static void inline enable_gpio_debounce_clk (uint8_t instance)
  \brief       Enable GPIO Debounce clock from EXPMST0.
  \param       instance     instance number
  \return      none
*/
static inline void enable_gpio_debounce_clk (GPIO_INSTANCE instance)
{
    volatile uint32_t *gpio_ctrl = (volatile uint32_t*) ((&CLKCTL_PER_SLV->GPIO_CTRL0) + instance);

    /* enable EXPMST0 GPIO de-bounce clock. */
    *gpio_ctrl |= GPIO_CTRL_DB_CKEN;
}

/**
  \fn          static inline void disable_gpio_debounce_clk (uint8_t instance)
  \brief       Disable GPIO Debounce clock from EXPMST0.
  \param       instance     instance number
  \return      none
*/
static inline void disable_gpio_debounce_clk (GPIO_INSTANCE instance)
{
    volatile uint32_t *gpio_ctrl = (volatile uint32_t*) ((&CLKCTL_PER_SLV->GPIO_CTRL0) + instance);

    /* disable EXPMST0 GPIO de-bounce clock. */
    *gpio_ctrl &= ~GPIO_CTRL_DB_CKEN;
}

/**
  \fn          static inline void set_gpio_debounce_clkdiv (uint16_t clk_div, uint8_t instance)
  \brief       Set GPIO Debounce clock divider.
  \param       clk_div     debounce clock divider
  \param       instance    instance number
  \return      none
*/
static inline void set_gpio_debounce_clkdiv (uint16_t clk_div, GPIO_INSTANCE instance)
{
    volatile uint32_t *gpio_ctrl = (volatile uint32_t*) ((&CLKCTL_PER_SLV->GPIO_CTRL0) + instance);

    /* config debounce clock divisor */
    *gpio_ctrl |= clk_div;
}

/**
  \fn          static void inline set_flexio_gpio_voltage_1v8 (void)
  \brief       Set GPIO voltage to 1.8V
  \param       none
  \return      none
*/
static inline void set_flexio_gpio_voltage_1v8 (void)
{
    /* config gpio voltage as 1.8V */
    VBAT->GPIO_CTRL = GPIO_CTRL_VOLT_1V8;
}

/**
  \fn          static void inline set_flexio_gpio_voltage_3v3 (void)
  \brief       Set GPIO voltage to 3.3V
  \param       none
  \return      none
*/
static inline void set_flexio_gpio_voltage_3v3 (void)
{
    /* config gpio voltage as 3.3V */
    VBAT->GPIO_CTRL = ~GPIO_CTRL_VOLT_1V8;
}

#ifdef __cplusplus
}
#endif
#endif /* SYS_CTRL_GPIO_H_ */
