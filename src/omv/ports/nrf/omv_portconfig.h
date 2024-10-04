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
 * OpenMV nRF port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__

#include "nrfx_twi.h"

// GPIO speeds.
#define OMV_GPIO_SPEED_LOW      0
#define OMV_GPIO_SPEED_HIGH     0

// GPIO pull.
#define OMV_GPIO_PULL_NONE      NRF_GPIO_PIN_NOPULL
#define OMV_GPIO_PULL_UP        NRF_GPIO_PIN_PULLUP
#define OMV_GPIO_PULL_DOWN      NRF_GPIO_PIN_PULLDOWN

// GPIO modes.
#define OMV_GPIO_MODE_INPUT     NRF_GPIO_PIN_DIR_INPUT
#define OMV_GPIO_MODE_OUTPUT    NRF_GPIO_PIN_DIR_OUTPUT

// omv_gpio_t definition
typedef uint32_t omv_gpio_t;

// omv_i2c_dev_t definition
typedef nrfx_twi_t omv_i2c_dev_t;

#define OMV_I2C_MAX_8BIT_XFER   (65535U)
#define OMV_I2C_MAX_16BIT_XFER  (65535U)

#endif // __OMV_PORTCONFIG_H__
