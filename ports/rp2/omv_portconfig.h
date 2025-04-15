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
