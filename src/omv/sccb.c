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
#include "sccb.h"
#define SCCB_FREQ   (100000) // We don't need fast I2C. 100KHz is fine here.
#define TIMEOUT     (1000) /* Can't be sure when I2C routines return. Interrupts
while polling hardware may result in unknown delays. */
static I2C_HandleTypeDef I2CHandle;

int SCCB_Init()
{
    /* Configure I2C */
    I2CHandle.Instance             = SCCB_I2C;
    I2CHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    #if defined(STM32F765xx) ||  defined(STM32F769xx)
    I2CHandle.Init.Timing          = 0x20404768; // 10KHz
    #else
    I2CHandle.Init.ClockSpeed      = SCCB_FREQ;
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

uint8_t SCCB_Probe()
{
    uint8_t reg = 0x00;
    uint8_t slv_addr = 0x00;

    for (int i=0; i<127; i++) {
        if (HAL_I2C_Master_Transmit(&I2CHandle, i, &reg, 1, TIMEOUT) == HAL_OK) {
            slv_addr = i;
            break;
        }
        if (i!=126) {
            systick_sleep(1); // Necessary for OV7725 camera (not for OV2640).
        }
    }
    return slv_addr;
}

uint8_t SCCB_Read(uint8_t slv_addr, uint8_t reg)
{
    uint8_t data=0;

    __disable_irq();
    if((HAL_I2C_Master_Transmit(&I2CHandle, slv_addr, &reg, 1, TIMEOUT) != HAL_OK)
    || (HAL_I2C_Master_Receive(&I2CHandle, slv_addr, &data, 1, TIMEOUT) != HAL_OK)) {
        data=0xFF;
    }
    __enable_irq();
    return data;
}

uint8_t SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data)
{
    uint8_t ret=0;
    uint8_t buf[] = {reg, data};

    __disable_irq();
    if(HAL_I2C_Master_Transmit(&I2CHandle, slv_addr, buf, 2, TIMEOUT) != HAL_OK) {
        ret=0xFF;
    }
    __enable_irq();
    return ret;
}
