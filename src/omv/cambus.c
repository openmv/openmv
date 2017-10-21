/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#include <stdbool.h>
#include STM32_HAL_H
#include <systick.h>
#include "omv_boardconfig.h"
#include "cambus.h"
#define I2C_FREQUENCY   (100000)
#define I2C_TIMEOUT     (1000)

static I2C_HandleTypeDef I2CHandle;

int cambus_init()
{
    /* Configure I2C */
    I2CHandle.Instance             = SCCB_I2C;
    I2CHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    #if defined(STM32F765xx) ||  defined(STM32F769xx)
    I2CHandle.Init.Timing          = 0x40912732; // 100KHz
    #else
    I2CHandle.Init.ClockSpeed      = I2C_FREQUENCY;
    I2CHandle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    #endif
    I2CHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    I2CHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    I2CHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
    I2CHandle.Init.OwnAddress1     = 0xFE;
    I2CHandle.Init.OwnAddress2     = 0xFE;

    if (HAL_I2C_Init(&I2CHandle) != HAL_OK) {
        /* Initialization Error */
        return -1;
    }
    return 0;
}

int cambus_scan()
{
    for (uint8_t addr=0x08; addr<=0x77; addr++) {
        __disable_irq();
        if (HAL_I2C_IsDeviceReady(&I2CHandle, addr << 1, 10, I2C_TIMEOUT) == HAL_OK) {
            __enable_irq();
            return (addr << 1);
        }
        __enable_irq();
    }

    return 0;
}

int cambus_readb(uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
    int ret = 0;

    __disable_irq();
    if((HAL_I2C_Master_Transmit(&I2CHandle, slv_addr, &reg_addr, 1, I2C_TIMEOUT) != HAL_OK)
    || (HAL_I2C_Master_Receive(&I2CHandle, slv_addr, reg_data, 1, I2C_TIMEOUT) != HAL_OK)) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_writeb(uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int ret=0;
    uint8_t buf[] = {reg_addr, reg_data};

    __disable_irq();
    if(HAL_I2C_Master_Transmit(&I2CHandle, slv_addr, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_readw(uint8_t slv_addr, uint8_t reg_addr, uint16_t *reg_data)
{
    int ret=0;
    __disable_irq();
    if (HAL_I2C_Mem_Read(&I2CHandle, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_t*) reg_data, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    *reg_data = (*reg_data >> 8) | (*reg_data << 8);
    return ret;
}

int cambus_writew(uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data)
{
    int ret=0;
    reg_data = (reg_data >> 8) | (reg_data << 8);
    __disable_irq();
    if (HAL_I2C_Mem_Write(&I2CHandle, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_t*) &reg_data, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}
