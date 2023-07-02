/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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
