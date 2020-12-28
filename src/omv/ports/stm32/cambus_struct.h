/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * STM32 port camera bus struct definition.
 */
#ifndef __CAMBUS_STRUCT_H__
#define __CAMBUS_STRUCT_H__

#include STM32_HAL_H

typedef enum _cambus_speed {
    #if defined(STM32F4)
    CAMBUS_SPEED_STANDARD     =(100000U),
    CAMBUS_SPEED_FULL         =(400000U),
    CAMBUS_SPEED_FAST         =(400000U),
    #elif defined(STM32F7)
    // These timing values are for f_I2CCLK=54MHz and are only approximate
    CAMBUS_SPEED_STANDARD     =(0x1090699B),
    CAMBUS_SPEED_FULL         =(0x70330309),
    CAMBUS_SPEED_FAST         =(0x50100103),
    #elif defined(STM32H7)
    // I2C timing obtained from the CUBEMX.
    CAMBUS_SPEED_STANDARD     =(0x20D09DE7),
    CAMBUS_SPEED_FULL         =(0x40900C22),
    CAMBUS_SPEED_FAST         =(0x4030040B),
    #else
    #error "no I2C timings for this MCU"
    #endif
} cambus_speed_t ;

typedef struct _cambus {
    uint32_t id;
    uint32_t speed;
    // This pointer will be set to its respective I2C handle defined in MicroPython
    // because all I2C IRQs are defined in stm32_it.c and handled by MicroPython.
    I2C_HandleTypeDef *i2c;
} cambus_t;
#endif // __CAMBUS_STRUCT_H__
