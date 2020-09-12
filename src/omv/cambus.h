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
#if defined(STM32F4)
#define I2C_TIMING_STANDARD     (0)
#define I2C_TIMING_FULL         (0)
#define I2C_TIMING_FAST         (0)
#elif defined(STM32F7)
// These timing values are for f_I2CCLK=54MHz and are only approximate
#define I2C_TIMING_STANDARD     (0x1090699B)
#define I2C_TIMING_FULL         (0x70330309)
#define I2C_TIMING_FAST         (0x50100103)
#elif defined(STM32H7)
// I2C timing obtained from the CUBEMX.
#define I2C_TIMING_STANDARD     (0x20D09DE7)
#define I2C_TIMING_FULL         (0x40900C22)
#define I2C_TIMING_FAST         (0x4030040B)
#else
#error "no I2C timings for this MCU"
#endif

int cambus_init(I2C_HandleTypeDef *i2c, I2C_TypeDef *instance, uint32_t timing);
int cambus_deinit(I2C_HandleTypeDef *i2c);
int cambus_scan(I2C_HandleTypeDef *i2c);
int cambus_gencall(I2C_HandleTypeDef *i2c, uint8_t cmd);
int cambus_readb(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data);
int cambus_writeb(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);
int cambus_readb2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr,  uint8_t *reg_data);
int cambus_writeb2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data);
int cambus_readw(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr,  uint16_t *reg_data);
int cambus_writew(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data);
int cambus_readw2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr,  uint16_t *reg_data);
int cambus_writew2(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data);
int cambus_read_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *buf, int len);
int cambus_write_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint8_t reg_addr, uint8_t *buf, int len);
int cambus_readw_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t *buf, int len);
int cambus_writew_bytes(I2C_HandleTypeDef *i2c, uint8_t slv_addr, uint16_t reg_addr, uint8_t *buf, int len);
#endif // __CAMBUS_H__
