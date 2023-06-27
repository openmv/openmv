/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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

typedef void (*omv_gpio_callback_t)(void *data);

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
