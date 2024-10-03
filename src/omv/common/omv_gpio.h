/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2023-2024 OpenMV, LLC.
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
 * GPIO abstraction layer.
 */
#ifndef __OMV_GPIO_H__
#define __OMV_GPIO_H__
#include <stdint.h>
#include <stdbool.h>
#include "omv_portconfig.h"

// Config options are defined in ports so they can be used
// directly to initialize peripherals without remapping them.

typedef void (*omv_gpio_callback_t) (void *data);

typedef struct _omv_gpio_irq_descr {
    bool enabled;
    void *data;
    omv_gpio_callback_t callback;
    #ifdef OMV_GPIO_IRQ_DESCR_PORT_BITS
    // Additional port-specific fields used internally.
    OMV_GPIO_IRQ_DESCR_PORT_BITS
    #endif
} omv_gpio_irq_descr_t;

void omv_gpio_init0(void);
void omv_gpio_config(omv_gpio_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint32_t af);
void omv_gpio_deinit(omv_gpio_t pin);
bool omv_gpio_read(omv_gpio_t pin);
void omv_gpio_write(omv_gpio_t pin, bool value);
void omv_gpio_irq_register(omv_gpio_t pin, omv_gpio_callback_t callback, void *data);
void omv_gpio_irq_enable(omv_gpio_t pin, bool enable);
void omv_gpio_clock_enable(omv_gpio_t pin, bool enable);
#endif // __OMV_GPIO_H__
