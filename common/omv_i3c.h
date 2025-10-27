/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2013-2025 OpenMV, LLC.
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

/* I3C CCC (Common Command Codes) related definitions */
#define I3C_CCC_DIRECT                                  BIT(7)

#define I3C_CCC_ID(id, broadcast)                       ((id) | ((broadcast) ? 0 : I3C_CCC_DIRECT))

/* Commands valid in both broadcast and unicast modes */
#define I3C_CCC_ENEC(broadcast)                         I3C_CCC_ID(0x0, broadcast)
#define I3C_CCC_DISEC(broadcast)                        I3C_CCC_ID(0x1, broadcast)
#define I3C_CCC_ENTAS(as, broadcast)                    I3C_CCC_ID(0x2 + (as), broadcast)
#define I3C_CCC_RSTDAA(broadcast)                       I3C_CCC_ID(0x6, broadcast)
#define I3C_CCC_SETMWL(broadcast)                       I3C_CCC_ID(0x9, broadcast)
#define I3C_CCC_SETMRL(broadcast)                       I3C_CCC_ID(0xa, broadcast)
#define I3C_CCC_SETXTIME(broadcast)                     ((broadcast) ? 0x28 : 0x98)
#define I3C_CCC_VENDOR(id, broadcast)                   ((id) + ((broadcast) ? 0x61 : 0xe0))

/* Broadcast-only commands */
#define I3C_CCC_ENTDAA                                  I3C_CCC_ID(0x7, true)
#define I3C_CCC_DEFSLVS                                 I3C_CCC_ID(0x8, true)
#define I3C_CCC_ENTTM                                   I3C_CCC_ID(0xb, true)
#define I3C_CCC_ENTHDR(x)                               I3C_CCC_ID(0x20 + (x), true)
#define I3C_CCC_SETAASA                                 I3C_CCC_ID(0x29, true)

/* Unicast-only commands */
#define I3C_CCC_SETDASA                                 I3C_CCC_ID(0x7, false)
#define I3C_CCC_SETNEWDA                                I3C_CCC_ID(0x8, false)
#define I3C_CCC_GETMWL                                  I3C_CCC_ID(0xb, false)
#define I3C_CCC_GETMRL                                  I3C_CCC_ID(0xc, false)
#define I3C_CCC_GETPID                                  I3C_CCC_ID(0xd, false)
#define I3C_CCC_GETBCR                                  I3C_CCC_ID(0xe, false)
#define I3C_CCC_GETDCR                                  I3C_CCC_ID(0xf, false)
#define I3C_CCC_GETSTATUS                               I3C_CCC_ID(0x10, false)
#define I3C_CCC_GETACCMST                               I3C_CCC_ID(0x11, false)
#define I3C_CCC_SETBRGTGT                               I3C_CCC_ID(0x13, false)
#define I3C_CCC_GETMXDS                                 I3C_CCC_ID(0x14, false)
#define I3C_CCC_GETHDRCAP                               I3C_CCC_ID(0x15, false)
#define I3C_CCC_GETXTIME                                I3C_CCC_ID(0x19, false)

/* List of some Defining byte values */
#define I3C_CCC_DEF_BYTE_SYNC_TICK                      0x7F
#define I3C_CCC_DEF_BYTE_DELAY_TIME                     0xBF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE0                    0xDF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE1                    0xEF
#define I3C_CCC_DEF_BYTE_ASYNC_MODE2                    0xF7
#define I3C_CCC_DEF_BYTE_ASYNC_MODE3                    0xFB
#define I3C_CCC_DEF_BYTE_ASYNC_TRIG                     0xFD
#define I3C_CCC_DEF_BYTE_TPH                            0x3F
#define I3C_CCC_DEF_BYTE_TU                             0x9F
#define I3C_CCC_DEF_BYTE_ODR                            0x8F

// Transfer speeds
typedef enum _omv_i3c_speed {
    OMV_I3C_SPEED_SDR = (OMV_I2C_SPEED_MAX),
    OMV_I3C_SPEED_HDR = (OMV_I2C_SPEED_MAX+1U),
    OMV_I3C_SPEED_MAX = (OMV_I2C_SPEED_MAX+2U)
} omv_i3c_speed_t;

int omv_i3c_init(omv_i2c_t *i3c, uint32_t bus_id, uint32_t speed);
int omv_i3c_deinit(omv_i2c_t *i3c);
int omv_i3c_assign(omv_i2c_t *i3c, uint8_t static_addr, uint8_t *dyn_addr);
int omv_i3c_scan_assign(omv_i2c_t *i3c, uint8_t *list, uint8_t size);
int omv_i3c_enable(omv_i2c_t *i3c, bool enable);
int omv_i3c_reset(omv_i2c_t *i3c, uint8_t tgt_addr);
int omv_i3c_set_scl(omv_i2c_t *i3c, uint32_t speed);
int omv_i3c_readb(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr,  uint8_t *reg_data);
int omv_i3c_writeb(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint8_t reg_data);
int omv_i3c_readb2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr,  uint8_t *reg_data);
int omv_i3c_writeb2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint8_t reg_data);
int omv_i3c_readw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr,  uint16_t *reg_data);
int omv_i3c_writew(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint16_t reg_data);
int omv_i3c_readw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr,  uint16_t *reg_data);
int omv_i3c_writew2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint16_t reg_data);
int omv_i3c_readdw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr,  uint32_t *reg_data);
int omv_i3c_writedw(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t reg_addr, uint32_t reg_data);
int omv_i3c_readdw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr,  uint32_t *reg_data);
int omv_i3c_writedw2(omv_i2c_t *i3c, uint8_t tgt_addr, uint16_t reg_addr, uint32_t reg_data);
int omv_i3c_read_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags);
int omv_i3c_write_bytes(omv_i2c_t *i3c, uint8_t tgt_addr, uint8_t *buf, int len, uint32_t flags);
#endif // __OMV_I3C_H__
