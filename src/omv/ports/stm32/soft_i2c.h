/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Software I2C implementation.
 */
#ifndef __SOFT_I2C_H__
#define __SOFT_I2C_H__
#include <stdint.h>
int soft_i2c_read_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop);
int soft_i2c_write_bytes(uint8_t slv_addr, uint8_t *buf, int len, bool stop);
void soft_i2c_init();
void soft_i2c_deinit();
#endif // __SOFT_I2C_H__
