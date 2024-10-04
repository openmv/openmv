/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2020-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * GPIO port for nrf.
 */
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"
#include "hal/nrf_gpio.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"

void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af) {
    (void) af; // Unsupported.
    uint32_t input = (mode == OMV_GPIO_MODE_INPUT) ?
                     NRF_GPIO_PIN_INPUT_CONNECT : NRF_GPIO_PIN_INPUT_DISCONNECT;
    nrf_gpio_cfg(pin, mode, input, pull, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
}

void omv_gpio_deinit(omv_gpio_t pin) {
    nrf_gpio_cfg_default(pin);
}

bool omv_gpio_read(omv_gpio_t pin) {
    return nrf_gpio_pin_read(pin);
}

void omv_gpio_write(omv_gpio_t pin, bool value) {
    nrf_gpio_pin_write(pin, value);
}
