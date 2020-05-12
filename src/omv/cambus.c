/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 */
#include <stdbool.h>
#include <stddef.h>
#include STM32_HAL_H
#include "systick.h"
#include "omv_boardconfig.h"
#include "cambus.h"
#define I2C_FREQUENCY   (100000)
#define I2C_TIMEOUT     (1000)

int cambus_init(I2C_HandleTypeDef *i2c, I2C_TypeDef *instance, uint32_t timing)
{
    /* Configure I2C */
    i2c->Instance             = instance;
    i2c->Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    #if !defined(STM32F4)
    i2c->Init.Timing          = timing;
    #else
    i2c->Init.ClockSpeed      = I2C_FREQUENCY;
    i2c->Init.DutyCycle       = I2C_DUTYCYCLE_2;
    #endif
    i2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    i2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    i2c->Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
    i2c->Init.OwnAddress1     = 0xFE;
    i2c->Init.OwnAddress2     = 0xFE;

    if (HAL_I2C_Init(i2c) != HAL_OK) {
        /* Initialization Error */
        return -1;
    }

    if (timing == I2C_TIMING_FAST) {
        // Enable FAST mode plus.
        if (instance == I2C1) {
            #if defined(I2C_FASTMODEPLUS_I2C1)
            HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
            #endif
        } else if (instance == I2C2) {
            #if defined(I2C_FASTMODEPLUS_I2C2)
            HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
            #endif
        } else if (instance == I2C3) {
            #if defined(I2C_FASTMODEPLUS_I2C3)
            HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C3);
            #endif
        } else {
            #if defined(I2C_FASTMODEPLUS_I2C4)
            HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C4);
            #endif
        }
    }
    return 0;
}

int cambus_deinit(I2C_HandleTypeDef *i2c)
{
    if (i2c->Instance) {
        HAL_I2C_DeInit(i2c);
    }
    i2c->Instance = NULL;
    return 0;
}

int cambus_scan(I2C_HandleTypeDef *i2c)
{
    for (uint8_t addr=0x09; addr<=0x77; addr++) {
        __disable_irq();
        if (HAL_I2C_IsDeviceReady(i2c, addr << 1, 10, I2C_TIMEOUT) == HAL_OK) {
            __enable_irq();
            return (addr << 1);
        }
        __enable_irq();
    }

    return 0;
}

int cambus_gencall(I2C_HandleTypeDef *i2c, uint8_t cmd)
{
    if (HAL_I2C_Master_Transmit(i2c, 0x00, &cmd, 1, I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int cambus_readb(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
    int ret = 0;

    __disable_irq();
    if((HAL_I2C_Master_Transmit(i2c, slv_addr, &reg_addr, 1, I2C_TIMEOUT) != HAL_OK)
    || (HAL_I2C_Master_Receive (i2c, slv_addr, reg_data, 1, I2C_TIMEOUT) != HAL_OK)) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_writeb(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int ret=0;
    uint8_t buf[] = {reg_addr, reg_data};

    __disable_irq();
    if(HAL_I2C_Master_Transmit(i2c, slv_addr, buf, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_readb2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t *reg_data)
{
    int ret=0;
    __disable_irq();
    if (HAL_I2C_Mem_Read(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_16BIT, reg_data, 1, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_writeb2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data)
{
    int ret=0;
    __disable_irq();
    if (HAL_I2C_Mem_Write(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_16BIT, &reg_data, 1, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_readw(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t *reg_data)
{
    int ret=0;
    __disable_irq();
    if (HAL_I2C_Mem_Read(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_t*) reg_data, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    *reg_data = (*reg_data >> 8) | (*reg_data << 8);
    return ret;
}

int cambus_writew(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data)
{
    int ret=0;
    reg_data = (reg_data >> 8) | (reg_data << 8);
    __disable_irq();
    if (HAL_I2C_Mem_Write(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, (uint8_t*) &reg_data, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_readw2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t *reg_data)
{
    int ret=0;
    __disable_irq();
    if (HAL_I2C_Mem_Read(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_16BIT, (uint8_t*) reg_data, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    *reg_data = (*reg_data >> 8) | (*reg_data << 8);
    return ret;
}

int cambus_writew2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data)
{
    int ret=0;
    reg_data = (reg_data >> 8) | (reg_data << 8);
    __disable_irq();
    if (HAL_I2C_Mem_Write(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_16BIT, (uint8_t*) &reg_data, 2, I2C_TIMEOUT) != HAL_OK) {
        ret = -1;
    }
    __enable_irq();
    return ret;
}

int cambus_read_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *buf, int len)
{
    if (HAL_I2C_Mem_Read(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, len, I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int cambus_write_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *buf, int len)
{
    if (HAL_I2C_Mem_Write(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_8BIT, buf, len, I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int cambus_readw_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t *buf, int len)
{
    if (HAL_I2C_Mem_Read(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_16BIT, buf, len, I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int cambus_writew_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t *buf, int len)
{
    if (HAL_I2C_Mem_Write(i2c, slv_addr, reg_addr,
                I2C_MEMADD_SIZE_16BIT, buf, len, I2C_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}
