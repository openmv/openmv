/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OpenMV RP2 port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// GPIO speeds.
#define OMV_GPIO_SPEED_LOW      GPIO_SLEW_RATE_SLOW
#define OMV_GPIO_SPEED_HIGH     GPIO_SLEW_RATE_FAST

// GPIO pull.
#define OMV_GPIO_PULL_NONE      0
#define OMV_GPIO_PULL_UP        1
#define OMV_GPIO_PULL_DOWN      2

// GPIO modes.
#define OMV_GPIO_MODE_INPUT     GPIO_IN
#define OMV_GPIO_MODE_OUTPUT    GPIO_OUT
#define OMV_GPIO_MODE_ALT       3

// omv_gpio_t definition
typedef uint32_t omv_gpio_t;

// omv_i2c_dev_t definition
typedef i2c_inst_t *omv_i2c_dev_t;

#endif // __OMV_PORTCONFIG_H__
