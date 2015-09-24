/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_msp.h>
#include "sccb.h"
#include "mdefs.h"
#define SCCB_FREQ       (100000)
#define SLAVE_ADDR      (0x60)
#define TIMEOUT         (10000)
static I2C_HandleTypeDef I2CHandle;

void SCCB_Init()
{
    /* Configure I2C */
    I2CHandle.Instance             = SCCB_I2C;
    I2CHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2CHandle.Init.ClockSpeed      = SCCB_FREQ;
    I2CHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    I2CHandle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    I2CHandle.Init.GeneralCallMode = I2C_GENERALCALL_ENABLED;
    I2CHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
    I2CHandle.Init.OwnAddress1     = 0xFE;
    I2CHandle.Init.OwnAddress2     = 0xFE;

    if (HAL_I2C_Init(&I2CHandle) != HAL_OK) {
        /* Initialization Error */
        BREAK();
    }
}

uint8_t SCCB_Write(uint8_t addr, uint8_t data)
{
    uint8_t ret=0;
    uint8_t buf[] = {addr, data};

    __disable_irq();
    if (HAL_I2C_Master_Transmit(&I2CHandle, SLAVE_ADDR, buf, 2, TIMEOUT) != HAL_OK) {
        ret=0xFF;
    }
    __enable_irq();
    return ret;
}

uint8_t SCCB_Read(uint8_t addr)
{
    uint8_t data=0;

    __disable_irq();
    if (HAL_I2C_Master_Transmit(&I2CHandle, SLAVE_ADDR, &addr, 1, TIMEOUT) != HAL_OK) {
        data = 0xFF;
        goto error_w;
    }
    if (HAL_I2C_Master_Receive(&I2CHandle, SLAVE_ADDR, &data, 1, TIMEOUT) != HAL_OK) {
        data = 0xFF;
    }
error_w:
    __enable_irq();
    return data;
}
