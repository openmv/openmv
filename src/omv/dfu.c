/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * USB debug support.
 *
 */

#include "stm32f4xx_hal_rcc.h"
#include "dfu.h"

// TODO: include this in the Reset_Handler
uint32_t *magicp = (uint32_t *) 0x20002000;

/** Set a flag in a special memory location that the Reset_Handler
 *  will check. If the magic number is placed in this location,
 *  the Reset_Handler will run the bootloader.
 *
 *  See startup_stm32fxxxxx.s
 */
void reset_to_dfu(void) {
    *magicp = MAGIC_NUMBER;
    NVIC_SystemReset();
}
