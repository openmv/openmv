/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * I3C bus abstraction layer.
 */
#ifndef __OMV_I3C_H__
#define __OMV_I3C_H__
#include <stdint.h>
#include <stdbool.h>
#include "omv_i2c.h"

// Transfer speeds
typedef enum _omv_i3c_speed {
    OMV_I3C_SPEED_SDR = (0U),
    OMV_I3C_SPEED_HDR = (1U)
} omv_i3c_speed_t;

int omv_i3c_init(omv_i2c_t *i3c, uint32_t bus_id, uint32_t speed);
int omv_i3c_deinit(omv_i2c_t *i3c);
int omv_i3c_assign(omv_i2c_t *i3c, uint8_t static_addr, uint8_t *dyn_addr);
int omv_i3c_scan_assign(omv_i2c_t *i3c, uint8_t *list, uint8_t size);
int omv_i3c_enable(omv_i2c_t *i3c, bool enable);
int omv_i3c_gencall(omv_i2c_t *i3c, uint8_t cmd);
int omv_i3c_pulse_scl(omv_i2c_t *i3c);
int omv_i3c_readb(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr,  uint8_t *reg_data);
int omv_i3c_writeb(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint8_t reg_data);
int omv_i3c_readb2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr,  uint8_t *reg_data);
int omv_i3c_writeb2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint8_t reg_data);
int omv_i3c_readw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr,  uint16_t *reg_data);
int omv_i3c_writew(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint16_t reg_data);
int omv_i3c_readw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr,  uint16_t *reg_data);
int omv_i3c_writew2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint16_t reg_data);
int omv_i3c_read_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags);
int omv_i3c_write_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags);
#endif // __OMV_I3C_H__
