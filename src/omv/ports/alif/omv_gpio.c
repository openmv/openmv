/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * GPIO port for Alif.
 */
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "omv_gpio.h"
#include "omv_common.h"

static inline GPIO_Type *omv_gpio_get_base(omv_gpio_t pin) {
    GPIO_Type *gpio = NULL;
    if (pin->port == 15) {
        gpio = (GPIO_Type *) LPGPIO_BASE;
    } else {
        gpio = (GPIO_Type *) (GPIO0_BASE + (pin->port * (GPIO1_BASE - GPIO0_BASE)));
    }
    return gpio;
}

void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af) {
    GPIO_Type *gpio = omv_gpio_get_base(pin);
    uint16_t pad_config = (mode | pull | speed | pin->ren) & 0xFF;

    af = (af == -1) ? pin->af : af;
    pinconf_set(pin->port, pin->pin, af, pad_config);

    // If pad was configured for GPIO module, set the pin config.
    switch (mode) {
        case OMV_GPIO_MODE_ALT:
        case OMV_GPIO_MODE_ALT_OD: {
            // No GPIO config.
            break;
        }
        case OMV_GPIO_MODE_INPUT: {
            gpio_set_direction_input(gpio, pin->pin);
            break;
        }
        case OMV_GPIO_MODE_OUTPUT:
        case OMV_GPIO_MODE_OUTPUT_OD: {
            gpio_set_direction_output(gpio, pin->pin);
            break;
        }
        case OMV_GPIO_MODE_IT_FALL: {
            break;
        }
        case OMV_GPIO_MODE_IT_RISE: {
            break;
        }
        case OMV_GPIO_MODE_IT_BOTH: {
            break;
        }
    }
}

void omv_gpio_deinit(omv_gpio_t pin) {

}

bool omv_gpio_read(omv_gpio_t pin) {
    GPIO_Type *gpio = omv_gpio_get_base(pin);
    return gpio_get_value(gpio, pin->pin);
}

void omv_gpio_write(omv_gpio_t pin, bool value) {
    GPIO_Type *gpio = omv_gpio_get_base(pin);
    if (value) {
        gpio_set_value_high(gpio, pin->pin);
    } else{
        gpio_set_value_low(gpio, pin->pin);
    }
}
