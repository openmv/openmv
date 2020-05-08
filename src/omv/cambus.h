/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Camera bus driver.
 */
#ifndef __CAMBUS_H__
#define __CAMBUS_H__
#include <stdint.h>
#include STM32_HAL_H

int cambus_init(I2C_HandleTypeDef *i2c, I2C_TypeDef *instance, uint32_t timing);
int cambus_deinit(I2C_HandleTypeDef *i2c);
int cambus_scan(I2C_HandleTypeDef *i2c);
int cambus_readb(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data);
int cambus_writeb(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);
int cambus_readb2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr,  uint8_t *reg_data);
int cambus_writeb2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data);
int cambus_readw(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint16_t *reg_data);
int cambus_writew(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data);
int cambus_readw2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr,  uint16_t *reg_data);
int cambus_writew2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data);
#endif // __CAMBUS_H__
