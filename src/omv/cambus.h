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
int cambus_init();
int cambus_scan();
int cambus_readb(uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data);
int cambus_writeb(uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);
int cambus_readb2(uint8_t slv_addr, uint16_t reg_addr,  uint8_t *reg_data);
int cambus_writeb2(uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data);
int cambus_readw(uint8_t slv_addr, uint8_t reg_addr,  uint16_t *reg_data);
int cambus_writew(uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data);
int cambus_readw2(uint8_t slv_addr, uint16_t reg_addr,  uint16_t *reg_data);
int cambus_writew2(uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data);
#endif // __CAMBUS_H__
