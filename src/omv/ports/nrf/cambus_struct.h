/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * NRF port camera bus struct definition.
 */
#ifndef __CAMBUS_STRUCT_H__
#define __CAMBUS_STRUCT_H__

#include "nrfx_twi.h"

typedef enum _cambus_speed {
    CAMBUS_SPEED_STANDARD  = TWI_FREQUENCY_FREQUENCY_K100, ///< 100 kbps
    CAMBUS_SPEED_FULL      = TWI_FREQUENCY_FREQUENCY_K250, ///< 250 kbps
    CAMBUS_SPEED_FAST      = TWI_FREQUENCY_FREQUENCY_K400  ///< 400 kbps
} cambus_speed_t;

typedef struct _cambus {
    uint32_t id;
    uint32_t speed;
    uint32_t scl_pin;
    uint32_t sda_pin;
    uint32_t initialized;
    nrfx_twi_t twi;
} cambus_t;
#endif // __CAMBUS_STRUCT_H__
