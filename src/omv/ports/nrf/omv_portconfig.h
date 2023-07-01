/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
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
#endif // __OMV_PORTCONFIG_H__
