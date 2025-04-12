/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2021-2024 OpenMV, LLC.
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
 * GPIO port for rp2.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"

void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af) {
    if (mode == OMV_GPIO_MODE_ALT) {
        gpio_set_function(pin, af);
    } else {
        gpio_init(pin);
        gpio_set_dir(pin, mode);
    }
    if (pull == OMV_GPIO_PULL_UP) {
        gpio_pull_up(pin);
    } else if (pull == OMV_GPIO_PULL_DOWN) {
        gpio_pull_down(pin);
    }
    gpio_set_slew_rate(pin, speed);
}

void omv_gpio_deinit(omv_gpio_t pin) {
    gpio_deinit(pin);
}

bool omv_gpio_read(omv_gpio_t pin) {
    return gpio_get(pin);
}

void omv_gpio_write(omv_gpio_t pin, bool value) {
    gpio_put(pin, value);
}
