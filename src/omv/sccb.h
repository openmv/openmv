/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#ifndef __SCCB_H__
#define __SCCB_H__
#include <stdint.h>
void SCCB_Init();
uint8_t SCCB_Read(uint8_t addr);
uint8_t SCCB_Write(uint8_t addr, uint8_t data);
#endif // __SCCB_H__
