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
 * @file     pinconf.c
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     02-Feb-2023
 * @brief    Pin configuration.
 ******************************************************************************/
#include <stddef.h>
#include <pinconf.h>
#include "global_map.h"

#define PINMUX_ALTERNATE_FUNCTION_MASK                      7U
#define PADCTRL_SHIFT                                       16U
#define LPGPIO_PADCTRL_SHIFT                                0U
#define PADCTRL_MASK                                        0xFFU

/**
  \fn          static inline void lpgpio_pinconf_set(const uint8_t pin, const uint8_t pad_ctrl)
  \brief       Set pad control configuration for a pin in the LPGPIO port.
  \param[in]   pin        GPIO pin
  \param[in]   pad_ctrl   Pad control configuration for the pin
*/
static inline void lpgpio_pinconf_set(const uint8_t pin, const uint8_t pad_ctrl)
{
    *((volatile uint32_t *) (LPGPIO_CTRL_BASE + (pin << 2))) = pad_ctrl;
}

/**
  \fn          static inline void lpgpio_pinconf_set(const uint8_t pin, const uint8_t pad_ctrl)
  \brief       Get pad control configuration for a pin in the LPGPIO port.
  \param[in]   pin        GPIO pin
  \param[out]  pad_ctrl   Current pad control configuration for the pin
*/
static inline void lpgpio_pinconf_get(const uint8_t pin, uint8_t *pad_ctrl)
{
    uint32_t val;

    val = *((volatile uint32_t *) (LPGPIO_CTRL_BASE + (pin << 2)));
    *pad_ctrl = (uint8_t) ((val >> LPGPIO_PADCTRL_SHIFT) & PADCTRL_MASK);
}

/**
  \fn          int32_t pinconf_set(const uint8_t port, const uint8_t pin, const uint8_t alt_func, const uint8_t pad_ctrl)
  \brief       Set pinmux and pad control configuration for a pin.
  \param[in]   port       GPIO port
  \param[in]   pin        GPIO pin
  \param[in]   alt_func   The alternate function to be chosen
  \param[in]   pad_ctrl   Pad control configuration for the pin
  \return      execution_status
*/
int32_t pinconf_set(const uint8_t port, const uint8_t pin, const uint8_t alt_func, const uint8_t pad_ctrl)
{
    uint32_t offset;

    if ((port > PORT_15) || (pin > PIN_7))
    {
        return -1;
    }

    if (port == PORT_15)
    {
        /*
         * LPGPIO pad control configuration. Note that alt_func is ignored for this
         * group.
         */
        lpgpio_pinconf_set(pin, pad_ctrl);
        return 0;
    }

    /*
     * A single 32 bit register holds the pinmux and padctrl information for one pin.
     * Each port has 8 pins. Thus the offset = port * 32 + pin * 4.
     */
    offset = (uint32_t) ((port << 5) + (pin << 2));

    *((volatile uint32_t *) (PINMUX_BASE + offset)) = ((uint32_t) pad_ctrl << 16) | alt_func;

    return 0;
}

/**
  \fn          int32_t pinconf_get(const uint8_t port, const uint8_t pin, uint8_t *alt_func, uint8_t *pad_ctrl)
  \brief       Get pinmux and pad control configuration for a pin.
  \param[in]   port       GPIO port.
  \param[in]   pin        GPIO pin.
  \param[out]  alt_func   Current alternate function selected for the pin.
  \param[out]  pad_ctrl   Current pad control configuration for the pin.
  \return      execution_status
*/
int32_t pinconf_get(const uint8_t port, const uint8_t pin, uint8_t *alt_func, uint8_t *pad_ctrl)
{
    uint32_t offset, val;

    if ((port > PORT_15) || (pin > PIN_7) || (alt_func == NULL) || (pad_ctrl == NULL))
    {
        return -1;
    }

    if (port == PORT_15)
    {
        /* No alternate function for pads in the LPGPIO port */
        *alt_func = 0U;
        lpgpio_pinconf_get(pin, pad_ctrl);
        return 0;
    }

    /*
     * A single 32 bit register holds the pinmux and padctrl information for one pin.
     * Each port has 8 pins. Thus the offset = port * 32 + pin * 4.
     */
    offset = (uint32_t) ((port << 5) + (pin << 2));

    val = *((volatile uint32_t *) (PINMUX_BASE + offset));

    *alt_func = (uint8_t) (val & PINMUX_ALTERNATE_FUNCTION_MASK);
    *pad_ctrl = (uint8_t) (val >> PADCTRL_SHIFT) & PADCTRL_MASK;

    return 0;
}
