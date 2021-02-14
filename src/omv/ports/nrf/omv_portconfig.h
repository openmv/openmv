/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OpenMV nRF port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__

#include "nrfx_twi.h"

// omv_gpio_t definition
typedef uint32_t omv_gpio_t;

// cambus/i2c definition
typedef nrfx_twi_t omv_cambus_t;
#endif // __OMV_PORTCONFIG_H__
