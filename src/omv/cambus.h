/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Camera bus driver.
 *
 */
#ifndef __CAMBUS_H__
#define __CAMBUS_H__
#include <stdint.h>
int cambus_init();
int cambus_scan();
int cambus_readb(uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data);
int cambus_writeb(uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);
int cambus_read(uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data, uint8_t reg_size);
int cambus_write(uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t reg_size);
#endif // __CAMBUS_H__
