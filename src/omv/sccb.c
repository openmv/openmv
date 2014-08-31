#include <stm32f4xx_hal.h>
#include "sccb.h"
#include "pincfg.h"
#include "mdefs.h"

/* I2C defs */
#define SCCB_FREQ       (30000)
#define SLAVE_ADDR      (0x60)
#define TIMEOUT         (100000)
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

    HAL_I2C_DeInit(&I2CHandle);

    if (HAL_I2C_Init(&I2CHandle) != HAL_OK) {
        /* Initialization Error */
        BREAK();
    }
}

uint8_t SCCB_Write(uint8_t addr, uint8_t data)
{
    uint8_t buf[] = {addr, data};
    while (HAL_I2C_GetState(&I2CHandle) != HAL_I2C_STATE_READY);
    if (HAL_I2C_Master_Transmit(&I2CHandle, SLAVE_ADDR, buf, 2, TIMEOUT) != HAL_OK) {
        return 0xFF;
    }
    return 0;
}

uint8_t SCCB_Read(uint8_t addr)
{
    uint8_t data=0;

    while (HAL_I2C_GetState(&I2CHandle) != HAL_I2C_STATE_READY);
    if (HAL_I2C_Master_Transmit(&I2CHandle, SLAVE_ADDR, &addr, 1, TIMEOUT) != HAL_OK) {
        return 0xFF;
    }

    while (HAL_I2C_GetState(&I2CHandle) != HAL_I2C_STATE_READY);
    if (HAL_I2C_Master_Receive(&I2CHandle, SLAVE_ADDR, &data, 1, TIMEOUT) != HAL_OK) {
        return 0xFF;
    }
    return data;
}
