/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * GPIO port for nrf.
 */
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"
#include "hal/nrf_gpio.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"

void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af)
{
    (void) af; // Unsupported.
    uint32_t input = (mode == OMV_GPIO_MODE_INPUT) ?
        NRF_GPIO_PIN_INPUT_CONNECT : NRF_GPIO_PIN_INPUT_DISCONNECT;
    nrf_gpio_cfg(pin, mode, input, pull, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
}

void omv_gpio_deinit(omv_gpio_t pin)
{
    nrf_gpio_cfg_default(pin);
}

bool omv_gpio_read(omv_gpio_t pin)
{
    return nrf_gpio_pin_read(pin);
}

void omv_gpio_write(omv_gpio_t pin, bool value)
{
    nrf_gpio_pin_write(pin, value);
}
