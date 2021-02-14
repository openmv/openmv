/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OpenMV STM32 port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__
#include STM32_HAL_H

// omv_gpio_t definition
typedef struct _omv_gpio {
    uint32_t      pin;
    GPIO_TypeDef *port;
} omv_gpio_t;

// cambus/i2c definition
// This pointer will be set to its respective I2C handle which is defined in MicroPython along with
// IRQ handlers, if this I2C is enabled in Micropython, or defined and handled in stm32fxxx_hal_msp.c.
typedef I2C_HandleTypeDef *omv_cambus_t;
#endif // __OMV_PORTCONFIG_H__
